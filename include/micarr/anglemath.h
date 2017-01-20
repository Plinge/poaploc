#ifndef ANGLEMATH_H
#define ANGLEMATH_H
#pragma once

#include <math.h>

static inline double diff360(double a, double b)
{
    double d = a-b;
    if (d> 180.0) d-=360.0;
    if (d<-180.0) d+=360.0;
    return d;
}

static inline double make360(double d)
{
    if (d> 180.0) d-=360.0;
    if (d<-180.0) d+=360.0;
    return d;
}


static inline double absDiff360(double a,double b)
{
    double d = fabs(a-b);
    // only once round the circle
    while (d > 360.0) {
        d=d-360.0;
    }
    // choose the shorter way round the circle
    if (d >  180.0) {
        d = 360.0-d;
    }
    return d;
}



#endif // ANGLEMATH_H
