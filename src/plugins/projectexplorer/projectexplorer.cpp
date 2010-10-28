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

#include "projectexplorer.h"

#include "project.h"
#include "projectexplorersettings.h"
#include "target.h"
#include "targetsettingspanel.h"
#include "applicationrunconfiguration.h"
#include "allprojectsfilter.h"
#include "allprojectsfind.h"
#include "buildmanager.h"
#include "buildsettingspropertiespage.h"
#include "currentprojectfind.h"
#include "currentprojectfilter.h"
#include "customexecutablerunconfiguration.h"
#include "editorsettingspropertiespage.h"
#include "dependenciespanel.h"
#include "foldernavigationwidget.h"
#include "iprojectmanager.h"
#include "metatypedeclarations.h"
#include "nodesvisitor.h"
#include "outputwindow.h"
#include "persistentsettings.h"
#include "pluginfilefactory.h"
#include "processstep.h"
#include "projectexplorerconstants.h"
#include "customwizard.h"
#include "projectfilewizardextension.h"
#include "projecttreewidget.h"
#include "projectwindow.h"
#include "removefiledialog.h"
#include "runsettingspropertiespage.h"
#include "scriptwrappers.h"
#include "session.h"
#include "sessiondialog.h"
#include "target.h"
#include "projectexplorersettingspage.h"
#include "projectwelcomepage.h"
#include "projectwelcomepagewidget.h"
#include "corelistenercheckingforrunningbuild.h"
#include "buildconfiguration.h"
#include "buildconfigdialog.h"
#include "miniprojecttargetselector.h"

#include <coreplugin/basemode.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/filemanager.h>
#include <coreplugin/icore.h>
#include <coreplugin/mainwindow.h>
#include <coreplugin/mimedatabase.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/editormanager/ieditor.h>
#include <coreplugin/editormanager/ieditorfactory.h>
#include <coreplugin/editormanager/iexternaleditor.h>
#include <coreplugin/findplaceholder.h>
#include <coreplugin/basefilewizard.h>
#include <coreplugin/vcsmanager.h>
#include <coreplugin/iversioncontrol.h>
#include <welcome/welcomemode.h>
#include <extensionsystem/pluginmanager.h>
#include <utils/consoleprocess.h>
#include <utils/qtcassert.h>
#include <utils/parameteraction.h>

#include <QtCore/QtPlugin>
#include <QtCore/QDateTime>
#include <QtCore/QDebug>
#include <QtCore/QSettings>

#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QFileDialog>
#include <QtGui/QMenu>
#include <QtGui/QMessageBox>

Q_DECLARE_METATYPE(Core::IEditorFactory*);
Q_DECLARE_METATYPE(Core::IExternalEditor*);

namespace {
bool debug = false;
}

namespace ProjectExplorer {

struct ProjectExplorerPluginPrivate {
    ProjectExplorerPluginPrivate();

    QMenu *m_sessionContextMenu;
    QMenu *m_sessionMenu;
    QMenu *m_projectMenu;
    QMenu *m_subProjectMenu;
    QMenu *m_folderMenu;
    QMenu *m_fileMenu;
    QMenu *m_openWithMenu;

    QMultiMap<int, QObject*> m_actionMap;
    QAction *m_sessionManagerAction;
    QAction *m_newAction;
#if 0
    QAction *m_loadAction;
#endif
    Utils::ParameterAction *m_unloadAction;
    QAction *m_clearSession;
    QAction *m_buildProjectOnlyAction;
    Utils::ParameterAction *m_buildAction;
    Utils::ParameterAction *m_buildActionContextMenu;
    QAction *m_buildSessionAction;
    QAction *m_rebuildProjectOnlyAction;
    Utils::ParameterAction *m_rebuildAction;
    Utils::ParameterAction *m_rebuildActionContextMenu;
    QAction *m_rebuildSessionAction;
    QAction *m_cleanProjectOnlyAction;
    Utils::ParameterAction *m_cleanAction;
    Utils::ParameterAction *m_cleanActionContextMenu;
    QAction *m_cleanSessionAction;
    QAction *m_runAction;
    QAction *m_runActionContextMenu;
    QAction *m_cancelBuildAction;
    QAction *m_debugAction;
    QAction *m_addNewFileAction;
    QAction *m_addExistingFilesAction;
    QAction *m_openFileAction;
    QAction *m_showInGraphicalShell;
    QAction *m_openTerminalHere;
    QAction *m_removeFileAction;
    QAction *m_renameFileAction;
    QAction *m_projectSelectorAction;
    QAction *m_projectSelectorActionMenu;

    Internal::ProjectWindow *m_proWindow;
    SessionManager *m_session;
    QString m_sessionToRestoreAtStartup;

    Project *m_currentProject;
    Node *m_currentNode;

    BuildManager *m_buildManager;

    QList<Internal::ProjectFileFactory*> m_fileFactories;
    QStringList m_profileMimeTypes;
    Internal::OutputPane *m_outputPane;

    QList<QPair<QString, QString> > m_recentProjects; // pair of filename, displayname
    static const int m_maxRecentProjects = 7;

    QString m_lastOpenDirectory;
    RunConfiguration *m_delayedRunConfiguration; // TODO this is not right
    RunControl *m_debuggingRunControl;
    QString m_runMode;
    QString m_projectFilterString;
    Internal::MiniProjectTargetSelector * m_targetSelector;
    Internal::ProjectExplorerSettings m_projectExplorerSettings;
    Internal::ProjectWelcomePage *m_welcomePage;

    Core::BaseMode * m_projectsMode;
};

ProjectExplorerPluginPrivate::ProjectExplorerPluginPrivate() :
    m_currentProject(0),
    m_currentNode(0),
    m_delayedRunConfiguration(0),
    m_debuggingRunControl(0),
    m_projectsMode(0)
{
}

}  // namespace ProjectExplorer

using namespace ProjectExplorer;
using namespace ProjectExplorer::Internal;


ProjectExplorerPlugin *ProjectExplorerPlugin::m_instance = 0;

ProjectExplorerPlugin::ProjectExplorerPlugin()
    : d(new ProjectExplorerPluginPrivate)
{
    m_instance = this;
}

ProjectExplorerPlugin::~ProjectExplorerPlugin()
{
    removeObject(d->m_welcomePage);
    delete d->m_welcomePage;
    removeObject(this);
    delete d;
}

ProjectExplorerPlugin *ProjectExplorerPlugin::instance()
{
    return m_instance;
}

bool ProjectExplorerPlugin::parseArguments(const QStringList &arguments, QString * /* error */)
{
    CustomWizard::setVerbose(arguments.count(QLatin1String("-customwizard-verbose")));
    return true;
}

bool ProjectExplorerPlugin::initialize(const QStringList &arguments, QString *error)
{
    if (!parseArguments(arguments, error))
        return false;

    Core::ICore *core = Core::ICore::instance();
    Core::ActionManager *am = core->actionManager();

    d->m_welcomePage = new ProjectWelcomePage;
    connect(d->m_welcomePage, SIGNAL(manageSessions()), this, SLOT(showSessionManager()));
    addObject(d->m_welcomePage);
    addObject(this);

    connect(core->fileManager(), SIGNAL(currentFileChanged(QString)),
            this, SLOT(setCurrentFile(QString)));

    d->m_session = new SessionManager(this);

    connect(d->m_session, SIGNAL(projectAdded(ProjectExplorer::Project *)),
            this, SIGNAL(fileListChanged()));
    connect(d->m_session, SIGNAL(aboutToRemoveProject(ProjectExplorer::Project *)),
            this, SLOT(invalidateProject(ProjectExplorer::Project *)));
    connect(d->m_session, SIGNAL(projectRemoved(ProjectExplorer::Project *)),
            this, SIGNAL(fileListChanged()));
    connect(d->m_session, SIGNAL(startupProjectChanged(ProjectExplorer::Project *)),
            this, SLOT(startupProjectChanged()));
    connect(d->m_session, SIGNAL(dependencyChanged(ProjectExplorer::Project*,ProjectExplorer::Project*)),
            this, SLOT(updateActions()));

    d->m_proWindow = new ProjectWindow;

    QList<int> globalcontext;
    globalcontext.append(Core::Constants::C_GLOBAL_ID);

    QList<int> pecontext;
    pecontext << core->uniqueIDManager()->uniqueIdentifier(Constants::C_PROJECTEXPLORER);

    d->m_projectsMode = new Core::BaseMode;
    d->m_projectsMode->setDisplayName(tr("Projects"));
    d->m_projectsMode->setId(QLatin1String(Constants::MODE_SESSION));
    d->m_projectsMode->setIcon(QIcon(QLatin1String(":/fancyactionbar/images/mode_Project.png")));
    d->m_projectsMode->setPriority(Constants::P_MODE_SESSION);
    d->m_projectsMode->setWidget(d->m_proWindow);
    d->m_projectsMode->setContext(QList<int>() << pecontext);
    d->m_projectsMode->setEnabled(session()->startupProject());
    d->m_projectsMode->setContextHelpId(QLatin1String("Managing Projects"));
    addAutoReleasedObject(d->m_projectsMode);
    d->m_proWindow->layout()->addWidget(new Core::FindToolBarPlaceHolder(d->m_proWindow));

    d->m_buildManager = new BuildManager(this);
    connect(d->m_buildManager, SIGNAL(buildStateChanged(ProjectExplorer::Project *)),
            this, SLOT(buildStateChanged(ProjectExplorer::Project *)));
    connect(d->m_buildManager, SIGNAL(buildQueueFinished(bool)),
            this, SLOT(buildQueueFinished(bool)));

    addAutoReleasedObject(new CoreListenerCheckingForRunningBuild(d->m_buildManager));

    d->m_outputPane = new OutputPane;
    addAutoReleasedObject(d->m_outputPane);
    connect(d->m_session, SIGNAL(projectRemoved(ProjectExplorer::Project *)),
            d->m_outputPane, SLOT(projectRemoved()));

    AllProjectsFilter *allProjectsFilter = new AllProjectsFilter(this);
    addAutoReleasedObject(allProjectsFilter);

    CurrentProjectFilter *currentProjectFilter = new CurrentProjectFilter(this);
    addAutoReleasedObject(currentProjectFilter);

    addAutoReleasedObject(new BuildSettingsPanelFactory);
    addAutoReleasedObject(new RunSettingsPanelFactory);
    addAutoReleasedObject(new EditorSettingsPanelFactory);
    addAutoReleasedObject(new DependenciesPanelFactory(d->m_session));

    ProcessStepFactory *processStepFactory = new ProcessStepFactory;
    addAutoReleasedObject(processStepFactory);

    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    AllProjectsFind *allProjectsFind = new AllProjectsFind(this,
        pm->getObject<Find::SearchResultWindow>());
    addAutoReleasedObject(allProjectsFind);

    CurrentProjectFind *currentProjectFind = new CurrentProjectFind(this,
        pm->getObject<Find::SearchResultWindow>());
    addAutoReleasedObject(currentProjectFind);

    addAutoReleasedObject(new LocalApplicationRunControlFactory);
    addAutoReleasedObject(new CustomExecutableRunConfigurationFactory);

    addAutoReleasedObject(new ProjectFileWizardExtension);

    // Settings page
    addAutoReleasedObject(new ProjectExplorerSettingsPage);

    // context menus
    Core::ActionContainer *msessionContextMenu =
        am->createMenu(Constants::M_SESSIONCONTEXT);
    Core::ActionContainer *mproject =
        am->createMenu(Constants::M_PROJECTCONTEXT);
    Core::ActionContainer *msubProject =
        am->createMenu(Constants::M_SUBPROJECTCONTEXT);
    Core::ActionContainer *mfolder =
        am->createMenu(Constants::M_FOLDERCONTEXT);
    Core::ActionContainer *mfilec =
        am->createMenu(Constants::M_FILECONTEXT);

    d->m_sessionContextMenu = msessionContextMenu->menu();
    d->m_projectMenu = mproject->menu();
    d->m_subProjectMenu = msubProject->menu();
    d->m_folderMenu = mfolder->menu();
    d->m_fileMenu = mfilec->menu();

    Core::ActionContainer *mfile =
        am->actionContainer(Core::Constants::M_FILE);
    Core::ActionContainer *menubar =
        am->actionContainer(Core::Constants::MENU_BAR);

    // mode manager (for fancy actions)
    Core::ModeManager *modeManager = core->modeManager();

    // build menu
    Core::ActionContainer *mbuild =
        am->createMenu(Constants::M_BUILDPROJECT);
    mbuild->menu()->setTitle(tr("&Build"));
    menubar->addMenu(mbuild, Core::Constants::G_VIEW);

    // debug menu
    Core::ActionContainer *mdebug =
        am->createMenu(Constants::M_DEBUG);
    mdebug->menu()->setTitle(tr("&Debug"));
    menubar->addMenu(mdebug, Core::Constants::G_VIEW);

    Core::ActionContainer *mstartdebugging =
        am->createMenu(Constants::M_DEBUG_STARTDEBUGGING);
    mstartdebugging->menu()->setTitle(tr("&Start Debugging"));
    mdebug->addMenu(mstartdebugging, Core::Constants::G_DEFAULT_ONE);

    //
    // Groups
    //

    mbuild->appendGroup(Constants::G_BUILD_SESSION);
    mbuild->appendGroup(Constants::G_BUILD_PROJECT);
    mbuild->appendGroup(Constants::G_BUILD_OTHER);
    mbuild->appendGroup(Constants::G_BUILD_CANCEL);
    mbuild->appendGroup(Constants::G_BUILD_RUN);

    msessionContextMenu->appendGroup(Constants::G_SESSION_BUILD);
    msessionContextMenu->appendGroup(Constants::G_SESSION_FILES);
    msessionContextMenu->appendGroup(Constants::G_SESSION_OTHER);
    msessionContextMenu->appendGroup(Constants::G_SESSION_CONFIG);

    mproject->appendGroup(Constants::G_PROJECT_OPEN);
    mproject->appendGroup(Constants::G_PROJECT_NEW);
    mproject->appendGroup(Constants::G_PROJECT_BUILD);
    mproject->appendGroup(Constants::G_PROJECT_RUN);
    mproject->appendGroup(Constants::G_PROJECT_FILES);
    mproject->appendGroup(Constants::G_PROJECT_OTHER);
    mproject->appendGroup(Constants::G_PROJECT_CONFIG);

    msubProject->appendGroup(Constants::G_PROJECT_OPEN);
    msubProject->appendGroup(Constants::G_PROJECT_BUILD);
    msubProject->appendGroup(Constants::G_PROJECT_FILES);
    msubProject->appendGroup(Constants::G_PROJECT_OTHER);
    msubProject->appendGroup(Constants::G_PROJECT_CONFIG);

    mfolder->appendGroup(Constants::G_FOLDER_FILES);
    mfolder->appendGroup(Constants::G_FOLDER_OTHER);
    mfolder->appendGroup(Constants::G_FOLDER_CONFIG);

    mfilec->appendGroup(Constants::G_FILE_OPEN);
    mfilec->appendGroup(Constants::G_FILE_OTHER);
    mfilec->appendGroup(Constants::G_FILE_CONFIG);

    // "open with" submenu
    Core::ActionContainer * const openWith =
            am->createMenu(ProjectExplorer::Constants::M_OPENFILEWITHCONTEXT);
    openWith->setEmptyAction(Core::ActionContainer::EA_None);
    d->m_openWithMenu = openWith->menu();
    d->m_openWithMenu->setTitle(tr("Open With"));

    connect(d->m_openWithMenu, SIGNAL(triggered(QAction *)),
            this, SLOT(openWithMenuTriggered(QAction *)));

    //
    // Separators
    //

    Core::Command *cmd;
    QAction *sep;

    sep = new QAction(this);
    sep->setSeparator(true);
    cmd = am->registerAction(sep, QLatin1String("ProjectExplorer.Build.Sep"), globalcontext);
    mbuild->addAction(cmd, Constants::G_BUILD_PROJECT);

    sep = new QAction(this);
    sep->setSeparator(true);
    cmd = am->registerAction(sep, QLatin1String("ProjectExplorer.Files.Sep"), globalcontext);
    msessionContextMenu->addAction(cmd, Constants::G_SESSION_FILES);
    mproject->addAction(cmd, Constants::G_PROJECT_FILES);
    msubProject->addAction(cmd, Constants::G_PROJECT_FILES);

    sep = new QAction(this);
    sep->setSeparator(true);
    cmd = am->registerAction(sep, QLatin1String("ProjectExplorer.New.Sep"), globalcontext);
    mproject->addAction(cmd, Constants::G_PROJECT_NEW);

    sep = new QAction(this);
    sep->setSeparator(true);
    cmd = am->registerAction(sep, QLatin1String("ProjectExplorer.Config.Sep"), globalcontext);
    msessionContextMenu->addAction(cmd, Constants::G_SESSION_CONFIG);
    mproject->addAction(cmd, Constants::G_PROJECT_CONFIG);
    msubProject->addAction(cmd, Constants::G_PROJECT_CONFIG);

    sep = new QAction(this);
    sep->setSeparator(true);
    cmd = am->registerAction(sep, QLatin1String("ProjectExplorer.Projects.Sep"), globalcontext);
    mfile->addAction(cmd, Core::Constants::G_FILE_PROJECT);

    sep = new QAction(this);
    sep->setSeparator(true);
    cmd = am->registerAction(sep, QLatin1String("ProjectExplorer.Other.Sep"), globalcontext);
    mbuild->addAction(cmd, Constants::G_BUILD_OTHER);
    msessionContextMenu->addAction(cmd, Constants::G_SESSION_OTHER);
    mproject->addAction(cmd, Constants::G_PROJECT_OTHER);
    msubProject->addAction(cmd, Constants::G_PROJECT_OTHER);

    sep = new QAction(this);
    sep->setSeparator(true);
    cmd = am->registerAction(sep, QLatin1String("ProjectExplorer.Run.Sep"), globalcontext);
    mbuild->addAction(cmd, Constants::G_BUILD_RUN);
    mproject->addAction(cmd, Constants::G_PROJECT_RUN);

    sep = new QAction(this);
    sep->setSeparator(true);
    cmd = am->registerAction(sep, QLatin1String("ProjectExplorer.CancelBuild.Sep"), globalcontext);
    mbuild->addAction(cmd, Constants::G_BUILD_CANCEL);

    //
    // Actions
    //

    // new session action
    d->m_sessionManagerAction = new QAction(tr("Session Manager..."), this);
    cmd = am->registerAction(d->m_sessionManagerAction, Constants::NEWSESSION, globalcontext);
    cmd->setDefaultKeySequence(QKeySequence());

    // new action
    d->m_newAction = new QAction(tr("New Project..."), this);
    cmd = am->registerAction(d->m_newAction, Constants::NEWPROJECT, globalcontext);
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+N")));
    msessionContextMenu->addAction(cmd, Constants::G_SESSION_FILES);

#if 0
    // open action
    d->m_loadAction = new QAction(tr("Load Project..."), this);
    cmd = am->registerAction(d->m_loadAction, Constants::LOAD, globalcontext);
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+O")));
    mfile->addAction(cmd, Core::Constants::G_FILE_PROJECT);
    msessionContextMenu->addAction(cmd, Constants::G_SESSION_FILES);
#endif

    // Default open action
    d->m_openFileAction = new QAction(tr("Open File"), this);
    cmd = am->registerAction(d->m_openFileAction, ProjectExplorer::Constants::OPENFILE,
                       globalcontext);
    mfilec->addAction(cmd, Constants::G_FILE_OPEN);

    d->m_showInGraphicalShell = new QAction(FolderNavigationWidget::msgGraphicalShellAction(), this);
    cmd = am->registerAction(d->m_showInGraphicalShell, ProjectExplorer::Constants::SHOWINGRAPHICALSHELL,
                       globalcontext);
    mfilec->addAction(cmd, Constants::G_FILE_OPEN);
    mfolder->addAction(cmd, Constants::G_FOLDER_FILES);

    d->m_openTerminalHere = new QAction(FolderNavigationWidget::msgTerminalAction(), this);
    cmd = am->registerAction(d->m_openTerminalHere, ProjectExplorer::Constants::OPENTERMIANLHERE,
                       globalcontext);
    mfilec->addAction(cmd, Constants::G_FILE_OPEN);
    mfolder->addAction(cmd, Constants::G_FOLDER_FILES);

    // Open With menu
    mfilec->addMenu(openWith, ProjectExplorer::Constants::G_FILE_OPEN);

    // recent projects menu
    Core::ActionContainer *mrecent =
        am->createMenu(Constants::M_RECENTPROJECTS);
    mrecent->menu()->setTitle(tr("Recent P&rojects"));
    mfile->addMenu(mrecent, Core::Constants::G_FILE_OPEN);
    connect(mfile->menu(), SIGNAL(aboutToShow()),
        this, SLOT(updateRecentProjectMenu()));

    // unload action
    d->m_unloadAction = new Utils::ParameterAction(tr("Close Project"), tr("Close Project \"%1\""),
                                                      Utils::ParameterAction::EnabledWithParameter, this);
    cmd = am->registerAction(d->m_unloadAction, Constants::UNLOAD, globalcontext);
    cmd->setAttribute(Core::Command::CA_UpdateText);
    cmd->setDefaultText(d->m_unloadAction->text());
    mfile->addAction(cmd, Core::Constants::G_FILE_PROJECT);
    mproject->addAction(cmd, Constants::G_PROJECT_FILES);

    // unload session action
    d->m_clearSession = new QAction(tr("Close All Projects"), this);
    cmd = am->registerAction(d->m_clearSession, Constants::CLEARSESSION, globalcontext);
    mfile->addAction(cmd, Core::Constants::G_FILE_PROJECT);
    msessionContextMenu->addAction(cmd, Constants::G_SESSION_FILES);

    // session menu
    Core::ActionContainer *msession = am->createMenu(Constants::M_SESSION);
    msession->menu()->setTitle(tr("Session"));
    mfile->addMenu(msession, Core::Constants::G_FILE_PROJECT);
    d->m_sessionMenu = msession->menu();
    connect(mfile->menu(), SIGNAL(aboutToShow()),
            this, SLOT(updateSessionMenu()));

    // build session action
    QIcon buildIcon(Constants::ICON_BUILD);
    buildIcon.addFile(Constants::ICON_BUILD_SMALL);
    d->m_buildSessionAction = new QAction(buildIcon, tr("Build All"), this);
    cmd = am->registerAction(d->m_buildSessionAction, Constants::BUILDSESSION, globalcontext);
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+B")));
    mbuild->addAction(cmd, Constants::G_BUILD_SESSION);
    msessionContextMenu->addAction(cmd, Constants::G_SESSION_BUILD);
    // Add to mode bar
    modeManager->addAction(cmd, Constants::P_ACTION_BUILDSESSION);

    // rebuild session action
    QIcon rebuildIcon(Constants::ICON_REBUILD);
    rebuildIcon.addFile(Constants::ICON_REBUILD_SMALL);
    d->m_rebuildSessionAction = new QAction(rebuildIcon, tr("Rebuild All"), this);
    cmd = am->registerAction(d->m_rebuildSessionAction, Constants::REBUILDSESSION, globalcontext);
    mbuild->addAction(cmd, Constants::G_BUILD_SESSION);
    msessionContextMenu->addAction(cmd, Constants::G_SESSION_BUILD);

    // clean session
    QIcon cleanIcon(Constants::ICON_CLEAN);
    cleanIcon.addFile(Constants::ICON_CLEAN_SMALL);
    d->m_cleanSessionAction = new QAction(cleanIcon, tr("Clean All"), this);
    cmd = am->registerAction(d->m_cleanSessionAction, Constants::CLEANSESSION, globalcontext);
    mbuild->addAction(cmd, Constants::G_BUILD_SESSION);
    msessionContextMenu->addAction(cmd, Constants::G_SESSION_BUILD);

    // build action
    d->m_buildAction = new Utils::ParameterAction(tr("Build Project"), tr("Build Project \"%1\""),
                                                     Utils::ParameterAction::AlwaysEnabled, this);
    cmd = am->registerAction(d->m_buildAction, Constants::BUILD, globalcontext);
    cmd->setAttribute(Core::Command::CA_UpdateText);
    cmd->setDefaultText(d->m_buildAction->text());
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+B")));
    mbuild->addAction(cmd, Constants::G_BUILD_PROJECT);

    // rebuild action
    d->m_rebuildAction = new Utils::ParameterAction(tr("Rebuild Project"), tr("Rebuild Project \"%1\""),
                                                       Utils::ParameterAction::AlwaysEnabled, this);
    cmd = am->registerAction(d->m_rebuildAction, Constants::REBUILD, globalcontext);
    cmd->setAttribute(Core::Command::CA_UpdateText);
    cmd->setDefaultText(d->m_rebuildAction->text());
    mbuild->addAction(cmd, Constants::G_BUILD_PROJECT);

    // clean action
    d->m_cleanAction = new Utils::ParameterAction(tr("Clean Project"), tr("Clean Project \"%1\""),
                                                     Utils::ParameterAction::AlwaysEnabled, this);
    cmd = am->registerAction(d->m_cleanAction, Constants::CLEAN, globalcontext);
    cmd->setAttribute(Core::Command::CA_UpdateText);
    cmd->setDefaultText(d->m_cleanAction->text());
    mbuild->addAction(cmd, Constants::G_BUILD_PROJECT);

    // build action (context menu)
    d->m_buildActionContextMenu = new Utils::ParameterAction(tr("Build Project"), tr("Build Project \"%1\""),
                                                             Utils::ParameterAction::AlwaysEnabled, this);
    cmd = am->registerAction(d->m_buildActionContextMenu, Constants::BUILDCM, globalcontext);
    cmd->setAttribute(Core::Command::CA_UpdateText);
    cmd->setDefaultText(d->m_buildActionContextMenu->text());
    mproject->addAction(cmd, Constants::G_PROJECT_BUILD);

    // rebuild action (context menu)
    d->m_rebuildActionContextMenu = new Utils::ParameterAction(tr("Rebuild Project"), tr("Rebuild Project \"%1\""),
                                                               Utils::ParameterAction::AlwaysEnabled, this);
    cmd = am->registerAction(d->m_rebuildActionContextMenu, Constants::REBUILDCM, globalcontext);
    cmd->setAttribute(Core::Command::CA_UpdateText);
    cmd->setDefaultText(d->m_rebuildActionContextMenu->text());
    mproject->addAction(cmd, Constants::G_PROJECT_BUILD);

    // clean action (context menu)
    d->m_cleanActionContextMenu = new Utils::ParameterAction(tr("Clean Project"), tr("Clean Project \"%1\""),
                                                             Utils::ParameterAction::AlwaysEnabled, this);
    cmd = am->registerAction(d->m_cleanActionContextMenu, Constants::CLEANCM, globalcontext);
    cmd->setAttribute(Core::Command::CA_UpdateText);
    cmd->setDefaultText(d->m_cleanActionContextMenu->text());
    mproject->addAction(cmd, Constants::G_PROJECT_BUILD);

    // build without dependencies action
    d->m_buildProjectOnlyAction = new QAction(tr("Build Without Dependencies"), this);
    cmd = am->registerAction(d->m_buildProjectOnlyAction, Constants::BUILDPROJECTONLY, globalcontext);

    // rebuild without dependencies action
    d->m_rebuildProjectOnlyAction = new QAction(tr("Rebuild Without Dependencies"), this);
    cmd = am->registerAction(d->m_rebuildProjectOnlyAction, Constants::REBUILDPROJECTONLY, globalcontext);

    // clean without dependencies action
    d->m_cleanProjectOnlyAction = new QAction(tr("Clean Without Dependencies"), this);
    cmd = am->registerAction(d->m_cleanProjectOnlyAction, Constants::CLEANPROJECTONLY, globalcontext);

    // run action
    QIcon runIcon(Constants::ICON_RUN);
    runIcon.addFile(Constants::ICON_RUN_SMALL);
    d->m_runAction = new QAction(runIcon, tr("Run"), this);
    cmd = am->registerAction(d->m_runAction, Constants::RUN, globalcontext);
    cmd->setAttribute(Core::Command::CA_UpdateText);

    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+R")));
    mbuild->addAction(cmd, Constants::G_BUILD_RUN);

    modeManager->addAction(cmd, Constants::P_ACTION_RUN);

    d->m_runActionContextMenu = new QAction(runIcon, tr("Run"), this);
    cmd = am->registerAction(d->m_runActionContextMenu, Constants::RUNCONTEXTMENU, globalcontext);
    mproject->addAction(cmd, Constants::G_PROJECT_RUN);

    // cancel build action
    d->m_cancelBuildAction = new QAction(tr("Cancel Build"), this);
    cmd = am->registerAction(d->m_cancelBuildAction, Constants::CANCELBUILD, globalcontext);
    mbuild->addAction(cmd, Constants::G_BUILD_CANCEL);

    // debug action
    QIcon debuggerIcon(":/projectexplorer/images/debugger_start_small.png");
    debuggerIcon.addFile(":/projectexplorer/images/debugger_start.png");
    d->m_debugAction = new QAction(debuggerIcon, tr("Start Debugging"), this);
    cmd = am->registerAction(d->m_debugAction, Constants::DEBUG, globalcontext);
    cmd->setAttribute(Core::Command::CA_UpdateText);
    cmd->setAttribute(Core::Command::CA_UpdateIcon);
    cmd->setDefaultText(tr("Start Debugging"));
    cmd->setDefaultKeySequence(QKeySequence(tr("F5")));
    mstartdebugging->addAction(cmd, Core::Constants::G_DEFAULT_ONE);
    modeManager->addAction(cmd, Constants::P_ACTION_DEBUG);

    // add new file action
    d->m_addNewFileAction = new QAction(tr("Add New..."), this);
    cmd = am->registerAction(d->m_addNewFileAction, ProjectExplorer::Constants::ADDNEWFILE,
                       globalcontext);
    mproject->addAction(cmd, Constants::G_PROJECT_FILES);
    msubProject->addAction(cmd, Constants::G_PROJECT_FILES);
    mfolder->addAction(cmd, Constants::G_FOLDER_FILES);

    // add existing file action
    d->m_addExistingFilesAction = new QAction(tr("Add Existing Files..."), this);
    cmd = am->registerAction(d->m_addExistingFilesAction, ProjectExplorer::Constants::ADDEXISTINGFILES,
                       globalcontext);
    mproject->addAction(cmd, Constants::G_PROJECT_FILES);
    msubProject->addAction(cmd, Constants::G_PROJECT_FILES);
    mfolder->addAction(cmd, Constants::G_FOLDER_FILES);

    // remove file action
    d->m_removeFileAction = new QAction(tr("Remove File..."), this);
    cmd = am->registerAction(d->m_removeFileAction, ProjectExplorer::Constants::REMOVEFILE,
                       globalcontext);
    mfilec->addAction(cmd, Constants::G_FILE_OTHER);

    // renamefile action (TODO: Not supported yet)
    d->m_renameFileAction = new QAction(tr("Rename"), this);
    cmd = am->registerAction(d->m_renameFileAction, ProjectExplorer::Constants::RENAMEFILE,
                       globalcontext);
    mfilec->addAction(cmd, Constants::G_FILE_OTHER);
    d->m_renameFileAction->setEnabled(false);
    d->m_renameFileAction->setVisible(false);

    // target selector
    d->m_projectSelectorAction = new QAction(this);
    d->m_projectSelectorAction->setCheckable(true);
    d->m_projectSelectorAction->setEnabled(false);
    QWidget *mainWindow = Core::ICore::instance()->mainWindow();
    d->m_targetSelector = new Internal::MiniProjectTargetSelector(d->m_projectSelectorAction, mainWindow);
    connect(d->m_projectSelectorAction, SIGNAL(triggered()), d->m_targetSelector, SLOT(show()));
    modeManager->addProjectSelector(d->m_projectSelectorAction);

    d->m_projectSelectorActionMenu = new QAction(this);
    d->m_projectSelectorActionMenu->setEnabled(false);
    d->m_projectSelectorActionMenu->setText(tr("Open Build/Run Target Selector..."));
    connect(d->m_projectSelectorActionMenu, SIGNAL(triggered()), d->m_targetSelector, SLOT(show()));
    cmd = am->registerAction(d->m_projectSelectorActionMenu, ProjectExplorer::Constants::SELECTTARGET,
                       globalcontext);
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+T")));
    mbuild->addAction(cmd, Constants::G_BUILD_RUN);

    connect(d->m_session, SIGNAL(projectAdded(ProjectExplorer::Project*)),
            d->m_targetSelector, SLOT(addProject(ProjectExplorer::Project*)));
    connect(d->m_session, SIGNAL(projectRemoved(ProjectExplorer::Project*)),
            d->m_targetSelector, SLOT(removeProject(ProjectExplorer::Project*)));
    connect(d->m_targetSelector, SIGNAL(startupProjectChanged(ProjectExplorer::Project*)),
            this, SLOT(setStartupProject(ProjectExplorer::Project*)));
    connect(d->m_session, SIGNAL(startupProjectChanged(ProjectExplorer::Project*)),
            d->m_targetSelector, SLOT(changeStartupProject(ProjectExplorer::Project*)));

    connect(core, SIGNAL(saveSettingsRequested()),
        this, SLOT(savePersistentSettings()));

    addAutoReleasedObject(new ProjectTreeWidgetFactory);
    addAutoReleasedObject(new FolderNavigationWidgetFactory);

    if (QSettings *s = core->settings()) {
        const QStringList fileNames = s->value("ProjectExplorer/RecentProjects/FileNames").toStringList();
        const QStringList displayNames = s->value("ProjectExplorer/RecentProjects/DisplayNames").toStringList();
        if (fileNames.size() == displayNames.size()) {
            for (int i = 0; i < fileNames.size(); ++i) {
                if (QFileInfo(fileNames.at(i)).isFile())
                    d->m_recentProjects.append(qMakePair(fileNames.at(i), displayNames.at(i)));
            }
        }
    }

    if (QSettings *s = core->settings()) {
        d->m_projectExplorerSettings.buildBeforeRun = s->value("ProjectExplorer/Settings/BuildBeforeRun", true).toBool();
        d->m_projectExplorerSettings.saveBeforeBuild = s->value("ProjectExplorer/Settings/SaveBeforeBuild", false).toBool();
        d->m_projectExplorerSettings.showCompilerOutput = s->value("ProjectExplorer/Settings/ShowCompilerOutput", false).toBool();
        d->m_projectExplorerSettings.cleanOldAppOutput = s->value("ProjectExplorer/Settings/CleanOldAppOutput", false).toBool();
        d->m_projectExplorerSettings.useJom = s->value("ProjectExplorer/Settings/UseJom", true).toBool();
    }

    connect(d->m_sessionManagerAction, SIGNAL(triggered()), this, SLOT(showSessionManager()));
    connect(d->m_newAction, SIGNAL(triggered()), this, SLOT(newProject()));
#if 0
    connect(d->m_loadAction, SIGNAL(triggered()), this, SLOT(loadAction()));
#endif
    connect(d->m_buildProjectOnlyAction, SIGNAL(triggered()), this, SLOT(buildProjectOnly()));
    connect(d->m_buildAction, SIGNAL(triggered()), this, SLOT(buildProject()));
    connect(d->m_buildActionContextMenu, SIGNAL(triggered()), this, SLOT(buildProjectContextMenu()));
    connect(d->m_buildSessionAction, SIGNAL(triggered()), this, SLOT(buildSession()));
    connect(d->m_rebuildProjectOnlyAction, SIGNAL(triggered()), this, SLOT(rebuildProjectOnly()));
    connect(d->m_rebuildAction, SIGNAL(triggered()), this, SLOT(rebuildProject()));
    connect(d->m_rebuildActionContextMenu, SIGNAL(triggered()), this, SLOT(rebuildProjectContextMenu()));
    connect(d->m_rebuildSessionAction, SIGNAL(triggered()), this, SLOT(rebuildSession()));
    connect(d->m_cleanProjectOnlyAction, SIGNAL(triggered()), this, SLOT(cleanProjectOnly()));
    connect(d->m_cleanAction, SIGNAL(triggered()), this, SLOT(cleanProject()));
    connect(d->m_cleanActionContextMenu, SIGNAL(triggered()), this, SLOT(cleanProjectContextMenu()));
    connect(d->m_cleanSessionAction, SIGNAL(triggered()), this, SLOT(cleanSession()));
    connect(d->m_runAction, SIGNAL(triggered()), this, SLOT(runProject()));
    connect(d->m_runActionContextMenu, SIGNAL(triggered()), this, SLOT(runProjectContextMenu()));
    connect(d->m_cancelBuildAction, SIGNAL(triggered()), this, SLOT(cancelBuild()));
    connect(d->m_debugAction, SIGNAL(triggered()), this, SLOT(debugProject()));
    connect(d->m_unloadAction, SIGNAL(triggered()), this, SLOT(unloadProject()));
    connect(d->m_clearSession, SIGNAL(triggered()), this, SLOT(clearSession()));
    connect(d->m_addNewFileAction, SIGNAL(triggered()), this, SLOT(addNewFile()));
    connect(d->m_addExistingFilesAction, SIGNAL(triggered()), this, SLOT(addExistingFiles()));
    connect(d->m_openFileAction, SIGNAL(triggered()), this, SLOT(openFile()));
    connect(d->m_showInGraphicalShell, SIGNAL(triggered()), this, SLOT(showInGraphicalShell()));
    connect(d->m_openTerminalHere, SIGNAL(triggered()), this, SLOT(openTerminalHere()));
    connect(d->m_removeFileAction, SIGNAL(triggered()), this, SLOT(removeFile()));
    connect(d->m_renameFileAction, SIGNAL(triggered()), this, SLOT(renameFile()));

    updateActions();

    connect(Core::ICore::instance(), SIGNAL(coreAboutToOpen()),
            this, SLOT(determineSessionToRestoreAtStartup()));
    connect(Core::ICore::instance(), SIGNAL(coreOpened()), this, SLOT(restoreSession()));

    updateWelcomePage();

    return true;
}

// Find a factory by file mime type in a sequence of factories
template <class Factory, class Iterator>
    Factory *findFactory(const QString &mimeType, Iterator i1, Iterator i2)
{
    for ( ; i1 != i2; ++i2) {
        Factory *f = *i1;
        if (f->mimeTypes().contains(mimeType))
            return f;
    }
    return 0;
}

ProjectFileFactory * ProjectExplorerPlugin::findProjectFileFactory(const QString &filename) const
{
    // Find factory
    if (const Core::MimeType mt = Core::ICore::instance()->mimeDatabase()->findByFile(QFileInfo(filename)))
        if (ProjectFileFactory *pf = findFactory<ProjectFileFactory>(mt.type(), d->m_fileFactories.constBegin(), d->m_fileFactories.constEnd()))
            return pf;
    qWarning("Unable to find project file factory for '%s'", filename.toUtf8().constData());
    return 0;
}

void ProjectExplorerPlugin::loadAction()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::loadAction";


    QString dir = d->m_lastOpenDirectory;

    // for your special convenience, we preselect a pro file if it is
    // the current file
    if (Core::IEditor *editor = Core::EditorManager::instance()->currentEditor()) {
        if (const Core::IFile *file = editor->file()) {
            const QString fn = file->fileName();
            const bool isProject = d->m_profileMimeTypes.contains(file->mimeType());
            dir = isProject ? fn : QFileInfo(fn).absolutePath();
        }
    }

    QString filename = QFileDialog::getOpenFileName(0, tr("Load Project"),
                                                    dir,
                                                    d->m_projectFilterString);
    if (filename.isEmpty())
        return;
    if (ProjectFileFactory *pf = findProjectFileFactory(filename))
        pf->open(filename);
    updateActions();
}

void ProjectExplorerPlugin::unloadProject()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::unloadProject";

    Core::IFile *fi = d->m_currentProject->file();

    if (!fi || fi->fileName().isEmpty()) //nothing to save?
        return;

    QList<Core::IFile*> filesToSave;
    filesToSave << fi;

    // check the number of modified files
    int readonlycount = 0;
    foreach (const Core::IFile *file, filesToSave) {
        if (file->isReadOnly())
            ++readonlycount;
    }

    bool success = false;
    if (readonlycount > 0)
        success = Core::ICore::instance()->fileManager()->saveModifiedFiles(filesToSave).isEmpty();
    else
        success = Core::ICore::instance()->fileManager()->saveModifiedFilesSilently(filesToSave).isEmpty();

    if (!success)
        return;

    addToRecentProjects(fi->fileName(), d->m_currentProject->displayName());
    d->m_session->removeProject(d->m_currentProject);
    updateActions();
}

void ProjectExplorerPlugin::clearSession()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::clearSession";

    if (!d->m_session->clear())
        return; // Action has been cancelled
    updateActions();
}

void ProjectExplorerPlugin::extensionsInitialized()
{
    d->m_fileFactories = ProjectFileFactory::createFactories(&d->m_projectFilterString);
    foreach (ProjectFileFactory *pf, d->m_fileFactories) {
        d->m_profileMimeTypes += pf->mimeTypes();
        addAutoReleasedObject(pf);
    }
    // Add custom wizards, for which other plugins might have registered
    // class factories
    foreach(Core::IWizard *cpw, ProjectExplorer::CustomWizard::createWizards())
        addAutoReleasedObject(cpw);
}

void ProjectExplorerPlugin::aboutToShutdown()
{
    d->m_proWindow->aboutToShutdown(); // disconnect from session
    d->m_session->clear();
    d->m_projectsMode = 0;
//    d->m_proWindow->saveConfigChanges();
}

void ProjectExplorerPlugin::newProject()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::newProject";

    Core::ICore::instance()->showNewItemDialog(tr("New Project", "Title of dialog"),
                              Core::IWizard::wizardsOfKind(Core::IWizard::ProjectWizard));
    updateActions();
}

void ProjectExplorerPlugin::showSessionManager()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::showSessionManager";

    if (d->m_session->isDefaultVirgin()) {
        // do not save new virgin default sessions
    } else {
        d->m_session->save();
    }
    SessionDialog sessionDialog(d->m_session);
    sessionDialog.exec();

    updateActions();

    Core::ModeManager *modeManager = Core::ModeManager::instance();
    Core::IMode *welcomeMode = modeManager->mode(Core::Constants::MODE_WELCOME);
    if (modeManager->currentMode() == welcomeMode)
        updateWelcomePage();
}

void ProjectExplorerPlugin::setStartupProject(Project *project)
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::setStartupProject";

    if (!project)
        return;
    d->m_session->setStartupProject(project);
    updateActions();
}

void ProjectExplorerPlugin::savePersistentSettings()
{
    if (debug)
        qDebug()<<"ProjectExplorerPlugin::savePersistentSettings()";

    foreach (Project *pro, d->m_session->projects())
        pro->saveSettings();

    if (d->m_session->isDefaultVirgin()) {
        // do not save new virgin default sessions
    } else {
        d->m_session->save();
    }

    QSettings *s = Core::ICore::instance()->settings();
    if (s) {
        s->setValue("ProjectExplorer/StartupSession", d->m_session->file()->fileName());
        s->remove("ProjectExplorer/RecentProjects/Files");

        QStringList fileNames;
        QStringList displayNames;
        QList<QPair<QString, QString> >::const_iterator it, end;
        end = d->m_recentProjects.constEnd();
        for (it = d->m_recentProjects.constBegin(); it != end; ++it) {
            fileNames << (*it).first;
            displayNames << (*it).second;
        }

        s->setValue("ProjectExplorer/RecentProjects/FileNames", fileNames);
        s->setValue("ProjectExplorer/RecentProjects/DisplayNames", displayNames);

        s->setValue("ProjectExplorer/Settings/BuildBeforeRun", d->m_projectExplorerSettings.buildBeforeRun);
        s->setValue("ProjectExplorer/Settings/SaveBeforeBuild", d->m_projectExplorerSettings.saveBeforeBuild);
        s->setValue("ProjectExplorer/Settings/ShowCompilerOutput", d->m_projectExplorerSettings.showCompilerOutput);
        s->setValue("ProjectExplorer/Settings/CleanOldAppOutput", d->m_projectExplorerSettings.cleanOldAppOutput);
        s->setValue("ProjectExplorer/Settings/UseJom", d->m_projectExplorerSettings.useJom);
    }
}

bool ProjectExplorerPlugin::openProject(const QString &fileName)
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::openProject";

    QList<Project *> list = openProjects(QStringList() << fileName);
    if (!list.isEmpty()) {
        addToRecentProjects(fileName, list.first()->displayName());
        return true;
    }
    return false;
}

static inline QList<IProjectManager*> allProjectManagers()
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    return pm->getObjects<IProjectManager>();
}

QList<Project *> ProjectExplorerPlugin::openProjects(const QStringList &fileNames)
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin - opening projects " << fileNames;

    const QList<IProjectManager*> projectManagers = allProjectManagers();

    QList<Project*> openedPro;
    foreach (const QString &fileName, fileNames) {
        if (const Core::MimeType mt = Core::ICore::instance()->mimeDatabase()->findByFile(QFileInfo(fileName))) {
            foreach (IProjectManager *manager, projectManagers) {
                if (manager->mimeType() == mt.type()) {
                    if (Project *pro = manager->openProject(fileName)) {
                        if (pro->restoreSettings()) {
                            connect(pro, SIGNAL(fileListChanged()), this, SIGNAL(fileListChanged()));
                            d->m_session->addProject(pro);
                            // Make sure we always have a current project / node
                            if (!d->m_currentProject && !openedPro.isEmpty())
                                setCurrentNode(pro->rootProjectNode());
                            openedPro += pro;
                        } else {
                            delete pro;
                        }
                    }
                    d->m_session->reportProjectLoadingProgress();
                    break;
                }
            }
        }
    }
    updateActions();

    if (openedPro.isEmpty()) {
        qDebug() << "ProjectExplorerPlugin - Could not open any projects!";
    } else {
        Core::ModeManager::instance()->activateMode(Core::Constants::MODE_EDIT);
    }

    return openedPro;
}

Project *ProjectExplorerPlugin::currentProject() const
{
    if (debug) {
        if (d->m_currentProject)
            qDebug() << "ProjectExplorerPlugin::currentProject returns " << d->m_currentProject->displayName();
        else
            qDebug() << "ProjectExplorerPlugin::currentProject returns 0";
    }
    return d->m_currentProject;
}

Node *ProjectExplorerPlugin::currentNode() const
{
    return d->m_currentNode;
}

void ProjectExplorerPlugin::setCurrentFile(Project *project, const QString &filePath)
{
    setCurrent(project, filePath, 0);
}

void ProjectExplorerPlugin::setCurrentFile(const QString &filePath)
{
    Project *project = d->m_session->projectForFile(filePath);
    // If the file is not in any project, stay with the current project
    // e.g. on opening a git diff buffer, git log buffer, we don't change the project
    // I'm not 100% sure this is correct
    if (!project)
        project = d->m_currentProject;
    setCurrent(project, filePath, 0);
}

void ProjectExplorerPlugin::setCurrentNode(Node *node)
{
    setCurrent(d->m_session->projectForNode(node), QString(), node);
}

SessionManager *ProjectExplorerPlugin::session() const
{
    return d->m_session;
}

Project *ProjectExplorerPlugin::startupProject() const
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::startupProject";

    return d->m_session->startupProject();
}

void ProjectExplorerPlugin::updateWelcomePage()
{
    ProjectWelcomePageWidget::WelcomePageData welcomePageData;
    welcomePageData.sessionList =  d->m_session->sessions();
    welcomePageData.activeSession = d->m_session->activeSession();
    welcomePageData.previousSession = d->m_session->lastSession();
    welcomePageData.projectList = d->m_recentProjects;
    d->m_welcomePage->setWelcomePageData(welcomePageData);
}

void ProjectExplorerPlugin::currentModeChanged(Core::IMode *mode, Core::IMode *oldMode)
{
    if (mode && mode->id() == QLatin1String(Core::Constants::MODE_WELCOME))
        updateWelcomePage();
    if (oldMode == d->m_projectsMode)
        savePersistentSettings();
}

void ProjectExplorerPlugin::determineSessionToRestoreAtStartup()
{
    QStringList sessions = d->m_session->sessions();
    // We have command line arguments, try to find a session in them
    QStringList arguments = ExtensionSystem::PluginManager::instance()->arguments();
    // Default to no session loading
    d->m_sessionToRestoreAtStartup.clear();
    foreach (const QString &arg, arguments) {
        if (sessions.contains(arg)) {
            // Session argument
            d->m_sessionToRestoreAtStartup = arg;
            break;
        }
    }
    if (!d->m_sessionToRestoreAtStartup.isNull())
        Core::ICore::instance()->modeManager()->activateMode(Core::Constants::MODE_EDIT);
}

/*!
    \fn void ProjectExplorerPlugin::restoreSession()

    This method is connected to the ICore::coreOpened signal.  If
    there was no session explicitly loaded, it creates an empty new
    default session and puts the list of recent projects and sessions
    onto the welcome page.
*/
void ProjectExplorerPlugin::restoreSession()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::restoreSession";

    // We have command line arguments, try to find a session in them
    QStringList arguments = ExtensionSystem::PluginManager::instance()->arguments();
    arguments.removeOne(d->m_sessionToRestoreAtStartup);

    // Restore latest session or what was passed on the command line
    if (d->m_sessionToRestoreAtStartup.isEmpty()) {
        d->m_session->createAndLoadNewDefaultSession();
    } else {
        d->m_session->loadSession(d->m_sessionToRestoreAtStartup);
    }

    // update welcome page
    Core::ModeManager *modeManager = Core::ModeManager::instance();
    connect(modeManager, SIGNAL(currentModeChanged(Core::IMode*, Core::IMode*)),
            this, SLOT(currentModeChanged(Core::IMode*, Core::IMode*)));
    connect(d->m_welcomePage, SIGNAL(requestSession(QString)), this, SLOT(loadSession(QString)));
    connect(d->m_welcomePage, SIGNAL(requestProject(QString)), this, SLOT(loadProject(QString)));

    Core::ICore::instance()->openFiles(arguments);
    updateActions();

}

void ProjectExplorerPlugin::loadSession(const QString &session)
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::loadSession" << session;
    d->m_session->loadSession(session);
}


void ProjectExplorerPlugin::showContextMenu(const QPoint &globalPos, Node *node)
{
    QMenu *contextMenu = 0;

    if (!node)
        node = d->m_session->sessionNode();

    if (node->nodeType() != SessionNodeType) {
        Project *project = d->m_session->projectForNode(node);
        setCurrentNode(node);

        emit aboutToShowContextMenu(project, node);
        switch (node->nodeType()) {
        case ProjectNodeType:
            if (node->parentFolderNode() == d->m_session->sessionNode())
                contextMenu = d->m_projectMenu;
            else
                contextMenu = d->m_subProjectMenu;
            break;
        case FolderNodeType:
            contextMenu = d->m_folderMenu;
            break;
        case FileNodeType:
            populateOpenWithMenu();
            contextMenu = d->m_fileMenu;
            break;
        default:
            qWarning("ProjectExplorerPlugin::showContextMenu - Missing handler for node type");
        }
    } else { // session item
        emit aboutToShowContextMenu(0, node);

        contextMenu = d->m_sessionContextMenu;
    }

    updateContextMenuActions(node);
    if (contextMenu && contextMenu->actions().count() > 0) {
        contextMenu->popup(globalPos);
    }
}

BuildManager *ProjectExplorerPlugin::buildManager() const
{
    return d->m_buildManager;
}

void ProjectExplorerPlugin::buildStateChanged(Project * pro)
{
    if (debug) {
        qDebug() << "buildStateChanged";
        qDebug() << pro->file()->fileName() << "isBuilding()" << d->m_buildManager->isBuilding(pro);
    }
    Q_UNUSED(pro)
    updateActions();
}

void ProjectExplorerPlugin::executeRunConfiguration(RunConfiguration *runConfiguration, const QString &runMode)
{
    if (IRunControlFactory *runControlFactory = findRunControlFactory(runConfiguration, runMode)) {
        emit aboutToExecuteProject(runConfiguration->target()->project(), runMode);

        RunControl *control = runControlFactory->create(runConfiguration, runMode);
        startRunControl(control, runMode);
    }

}

void ProjectExplorerPlugin::startRunControl(RunControl *runControl, const QString &runMode)
{
    d->m_outputPane->createNewOutputWindow(runControl);
    if (runMode == ProjectExplorer::Constants::RUNMODE)
        d->m_outputPane->popup(false);
    d->m_outputPane->showTabFor(runControl);
    if (projectExplorerSettings().cleanOldAppOutput)
        d->m_outputPane->clearContents();

    connect(runControl, SIGNAL(addToOutputWindow(RunControl *, const QString &, bool)),
            d->m_outputPane, SLOT(appendApplicationOutput(RunControl*,const QString &, bool)));
    connect(runControl, SIGNAL(addToOutputWindowInline(RunControl *, const QString &, bool)),
            d->m_outputPane, SLOT(appendApplicationOutputInline(RunControl*,const QString &, bool)));
    connect(runControl, SIGNAL(appendMessage(RunControl*,QString,bool)),
            d->m_outputPane, SLOT(appendMessage(RunControl *, const QString &, bool)));

    connect(runControl, SIGNAL(finished()),
            this, SLOT(runControlFinished()));

    if (runMode == ProjectExplorer::Constants::DEBUGMODE)
        d->m_debuggingRunControl = runControl;

    runControl->start();
    updateRunActions();
}

void ProjectExplorerPlugin::buildQueueFinished(bool success)
{
    if (debug)
        qDebug() << "buildQueueFinished()" << success;

    updateActions();

    if (success && d->m_delayedRunConfiguration) {
        executeRunConfiguration(d->m_delayedRunConfiguration, d->m_runMode);
    } else {
        if (d->m_buildManager->tasksAvailable())
            d->m_buildManager->showTaskWindow();
    }
    d->m_delayedRunConfiguration = 0;
    d->m_runMode.clear();
}

void ProjectExplorerPlugin::setCurrent(Project *project, QString filePath, Node *node)
{
    if (debug)
        qDebug() << "ProjectExplorer - setting path to " << (node ? node->path() : filePath)
                << " and project to " << (project ? project->displayName() : "0");

    if (node)
        filePath = node->path();
    else
        node = d->m_session->nodeForFile(filePath, project);

    Core::ICore *core = Core::ICore::instance();

    bool projectChanged = false;
    if (d->m_currentProject != project) {
        QList<int> oldContext;
        QList<int> newContext;

        if (d->m_currentProject) {
            oldContext.append(d->m_currentProject->projectManager()->projectContext());
            oldContext.append(d->m_currentProject->projectManager()->projectLanguage());
        }
        if (project) {
            newContext.append(project->projectManager()->projectContext());
            newContext.append(project->projectManager()->projectLanguage());
        }

        core->updateAdditionalContexts(oldContext, newContext);

        d->m_currentProject = project;

        projectChanged = true;
    }

    if (projectChanged || d->m_currentNode != node) {
        d->m_currentNode = node;
        if (debug)
            qDebug() << "ProjectExplorer - currentNodeChanged(" << (node ? node->path() : "0") << ", " << (project ? project->displayName() : "0") << ")";
        emit currentNodeChanged(d->m_currentNode, project);
    }
    if (projectChanged) {
        if (debug)
            qDebug() << "ProjectExplorer - currentProjectChanged(" << (project ? project->displayName() : "0") << ")";
        emit currentProjectChanged(project);
        updateActions();
    }

    core->fileManager()->setCurrentFile(filePath);
}

void ProjectExplorerPlugin::updateActions()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::updateActions";

    Project *startupProject = session()->startupProject();
    bool enableBuildActions = startupProject
                              && ! (d->m_buildManager->isBuilding(startupProject))
                              && hasBuildSettings(startupProject);

    bool enableBuildActionsContextMenu = d->m_currentProject
                              && ! (d->m_buildManager->isBuilding(d->m_currentProject))
                              && hasBuildSettings(d->m_currentProject);


    bool hasProjects = !d->m_session->projects().isEmpty();
    bool building = d->m_buildManager->isBuilding();
    QString projectName = startupProject ? startupProject->displayName() : QString();
    QString projectNameContextMenu = d->m_currentProject ? d->m_currentProject->displayName() : QString();

    if (debug)
        qDebug() << "BuildManager::isBuilding()" << building;

    d->m_unloadAction->setParameter(projectNameContextMenu);

    d->m_buildAction->setParameter(projectName);
    d->m_rebuildAction->setParameter(projectName);
    d->m_cleanAction->setParameter(projectName);

    d->m_buildAction->setEnabled(enableBuildActions);
    d->m_rebuildAction->setEnabled(enableBuildActions);
    d->m_cleanAction->setEnabled(enableBuildActions);

    d->m_buildActionContextMenu->setParameter(projectNameContextMenu);
    d->m_rebuildActionContextMenu->setParameter(projectNameContextMenu);
    d->m_cleanActionContextMenu->setParameter(projectNameContextMenu);

    d->m_buildActionContextMenu->setEnabled(enableBuildActionsContextMenu);
    d->m_rebuildActionContextMenu->setEnabled(enableBuildActionsContextMenu);
    d->m_cleanActionContextMenu->setEnabled(enableBuildActionsContextMenu);

    d->m_buildProjectOnlyAction->setEnabled(enableBuildActions);
    d->m_rebuildProjectOnlyAction->setEnabled(enableBuildActions);
    d->m_cleanProjectOnlyAction->setEnabled(enableBuildActions);

    d->m_clearSession->setEnabled(hasProjects && !building);
    d->m_buildSessionAction->setEnabled(hasProjects && !building);
    d->m_rebuildSessionAction->setEnabled(hasProjects && !building);
    d->m_cleanSessionAction->setEnabled(hasProjects && !building);
    d->m_cancelBuildAction->setEnabled(building);

    d->m_projectSelectorAction->setEnabled(!session()->projects().isEmpty());
    d->m_projectSelectorActionMenu->setEnabled(!session()->projects().isEmpty());

    updateRunActions();
}

// NBS TODO check projectOrder()
// what we want here is all the projects pro depends on
QStringList ProjectExplorerPlugin::allFilesWithDependencies(Project *pro)
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::allFilesWithDependencies(" << pro->file()->fileName() << ")";

    QStringList filesToSave;
    foreach (Project *p, d->m_session->projectOrder(pro)) {
        FindAllFilesVisitor filesVisitor;
        p->rootProjectNode()->accept(&filesVisitor);
        filesToSave << filesVisitor.filePaths();
    }
    qSort(filesToSave);
    return filesToSave;
}

bool ProjectExplorerPlugin::saveModifiedFiles()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::saveModifiedFiles";

    QList<Core::IFile *> filesToSave = Core::ICore::instance()->fileManager()->modifiedFiles();
    if (!filesToSave.isEmpty()) {
        if (d->m_projectExplorerSettings.saveBeforeBuild) {
            Core::ICore::instance()->fileManager()->saveModifiedFilesSilently(filesToSave);
        } else {
            bool cancelled = false;
            bool alwaysSave = false;

            Core::FileManager *fm = Core::ICore::instance()->fileManager();
            fm->saveModifiedFiles(filesToSave, &cancelled, QString(),
                                  tr("Always save files before build"), &alwaysSave);

            if (cancelled)
                return false;
            if (alwaysSave)
                d->m_projectExplorerSettings.saveBeforeBuild = true;
        }
    }
    return true;
}

//NBS handle case where there is no activeBuildConfiguration
// because someone delete all build configurations

void ProjectExplorerPlugin::buildProjectOnly()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::buildProjectOnly";

    if (saveModifiedFiles())
        buildManager()->buildProject(session()->startupProject()->activeTarget()->activeBuildConfiguration());
}

void ProjectExplorerPlugin::buildProject()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::buildProject";

    if (saveModifiedFiles()) {
        QList<BuildConfiguration *> configurations;
        foreach (Project *pro, d->m_session->projectOrder(session()->startupProject()))
            if (pro->activeTarget()->activeBuildConfiguration())
                configurations << pro->activeTarget()->activeBuildConfiguration();

        d->m_buildManager->buildProjects(configurations);
    }
}

void ProjectExplorerPlugin::buildProjectContextMenu()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::buildProjectContextMenu";

    if (saveModifiedFiles()) {
        QList<BuildConfiguration *> configurations;
        foreach (Project *pro, d->m_session->projectOrder(d->m_currentProject))
            if (pro->activeTarget()->activeBuildConfiguration())
                configurations << pro->activeTarget()->activeBuildConfiguration();

        d->m_buildManager->buildProjects(configurations);
    }
}

void ProjectExplorerPlugin::buildSession()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::buildSession";

    if (saveModifiedFiles()) {
        QList<BuildConfiguration *> configurations;
        foreach (Project *pro, d->m_session->projectOrder())
            if (pro->activeTarget()->activeBuildConfiguration())
                configurations << pro->activeTarget()->activeBuildConfiguration();
        d->m_buildManager->buildProjects(configurations);
    }
}

void ProjectExplorerPlugin::rebuildProjectOnly()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::rebuildProjectOnly";

    if (saveModifiedFiles()) {
        d->m_buildManager->cleanProject(session()->startupProject()->activeTarget()->activeBuildConfiguration());
        d->m_buildManager->buildProject(session()->startupProject()->activeTarget()->activeBuildConfiguration());
    }
}

void ProjectExplorerPlugin::rebuildProject()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::rebuildProject";

    if (saveModifiedFiles()) {
        const QList<Project *> &projects = d->m_session->projectOrder(session()->startupProject());
        QList<BuildConfiguration *> configurations;
        foreach (Project *pro, projects)
            if (pro->activeTarget()->activeBuildConfiguration())
                configurations << pro->activeTarget()->activeBuildConfiguration();

        d->m_buildManager->cleanProjects(configurations);
        d->m_buildManager->buildProjects(configurations);
    }
}

void ProjectExplorerPlugin::rebuildProjectContextMenu()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::rebuildProjectContextMenu";

    if (saveModifiedFiles()) {
        const QList<Project *> &projects = d->m_session->projectOrder(d->m_currentProject);
        QList<BuildConfiguration *> configurations;
        foreach (Project *pro, projects)
            if (pro->activeTarget()->activeBuildConfiguration())
                configurations << pro->activeTarget()->activeBuildConfiguration();

        d->m_buildManager->cleanProjects(configurations);
        d->m_buildManager->buildProjects(configurations);
    }
}

void ProjectExplorerPlugin::rebuildSession()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::rebuildSession";

    if (saveModifiedFiles()) {
        const QList<Project *> & projects = d->m_session->projectOrder();
        QList<BuildConfiguration *> configurations;
        foreach (Project *pro, projects)
            if (pro->activeTarget()->activeBuildConfiguration())
                configurations << pro->activeTarget()->activeBuildConfiguration();

        d->m_buildManager->cleanProjects(configurations);
        d->m_buildManager->buildProjects(configurations);
    }
}

void ProjectExplorerPlugin::cleanProjectOnly()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::cleanProjectOnly";

    if (saveModifiedFiles())
        d->m_buildManager->cleanProject(session()->startupProject()->activeTarget()->activeBuildConfiguration());
}

void ProjectExplorerPlugin::cleanProject()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::cleanProject";

    if (saveModifiedFiles()) {
        const QList<Project *> & projects = d->m_session->projectOrder(session()->startupProject());
        QList<BuildConfiguration *> configurations;
        foreach (Project *pro, projects)
            if (pro->activeTarget()->activeBuildConfiguration())
                configurations << pro->activeTarget()->activeBuildConfiguration();
        d->m_buildManager->cleanProjects(configurations);
    }
}

void ProjectExplorerPlugin::cleanProjectContextMenu()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::cleanProjectContextMenu";

    if (saveModifiedFiles()) {
        const QList<Project *> & projects = d->m_session->projectOrder(d->m_currentProject);
        QList<BuildConfiguration *> configurations;
        foreach (Project *pro, projects)
            if (pro->activeTarget()->activeBuildConfiguration())
                configurations << pro->activeTarget()->activeBuildConfiguration();
        d->m_buildManager->cleanProjects(configurations);
    }
}

void ProjectExplorerPlugin::cleanSession()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::cleanSession";

    if (saveModifiedFiles()) {
        const QList<Project *> & projects = d->m_session->projectOrder();
        QList<BuildConfiguration *> configurations;
        foreach (Project *pro, projects)
            if (pro->activeTarget()->activeBuildConfiguration())
                configurations << pro->activeTarget()->activeBuildConfiguration();
        d->m_buildManager->cleanProjects(configurations);
    }
}

void ProjectExplorerPlugin::runProject()
{
    runProjectImpl(startupProject(), ProjectExplorer::Constants::RUNMODE);
}

void ProjectExplorerPlugin::runProjectContextMenu()
{
    runProjectImpl(d->m_currentProject, ProjectExplorer::Constants::RUNMODE);
}

bool ProjectExplorerPlugin::hasBuildSettings(Project *pro)
{
    const QList<Project *> & projects = d->m_session->projectOrder(pro);
    foreach(Project *pro, projects)
        if (pro->activeTarget()->activeBuildConfiguration())
            return true;
    return false;
}

void ProjectExplorerPlugin::runProjectImpl(Project *pro, QString mode)
{
    if (!pro)
        return;

    if (d->m_projectExplorerSettings.buildBeforeRun && hasBuildSettings(pro)) {
        if (!pro->activeTarget()->activeRunConfiguration()->isEnabled()) {
            if (!showBuildConfigDialog())
                return;
        }
        if (saveModifiedFiles()) {
            d->m_runMode = mode;
            d->m_delayedRunConfiguration = pro->activeTarget()->activeRunConfiguration();

            const QList<Project *> & projects = d->m_session->projectOrder(pro);
            QList<BuildConfiguration *> configurations;
            foreach(Project *pro, projects)
                if (pro->activeTarget()->activeBuildConfiguration())
                    configurations << pro->activeTarget()->activeBuildConfiguration();
            d->m_buildManager->buildProjects(configurations);

            updateRunActions();
        }
    } else {
        // TODO this ignores RunConfiguration::isEnabled()
        if (saveModifiedFiles())
            executeRunConfiguration(pro->activeTarget()->activeRunConfiguration(), mode);
    }
}

void ProjectExplorerPlugin::debugProject()
{
    Project *pro = startupProject();
    if (!pro || d->m_debuggingRunControl )
        return;

    runProjectImpl(pro, ProjectExplorer::Constants::DEBUGMODE);
}

bool ProjectExplorerPlugin::showBuildConfigDialog()
{
    Project *pro = startupProject();
    BuildConfigDialog *dialog = new BuildConfigDialog(pro,
                                                      Core::ICore::instance()->mainWindow());
    dialog->exec();
    BuildConfiguration *otherConfig = dialog->selectedBuildConfiguration();
    int result = dialog->result();
    dialog->deleteLater();
    switch (result) {
    case BuildConfigDialog::ChangeBuild:
        if (otherConfig) {
            pro->activeTarget()->setActiveBuildConfiguration(otherConfig);
            return true;
        }
        return false;
    case BuildConfigDialog::Cancel:
        return false;
    case BuildConfigDialog::Continue:
        return true;
    default:
        return false;
    }
}

void ProjectExplorerPlugin::runControlFinished()
{
    if (sender() == d->m_debuggingRunControl)
        d->m_debuggingRunControl = 0;

    updateRunActions();
}

void ProjectExplorerPlugin::startupProjectChanged()
{
    static QPointer<Project> previousStartupProject = 0;
    Project *project = startupProject();
    if (project == previousStartupProject)
        return;

    if (d->m_projectsMode)
        d->m_projectsMode->setEnabled(project);

    if (previousStartupProject) {
        disconnect(previousStartupProject->activeTarget(), SIGNAL(activeRunConfigurationChanged(ProjectExplorer::RunConfiguration*)),
                this, SLOT(updateRunActions()));

        disconnect(previousStartupProject, SIGNAL(activeTargetChanged(ProjectExplorer::Target*)),
                   this, SLOT(updateRunActions()));
        disconnect(previousStartupProject->activeTarget()->activeRunConfiguration(),
                   SIGNAL(isEnabledChanged(bool)), this, SLOT(updateRunActions()));

        foreach (Target *t, previousStartupProject->targets())
            disconnect(t, SIGNAL(activeRunConfigurationChanged(ProjectExplorer::RunConfiguration*)),
                       this, SLOT(updateActions()));
    }

    previousStartupProject = project;

    if (project) {
        connect(project, SIGNAL(activeTargetChanged(ProjectExplorer::Target*)),
                this, SLOT(updateRunActions()));

        connect(project->activeTarget(), SIGNAL(activeRunConfigurationChanged(ProjectExplorer::RunConfiguration*)),
                this, SLOT(updateRunActions()));

        if (project->activeTarget()->activeRunConfiguration()) {
            connect(project->activeTarget()->activeRunConfiguration(), SIGNAL(isEnabledChanged(bool)),
                    this, SLOT(updateRunActions()));
            foreach (Target *t, project->targets())
                connect(t, SIGNAL(activeRunConfigurationChanged(ProjectExplorer::RunConfiguration*)),
                    this, SLOT(updateActions()));
        }
    }

    updateRunActions();
}

// NBS TODO implement more than one runner
IRunControlFactory *ProjectExplorerPlugin::findRunControlFactory(RunConfiguration *config, const QString &mode)
{
    ExtensionSystem::PluginManager *pm = ExtensionSystem::PluginManager::instance();
    const QList<IRunControlFactory *> factories = pm->getObjects<IRunControlFactory>();
    foreach (IRunControlFactory *f, factories)
        if (f->canRun(config, mode))
            return f;
    return 0;
}

void ProjectExplorerPlugin::updateRunActions()
{
    const Project *project = startupProject();

    if (!project ||
        !project->activeTarget() ||
        !project->activeTarget()->activeRunConfiguration()) {

        d->m_runAction->setToolTip(tr("Cannot run without a project."));
        d->m_debugAction->setToolTip(tr("Cannot debug without a project."));

        d->m_runAction->setEnabled(false);
        d->m_debugAction->setEnabled(false);
        return;
    }
    d->m_runAction->setToolTip(QString());
    d->m_debugAction->setToolTip(QString());

    RunConfiguration *activeRC = project->activeTarget()->activeRunConfiguration();

    bool canRun = findRunControlFactory(activeRC, ProjectExplorer::Constants::RUNMODE)
                  && activeRC->isEnabled();
    const bool canDebug = !d->m_debuggingRunControl && findRunControlFactory(activeRC, ProjectExplorer::Constants::DEBUGMODE)
                          && activeRC->isEnabled();
    const bool building = d->m_buildManager->isBuilding();

    d->m_runAction->setEnabled(canRun && !building);

    canRun = session()->startupProject() && findRunControlFactory(activeRC, ProjectExplorer::Constants::RUNMODE);

    d->m_runActionContextMenu->setEnabled(canRun && !building);

    d->m_debugAction->setEnabled(canDebug && !building);

}

void ProjectExplorerPlugin::cancelBuild()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::cancelBuild";

    if (d->m_buildManager->isBuilding())
        d->m_buildManager->cancel();
}

void ProjectExplorerPlugin::addToRecentProjects(const QString &fileName, const QString &displayName)
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::addToRecentProjects(" << fileName << ")";

    if (fileName.isEmpty())
        return;
    QString prettyFileName(QDir::toNativeSeparators(fileName));

    QList<QPair<QString, QString> >::iterator it;
    for(it = d->m_recentProjects.begin(); it != d->m_recentProjects.end();)
        if ((*it).first == prettyFileName)
            it = d->m_recentProjects.erase(it);
        else
            ++it;

    if (d->m_recentProjects.count() > d->m_maxRecentProjects)
        d->m_recentProjects.removeLast();
    d->m_recentProjects.prepend(qMakePair(prettyFileName, displayName));
    QFileInfo fi(prettyFileName);
    d->m_lastOpenDirectory = fi.absolutePath();
}

void ProjectExplorerPlugin::updateRecentProjectMenu()
{
    typedef QList<QPair<QString, QString> >::const_iterator StringPairListConstIterator;
    if (debug)
        qDebug() << "ProjectExplorerPlugin::updateRecentProjectMenu";

    Core::ActionContainer *aci =
        Core::ICore::instance()->actionManager()->actionContainer(Constants::M_RECENTPROJECTS);
    QMenu *menu = aci->menu();
    menu->clear();

    menu->setEnabled(!d->m_recentProjects.isEmpty());

    //projects (ignore sessions, they used to be in this list)
    const StringPairListConstIterator end = d->m_recentProjects.constEnd();
    for (StringPairListConstIterator it = d->m_recentProjects.constBegin(); it != end; ++it) {
        const QPair<QString, QString> &s = *it;
        if (s.first.endsWith(QLatin1String(".qws")))
            continue;
        QAction *action = menu->addAction(s.first);
        action->setData(s.first);
        connect(action, SIGNAL(triggered()), this, SLOT(openRecentProject()));
    }
}

void ProjectExplorerPlugin::openRecentProject()
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::openRecentProject()";

    QAction *a = qobject_cast<QAction*>(sender());
    if (!a)
        return;
    QString fileName = a->data().toString();
    if (!fileName.isEmpty())
        openProject(fileName);
}

void ProjectExplorerPlugin::invalidateProject(Project *project)
{
    if (debug)
        qDebug() << "ProjectExplorerPlugin::invalidateProject" << project->displayName();
    if (d->m_currentProject == project) {
        //
        // Workaround for a bug in QItemSelectionModel
        // - currentChanged etc are not emitted if the
        // item is removed from the underlying data model
        //
        setCurrent(0, QString(), 0);
    }

    disconnect(project, SIGNAL(fileListChanged()), this, SIGNAL(fileListChanged()));
    updateActions();
}

void ProjectExplorerPlugin::goToTaskWindow()
{
    d->m_buildManager->gotoTaskWindow();
}

void ProjectExplorerPlugin::updateContextMenuActions(Node *node)
{
    d->m_addExistingFilesAction->setEnabled(false);
    d->m_addNewFileAction->setEnabled(false);
    d->m_removeFileAction->setEnabled(false);

    if (node->projectNode()) {
        QList<ProjectNode::ProjectAction> supportedActions = node->projectNode()->supportedActions();
        if (qobject_cast<FolderNode*>(node)) {
            const bool addFilesEnabled = supportedActions.contains(ProjectNode::AddFile);
            d->m_addExistingFilesAction->setEnabled(addFilesEnabled);
            d->m_addNewFileAction->setEnabled(addFilesEnabled);
        } else if (qobject_cast<FileNode*>(node)) {
            const bool removeFileEnabled = supportedActions.contains(ProjectNode::RemoveFile);
            d->m_removeFileAction->setEnabled(removeFileEnabled);
        }
    }
}

void ProjectExplorerPlugin::addNewFile()
{
    QTC_ASSERT(d->m_currentNode, return)
    QFileInfo fi(d->m_currentNode->path());
    const QString location = (fi.isDir() ? fi.absoluteFilePath() : fi.absolutePath());
    Core::ICore::instance()->showNewItemDialog(tr("New File", "Title of dialog"),
                              Core::IWizard::wizardsOfKind(Core::IWizard::FileWizard)
                              + Core::IWizard::wizardsOfKind(Core::IWizard::ClassWizard),
                              location);
}

void ProjectExplorerPlugin::addExistingFiles()
{
    QTC_ASSERT(d->m_currentNode, return)

    ProjectNode *projectNode = qobject_cast<ProjectNode*>(d->m_currentNode->projectNode());
    Core::ICore *core = Core::ICore::instance();
    QFileInfo fi(d->m_currentNode->path());
    const QString dir = (fi.isDir() ? fi.absoluteFilePath() : fi.absolutePath());
    QStringList fileNames = QFileDialog::getOpenFileNames(core->mainWindow(), tr("Add Existing Files"), dir);
    if (fileNames.isEmpty())
        return;

    QHash<FileType, QString> fileTypeToFiles;
    foreach (const QString &fileName, fileNames) {
        FileType fileType = typeForFileName(core->mimeDatabase(), QFileInfo(fileName));
        fileTypeToFiles.insertMulti(fileType, fileName);
    }

    QStringList notAdded;
    foreach (const FileType type, fileTypeToFiles.uniqueKeys()) {
        projectNode->addFiles(type, fileTypeToFiles.values(type), &notAdded);
    }
    if (!notAdded.isEmpty()) {
        QString message = tr("Could not add following files to project %1:\n").arg(projectNode->displayName());
        QString files = notAdded.join("\n");
        QMessageBox::warning(core->mainWindow(), tr("Add files to project failed"),
                             message + files);
        foreach (const QString &file, notAdded)
            fileNames.removeOne(file);
    }

    if (Core::IVersionControl *vcManager = core->vcsManager()->findVersionControlForDirectory(dir))
        if (vcManager->supportsOperation(Core::IVersionControl::AddOperation)) {
            const QString files = fileNames.join(QString(QLatin1Char('\n')));
            QMessageBox::StandardButton button =
                QMessageBox::question(core->mainWindow(), tr("Add to Version Control"),
                                      tr("Add files\n%1\nto version control (%2)?").arg(files, vcManager->displayName()),
                                      QMessageBox::Yes | QMessageBox::No);
            if (button == QMessageBox::Yes) {
                QStringList notAddedToVc;
                foreach (const QString &file, fileNames) {
                    if (!vcManager->vcsAdd(file))
                        notAddedToVc << file;
                }

                if (!notAddedToVc.isEmpty()) {
                    const QString message = tr("Could not add following files to version control (%1)\n").arg(vcManager->displayName());
                    const QString filesNotAdded = notAddedToVc.join(QString(QLatin1Char('\n')));
                    QMessageBox::warning(core->mainWindow(), tr("Add files to version control failed"),
                                         message + filesNotAdded);
                }
            }
        }
}

void ProjectExplorerPlugin::openFile()
{
    QTC_ASSERT(d->m_currentNode, return)
    Core::EditorManager *em = Core::EditorManager::instance();
    em->openEditor(d->m_currentNode->path());
    em->ensureEditorManagerVisible();
}

void ProjectExplorerPlugin::showInGraphicalShell()
{
    QTC_ASSERT(d->m_currentNode, return)
    FolderNavigationWidget::showInGraphicalShell(Core::ICore::instance()->mainWindow(),
                                                 d->m_currentNode->path());
}

void ProjectExplorerPlugin::openTerminalHere()
{
    QTC_ASSERT(d->m_currentNode, return)
    FolderNavigationWidget::openTerminal(d->m_currentNode->path());
}

void ProjectExplorerPlugin::removeFile()
{
    QTC_ASSERT(d->m_currentNode && d->m_currentNode->nodeType() == FileNodeType, return)

    FileNode *fileNode = qobject_cast<FileNode*>(d->m_currentNode);
    Core::ICore *core = Core::ICore::instance();

    const QString filePath = d->m_currentNode->path();
    const QString fileDir = QFileInfo(filePath).dir().absolutePath();
    RemoveFileDialog removeFileDialog(filePath, core->mainWindow());

    if (removeFileDialog.exec() == QDialog::Accepted) {
        const bool deleteFile = removeFileDialog.isDeleteFileChecked();

        // remove from project
        ProjectNode *projectNode = fileNode->projectNode();
        Q_ASSERT(projectNode);

        if (!projectNode->removeFiles(fileNode->fileType(), QStringList(filePath))) {
            QMessageBox::warning(core->mainWindow(), tr("Remove file failed"),
                                 tr("Could not remove file %1 from project %2.").arg(filePath).arg(projectNode->displayName()));
            return;
        }

        // remove from version control
        core->vcsManager()->promptToDelete(filePath);

        // remove from file system
        if (deleteFile) {
            QFile file(filePath);

            if (file.exists()) {
                // could have been deleted by vc
                if (!file.remove())
                    QMessageBox::warning(core->mainWindow(), tr("Delete file failed"),
                                         tr("Could not delete file %1.").arg(filePath));
            }
        }
    }
}

void ProjectExplorerPlugin::renameFile()
{
    QWidget *focusWidget = QApplication::focusWidget();
    while (focusWidget) {
        ProjectTreeWidget *treeWidget = qobject_cast<ProjectTreeWidget*>(focusWidget);
        if (treeWidget) {
            treeWidget->editCurrentItem();
            break;
        }
        focusWidget = focusWidget->parentWidget();
    }
}

void ProjectExplorerPlugin::populateOpenWithMenu(QMenu *menu, const QString &fileName)
{
    typedef QList<Core::IEditorFactory*> EditorFactoryList;
    typedef QList<Core::IExternalEditor*> ExternalEditorList;

    menu->clear();

    bool anyMatches = false;

    Core::ICore *core = Core::ICore::instance();
    if (const Core::MimeType mt = core->mimeDatabase()->findByFile(QFileInfo(fileName))) {
        const EditorFactoryList factories = core->editorManager()->editorFactories(mt, false);
        const ExternalEditorList externalEditors = core->editorManager()->externalEditors(mt, false);
        anyMatches = !factories.empty() || !externalEditors.empty();
        if (anyMatches) {
            const QList<Core::IEditor *> editorsOpenForFile = core->editorManager()->editorsForFileName(fileName);
            // Add all suitable editors
            foreach (Core::IEditorFactory *editorFactory, factories) {
                // Add action to open with this very editor factory
                QString const actionTitle = editorFactory->displayName();
                QAction * const action = menu->addAction(actionTitle);
                action->setData(qVariantFromValue(editorFactory));
                // File already open in an editor -> only enable that entry since
                // we currently do not support opening a file in two editors at once
                if (!editorsOpenForFile.isEmpty()) {
                    bool enabled = false;
                    foreach (Core::IEditor * const openEditor, editorsOpenForFile) {
                        if (editorFactory->id() == openEditor->id())
                            enabled = true;
                        break;
                    }
                    action->setEnabled(enabled);
                }
            } // for editor factories
            // Add all suitable external editors
            foreach (Core::IExternalEditor *externalEditor, externalEditors) {
                QAction * const action = menu->addAction(externalEditor->displayName());
                action->setData(qVariantFromValue(externalEditor));
            }
        } // matches
    }
    menu->setEnabled(anyMatches);
}

void ProjectExplorerPlugin::populateOpenWithMenu()
{
    populateOpenWithMenu(d->m_openWithMenu, currentNode()->path());
}

void ProjectExplorerPlugin::openWithMenuTriggered(QAction *action)
{
    if (!action)
        qWarning() << "ProjectExplorerPlugin::openWithMenuTriggered no action, can't happen.";
    else
        openEditorFromAction(action, currentNode()->path());
}

void ProjectExplorerPlugin::openEditorFromAction(QAction *action, const QString &fileName)
{
    Core::EditorManager *em = Core::EditorManager::instance();
    const QVariant data = action->data();
    if (qVariantCanConvert<Core::IEditorFactory *>(data)) {
        Core::IEditorFactory *factory = qVariantValue<Core::IEditorFactory *>(data);
        em->openEditor(fileName, factory->id());
        em->ensureEditorManagerVisible();
        return;
    }
    if (qVariantCanConvert<Core::IExternalEditor *>(data)) {
        Core::IExternalEditor *externalEditor = qVariantValue<Core::IExternalEditor *>(data);
        em->openExternalEditor(fileName, externalEditor->id());
    }
}

void ProjectExplorerPlugin::updateSessionMenu()
{
    d->m_sessionMenu->clear();
    QActionGroup *ag = new QActionGroup(d->m_sessionMenu);
    connect(ag, SIGNAL(triggered(QAction *)), this, SLOT(setSession(QAction *)));
    const QString &activeSession = d->m_session->activeSession();
    foreach (const QString &session, d->m_session->sessions()) {
        QAction *act = ag->addAction(session);
        act->setCheckable(true);
        if (session == activeSession)
            act->setChecked(true);
    }
    d->m_sessionMenu->addActions(ag->actions());
    d->m_sessionMenu->addSeparator();
    d->m_sessionMenu->addAction(d->m_sessionManagerAction);

    d->m_sessionMenu->setEnabled(true);
}

void ProjectExplorerPlugin::setSession(QAction *action)
{
    QString session = action->text();
    if (session != d->m_session->activeSession())
        d->m_session->loadSession(session);
}


void ProjectExplorerPlugin::setProjectExplorerSettings(const Internal::ProjectExplorerSettings &pes)
{
    if (d->m_projectExplorerSettings == pes)
        return;
    d->m_projectExplorerSettings = pes;
    emit settingsChanged();
}

Internal::ProjectExplorerSettings ProjectExplorerPlugin::projectExplorerSettings() const
{
    return d->m_projectExplorerSettings;
}

QStringList ProjectExplorerPlugin::projectFilePatterns()
{
    QStringList patterns;
    const Core::MimeDatabase *mdb = Core::ICore::instance()->mimeDatabase();
    foreach(const IProjectManager *pm, allProjectManagers())
        if (const Core::MimeType mt = mdb->findByType(pm->mimeType()))
            foreach(const QRegExp &re, mt.globPatterns())
                patterns += re.pattern();
    return patterns;
}

void ProjectExplorerPlugin::openOpenProjectDialog()
{
    Core::FileManager *fileMananger = Core::ICore::instance()->fileManager();
    const QString projectPatterns = ProjectExplorerPlugin::projectFilePatterns().join(QString(QLatin1Char(' ')));
    QString projectFilesFilter = tr("Projects (%1)").arg(projectPatterns);
    const QString allFilesFilter = tr("All Files (*)");
    const QString filters = allFilesFilter + QLatin1String(";;") + projectFilesFilter;
    const QString path = fileMananger->useProjectsDirectory() ? fileMananger->projectsDirectory() : QString();
    const QStringList files = fileMananger->getOpenFileNames(filters, path, &projectFilesFilter);
    if (!files.isEmpty())
        Core::ICore::instance()->openFiles(files);
}

Q_EXPORT_PLUGIN(ProjectExplorerPlugin)
