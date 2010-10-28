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

#ifndef FANCYMAINWINDOW_H
#define FANCYMAINWINDOW_H

#include "utils_global.h"

#include <QtGui/QMainWindow>

QT_BEGIN_NAMESPACE
class QSettings;
QT_END_NAMESPACE

namespace Utils {

struct FancyMainWindowPrivate;

// MainWindow with dock widgets and additional "lock" functionality
// (locking the dock widgets in place) and "reset layout" functionality.
// The dock actions and the additional actions should be accessible
// in a Window-menu.
class QTCREATOR_UTILS_EXPORT FancyMainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit FancyMainWindow(QWidget *parent = 0);
    virtual ~FancyMainWindow();

    /* The widget passed in should have an objectname set
     * which will then be used as key for QSettings. */
    QDockWidget *addDockForWidget(QWidget *widget);
    QList<QDockWidget *> dockWidgets() const;

    void setTrackingEnabled(bool enabled);
    bool isLocked() const;

    void saveSettings(QSettings *settings) const;
    void restoreSettings(const QSettings *settings);
    QHash<QString, QVariant> saveSettings() const;
    void restoreSettings(const QHash<QString, QVariant> &settings);

    // Additional context menu actions
    QAction *menuSeparator1() const;
    QAction *toggleLockedAction() const;
    QAction *menuSeparator2() const;
    QAction *resetLayoutAction() const;

    // Overwritten to add locked/reset.
    virtual QMenu *createPopupMenu();

signals:
    // Emitted by resetLayoutAction(). Connect to a slot
    // restoring the default layout.
    void resetLayout();

public slots:
    void setLocked(bool locked);
    void setDockActionsVisible(bool v);

protected:
    void hideEvent(QHideEvent *event);
    void showEvent(QShowEvent *event);

private slots:
    void onDockActionTriggered();
    void onDockVisibilityChange(bool);
    void onTopLevelChanged();

private:
    void updateDockWidget(QDockWidget *dockWidget);
    void handleVisibilityChanged(bool visible);

    FancyMainWindowPrivate *d;
};

} // namespace Utils

#endif // FANCYMAINWINDOW_H