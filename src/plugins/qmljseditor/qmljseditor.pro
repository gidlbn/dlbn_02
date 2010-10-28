TEMPLATE = lib
TARGET = QmlJSEditor
include(../../qtcreatorplugin.pri)
include(qmljseditor_dependencies.pri)

DEFINES += \
    QMLJSEDITOR_LIBRARY \
    QT_CREATOR

HEADERS += \
    qmljscodecompletion.h \
    qmljseditor.h \
    qmljseditor_global.h \
    qmljseditoractionhandler.h \
    qmljseditorconstants.h \
    qmljseditorfactory.h \
    qmljseditorplugin.h \
    qmlexpressionundercursor.h \
    qmlfilewizard.h \
    qmljshighlighter.h \
    qmljshoverhandler.h \
    qmljsmodelmanager.h \
    qmljsmodelmanagerinterface.h

SOURCES += \
    qmljscodecompletion.cpp \
    qmljseditor.cpp \
    qmljseditoractionhandler.cpp \
    qmljseditorfactory.cpp \
    qmljseditorplugin.cpp \
    qmlexpressionundercursor.cpp \
    qmlfilewizard.cpp \
    qmljshighlighter.cpp \
    qmljshoverhandler.cpp \
    qmljsmodelmanager.cpp \
    qmljsmodelmanagerinterface.cpp

RESOURCES += qmljseditor.qrc
OTHER_FILES += QmlJSEditor.pluginspec QmlJSEditor.mimetypes.xml
