//#include "stdafx.h"
#include "math.h"
#include <malloc.h>
#include <stdexcept>
#include "delayfilter.h"

DelayFilter::DelayFilter()
{
	iLength=0;
	mempos=0;
	memory=0;
}

DelayFilter::DelayFilter(int l)
{
    iLength=0;
    mempos=0;
    memory=0;
    SetLength(l);
}

DelayFilter::~DelayFilter()
{
    if (memory) {
		free(memory);
	}
	memory=0;
}


void DelayFilter::SetLength(int l)
{
	if (iLength!=l) {
		if (memory) {
			free(memory);
			memory = 0;
		}
		if (l>0) {
          memory = (double*)malloc(sizeof(double)*l);
          if (memory==0) {
              throw std::bad_alloc();
          }
        }
		iLength=l;	
		Reset();
	}
}

/*
void DelayFilter::ChangeLength(int l)
{
    if (iLength==l) return;
    
    if (memory==0) {
        SetLength(l);
        return;
    }         
    double* newmemory = (double*)malloc(sizeof(double)*l);
    if (newmemory==0) {
        throw new std::bad_alloc();
    }
    newmemory[0] = GetHistory(0);
    for (int i=1;i<std::min<int>(iLength,l);i++) {
        newmemory[iLength-1-i]=GetHistory(i);
    }
    mempos=0;
    free(memory);
    memory=newmemory;
    iLength=l;	        
}
*/
int DelayFilter::GetLength()
{
	return iLength;
}

double DelayFilter::Calc(double v)
{
	if (iLength<=0) return v;    
	double r = memory[mempos];
	memory[mempos] = v;	
	mempos++;
	if (mempos>=iLength) mempos=0;	
	return r;
}

void DelayFilter::Reset()
{
    if (memory!=0) {
        for (int i=0;i<iLength;i++){
            memory[i]=0.0;
        }
    }
	mempos=0;
}

double DelayFilter::GetDelay()
{
	return GetLength(); 

}

double DelayFilter::GetHistory( int index )
{
    if (iLength<1) {        
        return 0.0;
    }
    int pos = mempos -1 - index;
    if (pos>=iLength) pos-=iLength;
    if (pos<0) pos+=iLength;
    return memory[pos];
}
