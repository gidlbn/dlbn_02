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

#include "debuggerplugin.h"

#include "breakhandler.h"
#include "debuggeractions.h"
#include "debuggerdialogs.h"
#include "debuggerconstants.h"
#include "debuggermanager.h"
#include "debuggerrunner.h"
#include "debuggerstringutils.h"
#include "debuggeruiswitcher.h"
#include "debuggermainwindow.h"

#include "ui_commonoptionspage.h"
#include "ui_dumperoptionpage.h"

#include <coreplugin/actionmanager/actionmanager.h>
#include <coreplugin/actionmanager/actioncontainer.h>
#include <coreplugin/actionmanager/command.h>
#include <coreplugin/basemode.h>
#include <coreplugin/coreconstants.h>
#include <coreplugin/dialogs/ioptionspage.h>
#include <coreplugin/editormanager/editormanager.h>
#include <coreplugin/findplaceholder.h>
#include <coreplugin/icore.h>
#include <coreplugin/icorelistener.h>
#include <coreplugin/messagemanager.h>
#include <coreplugin/minisplitter.h>
#include <coreplugin/modemanager.h>
#include <coreplugin/navigationwidget.h>
#include <coreplugin/outputpane.h>
#include <coreplugin/rightpane.h>
#include <coreplugin/uniqueidmanager.h>

#include <cplusplus/ExpressionUnderCursor.h>

#include <cppeditor/cppeditorconstants.h>

#include <extensionsystem/pluginmanager.h>

#include <coreplugin/manhattanstyle.h>
#include <projectexplorer/projectexplorer.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/session.h>
#include <projectexplorer/project.h>

#include <texteditor/basetexteditor.h>
#include <texteditor/basetextmark.h>
#include <texteditor/itexteditor.h>
#include <texteditor/texteditorconstants.h>
#include <texteditor/texteditorsettings.h>

#include <utils/qtcassert.h>
#include <utils/styledbar.h>
#include <utils/savedaction.h>

#include <QtCore/QDebug>
#include <QtCore/QObject>
#include <QtCore/QPoint>
#include <QtCore/QSettings>
#include <QtCore/QtPlugin>
#include <QtCore/QCoreApplication>
#include <QtCore/QTimer>
#include <QtCore/QVariant>

#include <QtGui/QLineEdit>
#include <QtGui/QDockWidget>
#include <QtGui/QPlainTextEdit>
#include <QtGui/QTextBlock>
#include <QtGui/QTextCursor>
#include <QtGui/QToolButton>
#include <QtGui/QMessageBox>
#include <QtGui/QAction>
#include <QtGui/QMenu>

#include <climits>

using namespace Core;
using namespace Debugger;
using namespace Debugger::Constants;
using namespace Debugger::Internal;
using namespace ProjectExplorer;
using namespace TextEditor;

namespace CC = Core::Constants;
namespace PE = ProjectExplorer::Constants;


namespace Debugger {
namespace Constants {

const char * const M_DEBUG_START_DEBUGGING = "QtCreator.Menu.Debug.StartDebugging";

const char * const STARTEXTERNAL        = "Debugger.StartExternal";
const char * const ATTACHEXTERNAL       = "Debugger.AttachExternal";
const char * const ATTACHCORE           = "Debugger.AttachCore";
const char * const ATTACHREMOTE         = "Debugger.AttachRemote";
const char * const DETACH               = "Debugger.Detach";

const char * const RUN_TO_LINE1         = "Debugger.RunToLine1";
const char * const RUN_TO_LINE2         = "Debugger.RunToLine2";
const char * const RUN_TO_FUNCTION      = "Debugger.RunToFunction";
const char * const JUMP_TO_LINE1        = "Debugger.JumpToLine1";
const char * const JUMP_TO_LINE2        = "Debugger.JumpToLine2";
const char * const RETURN_FROM_FUNCTION = "Debugger.ReturnFromFunction";
const char * const SNAPSHOT             = "Debugger.Snapshot";
const char * const TOGGLE_BREAK         = "Debugger.ToggleBreak";
const char * const BREAK_BY_FUNCTION    = "Debugger.BreakByFunction";
const char * const BREAK_AT_MAIN        = "Debugger.BreakAtMain";
const char * const ADD_TO_WATCH1        = "Debugger.AddToWatch1";
const char * const ADD_TO_WATCH2        = "Debugger.AddToWatch2";
const char * const OPERATE_BY_INSTRUCTION  = "Debugger.OperateByInstruction";

#ifdef Q_WS_MAC
const char * const INTERRUPT_KEY            = "Shift+F5";
const char * const RESET_KEY                = "Ctrl+Shift+F5";
const char * const STEP_KEY                 = "F7";
const char * const STEPOUT_KEY              = "Shift+F7";
const char * const NEXT_KEY                 = "F6";
const char * const REVERSE_KEY              = "";
const char * const RUN_TO_LINE_KEY          = "Shift+F8";
const char * const RUN_TO_FUNCTION_KEY      = "Ctrl+F6";
const char * const JUMP_TO_LINE_KEY         = "Ctrl+D,Ctrl+L";
const char * const TOGGLE_BREAK_KEY         = "F8";
const char * const BREAK_BY_FUNCTION_KEY    = "Ctrl+D,Ctrl+F";
const char * const BREAK_AT_MAIN_KEY        = "Ctrl+D,Ctrl+M";
const char * const ADD_TO_WATCH_KEY         = "Ctrl+D,Ctrl+W";
const char * const SNAPSHOT_KEY             = "Ctrl+D,Ctrl+S";
#else
const char * const INTERRUPT_KEY            = "Shift+F5";
const char * const RESET_KEY                = "Ctrl+Shift+F5";
const char * const STEP_KEY                 = "F11";
const char * const STEPOUT_KEY              = "Shift+F11";
const char * const NEXT_KEY                 = "F10";
const char * const REVERSE_KEY              = "F12";
const char * const RUN_TO_LINE_KEY          = "";
const char * const RUN_TO_FUNCTION_KEY      = "";
const char * const JUMP_TO_LINE_KEY         = "";
const char * const TOGGLE_BREAK_KEY         = "F9";
const char * const BREAK_BY_FUNCTION_KEY    = "";
const char * const BREAK_AT_MAIN_KEY        = "";
const char * const ADD_TO_WATCH_KEY         = "Ctrl+Alt+Q";
const char * const SNAPSHOT_KEY             = "Ctrl+D,Ctrl+S";
#endif

} // namespace Constants
} // namespace Debugger


static ProjectExplorer::SessionManager *sessionManager()
{
    return ProjectExplorer::ProjectExplorerPlugin::instance()->session();
}

static QSettings *settings()
{
    return ICore::instance()->settings();
}

static QToolButton *toolButton(QAction *action)
{
    QToolButton *button = new QToolButton;
    button->setDefaultAction(action);
    return button;
}

///////////////////////////////////////////////////////////////////////
//
// DebugMode
//
///////////////////////////////////////////////////////////////////////

namespace Debugger {
namespace Internal {

class DebugMode : public Core::BaseMode
{
    Q_OBJECT

public:
    DebugMode(QObject *parent = 0);
    ~DebugMode();
};

DebugMode::DebugMode(QObject *parent)
  : BaseMode(parent)
{
    setDisplayName(tr("Debug"));
    setId(MODE_DEBUG);
    setIcon(QIcon(":/fancyactionbar/images/mode_Debug.png"));
    setPriority(P_MODE_DEBUG);
}

DebugMode::~DebugMode()
{
    // Make sure the editor manager does not get deleted
    EditorManager::instance()->setParent(0);
}

///////////////////////////////////////////////////////////////////////
//
// DebuggerListener: Close the debugging session if running.
//
///////////////////////////////////////////////////////////////////////

class DebuggerListener : public Core::ICoreListener
{
    Q_OBJECT
public:
    DebuggerListener() {}
    virtual bool coreAboutToClose();
};

bool DebuggerListener::coreAboutToClose()
{
    DebuggerManager *mgr = DebuggerManager::instance();
    if (!mgr)
        return true;
    // Ask to terminate the session.
    bool cleanTermination = false;
    switch (mgr->state()) {
    case DebuggerNotReady:
        return true;
    case AdapterStarted:     // Most importantly, terminating a running
    case AdapterStartFailed: // debuggee can cause problems.
    case InferiorUnrunnable:
    case InferiorStartFailed:
    case InferiorStopped:
    case InferiorShutDown:
        cleanTermination = true;
        break;
    default:
        break;
    }

    const QString question = cleanTermination ?
        tr("A debugging session is still in progress.\n"
           "Would you like to terminate it?") :
        tr("A debugging session is still in progress. "
           "Terminating the session in the current"
           " state (%1) can leave the target in an inconsistent state."
           " Would you still like to terminate it?")
        .arg(_(DebuggerManager::stateName(mgr->state())));

    QMessageBox::StandardButton answer =
        QMessageBox::question(DebuggerUISwitcher::instance()->mainWindow(),
            tr("Close Debugging Session"), question,
            QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);

    if (answer != QMessageBox::Yes)
        return false;

    mgr->exitDebugger();
    QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    return true;
}

} // namespace Internal
} // namespace Debugger


///////////////////////////////////////////////////////////////////////
//
// LocationMark
//
///////////////////////////////////////////////////////////////////////

namespace Debugger {
namespace Internal {

// Used in "real" editors
class LocationMark : public TextEditor::BaseTextMark
{
    Q_OBJECT

public:
    LocationMark(const QString &fileName, int linenumber)
        : BaseTextMark(fileName, linenumber)
    {}

    QIcon icon() const { return DebuggerManager::instance()->locationMarkIcon(); }
    void updateLineNumber(int /*lineNumber*/) {}
    void updateBlock(const QTextBlock & /*block*/) {}
    void removedFromEditor() {}
};

} // namespace Internal
} // namespace Debugger


///////////////////////////////////////////////////////////////////////
//
// CommonOptionsPage
//
///////////////////////////////////////////////////////////////////////

namespace Debugger {
namespace Internal {

class CommonOptionsPage : public Core::IOptionsPage
{
    Q_OBJECT

public:
    CommonOptionsPage() {}

    // IOptionsPage
    QString id() const
        { return _(DEBUGGER_COMMON_SETTINGS_ID); }
    QString displayName() const
        { return QCoreApplication::translate("Debugger", DEBUGGER_COMMON_SETTINGS_NAME); }
    QString category() const
        { return _(DEBUGGER_SETTINGS_CATEGORY);  }
    QString displayCategory() const
        { return QCoreApplication::translate("Debugger", DEBUGGER_SETTINGS_TR_CATEGORY); }
    QIcon categoryIcon() const
        { return QIcon(QLatin1String(DEBUGGER_COMMON_SETTINGS_CATEGORY_ICON)); }

    QWidget *createPage(QWidget *parent);
    void apply() { m_group.apply(settings()); }
    void finish() { m_group.finish(); }
    virtual bool matches(const QString &s) const;

private:
    Ui::CommonOptionsPage m_ui;
    Utils::SavedActionSet m_group;
    QString m_searchKeywords;
};

QWidget *CommonOptionsPage::createPage(QWidget *parent)
{
    QWidget *w = new QWidget(parent);
    m_ui.setupUi(w);
    m_group.clear();

    m_group.insert(theDebuggerAction(SwitchLanguageAutomatically),
        m_ui.checkBoxChangeLanguageAutomatically);

    m_group.insert(theDebuggerAction(ListSourceFiles),
        m_ui.checkBoxListSourceFiles);
    m_group.insert(theDebuggerAction(UseAlternatingRowColors),
        m_ui.checkBoxUseAlternatingRowColors);
    m_group.insert(theDebuggerAction(UseToolTipsInMainEditor),
        m_ui.checkBoxUseToolTipsInMainEditor);
    m_group.insert(theDebuggerAction(AutoDerefPointers), 0);
    m_group.insert(theDebuggerAction(UseToolTipsInLocalsView), 0);
    m_group.insert(theDebuggerAction(UseToolTipsInBreakpointsView), 0);
    m_group.insert(theDebuggerAction(UseAddressInBreakpointsView), 0);
    m_group.insert(theDebuggerAction(UseAddressInStackView), 0);
    m_group.insert(theDebuggerAction(MaximalStackDepth),
        m_ui.spinBoxMaximalStackDepth);
    m_group.insert(theDebuggerAction(ShowStdNamespace), 0);
    m_group.insert(theDebuggerAction(ShowQtNamespace), 0);
    m_group.insert(theDebuggerAction(LogTimeStamps), 0);
    m_group.insert(theDebuggerAction(VerboseLog), 0);
    m_group.insert(theDebuggerAction(UsePreciseBreakpoints), 0);
    m_group.insert(theDebuggerAction(BreakOnThrow), 0);
    m_group.insert(theDebuggerAction(BreakOnCatch), 0);
#ifdef Q_OS_WIN
    Utils::SavedAction *registerAction = theDebuggerAction(RegisterForPostMortem);
    m_group.insert(registerAction,
        m_ui.checkBoxRegisterForPostMortem);
    connect(registerAction, SIGNAL(toggled(bool)),
            m_ui.checkBoxRegisterForPostMortem, SLOT(setChecked(bool)));
#endif

    if (m_searchKeywords.isEmpty()) {
        QTextStream(&m_searchKeywords) << ' '
                << m_ui.checkBoxChangeLanguageAutomatically->text()
                << m_ui.checkBoxListSourceFiles->text()
                << ' ' << m_ui.checkBoxUseAlternatingRowColors->text()
                << ' ' << m_ui.checkBoxUseToolTipsInMainEditor->text()
#ifdef Q_OS_WIN
                << ' ' << m_ui.checkBoxRegisterForPostMortem->text()
#endif
                << ' ' << m_ui.labelMaximalStackDepth->text();
        m_searchKeywords.remove(QLatin1Char('&'));
    }
#ifndef Q_OS_WIN
    m_ui.checkBoxRegisterForPostMortem->setVisible(false);
#endif
    return w;
}

bool CommonOptionsPage::matches(const QString &s) const
{
    return m_searchKeywords.contains(s, Qt::CaseInsensitive);
}

} // namespace Internal
} // namespace Debugger


///////////////////////////////////////////////////////////////////////
//
// DebuggingHelperOptionPage
//
///////////////////////////////////////////////////////////////////////

static inline bool oxygenStyle()
{
    if (const ManhattanStyle *ms = qobject_cast<const ManhattanStyle *>(qApp->style()))
        return !qstrcmp("OxygenStyle", ms->baseStyle()->metaObject()->className());
    return false;
}

namespace Debugger {
namespace Internal {

class DebuggingHelperOptionPage : public Core::IOptionsPage
{
    Q_OBJECT

public:
    DebuggingHelperOptionPage() {}

    // IOptionsPage
    QString id() const { return _("Z.DebuggingHelper"); }
    QString displayName() const { return tr("Debugging Helper"); }
    QString category() const { return _(DEBUGGER_SETTINGS_CATEGORY); }
    QString displayCategory() const { return QCoreApplication::translate("Debugger", DEBUGGER_SETTINGS_TR_CATEGORY); }
    QIcon categoryIcon() const { return QIcon(QLatin1String(DEBUGGER_COMMON_SETTINGS_CATEGORY_ICON)); }

    QWidget *createPage(QWidget *parent);
    void apply() { m_group.apply(settings()); }
    void finish() { m_group.finish(); }
    virtual bool matches(const QString &s) const;

private:
    Ui::DebuggingHelperOptionPage m_ui;
    Utils::SavedActionSet m_group;
    QString m_searchKeywords;
};

QWidget *DebuggingHelperOptionPage::createPage(QWidget *parent)
{
    QWidget *w = new QWidget(parent);
    m_ui.setupUi(w);

    m_ui.dumperLocationChooser->setExpectedKind(Utils::PathChooser::Command);
    m_ui.dumperLocationChooser->setPromptDialogTitle(tr("Choose DebuggingHelper Location"));
    m_ui.dumperLocationChooser->setInitialBrowsePathBackup(
        Core::ICore::instance()->resourcePath() + "../../lib");

    m_group.clear();
    m_group.insert(theDebuggerAction(UseDebuggingHelpers),
        m_ui.debuggingHelperGroupBox);
    m_group.insert(theDebuggerAction(UseCustomDebuggingHelperLocation),
        m_ui.customLocationGroupBox);
    // Suppress Oxygen style's giving flat group boxes bold titles.
    if (oxygenStyle())
        m_ui.customLocationGroupBox->setStyleSheet(_("QGroupBox::title { font: ; }"));

    m_group.insert(theDebuggerAction(CustomDebuggingHelperLocation),
        m_ui.dumperLocationChooser);

    m_group.insert(theDebuggerAction(UseCodeModel),
        m_ui.checkBoxUseCodeModel);

#ifdef QT_DEBUG
    m_group.insert(theDebuggerAction(DebugDebuggingHelpers),
        m_ui.checkBoxDebugDebuggingHelpers);
#else
    m_ui.checkBoxDebugDebuggingHelpers->hide();
#endif

#ifndef QT_DEBUG
#if 0
    cmd = am->registerAction(m_manager->m_dumpLogAction,
        DUMP_LOG, globalcontext);
    //cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+D,Ctrl+L")));
    cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+Shift+F11")));
    mdebug->addAction(cmd);
#endif
#endif

    if (m_searchKeywords.isEmpty()) {
        QTextStream(&m_searchKeywords)
                << ' ' << m_ui.debuggingHelperGroupBox->title()
                << ' ' << m_ui.customLocationGroupBox->title()
                << ' ' << m_ui.dumperLocationLabel->text()
                << ' ' << m_ui.checkBoxUseCodeModel->text()
                << ' ' << m_ui.checkBoxDebugDebuggingHelpers->text();
        m_searchKeywords.remove(QLatin1Char('&'));
    }
    return w;
}

bool DebuggingHelperOptionPage::matches(const QString &s) const
{
    return m_searchKeywords.contains(s, Qt::CaseInsensitive);
}

} // namespace Internal
} // namespace Debugger


///////////////////////////////////////////////////////////////////////
//
// DebuggerPlugin
//
///////////////////////////////////////////////////////////////////////


DebuggerPlugin::AttachRemoteParameters::AttachRemoteParameters() :
    attachPid(0),
    winCrashEvent(0)
{
}

DebuggerPlugin::DebuggerPlugin()
  : m_manager(0),
    m_debugMode(0),
    m_locationMark(0),
    m_gdbRunningContext(0),
    m_cmdLineEnabledEngines(AllEngineTypes)
{}

DebuggerPlugin::~DebuggerPlugin()
{
    delete DebuggerSettings::instance();

    removeObject(m_debugMode);

    delete m_debugMode;
    m_debugMode = 0;

    delete m_locationMark;
    m_locationMark = 0;

    removeObject(m_manager);
    delete m_manager;
    m_manager = 0;

    removeObject(m_uiSwitcher);
    delete m_uiSwitcher;
    m_uiSwitcher = 0;
}

void DebuggerPlugin::aboutToShutdown()
{
    QTC_ASSERT(m_manager, /**/);
    if (m_manager)
        m_manager->aboutToShutdown();

    writeSettings();

    if (m_uiSwitcher)
        m_uiSwitcher->aboutToShutdown();
}

static QString msgParameterMissing(const QString &a)
{
    return DebuggerPlugin::tr("Option '%1' is missing the parameter.").arg(a);
}

static QString msgInvalidNumericParameter(const QString &a, const QString &number)
{
    return DebuggerPlugin::tr("The parameter '%1' of option '%2' is not a number.").arg(number, a);
}

// Parse arguments
static bool parseArgument(QStringList::const_iterator &it,
                          const QStringList::const_iterator &cend,
                          DebuggerPlugin::AttachRemoteParameters *attachRemoteParameters,
                          unsigned *enabledEngines, QString *errorMessage)
{
    const QString &option = *it;
    // '-debug <pid>'
    if (*it == _("-debug")) {
        ++it;
        if (it == cend) {
            *errorMessage = msgParameterMissing(*it);
            return false;
        }
        bool ok;
        attachRemoteParameters->attachPid = it->toULongLong(&ok);
        if (!ok) {
            attachRemoteParameters->attachPid = 0;
            attachRemoteParameters->attachCore = *it;
        }
        return true;
    }
    // -wincrashevent <event-handle>. A handle used for
    // a handshake when attaching to a crashed Windows process.
    if (*it == _("-wincrashevent")) {
        ++it;
        if (it == cend) {
            *errorMessage = msgParameterMissing(*it);
            return false;
        }
        bool ok;
        attachRemoteParameters->winCrashEvent = it->toULongLong(&ok);
        if (!ok) {
            *errorMessage = msgInvalidNumericParameter(option, *it);
            return false;
        }
        return true;
    }
    // engine disabling
    if (option == _("-disable-cdb")) {
        *enabledEngines &= ~Debugger::CdbEngineType;
        return true;
    }
    if (option == _("-disable-gdb")) {
        *enabledEngines &= ~Debugger::GdbEngineType;
        return true;
    }
    if (option == _("-disable-sdb")) {
        *enabledEngines &= ~Debugger::ScriptEngineType;
        return true;
    }

    *errorMessage = DebuggerPlugin::tr("Invalid debugger option: %1").arg(option);
    return false;
}

static bool parseArguments(const QStringList &args,
                           DebuggerPlugin::AttachRemoteParameters *attachRemoteParameters,
                           unsigned *enabledEngines, QString *errorMessage)
{
    const QStringList::const_iterator cend = args.constEnd();
    for (QStringList::const_iterator it = args.constBegin(); it != cend; ++it)
        if (!parseArgument(it, cend, attachRemoteParameters, enabledEngines, errorMessage))
            return false;
    if (Debugger::Constants::Internal::debug)
        qDebug().nospace() << args << "engines=0x"
            << QString::number(*enabledEngines, 16)
            << " pid" << attachRemoteParameters->attachPid
            << " core" << attachRemoteParameters->attachCore << '\n';
    return true;
}

void DebuggerPlugin::remoteCommand(const QStringList &options, const QStringList &)
{
    QString errorMessage;
    AttachRemoteParameters parameters;
    unsigned dummy = 0;
    // Did we receive a request for debugging (unless it is ourselves)?
    if (parseArguments(options, &parameters, &dummy, &errorMessage)
        && parameters.attachPid != quint64(QCoreApplication::applicationPid())) {
        m_attachRemoteParameters = parameters;
        attachCmdLine();
    }
}

bool DebuggerPlugin::initialize(const QStringList &arguments, QString *errorMessage)
{
    // Do not fail the whole plugin if something goes wrong here.
    if (!parseArguments(arguments, &m_attachRemoteParameters, &m_cmdLineEnabledEngines, errorMessage)) {
        *errorMessage = tr("Error evaluating command line arguments: %1")
            .arg(*errorMessage);
        qWarning("%s\n", qPrintable(*errorMessage));
        errorMessage->clear();
    }

    // Debug mode setup.
    m_debugMode = new DebugMode(this);
    m_uiSwitcher = new DebuggerUISwitcher(m_debugMode, this);
    ExtensionSystem::PluginManager::instance()->addObject(m_uiSwitcher);

    ICore *core = ICore::instance();
    QTC_ASSERT(core, return false);

    Core::ActionManager *am = core->actionManager();
    QTC_ASSERT(am, return false);

    Core::UniqueIDManager *uidm = core->uniqueIDManager();
    QTC_ASSERT(uidm, return false);

    QList<int> globalcontext;
    globalcontext << CC::C_GLOBAL_ID;

    QList<int> cppcontext;
    cppcontext << uidm->uniqueIdentifier(PE::LANG_CXX);

    QList<int> cppDebuggercontext;
    cppDebuggercontext << uidm->uniqueIdentifier(C_CPPDEBUGGER);

    QList<int> cppeditorcontext;
    cppeditorcontext << uidm->uniqueIdentifier(CppEditor::Constants::C_CPPEDITOR);

    QList<int> texteditorcontext;
    texteditorcontext << uidm->uniqueIdentifier(TextEditor::Constants::C_TEXTEDITOR);

    m_gdbRunningContext = uidm->uniqueIdentifier(Constants::GDBRUNNING);

    m_uiSwitcher->addLanguage(LANG_CPP, cppDebuggercontext);

    DebuggerManager *manager = new DebuggerManager(this);
    ExtensionSystem::PluginManager::instance()->addObject(manager);
    const QList<Core::IOptionsPage *> engineOptionPages =
        manager->initializeEngines(m_cmdLineEnabledEngines);

    // Register factory of DebuggerRunControl.
    m_debuggerRunControlFactory = new DebuggerRunControlFactory(manager);
    addAutoReleasedObject(m_debuggerRunControlFactory);

    QList<int> context;
    context.append(uidm->uniqueIdentifier(CC::C_EDITORMANAGER));
    context.append(uidm->uniqueIdentifier(C_DEBUGMODE));
    context.append(uidm->uniqueIdentifier(CC::C_NAVIGATION_PANE));
    m_debugMode->setContext(context);

    m_reverseToolButton = 0;

    // Handling of external applications.
    m_startExternalAction = new QAction(this);
    m_startExternalAction->setText(tr("Start and Debug External Application..."));
    connect(m_startExternalAction, SIGNAL(triggered()),
        this, SLOT(startExternalApplication()));

    m_attachExternalAction = new QAction(this);
    m_attachExternalAction->setText(tr("Attach to Running External Application..."));
    connect(m_attachExternalAction, SIGNAL(triggered()),
        this, SLOT(attachExternalApplication()));

    m_attachCoreAction = new QAction(this);
    m_attachCoreAction->setText(tr("Attach to Core..."));
    connect(m_attachCoreAction, SIGNAL(triggered()), this, SLOT(attachCore()));

    m_startRemoteAction = new QAction(this);
    m_startRemoteAction->setText(tr("Start and Attach to Remote Application..."));
    connect(m_startRemoteAction, SIGNAL(triggered()),
        this, SLOT(startRemoteApplication()));

    m_detachAction = new QAction(this);
    m_detachAction->setText(tr("Detach Debugger"));
    connect(m_detachAction, SIGNAL(triggered()),
        manager, SLOT(detachDebugger()));

    Core::Command *cmd = 0;
    const DebuggerManagerActions actions = manager->debuggerManagerActions();

    Core::ActionContainer *mstart =
        am->actionContainer(PE::M_DEBUG_STARTDEBUGGING);

    cmd = am->registerAction(actions.continueAction,
        PE::DEBUG, QList<int>() << m_gdbRunningContext);
    mstart->addAction(cmd, CC::G_DEFAULT_ONE);

    cmd = am->registerAction(m_startExternalAction,
        Constants::STARTEXTERNAL, globalcontext);
    cmd->setAttribute(Command::CA_Hide);
    mstart->addAction(cmd, CC::G_DEFAULT_ONE);

    cmd = am->registerAction(m_attachExternalAction,
        Constants::ATTACHEXTERNAL, globalcontext);
    cmd->setAttribute(Command::CA_Hide);
    mstart->addAction(cmd, CC::G_DEFAULT_ONE);

    cmd = am->registerAction(m_attachCoreAction,
        Constants::ATTACHCORE, globalcontext);
    cmd->setAttribute(Command::CA_Hide);
    mstart->addAction(cmd, CC::G_DEFAULT_ONE);

    cmd = am->registerAction(m_startRemoteAction,
        Constants::ATTACHREMOTE, globalcontext);
    cmd->setAttribute(Command::CA_Hide);
    mstart->addAction(cmd, CC::G_DEFAULT_ONE);

    cmd = am->registerAction(m_detachAction,
        Constants::DETACH, globalcontext);
    cmd->setAttribute(Command::CA_Hide);
    m_uiSwitcher->addMenuAction(cmd, CC::G_DEFAULT_ONE);

    cmd = am->registerAction(actions.stopAction,
        Constants::INTERRUPT, globalcontext);
    cmd->setAttribute(Command::CA_UpdateText);
    cmd->setAttribute(Command::CA_UpdateIcon);
    cmd->setDefaultKeySequence(QKeySequence(Constants::INTERRUPT_KEY));
    cmd->setDefaultText(tr("Stop Debugger/Interrupt Debugger"));
    m_uiSwitcher->addMenuAction(cmd, CC::G_DEFAULT_ONE);

    cmd = am->registerAction(actions.resetAction,
        Constants::RESET, globalcontext);
    cmd->setAttribute(Core::Command::CA_UpdateText);
    //cmd->setDefaultKeySequence(QKeySequence(Constants::RESET_KEY));
    cmd->setDefaultText(tr("Reset Debugger"));
    m_uiSwitcher->addMenuAction(cmd, CC::G_DEFAULT_ONE);

    QAction *sep = new QAction(this);
    sep->setSeparator(true);
    cmd = am->registerAction(sep, _("Debugger.Sep.Step"), globalcontext);
    m_uiSwitcher->addMenuAction(cmd, Constants::LANG_CPP);

    cmd = am->registerAction(actions.nextAction,
        Constants::NEXT, cppDebuggercontext);
    cmd->setDefaultKeySequence(QKeySequence(Constants::NEXT_KEY));
    cmd->setAttribute(Command::CA_Hide);
    m_uiSwitcher->addMenuAction(cmd, Constants::LANG_CPP);

    cmd = am->registerAction(actions.stepAction,
        Constants::STEP, cppDebuggercontext);
    cmd->setDefaultKeySequence(QKeySequence(Constants::STEP_KEY));
    cmd->setAttribute(Command::CA_Hide);
    m_uiSwitcher->addMenuAction(cmd, Constants::LANG_CPP);


    cmd = am->registerAction(actions.stepOutAction,
        Constants::STEPOUT, cppDebuggercontext);
    cmd->setDefaultKeySequence(QKeySequence(Constants::STEPOUT_KEY));
    cmd->setAttribute(Command::CA_Hide);
    m_uiSwitcher->addMenuAction(cmd, Constants::LANG_CPP);


    cmd = am->registerAction(actions.runToLineAction1,
        Constants::RUN_TO_LINE1, cppDebuggercontext);
    cmd->setDefaultKeySequence(QKeySequence(Constants::RUN_TO_LINE_KEY));
    cmd->setAttribute(Command::CA_Hide);
    m_uiSwitcher->addMenuAction(cmd, Constants::LANG_CPP);


    cmd = am->registerAction(actions.runToFunctionAction,
        Constants::RUN_TO_FUNCTION, cppDebuggercontext);
    cmd->setDefaultKeySequence(QKeySequence(Constants::RUN_TO_FUNCTION_KEY));
    cmd->setAttribute(Command::CA_Hide);
    m_uiSwitcher->addMenuAction(cmd, Constants::LANG_CPP);


    cmd = am->registerAction(actions.jumpToLineAction1,
        Constants::JUMP_TO_LINE1, cppDebuggercontext);
    cmd->setAttribute(Command::CA_Hide);
    m_uiSwitcher->addMenuAction(cmd, Constants::LANG_CPP);


    cmd = am->registerAction(actions.returnFromFunctionAction,
        Constants::RETURN_FROM_FUNCTION, cppDebuggercontext);
    cmd->setAttribute(Command::CA_Hide);
    m_uiSwitcher->addMenuAction(cmd, Constants::LANG_CPP);


    cmd = am->registerAction(actions.reverseDirectionAction,
        Constants::REVERSE, cppDebuggercontext);
    cmd->setDefaultKeySequence(QKeySequence(Constants::REVERSE_KEY));
    cmd->setAttribute(Command::CA_Hide);
    m_uiSwitcher->addMenuAction(cmd, Constants::LANG_CPP);


    sep = new QAction(this);
    sep->setSeparator(true);
    cmd = am->registerAction(sep, _("Debugger.Sep.Break"), globalcontext);
    m_uiSwitcher->addMenuAction(cmd, Constants::LANG_CPP);


    cmd = am->registerAction(actions.snapshotAction,
        Constants::SNAPSHOT, cppDebuggercontext);
    cmd->setDefaultKeySequence(QKeySequence(Constants::SNAPSHOT_KEY));
    cmd->setAttribute(Command::CA_Hide);
    m_uiSwitcher->addMenuAction(cmd, Constants::LANG_CPP);


    cmd = am->registerAction(theDebuggerAction(OperateByInstruction),
        Constants::OPERATE_BY_INSTRUCTION, cppDebuggercontext);
    cmd->setAttribute(Command::CA_Hide);
    m_uiSwitcher->addMenuAction(cmd, Constants::LANG_CPP);


    cmd = am->registerAction(actions.breakAction,
        Constants::TOGGLE_BREAK, cppeditorcontext);
    cmd->setDefaultKeySequence(QKeySequence(Constants::TOGGLE_BREAK_KEY));
    m_uiSwitcher->addMenuAction(cmd, Constants::LANG_CPP);
    connect(actions.breakAction, SIGNAL(triggered()),
        this, SLOT(toggleBreakpoint()));

    //mcppcontext->addAction(cmd);

    sep = new QAction(this);
    sep->setSeparator(true);
    cmd = am->registerAction(sep, _("Debugger.Sep.Watch"), globalcontext);
    m_uiSwitcher->addMenuAction(cmd, Constants::LANG_CPP);


    cmd = am->registerAction(actions.watchAction1,
        Constants::ADD_TO_WATCH1, cppeditorcontext);
    cmd->action()->setEnabled(true);
    //cmd->setDefaultKeySequence(QKeySequence(tr("Ctrl+D,Ctrl+W")));
    m_uiSwitcher->addMenuAction(cmd, Constants::LANG_CPP);


    // Editor context menu
    ActionContainer *editorContextMenu =
        am->actionContainer(CppEditor::Constants::M_CONTEXT);
    cmd = am->registerAction(sep, _("Debugger.Sep.Views"),
        cppDebuggercontext);
    editorContextMenu->addAction(cmd);
    cmd->setAttribute(Command::CA_Hide);

    cmd = am->registerAction(actions.watchAction2,
        Constants::ADD_TO_WATCH2, cppDebuggercontext);
    cmd->action()->setEnabled(true);
    editorContextMenu->addAction(cmd);
    cmd->setAttribute(Command::CA_Hide);

    cmd = am->registerAction(actions.runToLineAction2,
        Constants::RUN_TO_LINE2, cppDebuggercontext);
    cmd->action()->setEnabled(true);
    editorContextMenu->addAction(cmd);
    cmd->setAttribute(Command::CA_Hide);

    cmd = am->registerAction(actions.jumpToLineAction2,
        Constants::JUMP_TO_LINE2, cppDebuggercontext);
    cmd->action()->setEnabled(true);
    editorContextMenu->addAction(cmd);
    cmd->setAttribute(Command::CA_Hide);

    addAutoReleasedObject(new CommonOptionsPage);
    foreach (Core::IOptionsPage *op, engineOptionPages)
        addAutoReleasedObject(op);
    addAutoReleasedObject(new DebuggingHelperOptionPage);

    addAutoReleasedObject(new DebuggerListener);
    m_locationMark = 0;

    manager->setSimpleDockWidgetArrangement(LANG_CPP);
    readSettings();

    connect(ModeManager::instance(), SIGNAL(currentModeChanged(Core::IMode*)),
            this, SLOT(onModeChanged(Core::IMode*)));
    m_debugMode->widget()->setFocusProxy(EditorManager::instance());
    addObject(m_debugMode);

    m_manager = manager;


    //
    //  Connections
    //

    // TextEditor
    connect(TextEditorSettings::instance(),
        SIGNAL(fontSettingsChanged(TextEditor::FontSettings)),
        manager, SLOT(fontSettingsChanged(TextEditor::FontSettings)));

    // ProjectExplorer
    connect(sessionManager(), SIGNAL(sessionLoaded()),
       manager, SLOT(sessionLoaded()));
    connect(sessionManager(), SIGNAL(aboutToSaveSession()),
       manager, SLOT(aboutToSaveSession()));
    connect(sessionManager(), SIGNAL(aboutToUnloadSession()),
       manager, SLOT(aboutToUnloadSession()));

    // EditorManager
    QObject *editorManager = core->editorManager();
    connect(editorManager, SIGNAL(editorAboutToClose(Core::IEditor*)),
        this, SLOT(editorAboutToClose(Core::IEditor*)));
    connect(editorManager, SIGNAL(editorOpened(Core::IEditor*)),
        this, SLOT(editorOpened(Core::IEditor*)));

    // Application interaction
    connect(theDebuggerAction(SettingsDialog), SIGNAL(triggered()),
        this, SLOT(showSettingsDialog()));

    handleStateChanged(DebuggerNotReady);

    // Toolbar
    QWidget *toolbarContainer = new QWidget;

    QHBoxLayout *hbox = new QHBoxLayout(toolbarContainer);
    hbox->setMargin(0);
    hbox->setSpacing(0);
    hbox->addWidget(toolButton(am->command(PE::DEBUG)->action()));
    hbox->addWidget(toolButton(am->command(INTERRUPT)->action()));
    hbox->addWidget(toolButton(am->command(NEXT)->action()));
    hbox->addWidget(toolButton(am->command(STEP)->action()));
    hbox->addWidget(toolButton(am->command(STEPOUT)->action()));
    hbox->addWidget(toolButton(am->command(OPERATE_BY_INSTRUCTION)->action()));

    //hbox->addWidget(new Utils::StyledSeparator);
    m_reverseToolButton = toolButton(am->command(REVERSE)->action());
    hbox->addWidget(m_reverseToolButton);
    //m_reverseToolButton->hide();

    hbox->addWidget(new Utils::StyledSeparator);
    hbox->addWidget(new QLabel(tr("Threads:")));

    QComboBox *threadBox = new QComboBox;
    threadBox->setModel(m_manager->threadsModel());
    connect(threadBox, SIGNAL(activated(int)),
        m_manager->threadsWindow(), SIGNAL(threadSelected(int)));

    hbox->addWidget(threadBox);
    hbox->addSpacerItem(new QSpacerItem(4, 0));
    hbox->addWidget(m_manager->statusLabel(), 10);


    m_uiSwitcher->setToolbar(LANG_CPP, toolbarContainer);
    connect(m_uiSwitcher, SIGNAL(dockArranged(QString)), manager,
            SLOT(setSimpleDockWidgetArrangement(QString)));

    connect(theDebuggerAction(EnableReverseDebugging), SIGNAL(valueChanged(QVariant)),
        this, SLOT(enableReverseDebuggingTriggered(QVariant)));

    // UI Switcher
    connect(DebuggerUISwitcher::instance(), SIGNAL(languageChanged(QString)),
           this, SLOT(languageChanged(QString)));

    return true;
}

void DebuggerPlugin::extensionsInitialized()
{
    // time gdb -i mi -ex 'debuggerplugin.cpp:800' -ex r -ex q bin/qtcreator.bin
    const QByteArray env = qgetenv("QTC_DEBUGGER_TEST");
    //qDebug() << "EXTENSIONS INITIALIZED:" << env;
    if (!env.isEmpty())
        m_manager->runTest(QString::fromLocal8Bit(env));
    if (m_attachRemoteParameters.attachPid || !m_attachRemoteParameters.attachCore.isEmpty())
        QTimer::singleShot(0, this, SLOT(attachCmdLine()));

    readSettings();
    m_uiSwitcher->initialize();
}

void DebuggerPlugin::attachCmdLine()
{
    if (m_manager->state() != DebuggerNotReady)
        return;
    if (m_attachRemoteParameters.attachPid) {
        m_manager->showStatusMessage(tr("Attaching to PID %1.").arg(m_attachRemoteParameters.attachPid));
        const QString crashParameter =
                m_attachRemoteParameters.winCrashEvent ? QString::number(m_attachRemoteParameters.winCrashEvent) : QString();
        attachExternalApplication(m_attachRemoteParameters.attachPid, QString(), crashParameter);
        return;
    }
    if (!m_attachRemoteParameters.attachCore.isEmpty()) {
        m_manager->showStatusMessage(tr("Attaching to core %1.").arg(m_attachRemoteParameters.attachCore));
        attachCore(m_attachRemoteParameters.attachCore, QString());
    }
}

/*! Activates the previous mode when the current mode is the debug mode. */
void DebuggerPlugin::activatePreviousMode()
{
    Core::ModeManager *const modeManager = ICore::instance()->modeManager();

    if (modeManager->currentMode() == modeManager->mode(MODE_DEBUG)
            && !m_previousMode.isEmpty()) {
        modeManager->activateMode(m_previousMode);
        m_previousMode.clear();
    }
}

void DebuggerPlugin::activateDebugMode()
{
    ModeManager *modeManager = ModeManager::instance();
    m_previousMode = modeManager->currentMode()->id();
    modeManager->activateMode(_(MODE_DEBUG));
}

static bool isDebuggable(Core::IEditor *editor)
{
    // Only blacklist QML. Whitelisting would fail on C++ code in files
    // with strange names, more harm would be done this way.
    Core::IFile *file = editor->file();
    return !(file && file->mimeType() == "application/x-qml");
}

TextEditor::ITextEditor *DebuggerPlugin::currentTextEditor()
{
    EditorManager *editorManager = EditorManager::instance();
    if (!editorManager)
        return 0;
    Core::IEditor *editor = editorManager->currentEditor();
    return qobject_cast<ITextEditor*>(editor);
}

void DebuggerPlugin::editorOpened(Core::IEditor *editor)
{
    if (!isDebuggable(editor))
        return;
    ITextEditor *textEditor = qobject_cast<ITextEditor *>(editor);
    if (!textEditor)
        return;
    connect(textEditor, SIGNAL(markRequested(TextEditor::ITextEditor*,int)),
        this, SLOT(requestMark(TextEditor::ITextEditor*,int)));
    connect(editor, SIGNAL(tooltipRequested(TextEditor::ITextEditor*,QPoint,int)),
        this, SLOT(showToolTip(TextEditor::ITextEditor*,QPoint,int)));
    connect(textEditor, SIGNAL(markContextMenuRequested(TextEditor::ITextEditor*,int,QMenu*)),
        this, SLOT(requestContextMenu(TextEditor::ITextEditor*,int,QMenu*)));
}

void DebuggerPlugin::editorAboutToClose(Core::IEditor *editor)
{
    if (!isDebuggable(editor))
        return;
    ITextEditor *textEditor = qobject_cast<ITextEditor *>(editor);
    if (!textEditor)
        return;
    disconnect(textEditor, SIGNAL(markRequested(TextEditor::ITextEditor*,int)),
        this, SLOT(requestMark(TextEditor::ITextEditor*,int)));
    disconnect(editor, SIGNAL(tooltipRequested(TextEditor::ITextEditor*,QPoint,int)),
        this, SLOT(showToolTip(TextEditor::ITextEditor*,QPoint,int)));
    disconnect(textEditor, SIGNAL(markContextMenuRequested(TextEditor::ITextEditor*,int,QMenu*)),
        this, SLOT(requestContextMenu(TextEditor::ITextEditor*,int,QMenu*)));
}

void DebuggerPlugin::requestContextMenu(TextEditor::ITextEditor *editor,
    int lineNumber, QMenu *menu)
{
    if (!isDebuggable(editor))
        return;

    QString fileName, position;
    if (editor->property("DisassemblerView").toBool()) {
        QString fileName = editor->file()->fileName();
        QString line = editor->contents()
            .section('\n', lineNumber - 1, lineNumber - 1);
        fileName = line.left(line.indexOf(QLatin1Char(' ')));
        lineNumber = -1;
        position = _("*") + fileName;
    } else {
        fileName = editor->file()->fileName();
        position = fileName + QString(":%1").arg(lineNumber);
    }
    BreakpointData *data = m_manager->findBreakpoint(fileName, lineNumber);

    if (data) {
        // existing breakpoint
        QAction *act = new QAction(tr("Remove Breakpoint"), menu);
        act->setData(position);
        connect(act, SIGNAL(triggered()),
            this, SLOT(breakpointSetRemoveMarginActionTriggered()));
        menu->addAction(act);

        QAction *act2;
        if (data->enabled)
            act2 = new QAction(tr("Disable Breakpoint"), menu);
        else
            act2 = new QAction(tr("Enable Breakpoint"), menu);
        act2->setData(position);
        connect(act2, SIGNAL(triggered()),
            this, SLOT(breakpointEnableDisableMarginActionTriggered()));
        menu->addAction(act2);
    } else {
        // non-existing
        QAction *act = new QAction(tr("Set Breakpoint"), menu);
        act->setData(position);
        connect(act, SIGNAL(triggered()),
            this, SLOT(breakpointSetRemoveMarginActionTriggered()));
        menu->addAction(act);
    }
}

void DebuggerPlugin::breakpointSetRemoveMarginActionTriggered()
{
    QAction *act = qobject_cast<QAction *>(sender());
    QTC_ASSERT(act, return);
    QString str = act->data().toString();
    int pos = str.lastIndexOf(':');
    m_manager->toggleBreakpoint(str.left(pos), str.mid(pos + 1).toInt());
}

void DebuggerPlugin::breakpointEnableDisableMarginActionTriggered()
{
    QAction *act = qobject_cast<QAction *>(sender());
    QTC_ASSERT(act, return);
    BreakHandler *handler = m_manager->breakHandler();
    QTC_ASSERT(handler, return);

    QString str = act->data().toString();
    int pos = str.lastIndexOf(':');
    QString fileName = str.left(pos);
    int lineNumber = str.mid(pos + 1).toInt();

    BreakpointData *data = handler->at(handler->findBreakpoint(fileName, lineNumber));
    handler->toggleBreakpointEnabled(data);

    m_manager->attemptBreakpointSynchronization();
}

void DebuggerPlugin::requestMark(ITextEditor *editor, int lineNumber)
{
    if (!isDebuggable(editor))
        return;
    m_manager->toggleBreakpoint(editor->file()->fileName(), lineNumber);
}

void DebuggerPlugin::showToolTip(ITextEditor *editor, const QPoint &point, int pos)
{
    if (!isDebuggable(editor))
        return;
    if (!theDebuggerBoolSetting(UseToolTipsInMainEditor))
        return;
    if (m_manager->state() == DebuggerNotReady)
        return;

    m_manager->setToolTipExpression(point, editor, pos);
}

void DebuggerPlugin::setSessionValue(const QString &name, const QVariant &value)
{
    QTC_ASSERT(sessionManager(), return);
    sessionManager()->setValue(name, value);
}

QVariant DebuggerPlugin::sessionValue(const QString &name)
{
    QTC_ASSERT(sessionManager(), return QVariant());
    return sessionManager()->value(name);
}

void DebuggerPlugin::setConfigValue(const QString &name, const QVariant &value)
{
    QTC_ASSERT(m_debugMode, return);
    settings()->setValue(name, value);
}

QVariant DebuggerPlugin::configValue(const QString &name) const
{
    QTC_ASSERT(m_debugMode, return QVariant());
    return settings()->value(name);
}

void DebuggerPlugin::resetLocation()
{
    //qDebug() << "RESET_LOCATION: current:"  << currentTextEditor();
    //qDebug() << "RESET_LOCATION: locations:"  << m_locationMark;
    //qDebug() << "RESET_LOCATION: stored:"  << m_locationMark->editor();
    delete m_locationMark;
    m_locationMark = 0;
}

void DebuggerPlugin::gotoLocation(const QString &file, int line, bool setMarker)
{
    bool newEditor = false;
    ITextEditor *editor =
        BaseTextEditor::openEditorAt(file, line, 0, QString(), &newEditor);
    if (!editor)
        return;
    if (newEditor)
        editor->setProperty("OpenedByDebugger", true);
    if (setMarker) {
        resetLocation();
        m_locationMark = new LocationMark(file, line);
    }
}

void DebuggerPlugin::openTextEditor(const QString &titlePattern0,
    const QString &contents)
{
    QString titlePattern = titlePattern0;
    EditorManager *editorManager = EditorManager::instance();
    QTC_ASSERT(editorManager, return);
    Core::IEditor *editor = editorManager->openEditorWithContents(
        Core::Constants::K_DEFAULT_TEXT_EDITOR_ID, &titlePattern, contents);
    QTC_ASSERT(editor, return);
    editorManager->activateEditor(editor);
}

void DebuggerPlugin::handleStateChanged(int state)
{
    // Prevent it from beeing triggered on setup.
    if (!m_manager)
        return;

    const bool startIsContinue = (state == InferiorStopped);
    ICore *core = ICore::instance();
    if (startIsContinue)
        core->updateAdditionalContexts(QList<int>(), QList<int>() << m_gdbRunningContext);
    else
        core->updateAdditionalContexts(QList<int>() << m_gdbRunningContext, QList<int>());

    const bool started = state == InferiorRunning
        || state == InferiorRunningRequested
        || state == InferiorStopping
        || state == InferiorStopped;

    const bool starting = state == EngineStarting;
    //const bool running = state == InferiorRunning;

    const bool detachable = state == InferiorStopped
        && m_manager->startParameters()->startMode != AttachCore;

    m_startExternalAction->setEnabled(!started && !starting);
    m_attachExternalAction->setEnabled(!started && !starting);
#ifdef Q_OS_WIN
    m_attachCoreAction->setEnabled(false);
#else
    m_attachCoreAction->setEnabled(!started && !starting);
#endif
    m_startRemoteAction->setEnabled(!started && !starting);
    m_detachAction->setEnabled(detachable);
}

void DebuggerPlugin::languageChanged(const QString &language)
{
    if (!m_manager)
        return;

    const bool debuggerIsCPP = (language == Constants::LANG_CPP);

    m_startExternalAction->setVisible(debuggerIsCPP);
    m_attachExternalAction->setVisible(debuggerIsCPP);
    m_attachCoreAction->setVisible(debuggerIsCPP);
    m_startRemoteAction->setVisible(debuggerIsCPP);
    m_detachAction->setVisible(debuggerIsCPP);

}

void DebuggerPlugin::writeSettings() const
{
    QSettings *s = settings();
    DebuggerSettings::instance()->writeSettings(s);
}

void DebuggerPlugin::readSettings()
{
    QSettings *s = settings();
    DebuggerSettings::instance()->readSettings(s);
}

static bool isCurrentProjectCppBased()
{
    ProjectExplorer::ProjectExplorerPlugin *projectExplorer = ProjectExplorer::ProjectExplorerPlugin::instance();
    Project *startupProject = projectExplorer->startupProject();
    const QStringList cppProjectIds = QStringList() << QLatin1String("GenericProjectManager.GenericProject")
                                                    << QLatin1String("CMakeProjectManager.CMakeProject")
                                                    << QLatin1String("Qt4ProjectManager.Qt4Project");
    return (startupProject && cppProjectIds.contains(startupProject->id()));
}

void DebuggerPlugin::onModeChanged(IMode *mode)
{
     // FIXME: This one gets always called, even if switching between modes
     //        different then the debugger mode. E.g. Welcome and Help mode and
     //        also on shutdown.

    if (mode != m_debugMode)
        return;

    EditorManager *editorManager = EditorManager::instance();
    if (editorManager->currentEditor()) {
        editorManager->currentEditor()->widget()->setFocus();

        if (isCurrentProjectCppBased())
            m_uiSwitcher->setActiveLanguage(LANG_CPP);

    }
}



void DebuggerPlugin::showSettingsDialog()
{
    Core::ICore::instance()->showOptionsDialog(
        _(DEBUGGER_SETTINGS_CATEGORY),
        _(DEBUGGER_COMMON_SETTINGS_ID));
}

void DebuggerPlugin::startExternalApplication()
{
    const DebuggerStartParametersPtr sp(new DebuggerStartParameters);
    StartExternalDialog dlg(m_uiSwitcher->mainWindow());
    dlg.setExecutableFile(
            configValue(_("LastExternalExecutableFile")).toString());
    dlg.setExecutableArguments(
            configValue(_("LastExternalExecutableArguments")).toString());
    if (dlg.exec() != QDialog::Accepted)
        return;

    setConfigValue(_("LastExternalExecutableFile"),
                   dlg.executableFile());
    setConfigValue(_("LastExternalExecutableArguments"),
                   dlg.executableArguments());
    sp->executable = dlg.executableFile();
    sp->startMode = StartExternal;
    if (!dlg.executableArguments().isEmpty())
        sp->processArgs = dlg.executableArguments().split(QLatin1Char(' '));

    if (dlg.breakAtMain())
        m_manager->breakByFunctionMain();

    if (RunControl *runControl = m_debuggerRunControlFactory->create(sp))
        ProjectExplorerPlugin::instance()->startRunControl(runControl, PE::DEBUGMODE);
}

void DebuggerPlugin::attachExternalApplication()
{
    AttachExternalDialog dlg(m_uiSwitcher->mainWindow());
    if (dlg.exec() == QDialog::Accepted)
        attachExternalApplication(dlg.attachPID(), dlg.executable(), QString());
}

void DebuggerPlugin::attachExternalApplication(qint64 pid,
                                               const QString &binary,
                                               const QString &crashParameter)
{
    if (pid == 0) {
        QMessageBox::warning(m_uiSwitcher->mainWindow(), tr("Warning"),
            tr("Cannot attach to PID 0"));
        return;
    }
    const DebuggerStartParametersPtr sp(new DebuggerStartParameters);
    sp->attachPID = pid;
    sp->executable = binary;
    sp->crashParameter = crashParameter;
    sp->startMode = crashParameter.isEmpty() ? AttachExternal : AttachCrashedExternal;
    if (RunControl *runControl = m_debuggerRunControlFactory->create(sp))
        ProjectExplorerPlugin::instance()->startRunControl(runControl, PE::DEBUGMODE);
}

void DebuggerPlugin::attachCore()
{
    AttachCoreDialog dlg(m_uiSwitcher->mainWindow());
    dlg.setExecutableFile(
            configValue(_("LastExternalExecutableFile")).toString());
    dlg.setCoreFile(
            configValue(_("LastExternalCoreFile")).toString());
    if (dlg.exec() != QDialog::Accepted)
        return;
    setConfigValue(_("LastExternalExecutableFile"),
                   dlg.executableFile());
    setConfigValue(_("LastExternalCoreFile"),
                   dlg.coreFile());
    attachCore(dlg.coreFile(), dlg.executableFile());
}

void DebuggerPlugin::attachCore(const QString &core, const QString &exe)
{
    const DebuggerStartParametersPtr sp(new DebuggerStartParameters);
    sp->executable = exe;
    sp->coreFile = core;
    sp->startMode = AttachCore;
    if (RunControl *runControl = m_debuggerRunControlFactory->create(sp))
        ProjectExplorerPlugin::instance()->
            startRunControl(runControl, PE::DEBUGMODE);
}

void DebuggerPlugin::startRemoteApplication()
{
    const DebuggerStartParametersPtr sp(new DebuggerStartParameters);
    StartRemoteDialog dlg(m_uiSwitcher->mainWindow());
    QStringList arches;
    arches.append(_("i386:x86-64:intel"));
    arches.append(_("i386"));
    QString lastUsed = configValue(_("LastRemoteArchitecture")).toString();
    if (!arches.contains(lastUsed))
        arches.prepend(lastUsed);
    dlg.setRemoteArchitectures(arches);
    dlg.setRemoteChannel(
            configValue(_("LastRemoteChannel")).toString());
    dlg.setLocalExecutable(
            configValue(_("LastLocalExecutable")).toString());
    dlg.setDebugger(configValue(_("LastDebugger")).toString());
    dlg.setRemoteArchitecture(lastUsed);
    dlg.setServerStartScript(
            configValue(_("LastServerStartScript")).toString());
    dlg.setUseServerStartScript(
            configValue(_("LastUseServerStartScript")).toBool());
    dlg.setSysRoot(configValue(_("LastSysroot")).toString());
    if (dlg.exec() != QDialog::Accepted)
        return;
    setConfigValue(_("LastRemoteChannel"), dlg.remoteChannel());
    setConfigValue(_("LastLocalExecutable"), dlg.localExecutable());
    setConfigValue(_("LastDebugger"), dlg.debugger());
    setConfigValue(_("LastRemoteArchitecture"), dlg.remoteArchitecture());
    setConfigValue(_("LastServerStartScript"), dlg.serverStartScript());
    setConfigValue(_("LastUseServerStartScript"), dlg.useServerStartScript());
    setConfigValue(_("LastSysroot"), dlg.sysRoot());
    sp->remoteChannel = dlg.remoteChannel();
    sp->remoteArchitecture = dlg.remoteArchitecture();
    sp->executable = dlg.localExecutable();
    sp->debuggerCommand = dlg.debugger(); // Override toolchain-detection.
    if (!sp->debuggerCommand.isEmpty())
        sp->toolChainType = ProjectExplorer::ToolChain::INVALID;
    sp->startMode = StartRemote;
    if (dlg.useServerStartScript())
        sp->serverStartScript = dlg.serverStartScript();
    sp->sysRoot = dlg.sysRoot();

    if (RunControl *runControl = m_debuggerRunControlFactory->create(sp))
        ProjectExplorerPlugin::instance()
            ->startRunControl(runControl, PE::DEBUGMODE);
}

void DebuggerPlugin::enableReverseDebuggingTriggered(const QVariant &value)
{
    QTC_ASSERT(m_reverseToolButton, return);
    m_reverseToolButton->setVisible(value.toBool());
    m_manager->debuggerManagerActions().reverseDirectionAction->setChecked(false);
    m_manager->debuggerManagerActions().reverseDirectionAction->setEnabled(value.toBool());
}

void DebuggerPlugin::toggleBreakpoint()
{
    ITextEditor *textEditor = currentTextEditor();
    QTC_ASSERT(textEditor, return);
    QString fileName = textEditor->file()->fileName();
    int lineNumber = textEditor->currentLine();
    if (lineNumber >= 0)
        m_manager->toggleBreakpoint(fileName, lineNumber);
}

#include "debuggerplugin.moc"

Q_EXPORT_PLUGIN(DebuggerPlugin)
