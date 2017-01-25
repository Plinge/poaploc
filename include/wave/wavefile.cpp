/*
 * Copyright 2010-2015 (c) Axel Plinge / TU Dortmund University
 *
 * ALL THE COMPUTER PROGRAMS AND SOFTWARE ARE PROVIDED "AS IS" WITHOUT WARRANTY OF ANY KIND.
 * WE MAKE NO WARRANTIES,  EXPRESS OR IMPLIED, THAT THEY ARE FREE OF ERROR, OR ARE CONSISTENT
 * WITH ANY PARTICULAR STANDARD OF MERCHANTABILITY, OR THAT THEY WILL MEET YOUR REQUIREMENTS
 * FOR ANY PARTICULAR APPLICATION. THEY SHOULD NOT BE RELIED ON FOR SOLVING A PROBLEM WHOSE
 * INCORRECT SOLUTION COULD RESULT IN INJURY TO A PERSON OR LOSS OF PROPERTY. IF YOU DO USE
 * THEM IN SUCH A MANNER, IT IS AT YOUR OWN RISK. THE AUTHOR AND PUBLISHER DISCLAIM ALL
 * LIABILITY FOR DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES RESULTING FROM YOUR USE OF THE
 * PROGRAMS.
 */

#include "wavefile.h"
#include "floatwave.h"
// #include <malloc.h>
#include <QFile>
#include <QDebug>

using namespace Audio;

WaveFile::WaveFile() : dataLengthInBytes(0), data(0)
{
    waveHeader.bitsPerSample=16;
    waveHeader.bytesPerFrame=2;    
    waveHeader.samplingFrequency=16000;
    waveHeader.bytesPerSecond=waveHeader.samplingFrequency* waveHeader.bytesPerSecond;
    waveHeader.format = format = WAVE_FORMAT_PCM;
    setupHeaders();
}

void WaveFile::copy( const WaveFile& other )
{
    throw std::runtime_error("WaveFile Copy not implemented!");
}

WaveFile::WaveFile( const WaveFile& other ) : data(0), dataLengthInBytes(0)
{
    copy(other);
}

const WaveFile& WaveFile::operator=( const WaveFile& other )
{
    copy(other);
    return *this;
}

WaveFile::~WaveFile()
{
    clear();
}

void WaveFile::setupHeaders()
{
    riffHeader.rstr[0]='R';
    riffHeader.rstr[1]='I';
    riffHeader.rstr[2]='F';
    riffHeader.rstr[3]='F';
    riffHeader.length=sizeof(waveHeader)+sizeof(dataHeader)+dataLengthInBytes;

    waveHeader.wstr[0]='W';
    waveHeader.wstr[1]='A';
    waveHeader.wstr[2]='V';
    waveHeader.wstr[3]='E';
    waveHeader.fstr[0]='f';
    waveHeader.fstr[1]='m';
    waveHeader.fstr[2]='t';
    waveHeader.fstr[3]=' ';
    waveHeader.subChunkSize = 16;
    if (format == WAVE_FORMAT_PCM) {
        waveHeader.format = WAVE_FORMAT_PCM;

    } else if (format==WAVE_FORMAT_IEEE_FLOAT) {
        waveHeader.format = WAVE_FORMAT_IEEE_FLOAT;
//        waveHeader.format=WAVE_FORMAT_EXTENSIBLE;
//        riffHeader.length += sizeof(extHeader);
//        waveHeader.subChunkSize += sizeof(extHeader);
//        extHeader.dwChannelMask=(1  << waveHeader.channels)-1;
//        extHeader.Samples.wValidBitsPerSample=waveHeader.bitsPerSample;
//        extHeader.Samples.wSamplesPerBlock = 0;
//        memcpy(&extHeader.SubFormat,&KSDATAFORMAT_SUBTYPE_IEEE_FLOAT,sizeof(WavGUID));
    } else {
        throw WaveException("unknown format!");
    }
    waveHeader.bytesPerSecond = waveHeader.samplingFrequency * waveHeader.bytesPerFrame;


    dataHeader.name[0]='d';
    dataHeader.name[1]='a';
    dataHeader.name[2]='t';
    dataHeader.name[3]='a';
    dataHeader.length=dataLengthInBytes;
}

void WaveFile::clearCues()
{
    cues.clear();
}

QList<WaveFile::Cue> WaveFile::getCues() const
{
    return cues;
}

void WaveFile::setCues(const QList<WaveFile::Cue> & other)
{
    cues = other;
}

WaveFile::Cue & WaveFile::getCueByName( unsigned long name )
{
    for (int i = 0; i < cues.size (); i++) {
        if (cues[i].name==name) {
            return cues[i];
        }
    }

    throw std::runtime_error ("No such cue name");
}

bool WaveFile::readListInfo(QFile & file, unsigned long bytesLeft)
{
    while(bytesLeft>0) {
        WavChunkEntry chunk;
         unsigned long pos = file.pos();
        int bytesRead =  file.read(reinterpret_cast<char *>(&chunk),sizeof(chunk));
        if (bytesRead < (int) sizeof(chunk)) {
            return false;
        }
        bytesLeft-=bytesRead;
        QString chunkString = chunk.getString();
        if (chunkString == "adtl") {
            file.seek(pos);
            return true;
        }
        if (chunkString.compare("ICMT")==0) {
            char* buffer = reinterpret_cast<char*>( malloc(chunk.length+1) );
            buffer[chunk.length]=0;
            bytesRead = file.read(buffer,chunk.length);
            bytesLeft-=bytesRead;
            comment += QString::fromUtf8(buffer);
            free(buffer);
        }
    }
    return true;
}


bool WaveFile::readListAdtl(QFile & file, unsigned long bytesLeft)
{
    while (bytesLeft>0) {
        WavChunkEntry chunk;
        unsigned long pos = file.pos();
        int bytesRead =  file.read(reinterpret_cast<char *>(&chunk),sizeof(chunk));
        if (bytesRead < (int) sizeof(chunk)) {
            return false;
        }
        bytesLeft-=bytesRead;
        QString chunkString = chunk.getString();
        if (chunkString == "INFO") {
            file.seek(pos);
            return true;
        }
        if (chunkString.compare("labl")==0 || chunkString.compare("note")==0 || chunkString.compare("ltxt")==0) {
            unsigned long name;
            if (chunkString.compare("ltxt")==0) {
                CueLtxt cueLtxt;
                if (bytesLeft<sizeof(cueLtxt)) {
                    throw std::runtime_error("WaveFile.readListAdtl: Error in ltxt chunk!");
                }
                bytesRead =  file.read(reinterpret_cast<char *>(&cueLtxt),sizeof(cueLtxt));
                name = cueLtxt.dwName;
            } else {
                if (bytesLeft<sizeof(name)) {
                    throw std::runtime_error("WaveFile.readListAdtl: Error in labl/note chunk!");
                }
                bytesRead =  file.read(reinterpret_cast<char *>(&name),sizeof(name));
            }
            bytesLeft-=bytesRead;
            QString text;
            char* buffer = new char[chunk.length];
            if (bytesLeft<chunk.length-4) {
                throw std::runtime_error("WaveFile.readListAdtl: Error in chunk!");
            }
            bytesRead = file.read(buffer, chunk.length-4); bytesLeft-=bytesRead;
            buffer[chunk.length-3]=0;
            text = QString::fromLocal8Bit(buffer);
            // handle word alignment
            if (chunk.length&1) {
                bytesRead =  file.read(buffer,1); bytesLeft-=bytesRead;
            }
            delete [] buffer;

            Cue & cue = getCueByName(name);
            if (chunkString.compare("labl")==0) {
                cue.label = text;
            }
            if (chunkString.compare("note")==0) {
                cue.note = text;
            }
            if (chunkString.compare("ltxt")==0) {
                cue.text = text;
            }
        }
    }
    return true;
}


bool WaveFile::readList(QFile & file, unsigned long bytesLeft)
{
    while (bytesLeft>0) {
        WavChunkEntry chunk;
        unsigned long pos = file.pos();
        int bytesRead =  file.read(reinterpret_cast<char *>(&chunk),sizeof(chunk));
        if (bytesRead < (int) sizeof(chunk)) {
            return false;
        }
        bytesLeft-=bytesRead;
        QString chunkString = chunk.getString();
        if (chunkString.compare("INFO")==0) {
            file.seek(pos+4);
            bytesLeft+=4;
            if (!readListInfo(file,bytesLeft)) {
                return false;
            }
        }
        if (chunkString.compare("adtl")==0) {
            file.seek(pos+4);
            bytesLeft+=4;
            if (!readListAdtl(file,bytesLeft)) {
                return false;
            }
        }
    }
    return true;
}


bool WaveFile::readCue( QFile & file, unsigned long bytesLeft )
{
    unsigned long cueCount;
    int bytesRead =  file.read(reinterpret_cast<char *>(&cueCount),sizeof(cueCount));
    bytesLeft-=bytesRead;
    for (int cueIndex=0; cueIndex<cueCount; cueIndex++) {
       CuePt cuePt;
       if (bytesLeft<sizeof(cuePt)) {
           throw std::runtime_error("Error in cue chunk!");
       }
       bytesRead =  file.read(reinterpret_cast<char *>(&cuePt),sizeof(cuePt));
       bytesLeft-=bytesRead;
       Cue cue;
       cue.name = cuePt.dwName;
       cue.position = cuePt.dwPosition;
       cues.append(cue);
    }
    if (bytesLeft!=0) {
        throw std::runtime_error("Lazy formatted cue chunk!");
    }
    return true;
}

bool WaveFile::read( QString pathName )
{
    clear();
    QFile file(pathName);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "WaveFile.read(): Failed to open file '"  << pathName << "' !" ;
        return false;
    }
    //unsigned long fLength  = mFile.GetLength();
    file.read(reinterpret_cast<char *>(&riffHeader),sizeof(riffHeader));

    if ((riffHeader.rstr[0]!='R') || (riffHeader.rstr[1]!='I') ||	(riffHeader.rstr[2]!='F') || (riffHeader.rstr[3]!='F')) {
        qDebug() << "WaveFile.read(): Error in header!"   << pathName << "' !" ;
        file.close();
        return false;
    }

    file.read(reinterpret_cast<char *>(&waveHeader),sizeof(waveHeader));
    if ((waveHeader.wstr[0]!='W') || (waveHeader.wstr[1]!='A') || (waveHeader.wstr[2]!='V') || (waveHeader.wstr[3]!='E')) {
        qDebug() << "WaveFile.read(): Error in header!"   << pathName << "' !" ;
        file.close();
        return false;
    }
    if ((waveHeader.fstr[0]!='f') || (waveHeader.fstr[1]!='m') || (waveHeader.fstr[2]!='t') || (waveHeader.fstr[3]!=' ')) {
        qDebug() << "WaveFile.read(): Error in header!"   << pathName << "' !" ;
        file.close();
        return false;
    }

    unsigned long pos = file.pos();

    format = waveHeader.format;
    if (format == WAVE_FORMAT_EXTENSIBLE) {        
        file.read(reinterpret_cast<char *>(&extHeader),sizeof(extHeader));
        if (extHeader.SubFormat.l[0] == KSDATAFORMAT_SUBTYPE_PCM.l[0]) {
            format = WAVE_FORMAT_PCM;
        }
        if (extHeader.SubFormat.l[0] == KSDATAFORMAT_SUBTYPE_IEEE_FLOAT.l[0]) {
            format = WAVE_FORMAT_IEEE_FLOAT;
        }
    }

    if (format != WAVE_FORMAT_PCM && format != WAVE_FORMAT_IEEE_FLOAT) {
        qDebug() << "WaveFile.read(): Unknown format!";
        file.close();
        return false;
    }
    // NTH: WAVE_FORMAT_EXTENSIBLE
    int extraBytesInWaveHeader = waveHeader.subChunkSize-16;
    if (extraBytesInWaveHeader>0) {

        file.seek(pos+extraBytesInWaveHeader);
    }

    WavChunkEntry chunk;
    file.read(reinterpret_cast<char *>(&chunk),sizeof(chunk));

    unsigned long fileLength = file.size();

    do {
        unsigned long pos = file.pos();
        QString chunkString = chunk.getString();
        // qDebug() << "WaveFile.read() " << chunkString << " chunk  at " << pos - sizeof(chunk);
        if (chunkString.compare("data")==0) {
            memcpy(&dataHeader,&chunk,sizeof(WavChunkEntry));
            dataLengthInBytes = dataHeader.length;
            data = reinterpret_cast<unsigned char *>(malloc(dataLengthInBytes));
            if (data==0) {
                throw std::bad_alloc();
            }
            qint64 bytesRead = file.read(reinterpret_cast<char *>(data), dataLengthInBytes);
			if (bytesRead<=0) {
				qDebug() << "WaveFile.read(): Read error!";
				return false;
			}
			if (bytesRead < dataLengthInBytes) {
				qDebug() << "WaveFile.read(): Read error!";
				return false;
			}
        }
        if (chunkString.compare("LIST")==0) {
            if (!readList(file,chunk.length)) {
               break;
            }
        }
        if (chunkString.startsWith("cue")) {
            if (!readCue(file,chunk.length)) {
                break;
            }
        }
        if (pos+chunk.length>=fileLength) {
            break;
        }
        file.seek(pos+chunk.length);
        if (file.read(reinterpret_cast<char *>(&chunk),sizeof(chunk))<sizeof(chunk)) {
            break;
        }
    } while (1);

    if (dataLengthInBytes==0) {
        qDebug() << "WaveFile.read(): Error in file, no DATA Chunk found!";
        file.close();
        return false;
    }

    file.close();
    return true;
}

bool WaveFile::write( QString pathName )
{
    QFile file(pathName);
    if (!file.open(QIODevice::WriteOnly|QIODevice::Truncate)) {
        qDebug() << "WaveFile.write(): Failed to open file '"  << pathName << "' !" ;
        return false;
    }

    setupHeaders();
    // calculate size of comment
    QByteArray ba = comment.toUtf8();
    int commentLength = ba.size();
    int commentPadding = 0;
    if (commentLength>0) {
        commentPadding=1;
        if ((commentPadding+commentLength)&1) commentPadding++;
        riffHeader.length+=commentLength+commentPadding;
    }
    // calculate size of cue data
    int nCues = getCueCount();
    unsigned long szLabels=0;
    unsigned long szNotes=0;
    unsigned long szAdtl=0;
    unsigned long szCues=0;
    if (nCues>0) {
        foreach (const Cue & p, cues) {
            int nLen = (p.label.size())+1;
            if (nLen&1) nLen++;
            szLabels+=nLen;
            szLabels+=12;

            nLen = (p.note.size())+1;
            if (nLen&1) nLen++;
            szNotes+=nLen;
            szNotes+=12;
        }
        szCues=(sizeof(CuePt)*nCues)+8;
        szAdtl=4;
    }
    riffHeader.length+=szCues+szAdtl+szLabels+szNotes;

    if (commentLength>0 ||  nCues>0) {
        riffHeader.length+=sizeof(WavChunkEntry); // LIST
    }
    if (commentLength>0) {
        riffHeader.length+=sizeof(WavChunkEntry)+4; // INFO ICMT
    }

    file.write(reinterpret_cast<char *>(&riffHeader),sizeof(riffHeader));
    file.write(reinterpret_cast<char *>(&waveHeader),sizeof(waveHeader));
    if (waveHeader.format==WAVE_FORMAT_EXTENSIBLE) {
        file.write(reinterpret_cast<char *>(&extHeader),sizeof(extHeader));
    }
    file.write(reinterpret_cast<char *>(&dataHeader),sizeof(dataHeader));
    file.write(reinterpret_cast<char *>(data),dataLengthInBytes);

    if (nCues>0) {
        WavChunkEntry cueChunk;
        cueChunk.setString("cue ");
        cueChunk.length = nCues*sizeof(CuePt)+4;
        file.write(reinterpret_cast<char *>(&cueChunk),sizeof(cueChunk));
        file.write(reinterpret_cast<char *>(&nCues),sizeof(nCues));
        foreach(const Cue & cue, cues) {
            CuePt cuePt;
            cuePt.dwName = cue.name;
            cuePt.dwPosition = cue.position;
            file.write(reinterpret_cast<char *>(&cuePt),sizeof(CuePt));
        }
    }
    WavChunkEntry chunk;
    char aZero=0;
    if (commentLength>0 ||  nCues>0) {
        chunk.setString("LIST");
        chunk.length=0;
        if (commentLength>0) {
            chunk.length += commentLength + commentPadding + sizeof(WavChunkEntry) + 4;
        }
        if (szAdtl) {
           chunk.length += szAdtl + szLabels + szNotes;
        }
        file.write(reinterpret_cast<char *>(&chunk), sizeof(chunk));

        if (commentLength>0) {
            chunk.setString("INFO");
            chunk.length = 0; // commentLength + sizeof(WavChunkEntry);
            file.write(reinterpret_cast<char *>(&chunk), 4);
            chunk.setString("ICMT");
            chunk.length = commentLength + commentPadding;
            file.write(reinterpret_cast<char *>(&chunk), sizeof(chunk));
            file.write(ba);
            for (int i=0;i<commentPadding;i++) {
                file.write(&aZero,1);
            }
        }

        if (szAdtl) {
            chunk.setString("adtl");
            file.write(reinterpret_cast<char *>(chunk.name), sizeof(chunk.name));
            foreach(const Cue & cue, cues) {
                int len = cue.label.size()+1;
                chunk.setString("labl");
                chunk.length = len+4;
                file.write(reinterpret_cast<char *>(&chunk), sizeof(chunk));
                file.write(reinterpret_cast<const char *>(&cue.name), sizeof(cue.name));
                file.write(static_cast<const char *>(cue.label.toLocal8Bit()), len-1);
                file.write(&aZero,1);
                if (len&1) {
                    file.write(&aZero,1);
                }
                len = cue.note.size()+1;
                chunk.setString("note");
                chunk.length = len+4;
                file.write(reinterpret_cast<char *>(&chunk), sizeof(chunk));
                file.write(reinterpret_cast<const char *>(&cue.name), sizeof(cue.name));
                file.write(static_cast<const char *>(cue.note.toLocal8Bit()), len-1);
                file.write(&aZero,1);
                if (len&1) {
                    file.write(&aZero,1);
                }
            }
        }
       
    }
    file.close();
    return true;
}

void WaveFile::create(double samplingFrequency, int channelCount, int sampleCount,bool dirty)
{    
    clear();
    int precision=32;
    waveHeader.bitsPerSample = precision;
    waveHeader.bytesPerFrame = ((precision+7)>>3) * channelCount; // SampleRate * NumChannels * BitsPerSample/8
    waveHeader.bytesPerSecond = samplingFrequency * waveHeader.bytesPerFrame;  
    waveHeader.channels = channelCount;
    waveHeader.samplingFrequency = samplingFrequency;
    format = waveHeader.format = WAVE_FORMAT_PCM;
    dataLengthInBytes = sampleCount *  waveHeader.bytesPerFrame;
    data = reinterpret_cast<unsigned char *>(malloc(dataLengthInBytes));
    if (data==0) {
        throw std::bad_alloc();
    }
    if (!dirty) {
        memset(data, 0, dataLengthInBytes);
    }
    setupHeaders();
}

void WaveFile::createFromFloatWave(const FloatWave& floatwave)
{
    waveHeader.bitsPerSample = 32;
    waveHeader.bytesPerFrame = 4 * floatwave.getChannels();
    waveHeader.bytesPerSecond = waveHeader.samplingFrequency * waveHeader.bytesPerFrame;
    waveHeader.channels = floatwave.getChannels();
    waveHeader.samplingFrequency = floatwave.getSamplingFrequency();
    dataLengthInBytes = floatwave.getFrameCount() *  waveHeader.bytesPerFrame;
    data = reinterpret_cast<unsigned char *>(malloc(dataLengthInBytes));
    memcpy(data,floatwave.getConstPtr(),dataLengthInBytes);
    format = WAVE_FORMAT_IEEE_FLOAT;
    setupHeaders();
}

void WaveFile::createFromWave(const Wave & wave) 
{
	if (wave.getTypeNumber()==WAVE_TYPE_FLOATWAVE) {
        const FloatWave* fl = (FloatWave*)&wave;
        createFromFloatWave(*fl);
        return;
    }
    waveHeader.bitsPerSample = 32;
    waveHeader.bytesPerFrame = 4 * wave.getChannels();
    waveHeader.bytesPerSecond = waveHeader.samplingFrequency * waveHeader.bytesPerFrame;
    waveHeader.channels = wave.getChannels();
    waveHeader.samplingFrequency = wave.getSamplingFrequency();
    dataLengthInBytes = wave.getFrameCount() *  waveHeader.bytesPerFrame;
    data = reinterpret_cast<unsigned char *>(malloc(dataLengthInBytes));
    format = WAVE_FORMAT_IEEE_FLOAT;
    setupHeaders();
    wave.copySamplesTo(this);
}
WaveFile::WaveFile(const FloatWave & floatwave)
{
    createFromFloatWave(floatwave);
}

WaveFile::WaveFile(const Wave & wave)
{
    createFromWave(wave);
}

void WaveFile::clear()
{
    if (data!=0) {
        free(data);
        dataLengthInBytes = 0;
    }
    comment.clear();
    clearCues();
}


