#include <QCoreApplication>
#include "qtap/qcommandlinearguments.h"
#include "wave/wavefile.h"
#include "neuro/poapifywave.h"

#include <iostream>
#include <omp.h>

using namespace std;

void copyright()
{
    cerr << endl;
    cerr << "This software implements the cochlear model decribed in:" << endl;
    cerr << "[1] Plinge, A., Hennecke, M. H., & Fink, G. A. (2010)." << endl;
    cerr << "    Robust Neuro-Fuzzy Speaker Localization Using a Circular Microphone Array." << endl;
    cerr << "    In 12th International Workshop on Acoustic Echo and Noise Control; Tel Aviv, Israel." << endl;
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
    double bandwidth=1.0;
    options.addOption(0,"bandwidth", &bandwidth);

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

    bool bandsum=false;
    options.addSwitch(0,"sum-bands",&bandsum);

    QString file;
    options.addArgument("input", &file);
    QString fileout;
    options.addArgument("output", &fileout);
    double gain=0.0;
    options.addOption(0,"gain", &gain);

    int gainMode=0;
    options.addOption(0,"gain-mode", &gainMode);
    double gainSmooth=0;
    options.addOption(0,"gain-smooth", &gainSmooth);
    double gainMax=50;
    options.addOption(0,"gain-max", &gainMax);
    int frameCount = 1024; // 8192; // 65536
    options.addOption(0, "frame-count", &frameCount);

    bool quiet=false;
    options.addSwitch(0,"quiet",&quiet);

    bool usage=false;
    options.addSwitch('h',"help",&usage);
    int debug=0;
    options.addOption(0,"debug",&debug);

    if (!options.parse() || usage) {
        copyright();
        cerr << (const char*)options.help().toLocal8Bit() << endl;
        cerr << "Up to " << omp_get_max_threads() << " threads for parallel processing." << endl;
        cerr << endl;
        cerr << "EXAMPLE: ";
        cerr << "poapify --bands 16 input.wav output.wav" << endl;
        cerr << "This will create an output.wav with n x 16 channels,  where n is the number of" << endl;
        cerr << "channels in input.wav." << endl;
        cerr << endl;
        cerr << "PARAMETERS:" << endl;
        cerr << "--spike-mode 1" << endl;
        cerr << "mode 0: no spike generation, just apply filterbank."  << endl;
        cerr << "mode 1: halfway rectification for spike generation (default)."  << endl;
        cerr << "mode 2: two way rectification for spike generation."  << endl;
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
        return usage?0:12;
    }

    if (debug) {
        std::cerr << (const char*)options.list().toLocal8Bit() << std::endl;
    }



    copyright();


    try{
        Audio::WaveFile wav;
        Audio::WaveFile out;

        if (!wav.read(file)) {
            return 13;
        }
        PoAPifyWave poapifier(wav);

        poapifier.setQuiet(quiet);
        poapifier.setGain(static_cast<PoAPifier::GainMode>(gainMode),gainSmooth,gainMax);
        poapifier.setFilter(bandCount,loFreq,hiFreq,gain,bandwidth);
        poapifier.setSpikes(spikePrecedence,spikeAverage,spikeGain,relativeSpikeThreshold,absoluteSpikeThreshold,spikeMode);
        poapifier.setSumBands(bandsum);
        poapifier.setDebug(debug);

        QVector<double> fs = poapifier.getCenterFrequencies();
        cerr << "Center frequencies: " ;
        for (int index=0;index<fs.size();index++) {
            if (index) {
                cerr << ", ";
            }
            cerr << fs[index];
        }
        cerr << endl;

        poapifier.process(frameCount,out,start,end);

        if (!out.write(fileout)) {
            cerr << "Failed to write file " << (const char*)fileout.toLocal8Bit() << " !" << endl;
            return 15;
        }
    }
    catch (std::bad_alloc) {
        cerr << "Out of memory! Aborting." << endl;
        return 455;
    }
    catch (std::runtime_error e) {
        cerr << "ERROR! " << e.what() << endl;
        return 255;
    }
    return 0;
}
