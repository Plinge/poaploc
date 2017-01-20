#include "filelistenerf.h"

// created by CppFunctionExtractor, 29.07.2010, 10:40

FileListenerF::FileListenerF(): position(0)
{
}

FileListenerF::~FileListenerF()
{
}

void FileListenerF::run()
{
	abortFlag=false;
    int maxPosition = wav.getFrameCount()*channels;
    int maxFrame = wav.getFrameCount();
	continuous=false;    
    int frame=0;
	while (position<maxPosition && !abortFlag) {
		//LogFmtA("FileListener.run reading form %D %.3fs", position, wav.sampleToSec(position/channels) );
        int otherIndex = bufferIndex^1;

        float *po= buffer[otherIndex];
        int xx = bufferSize/channels;
        if (frame+xx>=maxFrame) {
            xx = maxFrame - frame;
        }
        for (int i=0;i<xx;i++) {
            for (int c=0;c<channels;c++) {
                *po++ =  wav.getSample(frame+i,c);
            }
        }

        if (tryLock(60*1000)) {
            bufferIndex = otherIndex;
            //LogFmtA("FileListener.run got data, buffer %d (0x%08X) %D samples %D bytes", bufferIndex, buffer[bufferIndex], bufferSize , dx);
            emit(gotData(buffer[bufferIndex], continuous));
            continuous = true;
        } else {
            //LogFmtA("W_ FileListener.run got data, but locked, skipping bytes!");
            continuous = false;
        }
        position += frameCount*channels;
        frame +=  frameCount;
	}
    //LogMsgA("FileListener.run done");
	closeDevice();    
}

bool FileListenerF::setup(QString name,unsigned int ch,unsigned int fs,unsigned int sz)
{
    if (!wav.read(name)) {
        qWarning("Failed to read %s!", (const char*)name.toLocal8Bit());
        return false;
    }
    if (fs>0) {
        if (fabs(fs - wav.getSamplingFrequency())>2.0) {
            qWarning( "Sampling rate mismatch!");
            return false;
        }
    }
    if (ch>0 && channels!=ch) {
        qWarning( "Channel count mismatch!");
        return false;
    }
    channels = wav.getChannels();
    return ListenerThreadF::setup(name,channels,fs,sz);
}

bool FileListenerF::setupDevice()
{
	position=0;
	continuous=false;
	return true;
}

void FileListenerF::closeDevice()
{
    wav.clear();
	emit(closed());
}

void FileListenerF::stop()
{
	abortFlag=true;
}
