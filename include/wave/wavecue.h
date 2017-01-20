#pragma once
#ifndef __WAVECUE_H_INCLUDED
#define __WAVECUE_H_INCLUDED

#include <QString>


/**
 * @class WaveCue 
 * @brief a cue with label in a microsoft-pcm-wav file.
 */
class WaveCue 
{
public:
    unsigned long dwPosition;
    unsigned long dwName;
    unsigned long dwSampleLength;

    QString sLabel;
    QString sNote;
    QString sText;

public:
	WaveCue() { dwPosition=0; dwName=0;dwSampleLength=0;}
	virtual ~WaveCue() {}

	WaveCue(unsigned long dw,const char* s) 
	{
		dwPosition = dw;
		dwName=0;
		dwSampleLength=0;
		sLabel = s;
	}

public:	
	WaveCue(unsigned long dw,unsigned long dn,const char* s) 
	{
		dwPosition = dw;
		dwName = dn;
		
		dwSampleLength=0;
		sLabel =s;
	}
	
	int Compare(WaveCue* pOther)
	{
		return (pOther->dwPosition - dwPosition);
	}

	
};

#endif // __WAVECUE_H_INCLUDED
