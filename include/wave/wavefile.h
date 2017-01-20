#pragma once
#ifndef __WAVEFILE_H_INCLUDED
#define __WAVEFILE_H_INCLUDED

#include "wave.h"
#include "wavestruct.h"
#include "qstring.h"
#include "qfile.h"
#include "qdebug.h"
#include "filter/clip.h"
#include <stdexcept>
#include <math.h>

namespace Audio {

class FloatWave;

/**
 * @class WaveFile
 * @brief basic wave file interface
 * @author ap
 */
class WaveFile: public Wave
{
public:
    /**
     * @class Cue
     * @author ap
     *
     * Represent a  cue point in the wave file with all its properties.
     *
     * @par history
     *   - 2011-12-08  ap  finalized
     *   - 2011-11-10  ap  started
     */
    class Cue
    {
    public:
        QString label; //< the label, used for phonemic and other transcriptions
        QString note;  //< a note
        QString text;  //< some longer text, can have different encoding (not used)
        unsigned long position; //< the sample position
        unsigned long name;     //< the name, a number that is used to identify the cue and glue together the information distributed through the file

    private:
        inline void copy(const Cue & other)
        {
            label = other.label;
            note  = other.note;
            text  = other.text;
            position = other.position;
            name     = other.name;
        }

    public:
        Cue() : position(0), name(0)
        {

        }

        Cue(int p,int n,QString l) : label(l), position(p), name(n)
        {

        }

        Cue(const Cue & other)
        {
            copy(other);
        }

        Cue& operator=(const Cue & other)
        {
            copy(other);
            return *this;
        }
    };

protected:
    WavRiffEntry riffHeader;
    WavChunkEntry dataHeader;
    WavWaveEntry waveHeader;
    WavExEntry  extHeader;
    unsigned char* data;
    unsigned long dataLengthInBytes;
    QString comment;
    QList<Cue> cues;

    unsigned short format; //< actual formmat, float on pcm

protected:

    /** get cue by its name */
    Cue & getCueByName(unsigned long name);

    /** remove all cue data */
    void clearCues();

    /** get number of cues */
    inline int getCueCount() const
    {
        return cues.size();
    }

    /** get byte position for sample */
    inline int getBytePosition(int position, int channel ) const
    {
        return  position * waveHeader.bytesPerFrame + ((channel*waveHeader.bitsPerSample)>>3);
    }

    /** check if position in range */
    inline bool bytePositionInData(int position) const
    {
        if (position < 0) return false;
        if ((unsigned long)position >= dataLengthInBytes) return false;
        return true;
    }

    /** check if position in range and throw WaveFileException if not */
    inline void testBytePosition(int position, int offset=0) const throw (std::exception)
    {
        bool fail=false;
        if (!bytePositionInData(position)) {
            qDebug() << "WaveFile: Byte position " << position << " out of data range!";
            fail=true;
        } else if (offset) {
            if (!bytePositionInData(position+offset)) {
                qDebug() << "WaveFile: Byte position " << position+offset << " out of data range!";
                fail=true;
            }
        }
        if (fail) {
            throw WaveIndexOutofRangeException("Byte position out of range (WaveFile)!");
        }
    }


    /** fill headers */
    void setupHeaders();


    /** read and parse cue chunk */
    bool readCue(QFile & file, unsigned long bytesLeft);
    /** read and parse LIST chunk */
    bool readList(QFile & file, unsigned long bytesLeft);
    /** read and parse LIST INFO chunk */
    bool readListInfo(QFile & file, unsigned long bytesLeft);
    /** read and parse LIST adtl chunk */
    bool readListAdtl(QFile & file, unsigned long bytesLeft);

public:
    WaveFile();

    WaveFile(const WaveFile& other);
    const WaveFile& operator = (const WaveFile& other);

    /** create a WAV file in memory from ... */
    WaveFile(const FloatWave& floatwave);
    WaveFile(const FloatWave* fw) {  createFromFloatWave(*fw); }
    WaveFile(const Wave& wave);
    WaveFile(const Wave* wave)  { createFromWave(*wave); }

    virtual ~WaveFile();
private:
    void copy(const WaveFile& other);
	void createFromWave(const Wave& wave);
	void createFromFloatWave(const FloatWave& floatwave);

public:
    /** read a WAV file from disk. */
    virtual bool read(QString pathName);

    /** write a WAV file to disk. */
    virtual bool write(QString pathName);

    /** create a WAV file in memory. */
    virtual void create(double samplingFrequency, int channelCount, int sampleCount,bool dirty=false);

    /** return pointer to data buffer. */
    unsigned char* ptr() {
        return data;
    }

    /** clear data */
    void clear();
    
    /** return total bytes audio data */
    unsigned long size() {
       return dataLengthInBytes;
    }

    /** get number bits per sample */
    inline int getBitsPerSample() const
    {
        return   waveHeader.bitsPerSample;
    }

    /** get number of samples in each channel */
    virtual int getFrameCount() const
    {
        return dataHeader.length / waveHeader.bytesPerFrame;
    }

    virtual void setSampleCount(long n)
    {
        throw WaveException("NYI");
    }

    virtual void zero()
    {
#if defined(__STDC_IEC_559__)
         memset(data,0,dataLengthInBytes);
#else
         if (format==WAVE_FORMAT_PCM) {
             memset(data,0,dataLengthInBytes);
         } else {
             for (long p=0;p<getFrameCount();p++) {
                 for (int c=0;c<getChannels();c++) {
                     setSample(p,c,0.0);
                 }
             }
         }
#endif
    }

    virtual int getNextNonzeroIndex(long p,int chan, int & hint,double & v) const
    {
        int n = getFrameCount();
        while (++p<n) {
            double w = getSample(p,chan);
            if (fabs(w)>0.0) {
                v=w;
                return p;
            }
        }
        return -1;
    }



    virtual double getMaxSample(int channel=-1) const
    {
        if (format == WAVE_FORMAT_IEEE_FLOAT && channel<0) {
            if (waveHeader.bitsPerSample==32) {
                float* p = reinterpret_cast<float*>(data);
                int n = getFrameCount()*getChannels();
                float m = *p; n--;
                while (n-->0) {
                    p++;
                    if (*p>m) m=*p;
                }
                return m;
            }
            if (waveHeader.bitsPerSample==64) {
                double* p = reinterpret_cast<double*>(data);
                int n = getFrameCount()*getChannels();
                double m = *p; n--;
                while (n-->0) {
                    p++;
                    if (*p>m) m=*p;
                }
                return m;
            }
        }

        if (channel<0) {
            double m = getMaxSample(0);
            for (int c=1;c<getChannels();c++) {
                double v = getMaxSample(c);
                if (v>m) m=v;
            }
            return m;
        } else {
            double m = getSample(0,channel);
            for (int i=1;i<getFrameCount();i++) {
                double v = getSample(i,channel);
                if  (v>m) m=v;
            }
            return m;
        }
    }

    /** get number of seconds that the data takes to play with the sample rate set in the file */
    inline double getTotalSeconds () const
    {
        return ((double) getFrameCount ()) / ((double) getSamplingFrequency());
    }

    /** get sample rate or sampling frequency */
    inline double getSamplingFrequency() const
    {
        return waveHeader.samplingFrequency;
    }

    /** get number of channels */
    inline int getChannels() const
    {
        return waveHeader.channels;
    }

    /** get all cue data */
    QList<Cue> getCues() const;

    /** set all cue data */
    void setCues(const QList<Cue> & other);

    virtual double getSample(long position, int channel) const throw (std::exception)
    {
        int bytePosition = getBytePosition(position, channel);
        testBytePosition(bytePosition);
        if (format == WAVE_FORMAT_PCM) {
            if (waveHeader.bitsPerSample == 16)  {
                 int16_t val = *(int16_t*)(data+bytePosition);
                 return (double)val / 32768.0;
            } else if (waveHeader.bitsPerSample == 32)  {
                int32_t val = *(int32_t*)(data+bytePosition);
                return (double)val / (32768.0*65536.0);
            }
        }
        if (format == WAVE_FORMAT_IEEE_FLOAT) {
            if (waveHeader.bitsPerSample==32) {
                return * ((float*)(data+bytePosition));
            }
            if (waveHeader.bitsPerSample==64) {
                return * ((double*)(data+bytePosition));
            }
        }
        qWarning("Sample Precision not Implemented (getSample)!");
        return 0;
    }


    virtual void setSample(long position, int channel,double val) throw (std::exception)
    {
        int bytePosition = getBytePosition(position, channel);
        testBytePosition(bytePosition);
        if (format == WAVE_FORMAT_PCM) {
            if (waveHeader.bitsPerSample == 16)  {
                 *(int16_t*)(data+bytePosition) = clip15s(val *32768.0);
                 return;
            } else if (waveHeader.bitsPerSample == 32)  {
                 *(int32_t*)(data+bytePosition) = clip31s(val*32768.0*65536.0);
                 return;
            }
        }
        if (format == WAVE_FORMAT_IEEE_FLOAT) {
            if (waveHeader.bitsPerSample==32) {
                *((float*)(data+bytePosition))=val;
                return;
            }
            if (waveHeader.bitsPerSample==64) {
                * ((double*)(data+bytePosition))=val;
                return;
            }
        }
        throw WaveException("Sample Precision not Implemented (setSample)!");
    }

    /** get a single sample */
    inline int16_t get16bitSample(int position, int channel) const throw (std::exception)
    {
          int bytePosition = getBytePosition(position, channel);
          testBytePosition(bytePosition);
          if (waveHeader.bitsPerSample == 16)  {
               return *(int16_t*)(data+bytePosition);
          } else if (waveHeader.bitsPerSample == 32)  {
                int32_t val = *(int32_t*)(data+bytePosition);
                val >>= 16;
                return val;
          } else {

              qWarning("Sample Precision not Implemented (get16bitSample)!");
              return 0;
          }
    }

    /** get a single sample */
    inline int32_t get32bitSample(int position, int channel) const throw (std::exception)
    {
        int bytePosition = getBytePosition(position, channel);
        testBytePosition(bytePosition);
        if (waveHeader.bitsPerSample == 32)  {
            return *(int32_t*)(data+bytePosition);
        } else if (waveHeader.bitsPerSample == 16)  {
            return ((int32_t)(*(int16_t*)(data+bytePosition)))<<16;
        } else {
            qWarning("Sample Precision not Implemented (get32bitSample)!");
            return 0;
        }
    }

    
};

}

#endif // __WAVEFILE_H_INCLUDED
