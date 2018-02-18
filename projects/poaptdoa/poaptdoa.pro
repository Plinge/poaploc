#-------------------------------------------------
#
# Project created by QtCreator 2015-09-17T22:36:31
#
#-------------------------------------------------

TEMPLATE = app

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

SOURCES += main.cpp \
    ../../include/localization/poapcorrelator.cpp \
    ../../include/localization/poapcorrwave.cpp \
    ../../include/qtap/qcommandlinearguments.cpp \
    ../../include/wave/floatwave.cpp \
    ../../include/wave/sparsewave.cpp \
    ../../include/wave/wavefile.cpp \
    ../../include/neuro/poapifier.cpp \
    ../../include/neuro/spikecorrelator.cpp \
    ../../include/neuro/histogram.cpp \
    ../../include/filter/delayfilter.cpp \
    ../../include/filter/fftfilterbank-fftwf.cpp \
    ../../include/hearing/erbscaling.cpp

HEADERS += \
    ../../include/localization/poapcorrelator.h \
    ../../include/localization/poapcorrwave.h \
    ../../include/filter/fftfilterbank_impl.h
