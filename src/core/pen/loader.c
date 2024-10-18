// Loader for pen strokes

#include "stroke.h"
#include <pthread.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/stat.h>
#include <errno.h>
#include <time.h>
#include <unistd.h>
#include <stdio.h>
#include <fcntl.h>
#include <stdarg.h>


// time to wait for samples
#define SAMPLE_WAIT_MICROS 30000

// Pen data headers
#define TYPE_SIMPLE_COORD   0x00
#define TYPE_PEN_UP         0x01
#define LENGTH_SIMPLE_COORD 0x0B
#define LENGTH_PEN_UP       0x00

// expected byte type for reading in samples
typedef enum {
	E_TYPE,
	E_LENGTH,
	E_X,
	E_Y,
	E_XFRACTION,
	E_YFRACTION,
	E_FORCE
} ExpectedByte;


int inFileDesc = -1;
char outFilename[50];
FILE *outFile = NULL, *logF = NULL;
BOOL alreadyFinished = FALSE, didWriteToOutFile;
int showDebug = 0;


// file used for logging output messages (Cygwin's wish shell doesn't show stdout/stderr)
FILE* logFile()
{
	if (!showDebug) {
		return stdout;
	}
	if (logF != NULL) {
		return logF;
	}
	logF = fopen("pen/log.txt", "a");
	if (logF == NULL) {
		return stdout;
	}
	time_t tt = time(NULL);
	fprintf(logF, "\n\nStarting log messages for: %s", ctime(&tt));
	return logF;
}

void printLog(char* formatString, ...) {
	if (!showDebug) return;
	va_list argp;
	va_start(argp, formatString);
	vfprintf(logFile(), formatString, argp);
	fflush(logFile());
	va_end(argp);
}



BOOL isPenLoaderStarted()
{
	return (inFileDesc == -1) ? FALSE : TRUE;
}

BOOL hasPenLoaderFinished()
{
	return alreadyFinished;
}

void closePenOutputFile()
{
	if (outFile == NULL) {
		return;
	}
	fclose(outFile);
	// clean up empty file
	if (!didWriteToOutFile) {
		printLog("Removing unused game file '%s'\n", outFilename);
		unlink(outFilename);
	}
}

// opens a new file for saving strokes (only when debug is on)
void startNewPenOutputFile()
{
	if (!isPenLoaderStarted() || !showDebug) {
		return;
	}
	closePenOutputFile();
	didWriteToOutFile = FALSE;
	time_t tt = time(NULL);
	struct tm *t = localtime(&tt);
	sprintf(outFilename, "pen/%02d%02d%02d_%02d%02d%02d.dat", (t->tm_year-100), (t->tm_mon+1), t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec);
	outFile = fopen(outFilename, "wb");
	if (outFile == NULL) {
		printLog("*** Warning: could not open '%s' for writing, will not be saving strokes\n", outFilename);
	} else {
		printLog("Started new game file '%s'\n", outFilename);
	}
}


// writes byte to output file
void writePenOutputByte(unsigned char byte, BOOL shouldFlush)
{
	if (outFile == NULL || !showDebug) {
		return;
	}
	didWriteToOutFile = TRUE;
	fputc(byte, outFile);
	if (shouldFlush) {
		fflush(outFile);
	}
}


// start the loader by opening input file
int startPenLoader(char *filename, int debug)
{
	showDebug = debug;
	// directory used for per-game pen data and log files
	if (showDebug) {
		mkdir("pen", 0755);
	}
	if (isPenLoaderStarted()) {
		// don't start again if already running
		printLog("*** Warning: Pen Loader is already running\n");
		return 1;
	}
	printLog("Starting Pen Loader\n");
	inFileDesc = open(filename, O_RDONLY | O_NONBLOCK, 0400);
	if (inFileDesc == -1) {
		printLog("*** Warning: unable to open '%s' to read pen data, proceeding without pen support\n", filename);
		return 2;
	}
	printLog("Opened input file '%s'\n", filename);
	startNewPenOutputFile();
	return 0;
}



// returns immediately if no data available (with nSamples = 0), otherwise reads the complete next stroke
Stroke getNextAvailableStroke()
{
	// temporary state
	BOOL haveStroke = FALSE;
	ExpectedByte expect = E_TYPE;
	int readResult, tempValue, valueCount, xInteger, yInteger;
	unsigned char curByte, curType, xFraction, yFraction;
	Stroke stroke = newStroke();

	if (inFileDesc == -1) {
		printLog("*** Warning: input file is not open\n");
		return stroke;
	}

	while (!haveStroke) {
		readResult = read(inFileDesc, &curByte, 1);
		if ((readResult == -1) && (errno == EAGAIN)) {
			// no data available - either exit or wait
			if ((expect == E_TYPE) && (stroke.nSamples == 0)) {
				return stroke;
			}
			usleep(SAMPLE_WAIT_MICROS);
			continue;
		} else if (readResult != 1) {
			printLog("*** Warning: reached EOF or got an error, stopping loader\n");
			closePenOutputFile();
			close(inFileDesc);
			inFileDesc = -1;
			stroke.nSamples = 0;
			alreadyFinished = TRUE;
			return stroke;
		}

		switch(expect) {
		case E_TYPE:
			if ((curByte != TYPE_SIMPLE_COORD) && (curByte != TYPE_PEN_UP)) {
				printLog("*** Invalid TYPE %d\n", curByte);
				break; // invalid, just skip byte and continue
			}
			curType = curByte;
			expect = E_LENGTH;
			valueCount = 0;
			break;

		case E_LENGTH:
			if ((valueCount == 0) && (curByte != 0)) {
				// first byte of 2-byte length should always be 0 - restart
				printLog("*** Invalid first LENGTH byte %d, restarting with TYPE\n", curByte);
				expect = E_TYPE;
				if ((curByte == TYPE_SIMPLE_COORD) || (curByte == TYPE_PEN_UP)) {
					expect = E_LENGTH;
					curType = curByte;
				}
			} else if (valueCount == 0) {
				// first byte of length is 0 - continue to next one
				valueCount = 1;
			} else if ((curType == TYPE_SIMPLE_COORD) && (curByte == LENGTH_SIMPLE_COORD)) {
				// header finished - write 3-byte header, prepare for sample data
				writePenOutputByte(TYPE_SIMPLE_COORD, FALSE);
				writePenOutputByte(0, FALSE);
				writePenOutputByte(LENGTH_SIMPLE_COORD, FALSE);
				expect = E_X;
				valueCount = 0;
				tempValue = 0;
			} else if ((curType == TYPE_PEN_UP) && (curByte == LENGTH_PEN_UP)) {
				// stroke finished - write 3-byte header to output file with flush, and exit
				writePenOutputByte(TYPE_PEN_UP, FALSE);
				writePenOutputByte(0, FALSE);
				writePenOutputByte(LENGTH_PEN_UP, TRUE);
				if (stroke.nSamples != 0) {
					haveStroke = TRUE;
				}
			} else {
				// unsupported length - restart
				printLog("*** Invalid second LENGTH byte %d, restarting with TYPE\n", curByte);
				expect = E_TYPE;
				if (curByte == 0) {
					// treat previous byte as TYPE_SIMPLE_COORD, and current byte as first byte of length
					valueCount = 1;
					curType = TYPE_SIMPLE_COORD;
					expect = E_LENGTH;
				}
			}
			break;

		case E_X:
			writePenOutputByte(curByte, FALSE);
			tempValue = (tempValue << 8) | (curByte & 0xFF);
			valueCount++;
			if (valueCount == 4) {
				// done with X
				xInteger = tempValue;
				expect = E_Y;
				valueCount = 0;
				tempValue = 0;
			}
			break;

		case E_Y:
			writePenOutputByte(curByte, FALSE);
			tempValue = (tempValue << 8) | (curByte & 0xFF);
			valueCount++;
			if (valueCount == 4) {
				// done with Y
				yInteger = tempValue;
				expect = E_XFRACTION;
			}
			break;

		case E_XFRACTION:
			writePenOutputByte(curByte, FALSE);
			xFraction = curByte;
			expect = E_YFRACTION;
			break;

		case E_YFRACTION:
			writePenOutputByte(curByte, FALSE);
			yFraction = curByte;
			expect = E_FORCE;
			break;

		case E_FORCE:
			// Finished with sample - calculate actual coordinates (curent force byte isn't used)
			writePenOutputByte(curByte, FALSE);
			expect = E_TYPE;
			double sampleX = xInteger + (0.125 * ((xFraction >> 5) & 7));
			double sampleY = yInteger + (0.125 * ((yFraction >> 5) & 7));
			if ((sampleX > 41940000) && (sampleX < 41950000)) {
				printLog("*** Ignoring sample in 'Streaming On/Off' area\n");
			} else {
				addStrokeSample(&stroke, sampleX, sampleY);
			}
			break;
		}
	}

	calculateStrokeProperties(&stroke);
	printLog(" * New Stroke: %d samples, center: (%.1f, %.1f), radius: %.1f, avg dist: %.1f, type: ", \
	         stroke.nSamples, stroke.bounds.centerX, stroke.bounds.centerY, stroke.radius, stroke.averageSampleDistance);
	if (stroke.type == STROKE_DOT) {
		printLog("DOT\n");
	} else if (stroke.type == STROKE_SHAPE) {
		printLog("SHAPE\n");
	} else if (stroke.type == STROKE_LINE) {
		printLog("LINE (%.1f, %.1f) - (%.1f, %.1f)\n", stroke.line.x1, stroke.line.y1, stroke.line.x2, stroke.line.y2);
	}

	return stroke;
}



