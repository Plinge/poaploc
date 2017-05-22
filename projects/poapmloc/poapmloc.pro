#
#
#
macx {
QMAKE_MAC_SDK = macosx10.12
}

# adapt the follwing where necessary
MAC_INCLUDE =  /Users/pli/homebrew/opt/gcc-fftw3/include
#/Users/pli/local/include
#/Users/pli/homebrew/Cellar/gcc/6.3.0_1/include/c++/6.3.0
MAC_LIBS = -L/Users/pli/homebrew/opt/gcc-fftw3/lib

macx {
# MacOS X
INCLUDEPATH += $${MAC_INCLUDE}
LIBS += $${MAC_LIBS}
}

TEMPLATE = app
TARGET = poapmloc
macx {
DESTDIR = /Users/pli/bin/
CONFIG -= app_bundle
} else {
unix {
DESTDIR = /home/aplinge/bin/
DEFINES += HAVE_ALSA
}}
win32 {
DESTDIR = 'C:/local_bin/'
}

CONFIG += console warn_off

CONFIG -= warn_on
CONFIG += c++11

DEFINES += QT_CORE_LIB
DEFINES += USE_FFTW
win32 {
DEFINES += WIN32
}
DEFINES += M_PI=3.14159265358979323846

QMAKE_CFLAGS += -fPIC
QMAKE_CXXFLAGS += -fPIC



win32 {
QMAKE_CXXFLAGS_RELEASE += /openmp /O2 /Ot /fp:fast
} else {
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -g
QMAKE_CXXFLAGS_RELEASE += -O4 -finline-functions -fopenmp -ffast-math -march=corei7
}


DEPENDPATH += .
MOC_DIR += ./tmp_moc
UI_DIR += ./tmp_ui
RCC_DIR += ./tmp_rcc

HEADERS += onlinemultilocalizer.h \
    angulargaussian.h \
    angulardetection.h \
    ../../include/qtap/qcommandlinearguments.h \
    ../../include/micarr/azimuthelevationbackprojection.h \
    ../../include/filter/azimuthelevationfilter.h \
    ../../include/wave/sparsewave.h \
    ../../include/wave/wave.h \
    ../../include/wave/wavgroup.h \
    ../../include/micarr/micarray.h \
    ../../include/neuro/spikecorrelator.h \
    ../../include/micarr/poaplocalizer.h \
    ../../include/micarr/poaplocalizergeneric.h \
    ../../include/neuro/spikegeneratorpoapd.h \
    ../../include/micarr/wavecombination.h \
    ../../include/micarr/hamachercombination.h \
    ../../include/neuro/histogram.h \
    ../../include/wavio/listenerthreadf.h \
    ../../include/wavio/filelistenerf.h \
    ../../include/wave/wavefile.h \
    ../../include/wave/wave.h \
    ../../include/filter/auditoryfilterbankf.h \
    ../../include/geometry/doublept.h \
    ../../include/filter/fftfilterbank_impl.h \
    ../../include/filter/fftfilterbank.h \
    ../../include/filter/filterbankcentered.h \
    ../../include/wave/floatwave.h \
    ../../include/wave/sparsewave.h \
    ../../include/wave/wavgroup.h \
    ../../include/localization/azimuthelevationbackprojection.h \
    ../../include/localization/hamachercombination.h \
    ../../include/localization/poaplocalizer.h \
    ../../include/localization/poaplocalizergeneric.h \
    ../../include/localization/tabledbackprojection.h \
    ../../include/localization/tabulartwodeebackprojection.h \
    ../../include/neuro/poapifier.h

#Source files
SOURCES +=  ./main.cpp \
    onlinemultilocalizer.cpp \
    localizedspectrum.cpp \
    ../../include/qtap/qcommandlinearguments.cpp \
    ../../include/filter/azimuthelevationfilter.cpp \
    ../../include/neuro/histogram.cpp \
    ../../include/wavio/listenerthreadf.cpp \
    ../../include/wavio/filelistenerf.cpp \
    ../../include/neuro/spikecorrelator.cpp \
    ../../include/wave/wavefile.cpp \
    ../../include/geometry/doublept.cpp \
    ../../include/filter/fftfilterbank-fftwf.cpp \
    ../../include/wave/floatwave.cpp \
    ../../include/wave/sparsewave.cpp \
    ../../include/hearing/erbscaling.cpp \
    ../../include/wave/wavgroup.cpp \
    ../../include/wave/average.cpp \
    ../../include/filter/delayfilter.cpp \
    ../../include/localization/hamachercombination.cpp \
    ../../include/localization/poaplocalizer.cpp \
    ../../include/localization/poaplocalizergeneric.cpp \
    ../../include/localization/tabulartwodeebackprojection.cpp \
    ../../include/wave/wavecombination.cpp \
    ../../include/neuro/poapifier.cpp

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
LIBS += -lgomp
LIBS += -lfftw3 -lfftw3f
}


