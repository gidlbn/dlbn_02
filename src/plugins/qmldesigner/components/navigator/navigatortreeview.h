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


#ifndef NAVIGATORTREEVIEW_H
#define NAVIGATORTREEVIEW_H

#include <QTreeView>

#include <QStyledItemDelegate>

#include <QPainter>

QT_BEGIN_NAMESPACE
class QTreeView;
class QStandardItem;
class QItemSelection;
class QModelIndex;
QT_END_NAMESPACE

namespace QmlDesigner {

class NavigatorWidget;
class NavigatorTreeModel;


class IconCheckboxItemDelegate : public QStyledItemDelegate
{
    public:
    explicit IconCheckboxItemDelegate(QObject *parent = 0, QString checkedPixmapURL="", QString uncheckedPixmapURL="", NavigatorTreeModel *treeModel=NULL)
            : QStyledItemDelegate(parent),offPix(uncheckedPixmapURL),onPix(checkedPixmapURL),m_TreeModel(treeModel)
    {}

    QSize sizeHint(const QStyleOptionViewItem &option,
                   const QModelIndex &index) const;

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option, const QModelIndex &index) const;

    private:
    QPixmap offPix;
    QPixmap onPix;
    NavigatorTreeModel *m_TreeModel;

};

class IdItemDelegate : public QStyledItemDelegate
{
    public:
    explicit IdItemDelegate(QObject *parent=0, NavigatorTreeModel *treeModel=NULL) : QStyledItemDelegate(parent),m_TreeModel(treeModel) {}

    void paint(QPainter *painter,
               const QStyleOptionViewItem &option, const QModelIndex &index) const;

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const;
    void setEditorData(QWidget *editor, const QModelIndex &index) const;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const;
    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const;

    private:
    NavigatorTreeModel *m_TreeModel;
};

    class NavigatorTreeView : public QTreeView
    {
    public:
        NavigatorTreeView(QWidget *parent = 0);
  };
}

#endif // NAVIGATORTREEVIEW_H
