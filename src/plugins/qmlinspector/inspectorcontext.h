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

#ifndef INSPECTORCONTEXT_H
#define INSPECTORCONTEXT_H

#include <coreplugin/icontext.h>
#include <QList>

QT_BEGIN_NAMESPACE
class QWidget;
QT_END_NAMESPACE

namespace Qml {
namespace Internal {

class ObjectPropertiesView;
class ObjectTree;
class DesignModeWidget;

/**
  * Bauhaus Design mode context object
  */
class InspectorContext : public Core::IContext
{
    Q_OBJECT

public:
    InspectorContext(QWidget *widget);
    ~InspectorContext();

    QList<int> context() const;
    QWidget *widget();

    QString contextHelpId() const;

    static QString contextHelpIdForProperty(const QString &itemName, const QString &propName);
    static QString contextHelpIdForItem(const QString &itemName);

public slots:
    void setContextHelpId(const QString &helpId);

private:
    QList<int> m_context;
    QWidget *m_widget;
    QString m_contextHelpId;

};

} // Internal
} // Qml

#endif // DESIGNMODECONTEXT_H
