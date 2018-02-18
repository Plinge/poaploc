#pragma once
#ifndef __WAVESTRUCT_H_INCLUDED
#define __WAVESTRUCT_H_INCLUDED
#include <QString>
#include <stdint.h>
   
/**
 * @ingroup GacUi
 * @brief RIFF header.
 * @author ap
 * @par history
 *   - 2005-06-16 ap commented
 */
typedef struct WavRiffEntry {
 char           rstr[4]; // "RIFF"
 uint32_t  length;  // filesize-sizeof(WavRiffEntry)
} hRIFF;

/**
 * @ingroup GacUi
 * @brief wave entry in WAV file.
 * @author ap
 * @par history
 *   - 2005-06-16 ap commented
 */
typedef struct  WavWaveEntry {
 char           wstr[4]; // "WAVE"
 char           fstr[4]; // "fmt "
 uint32_t  subChunkSize;
 uint16_t format;
 uint16_t channels;
 uint32_t samplingFrequency;
 uint32_t bytesPerSecond;
 uint16_t bytesPerFrame; // = Channels * Bit/8
 uint16_t bitsPerSample;
} hWAVE;

#ifndef WAVE_FORMAT_PCM
#define	WAVE_FORMAT_PCM 	    0x0001
#endif
#define	WAVE_FORMAT_IEEE_FLOAT 	0x0003
#define WAVE_FORMAT_ALAW 	    0x0006
#define WAVE_FORMAT_MULAW 	    0x0007
#define WAVE_FORMAT_EXTENSIBLE 	0xFFFE

typedef struct WavGUID {
    uint32_t l[1];
    uint16_t w[2];
    uint8_t c[8];
} xWaveGUID;

typedef struct WavExEntry {
  union {
  uint16_t wValidBitsPerSample;
  uint16_t wSamplesPerBlock;
  uint16_t wReserved;
  } Samples;
  uint32_t  dwChannelMask;
  WavGUID  SubFormat;
} xWAVE;


//DEFINE_GUIDSTRUCT("00000001-0000-0010-8000-00aa00389b71", KSDATAFORMAT_SUBTYPE_PCM);
//DEFINE_GUIDSTRUCT("00000003-0000-0010-8000-00aa00389b71", KSDATAFORMAT_SUBTYPE_IEEE_FLOAT);
static const WavGUID KSDATAFORMAT_SUBTYPE_PCM        = { 0x00000001, 0x0000,0x0010, 0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71 };
static const WavGUID KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = { 0x00000003, 0x0000,0x0010, 0x80,0x00,0x00,0xaa,0x00,0x38,0x9b,0x71 };


/**
 * @brief chunk entry in WAV file
 * @author ap
 * @par history
 *   - 2005-06-16 ap commented
 */
class WavChunkEntry {
public:
     char           name[4];
     uint32_t  length;
public:
    QString getString() {
        char chunkName[5];
        chunkName[4]=0;    
        memcpy(chunkName, name, 4);
        return QString(chunkName);
    }

    void setString(QString str) {        
        memcpy(name, static_cast<const char*>(str.toLocal8Bit()), std::min<int>(4,str.size()));
    }
};

/**
 * @brief cue point desc.
 */
class CuePt { 
public:
    uint32_t dwName;
    uint32_t dwPosition;
    uint32_t fccChunk;
    uint32_t dwChunckStart;
    uint32_t dwBlockStart;
    uint32_t dwSampleOffset;
public:
    CuePt() : dwName(0), dwPosition(0), fccChunk(0), dwChunckStart(0), dwBlockStart(0), dwSampleOffset(0)
    {
        // whatever
    }
}; 


/**
 * @brief	cue text desc.
 * @author ap
 */
typedef struct CueLtxt { 
    uint32_t dwName;
    uint32_t dwSampleLength;
    uint32_t dwPurpose;
    uint16_t wCountry;
    uint16_t wLanguage;
    uint16_t wDialect;
    uint16_t wCodePage;
} cue_pl; 


#endif // __WAVESTRUCT_H_INCLUDED
