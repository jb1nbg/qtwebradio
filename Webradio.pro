#-------------------------------------------------
#
# Project created by QtCreator 2016-11-27T18:58:58
#
#-------------------------------------------------

QT       += core gui network bluetooth

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

target.path = /usr/bin
TARGET = Webradio
TEMPLATE = app

# The following define makes your compiler emit warnings if you use
# any feature of Qt which has been marked as deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

INSTALLS += target
SOURCES += main.cpp\
        mainwindow.cpp \
    webradiodatabase.cpp \
    webradiostream.cpp \
    webstreamlistitem.cpp \
    webradioapplication.cpp \
    sysplayer.cpp \
    playlist.cpp \
    mixertoggle.cpp \
    mixerslider.cpp \
    alsamixer.cpp \
    evdevlistener.cpp \
    evdevthread.cpp \
    volume.cpp \
    alsamixerwindow.cpp \
    clock.cpp \
    blctrl.cpp

HEADERS  += mainwindow.h \
    webradiodatabase.h \
    webradiostream.h \
    webstreamlistitem.h \
    webradioapplication.h \
    sysplayer.h \
    playlist.h \
    mixertoggle.h \
    mixerslider.h \
    alsamixer.h \
    evdevlistener.h \
    evdevthread.h \
    volume.h \
    favoriteuser.h \
    alsamixerwindow.h \
    clock.h \
    blctrl.h

FORMS    += \
    mainwindow.ui \
    clock.ui

DISTFILES += \
    style.css

RESOURCES += \
    webradio.qrc

INCLUDEPATH += /usr/include/libevdev-1.0/libevdev/ /usr/include/qtcomp/

LIBS    += -lasound -levdev -lqclicklabel
