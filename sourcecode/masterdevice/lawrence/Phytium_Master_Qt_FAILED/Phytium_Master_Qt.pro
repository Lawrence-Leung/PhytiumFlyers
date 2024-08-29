QT       += core gui multimedia network concurrent

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

# Add OpenCV Support
######################################### Edited by Lawrence Leung 2024
INCLUDEPATH += /usr/local/include/ \
               /usr/local/include/opencv4 \     # OpenCV4 support
               /home/lawrence/programs/onnxruntime-linux-x64-1.17.1/include  # ONNX Runtime support
######################################### Edited by Lawrence Leung 2024


######################################### Edited by Lawrence Leung 2024
LIBS += /usr/local/lib/libopencv_highgui.so \
        /usr/local/lib/libopencv_core.so    \
        /usr/local/lib/libopencv_imgproc.so \
        /usr/local/lib/libopencv_imgcodecs.so \ # The four items above are basic OpenCV modules for use
        /usr/local/lib/libopencv_dnn.so \       # Solved undefined reference to DNN
        /usr/local/lib/libopencv_videoio.so \   # Solved undefined reference to Video IO components
        /usr/local/lib/libonnxruntime.so \      # ONNX Runtime support
        /usr/local/lib/libonnxruntime.so.1.17.1
        # ONNX Runtime Library Support
######################################### Edited by Lawrence Leung 2024

PKGCONFIG += opencv4

# You can make your code fail to compile if it uses deprecated APIs.
# In order to do so, uncomment the following line.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += \
    camera.cpp \
    main.cpp \
    mainwindow.cpp \
    ultrafastlane.cpp

HEADERS += \
    camera.h \
    mainwindow.h \
    ultrafastlane.h

FORMS += \
    mainwindow.ui

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target

DISTFILES += \
    ultrafastLaneDetector.py
