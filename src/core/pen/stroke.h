// Stroke structures and supporting functions

#ifndef STROKE_H
#define STROKE_H

#ifndef BOOL
#define BOOL int
#endif

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif


typedef struct {
	double x, y;
} Sample;


typedef enum {
	STROKE_LINE,
	STROKE_DOT,
	STROKE_SHAPE,
	STROKE_UNKNOWN
} StrokeType;


typedef struct {
	double minX, minY; // Minumum coordinate
	double maxX, maxY; // Maximum coordinate
	double centerX, centerY; // Center coordinate (average of max & min)
	double width, height; // Dimension (max - min)
	double area; // Rectangular area (width * height)
} Rect;


typedef struct {
	double x1, y1, x2, y2;
	double centerX, centerY;
	double length;
	double angleRadians, angleDegrees;
} Line;


#define MAX_SAMPLES 1000

typedef struct {
	Sample samples[MAX_SAMPLES]; // pen samples for this stroke
	int nSamples;               // number of samples
	Rect bounds;                // rectangular bound of the samples
	double radius;              // smallest enclosing radius for samples, around (bounds.centerX, bounds.centerY)
	double averageSampleDistance; // average distance from each sample to (bounds.centerX, bounds.centerY)
	StrokeType type;            // stroke type (see util.h)
	Line line;                  // line representation (only set for type = LINE)
} Stroke;


Rect newRect(double minX, double minY, double maxX, double maxY);

Stroke newStroke();
void addStrokeSample(Stroke *s, double x, double y);
void calculateStrokeProperties(Stroke *s);

Line newLine(double x1, double y1, double x2, double y2);
double projectPoint(Line *l, double x, double y);
double projectedX(Line *l, double t, BOOL limitToLine);
double projectedY(Line *l, double t, BOOL limitToLine);

double distance(double x1, double y1, double x2, double y2);
double fitLeastSquares(Stroke *stroke, Line *lineResult);
double strokeDistance(Stroke *s1, Stroke *s2);
double intersectionPoint(Line *l1, Line *l2);
BOOL doLinesIntersect(Line *l1, Line *l2);
BOOL doesLineIntersectBoundingCircle(Line *l, Stroke *s);

#endif //STROKE_H
