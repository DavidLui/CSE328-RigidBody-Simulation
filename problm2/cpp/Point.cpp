
#include "Point.h"
#include <cmath>

CPoint::CPoint()
{}

CPoint::CPoint(float x, float y)
{
	this->x = x;
	this->y = y;
	invisible = false;
}

CPoint::~CPoint()
{}

CPoint& CPoint::operator+(CPoint &r)
{
	CPoint result;
	result.x = x + r.x;
	result.y = x + r.y;
	return result;
}

CPoint& CPoint::operator-(CPoint &r)
{
	CPoint result;
	result.x = x - r.x;
	result.y = y - r.y;
	return result;
}

bool CPoint::setInvisible() {
	invisible = true;
	return false;
}

float CPoint::norm()
{
	return sqrt(x*x + y*y);
}