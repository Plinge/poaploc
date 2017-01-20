#include "filter/azimuthelevationfilter.h"

void AzimuthElevationFilter::prepare(Audio::Wave& in, const char* operation)
{    
    azimuthCount   = backPro->getAzimuthStepCount();
    azimuthStep =  backPro->getAzimuthStep();
    int channels = in.getChannels();
    int e  = (int)ceil((double)channels / azimuthCount);
    elevationCount = e>1 ? backPro->getElevationStepCount() : 1;
    sampleCount = in.getFrameCount();
//    LogFmtA("AzimuthElevationFilter.prepare %s %d as, %d es, %d ss (e=%d, c=%d)", operation, azimuthCount, elevationCount, sampleCount, e, channels);
}

void AzimuthElevationFilter::bandSum( Audio::WavGroup& data, Audio::SparseWave & out, int minBands, double th)
{
    int sampleCount = data.getFrameCount();
    int bandCount = data.count();
    int channelCount = data.getChannels();
    out.create(data.getSamplingFrequency(), channelCount, sampleCount);
    for (int sample=0;sample < sampleCount;sample++) {
// #pragma omp parallel for
        for (int channel=0;channel < channelCount;channel++) {
            int bands=0;
            double sum=0.0;
            for (int band=0;band<bandCount;band++) {
                Audio::Wave* bandWave = data.at(band).data();
                if (sample < bandWave->getFrameCount() && channel < bandWave->getChannels()) {
                    double v = bandWave->getSample(sample, channel);
                    if (v > 0) {
                        sum+=v;
                        bands++;
                    }
                }
            }
            if (bands >= minBands && sum>th) {
                out.setSample(sample, channel, sum);
            }
        }
    }
}


void AzimuthElevationFilter::thresholdBandcount( Audio::WavGroup& data, int minBands, double th)
{
    int sampleCount = data.getFrameCount();
    int bandCount = data.count();
    int channelCount = data.getChannels();    
    for (int sample=0;sample < sampleCount;sample++) {
// #pragma omp parallel for
		for (int channel=0;channel < channelCount;channel++) {
			int bands=0;
			double sum=0.0;
			for (int band=0;band<bandCount;band++) {
				Audio::Wave* bandWave = data.at(band).data();
				if (sample < bandWave->getFrameCount() && channel < bandWave->getChannels()) {
					double v = bandWave->getSample(sample, channel);
					if (v > 0) {
						sum+=v;
						bands++;
					}
				}
			}
			if (bands < minBands) {
				for (int band=0;band<bandCount;band++) {
					Audio::Wave* bandWave = data.at(band).data();
					if (sample < bandWave->getFrameCount() && channel < bandWave->getChannels()) {
						bandWave->setSample(sample, channel, 0.0);
					}
				}
			} else {
				for (int band=0;band<bandCount;band++) {
					Audio::Wave* bandWave = data.at(band).data();
					if (sample < bandWave->getFrameCount() && channel < bandWave->getChannels()) {
						double v = bandWave->getSample(sample, channel);
						if (v < th) {
							bandWave->setSample(sample, channel, 0.0);
						}
					}
				}
			}
		}
    }
}




void AzimuthElevationFilter::elevationMax(Audio::Wave& in, Audio::SparseWave & out)
{
    prepare(in,"elevationMax");
    double fout = in.getSamplingFrequency();
    out.create(fout, azimuthCount, sampleCount);
    for (int sample=0;sample<sampleCount;sample++) {
        for (int azimuth=0;azimuth<azimuthCount;azimuth++) {
            double ma = 0.0;
            for (int elevation=0;elevation<elevationCount;elevation++) {
                double v = getSample(in, sample, azimuth, elevation);
                if (v>ma) ma=v;
            }
            out.setSample(sample, azimuth, ma);
        }
    }
    elevationCount=1;
}

void AzimuthElevationFilter::elevationSum(Audio::Wave& in, Audio::SparseWave & out)
{
    prepare(in);
    double fout = in.getSamplingFrequency();
    out.create(fout, azimuthCount, sampleCount);
    for (int sample=0;sample<sampleCount;sample++) {
        for (int azimuth=0;azimuth<azimuthCount;azimuth++) {
            double sum = 0.0;
            for (int elevation=0;elevation<elevationCount;elevation++) {
                sum+= getSample(in, sample, azimuth, elevation);
            }
            out.setSample(sample, azimuth, sum);
        }
    }
    elevationCount=1;
}

void AzimuthElevationFilter::argmax(Audio::Wave& in, Audio::SparseWave & out)
{
    prepare(in,"argmax");
    double fout = in.getSamplingFrequency();
    out.create(fout, azimuthCount*elevationCount, sampleCount);
    for (int sample=0;sample<sampleCount;sample++ ) {
        double ma = 0.0;
        int a=0,e=0;
        for (int azimuth=0;azimuth<azimuthCount;azimuth++) {
            for ( int elevation=0;elevation<elevationCount;elevation++ ) {
                double v = getSample(in, sample, azimuth, elevation);
                if (v>ma) {
                     ma = v;
                     a = azimuth;
                     e = elevation;
                }
            }
        }
        if (ma>0.0) {
            setSample(out, sample, a, e, ma);
        }
    }
}

void AzimuthElevationFilter::threshold( Audio::Wave& in, Audio::SparseWave & out,double th)
{
    prepare(in);
    double fout = in.getSamplingFrequency();
    out.create(fout, azimuthCount*elevationCount, sampleCount);
    for ( int sample=0;sample<sampleCount;sample++ ) {
        for (int index=0;index<azimuthCount*elevationCount;index++) {
            double v = in.getSample(sample, index );
            if (v>th) {
                out.setSample( sample, index , v );
            }
        }
    }
}

void AzimuthElevationFilter::poaAzimuthSub(bool poap, Audio::Wave& in, Audio::SparseWave & out,  int average1, int average2, int postThreshold)
{
    if (average1==0 || average2==0) {
        in.copyTo(&out);
        return;
    }
    prepare(in,"poaAzimuthSub");
    double* aval = (double*) malloc(azimuthCount*sizeof (double));
    int averageDegrees = (int)ceil(average2 / azimuthStep) +1;
    int averageDegrees_2 = averageDegrees>>1;
    double fout = in.getSamplingFrequency();
    out.create(fout, elevationCount*azimuthCount, sampleCount);
    out.zero();
    int averageDeg1   = std::max<int>(1, average1 / azimuthStep)+1;
    int averageDeg1_2 = averageDeg1>>1;
    for (int sample=0;sample<sampleCount;sample++) {
        for (int elevation=0;elevation<elevationCount;elevation++) {
            double f = 1.0 / (double)(averageDeg1);
            double sum = getSample(in, sample, 0, elevation);
            for ( int azimuth=1;azimuth<averageDeg1_2;azimuth++ ) {
                sum+= getSample(in, sample, (azimuthCount-azimuth), elevation);
                sum+= getSample(in, sample, azimuth, elevation);
            }
            int lastAngle = azimuthCount-averageDeg1_2+1;
            int nextAngle = averageDeg1_2;
            bool any = sum>0;
            for (int azimuth=0;azimuth<azimuthCount;azimuth++) {
                if (lastAngle>=azimuthCount) lastAngle=0;
                if (nextAngle>=azimuthCount) nextAngle=0;
                aval[azimuth] = sum*f;
                sum += getSample(in, sample, nextAngle, elevation);
                sum -= getSample(in, sample, lastAngle, elevation);
                lastAngle++;
                nextAngle++;
                if (sum>0) any=true;
            }
            if (!any) continue;  // skip empty
            // init by warped val
            sum = 0.0;
            for (int angle=0;angle<averageDegrees_2;angle++) {
                sum += aval[azimuthCount-angle-1];
                sum += aval[angle];
            }
            lastAngle = azimuthCount-averageDegrees_2;
            nextAngle = averageDegrees_2;
            int midAngle = 0;
            double opoa = 0.0;
            double poaMax = 0.0;
            int poaMaxPos = 0;
            for (int azimuth=0;azimuth<azimuthCount+averageDegrees+azimuthCount/12;azimuth++) {
                sum -= aval[lastAngle];
                sum += aval[nextAngle];
                double avg = sum / (double)averageDegrees;
                double peak = aval[midAngle];
                // increase average by threshold
                if (postThreshold!=0) {
                    avg *= pow(2.0, postThreshold/6.0);
                }
                double poa = 0.0;
                if (peak>0.0 && avg>0.0) {
                    poa = 20.0*log10 ( peak / avg ) ;
                }
                double poad = peak-avg;
                if (poap) {
                    if (poa>0) {
                        if (opoa<=0) { // upcross
                            // init max
                            if (poad > poaMax) {
                                poaMax = poad;
                                poaMaxPos = midAngle;
                            }
                        } else  {
                            // check for max
                            if (poad > poaMax) {
                                poaMax = poad;
                                poaMaxPos = midAngle;
                            }
                        }
                    }
                    if (poa<=0 && opoa>0) { // downcross
                        setSample(out, sample, poaMaxPos, elevation, poaMax);
                        poaMax = 0;
                    }
                    opoa=poa;
                } else {
                    if (poa>0) {
                        int az = azimuth;
                        while (az>=azimuthCount) az-=azimuthCount;
                        setSample(out, sample, az, elevation, poad);
                    }
                }
                midAngle++;
                lastAngle++;
                nextAngle++;
                if (lastAngle>=azimuthCount) lastAngle=0;
                if (midAngle>=azimuthCount) midAngle=0;
                if (nextAngle>=azimuthCount) nextAngle=0;
            }
        }
    }
    free(aval);
}


void AzimuthElevationFilter::peakAzimuthElevationGrid(Audio::Wave& in, Audio::SparseWave & out, int average1, int average2, int postThreshold)
{
    //azimuthCount   = backPro->getDim1Size();
    //elevationCount = in.getChannels()/azimuthCount;
    //double azimuthStep =  backPro->getAzimuthStep();
    prepare(in,"peakAzimuthElevationGrid");
    int sampleCount = in.getFrameCount();
    double* aval = ( double* ) malloc ( azimuthCount*sizeof ( double ) );
    int averageDegrees = (int)ceil( average2 / azimuthStep ) +1;
    int averageDegrees_2 = averageDegrees>>1;
    int averageDeg1  = std::max<int>(1, average1/ azimuthStep)+1;
    int averageDeg1_2 = averageDeg1>>1;
    out.create(in.getSamplingFrequency(), elevationCount*azimuthCount, sampleCount);
    int nout=0;
    for (int sample=0;sample<sampleCount;sample++) {
        // calc average over all elevations
        double f =  1.0 / ( double ) ( averageDeg1 );
        double sum = 0.0;
        for (int elevation=0;elevation<elevationCount;elevation++) {
            sum +=  getSample(in, sample, 0, elevation);
            for (int azimuth=1;azimuth<averageDeg1_2;azimuth++) {
                sum+= getSample(in, sample, azimuthCount-azimuth, elevation);
                sum+= getSample(in, sample, azimuth, elevation);
            }
        }
        bool any = sum>0;
        int lastAngle = azimuthCount-averageDeg1_2+1;
        int nextAngle = averageDeg1_2;
        for (int azimuth=0;azimuth<azimuthCount;azimuth++) {
            if ( lastAngle>=azimuthCount ) lastAngle=0;
            if ( nextAngle>=azimuthCount ) nextAngle=0;
            aval[azimuth] = sum*f;
            for ( int elevation=0;elevation<elevationCount;elevation++ ) {
                sum += getSample(in, sample, nextAngle, elevation);
                sum -= getSample(in, sample, lastAngle, elevation);
            }
            lastAngle++;
            nextAngle++;
            if (sum>0) any=true;
        }
        if (!any) continue;  // skip empty
        // init by warped val
        sum = 0.0;
        for (int angle=0;angle<averageDegrees_2;angle++) {
            sum += aval[azimuthCount-angle-1];
            sum += aval[angle];
        }
        lastAngle = azimuthCount-averageDegrees_2;
        nextAngle = averageDegrees_2;
        int midAngle = 0;
        double opoa = 0.0;
        double poaMax = 0.0;
        int poaMaxAzimuth = 0;
        for (int azimuth=0;azimuth<azimuthCount+averageDegrees+azimuthCount/12;azimuth++) {
            if (lastAngle>=azimuthCount) lastAngle=0;
            if (midAngle>=azimuthCount) midAngle=0;
            if (nextAngle>=azimuthCount) nextAngle=0;
            sum -= aval[lastAngle];
            sum += aval[nextAngle];
            double avg = sum / ( double ) averageDegrees;
            double peak = aval[midAngle];
            if (postThreshold!=0) {
                avg *= pow ( 2.0, postThreshold/6.0 );
            }
            double poa = 0.0;
            if (peak>0.0 && avg>0.0) {
                poa = 20.0*log10 ( peak / avg ) ;
            }
            double poad = peak-avg;
            if (poa>0) {
                if (opoa<=0) { // upcross
                    if (poad > poaMax) {
                        poaMax = poad;
                        poaMaxAzimuth = midAngle;
                    }
                } else  {
                    if (poad > poaMax) {
                        poaMax = poad;
                        poaMaxAzimuth = midAngle;
                    }
                }
            }
            if (poa<=0 && opoa>0) { // downcross
                // search maximum in elevation direction
                int maxElevation = 0;
                if (elevationCount>0) {
                    double maxVal = 0;
                    for ( int el=0;el<elevationCount;el++ ) {
                        double sum=0;
                        for ( int az=0;az<averageDegrees;az++ ) {
                            int a = az + poaMaxAzimuth - ((averageDegrees-1)>>1);
                            if (a<0) a+=azimuthCount;
                            if (a>=azimuthCount) a-=azimuthCount;
                            sum += getSample(in, sample, a, el);
                        }
                        if (sum>maxVal) {
                            maxElevation=el;
                            maxVal=sum;
                        }
                    }
                }
                setSample(out, sample, poaMaxAzimuth, maxElevation, poaMax);
                ++nout;
                poaMax=0;
            }
            opoa=poa;
            midAngle++;
            lastAngle++;
            nextAngle++;
        } // azimuth loop
    } // sample loop
    free ( aval );

    // qDebug("%d peaks found, %d nonzero samples", nout, out.getNonzeroSampleCount());
}


void AzimuthElevationFilter::convolveWithGaussian(Audio::Wave& in, Audio::Wave & out)
{
    prepare(in,"convolveWithGaussian");
    double fout = in.getSamplingFrequency();
    out.create(fout, elevationCount*azimuthCount, sampleCount);
    out.zero();
    for (int sample=0;sample<sampleCount;sample++) {
        for (int elevation=0;elevation<elevationCount;elevation++) {
            for (int azimuth=0;azimuth<azimuthCount;azimuth++) {
                double v = getSample(in, sample, azimuth, elevation);
                if (v>0.0) {
                    for (int az=0;az<azimuthCount;az++) {
                        double d = abs(az-azimuth);
                        if (d<=15.0) {
                            double vv  = exp(-d/3.0);
                            vv += getSample(out, sample, azimuth, elevation);
                            if (vv>1.0) vv=1.0;
                            setSample(out, sample, azimuth, elevation, vv);
                        }
                    }
                }
            }
        }
    }
}

void AzimuthElevationFilter::averageAzimuth(Audio::Wave& in, Audio::SparseWave & out, int average2, double threshold)
{
    prepare(in,"averageAzimuth");
    double* aval = (double*)malloc(azimuthCount*sizeof(double));
    int averageDegrees = (int)ceil(average2/azimuthStep)+1;
    int averageDegrees_2 = averageDegrees>>1;
    out.create(in.getSamplingFrequency(), elevationCount*azimuthCount, sampleCount);
    for (int sample=0;sample<sampleCount;sample++) {
        for (int elevation=0;elevation<elevationCount;elevation++) {
            // init by warped val
            double sum = getSample(in, sample, 0, elevation);
            for (int angle=1;angle<averageDegrees_2;angle++) {
                sum += getSample(in, sample, (azimuthCount-angle), elevation);
                sum += getSample(in, sample, angle, elevation);
            }
            int lastAngle = azimuthCount-averageDegrees_2+1;
            int nextAngle = averageDegrees_2;
            int midAngle = 0;
            for ( int azimuth=0;azimuth<azimuthCount+averageDegrees;azimuth++ ) {
                if ( lastAngle>=azimuthCount ) lastAngle=0;
                if ( midAngle>=azimuthCount ) midAngle=0;
                if ( nextAngle>=azimuthCount ) nextAngle=0;
                sum -= getSample(in, sample, lastAngle , elevation );
                sum += getSample(in, sample, nextAngle, elevation );
                double avg = sum / ( double ) averageDegrees -  threshold;
                if (avg>0.0) {
                    setSample(out, sample, midAngle, elevation, avg );
                }
                midAngle++;
                lastAngle++;
                nextAngle++;
            }
        }
    }
}
