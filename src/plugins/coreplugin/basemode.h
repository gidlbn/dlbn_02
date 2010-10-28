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

#ifndef BASEMODE_H
#define BASEMODE_H

#include "core_global.h"
#include "imode.h"

#include <QtCore/QObject>

#include <QtGui/QIcon>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace Core {

class CORE_EXPORT BaseMode
  : public IMode
{
    Q_OBJECT

public:
    BaseMode(QObject *parent = 0);
    ~BaseMode();

    // IMode
    QString displayName() const { return m_displayName; }
    QIcon icon() const { return m_icon; }
    int priority() const { return m_priority; }
    QWidget *widget() { return m_widget; }
    QString id() const { return m_id; }
    QList<int> context() const { return m_context; }
    QString contextHelpId() const { return m_helpId; }

    void setDisplayName(const QString &name) { m_displayName = name; }
    void setIcon(const QIcon &icon) { m_icon = icon; }
    void setPriority(int priority) { m_priority = priority; }
    void setWidget(QWidget *widget) { m_widget = widget; }
    void setId(const QString &id) { m_id = id; }
    void setContextHelpId(const QString &helpId) { m_helpId = helpId; }
    void setContext(const QList<int> &context) { m_context = context; }

private:
    QString m_displayName;
    QIcon m_icon;
    int m_priority;
    QWidget *m_widget;
    QString m_id;
    QString m_helpId;
    QList<int> m_context;
};

} // namespace Core

#endif // BASEMODE_H
