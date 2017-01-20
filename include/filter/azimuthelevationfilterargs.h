#ifndef AZIMUTHELEVATIONFILTERARGS_H
#define AZIMUTHELEVATIONFILTERARGS_H

#include "filter/azimuthelevationfilter.h"

class AzimuthElevationFilterArgs : public AzimuthElevationFilter
{
protected:
    double frameStep;

public:
    AzimuthElevationFilterArgs(WavGroupAzimuthElevation* b, AzimuthElevationBackprojection* p)
      : AzimuthElevationFilter(b,p)
    {}

    virtual ~AzimuthElevationFilterArgs()
    {}

    virtual bool postFilterByArgs(QString & postOptionString)
    {
        QStringList opt = postOptionString.split("-");
        int i = 0;
        while (i<opt.size()) {
            LogFmtA("AzimuthElevationFilter.postFilterByArgs %s", (const char*)opt.at(i).toLocal8Bit());
            QStringList kv = opt.at(i).split(":");
            if (!postFilterBy( kv )) {
              LogFmtA("E_ postFilter failed at %s",(const char*)opt.at(i).toLocal8Bit());
              return false;
            }
            LogFmtA("AzimuthElevationFilter.postFilterByArgs => %d as, %d es, %d ss", azimuthCount, elevationCount, sampleCount);
            ++i;
        }
        return true;
    }

    virtual bool postFilterBy(QStringList & kv)
    {
        /*
        if (kv.at(0) == "av") {
            frameStep = 1.0 / bands->getSamplingFrequnecy();
            double t = kv.at(1).toDouble();
            if (t<=0.0) t = frameStep*3.0;
            cout << "Averaging " << t << "s" << endl;
            postAverage( t / frameStep, t*0.25 / frameStep );
        }
        */

        if (kv.at(0) == "bs") {
            double div = 0;
            double en = -90.0;
            if (kv.size()>1) {
                QString t = kv.at(1);
                div = t.replace('m','-').toDouble();
            }
            if (kv.size()>2) {
                en = -kv.at(2).toDouble();
            }
            if (div==0) {
                if (kv.size()>1) {
                    cerr << "ignored " << (const char*)kv.at(1).toLocal8Bit() << " as 0" << endl;
                }
                cout << "Summing bands, e > " << en << endl;
                postBandSum( 0, en);
            } else if (div<0)  {
                cout << "Summing bands, n > " << -div << ", e > " << en << endl;
                postBandSum( -div , en);
            } else {
                int bandCount = bands->getCount();
                cout << "Summing bands, n > " <<  bandCount / div << ", e > " << en << endl;
                postBandSum( bandCount / div , en);
            }
            return true;
        }
        if (kv.at(0)=="grid") {
            int a1=5,a2=45;
            if (kv.size()>1) {
                a1 = kv.at(1).toInt();
            }
            if (kv.size()>2) {
                a2 = kv.at(2).toInt();
            }
            cout << "PoAP Filtering "<<a1<<"/"<<a2<<", grid searching maxima" << endl;
            return postFilter(POST_AEGRID,a1,a2,0);
        }
        if (kv.at(0)=="th") {
            int th = kv.at(1).toInt();
            cout << "Thresholding " << th << endl;
            return postFilter(POST_THRESHOLD,th,0,0);
        }
        if (kv.at(0)=="poap") {
            int a1=5,a2=45;
            if (kv.size()>1) {
                a1 = kv.at(1).toInt();
            }
            if (kv.size()>2) {
                a2 = kv.at(2).toInt();
            }
            cout << "Sum elevation, PoAP Filtering  "<<a1<<"/"<<a2<<" .." << endl;
            return postFilter(POST_SUME_AZPOAP,a1,a2,0);
        }
        if (kv.at(0)=="poa") {
            int a1=5,a2=45;
            if (kv.size()>1) {
                a1 = kv.at(1).toInt();
            }
            if (kv.size()>2) {
                a2 = kv.at(2).toInt();
            }
            cout << "PoA Filtering  "<<a1<<"/"<<a2<<" .." << endl;
            return postFilter(POST_AZPOA,a1,a2,0);
        }
        if (kv.at(0)=="sea") {
            cout << "Sum elevation, argmax azimuth" << endl;
            return postFilter(POST_SUME_ARGMAX,0,0,0);
        }
        if (kv.at(0)=="es") {
            cout << "Sum elevation" << endl;
            return postFilter(POST_SUME,0,0,0);
        }
        if (kv.at(0)=="em") {
            cout << "Max elevation" << endl;
            return postFilter(POST_MAXE,0,0,0);
        }
        if (kv.at(0)=="max") {
            cout << "Argmax azimuth/elevation" << endl;
            return postFilter(POST_ARGMAX,0,0,0);
        }
        return false;
    }

};

#endif // AZIMUTHELEVATIONFILTERARGS_H
