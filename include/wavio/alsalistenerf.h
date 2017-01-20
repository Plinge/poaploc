#pragma once
#ifndef WINVER
#include <alsa/asoundlib.h>
#include "listenerthreadf.h"

class AlsaListenerF: public ListenerThreadF
{
  Q_OBJECT
public:
  AlsaListenerF();
  virtual ~AlsaListenerF();
  virtual bool setup(QString deviceName, uint ch,uint fs,uint bufsz);
  typedef enum {
      Running,
      StopRequested,      
      StopDone,
      DeviceClosed
  } State;
  virtual QString getErrorMessage();
  virtual bool isRealTime();
  virtual unsigned int getFrameCount();
protected:
  State state;
public slots:
  virtual void stop();
protected:
  virtual void run();
  virtual bool setupDevice();
  virtual void closeDevice();
  virtual void closeDo();
  bool continuous;
  ssize_t pcmRead(unsigned char*data, ssize_t rcount);
  int xrun(void);
  int suspend(void);
  int setParams(void);
private:
  int32_t* ibuffer[2];
  snd_pcm_t *deviceHandle;
  snd_output_t *logHandle;
  snd_pcm_uframes_t actualFrameCount;
  unsigned int bytesFrame;
  QString pcm;
  QString errorMessage;
  int readCount;
};

#endif
