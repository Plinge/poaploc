#include "listenerthreadf.h"

ListenerThreadF::ListenerThreadF() : channels(0), bufferSize(0), frameCount(0)
{
	buffer[0]=0; buffer[1]=0; lk=false;
}

ListenerThreadF::~ListenerThreadF()
{    
    freeBuffers();
}

bool ListenerThreadF::setup(QString deviceName,unsigned int ch,unsigned int fs,unsigned int sz)
{
	freeBuffers();
    channels = ch;
    frameCount = sz;
    bufferSize = ch * sz;
    if (bufferSize>0) {
        for (int i=0;i<2;i++) {
            buffer[i] = (float*)malloc(bufferSize*sizeof(float));
        }
    }
    samplingRate=fs;
    bufferIndex=0;
	return true;
}

unsigned int ListenerThreadF::getChannels()
{
    return channels;
}

unsigned int ListenerThreadF::getFrameCount()
{
     return frameCount;
}

void ListenerThreadF::freeBuffers()
{
	for (int i=0;i<2;i++) {
		if (buffer[i]!=0) {
			free(buffer[i]);
		}
		buffer[i]=0;
	}
}
