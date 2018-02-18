# configure this according to your installation

win32 {
FFTWPATH=C:\Users\pli\Documents\checkouts\FFTW
}

# -----------------------------------------------------------

# output 
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

# -----------------------------------------------------------

# libraries

win32 {
message(Building for Windows with $${FFTWPATH})
INCLUDEPATH += $${FFTWPATH}
LIBS += -L$${FFTWPATH}
LIBS += -llibfftw3-3 -llibfftw3f-3
} 
unix {
message("Building for unix. relax.")
LIBS += -lgomp
LIBS += -lfftw3 -lfftw3f
}

# adapt the follwing where necessary
#MAC_INCLUDE =  /Users/pli/homebrew/opt/gcc-fftw3/include
#MAC_LIBS = -L/Users/pli/homebrew/opt/gcc-fftw3/lib
# macx {
# INCLUDEPATH += $${MAC_INCLUDE}
# LIBS += $${MAC_LIBS}
# }

# -----------------------------------------------------------

# complier flags

win32 {
QMAKE_CXXFLAGS_RELEASE += /openmp /O2 /Ot /fp:fast
} else {
QMAKE_CXXFLAGS += -std=c++11
QMAKE_CXXFLAGS += -g
QMAKE_CXXFLAGS_RELEASE += -O4 -finline-functions -fopenmp -ffast-math -march=corei7
}
macx {
QMAKE_MAC_SDK = macosx10.12
}
