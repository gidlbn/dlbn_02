include($$PWD/filemanager/filemanager.pri)
include (../config.pri)

QT += script \
    declarative

DEFINES += TEST_EXPORTS
INCLUDEPATH += $$PWD \
    $$PWD/include
DEPENDPATH += $$PWD $$PWD/include
SOURCES += $$PWD/model/abstractview.cpp \
    $$PWD/instances/nodeinstanceview.cpp \
    $$PWD/model/rewriterview.cpp \
    $$PWD/metainfo/enumeratormetainfo.cpp \
    $$PWD/metainfo/metainfo.cpp \
    $$PWD/metainfo/metainfoparser.cpp \
    $$PWD/metainfo/propertymetainfo.cpp \
    $$PWD/metainfo/nodemetainfo.cpp \
    $$PWD/metainfo/itemlibraryinfo.cpp \
    $$PWD/metainfo/subcomponentmanager.cpp \
    $$PWD/model/internalproperty.cpp \
    $$PWD/model/model.cpp \
    $$PWD/model/modelnode.cpp \
    $$PWD/model/painteventfilter.cpp \
    $$PWD/model/internalnode.cpp \
    $$PWD/model/propertyparser.cpp \
    $$PWD/model/propertycontainer.cpp \
    $$PWD/pluginmanager/widgetpluginmanager.cpp \
    $$PWD/pluginmanager/widgetpluginpath.cpp \
    $$PWD/instances/nodeinstance.cpp \
    $$PWD/instances/objectnodeinstance.cpp \
    $$PWD/instances/widgetnodeinstance.cpp \
    $$PWD/instances/graphicswidgetnodeinstance.cpp \
    $$PWD/instances/qmlgraphicsitemnodeinstance.cpp \
    $$PWD/instances/graphicsscenenodeinstance.cpp \
    $$PWD/instances/graphicsviewnodeinstance.cpp \
    $$PWD/instances/proxywidgetnodeinstance.cpp \
    $$PWD/instances/qmlviewnodeinstance.cpp \
    $$PWD/instances/dummynodeinstance.cpp \
    $$PWD/instances/qmlpropertychangesnodeinstance.cpp \
    $$PWD/instances/qmlstatenodeinstance.cpp \
    $$PWD/exceptions/exception.cpp \
    $$PWD/exceptions/invalidnodeinstanceexception.cpp \
    $$PWD/exceptions/invalidpropertyexception.cpp \
    $$PWD/exceptions/invalidmodelnodeexception.cpp \
    $$PWD/exceptions/invalidreparentingexception.cpp \
    $$PWD/exceptions/invalidmetainfoexception.cpp \
    $$PWD/exceptions/invalidargumentexception.cpp \
    $$PWD/exceptions/noanchoringpossibleexception.cpp \
    $$PWD/exceptions/notimplementedexception.cpp \
    $$PWD/exceptions/invalidnodestateexception.cpp \
    $$PWD/model/variantparser.cpp \
    $$PWD/exceptions/invalidmodelstateexception.cpp \
    $$PWD/exceptions/removebasestateexception.cpp \
    $$PWD/exceptions/invalididexception.cpp \
    $$PWD/model/propertynode.cpp \
    $$PWD/exceptions/invalidslideindexexception.cpp \
    $$PWD/instances/graphicsobjectnodeinstance.cpp \
    $$PWD/model/import.cpp \
    $$PWD/instances/componentnodeinstance.cpp \
    $$PWD/exceptions/invalidqmlsourceexception.cpp \
    $$PWD/model/viewlogger.cpp \
    $$PWD/model/internalvariantproperty.cpp \
    $$PWD/model/internalnodelistproperty.cpp \
    $$PWD/model/variantproperty.cpp \
    $$PWD/model/nodelistproperty.cpp \
    $$PWD/model/abstractproperty.cpp \
    $$PWD/model/internalbindingproperty.cpp \
    $$PWD/model/bindingproperty.cpp \
    $$PWD/model/internalnodeproperty.cpp \
    $$PWD/model/internalnodeabstractproperty.cpp \
    $$PWD/model/nodeabstractproperty.cpp \
    $$PWD/model/nodeproperty.cpp \
    $$PWD/model/modeltotextmerger.cpp \
    $$PWD/model/texttomodelmerger.cpp \
    $$PWD/model/plaintexteditmodifier.cpp \
    $$PWD/model/componenttextmodifier.cpp \
    $$PWD/model/textmodifier.cpp \
    $$PWD/model/qmlmodelview.cpp \
    $$PWD/model/qmlitemnode.cpp \
    $$PWD/model/qmlstate.cpp \
    $$PWD/model/qmlchangeset.cpp \
    $$PWD/model/qmlmodelnodefacade.cpp \
    $$PWD/model/qmlobjectnode.cpp \
    $$PWD/model/qmlanchors.cpp \
    $$PWD/rewritertransaction.cpp \
    $$PWD/model/rewriteaction.cpp \
    $$PWD/model/modelnodepositionstorage.cpp \
    $$PWD/model/modelnodepositionrecalculator.cpp \
    $$PWD/model/rewriteactioncompressor.cpp \
    $$PWD/model/qmltextgenerator.cpp \
    $$PWD/model/modelmerger.cpp \
    $$PWD/instances/qmltransitionnodeinstance.cpp \
    $$PWD/exceptions/rewritingexception.cpp \
    $$PWD/instances/nodeinstancemetaobject.cpp \
    $$PWD/instances/behaviornodeinstance.cpp \
    $$PWD/instances/nodeinstancesignalspy.cpp

HEADERS += $$PWD/include/corelib_global.h \
    $$PWD/include/abstractview.h \
    $$PWD/include/nodeinstanceview.h \
    $$PWD/include/rewriterview.h \
    $$PWD/include/enumeratormetainfo.h \
    $$PWD/include/metainfo.h \
    $$PWD/include/metainfoparser.h \
    $$PWD/include/nodemetainfo.h \
    $$PWD/include/propertymetainfo.h \
    $$PWD/include/itemlibraryinfo.h \
    $$PWD/model/internalproperty.h \
    $$PWD/include/modelnode.h \
    $$PWD/include/model.h \
    $$PWD/include/nodeproperty.h \
    $$PWD/include/widgetqueryview.h \
    $$PWD/include/subcomponentmanager.h \
    $$PWD/include/propertycontainer.h \
    $$PWD/model/internalnode_p.h \
    $$PWD/model/model_p.h \
    $$PWD/model/painteventfilter_p.h \
    $$PWD/model/propertyparser.h \
    $$PWD/pluginmanager/widgetpluginmanager.h \
    $$PWD/pluginmanager/widgetpluginpath.h \
    $$PWD/include/nodeinstance.h \
    $$PWD/instances/objectnodeinstance.h \
    $$PWD/instances/widgetnodeinstance.h \
    $$PWD/instances/graphicswidgetnodeinstance.h \
    $$PWD/instances/qmlgraphicsitemnodeinstance.h \
    $$PWD/instances/graphicsscenenodeinstance.h \
    $$PWD/instances/graphicsviewnodeinstance.h \
    $$PWD/instances/proxywidgetnodeinstance.h \
    $$PWD/instances/qmlviewnodeinstance.h \
    $$PWD/instances/dummynodeinstance.h \
    $$PWD/instances/qmlpropertychangesnodeinstance.h \
    $$PWD/instances/qmlstatenodeinstance.h \
    $$PWD/include/exception.h \
    $$PWD/include/invalidnodeinstanceexception.h \
    $$PWD/include/invalidmodelnodeexception.h \
    $$PWD/include/invalidreparentingexception.h \
    $$PWD/include/invalidmetainfoexception.h \
    $$PWD/include/invalidargumentexception.h \
    $$PWD/include/notimplementedexception.h \
    $$PWD/include/invalidpropertyexception.h \
    $$PWD/include/invalidmodelstateexception.h \
    $$PWD/include/removebasestateexception.h \
    $$PWD/include/invalididexception.h \
    $$PWD/model/variantparser.h \
    $$PWD/include/propertynode.h \
    $$PWD/include/invalidslideindexexception.h \
    $$PWD/instances/graphicsobjectnodeinstance.h \
    $$PWD/include/import.h \
    $$PWD/instances/componentnodeinstance.h \
    $$PWD/include/invalidqmlsourceexception.h \
    $$PWD/model/viewlogger.h \
    $$PWD/model/internalvariantproperty.h \
    $$PWD/model/internalnodelistproperty.h \
    $$PWD/include/variantproperty.h \
    $$PWD/include/nodelistproperty.h \
    $$PWD/include/abstractproperty.h \
    $$PWD/model/internalbindingproperty.h \
    $$PWD/include/bindingproperty.h \
    $$PWD/model/internalnodeproperty.h \
    $$PWD/model/internalnodeabstractproperty.h \
    $$PWD/include/nodeabstractproperty.h \
    $$PWD/include/plaintexteditmodifier.h \
    $$PWD/include/basetexteditmodifier.h \
    $$PWD/include/componenttextmodifier.h \
    $$PWD/include/textmodifier.h \
    $$PWD/model/modeltotextmerger.h \
    $$PWD/model/texttomodelmerger.h \
    $$PWD/include/qmlmodelview.h \
    $$PWD/include/qmlitemnode.h \
    $$PWD/include/qmlstate.h \
    $$PWD/include/qmlchangeset.h \
    $$PWD/include/qmlmodelnodefacade.h \
    $$PWD/include/forwardview.h \
    $$PWD/include/qmlobjectnode.h \
    $$PWD/include/qmlanchors.h \
    $$PWD/rewritertransaction.h \
    $$PWD/model/rewriteaction.h \
    $$PWD/include/modelnodepositionstorage.h \
    $$PWD/model/modelnodepositionrecalculator.h \
    $$PWD/model/rewriteactioncompressor.h \
    $$PWD/model/qmltextgenerator.h \
    $$PWD/include/modelmerger.h \
    $$PWD/include/mathutils.h \
    $$PWD/instances/qmltransitionnodeinstance.h \
    $$PWD/include/customnotifications.h \
    $$PWD/include/rewritingexception.h \
    $$PWD/instances/nodeinstancemetaobject.h \
    $$PWD/instances/behaviornodeinstance.h \
    $$PWD/instances/nodeinstancesignalspy.h

contains(CONFIG, plugin) {
  # If core.pri has been included in the qmldesigner plugin
  SOURCES += $$PWD/model/basetexteditmodifier.cpp
  HEADERS += $$PWD/include/basetexteditmodifier.h
}

DISTFILES += $$PWD/metafile/widget.metafile
RESOURCES += $$PWD/designercore.qrc
OTHER_FILES += $$PWD/metainfo/gui.metainfo
