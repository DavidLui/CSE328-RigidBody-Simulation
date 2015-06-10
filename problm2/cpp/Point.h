
// class for a 2D point (x, y)

#ifndef __POINT_H__
#define __POINT_H__

class CPoint
{
public:
	float x, y;
	CPoint();
	CPoint(float x, float y);
	~CPoint();
	CPoint& operator+(CPoint &);
	CPoint& operator-(CPoint &);
	float norm();
	bool invisible;
	bool setInvisible();
};

#endif