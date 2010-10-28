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

#include "stateseditorview.h"
#include "stateseditormodel.h"
#include <customnotifications.h>

#include <QPainter>
#include <QTimerEvent>
#include <QDebug>
#include <math.h>

enum {
    debug = false
};

namespace QmlDesigner {
namespace Internal {

/**
  We always have 'one' current state, where we get updates from (see sceneChanged()). In case
  the current state is the base state, we render the base state + all other states.
  */
StatesEditorView::StatesEditorView(StatesEditorModel *editorModel, QObject *parent) :
        QmlModelView(parent),
        m_editorModel(editorModel),
        m_attachedToModel(false), m_settingSilentState(false)
{
    Q_ASSERT(m_editorModel);
}

void StatesEditorView::setCurrentStateSilent(int index)
{
    m_settingSilentState = true;
    if (debug)
        qDebug() << __FUNCTION__ << index;

    Q_ASSERT(index >= 0 && index < m_modelStates.count());

    // TODO
    QmlModelState state(m_modelStates.at(index));
    if (!state.isValid()) {
        m_settingSilentState = false;
        return;
    }
    if (state == currentState()) {
        m_settingSilentState = false;
        return;
    }
    QmlModelView::activateState(state);

    m_settingSilentState = false;
}

void StatesEditorView::setCurrentState(int index)
{
    if (debug)
        qDebug() << __FUNCTION__ << index;

    // happens to be the case for an invalid document / no base state
    if (m_modelStates.isEmpty())
        return;

    Q_ASSERT(index < m_modelStates.count());
    if (index == -1)
        return;

    if (m_modelStates.indexOf(currentState()) == index)
        return;

    QmlModelState state(m_modelStates.at(index));
    Q_ASSERT(state.isValid());
    QmlModelView::setCurrentState(state);
}

void StatesEditorView::createState(const QString &name)
{
    if (debug)
        qDebug() << __FUNCTION__ << name;

    stateRootNode().states().addState(name);
}

void StatesEditorView::removeState(int index)
{
    if (debug)
        qDebug() << __FUNCTION__ << index;

    Q_ASSERT(index > 0 && index < m_modelStates.size());
    QmlModelState state = m_modelStates.at(index);
    Q_ASSERT(state.isValid());

    setCurrentState(0);

    m_modelStates.removeAll(state);
    state.destroy();
    m_editorModel->removeState(index);

    int newIndex = (index < m_modelStates.count()) ? index : m_modelStates.count() - 1;
    setCurrentState(newIndex);
}

void StatesEditorView::renameState(int index, const QString &newName)
{
    if (debug)
        qDebug() << __FUNCTION__ << index << newName;

    Q_ASSERT(index > 0 && index < m_modelStates.size());
    QmlModelState state = m_modelStates.at(index);
    Q_ASSERT(state.isValid());
    if (state.name() != newName) {
        // Jump to base state for the change
        QmlModelState oldState = currentState();
        setCurrentStateSilent(0);
        state.setName(newName);
        setCurrentState(m_modelStates.indexOf(oldState));
    }
}

void StatesEditorView::duplicateCurrentState(int index)
{
    if (debug)
        qDebug() << __FUNCTION__ << index;

    Q_ASSERT(index > 0 && index < m_modelStates.size());

    QmlModelState state = m_modelStates.at(index);
    Q_ASSERT(state.isValid());
    QString newName = state.name();

    // Strip out numbers at the end of the string
    QRegExp regEx(QString("[0-9]+$"));
    int numberIndex = newName.indexOf(regEx);
    if ((numberIndex != -1) && (numberIndex+regEx.matchedLength()==newName.length()))
        newName = newName.left(numberIndex);

    int i = 1;
    QStringList stateNames = state.stateGroup().names();
    while (stateNames.contains(newName + QString::number(i)))
        i++;
    state.duplicate(newName + QString::number(i));
}

void StatesEditorView::modelAttached(Model *model)
{
    if (debug)
        qDebug() << __FUNCTION__;

    if (model == QmlModelView::model())
        return;

    Q_ASSERT(model);
    QmlModelView::modelAttached(model);
    clearModelStates();

    // Add base state
    if (!baseState().isValid())
        return;

    m_modelStates.insert(0, baseState());
    m_attachedToModel = true;
    m_editorModel->insertState(0, baseState().name());

    // Add custom states
    m_stateRootNode = QmlItemNode(rootModelNode());
    if (!m_stateRootNode.isValid())
        return;    

    for (int i = 0; i < m_stateRootNode.states().allStates().size(); ++i) {
        QmlModelState state = QmlItemNode(rootModelNode()).states().allStates().at(i);
        insertModelState(i, state);
    }

}

void StatesEditorView::modelAboutToBeDetached(Model *model)
{
    if (debug)
        qDebug() << __FUNCTION__;

    m_attachedToModel = false;

    clearModelStates();

    QmlModelView::modelAboutToBeDetached(model);
}

void StatesEditorView::propertiesAboutToBeRemoved(const QList<AbstractProperty> &propertyList)
{
    if (debug)
        qDebug() << __FUNCTION__;

    foreach (const AbstractProperty &property, propertyList) {
        // remove all states except base state
        if ((property.name()=="states") && (property.parentModelNode().isRootNode())) {
            foreach (const QmlModelState &state, m_modelStates) {
                if (!state.isBaseState())
                    removeModelState(state);
            }
        } else {
            ModelNode node (property.parentModelNode().parentProperty().parentModelNode());
            if (QmlModelState(node).isValid()) {
                startUpdateTimer(modelStateIndex(node) + 1, 0);
            } else { //a change to the base state update all
                for (int i = 0; i < m_modelStates.count(); ++i)
                    startUpdateTimer(i, 0);
            }
        }
    }
    QmlModelView::propertiesAboutToBeRemoved(propertyList);
}

void StatesEditorView::propertiesRemoved(const QList<AbstractProperty> &propertyList)
{
    if (debug)
        qDebug() << __FUNCTION__;

    QmlModelView::propertiesRemoved(propertyList);
}

void StatesEditorView::variantPropertiesChanged(const QList<VariantProperty> &propertyList, PropertyChangeFlags propertyChange)
{
    if (debug)
        qDebug() << __FUNCTION__;

    QmlModelView::variantPropertiesChanged(propertyList, propertyChange);
    foreach (const VariantProperty &property, propertyList) {
        ModelNode node (property.parentModelNode());
        if (QmlModelState(node).isValid() && (property.name() == QLatin1String("name"))) {
            int index = m_modelStates.indexOf(node);
            if (index != -1)
                m_editorModel->renameState(index, property.value().toString());
        }
    }
}

void StatesEditorView::nodeAboutToBeRemoved(const ModelNode &removedNode)
{
    if (debug)
        qDebug() << __FUNCTION__;

    if (removedNode.parentProperty().parentModelNode() == m_stateRootNode
          && QmlModelState(removedNode).isValid()) {
        removeModelState(removedNode);
    }

    QmlModelView::nodeAboutToBeRemoved(removedNode);

    if (QmlModelState(removedNode).isValid()) {
        startUpdateTimer(modelStateIndex(removedNode) + 1, 0);
    } else { //a change to the base state update all
        for (int i = 0; i < m_modelStates.count(); ++i)
            startUpdateTimer(i, 0);
    }
}


void StatesEditorView::nodeReparented(const ModelNode &node, const NodeAbstractProperty &newPropertyParent, const NodeAbstractProperty &oldPropertyParent, AbstractView::PropertyChangeFlags propertyChange)
{
    if (debug)
        qDebug() << __FUNCTION__;

    QmlModelView::nodeReparented(node, newPropertyParent, oldPropertyParent, propertyChange);

    // this would be sliding
    Q_ASSERT(newPropertyParent != oldPropertyParent);

    if (QmlModelState(node).isValid()) {
        if (oldPropertyParent.parentModelNode() == m_stateRootNode) {
            if (oldPropertyParent.isNodeListProperty()
                && oldPropertyParent.name() == "states") {
                removeModelState(node);
            } else {
                qWarning() << "States Editor: Reparented model state was not in states property list";
            }
        }

        if (newPropertyParent.parentModelNode() == m_stateRootNode) {
            if (newPropertyParent.isNodeListProperty()
                && newPropertyParent.name() == "states") {
                NodeListProperty statesProperty = newPropertyParent.toNodeListProperty();
                int index = statesProperty.toModelNodeList().indexOf(node);
                Q_ASSERT(index >= 0);
                insertModelState(index, node);
            } else {
                qWarning() << "States Editor: Reparented model state is not in the states property list";
            }
        }
    }
}

void StatesEditorView::nodeOrderChanged(const NodeListProperty &listProperty, const ModelNode &movedNode, int oldIndex)
{
    if (debug)
        qDebug() << __FUNCTION__;

    QmlModelView::nodeOrderChanged(listProperty, movedNode, oldIndex);
    if (listProperty.parentModelNode() == m_stateRootNode
        && listProperty.name() == "states") {

        int newIndex = listProperty.toModelNodeList().indexOf(movedNode);
        Q_ASSERT(newIndex >= 0);

        QmlModelState state = QmlModelState(movedNode);
        if (state.isValid()) {
            Q_ASSERT(oldIndex == modelStateIndex(state));
            removeModelState(state);
            insertModelState(newIndex, state);
            Q_ASSERT(newIndex == modelStateIndex(state));
        }
    }
}

void StatesEditorView::nodeInstancePropertyChanged(const ModelNode &node, const QString &propertyName)
{
    // sets currentState() used in sceneChanged
    QmlModelView::nodeInstancePropertyChanged(node, propertyName);

    if (!m_settingSilentState)
        sceneChanged();
}

void StatesEditorView::stateChanged(const QmlModelState &newQmlModelState, const QmlModelState &oldQmlModelState)
{
    if (debug)
        qDebug() << __FUNCTION__;

    QmlModelView::stateChanged(newQmlModelState, oldQmlModelState);

    if (!m_settingSilentState) {
        if (newQmlModelState.isBaseState())
            m_editorModel->emitChangedToState(0);
        else
            m_editorModel->emitChangedToState(m_modelStates.indexOf(newQmlModelState));
    }
}

void StatesEditorView::transformChanged(const QmlObjectNode &qmlObjectNode, const QString &propertyName)
{
    if (debug)
        qDebug() << __FUNCTION__;

    QmlModelView::transformChanged(qmlObjectNode, propertyName);
}

void StatesEditorView::parentChanged(const QmlObjectNode &qmlObjectNode)
{
    if (debug)
        qDebug() << __FUNCTION__;

    QmlModelView::parentChanged(qmlObjectNode);
}

void StatesEditorView::otherPropertyChanged(const QmlObjectNode &qmlObjectNode, const QString &propertyName)
{
    if (debug)
        qDebug() << __FUNCTION__;

    QmlModelView::otherPropertyChanged(qmlObjectNode, propertyName);
}


void StatesEditorView::customNotification(const AbstractView * view, const QString & identifier, const QList<ModelNode> & nodeList, const QList<QVariant> & data)
{
    if (debug)
        qDebug() << __FUNCTION__;

    QmlModelView::customNotification(view, identifier, nodeList, data);
}

void StatesEditorView::scriptFunctionsChanged(const ModelNode &node, const QStringList &scriptFunctionList)
{
    if (debug)
        qDebug() << __FUNCTION__;

    QmlModelView::scriptFunctionsChanged(node, scriptFunctionList);
}


QPixmap StatesEditorView::renderState(int i)
{
    if (debug)
        qDebug() << __FUNCTION__ << i;

    if (!m_attachedToModel)
        return QPixmap();

    Q_ASSERT(i >= 0 && i < m_modelStates.size());
    QmlModelState oldState = currentState();
    setCurrentStateSilent(i);

    Q_ASSERT(nodeInstanceView());

    const int checkerbordSize= 10;
    QPixmap tilePixmap(checkerbordSize * 2, checkerbordSize * 2);
    tilePixmap.fill(Qt::white);
    QPainter tilePainter(&tilePixmap);
    QColor color(220, 220, 220);
    tilePainter.fillRect(0, 0, checkerbordSize, checkerbordSize, color);
    tilePainter.fillRect(checkerbordSize, checkerbordSize, checkerbordSize, checkerbordSize, color);
    tilePainter.end();


    QSizeF pixmapSize(nodeInstanceView()->sceneRect().size());
    if (pixmapSize.width() > 100 || pixmapSize.height() > 100) // sensible maximum size
        pixmapSize.scale(QSize(100, 100), Qt::KeepAspectRatio);
    QSize cutSize(floor(pixmapSize.width()),floor(pixmapSize.height()));
    pixmapSize.setWidth(ceil(pixmapSize.width()));
    pixmapSize.setHeight(ceil(pixmapSize.height()));
    QPixmap pixmap(pixmapSize.toSize());

    QPainter painter(&pixmap);
    painter.drawTiledPixmap(pixmap.rect(), tilePixmap);
    nodeInstanceView()->render(&painter, pixmap.rect(), nodeInstanceView()->sceneRect());

    setCurrentStateSilent(m_modelStates.indexOf(oldState));

    Q_ASSERT(oldState == currentState());

    return pixmap.copy(0,0,cutSize.width(),cutSize.height());
}

void StatesEditorView::sceneChanged()
{
    if (debug)
        qDebug() << __FUNCTION__;

    // If we are in base state we have to update the pixmaps of all states,
    // otherwise only the pixmap for the current state

    // TODO: Since a switch to the base state also results in nodePropertyChanged and
    // therefore sceneChanged calls, we're rendering too much here

    if (currentState().isValid()) { //during setup we might get sceneChanged signals with an invalid currentState()
        if (currentState().isBaseState()) {
            for (int i = 0; i < m_modelStates.count(); ++i)
                startUpdateTimer(i, i * 80);
        } else {
            startUpdateTimer(modelStateIndex(currentState()) + 1, 0);
        }
    }
}

void StatesEditorView::startUpdateTimer(int i, int offset) {
    if (debug)
        qDebug() << __FUNCTION__ << i << offset;

    if (i < 0 || i >  m_modelStates.count())
        return;

    if (i < m_updateTimerIdList.size() && m_updateTimerIdList.at(i) != 0)
        return;
    // TODO: Add an offset so not all states are rendered at once


    if (i < m_updateTimerIdList.size() && i > 0)
        if (m_updateTimerIdList.at(i))
            killTimer(m_updateTimerIdList.at(i));
    int j = i;

    while (m_updateTimerIdList.size() <= i) {
        m_updateTimerIdList.insert(j, 0);
        j++;
    }
    m_updateTimerIdList[i] =  startTimer(100 + offset);
}

// index without base state
void StatesEditorView::insertModelState(int i, const QmlModelState &state)
{
    if (debug)
        qDebug() << __FUNCTION__ << i << state.name();

    Q_ASSERT(state.isValid());
    Q_ASSERT(!state.isBaseState());
    // For m_modelStates / m_editorModel, i=0 is base state
    m_modelStates.insert(i+1, state);
    m_editorModel->insertState(i+1, state.name());
}

void StatesEditorView::removeModelState(const QmlModelState &state)
{
    if (debug)
        qDebug() << __FUNCTION__ << state.name();

    Q_ASSERT(state.isValid());
    Q_ASSERT(!state.isBaseState());
    int index = m_modelStates.indexOf(state);
    if (index != -1) {
        m_modelStates.removeOne(state);

        if (m_updateTimerIdList.contains(index)) {
            killTimer(m_updateTimerIdList[index]);
            m_updateTimerIdList[index] = 0;
        }
        m_editorModel->removeState(index);
    }
}

void StatesEditorView::clearModelStates()
{
    if (debug)
        qDebug() << __FUNCTION__;


    // Remove all states
    const int modelStateCount = m_modelStates.size();
    for (int i=modelStateCount-1; i>=0; --i) {
        m_modelStates.removeAt(i);
        m_editorModel->removeState(i);
    }
}

// index without base state
int StatesEditorView::modelStateIndex(const QmlModelState &state)
{
    return m_modelStates.indexOf(state) - 1;
}

void StatesEditorView::timerEvent(QTimerEvent *event)
{
    if (debug)
        qDebug() << __FUNCTION__;

    int index = m_updateTimerIdList.indexOf(event->timerId());
    if (index > -1) {
        event->accept();
        Q_ASSERT(index >= 0);
        if (index < m_modelStates.count()) //there might be updates for a state already deleted 100ms are long
            m_editorModel->updateState(index);
        killTimer(m_updateTimerIdList[index]);
       m_updateTimerIdList[index] = 0;
    } else {
        QmlModelView::timerEvent(event);
    }
}

} // namespace Internal
} // namespace QmlDesigner
