#pragma once
#include <QtCore>
#include <QThread>

class ListenerThreadF : public QThread
{
Q_OBJECT
public:
    ListenerThreadF();
    virtual ~ListenerThreadF();
    virtual bool setup(QString deviceName, unsigned int ch,unsigned int fs,unsigned int sz);
    virtual unsigned int getChannels();
    virtual unsigned int getFrameCount();
    virtual bool isRealTime() { return false; }
    virtual bool stopped() { return abortFlag; }

protected:
    float* buffer[2];
    bool lk;
    bool abortFlag;

public slots:
    virtual void stop() = 0;

    /**
      * called externally when buffer is processed
      */
    virtual void next()
    {
        lk=false;
    }

    virtual void close()
    {
        closeDevice();
    }

signals:
    void gotData(float*,bool);
    void gotError(QString);
    void closed();

protected:
    virtual void run() = 0;
    virtual bool setupDevice() = 0;
    virtual void closeDevice() = 0;
    virtual void freeBuffers();
    unsigned int frameCount; //< Anzahl Frames im Buffer
    unsigned int bufferSize; //< Anzahl Bytes im Buffer
    unsigned int bufferIndex; //< aktiver Buffer
    unsigned int channels; //< Anzahl Kanaele auf denen gelauscht wird		
    unsigned int samplingRate; //< Abtastrate

    /**
      * called internally when updating buffer
      */
    inline bool tryLock(long to)
    {
            while (lk && to>0 && !abortFlag)	{
                    QThread::msleep(2);
                    to--;
            }
            if (lk) return false;
            lk=true;
            return true;
    }

};

