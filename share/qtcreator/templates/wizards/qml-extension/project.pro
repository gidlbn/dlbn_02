TEMPLATE = lib
TARGET  = %ProjectName%
QT += declarative
CONFIG += qt plugin

TARGET = $$qtLibraryTarget($$TARGET)

# Input
SOURCES += \
    %ProjectName%.cpp \
    %ObjectName%.cpp

OTHER_FILES=qmldir

HEADERS += \
    %ProjectName%.h \
    %ObjectName%.h
