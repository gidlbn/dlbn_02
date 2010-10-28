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

#ifndef RUNSETTINGSPROPERTIESPAGE_H
#define RUNSETTINGSPROPERTIESPAGE_H

#include "iprojectproperties.h"

#include <QtGui/QWidget>
#include <QtCore/QAbstractListModel>

QT_BEGIN_NAMESPACE
class QMenu;
class QLabel;
QT_END_NAMESPACE

namespace ProjectExplorer {

class RunConfiguration;

namespace Internal {

const char * const RUNSETTINGS_PANEL_ID("ProjectExplorer.RunSettingsPanel");

namespace Ui {
class RunSettingsPropertiesPage;
}

class RunConfigurationsModel;
class RunSettingsWidget;

class RunSettingsPanelFactory : public ITargetPanelFactory
{
public:
    QString id() const;
    QString displayName() const;
    bool supports(Target *target);
    IPropertiesPanel *createPanel(Target *target);
};

class RunSettingsPanel : public IPropertiesPanel
{
public:
    RunSettingsPanel(Target *target);
    ~RunSettingsPanel();

    QString displayName() const;
    QWidget *widget() const;
    QIcon icon() const;

private:
    RunSettingsWidget *m_widget;
    QIcon m_icon;
};

class RunSettingsWidget : public QWidget
{
    Q_OBJECT
public:
    RunSettingsWidget(Target *target);
    ~RunSettingsWidget();

private slots:
    void currentRunConfigurationChanged(int index);
    void aboutToShowAddMenu();
    void addRunConfiguration();
    void removeRunConfiguration();
    void activeRunConfigurationChanged();
private:
    Target *m_target;
    RunConfigurationsModel *m_runConfigurationsModel;
    Ui::RunSettingsPropertiesPage *m_ui;
    QWidget *m_runConfigurationWidget;
    QMenu *m_addMenu;
    bool m_ignoreChange;
};

/*! A model to represent the run configurations of a target. */
class RunConfigurationsModel : public QAbstractListModel
{
    Q_OBJECT
public:
    RunConfigurationsModel(Target *target, QObject *parent = 0);

    int rowCount(const QModelIndex &parent = QModelIndex()) const;
    int columnCount(const QModelIndex &parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const;

    RunConfiguration *runConfigurationAt(int i);
    RunConfiguration *runConfigurationFor(const QModelIndex &idx);
    QModelIndex indexFor(RunConfiguration *rc);
private slots:
    void addedRunConfiguration(ProjectExplorer::RunConfiguration*);
    void removedRunConfiguration(ProjectExplorer::RunConfiguration*);
    void displayNameChanged();
private:
    Target *m_target;
    QList<RunConfiguration *> m_runConfigurations;
};



} // namespace Internal
} // namespace ProjectExplorer

#endif // RUNSETTINGSPROPERTIESPAGE_H
