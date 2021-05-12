#-------------------------------------------------
#
# Project created by QtCreator 2021-03-24T18:28:51
#
#-------------------------------------------------

QT       += core gui

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = OpenCL_test
TEMPLATE = app

RESOURCES += \
    app_resources.qrc

RC_ICONS = tng.ico

OPEN_CL_SDK = "C:/Program Files (x86)/AMD APP SDK/3.0"
OPEN_CL_SDK_INCLUDE = $${OPEN_CL_SDK}/include
OPEN_CL_SDK_LIB =     $${OPEN_CL_SDK}/lib/x86_64
OPEN_CL_SDK_BIN =     $${OPEN_CL_SDK}/bin/x86_64
INCLUDEPATH += $${OPEN_CL_SDK_INCLUDE}
DEPENDPATH  += $${OPEN_CL_SDK_INCLUDE}

SOURCES += main.cpp\
        mainwindow.cpp \
        qt_cpu_calc_float_midle_class.cpp \
        qt_cpu_calc_midle_class.cpp

HEADERS  += mainwindow.h \
    qt_cpu_calc_float_midle_class.h \
    qt_cpu_calc_midle_class.h

FORMS    += mainwindow.ui

# Ключи для mingw
#QMAKE_CXXFLAGS += -O3 -march=i686 -std=c++11 -minline-all-stringops
#QMAKE_CXXFLAGS += -O3 -march=i686 -std=c++11 -minline-all-stringops -mmmx -msse -msse2 -msse3 -mssse3
#QMAKE_CXXFLAGS += -Ofast -march=i686 -std=c++11 -minline-all-stringops -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2
#QMAKE_CXXFLAGS += -march=i686 -std=c++11 -minline-all-stringops -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2

QMAKE_CXXFLAGS += -Ofast -m64 -std=c++11 -minline-all-stringops -mmmx -msse -msse2 -msse3 -mssse3 -msse4.1 -msse4.2

#QMAKE_CXXFLAGS += -Ofast -march=i686 -std=c++11 -minline-all-stringops
#QMAKE_CXXFLAGS_DEBUG += -pg
#QMAKE_LFLAGS_DEBUG += -pg
#QMAKE_LFLAGS += -pg
#QMAKE_LFLAGS_DEBUG += -pg -LC:\\mingw491\\mingw32\\lib
win32{
LIBS    += "-L$${OPEN_CL_SDK_LIB}"\
           "-L$${OPEN_CL_SDK_BIN}"\
	    -lOpenCL
#LIBS    += "C:\Program Files (x86)\AMD APP SDK\3.0\lib\x86_64\OpenCL.lib"
}
#QMAKE_LFLAGS
#RC_FILE
#CONFIG += O3
#CONFIG += O3 std=c++11 mmx sse sse2 sse3 ssse3

linux{
}

#DISTFILES += \
#    libOpenCL.a
