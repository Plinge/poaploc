/*
 * AlsaListener.cpp
 *
 *  Created on: Jul 13, 2009
 *      @author axel plinge, marius hennecke
 */
#ifndef WIN32
#include "alsalistenerf.h"
#include <sys/asoundlib.h>

AlsaListenerF::AlsaListenerF() :
    ListenerThreadF(),
	deviceHandle(0),
	logHandle(0),		
    state(StopDone),
    actualFrameCount(1024)
{
	
	
}

AlsaListenerF::~AlsaListenerF() {

    if (state == Running) {
        stop();
    }
    if (deviceHandle != 0) { 
        int to = 50;
        while (deviceHandle && to-->0) {
            usleep(100);
        }
        if (deviceHandle) {
            qDebug("AlsaListener.~AlsaListener closing anyway");
            closeDevice(); 
        }   
    }    
}

unsigned int AlsaListenerF::getFrameCount()
{
    return (unsigned int)frameCount;
}

void AlsaListenerF::run() {
    abortFlag=false;

	if (!setupDevice()) {
        state = StopRequested;
    } else {
        state = Running;        
    }       
    int errorCount=0;
    continuous = false;

    while (state == Running && !abortFlag) {
        ssize_t framesToRead = frameCount;
        int otherIndex = 1^bufferIndex;
        int framesRead = pcmRead((unsigned char*)ibuffer[otherIndex], framesToRead);
        if (framesRead != framesToRead) {
            qWarning("AlsaListener.run Couldn't read %d from device, got %d (after %d times).", framesToRead, framesRead, readCount);
            errorCount++;
            closeDo();
            if (errorCount>5) {
               state = StopRequested;
               qWarning("AlsaListener.run Couldn't read from device %d times in a row!",errorCount);
               errorMessage = "Too many errors reading from pcm device!";
               break;
            }
            msleep(100);
            if (!setupDevice()) {
               errorCount = 15;
               state = StopRequested;
               break;
            }
            continuous = false;
        } else {
            if (tryLock(1000)) {
                 // qDebug(" AlsaListener.run got %d frames.",framesRead);
                bufferIndex = otherIndex;
                //qDebug("AlsaListener.gotData");

                for (int i=0;i<frameCount;i++) {
                    buffer[bufferIndex][i] = (float)(ibuffer[bufferIndex][i] * (1.0F / (65536.0F*32767.0F)));
                }
                emit(gotData(buffer[bufferIndex],continuous));
                continuous = true;
            } else {
               if (abortFlag) {
                  qDebug("AlsaListener.run got data, but aborting, discarding %d frames!",framesRead);
               } else {
                  qDebug("AlsaListener.run got data, but locked, discarding %d frames!",framesRead);
               }
               continuous = false;
            }
            errorCount=0;
            readCount++;
        }
    } 
    qDebug("AlsaListener.run done (%d consecutive reads)",readCount);
    state = StopDone;
    closeDevice();    
    if (errorCount>5) {        
        emit(gotError(errorMessage));
    } 
}

bool AlsaListenerF::setupDevice() {
  int err;
  int open_mode = 0;

  qDebug("AlsaListener.setupDevice");
  
  if (!deviceHandle) {
    
    err = snd_output_stdio_attach(&logHandle, stderr, 0);
    Q_ASSERT(err >= 0);

    //open_mode |= SND_PCM_NO_AUTO_RESAMPLE;
    //open_mode |= SND_PCM_NO_AUTO_CHANNELS;
    //open_mode |= SND_PCM_NO_AUTO_FORMAT;
    open_mode |= SND_PCM_NO_SOFTVOL;

    qDebug("AlsaListener.setupDevice Opening sound device '%s'", (const char*)pcm.toLocal8Bit() );
    
    err = snd_pcm_open(&deviceHandle, qPrintable(pcm) , SND_PCM_STREAM_CAPTURE, open_mode);
    if (err < 0) {
        errorMessage = "Audio open failed: ";
        errorMessage += QString::fromLocal8Bit(snd_strerror(err));
        qDebug("audio open error: %s", (const char*)errorMessage.toLocal8Bit());
	    emit(gotError(errorMessage));
	    return false;
    }
    snd_pcm_info_t *m_info;
    snd_pcm_info_alloca(&m_info);
    if ((err = snd_pcm_info(deviceHandle, m_info)) < 0) {
            errorMessage = QString::fromLocal8Bit(snd_strerror(err));
        qDebug("info error: %s", (const char*)errorMessage.toLocal8Bit());
        emit(gotError(errorMessage));
	    return false;
    }
    /* setup sound hardware */
    if ((err = setParams()) == EXIT_FAILURE) {
        qDebug("sound hardware setup failed");
        emit(gotError(errorMessage));
        actualFrameCount=frameCount;
	    return false;
    }

  }
  errorMessage = "No error.";
  bufferIndex=0;
  return true;
}

void AlsaListenerF::closeDo()
{
  if (deviceHandle) {
        qDebug("AlsaListener.closeDevice");
        snd_pcm_close(deviceHandle);     
        snd_output_close(logHandle);
        snd_config_update_free_global();    
        deviceHandle = 0;
  }
}

void AlsaListenerF::closeDevice()
{   
    if (deviceHandle) {
        closeDo();
        state = DeviceClosed;        
        qDebug("AlsaListener.closeDevice closed");
        emit(closed());
    }
}

void AlsaListenerF::stop()
{
  // Set flag so that threads loop can terminate gracefully
    if (state == Running) {
        state = StopRequested;
        abortFlag = true;
        qDebug("AlsaListener.stopping .. ");
    }
   //wait();     
}

ssize_t AlsaListenerF::pcmRead(unsigned char* data, ssize_t n_frames) {
	ssize_t read_frames;
	size_t count;
	size_t result = 0;
	count = n_frames;
        //AP_TimerOMP timer;
        //timer.start();
	while (count > 0) {
		read_frames = snd_pcm_readi(deviceHandle, data, count);
        //qDebug("AlsaListener.pcmRead read %d frames", read_frames);
        if (read_frames == -EAGAIN || (read_frames >= 0 && read_frames < (snd_pcm_sframes_t) count)) {
			snd_pcm_wait(deviceHandle, 100);
		} else if (read_frames == -EPIPE) {			
            qDebug("AlsaListener.pcmRead: %s", snd_strerror(read_frames));
            if (xrun()==EXIT_SUCCESS) {
                continue;
            }
			return 0;
		} else if (read_frames == -ESTRPIPE) {
			suspend();
            qDebug("suspend: %s", snd_strerror(read_frames));
		} else if (read_frames < 0) {
            qDebug("read error: %s", snd_strerror(read_frames));
			return 0;
		}
		if (read_frames > 0) {

			result += read_frames;
			count -= read_frames;
            data += read_frames * bytesFrame;
            //qDebug("n_frames: %i, read_frames: %i, count: %i", n_frames, read_frames, count);
		} 
//		if (count>0) {
//			double time = timer.getElapsedSeconds();
//			if (time>1.0) {
//				qDebug("AlsaListener.pcmRead took %.2f ms, giving up!", time*1000.0);
//				return 0;
//			}
//		}
	}
	
	if (count != 0) {
       qDebug("n_frames does not match!");
	}
    //qDebug("n_frames read");
	return result;
}

/* I/O error deviceHandler */
int AlsaListenerF::xrun(void) {
    qDebug("W_ AlsaListener.xrun");
	snd_pcm_status_t *status;
	int res;
	snd_pcm_status_alloca(&status);
	if ((res = snd_pcm_status(deviceHandle, status))<0) {
        qWarning("status error: %s", snd_strerror(res));
		return EXIT_FAILURE;
	}
	if (snd_pcm_status_get_state(status) == SND_PCM_STATE_XRUN) {		
		if ((res = snd_pcm_prepare(deviceHandle))<0) {
            qWarning("xrun: prepare error: %s", snd_strerror(res));
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;		/* ok, data should be accepted again */
	}
	if (snd_pcm_status_get_state(status) == SND_PCM_STATE_DRAINING) {
		fprintf(stderr, "capture stream format change? attempting recovery...");
		if ((res = snd_pcm_prepare(deviceHandle))<0) {
            qWarning("xrun(DRAINING): prepare error: %s", snd_strerror(res));
			return EXIT_FAILURE;
		}
		return EXIT_SUCCESS;
	}

    qWarning("read/write error, state = %s", snd_pcm_state_name(snd_pcm_status_get_state(status)) );
	return EXIT_FAILURE;
}

/* I/O suspend deviceHandler */
int AlsaListenerF::suspend(void) {
	int res;

    qDebug("Suspended. Trying resume. ");
	while ((res = snd_pcm_resume(deviceHandle)) == -EAGAIN)
		sleep(1);	/* wait until suspend flag is released */
	if (res < 0) {
        qDebug("Failed. Restarting stream. ");
		if ((res = snd_pcm_prepare(deviceHandle)) < 0) {
            qWarning("suspend: prepare error: %s", snd_strerror(res));
			return EXIT_FAILURE;
		}
	}
    qDebug("Done.");
	return EXIT_SUCCESS;
}

int  AlsaListenerF::setParams(void) {

    // LogFmtF("AlsaListener.setParams");
    snd_pcm_hw_params_t *params;
    snd_pcm_sw_params_t *swparams;
    snd_pcm_uframes_t buffer_size;
    int err;

    unsigned period_time = 0;
    unsigned buffer_time = 0;
    snd_pcm_uframes_t period_frames = frameCount;
    snd_pcm_uframes_t buffer_frames = frameCount*4;
    if (buffer_frames<8192) {
        buffer_frames=8192;
    }
    errorMessage = "Error in set params.";
    snd_pcm_hw_params_alloca(&params);
    snd_pcm_sw_params_alloca(&swparams);
    err = snd_pcm_hw_params_any(deviceHandle, params);
    if (err < 0) {
      qWarning("Broken configuration for this PCM: no configurations available");
      return EXIT_FAILURE;
    }
    err = snd_pcm_hw_params_set_access(deviceHandle, params, SND_PCM_ACCESS_RW_INTERLEAVED);
    if (err < 0) {
      qWarning("Access type not available");
      return EXIT_FAILURE;
    }
    err = snd_pcm_hw_params_set_format(deviceHandle, params, SND_PCM_FORMAT_S32_LE);
    if (err < 0) {
      qWarning("Sample format not available");
      return EXIT_FAILURE;
    }
    err = snd_pcm_hw_params_set_channels(deviceHandle, params, channels);
    if (err < 0) {
      qWarning("Channel count not available");
      return EXIT_FAILURE;
    }
    err = snd_pcm_hw_params_set_rate(deviceHandle, params, samplingRate, 0);
    if (err!=0) {
      qWarning("Sample rate non available");
      return EXIT_FAILURE;
    }

    // set period to some multiple of 1024
    do {
        err = snd_pcm_hw_params_set_period_size_near(deviceHandle, params, &period_frames, 0);
        if (err==0) break;
        period_frames-=1024;
    } while (err < 0 && period_frames>1024);
//    while (err < 0 && period_frames<buffer_frames) {
//        period_frames+=1024;
//        err = snd_pcm_hw_params_set_period_size(deviceHandle, params, period_frames, 0);
//    }

    if (err < 0) {
      qWarning("Unable to set period size!");
      period_frames = frameCount;
      snd_pcm_hw_params_set_period_size_near(deviceHandle, params, &period_frames, 0);
      //return EXIT_FAILURE;
    }
    // set buffer to some multiple of period
    do {
      err = snd_pcm_hw_params_set_buffer_size_near(deviceHandle, params, &buffer_frames);
      if (err==0) break;
      buffer_frames-=period_frames;
    } while (err < 0 && buffer_frames>1024 && buffer_frames>period_frames);
    if (err < 0) {
      qWarning("Unable to set buffer size!");
      buffer_frames=frameCount*4;
      snd_pcm_hw_params_set_buffer_size_near(deviceHandle, params, &buffer_frames);
      //return EXIT_FAILURE;
    }
    err = snd_pcm_hw_params(deviceHandle, params);
    if (err < 0) {
      qWarning("Unable to install hw params:");
      snd_pcm_hw_params_dump(params, logHandle);
      return EXIT_FAILURE;
    }
    snd_pcm_hw_params_get_period_size(params, &actualFrameCount, 0);
    snd_pcm_hw_params_get_buffer_size(params, &buffer_size);
    if (actualFrameCount == buffer_size) {
      qWarning("Can't use period equal to buffer size (%D == %D)",
        actualFrameCount, buffer_size);
      return EXIT_FAILURE;
    }

    //uint m_bits_per_sample =snd_pcm_format_physical_width(SND_PCM_FORMAT_S32_LE);
    //uint m_bits_per_frame = m_bits_per_sample * channels;
    //bytesFrame = (m_bits_per_frame+7)/8;
    bytesFrame = channels * 4;
    uint m_chunk_bytes = std::max<uint>(frameCount,  actualFrameCount) * bytesFrame;
    //m_chunk_bytes = std::max<uint>((buffer_size * bytesFrame),  m_chunk_bytes);
    for (int i=0;i<2;i++) {
       ibuffer[i] = (int32_t*)malloc(m_chunk_bytes);
       if (!ibuffer[i]) {
           errorMessage = "Memory error.";
           qWarning("not enough memory");
           return EXIT_FAILURE;
      }
    }

    //qDebug("%D bits per sample, %D per frame, chunk siye is %D frames",m_bits_per_sample,m_bits_per_frame,m_chunk_bytes  );
    qDebug("AlsaListener.setParams period = %D frames, buffer = %D frames, read = %D frames, buffer size = %D bytes",
            (int)actualFrameCount, (int)buffer_size, (int)frameCount, (int)m_chunk_bytes);
    readCount=0;
    errorMessage = "No error.";
    return EXIT_SUCCESS;
}


bool AlsaListenerF::setup(QString deviceName,uint ch,uint fs,uint bufsz)
{
    pcm = deviceName;
    return ListenerThreadF::setup(deviceName,ch,fs,bufsz);
}

QString AlsaListenerF::getErrorMessage()
{
     return errorMessage;
}

bool AlsaListenerF::isRealTime()
{
     return true;
}

#endif
