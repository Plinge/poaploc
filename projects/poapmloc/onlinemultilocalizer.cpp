#include "onlinemultilocalizer.h"
#include <iostream>
//#include <malloc.h>
#include "math/floatmatrix.h"
#include "geometry/angles.h"
#include "assert.h"

using namespace std;

OlineMultiLocalizer::OlineMultiLocalizer(QObject *parent) :
    QObject(parent), frames(0), channels(0), done(false), buf(0), 
	argmax(false), em(false), minLikelihood(0), port(0), debug(0), framesMax(0),
    lastEmTime(0), quiet(false)
{
}


void OlineMultiLocalizer::init(ListenerThreadF* listener, int s, int o, double t)
{
    frames = listener->getFrameCount();
    channels = listener->getChannels();
    arrayOffset = o;
    timeOffset = t;
    if (!connect(listener,SIGNAL(closed()),this,SLOT(stopped())) ) {
        cerr << "fatal" << endl;
    }
    if (!connect(listener,SIGNAL(gotData(float*,bool)),this,SLOT(calc(float*,bool)))) {
          cerr << "fatal" << endl;
    }
    if (!connect(listener,SIGNAL(gotError(QString)),this,SLOT(error(QString)))) {
        cerr << "fatal" << endl;
    }
    buf = (float*)malloc(frames*channels*sizeof(float));
    if (buf==0) {
        cerr << "Out of memory!" << endl;
    }
    input = listener;
    frameSkip = s;
    if (port <= 0) {
        cout << "time" << "\t" << "micarray";
        if (!em && cor.first()->getOutputMicPairCount()>1) {
            cout << "\t" << "pair";
        }
        cout << "\t" << "angle" << "\t" << "elevation";
        cout << "\t" << "energy" << "\t" << "value";
        int bandCount = cor.first()->getOutputBandCount();
        if (bandCount>1) {
            for (int band=0;band<bandCount;band++) {
                cout << "\t" << "s" << band ;
            }
        }
        cout << "\tgain\tmeanen";
        cout << endl;
    }


    while (clusters.size()< arrayChannels.size()) {
        clusters.append(QList<AngularGaussian>());
		history.append(QList<LocalizedSpectrum>());
    }
}

void OlineMultiLocalizer::start()
{
    framesDone=0;
    timer.start();
    double a = cor.first()->getInputFrameCount();
    double b = cor.first()->getSamplingFrequency();
    frameTime = a/b;
    input->start(QThread::HighPriority);   
	lastEmTime = 0;
	linesout = 0;
}

void OlineMultiLocalizer::stopped()
{
    done = true;
    double timeTotal = cor.first()->getInputTime();
    //timer.stop();
    double procTime = timer.elapsed()/1000.0;
    fprintf(stderr,"%.2fs processed in %.2fs (%.3f x real time)                                           \n",
            timeTotal, procTime, procTime / timeTotal);    
    exit(0);
}

void OlineMultiLocalizer::error(QString message)
{
    std::cerr <<  (const char*)message.toLocal8Bit() << std::endl;
    done = true;
}


double OlineMultiLocalizer::emCluster(QList<LocalizedSpectrum> data, int arrayIndex, QList<AngularGaussian> & clusters)
{
    if (data.size()<1) {
        return 1.0;
    }

	int nband = data.front().getBandCount();
    int ndata = data.size();
    // data values
    QVector<double> azimuths(ndata);
    QVector<double> elevations(ndata);
    QVector<double> times(ndata);
    QVector<double> spectalEnergyN(ndata);
    QVector<double> spectalEnergy(ndata);

    QList<LocalizedSpectrum>::iterator it=data.begin();
    for (int sampleIndex=0;sampleIndex<ndata;sampleIndex++, ++it) {
        double s = it->getSpectralSum();
        double a = it->getAngle();
        double e = it->getElevation();
        double t = it->getTime();
        spectalEnergy[sampleIndex] = spectalEnergyN[sampleIndex] = s;
        azimuths[sampleIndex] = a;
        elevations[sampleIndex] = e;
        times[sampleIndex]=t;
    }
	normalize_vector(spectalEnergyN.data(), ndata);

    if (debug) {
        for (int sampleIndex=0;sampleIndex<ndata;sampleIndex++) {
            cerr << "t=" << times[sampleIndex] << " " <<  azimuths[sampleIndex] << "(" << spectalEnergy[sampleIndex] << ")" <<", " ;
        }
        cerr << endl;
    }

    int changes=0;
    int iter=0;
    double oldLike;

    while (iter++ < 10 || (changes>0 && iter<50)) {

        if (clusters.isEmpty())  {
            AngularGaussian a(data.front());
            a.setLabel(arrayIndex);
            DoublePoint3  center  =  cor.at(arrayIndex)->getMicrophoneCenter();
            a.x = center.x;
            a.y = center.y;
            a.z = center.z;
            a.updateAngle(a.azimuth,10.0);
            clusters.append(a);
            changes+=2;
        }

        if (changes==0) {


            if (clusters.size()>1)  {


                //
                // join close
                //

                for (int i=clusters.size()-1;i>=0;i--) {
                    for (int j=i+1;j<clusters.size();j++) {
                        double d = angleDiffernece( clusters[i].azimuth , clusters[j].azimuth);
                        if (d<=12.0) {
                            if (debug||true) {
                                cerr <<  "EM join " << clusters[i].azimuth << " & " << clusters[j].azimuth << " (" <<  d << ")" << endl;
                            }
                            double as[2]= { clusters[i].azimuth, clusters[j].azimuth  };
                            clusters[i].updateAngle( angleAverage(as,2), 0.5*(clusters[i].getVariance()+clusters[j].getVariance()) );
                            clusters.removeAt(j);
                            changes+=2;
                        }
                    }
                }

                //
                // clear empty
                //

                for (int densityIndex = clusters.size()-1;densityIndex >= 0;densityIndex--) {
                    if (clusters[densityIndex].getWeight()<0.01) {
                        // remove when low support
                        changes+=2;
                        cerr << "removing empty cluster " << clusters[densityIndex].toInfoString() << endl;
                        clusters.removeAt(densityIndex);
                    }
                }


            }

            //
            // split wide
            //

            for (int i=0;i<clusters.size();i++) {
                double v = clusters[i].getVariance();
                if (v>22.0*22.0) {
					
                    AngularGaussian ng(clusters[i]);
                    double sigma = sqrt(v);
                    double a1 = clusters[i].azimuth - sigma;
                    double a2 = clusters[i].azimuth + sigma;

                    if (debug||true) {
                        cerr <<  "EM split " << clusters[i].azimuth << " +- " << sqrt(v) << " => " << a1 <<", " << a2 << endl;
                    }

                    clusters[i].updateAngle(a1,v*0.5);
                    ng.updateAngle(a2,v*0.5);
                    double w = clusters[i].getWeight();
                    clusters[i].setWeight(w*0.5);
                    ng.setWeight(w*0.5);
                    clusters.append(ng);
                    changes+=2;

                    break;
                }
            }



        }

        int ndens = clusters.size();
        QVector<double> p_ij(ndens * ndata);
        QVector<double> p_j(ndata);
        QVector<double> p_i(ndens);
        double like=0;

        // E-Step

        it=data.begin();
        for (int sampleIndex=0;sampleIndex<ndata;sampleIndex++,++it) {
            p_j[sampleIndex]=0;
            for (int densityIndex=0;densityIndex<clusters.size();densityIndex++) {
                double p = clusters.at(densityIndex).prob(*it);
                if (p<1e-8) {
                    p=1e-8;
                }
                p_ij[densityIndex*ndata+sampleIndex] = p;
                p_j[sampleIndex] += p;
                like+=p;
            }
        }

        for (int sampleIndex=0;sampleIndex<ndata;sampleIndex++) {
            for (int densityIndex=0;densityIndex<clusters.size();densityIndex++) {
                p_ij[densityIndex*ndata+sampleIndex] /= p_j[sampleIndex];
                p_ij[densityIndex*ndata+sampleIndex] *= spectalEnergyN[sampleIndex];
                if (debug>999999) {
                    cerr << "y a="  << clusters.at(densityIndex).azimuth << " | " <<  azimuths[sampleIndex] << "(" << spectalEnergyN[sampleIndex] << ")" <<" => " << p_ij[densityIndex*ndata+sampleIndex] << endl;
                }
            }
        }

        if (debug) {
            cerr << "E-step " << iter << ", like " << like << endl;
            for (int densityIndex=0;densityIndex<clusters.size();densityIndex++) {
                cerr << clusters[densityIndex].toInfoString() << endl;
            }
        }
		// 
		// check if we are done
		//
        double dl=like - oldLike;
        oldLike=like;
        if (iter>5 && dl < 1e-6 && changes<=0) {
            // update values not used in the estimation
            for (int densityIndex=0;densityIndex<ndens;densityIndex++) {
                if (p_i[densityIndex]>0) {
                    QVector<double> y_j(ndata);
                    for (int sampleIndex=0;sampleIndex<ndata;sampleIndex++) {
                        y_j[sampleIndex] = p_ij[densityIndex*ndata+sampleIndex] / p_i[densityIndex];
                    }
                    double  a = weightedAngleAverage(azimuths.data(),y_j.data(),ndata);
                    double  e = weightedAngleAverage(elevations.data(),y_j.data(),ndata);
                    double  v = weightedAngleVariance(a,azimuths.data(),y_j.data(),ndata);
                    clusters[densityIndex].updateAngle(a,v,e);
                    double  t =  weightedAverage(times.data(),y_j.data(),ndata);
                    clusters[densityIndex].updateTime(t);
                }
            }
            break;
        }

        // M-Step

        QList<int> removals;

        sum_dim1(p_ij.data(),ndata,ndens,p_i.data());
        QVector<double> weight(ndens);
        double sumWeight=0.0;
        int remain = 0;

        for (int densityIndex=0;densityIndex<ndens;densityIndex++) {
            if (p_i[densityIndex]>0) {
                // training weights
                QVector<double> y_j(ndata);
                for (int sampleIndex=0;sampleIndex<ndata;sampleIndex++) {
                    y_j[sampleIndex] = p_ij[densityIndex*ndata+sampleIndex]; // p_i[densityIndex];
                }

                // angle
				double  a = weightedAngleAverage(azimuths.data(),y_j.data(),ndata);
                //double  e = weightedAngleAverage(elevations,y_j,ndata);
                double  v = weightedAngleVariance(a,azimuths.data(),y_j.data(),ndata);
                clusters[densityIndex].updateAngle(a,v);

                // spectrum as weighted average
                QVector<double> s(nband);
				double sn=0.0;
                for (int b=0;b<nband;b++) {
                    s[b]=0.0;
                }
                it=data.begin();
                for (int sampleIndex=0;sampleIndex<ndata;sampleIndex++,++it) {
                    for (int b=0;b<nband;b++) {
                        s[b] += y_j[sampleIndex] * it->getSpectralValue(b);
                        sn+= y_j[sampleIndex];
                    }
                }
                for (int b=0;b<nband;b++) {
                    s[b] /= sn;
                }
                clusters[densityIndex].updateSpectrum(s.data());

                // mixutre weight
                double w = p_i[densityIndex]/(double)ndata;
                //clusters[densityIndex].updateWeight(w);
                weight[remain++]=w;
                sumWeight+=w;
                double li=0;
                it=data.begin();
                for (int sampleIndex=0;sampleIndex<ndata;sampleIndex++,++it) {
                    li += y_j[sampleIndex] * it->getSpectralSum();
                }
                clusters[densityIndex].setProbability(li);

            } else {
                if (debug) {
                    cerr << "cluster " << clusters[densityIndex].toInfoString()  << "  has zero probability?!" << endl;
                }
                if (ndens>1) {
                    // remove when low support
                    removals.append(densityIndex);
                    changes+=3;
                }
            }
        }

        for (int densityIndex=ndens-1;densityIndex>=0;densityIndex--) {
            if (removals.contains(densityIndex)) {
                clusters.removeAt(densityIndex);
            }
        }    

        if  (clusters.size()>1) {
            for (int densityIndex=0;densityIndex<clusters.size();densityIndex++) {
                clusters[densityIndex].setWeight(weight[densityIndex]/sumWeight);
            }
        } else {
            if (clusters.size()) {
                clusters[0].setWeight(1.0);
            }
        }


        if (debug) {
            cerr << "M_step " << iter << ", clusters " << endl;
            for (int densityIndex=0;densityIndex<clusters.size();densityIndex++) {
                cerr << clusters[densityIndex].toInfoString() << endl;
            }
        }

        if (changes>0) {
            changes--;
        }

    }
}

bool OlineMultiLocalizer::checkEmParam(const AngularGaussian & a)
{
    if (sqrt(a.getVariance())>24.0) {
        return false;
    }

    if (emparam.size()>3) {
        if (a.getNonzeroBandCount()<emparam[3]) {
            return false;
        }
    } else {
        if (a.getNonzeroBandCount()<a.getBandCount()/2) {
            return false;
        }
    }
    if (emparam.size()>4) {
        if (a.getSpectralSum()<emparam[4]) {
            return false;
        }
    }
    return true;
}

void OlineMultiLocalizer::calc(float *data, bool continued)
{
    double inputTime = cor.first()->getInputTime();

	bool doEmThisTime = false;
	double emTime;
    bool printPairs = cor.first()->getOutputMicPairCount()>1;
    bool printBands = cor.first()->getOutputBandCount()>1;
    for (int arrayIndex=0;arrayIndex<arrayChannels.size();arrayIndex++) {
        PoAPLocalizerGeneric* p = cor.at(arrayIndex);
        const int* c0 = arrayChannels.at(arrayIndex).data();
        int cn = p->getChannelCount();
        float* ptr =buf;
        int index;
        for (index=0;index<frames;index++) {
            const int* cp = c0;
            for (int channel=0;channel<cn;channel++) {
                *ptr++ = *(data + index * channels + *cp++);
            }
        }

        p->calc(buf, frames, continued);

        if (em) {
            if (arrayIndex==0 && inputTime > emTimeWindow && cor.first()->getMaxOutTime() > lastEmTime + emTimeWindow) {
                doEmThisTime = true;
                emTime = lastEmTime + 0.5 * emTimeWindow;
            }
        }
        if (debug && em) {
			cerr << "inputTime = " << inputTime 
				 << " max out time = " << p->getMaxOutTime() ;
			if (doEmThisTime) {
				cerr << " will do EM for " << emTime ;
			}				 
		  	 cerr << endl;
		}

        std::list<LocalizedSpectrum> res;
        bool any = p->getLocalizedSpectra(res);
				
        if (em) { 
			if (framesDone >= frameSkip) {
				history[arrayIndex].append(QList<LocalizedSpectrum>::fromStdList(res));
			}
			
			if (doEmThisTime) {

				double minTime = emTime - 0.5 * emTimeWindow;
				for(int i=0;i<history[arrayIndex].size();) {
					if (history[arrayIndex].at(i).time < minTime) {
						history[arrayIndex].removeAt(i);
					} else {
						i++;
					}				
				}			 
            
				emCluster(history[arrayIndex], arrayIndex,clusters[arrayIndex]);
				
				foreach (AngularGaussian a, clusters[arrayIndex])  {
					if  (checkEmParam(a)) {
						// double t = a.getTime()  + timeOffset;
						double t = emTime;
						cout << t << "\t" << arrayIndex + arrayOffset << "\t";
						cout << a.getAngle() << "\t" << a.getElevation();
						double s = a.getSpectralSum();
						cout << "\t" << (s>0? 20.0*log10(s) : -200) << "\t" << s;
                        if (a.getBandCount()>1) {
							for (int band=0;band<a.getBandCount();band++) {
								cout << "\t" << a.getSpectralValue(band);
							}
						}
						cout << "\t" << p->getActiveAverageGain() << "\t" << p->getEnergyEstimate();
						cout << endl;
						cout << flush;
						++linesout;
					}
				}			        
			} 
		} else {
			//
			// processing without em
			// 
            if (any && argmax) {
                std::list<LocalizedSpectrum> res2;             
				if (sumbands) {
					//
					// argmax on all bands, return the spectrum with the highest energy
					// 
					LocalizedSpectrum maxSpec;
					double maxValue=0.0;
					for (std::list<LocalizedSpectrum>::iterator it=res.begin(); it != res.end(); ++it) {						
						double s = it->getSpectralSum();
						if (s>maxValue) {
							maxSpec = *it;
							maxValue = s;
						}
					}
					if (maxValue>0.0) {
						res2.push_back(maxSpec);
					}
				} else {
					//
					// argmax bandwise
					//
					double minValue  = pow(2.0, (double)cor.first()->getPostMinEn()/6.0);
					int bandCount = cor.first()->getBandCount();
					for (int bandIndex=0;bandIndex<bandCount;bandIndex++) {
						LocalizedSpectrum maxSpec;
						double maxValue = minValue;		
						for (std::list<LocalizedSpectrum>::iterator it=res.begin(); it != res.end(); ++it) {
							double s = it->getSpectralValue(bandIndex);
							if (s>maxValue) {
								maxSpec = *it;
								for (int otherBand=0;otherBand<bandCount;otherBand++) {
									if (otherBand!=bandIndex) {
										maxSpec.setSpectralValue(otherBand,0.0);
									}
								}
								assert( maxSpec.getSpectralSum() > maxValue);
								maxValue = s;								
							}
						}
						if (maxValue>minValue) {
							assert( maxSpec.getSpectralSum() > minValue);
							//double test =  maxSpec.getSpectralSum();
							res2.push_back(maxSpec);
						}
					}
				}
                res.clear();
                res = res2;                  
            }
            if (framesDone >= frameSkip) {
                for (std::list<LocalizedSpectrum>::iterator it=res.begin(); it != res.end(); ++it) {
                    LocalizedSpectrum spec = *it;
					double s = spec.getSpectralSum();
					if (s<=0.0) continue;
                    double t = spec.getTime();
                    cout << t + timeOffset << "\t" << arrayIndex + arrayOffset << "\t";
                    if (printPairs) {
                        cout << spec.getIndex() << "\t";
                    }
                    cout << spec.getAngle() << "\t" << spec.getElevation();                    
                    cout << "\t" << (s>0? 20.0*log10(s) : -200) << "\t" << s;
                    if (printBands) {
                        for (int band=0;band<spec.getBandCount();band++) {
                            cout << "\t" << spec.getSpectralValue(band);
                        }
                    }
                    cout << "\t" << p->getActiveAverageGain() << "\t" << p->getEnergyEstimate();
                    cout << endl;
                    cout << flush;
					++linesout;
                }
            }
        }
    }

	if (doEmThisTime) {
		lastEmTime = emTime;
	}
    double procTime = timer.elapsed()/1000.0;

    if (!quiet) {
        fprintf(stderr,"%.2fs processed in %.2fs (%.3f x real time) %d data points for %d windows            \r",
                inputTime + frameTime, procTime, procTime / (inputTime+ frameTime), linesout, framesDone+1);
    }

    if (input->stopped()) {
        return;
    }

    input->next();

    ++framesDone;    
    if  ((debug && framesDone >= debug) || (framesMax && framesDone>1+framesMax)) {
        input->stop();
        fprintf(stderr,"%.2fs processed in %.2fs (%.3f x real time) %d data points for %d windows            \r",
                inputTime + frameTime, procTime, procTime / (inputTime+ frameTime), linesout, framesDone+1);
    }
    return;
}
