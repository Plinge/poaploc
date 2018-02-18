#-------------------------------------------------
#
# Project created by QtCreator 2015-09-17T20:29:47
#
#-------------------------------------------------

TARGET = poapify


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

include(../config.pro)

DEPENDPATH += .
MOC_DIR += ./tmp_moc
UI_DIR += ./tmp_ui
RCC_DIR += ./tmp_rcc

INCLUDEPATH += . ../../include

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
