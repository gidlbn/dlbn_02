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

#include "qmlmodelview.h"
#include "qmlobjectnode.h"
#include "qmlitemnode.h"
#include "itemlibraryinfo.h"
#include "modelutilities.h"
#include "mathutils.h"
#include "invalididexception.h"
#include <QDir>
#include <QFileInfo>
#include <QDebug>
#include <QMessageBox>

enum {
    debug = false
};


namespace QmlDesigner {

QmlModelView::QmlModelView(QObject *parent)
    : ForwardView<NodeInstanceView>(parent)
{
    NodeInstanceView *nodeInstanceView = new NodeInstanceView(this);
    nodeInstanceView->setQmlModelView(this);
    appendView(nodeInstanceView);
}

void QmlModelView::setCurrentState(const QmlModelState &state)
{
    if (!state.isValid())
        return;

    emitCustomNotification("__state changed__", QList<ModelNode>() << state.modelNode());
}

QmlModelState QmlModelView::currentState() const
{
    return m_state;
}

QmlModelState QmlModelView::baseState() const
{
    return QmlModelState::createBaseState(this);
}

QmlObjectNode QmlModelView::createQmlObjectNode(const QString &typeString,
                     int majorVersion,
                     int minorVersion,
                     const PropertyListType &propertyList)
{
    return QmlObjectNode(createModelNode(typeString, majorVersion, minorVersion, propertyList));
}

QmlItemNode QmlModelView::createQmlItemNode(const QString &typeString,
                                int majorVersion,
                                int minorVersion,
                                const PropertyListType &propertyList)
{
    return createQmlObjectNode(typeString, majorVersion, minorVersion, propertyList).toQmlItemNode();
}

QmlItemNode QmlModelView::createQmlItemNodeFromImage(const QString &imageName, const QPointF &position, QmlItemNode parentNode)
{
    if (!parentNode.isValid())
        parentNode = rootQmlItemNode();

    if (!parentNode.isValid())
        return QmlItemNode();

    QmlItemNode newNode;
    RewriterTransaction transaction = beginRewriterTransaction();
    {
        const QString newImportUrl = QLatin1String("Qt");
        const QString newImportVersion = QLatin1String("4.7");
        Import newImport = Import::createLibraryImport(newImportUrl, newImportVersion);

        foreach (const Import &import, model()->imports()) {
            if (import.isLibraryImport()
                && import.url() == newImport.url()
                && import.version() == newImport.version()) {
                // reuse this import
                newImport = import;
                break;
            }
        }

        if (!model()->imports().contains(newImport)) {
            model()->addImport(newImport);
        }

        QList<QPair<QString, QVariant> > propertyPairList;
        propertyPairList.append(qMakePair(QString("x"), QVariant( round(position.x(), 4))));
        propertyPairList.append(qMakePair(QString("y"), QVariant( round(position.y(), 4))));

        QString relativeImageName = imageName;

        //use relative path
        if (QFileInfo(model()->fileUrl().toLocalFile()).exists()) {
            QDir fileDir(QFileInfo(model()->fileUrl().toLocalFile()).absolutePath());
            relativeImageName = fileDir.relativeFilePath(imageName);
        }

        propertyPairList.append(qMakePair(QString("source"), QVariant(relativeImageName)));
        newNode = createQmlItemNode("Qt/Image", 4, 7, propertyPairList);
        parentNode.nodeAbstractProperty("data").reparentHere(newNode);

        Q_ASSERT(newNode.isValid());

        QString id;
        int i = 1;
        QString name("image");
        name.remove(QLatin1Char(' '));
        do {
            id = name + QString::number(i);
            i++;
        } while (hasId(id)); //If the name already exists count upwards

        newNode.setId(id);
        if (!currentState().isBaseState()) {
            newNode.modelNode().variantProperty("opacity") = 0;
            newNode.setVariantProperty("opacity", 1);
        }

        Q_ASSERT(newNode.isValid());
    }

    Q_ASSERT(newNode.isValid());

    return newNode;
}

QmlItemNode QmlModelView::createQmlItemNode(const ItemLibraryEntry &itemLibraryEntry, const QPointF &position, QmlItemNode parentNode)
{
    if (!parentNode.isValid())
        parentNode = rootQmlItemNode();

    Q_ASSERT(parentNode.isValid());

    QmlItemNode newNode;
    RewriterTransaction transaction = beginRewriterTransaction();
    {
        if (itemLibraryEntry.typeName().contains('/')) {
            const QString newImportUrl = itemLibraryEntry.typeName().split('/').first();
            const QString newImportVersion = QString("%1.%2").arg(QString::number(itemLibraryEntry.majorVersion()), QString::number(itemLibraryEntry.minorVersion()));
            Import newImport = Import::createLibraryImport(newImportUrl, newImportVersion);

            foreach (const Import &import, model()->imports()) {
                if (import.isLibraryImport()
                    && import.url() == newImport.url()
                    && import.version() == newImport.version()) {
                    // reuse this import
                    newImport = import;
                    break;
                }
            }

            if (!model()->imports().contains(newImport)) {
                model()->addImport(newImport);
            }
        }

        QList<QPair<QString, QVariant> > propertyPairList;
        propertyPairList.append(qMakePair(QString("x"), QVariant(round(position.x(), 4))));
        propertyPairList.append(qMakePair(QString("y"), QVariant(round(position.y(), 4))));

        foreach (const PropertyContainer &property, itemLibraryEntry.properties())
            propertyPairList.append(qMakePair(property.name(), property.value()));

        newNode = createQmlItemNode(itemLibraryEntry.typeName(), itemLibraryEntry.majorVersion(), itemLibraryEntry.minorVersion(), propertyPairList);
        parentNode.nodeAbstractProperty("data").reparentHere(newNode);

        Q_ASSERT(newNode.isValid());

        QString id;
        int i = 1;
        QString name(itemLibraryEntry.name().toLower());
        //remove forbidden characters
        name.replace(QRegExp(QLatin1String("[^a-zA-Z0-9_]")), QLatin1String("_"));
        do {
            id = name + QString::number(i);
            i++;
        } while (hasId(id)); //If the name already exists count upwards

        try {
            newNode.setId(id);
        } catch (InvalidIdException &e) {
            // should never happen
            QMessageBox::warning(0, tr("Invalid Id"), e.description());
        }

        if (!currentState().isBaseState()) {
            newNode.modelNode().variantProperty("opacity") = 0;
            newNode.setVariantProperty("opacity", 1);
        }

        Q_ASSERT(newNode.isValid());
    }

    Q_ASSERT(newNode.isValid());

    return newNode;
}

QmlObjectNode QmlModelView::rootQmlObjectNode() const
{
    return QmlObjectNode(rootModelNode());
}

QmlItemNode QmlModelView::rootQmlItemNode() const
{
    return rootQmlObjectNode().toQmlItemNode();
}

void QmlModelView::setSelectedQmlObjectNodes(const QList<QmlObjectNode> &selectedNodeList)
{
    setSelectedModelNodes(QmlDesigner::toModelNodeList(selectedNodeList));
}

void QmlModelView::selectQmlObjectNode(const QmlObjectNode &node)
{
     selectModelNode(node.modelNode());
}

void QmlModelView::deselectQmlObjectNode(const QmlObjectNode &node)
{
    deselectModelNode(node.modelNode());
}

QList<QmlObjectNode> QmlModelView::selectedQmlObjectNodes() const
{
    return toQmlObjectNodeList(selectedModelNodes());
}

QList<QmlItemNode> QmlModelView::selectedQmlItemNodes() const
{
    return toQmlItemNodeList(selectedModelNodes());
}

void QmlModelView::setSelectedQmlItemNodes(const QList<QmlItemNode> &selectedNodeList)
{
    return setSelectedModelNodes(QmlDesigner::toModelNodeList(selectedNodeList));
}

QmlObjectNode QmlModelView::fxObjectNodeForId(const QString &id)
{
    return QmlObjectNode(modelNodeForId(id));
}

void QmlModelView::customNotification(const AbstractView * /* view */, const QString &identifier, const QList<ModelNode> &nodeList, const QList<QVariant> & /* data */)
{
    if (debug)
        qDebug() << this << __FUNCTION__ << identifier << nodeList;

    if (identifier == "__state changed__") {
        QmlModelState state(nodeList.first());
        if (state.isValid()) {
            activateState(state);
        } else {
            activateState(baseState());
        }
    }
}

NodeInstance QmlModelView::instanceForModelNode(const ModelNode &modelNode)
{
    return nodeInstanceView()->instanceForNode(modelNode);
}

bool QmlModelView::hasInstanceForModelNode(const ModelNode &modelNode)
{
    return nodeInstanceView()->hasInstanceForNode(modelNode);
}

NodeInstanceView *QmlModelView::nodeInstanceView() const
{
    return firstView();
}

void QmlModelView::modelAttached(Model *model)
{
    m_state = QmlModelState();
    ForwardView<NodeInstanceView>::modelAttached(model);
    m_state = baseState();
    Q_ASSERT(m_state.isBaseState());
}

void QmlModelView::modelAboutToBeDetached(Model *model)
{
    ForwardView<NodeInstanceView>::modelAboutToBeDetached(model);
    m_state = QmlModelState();
}

static bool isTransformProperty(const QString &name)
{
    static QStringList transformProperties(QStringList() << "x"
                                                         << "y"
                                                         << "width"
                                                         << "height"
                                                         << "z"
                                                         << "rotation"
                                                         << "scale"
                                                         << "transformOrigin");

    return transformProperties.contains(name);
}

void QmlModelView::nodeInstancePropertyChanged(const ModelNode &node, const QString &propertyName)
{
    if (debug)
        qDebug() << this << __FUNCTION__ << node << propertyName;

    QmlObjectNode qmlObjectNode(node);

    if (!qmlObjectNode.isValid())
        return;

    if (isTransformProperty(propertyName))
        transformChanged(qmlObjectNode, propertyName);
    else if (propertyName == "parent")
        parentChanged(qmlObjectNode);
    else if (propertyName == "state")
        changeToState(node, qmlObjectNode.instanceValue(propertyName).toString());
    else
        otherPropertyChanged(qmlObjectNode, propertyName);
}

void QmlModelView::activateState(const QmlModelState &state)
{
    if (debug)
        qDebug() << this << __FUNCTION__ << state;

    if (!state.isValid())
        return;

    if (m_state == state)
        return;

    QmlModelState oldState = m_state;

    NodeInstance newStateInstance = instanceForModelNode(state.modelNode());

    if (state.isBaseState()) {
        nodeInstanceView()->activateBaseState();
    } else {
        nodeInstanceView()->activateState(newStateInstance);
    }
}

void QmlModelView::changeToState(const ModelNode &node, const QString &stateName)
{
    if (debug)
        qDebug() << this << __FUNCTION__ << node << stateName;

    QmlItemNode itemNode(node);

    QmlModelState newState;
    if (stateName.isEmpty())
        newState = baseState();
    else
        newState = itemNode.states().state(stateName);

    QmlModelState oldState = m_state;

    if (newState.isValid() && oldState != newState) {
        m_state = newState;
        stateChanged(newState, oldState);
    }
}


void QmlModelView::transformChanged(const QmlObjectNode &/*qmlObjectNode*/, const QString &/*propertyName*/)
{
}

void QmlModelView::parentChanged(const QmlObjectNode &/*qmlObjectNode*/)
{
}

void QmlModelView::otherPropertyChanged(const QmlObjectNode &/*qmlObjectNode*/, const QString &/*propertyName*/)
{
}

void  QmlModelView::stateChanged(const QmlModelState &newQmlModelState, const QmlModelState &oldQmlModelState)
{
    if (debug)
        qDebug() << this << __FUNCTION__ << oldQmlModelState << "to" << newQmlModelState;
}

} //QmlDesigner
