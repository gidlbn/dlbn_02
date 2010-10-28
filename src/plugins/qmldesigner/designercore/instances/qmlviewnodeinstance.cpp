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

#include "qmlviewnodeinstance.h"

#include <QDeclarativeView>
#include <QDeclarativeItem>

#include <invalidnodeinstanceexception.h>

namespace QmlDesigner {
namespace Internal {

QDeclarativeViewNodeInstance::QDeclarativeViewNodeInstance(QDeclarativeView *view)
  : GraphicsViewNodeInstance(view)
{
}


QDeclarativeViewNodeInstance::Pointer QDeclarativeViewNodeInstance::create(const NodeMetaInfo &nodeMetaInfo, QDeclarativeContext *context, QObject *objectToBeWrapped)
{
    QObject *object = 0;
    if (objectToBeWrapped)
        object = objectToBeWrapped;
    else
        createObject(nodeMetaInfo, context);

    QDeclarativeView* view = qobject_cast<QDeclarativeView*>(object);
    if (view == 0)
        throw InvalidNodeInstanceException(__LINE__, __FUNCTION__, __FILE__);

    Pointer instance(new QDeclarativeViewNodeInstance(view));

    if (objectToBeWrapped)
        instance->setDeleteHeldInstance(false); // the object isn't owned

    instance->populateResetValueHash();

    return instance;
}

QDeclarativeView* QDeclarativeViewNodeInstance::view() const
{
    QDeclarativeView* view = qobject_cast<QDeclarativeView*>(widget());
    Q_ASSERT(view);
    return view;
}

bool QDeclarativeViewNodeInstance::isQDeclarativeView() const
{
    return true;
}

void QDeclarativeViewNodeInstance::addItem(QDeclarativeItem *item)
{
    QDeclarativeItem *rootItem = qobject_cast<QDeclarativeItem *>(view()->rootObject());
    Q_ASSERT_X(rootItem, Q_FUNC_INFO, "root item is QDeclarativeItem based");
    item->setParent(rootItem);
}

}
}
