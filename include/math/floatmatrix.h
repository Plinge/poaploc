#ifndef FLAOTMATRIX_H
#define FLAOTMATRIX_H

//
// matrix helpers
//

inline void normalize_vector(double* a,int l)
{
    double* p = a;
    double s=0.0;
    for (int i=0;i<l;i++) {
        s+=*p++;
    }
    p = a;
    for (int i=0;i<l;i++) {
        *p = *p / s; p++;
    }
}

inline void sum_dim0(double* matrix, int dim0, int dim1, double* dest_vector)
{
    for (int i=0;i<dim0;i++) {
        double s=0.0;
        for (int j=0;j<dim1;j++) {
            s+= matrix[j*dim0+i];
        }
        dest_vector[i]=s;
    }
}

inline void sum_dim1(double* matrix, int dim0, int dim1, double* dest_vector)
{
     for (int j=0;j<dim1;j++) {
        double s=0.0;
        for (int i=0;i<dim0;i++) {
            s += matrix[j*dim0+i];
        }
        dest_vector[j]=s;
    }
}

#endif
