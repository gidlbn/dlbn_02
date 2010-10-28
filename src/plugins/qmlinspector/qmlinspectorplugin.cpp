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
#include "qmlinspectorconstants.h"
#include "qmlinspector.h"
#include "qmlinspectorplugin.h"

#include <debugger/debuggeruiswitcher.h>
#include <debugger/debuggerconstants.h>

#include <qmlprojectmanager/qmlproject.h>
#include <qmljseditor/qmljseditorconstants.h>

#include <coreplugin/modemanager.h>
#include <projectexplorer/projectexplorer.h>
#include <extensionsystem/pluginmanager.h>
#include <coreplugin/icore.h>

#include <projectexplorer/runconfiguration.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/project.h>

#include <coreplugin/coreconstants.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/command.h>

#include <QtCore/QStringList>
#include <QtCore/QtPlugin>
#include <QtCore/QTimer>

#include <QtGui/QHBoxLayout>
#include <QtGui/QToolButton>

#include <QtCore/QDebug>


using namespace Qml;


static QToolButton *createToolButton(QAction *action)
{
    QToolButton *button = new QToolButton;
    button->setDefaultAction(action);
    return button;
}

QmlInspectorPlugin::QmlInspectorPlugin()
    : m_inspector(0)
{

}

QmlInspectorPlugin::~QmlInspectorPlugin()
{
    removeObject(m_inspector);
    delete m_inspector;
    m_inspector = 0;
}

void QmlInspectorPlugin::aboutToShutdown()
{
    m_inspector->shutdown();
}

bool QmlInspectorPlugin::initialize(const QStringList &arguments, QString *errorString)
{
    Q_UNUSED(arguments);
    Q_UNUSED(errorString);
    Core::ICore *core = Core::ICore::instance();
    connect(Core::ModeManager::instance(), SIGNAL(currentModeChanged(Core::IMode*)),
            SLOT(prepareDebugger(Core::IMode*)));

    ExtensionSystem::PluginManager *pluginManager = ExtensionSystem::PluginManager::instance();
    Debugger::DebuggerUISwitcher *uiSwitcher = pluginManager->getObject<Debugger::DebuggerUISwitcher>();

    uiSwitcher->addLanguage(Qml::Constants::LANG_QML,
                            QList<int>() << core->uniqueIDManager()->uniqueIdentifier(Constants::C_INSPECTOR));
    m_inspector = new QmlInspector;
    m_inspector->createDockWidgets();
    addObject(m_inspector);

    return true;
}

void QmlInspectorPlugin::extensionsInitialized()
{
    ExtensionSystem::PluginManager *pluginManager = ExtensionSystem::PluginManager::instance();
    Debugger::DebuggerUISwitcher *uiSwitcher = pluginManager->getObject<Debugger::DebuggerUISwitcher>();

    connect(uiSwitcher, SIGNAL(dockArranged(QString)), SLOT(setDockWidgetArrangement(QString)));

    ProjectExplorer::ProjectExplorerPlugin *pex = ProjectExplorer::ProjectExplorerPlugin::instance();
    if (pex) {
        connect(pex, SIGNAL(aboutToExecuteProject(ProjectExplorer::Project*, QString)),
                SLOT(activateDebuggerForProject(ProjectExplorer::Project*, QString)));
    }

    QWidget *configBar = new QWidget;
    configBar->setProperty("topBorder", true);

    QHBoxLayout *configBarLayout = new QHBoxLayout(configBar);
    configBarLayout->setMargin(0);
    configBarLayout->setSpacing(5);

    Core::ICore *core = Core::ICore::instance();
    Core::ActionManager *am = core->actionManager();
    configBarLayout->addWidget(createToolButton(am->command(ProjectExplorer::Constants::DEBUG)->action()));
    configBarLayout->addWidget(createToolButton(am->command(ProjectExplorer::Constants::STOP)->action()));

    configBarLayout->addStretch();

    uiSwitcher->setToolbar(Qml::Constants::LANG_QML, configBar);
}

void QmlInspectorPlugin::activateDebuggerForProject(ProjectExplorer::Project *project, const QString &runMode)
{
    if (runMode == ProjectExplorer::Constants::DEBUGMODE) {
        // FIXME we probably want to activate the debugger for other projects than QmlProjects,
        // if they contain Qml files. Some kind of options should exist for this behavior.
        QmlProjectManager::QmlProject *qmlproj = qobject_cast<QmlProjectManager::QmlProject*>(project);
        if (qmlproj && m_inspector->setDebugConfigurationDataFromProject(qmlproj))
            m_inspector->startQmlProjectDebugger();
    }

}

void QmlInspectorPlugin::prepareDebugger(Core::IMode *mode)
{
    if (mode->id() != Debugger::Constants::MODE_DEBUG)
        return;

    ProjectExplorer::ProjectExplorerPlugin *pex = ProjectExplorer::ProjectExplorerPlugin::instance();

    if (pex->startupProject() && pex->startupProject()->id() == "QmlProjectManager.QmlProject")
    {
        ExtensionSystem::PluginManager *pluginManager = ExtensionSystem::PluginManager::instance();
        Debugger::DebuggerUISwitcher *uiSwitcher = pluginManager->getObject<Debugger::DebuggerUISwitcher>();
        uiSwitcher->setActiveLanguage(Qml::Constants::LANG_QML);
    }
}

void QmlInspectorPlugin::setDockWidgetArrangement(const QString &activeLanguage)
{
    if (activeLanguage == Qml::Constants::LANG_QML || activeLanguage.isEmpty())
        m_inspector->setSimpleDockWidgetArrangement();
}


Q_EXPORT_PLUGIN(QmlInspectorPlugin)
