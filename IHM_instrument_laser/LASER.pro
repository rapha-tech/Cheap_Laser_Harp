QT += core gui widgets

CONFIG += c++17

SOURCES += \
    configFile.cpp \
    main.cpp \
    MainWindow.cpp \
    EngineLaser.cpp \
    lib/RtMidi.cpp \
    lib/yyjson.c

HEADERS += \
    MainWindow.h \
    EngineLaser.h \
    configFile.h \
    lib/RtMidi.h \
    lib/tsf.h \
    lib/miniaudio.h \
    lib/yyjson.h

RC_FILE = app.rc

unix:!macx {
    DEFINES += __LINUX_ALSA__
    LIBS += -lasound -lpthread
}

win32 {
    DEFINES += __WINDOWS_MM__
    LIBS += -lwinmm
}

macx {
    DEFINES += __MACOSX_CORE__
    LIBS += -framework CoreMIDI -framework CoreAudio -framework CoreFoundation
}

RESOURCES += \
    ressources.qrc

FORMS += \
    mainwindow.ui

DISTFILES += \
    app.rc
