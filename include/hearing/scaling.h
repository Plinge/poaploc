#ifndef SCALING_H
#define SCALING_H
#pragma once

class Scaling
{
public:
    virtual double scale(double x) 
    {
        return x;
    }
    virtual double unscale(double y) 
    {
        return y; 
    }
};

#endif
