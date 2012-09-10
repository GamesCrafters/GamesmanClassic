// Stroke structures and supporting functions

#include "stroke.h"
#include <math.h>
#include <stdlib.h>


// thresholds for dot detection (each Anoto unit = 0.3 mm)
#define DOT_MAX_RADIUS              7.0
#define DOT_MAX_AVERAGE_DISTANCE    4.0
// threshold for max line deviation, as percentage of length
#define LINE_MAX_DEVIATION          0.1


// Stroke functions

Rect newRect(double minX, double minY, double maxX, double maxY)
{
	Rect r;
	r.minX = minX;
	r.minY = minY;
	r.maxX = maxX;
	r.maxY = maxY;
	r.width = maxX - minX;
	r.height = maxY - minY;
	r.centerX = (minX + maxX) / 2;
	r.centerY = (minY + maxY) / 2;
	r.area = r.width * r.height;
	return r;
}


Stroke newStroke()
{
	Stroke s;
	s.nSamples = 0;
	s.radius = 0;
	s.averageSampleDistance = 0;
	s.type = STROKE_UNKNOWN;
	return s;
}

void addStrokeSample(Stroke *s, double x, double y)
{
	if (s->nSamples < MAX_SAMPLES) {
		s->samples[s->nSamples] = (Sample){x, y};
		s->nSamples++;
	}
}

void calculateStrokeProperties(Stroke *s)
{
	if (s->nSamples == 0) {
		return;
	}

	// calculate bounds
	int idx;
	double minX = s->samples[0].x;
	double minY = s->samples[0].y;
	double maxX = minX, maxY = minY;
	for (idx = 1; idx < s->nSamples; idx++) {
		minX = fmin(minX, s->samples[idx].x);
		minY = fmin(minY, s->samples[idx].y);
		maxX = fmax(maxX, s->samples[idx].x);
		maxY = fmax(maxY, s->samples[idx].y);
	}
	s->bounds = newRect(minX, minY, maxX, maxY);

	// calculate radius, average sample distance
	double dist, totalDist = 0;
	s->radius = 0;
	for (idx = 0; idx < s->nSamples; idx++) {
		dist = distance(s->bounds.centerX, s->bounds.centerY, s->samples[idx].x, s->samples[idx].y);
		s->radius = fmax(s->radius, dist);
		totalDist += dist;
	}
	s->averageSampleDistance = totalDist / s->nSamples;

	// determine stroke type
	if ((s->radius <= DOT_MAX_RADIUS) && (s->averageSampleDistance <= DOT_MAX_AVERAGE_DISTANCE)) {
		s->type = STROKE_DOT;
	} else {
		Line tempLine;
		double fitDeviation = fitLeastSquares(s, &tempLine);
		if (fitDeviation <= LINE_MAX_DEVIATION) {
			s->type = STROKE_LINE;
			s->line = tempLine;
		} else {
			s->type = STROKE_SHAPE;
		}
	}
}


Line newLine(double x1, double y1, double x2, double y2)
{
	Line l;
	l.x1 = x1;
	l.y1 = y1;
	l.x2 = x2;
	l.y2 = y2;
	l.centerX = (x1 + x2) / 2;
	l.centerY = (y1 + y2) / 2;
	l.length = distance(x1, y1, x2, y2);
	l.angleRadians = atan2((y2 - y1), (x2 - x1));
	l.angleDegrees = l.angleRadians * 180 / M_PI;
	return l;
}


// Calculate the orthogonal projection of a point onto this line
// returns fraction of this line that the projection covers, negative if opposite direction
double projectPoint(Line *l, double x, double y)
{
	double ux = x - l->x1;
	double uy = y - l->y1;
	double vx = l->x2 - l->x1;
	double vy = l->y2 - l->y1;
	return (vx*ux + vy*uy) / (vx*vx + vy*vy);
}

// Calculate projected X coordinate from given fraction value
double projectedX(Line *l, double t, BOOL limitToLine)
{
	if ((limitToLine == TRUE) && (t <= 0)) {
		return l->x1;
	} else if ((limitToLine == TRUE) && (t >= 1)) {
		return l->x2;
	}
	return l->x1 + (l->x2 - l->x1) * t;
}

// Calculate projected Y coordinate from given fraction value
double projectedY(Line *l, double t, BOOL limitToLine)
{
	if ((limitToLine == TRUE) && (t <= 0)) {
		return l->y1;
	} else if ((limitToLine == TRUE) && (t >= 1)) {
		return l->y2;
	}
	return l->y1 + (l->y2 - l->y1) * t;
}


// General helper functions

double distance(double x1, double y1, double x2, double y2)
{
	double dx = x2 - x1;
	double dy = y2 - y1;
	return sqrt(dx*dx + dy*dy);
}

// Fit samples to a line using least squares approximation, return deviation percentage
double fitLeastSquares(Stroke *stroke, Line *lineResult)
{
	int n = stroke->nSamples;
	if (n == 0) {
		return 0;
	}

	// apply Least Squares
	double st = 0, st2 = 0;
	double sx = 0, sx2 = 0, stx = 0;
	double sy = 0, sy2 = 0, sty = 0;
	// note: no timestamps at the moment
	Sample s;
	double t, x, y;
	double boundX = stroke->bounds.minX, boundY = stroke->bounds.minY;
	int idx;
	for (idx = 0; idx < n; idx++) {
		s = stroke->samples[idx];
		// treat each timestamp ass 1ms apart
		t = idx + 1;
		st += t;
		st2 += t*t;
		x = s.x - boundX;
		y = s.y - boundY;
		sx += x;
		sx2 += x*x;
		stx += t*x;
		sy += y;
		sy2 += y*y;
		sty += t*y;
	}
	double sst = n*st2 - st*st;
	double sstx = n*stx - st*sx;
	double ssty = n*sty - st*sy;

	// Lines are of the form a + b*t, with a,b distinct in X and Y
	// Note: b values are premultiplied by 100 to add precision
	// The factor won't matter, since actual endpoints will be used
	double bx = 100*sstx / sst;
	double ax = (sx - bx*st)/n;
	double by = 100*ssty / sst;
	double ay = (sy - by*st)/n;

	// get bounds for the closest projected sample coordinates, calculate fit deviation
	double fitDeviation = 0;
	Line tempLine = newLine(ax, ay, ax+bx, ay+by);
	double minT, maxT;
	for (idx = 0; idx < n; idx++) {
		s = stroke->samples[idx];
		x = s.x - boundX;
		y = s.y - boundY;
		double newT = projectPoint(&tempLine, x, y);
		fitDeviation += distance(projectedX(&tempLine, newT, FALSE), projectedY(&tempLine, newT, FALSE), x, y);
		if (idx == 0) {
			minT = maxT = newT;
		} else {
			minT = fmin(minT, newT);
			maxT = fmax(maxT, newT);
		}
	}
	double x1 = projectedX(&tempLine, minT, FALSE);
	double y1 = projectedY(&tempLine, minT, FALSE);
	double x2 = projectedX(&tempLine, maxT, FALSE);
	double y2 = projectedY(&tempLine, maxT, FALSE);
	fitDeviation = fitDeviation / (n * distance(x1, y1, x2, y2)); // scale based on line length, sample count
	*lineResult = newLine(boundX + x1, boundY + y1, boundX + x2, boundY + y2);
	return fitDeviation;
}


// Compute distance between two strokes. Dots and Shapes use radius, Lines use perpendicular projection
double strokeDistance(Stroke *s1, Stroke *s2)
{
	double c1x = s1->bounds.centerX;
	double c1y = s1->bounds.centerY;
	double c2x = s2->bounds.centerX;
	double c2y = s2->bounds.centerY;
	double dist;
	if ((s1->type != STROKE_LINE) && (s2->type != STROKE_LINE)) {
		dist = distance(c1x, c1y, c2x, c2y) - (s1->radius + s2->radius);
	} else if (!((s1->type == STROKE_LINE) && (s2->type == STROKE_LINE))) {
		if (s2->type == STROKE_LINE) {
			// swap to use s1 as the line, s2 as shape
			Stroke *temp = s2;
			s2 = s1;
			s1 = temp;
			c2x = c1x;
			c2y = c1y;
		}
		double t = projectPoint(&(s1->line), c2x, c2y);
		dist = distance(projectedX(&(s1->line), t, TRUE), projectedY(&(s1->line), t, TRUE), c2x, c2y) - s2->radius;
	} else if (doLinesIntersect(&(s1->line), &(s2->line))) {
		dist = 0;
	} else {
		// non-intersecting lines, get shortest distance between endpoints and lines
		double distances[4];
		Line l1 = s1->line, l2 = s2->line;
		double t11 = projectPoint(&l1, l2.x1, l2.y1);
		double t12 = projectPoint(&l1, l2.x2, l2.y2);
		double t21 = projectPoint(&l2, l1.x1, l1.y1);
		double t22 = projectPoint(&l2, l1.x2, l1.y2);
		distances[0] = distance(projectedX(&l1, t11, TRUE), projectedY(&l1, t11, TRUE), l2.x1, l2.y1);
		distances[1] = distance(projectedX(&l1, t12, TRUE), projectedY(&l1, t12, TRUE), l2.x2, l2.y2);
		distances[2] = distance(projectedX(&l2, t21, TRUE), projectedY(&l2, t21, TRUE), l1.x1, l1.y1);
		distances[3] = distance(projectedX(&l2, t22, TRUE), projectedY(&l2, t22, TRUE), l1.x2, l1.y2);

		double dist = distances[0];
		int idx;
		for (idx = 1; idx < 4; idx++) {
			dist = fmin(dist, distances[idx]);
		}
	}
	return fmax(0, dist);
}

// calculates intersection (in terms of l1), or -1 when no intersection found
double intersectionPoint(Line *l1, Line *l2)
{
	double dx1 = (l1->x2 - l1->x1);
	double dx2 = (l2->x2 - l2->x1);
	double dy1 = (l1->y2 - l1->y1);
	double dy2 = (l2->y2 - l2->y1);
	double px = (l1->x1 - l2->x1);
	double py = (l1->y1 - l2->y1);

	double d = dy2*dx1 - dx2*dy1; // 0 when lines are parallel or collinear
	double t = dx2*py - dy2*px; // t/d is the distance along l1
	double s = dx1*py - dy1*px; // s/d is the distance along l2
	if (d == 0 && (t == 0 || s == 0)) {
		// collinear segments, check overlap
		double p1 = projectPoint(l1, l2->x1, l2->y1);
		double p2 = projectPoint(l1, l2->x2, l2->y2);

		if ((p1 <= 0 && p2 >= 1) || (p2 <= 0 && p1 >= 1)) {
			// l2 encloses l1
			return 0.5;
		}
		if ((p1 >= 0 && p1 <= 1) && (p2 >= 0 && p2 <= 1)) {
			// l1 encloses l2
			return (p1 + p2) / 2;
		}
		if (p1 >= 0 && p1 <= 1) {
			// p1 inside, p2 outside
			return (p2 < 0) ? (p2 / 2) : ((p2 + 1) / 2);
		}
		if (p2 >= 0 && p2 <= 1) {
			// p2 inside, p1 outside
			return (p1 < 0) ? (p1 / 2) : ((p1 + 1) / 2);
		}
		return -1;
	} else if (d == 0) {
		// parralel segments
		return -1;
	}
	t /= d;
	s /= d;
	if ((t >= 0 && t <= 1) && (s >= 0 && s <= 1)) {
		// intersection exists, use l1's coordinate
		return t;
	}
	// no intersection
	return -1;
}

BOOL doLinesIntersect(Line *l1, Line *l2)
{
	return (intersectionPoint(l1, l2) != -1);
}

BOOL doesLineIntersectBoundingCircle(Line *l, Stroke *s)
{
	double t = projectPoint(l, s->bounds.centerX, s->bounds.centerY);
	double x = projectedX(l, t, TRUE);
	double y = projectedY(l, t, TRUE);
	double d = distance(x, y, s->bounds.centerX, s->bounds.centerY);
	return (d <= s->radius);
}
