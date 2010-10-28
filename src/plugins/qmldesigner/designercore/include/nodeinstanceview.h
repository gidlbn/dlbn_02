/**************************************************************************
**
** This file is part of Qt Creator
**
** Copyright (c) 2010 Nokia Corporation and/or its subsidiary(-ies).
**
** Contact: Nokia Corporation (qt-info@nokia.com)
**
** Commercial Usage
**
** Licensees holding valid Qt Commercial licenses may use this file in
** accordance with the Qt Commercial License Agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Nokia.
**
** GNU Lesser General Public License Usage
**
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** If you are unsure which license is appropriate for your use, please
** contact the sales department at http://qt.nokia.com/contact.
**
**************************************************************************/

#ifndef NODEINSTANCEVIEW_H
#define NODEINSTANCEVIEW_H

#include "corelib_global.h"
#include "abstractview.h"

#include <modelnode.h>
#include <nodeinstance.h>

#include <QHash>
#include <QWeakPointer>

QT_BEGIN_NAMESPACE
class QDeclarativeEngine;
class QGraphicsView;
class QFileSystemWatcher;
QT_END_NAMESPACE


namespace QmlDesigner {

namespace Internal {
    class ChildrenChangeEventFilter;
    class QmlStateNodeInstance;
}

class CORESHARED_EXPORT NodeInstanceView : public AbstractView
{
    Q_OBJECT

    friend class NodeInstance;
    friend class Internal::ObjectNodeInstance;
    friend class Internal::QmlStateNodeInstance;

public:
    typedef QWeakPointer<NodeInstanceView> Pointer;
    typedef QPair<QWeakPointer<QObject>, QString> ObjectPropertyPair;

    NodeInstanceView(QObject *parent = 0);
    ~NodeInstanceView();

    void modelAttached(Model *model);
    void modelAboutToBeDetached(Model *model);
    void nodeCreated(const ModelNode &createdNode);
    void nodeAboutToBeRemoved(const ModelNode &removedNode);
    void nodeRemoved(const ModelNode &removedNode, const NodeAbstractProperty &parentProperty, PropertyChangeFlags propertyChange);
    void propertiesAdded(const ModelNode &node, const QList<AbstractProperty>& propertyList);
    void propertiesAboutToBeRemoved(const QList<AbstractProperty>& propertyList);
    void propertiesRemoved(const QList<AbstractProperty>& propertyList);
    void variantPropertiesChanged(const QList<VariantProperty>& propertyList, PropertyChangeFlags propertyChange);
    void bindingPropertiesChanged(const QList<BindingProperty>& propertyList, PropertyChangeFlags propertyChange);
    void nodeReparented(const ModelNode &node, const NodeAbstractProperty &newPropertyParent, const NodeAbstractProperty &oldPropertyParent, AbstractView::PropertyChangeFlags propertyChange);
    void rootNodeTypeChanged(const QString &type, int majorVersion, int minorVersion);
    void fileUrlChanged(const QUrl &oldUrl, const QUrl &newUrl);
    void nodeIdChanged(const ModelNode& node, const QString& newId, const QString& oldId);
    void nodeOrderChanged(const NodeListProperty &listProperty, const ModelNode &movedNode, int oldIndex);
    void selectedNodesChanged(const QList<ModelNode> &selectedNodeList, const QList<ModelNode> &lastSelectedNodeList);
    void scriptFunctionsChanged(const ModelNode &node, const QStringList &scriptFunctionList);


    QList<NodeInstance> instances() const;
    NodeInstance instanceForNode(const ModelNode &node);
    bool hasInstanceForNode(const ModelNode &node);

    NodeInstance instanceForObject(QObject *object);
    bool hasInstanceForObject(QObject *object);

    void render(QPainter *painter, const QRectF &target=QRectF(), const QRectF &source=QRect(), Qt::AspectRatioMode aspectRatioMode=Qt::KeepAspectRatio);

    QRectF sceneRect() const;

    void notifyPropertyChange(const ModelNode &modelNode, const QString &propertyName);

    void setQmlModelView(QmlModelView *qmlModelView);
    QmlModelView *qmlModelView() const ;

    void setBlockStatePropertyChanges(bool block);

    NodeInstance activeStateInstance() const;

    void activateState(const NodeInstance &instance);
    void activateBaseState();

private slots:
    void emitParentChanged(QObject *child);
    void refreshLocalFileProperty(const QString &path);
    void removeIdFromContext(QObject *object);

private: // functions
    NodeInstance rootNodeInstance() const;

    NodeInstance loadNode(const ModelNode &rootNode, QObject *objectToBeWrapped = 0);
    void loadModel(Model *model);
    void loadNodes(const QList<ModelNode> &nodeList);

    void removeAllInstanceNodeRelationships();

    void removeRecursiveChildRelationship(const ModelNode &removedNode);

    void insertInstanceNodeRelationship(const ModelNode &node, const NodeInstance &instance);
    void removeInstanceNodeRelationship(const ModelNode &node);

    QDeclarativeEngine *engine();
    Internal::ChildrenChangeEventFilter *childrenChangeEventFilter();
    void removeInstanceAndSubInstances(const ModelNode &node);

    void setInstancePropertyVariant(const VariantProperty &property);
    void setInstancePropertyBinding(const BindingProperty &property);
    void resetInstanceProperty(const AbstractProperty &property);

    void setStateInstance(const NodeInstance &stateInstance);
    void clearStateInstance();

    QFileSystemWatcher *fileSystemWatcher();

    void addFilePropertyToFileSystemWatcher(QObject *object, const QString &propertyName, const QString &path);
    void removeFilePropertyFromFileSystemWatcher(QObject *object, const QString &propertyName, const QString &path);

private: //variables
    NodeInstance m_rootNodeInstance;
    NodeInstance m_activeStateInstance;
    QScopedPointer<QGraphicsView> m_graphicsView;

    QHash<ModelNode, NodeInstance> m_nodeInstanceHash;
    QHash<QObject*, NodeInstance> m_objectInstanceHash; // This is purely internal. Might contain dangling pointers!
    QMultiHash<QString, ObjectPropertyPair> m_fileSystemWatcherHash;
    QWeakPointer<QDeclarativeEngine> m_engine;
    QWeakPointer<Internal::ChildrenChangeEventFilter> m_childrenChangeEventFilter;

    QWeakPointer<QmlModelView> m_qmlModelView;

    QWeakPointer<QFileSystemWatcher> m_fileSystemWatcher;

    bool m_blockStatePropertyChanges;


};

} // namespace NodeInstanceView

#endif // NODEINSTANCEVIEW_H
