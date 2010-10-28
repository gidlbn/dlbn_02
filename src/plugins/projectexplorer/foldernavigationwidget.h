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

#ifndef FOLDERNAVIGATIONWIDGET_H
#define FOLDERNAVIGATIONWIDGET_H

#include <coreplugin/inavigationwidgetfactory.h>

#include <QtGui/QWidget>

QT_BEGIN_NAMESPACE
class QLabel;
class QListView;
class QSortFilterProxyModel;
class QModelIndex;
class QFileSystemModel;
class QDir;
class QAction;
QT_END_NAMESPACE

namespace ProjectExplorer {

class ProjectExplorerPlugin;
class Project;
class Node;

namespace Internal {

class FolderNavigationWidget : public QWidget
{
    Q_OBJECT
    Q_PROPERTY(bool autoSynchronization READ autoSynchronization WRITE setAutoSynchronization)
public:
    FolderNavigationWidget(QWidget *parent = 0);

    bool autoSynchronization() const;


    // Helpers for common directory browser options.
    static void showInGraphicalShell(QWidget *parent, const QString &path);
    static void openTerminal(const QString &path);
    // Platform-dependent action descriptions
    static QString msgGraphicalShellAction();
    static QString msgTerminalAction();

public slots:
    void setAutoSynchronization(bool sync);
    void toggleAutoSynchronization();

private slots:
    void setCurrentFile(const QString &filePath);
    void slotOpenItem(const QModelIndex &viewIndex);

protected:
    virtual void contextMenuEvent(QContextMenuEvent *ev);

private:
    void setCurrentTitle(QString dirName, const QString &fullPath);
    bool setCurrentDirectory(const QString &directory);
    void openItem(const QModelIndex &srcIndex);
    QModelIndex currentItem() const;
    QString currentDirectory() const;

    QListView *m_listView;
    QFileSystemModel *m_fileSystemModel;
    QSortFilterProxyModel *m_filterModel;
    QLabel *m_title;
    bool m_autoSync;
};

class FolderNavigationWidgetFactory : public Core::INavigationWidgetFactory
{
    Q_OBJECT
public:
    FolderNavigationWidgetFactory();
    ~FolderNavigationWidgetFactory();

    QString displayName() const;
    QString id() const;
    QKeySequence activationSequence() const;
    Core::NavigationView createWidget();
};

} // namespace Internal
} // namespace ProjectExplorer

#endif // FOLDERNAVIGATIONWIDGET_H
