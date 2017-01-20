#ifndef CLIP_H
#define CLIP_H

inline double clip1(double value)
{
    if (value> 1.0) return  1.0;
    if (value<-1.0) return -1.0;
    return value;
}

inline float clip1(float value)
{
    if (value> 1.0F) return  1.0F;
    if (value<-1.0F) return -1.0F;
    return value;
}

inline short clip15s(int value) {
    if (value<-32767) return -32767;
    if (value> 32767) return  32767;
    return (short)value;
}

inline short clip15s(long value) {
    if (value<-32767) return -32767;
    if (value> 32767) return  32767;
    return (short)value;
}

inline short clip15s(double value) {
    if (value<-32767.0) return -32767;
    if (value> 32767.0) return  32767;
    return (short)value;
}


inline long clip31s(double value) {
    if (value<-32767.0*65536) return -32767*65536;
    if (value> 32767.0*65536) return  32767*65536;
    return (long)value;
}
#endif // CLIP_H
