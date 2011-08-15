SOURCES += main.cpp \
    boardscanner.cpp \
    mainwindow.cpp \
    camerahandler.cpp \
    utilities.cpp \
    fiducialdetector.cpp

HEADERS += \
    boardscanner.h \
    mainwindow.h \
    camerahandler.h \
    utilities.h \
    fiducialdetector.h \
    cvHomography.h

FORMS += \
    mainwindow.ui

INCLUDEPATH += "C:/OpenCV2.0/include/opencv"
LIBS += -L"C:\OpenCV2.0\lib" -lcv200 -lcxcore200 -lhighgui200

OTHER_FILES += \
    coordinates.txt
