#include "revision.h"
#include "onlinemultilocalizer.h"
#include <QtCore/QCoreApplication>
#include "qtap/qcommandlinearguments.h"
#include "localization/poaplocalizergeneric.h"
#include "wavio/listenerthreadf.h"
#ifdef HAVAE_ALSA
#include "wavio/alsalistenerf.h"
#endif
#include "wavio/filelistenerf.h"
#include <iostream>
#include <fstream>
#include <signal.h>

#include <omp.h>

using namespace std;

ListenerThreadF* input = 0;

void copyright()
{
//    cerr << "poapmloc " << COPYRIGHTSTR << " Axel Plinge, code revision " <<  REVISIONSTR << endl;
    cerr << endl;
    cerr << "This software implements the cochlear & mid-brain model localization decribed in:" << endl;
    cerr << "[1] Plinge, A., Hennecke, M. H., & Fink, G. A. (2010)." << endl;
    cerr << "    Robust Neuro-Fuzzy Speaker Localization Using a Circular Microphone Array." << endl;
    cerr << "    International Workshop on Acoustic Echo and Noise Control (IWAENC); Tel Aviv, Israel." << endl;
    cerr << endl;
//    cerr << "[2] Plinge, A., & Fink, G. A. (2013). " << endl;
//    cerr << "    Online Multi-Speaker Tracking Using Multiple Microphone Arrays Informed by Auditory Scene Analysis." << endl;
//    cerr << "    European Signal Processing Conference (EUSIPCO); Marrakesh, Morocco." << endl;
//    cerr << endl;
}

void abort(int num)
{
    if (input) {
        input->stop();
    }
}

using namespace Audio;

OlineMultiLocalizer it;

int main(int argc, char *argv[])
{    
    QCoreApplication a(argc, argv);
    
    QCommandLineArguments options(argc,argv);
    int bandCount=12;
    options.addOption('b', "bands", &bandCount);
	double loFreq=500;
    options.addOption('s', "fmin", &loFreq);
	double hiFreq=3600;
    options.addOption('e', "fmax", &hiFreq);
    double step=6;
    options.addOption('t', "frame-step",&step);
    double wlen=12;
    options.addOption('w', "frame-length",&wlen);
    double gamma=0.3;
    options.addOption(0, "gamma", &gamma);
    double relativeSpikeThreshold=6.0;
    options.addOption(0, "spike-mth", &relativeSpikeThreshold);
    double absoluteSpikeThreshold=0.0;
    options.addOption(0, "spike-ath", &absoluteSpikeThreshold);
    double spikeAverage=30.0;
    options.addOption(0, "spike-avg", &spikeAverage);
    double spikePrecedence=3.0;
    options.addOption(0, "spike-pre", &spikePrecedence);
    int spikeMode=1;
    options.addOption(0,"spike-mode",&spikeMode);
    double spikeGain = 4.0;
    options.addOption(0,"spike-gain",&spikeGain);
    double precision=3.0;
    options.addOption(0, "precision", &precision);
    double precision2=7.0;
    options.addOption(0, "elevation-precision", &precision2);
    double elevation = 30.0;
    options.addOption('z', "max-elevation", &elevation);
    double elevationMin = 0.0;
    options.addOption(0, "min-elevation", &elevationMin);
    double rotation=0.0;
    options.addOption(0, "rotation", &rotation);
    double postTime=0.5;
    options.addOption(0, "post-time", &postTime);
    double postStep=0;
    options.addOption(0, "post-step", &postStep);
    int postBands=3;
    options.addOption(0, "post-min-bands", &postBands);
    double postEnergy=-999.0;
    options.addOption(0, "post-min-energy", &postEnergy);
    double poaP=5.0;
    options.addOption(0, "post-angles-speaker", &poaP);
    double poaA=45.0;
    options.addOption(0, "post-angles-background", &poaA);
    double postTh=0.0;
    bool postPeak = false;
    options.addSwitch(0, "poap", &postPeak);
    bool argmax=false;
    options.addSwitch(0, "argmax", &argmax);
    QString em;
    //options.addOption(0, "em", &em);
    bool sumbands=false;
    options.addSwitch(0, "sum-bands", &sumbands);
    bool elmax=false;
    options.addSwitch(0, "maxelevation", &elmax);
    int skip=6;
    options.addOption(0, "skip", &skip);
    double maxTime=0;
    options.addOption(0,"time-limit",&maxTime);
    bool noBpInter = false;
    options.addSwitch(0, "integer-bp", &noBpInter);
    bool timingout = false;
    options.addSwitch(0, "timing", &timingout);
    QString file;
    options.addArgument("input", &file);
    double gain=0.0;
    options.addOption(0,"gain", &gain);
    double bandwidth=1.0;
    options.addOption(0,"bandwidth", &bandwidth);
    int gainMode=0;
    options.addOption(0,"gain-mode", &gainMode);
    double gainSmooth=0;
    options.addOption(0,"gain-smooth", &gainSmooth);
    double gainMax=50;
    options.addOption(0,"gain-max", &gainMax);
    QString micPos;
    options.addOption(0, "mic-positions", &micPos);    
    QString micGrp;
    options.addOption(0, "mic-arrays", &micGrp);
    int frameCount = 1024; // 8192; // 65536
    options.addOption(0, "frame-count", &frameCount);
    int micarrayoffset=0;
    options.addOption(0, "array-offset", &micarrayoffset);
    double outtimeoffset=0;
    options.addOption(0, "time-offset",&outtimeoffset);
    bool alias = false;       
    options.addSwitch(0,"alias", &alias);

    int samplingFrequnecy = 48000;
    options.addOption(0, "sampling-frequency", &samplingFrequnecy);
    int channels=0;
    options.addOption(0,"channels",&channels);
    //AP_MP::enable(true);
    double minLikelihood=0.005;
    options.addOption(0,"min-likelihood",&minLikelihood);

    int debug=0;
    options.addOption(0,"debug",&debug);
    bool quiet=false;
    options.addSwitch('q', "quiet", &quiet);

    bool usage=false;
    options.addSwitch('h',"help",&usage);

	if (!options.parse() || usage) {
        copyright();
        cerr << (const char*)options.help().toLocal8Bit() << endl;
#ifdef HAVAE_ALSA
        cerr << "input: use filename or alsa:devicename" << endl;
#endif
        cerr << endl;
        cerr << "PARAMETERS:" << endl;
        cerr << "--bands 16 --fmin 300 --fmax 3000 --bandwidth 1.0" << endl;
        cerr << "Use  16 gammatone filters  with ERB spaced center frequencies  between 300 and" << endl;
        cerr << "3000 Hz and a Glaberg/Moore bandwith of 1 (default)." << endl;
        cerr << endl;
        cerr << "--gain 30 --frame-count 1024" << endl;
        cerr << "Apply a fixed gain of 30 dB to the input." << endl;
        cerr << "Use 1024 frames for the FFT overlap-add processing." << endl;
        cerr << endl;
        cerr << "--spike-ath 0.0 --spike-mth 6 --spike-pre 3 --spike-avg 30" << endl;
        cerr << "Use  an absolute threshold of 0.0  and  a relative threshold of 6 dB  for spike" << endl;
        cerr << "generation. Apply a precedence shift of 3 ms and use an 30 ms average." << endl;
        cerr << endl;
        cerr << "--gain-mode 3 --gain-smooth 0.05 --gain-max 12" << endl;
        cerr << "Apply an  automatic gain  based on the energy histogram  in each band  for each" << endl;
        cerr << "input channel with a smoothing factor of 0.05  and limiting the dynamicly added" << endl;
        cerr << "gain to 12 dB." << endl;
        cerr << endl;
        cerr << "--mic-positions x1,y1,z1,x2,y2,z2,.. --mic-arrays 0-3,4-7" << endl;
        cerr << "Define microphone positions by giving x,y,z per channel. Then combine channels" << endl;
        cerr << "by specifining continous ranges, e.g. 0-3,4-7 to group 8 channels into two 4" << endl;
        cerr << "channel microphone arrays." << endl;
        cerr << endl;

        cerr << "Up to " << omp_get_max_threads() << " threads for parallel processing." << endl;
        return usage?0:12;
	}        	    

    if (debug) {
        std::cerr << (const char*)options.list().toLocal8Bit() << std::endl;
        it.setDebug(debug);
    }
    copyright();
    cerr.precision(6);

    if (!quiet) {
        cerr << "Frame count is " <<  frameCount << ", "
             << (frameCount / (step*1e-3 * samplingFrequnecy)) << " correlations each."
             << endl;
    }

    try {

    if (file.startsWith("alsa:")) {
#ifdef HAVAE_ALSA
        input = new AlsaListenerF();
        if (!input->setup( file.mid(5), channels, samplingFrequnecy, frameCount)) {
            std::cerr << "setup error" << std::endl;
            return 13;
        }
#else
        std::cerr << "No ALSA support for this system, sorry!" << std::endl;
        return 45;
#endif

    } else {
        input = new FileListenerF();
        if (!input->setup(file,0,samplingFrequnecy,frameCount)) {
            std::cerr << "setup error, file not useable" << std::endl;
            return 12;
        }
    }

    }
    catch (std::bad_alloc) {
        std::cerr <<"Out of memory!" << std::endl;
        exit(100);

    }
    catch (std::runtime_error e) {
            std::cerr << e.what() << std::endl;
            exit(101);
    }

    cout.precision(6);
    cout << fixed;  

    QList<DoublePoint3> micPositions;
    QStringList coords = micPos.split(",");
    for (int o=0;o+2<coords.count();o+=3) {
         micPositions.append(DoublePoint3(coords[o+0].toDouble(), coords[o+1].toDouble(), coords[o+2].toDouble()));
    }

    QStringList arrys = micGrp.split(",");
    for (int i=0;i<arrys.size();i++) {
        ArbitraryMicArray * amic = new ArbitraryMicArray();
        int startIndex = -1;
        QList<int> chan;
        if (arrys[i].isEmpty()) {
            for (int micIndex = 0; micIndex< micPositions.size(); micIndex++) {
                amic->add(micPositions.at(micIndex));
                chan.append(micIndex);
                if (startIndex<0) {
                    startIndex = micIndex;
                }
            }
        } else if (arrys[i].contains('-')) {
            QStringList  se = arrys[i].split("-");
            if (se.size()!=2)  {
                cerr << "Invalid array specification, expected #-#!" << endl;
                cerr << (const char*)arrys[i].toLocal8Bit() << endl;
                return 22;
            }
            startIndex = se[0].toInt();
            int endIndex = se[1].toInt();
            if (endIndex - startIndex < 1)  {
                cerr << "Invalid array specification, expected start-end!" << endl;
                cerr << (const char*)arrys[i].toLocal8Bit() << endl;
                return 23;
            }
            if (endIndex>=micPositions.size()) {
                cerr << "position " <<  endIndex << " not specified!" << endl;
                return 24;
            }
            if (input->getChannels() < endIndex) {
                cerr << "Input has less channels than specified." << endl;
                return 78;
            }

            for (int index = startIndex;index<=endIndex;index++) {
                amic->add(micPositions.at(index));
                chan.append(index);
            }
            //cerr << "array " <<  i << "  #" << startIndex << ".. @ " << amic->getMicPosString() << endl;
        } else {
            QStringList se = arrys[i].split("+");
            if (se.size()<2)  {
                cerr << "Invalid array specification, expected #+#!" << endl;
                cerr << (const char*)arrys[i].toLocal8Bit() << endl;
                return 22;
            }
            foreach (QString micstr, se) {
                int micIndex = micstr.toInt();
                amic->add(micPositions.at(micIndex));
                chan.append(micIndex);
                if (startIndex<0) {
                    startIndex = micIndex;
                }
            }
            //cerr << "array " <<  i << "  #" << startIndex << ".. @ " << amic->getMicPosString() << endl;
        }
        if (chan.isEmpty() && (elevation > -999)) {
            cerr << "No microphone positions specified!" << endl;
        }

        if (!quiet) {
            cerr << "array "<< i ;
            cerr.precision(4);
            cerr << fixed;

            for (int mi=0;mi<amic->getMicrophoneCount();mi++) {
                DoublePoint3 mp =  amic->getMicPosition(mi);
                cerr << " " << mp.x << ","  << mp.y << "," << mp.z;
            }
            cerr << endl;
        }

        PoAPLocalizerGeneric* p = new PoAPLocalizerGeneric();
        it.add(p,chan.toVector());
        if (debug && i==0) {
            p->setDebug(debug);
        }
        if (input->getChannels() < p->getChannelCount()) {
            cerr << "input has less channels than specified." << endl;
            return 79;
        }
        try {
            p->setInputFrameCount(frameCount);
            p->setMic(amic);
            p->setSamplingFrequency(samplingFrequnecy);
            p->setGain(static_cast<PoAPLocalizer::GainMode>(gainMode),gainSmooth,gainMax);
            p->setFilter(bandCount,loFreq,hiFreq,gain,bandwidth);
            p->setSpikes(spikePrecedence,spikeAverage,spikeGain,relativeSpikeThreshold,absoluteSpikeThreshold,spikeMode);
            p->setCorrelationFraming(wlen,step);
            p->setBackpro(precision, rotation, elevation-elevationMin, precision2, !noBpInter, elevationMin);
            p->setCombination(gamma);
            p->setBandSum(sumbands);
            p->setPost(postTime,postBands,postEnergy,poaP,poaA,postTh,postPeak,postStep);
            p->precalc();
            p->setElevationMode(elmax ? 1:0);
        }
        catch (std::runtime_error e) {
            std::cerr << e.what() << std::endl;
            exit(111);
        }
    }

    if (!quiet) {
        QVector<double> fs = it.getCenterFrequencies();
        cerr << "Center frequencies: " ;
        for (int index=0;index<fs.size();index++) {
            if (index) {
                cerr << ", ";
            }
            cerr << fs[index];
        }
        cerr << endl;
    }

    signal(SIGINT, abort);

    it.init(input, skip, micarrayoffset, outtimeoffset);
    it.setQuiet(quiet);
    if (maxTime>0) {
        it.setFrameLimit((maxTime*samplingFrequnecy)/frameCount);
    }
    it.setMinLikelihood(minLikelihood);
    if (em.length()) {
        QStringList emprs = em.split(',');
        QList<double> empr;
        foreach(QString s, emprs) {
            empr.append(s.toDouble());
        }
        it.setEM(postTime,empr);
    } else {
        it.setArgmax(argmax,sumbands);
    }

    it.start();

    return a.exec();
}
