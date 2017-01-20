#ifndef FILELISTENER_H_
#define FILELISTENER_H_

#include "wave/wavefile.h"
#include "listenerthreadf.h"

class FileListenerF: public ListenerThreadF
{
   Q_OBJECT
protected:
	bool continuous;
	unsigned int position;
    int bytespersample;
    Audio::WaveFile wav;
public:
    FileListenerF();
    virtual ~FileListenerF();
protected:
    virtual void run();
	virtual bool setup(QString name,unsigned int ch,unsigned int fs,unsigned int sz);
    virtual bool setupDevice();
	virtual void closeDevice();
public slots:
    virtual void stop();
};

#endif

