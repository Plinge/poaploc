#pragma once
#ifndef _USE_MATH_DEFINES
#define _USE_MATH_DEFINES
#endif
#include <math.h>

class DoublePoint3;

class DoublePoint 
{
public:
	double x,y;
protected:
	void copy(const DoublePoint & other);
	void copy3(const DoublePoint3 & other);
public:
	DoublePoint();
	DoublePoint(double a,double b);
	virtual ~DoublePoint();
	DoublePoint(const DoublePoint& other);
	const DoublePoint& operator=(const DoublePoint& other);
	DoublePoint(const DoublePoint3& other);
	const DoublePoint& operator=(const DoublePoint3& other);
	void operator-=(DoublePoint sub);
	void operator-=(double yoffset);
	void operator+=(DoublePoint add);
	void operator+=(double yoffset);
	/** eukledian distance. */
	static double distance(const DoublePoint & a,const DoublePoint & b);
	/** angle. */
	static double angle(const DoublePoint & a,const DoublePoint & b);
	double distance(const DoublePoint & sub);
};

class DoubleRect {
public:
	double l,t,r,b;
	void copy(const DoubleRect & other);
public:
    DoubleRect();
	DoubleRect(double a,double b,double c,double d);
	DoubleRect(const DoubleRect& other);
	DoubleRect(const DoublePoint& p);
	const DoubleRect& operator=(const DoubleRect& other);
	double getWidth();
	double getHeight();
	double getTop();
	double getLeft();
	double getRight();
	double getBottom();
	void setLeft(double d);
	void setRight(double d);
	void setTop(double d);
	void setBottom(double d);
	void offset(double x,double y);
	double getCenterX();
	double getCenterY();
	void scale(double s);
	void inflate(double x);
};


class DoublePoint3
{
public:
    double x,y,z;
protected:
	void copy(const DoublePoint3 & other);
	void copy2(const DoublePoint & other);
public:
    DoublePoint3();
    DoublePoint3(double a,double b,double c);
    DoublePoint3(const DoublePoint3& other);
    DoublePoint3(const DoublePoint& p);

    const DoublePoint3& operator=(const DoublePoint3& other);

    const DoublePoint3& operator=(const DoublePoint& other);

    const DoublePoint3& operator+=(const DoublePoint3& other);

    const DoublePoint3& operator-=(const DoublePoint3& other);

    DoublePoint3 operator-(const DoublePoint3& other)
    {
        DoublePoint3 pt(*this);
        pt-=other;
        return pt;
    }

    DoublePoint3 operator+(const DoublePoint3& other)
    {
        DoublePoint3 pt(*this);
        pt+=other;
        return pt;
    }

    DoublePoint3 operator*(double f)
    {
        DoublePoint3 pt(*this);
        pt.x*=f;
        pt.y*=f;
        pt.z*=f;
        return pt;
    }

    DoublePoint3 operator*=(double f)
    {
        x*=f;
        y*=f;
        z*=f;
        return *this;
    }

    /** eukledian distance. */
    static double distance(const DoublePoint3 & a,const DoublePoint3 & b);

    /** eukledian distance. */
    static double distance(const DoublePoint & a,const DoublePoint3 & b);

    /** cartesian length (l2) */
    double length()
    {
        return x*x + y*y + z*z;
    }


    double distance(const DoublePoint3 & sub);

    static DoublePoint3 spherical(double r, double a, double e);

    static DoublePoint3 cross(DoublePoint3  a, DoublePoint3  b);
};


