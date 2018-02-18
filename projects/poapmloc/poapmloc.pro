#
#
#

TEMPLATE = app
TARGET = poapmloc

CONFIG += console warn_off

CONFIG -= warn_on
CONFIG += c++11

DEFINES += QT_CORE_LIB
DEFINES += USE_FFTW
win32 {
DEFINES += WIN32
}
DEFINES += M_PI=3.14159265358979323846

include(../config.pro)

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
    ../../include/neuro/poapifier.h \
    angulardetection.h \
    angulargaussian.h \
    localizedspectra.h \
    localizedspectrum.h \
    onlinemultilocalizer.h \
    stdafx.h \
    ../../../FFTW/fftw3.h \
    ../../../FFTW/fftw3.h

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
    ../../include/neuro/poapifier.cpp \
    localizedspectra.cpp \
    localizedspectrum.cpp \
    main.cpp \
    onlinemultilocalizer.cpp \
    stdafx.cpp

INCLUDEPATH += . ../../include




