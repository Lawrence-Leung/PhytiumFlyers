TEMPLATE=app
TARGET=Car
INCLUDEPATH+=.

QT       += core gui multimedia network concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++11

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
INCLUDEPATH += /usr/local/include/opencv4
LIBS += -L/home/kdr2/programs/opencv/lib -lopencv_core -lopencv_imgproc -lopencv_imgcodecs -lopencv_video -lopencv_videoio
PKGCONFIG += opencv4
QMAKE_CXXFLAGS += $$system(pkg-config --cflags opencv4)
LIBS += $$system(pkg-config --libs opencv4)
SOURCES += \
    camera.cpp \
    detection.cpp \
    device_thread.cpp \
    form.cpp \
    main.cpp \
    mainwindow.cpp

HEADERS += \
    camera.h \
    device_thread.h \
    form.h \
    mainwindow.h

FORMS += \
    form.ui \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
