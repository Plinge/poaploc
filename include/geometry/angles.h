#ifndef ANGLES_H
#define ANGLES_H

//
// angle helpers
//

inline double angleAverage(double* a, int n)
{
    double mi=0.0,ma=0.0;

    mi=ma=*a;
    for (int i=1;i<n;i++) {
        mi=std::min<double>(a[i],mi);
        ma=std::max<double>(a[i],ma);
    }
    double s=0.0;
    if (mi<-90.0 && ma>90.0) {
        for (int i=0;i<n;i++) {
            double xx = make180(a[i]+180.0);
            s+= xx;

        }
        if (n>0.0) {
            s = make180((s)-180.0);
        }
    } else {
        for (int i=0;i<n;i++) {
            s+= a[i];
        }
        if (n>0.0) {
            s = make180(s);
        }
    }
    return s;
}


inline double weightedAverage(double* a,double *w, int n)
{
    double sum_angles_weighted=0.0;
    double sum_weights=0;
    double mean;
    for (int i=0;i<n;i++) {
        sum_angles_weighted  += a[i]*w[i];
        sum_weights += w[i];
    }
    if (sum_weights>0.0) {
        mean = make180(sum_angles_weighted/sum_weights);
    }
    return mean;
}


inline double weightedAngleAverage(double* a,double *w, int n)
{
    double mi=0.0,ma=0.0;

    mi=ma=*a;
    for (int i=1;i<n;i++) {
        mi=std::min<double>(a[i],mi);
        ma=std::max<double>(a[i],ma);
    }
    double sum_angles_weighted=0.0;
    double sum_weights=0;
    double mean;
    if (mi<-90.0 && ma>90.0) {
        for (int i=0;i<n;i++) {
            double xx = make180(a[i]+180.0);
            sum_angles_weighted+= xx*w[i];
            sum_weights += w[i];
        }
        if (sum_weights>0.0) {
            mean = make180((sum_angles_weighted/sum_weights)-180.0);
        }
    } else {
        for (int i=0;i<n;i++) {
            sum_angles_weighted  += a[i]*w[i];
            sum_weights += w[i];
        }
        if (sum_weights>0.0) {
            mean = make180(sum_angles_weighted/sum_weights);
        }
    }
    return mean;
}

inline double weightedAngleVariance(double mean, double* a,double *w, int n)
{
    double sum_square_diff_weighted=0.0,sum_weights=0.0;
    for (int i=0;i<n;i++) {
        double d = angleDiffernece(mean,a[i]);
        sum_square_diff_weighted+=w[i]*d*d;
        sum_weights+=w[i];
    }
    return sum_square_diff_weighted/sum_weights;
}

#endif // ANGLES_H

