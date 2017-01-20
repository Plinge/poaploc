#-------------------------------------------------
#
# Project created by QtCreator 2015-09-17T20:29:47
#
#-------------------------------------------------

TARGET = poapify

win32 {
DESTDIR = 'C:/local_bin/'
} else {
DESTDIR = ~/bin/
}

CONFIG += console \
 warn_off

CONFIG -= warn_on
CONFIG += c++11

DEFINES += QT_CORE_LIB
DEFINES += USE_FFTW
win32 {
DEFINES += WIN32
}
DEFINES += M_PI=3.14159265358979323846

win32 {
QMAKE_CXXFLAGS_RELEASE += /openmp /O2 /Ot /fp:fast
} else {
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -ffast-math
QMAKE_CXXFLAGS += -g -march=corei7
QMAKE_CXXFLAGS_RELEASE += -g -O4 -finline-functions -fopenmp
}

DEPENDPATH += .
MOC_DIR += ./tmp_moc
UI_DIR += ./tmp_ui
RCC_DIR += ./tmp_rcc

INCLUDEPATH += . ../../include

win32 {
!contains(QMAKE_TARGET.arch, x86_64) {
INCLUDEPATH += C:/usr/lib/fftw32
LIBS += -LC:/usr/lib/fftw32
LIBS += -LC:/Qt/Tools/mingw492_32/lib/gcc/i686-w64-mingw32/4.9.2
LIBS += -lgomp
LIBS += -lfftw3-3 -lfftw3f-3
} else {
INCLUDEPATH += C:/usr/lib/fftw64
LIBS += -LC:/usr/lib/fftw64
LIBS += -lfftw3-3 -lfftw3f-3
}
} else {
macx: {
INCLUDEPATH += /Users/pli/homebrew/Cellar/fftw/3.3.5/include
LIBS += /Users/pli/homebrew/Cellar/fftw/3.3.5/lib
LIBS += -lfftw3 -lfftw3f
} else {
LIBS += -lgomp
LIBS += -lfftw3 -lfftw3f
}
}

SOURCES += \
    main.cpp \
    ../../include/qtap/qcommandlinearguments.cpp \
    ../../include/hearing/erbscaling.cpp \
    ../../include/wave/floatwave.cpp \
    ../../include/wave/sparsewave.cpp \
    ../../include/wave/wavefile.cpp \
    ../../include/neuro/poapifier.cpp \
    ../../include/neuro/poapifywave.cpp \
    ../../include/neuro/histogram.cpp \
    ../../include/filter/delayfilter.cpp \
    ../../include/filter/fftfilterbank-fftwf.cpp

HEADERS += \
    ../../include/neuro/poapifier.h \
    ../../include/neuro/poapifywave.h \
    ../../include/neuro/histogram.h \
    ../../include/filter/delayfilter.h \
    ../../include/filter/fftfilterbank.h \
    ../../include/filter/fftfilterbank_impl.h \
    ../../include/filter/fftfilterbank-fftwf.h \
    ../../include/filter/filter.h \
    ../../include/filter/filterbank.h \
    ../../include/filter/filterbankcentered.h \
    ../../include/filter/filtering.h \
    ../../include/filter/movingaverage.h

