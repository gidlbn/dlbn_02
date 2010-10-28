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

#ifndef FINDTOOLWINDOW_H
#define FINDTOOLWINDOW_H

#include "ui_finddialog.h"
#include "ifindfilter.h"

#include <QtCore/QList>

QT_FORWARD_DECLARE_CLASS(QCompleter)

namespace Find {
class FindPlugin;

namespace Internal {

class FindToolWindow : public QDialog
{
    Q_OBJECT

public:
    explicit FindToolWindow(FindPlugin *plugin);
    ~FindToolWindow();

    void setFindFilters(const QList<IFindFilter *> &filters);

    void setFindText(const QString &text);
    void open(IFindFilter *filter);
    void readSettings();
    void writeSettings();

protected:
    bool eventFilter(QObject *obj, QEvent *event);

private slots:
    void search();
    void replace();
    void setCurrentFilter(int index);
    void updateButtonStates();

private:
    void acceptAndGetParameters(QString *term, IFindFilter **filter);

    Ui::FindDialog m_ui;
    FindPlugin *m_plugin;
    QList<IFindFilter *> m_filters;
    QCompleter *m_findCompleter;
    QWidgetList m_configWidgets;
    IFindFilter *m_currentFilter;
};

} // namespace Internal
} // namespace Find

#endif // FINDTOOLWINDOW_H
