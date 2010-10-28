!isEmpty(SUPPORT_QT_S60) {
    DEFINES += QTCREATOR_WITH_S60
}
SOURCES += $$PWD/s60devices.cpp \
    $$PWD/s60devicespreferencepane.cpp \
    $$PWD/s60manager.cpp \
    $$PWD/winscwtoolchain.cpp \
    $$PWD/gccetoolchain.cpp \
    $$PWD/s60emulatorrunconfiguration.cpp \
    $$PWD/s60devicerunconfiguration.cpp \
    $$PWD/s60devicerunconfigurationwidget.cpp \
    $$PWD/s60projectchecker.cpp \
    $$PWD/rvcttoolchain.cpp \
    $$PWD/s60runconfigbluetoothstarter.cpp \
    $$PWD/abldparser.cpp \
    $$PWD/rvctparser.cpp \
    $$PWD/winscwparser.cpp \
    qt-s60/s60createpackagestep.cpp
HEADERS += $$PWD/s60devices.h \
    $$PWD/s60devicespreferencepane.h \
    $$PWD/s60manager.h \
    $$PWD/winscwtoolchain.h \
    $$PWD/gccetoolchain.h \
    $$PWD/s60emulatorrunconfiguration.h \
    $$PWD/s60devicerunconfiguration.h \
    $$PWD/s60devicerunconfigurationwidget.h \
    $$PWD/s60projectchecker.h \
    $$PWD/rvcttoolchain.h \
    $$PWD/s60runconfigbluetoothstarter.h \
    $$PWD/abldparser.h \
    $$PWD/rvctparser.h \
    $$PWD/winscwparser.h \
    qt-s60/s60createpackagestep.h
FORMS += $$PWD/s60devicespreferencepane.ui \
    qt-s60/s60createpackagestep.ui
