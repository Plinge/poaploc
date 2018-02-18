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

#include <QCoreApplication>
#include "qtap/qcommandlinearguments.h"
#include "wave/wavefile.h"
#include "localization/poapcorrwave.h"
#include <iostream>
#include <omp.h>

using namespace std;

void copyright()
{
    cerr << endl;
    cerr << "This software implements the cochlear model decribed in:" << endl;
    cerr << "[1] Plinge, A., Hennecke, M. H., & Fink, G. A. (2010)." << endl;
    cerr << "    Robust Neuro-Fuzzy Speaker Localization Using a Circular Microphone Array." << endl;
    cerr << "    In 12th International Workshop on Acoustic Echo and Noise Control (IWAENC)" << endl;
    cerr << "    Tel Aviv, Israel; Sep 2010." << endl;
    cerr << endl;
}

int main(int argc, char *argv[])
{
//    QCoreApplication a(argc, argv);

    QCommandLineArguments options(argc,argv);
    double start=0.0;
    options.addOption(0,"start-time",&start);
    double end=-1.0;
    options.addOption(0,"end-time",&end);
    int bandCount=16;
    options.addOption('b', "bands", &bandCount);
    double loFreq=300;
    options.addOption('s', "fmin", &loFreq);
    double hiFreq=3000;
    options.addOption('e', "fmax", &hiFreq);
    double step=6;
    options.addOption('t', "frame-step",&step);
    double wlen=12;
    options.addOption('w', "frame-length",&wlen);
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
    int frameCount = 1024; // 8192; // 65536
    options.addOption(0, "frame-count", &frameCount);
    bool usage=false;
    options.addSwitch('h',"help",&usage);
    int debug=0;
    options.addOption(0,"debug",&debug);
    int samplingFrequnecy = 48000;
    options.addOption(0, "sampling-frequency", &samplingFrequnecy);
    int channels=0;
    options.addOption(0,"channels",&channels);
    int maxTdoa=-1;
    options.addOption(0,"max-tau",&maxTdoa);
    QString pair;
    options.addOption(0,"pair",&pair);
    bool bandsum=false;
    options.addSwitch(0,"bandsum",&bandsum);
    bool quiet=false;
    options.addSwitch(0,"quiet",&quiet);

    if (!options.parse() || usage) {
        cerr << (const char*)options.help().toLocal8Bit() << endl;
        cerr << endl;
        cerr << "PARAMETERS:" << endl;
        cerr << "--spike-mode 1" << endl;
        cerr << "mode 0: no spike generation, just apply filterbank."  << endl;
        cerr << "mode 1: halfway rectification PoAP spike generation (default)."  << endl;
        cerr << "mode 2: two way rectification PoAP spike generation (experimental)."  << endl;
        cerr << "mode 4: zero-crossing spike generation."  << endl;
        cerr << "mode 6: half-way rectification."  << endl;
        cerr << endl;
        cerr << "--bands 16 --fmin 300 --fmax 3000 --bandwidth 1.0" << endl;
        cerr << "Use  16 gammatone filters  with ERB spaced center frequnecies  between 300 and" << endl;
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
        copyright();
        return usage?0:12;
    }

    if (debug) {
        std::cerr << (const char*)options.list().toLocal8Bit() << std::endl;
    }

    copyright();

    if (frameCount < (step*1e-3 * samplingFrequnecy)) {
        cerr << "Parameter error, frame to small." << endl;
        return 12;
    }
    cerr.precision(6);
    cerr << "Frame count is " <<  frameCount << ", "
         << (frameCount / (step*1e-3 * samplingFrequnecy)) << " correlations each."
         << endl;

    try{
        Audio::WaveFile wav;
        Audio::FloatWave tmp;
        PoAPCorrWave* poapifier;
        if (!wav.read(file)) {
            return 13;
        }

        if (pair.size()>2) {
            QStringList chs = pair.split(',');
            int ch0 = chs[0].toInt();
            int ch1 = chs[1].toInt();
            tmp.create(wav.getSamplingFrequency(),2,wav.getFrameCount(),true);
            wav.copyChannelTo(&tmp,ch0,-1,0);
            wav.copyChannelTo(&tmp,ch1,-1,1);
            poapifier = new PoAPCorrWave(tmp);
        } else {
            poapifier = new PoAPCorrWave(wav);
        }

        poapifier->setQuiet(quiet);
        poapifier->setGain(static_cast<PoAPifier::GainMode>(gainMode),gainSmooth,gainMax);
        poapifier->setFilter(bandCount,loFreq,hiFreq,gain,bandwidth);
        poapifier->setSpikes(spikePrecedence,spikeAverage,spikeGain,relativeSpikeThreshold,absoluteSpikeThreshold,spikeMode);
        poapifier->setCorrelationFraming(wlen,step,maxTdoa);
        poapifier->setDebug(debug);
        poapifier->setBandSum(bandsum);

        poapifier->process(frameCount, start, end);

        delete poapifier;
    }
    catch (std::bad_alloc) {
        cerr << "Out of memory! Aborting." << endl;
        return 455;
    }
    catch (std::runtime_error e) {
        cerr << e.what() << endl;
        return 55;
    }

    return 0;
}
