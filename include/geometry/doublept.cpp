#include "stdafx.h"
#include "geometry/doublept.h"

DoublePoint::DoublePoint()
{
}

DoublePoint::DoublePoint(double a,double b)
{
	x=a;y=b;
}

DoublePoint::~DoublePoint()
{
}

// created by CppFunctionExtractor, 10.07.2009, 19:49

void DoublePoint::operator-=(DoublePoint sub)
{
	x-=sub.x;
	y-=sub.y;		
}

void DoublePoint::operator-=(double yoffset)
{
	y-=yoffset;		
}

void DoublePoint::operator+=(DoublePoint add)
{
	x+=add.x;
	y+=add.y;		
}

void DoublePoint::operator+=(double yoffset)
{
	y+=yoffset;		
}

void DoublePoint::copy3(const DoublePoint3& other)
{
	x=other.x; y=other.y;
}

void DoublePoint::copy(const DoublePoint& other)
{
	x=other.x; y=other.y;
}

DoublePoint::DoublePoint(const DoublePoint& other)
{
	copy(other);
}

const DoublePoint & DoublePoint::operator=(const DoublePoint& other)
{
	copy(other);
	return *this;
}

DoublePoint::DoublePoint(const DoublePoint3& other)
{
	copy3(other);
}

const DoublePoint & DoublePoint::operator=(const DoublePoint3& other)
{
	copy3(other);
	return *this;
}

// extracted by CppFunctionExtractor, 29.06.2010, 20:49

double DoublePoint::distance(const DoublePoint& a,const DoublePoint& b)
{
	double dx = a.x - b.x;
	double dy = a.y - b.y;
	return sqrt(dx*dx+dy*dy);
}

double DoublePoint::angle(const DoublePoint& a,const DoublePoint& b)
{
	double dx = a.x - b.x;
	double dy = a.y - b.y;
	return atan2(dy,dx);
}

double DoublePoint::distance(const DoublePoint& sub)
{
	return distance(*this,sub);
}

void DoubleRect::copy(const DoubleRect& other)
{
	l=other.l;
	t=other.t;
	r=other.r;
	b=other.b;
}

DoubleRect::DoubleRect(): l(0.0), t(0.0), r(0.0), b(0.0)
{
}

DoubleRect::DoubleRect(double a,double b,double c,double d): l(a), t(b), r(c), b(d)
{
}

DoubleRect::DoubleRect(const DoubleRect& other)
{
	copy(other);
}

DoubleRect::DoubleRect(const DoublePoint& p)
{
	l=r=p.x;
	t=b=p.y;
}

const DoubleRect & DoubleRect::operator=(const DoubleRect& other)
{
	copy(other);
	return *this;
}

double DoubleRect::getWidth()
{
	return r-l;
}

double DoubleRect::getHeight()
{
	return b-t;
}

double DoubleRect::getTop()
{
	return t;
}

double DoubleRect::getLeft()
{
	return l;
}

double DoubleRect::getRight()
{
	return r;
}

double DoubleRect::getBottom()
{
	return b;
}

void DoubleRect::setLeft(double d)
{
	l=d; 
}

void DoubleRect::setRight(double d)
{
	r=d; 
}

void DoubleRect::setTop(double d)
{
	t=d; 
}

void DoubleRect::setBottom(double d)
{
	b=d; 
}

void DoubleRect::offset(double x,double y)
{
	r+=x;l+=x;
	t+=y;b+=y;
}

double DoubleRect::getCenterX()
{
	return (r+l)/2.0;
}

double DoubleRect::getCenterY()
{
	return (t+b)/2.0;
}

void DoubleRect::scale(double s)
{
	double w = getWidth()*s;
	double h = getHeight()*s;
	double x = getCenterX();
	double y = getCenterY();
	l = x - w/2.0;
	r = x + w/2.0;
	t = y - h/2.0;
	b = y + h/2.0;
}

void DoubleRect::inflate(double x)
{
	r+=x;
	l-=x;
	t-=x;
	b+=x;
}

void DoublePoint3::copy(const DoublePoint3& other)
{
        x=other.x;
        y=other.y;
        z=other.z;
}

void DoublePoint3::copy2(const DoublePoint& other)
{
        x=other.x;
        y=other.y;
        z=0.0;
}

DoublePoint3::DoublePoint3(): x(0.0),y(0.0),z(0.0)
{
}

DoublePoint3::DoublePoint3(double a,double b,double c): x(a), y(b), z(c)
{
}

DoublePoint3::DoublePoint3(const DoublePoint3& other)
{
        copy(other);
}

DoublePoint3::DoublePoint3(const DoublePoint& p)
{
        copy2(p);
}

const DoublePoint3 & DoublePoint3::operator=(const DoublePoint3& other)
{
        copy(other);
        return *this;
}

const DoublePoint3 & DoublePoint3::operator=(const DoublePoint& other)
{
        copy2(other);
        return *this;
}

const DoublePoint3 & DoublePoint3::operator-=(const DoublePoint3& other)
{
    x -= other.x;
    y -= other.y;
    z -= other.z;
    return *this;
}

double DoublePoint3::distance(const DoublePoint3& a,const DoublePoint3& b)
{
        double dx = a.x - b.x;
        double dy = a.y - b.y;
        double dz = a.z - b.z;
        return sqrt(dx*dx+dy*dy+dz*dz);
}

double DoublePoint3::distance(const DoublePoint& a,const DoublePoint3& b)
{
        double dx = a.x - b.x;
        double dy = a.y - b.y;
        double dz = b.z;
        return sqrt(dx*dx+dy*dy+dz*dz);
}

double DoublePoint3::distance(const DoublePoint3& sub)
{
        return distance(*this,sub);
}

DoublePoint3 DoublePoint3::spherical(double r,double a,double e)
{
    a = (a*M_PI)/180.0;
    e = (e*M_PI)/180.0;
    return DoublePoint3( r * cos(e) * sin(a), r * cos(e) * cos(a), r * sin(e));
}


DoublePoint3 DoublePoint3::cross(DoublePoint3  a, DoublePoint3  b)
{
    return DoublePoint3(a.y*b.z - a.z*b.y, a.z*b.x - a.x*b.z, a.x*b.y - a.y*b.x);
}

const DoublePoint3& DoublePoint3::operator+=(const DoublePoint3& other)
{
    x += other.x;
    y += other.y;
    z += other.z;
    return *this;
}
