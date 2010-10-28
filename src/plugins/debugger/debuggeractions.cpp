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

#include "debuggeractions.h"
#ifdef Q_OS_WIN
#include "registerpostmortemaction.h"
#endif

#include <projectexplorer/toolchain.h>

#include <utils/savedaction.h>
#include <utils/qtcassert.h>
#include <utils/pathchooser.h>

#include <QtCore/QDebug>
#include <QtCore/QVariant>
#include <QtCore/QSettings>

#include <QtGui/QAction>
#include <QtGui/QAbstractButton>
#include <QtGui/QRadioButton>
#include <QtGui/QCheckBox>
#include <QtGui/QLineEdit>

using namespace Utils;

static const char debugModeSettingsGroupC[] = "DebugMode";
static const char gdbBinariesSettingsGroupC[] = "GdbBinaries";
static const char debugModeGdbBinaryKeyC[] = "GdbBinary";

namespace Debugger {
namespace Internal {

//////////////////////////////////////////////////////////////////////////
//
// DebuggerSettings
//
//////////////////////////////////////////////////////////////////////////

DebuggerSettings::DebuggerSettings(QObject *parent)
    : QObject(parent), m_gdbBinaryToolChainMap(new GdbBinaryToolChainMap)
{}

DebuggerSettings::~DebuggerSettings()
{
    qDeleteAll(m_items);
}

void DebuggerSettings::insertItem(int code, SavedAction *item)
{
    QTC_ASSERT(!m_items.contains(code),
        qDebug() << code << item->toString(); return);
    QTC_ASSERT(item->settingsKey().isEmpty() || item->defaultValue().isValid(),
        qDebug() << "NO DEFAULT VALUE FOR " << item->settingsKey());
    m_items[code] = item;
}

void DebuggerSettings::readSettings(QSettings *settings)
{
    foreach (SavedAction *item, m_items)
        item->readSettings(settings);
    // Convert gdb binaries from flat settings list (see writeSettings)
    // into map ("binary1=gdb,1,2", "binary2=symbian_gdb,3,4").
    m_gdbBinaryToolChainMap->clear();
    const QChar separator = QLatin1Char(',');
    const QString keyRoot = QLatin1String(gdbBinariesSettingsGroupC) + QLatin1Char('/') +
                            QLatin1String(debugModeGdbBinaryKeyC);
    for (int i = 1; ; i++) {
        const QString value = settings->value(keyRoot + QString::number(i)).toString();
        if (value.isEmpty())
            break;
        // Split apart comma-separated binary and its numerical toolchains.
        QStringList tokens = value.split(separator);
        if (tokens.size() < 2)
            break;
        const QString binary = tokens.front();
        tokens.pop_front();
        foreach(const QString &t, tokens)
            m_gdbBinaryToolChainMap->insert(binary, t.toInt());
    }
    // Linux defaults
#ifdef Q_OS_UNIX
    if (m_gdbBinaryToolChainMap->isEmpty()) {
        const QString gdb = QLatin1String("gdb");
        m_gdbBinaryToolChainMap->insert(gdb, ProjectExplorer::ToolChain::GCC);
        m_gdbBinaryToolChainMap->insert(gdb, ProjectExplorer::ToolChain::OTHER);
        m_gdbBinaryToolChainMap->insert(gdb, ProjectExplorer::ToolChain::UNKNOWN);
    }
#endif
}

void DebuggerSettings::writeSettings(QSettings *settings) const
{
    foreach (SavedAction *item, m_items)
        item->writeSettings(settings);
    // Convert gdb binaries map into a flat settings list of
    // ("binary1=gdb,1,2", "binary2=symbian_gdb,3,4"). It needs to be ASCII for installers
    QString lastBinary;
    QStringList settingsList;
    const QChar separator = QLatin1Char(',');
    const GdbBinaryToolChainMap::const_iterator cend = m_gdbBinaryToolChainMap->constEnd();
    for (GdbBinaryToolChainMap::const_iterator it = m_gdbBinaryToolChainMap->constBegin(); it != cend; ++it) {
        if (it.key() != lastBinary) {
            lastBinary = it.key(); // Start new entry with first toolchain
            settingsList.push_back(lastBinary);
        }
        settingsList.back().append(separator); // Append toolchain to last binary
        settingsList.back().append(QString::number(it.value()));
    }
    settings->beginGroup(QLatin1String(gdbBinariesSettingsGroupC));
    settings->remove(QString()); // remove all keys in group.
    const int count = settingsList.size();
    const QString keyRoot = QLatin1String(debugModeGdbBinaryKeyC);
    for (int i = 0; i < count; i++)
        settings->setValue(keyRoot + QString::number(i + 1), settingsList.at(i));
    settings->endGroup();
}

SavedAction *DebuggerSettings::item(int code) const
{
    QTC_ASSERT(m_items.value(code, 0), qDebug() << "CODE: " << code; return 0);
    return m_items.value(code, 0);
}

QString DebuggerSettings::dump() const
{
    QString out;
    QTextStream ts(&out);
    ts << "Debugger settings: ";
    foreach (SavedAction *item, m_items) {
        QString key = item->settingsKey();
        if (!key.isEmpty())
            ts << '\n' << key << ": " << item->value().toString()
               << "  (default: " << item->defaultValue().toString() << ")";
    }
    return out;
}

DebuggerSettings *DebuggerSettings::instance()
{
    static DebuggerSettings *instance = 0;
    if (instance)
        return instance;

    const QString debugModeGroup = QLatin1String(debugModeSettingsGroupC);
    instance = new DebuggerSettings;

    SavedAction *item = 0;

    item = new SavedAction(instance);
    instance->insertItem(SettingsDialog, item);
    item->setText(tr("Debugger Properties..."));

    //
    // View
    //
    item = new SavedAction(instance);
    item->setText(tr("Adjust Column Widths to Contents"));
    instance->insertItem(AdjustColumnWidths, item);

    item = new SavedAction(instance);
    item->setText(tr("Always Adjust Column Widths to Contents"));
    item->setCheckable(true);
    instance->insertItem(AlwaysAdjustColumnWidths, item);

    item = new SavedAction(instance);
    item->setText(tr("Use Alternating Row Colors"));
    item->setSettingsKey(debugModeGroup, QLatin1String("UseAlternatingRowColours"));
    item->setCheckable(true);
    item->setDefaultValue(false);
    instance->insertItem(UseAlternatingRowColors, item);

    item = new SavedAction(instance);
    item->setText(tr("Show a Message Box When Receiving a Signal"));
    item->setSettingsKey(debugModeGroup, QLatin1String("UseMessageBoxForSignals"));
    item->setCheckable(true);
    item->setDefaultValue(true);
    item->setValue(true);
    instance->insertItem(UseMessageBoxForSignals, item);

    item = new SavedAction(instance);
    item->setText(tr("Log Time Stamps"));
    item->setSettingsKey(debugModeGroup, QLatin1String("LogTimeStamps"));
    item->setCheckable(true);
    item->setDefaultValue(false);
    instance->insertItem(LogTimeStamps, item);

    item = new SavedAction(instance);
    item->setText(tr("Verbose Log"));
    item->setSettingsKey(debugModeGroup, QLatin1String("VerboseLog"));
    item->setCheckable(true);
    item->setDefaultValue(false);
    instance->insertItem(VerboseLog, item);

    item = new SavedAction(instance);
    item->setText(tr("Operate by Instruction"));
    item->setCheckable(true);
    item->setDefaultValue(false);
    item->setIcon(QIcon(":/debugger/images/debugger_singleinstructionmode.png"));
    item->setToolTip(tr("This switches the debugger to instruction-wise "
        "operation mode. In this mode, stepping operates on single "
        "instructions and the source location view also shows the "
        "disassembled instructions."));
    item->setIconVisibleInMenu(false);
    instance->insertItem(OperateByInstruction, item);

    item = new SavedAction(instance);
    item->setText(tr("Dereference Pointers Automatically"));
    item->setCheckable(true);
    item->setDefaultValue(true);
    item->setToolTip(tr("This switches the Locals&Watchers view to "
        "automatically derefence pointers. This saves a level in the "
        "tree view, but also loses data for the now-missing intermediate "
        "level."));
    instance->insertItem(AutoDerefPointers, item);

    //
    // Locals & Watchers
    //
    item = new SavedAction(instance);
    item->setTextPattern(tr("Watch Expression \"%1\""));
    instance->insertItem(WatchExpression, item);

    item = new SavedAction(instance);
    item->setTextPattern(tr("Remove Watch Expression \"%1\""));
    instance->insertItem(RemoveWatchExpression, item);

    item = new SavedAction(instance);
    item->setTextPattern(tr("Watch Expression \"%1\" in Separate Window"));
    instance->insertItem(WatchExpressionInWindow, item);

    item = new SavedAction(instance);
    instance->insertItem(AssignValue, item);

    item = new SavedAction(instance);
    instance->insertItem(AssignType, item);

    item = new SavedAction(instance);
    instance->insertItem(WatchPoint, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("ShowStandardNamespace"));
    item->setText(tr("Show \"std::\" Namespace in Types"));
    item->setCheckable(true);
    item->setDefaultValue(true);
    item->setValue(true);
    instance->insertItem(ShowStdNamespace, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("ShowQtNamespace"));
    item->setText(tr("Show Qt's Namespace in Types"));
    item->setCheckable(true);
    item->setDefaultValue(true);
    item->setValue(true);
    instance->insertItem(ShowQtNamespace, item);

    //
    // DebuggingHelper
    //
    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("UseDebuggingHelper"));
    item->setText(tr("Use Debugging Helpers"));
    item->setCheckable(true);
    item->setDefaultValue(true);
    item->setValue(true);
    instance->insertItem(UseDebuggingHelpers, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("UseCustomDebuggingHelperLocation"));
    item->setCheckable(true);
    item->setDefaultValue(false);
    item->setValue(false);
    instance->insertItem(UseCustomDebuggingHelperLocation, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("CustomDebuggingHelperLocation"));
    item->setCheckable(true);
    item->setDefaultValue(QString());
    item->setValue(QString());
    instance->insertItem(CustomDebuggingHelperLocation, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("DebugDebuggingHelpers"));
    item->setText(tr("Debug Debugging Helpers"));
    item->setCheckable(true);
    item->setDefaultValue(false);
    item->setValue(false);
    instance->insertItem(DebugDebuggingHelpers, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("UseCodeModel"));
    item->setText(tr("Use Code Model"));
    item->setToolTip(tr("Selecting this causes the C++ Code Model being asked "
      "for variable scope information. This might result in slightly faster "
      "debugger operation but may fail for optimized code."));
    item->setCheckable(true);
    item->setDefaultValue(true);
    item->setValue(true);
    instance->insertItem(UseCodeModel, item);

    item = new SavedAction(instance);
    item->setText(tr("Recheck Debugging Helper Availability"));
    instance->insertItem(RecheckDebuggingHelpers, item);

    //
    // Breakpoints
    //
    item = new SavedAction(instance);
    item->setText(tr("Synchronize Breakpoints"));
    instance->insertItem(SynchronizeBreakpoints, item);

    item = new SavedAction(instance);
    item->setText(tr("Use Precise Breakpoints"));
    item->setToolTip(tr("Selecting this causes breakpoint synchronization "
      "being done after each step. This results in up-to-date breakpoint "
      "information on whether a breakpoint has been resolved after "
      "loading shared libraries, but slows down stepping."));
    item->setCheckable(true);
    item->setDefaultValue(false);
    item->setValue(false);
    item->setSettingsKey(debugModeGroup, QLatin1String("UsePreciseBreakpoints"));
    instance->insertItem(UsePreciseBreakpoints, item);

    item = new SavedAction(instance);
    item->setText(tr("Break on \"throw\""));
    item->setCheckable(true);
    item->setDefaultValue(false);
    item->setValue(false);
    item->setSettingsKey(debugModeGroup, QLatin1String("BreakOnThrow"));
    instance->insertItem(BreakOnThrow, item);

    item = new SavedAction(instance);
    item->setText(tr("Break on \"catch\""));
    item->setCheckable(true);
    item->setDefaultValue(false);
    item->setValue(false);
    item->setSettingsKey(debugModeGroup, QLatin1String("BreakOnCatch"));
    instance->insertItem(BreakOnCatch, item);

    //
    // Settings
    //

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("Environment"));
    item->setDefaultValue(QString());
    instance->insertItem(GdbEnvironment, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("ScriptFile"));
    item->setDefaultValue(QString());
    instance->insertItem(GdbScriptFile, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("AutoQuit"));
    item->setText(tr("Automatically Quit Debugger"));
    item->setCheckable(true);
    item->setDefaultValue(false);
    instance->insertItem(AutoQuit, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("UseToolTips"));
    item->setText(tr("Use tooltips in main editor when debugging"));
    item->setToolTip(tr("Checking this will enable tooltips for variable "
        "values during debugging. Since this can slow down debugging and "
        "does not provide reliable information as it does not use scope "
        "information, it is switched off by default."));
    item->setCheckable(true);
    item->setDefaultValue(false);
    instance->insertItem(UseToolTipsInMainEditor, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("UseToolTipsInLocalsView"));
    item->setText(tr("Use Tooltips in Locals View When Debugging"));
    item->setToolTip(tr("Checking this will enable tooltips in the locals "
        "view during debugging."));
    item->setCheckable(true);
    item->setDefaultValue(false);
    instance->insertItem(UseToolTipsInLocalsView, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("UseToolTipsInBreakpointsView"));
    item->setText(tr("Use Tooltips in Breakpoints View When Debugging"));
    item->setToolTip(tr("Checking this will enable tooltips in the breakpoints "
        "view during debugging."));
    item->setCheckable(true);
    item->setDefaultValue(false);
    instance->insertItem(UseToolTipsInBreakpointsView, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("UseAddressInBreakpointsView"));
    item->setText(tr("Show Address Data in Breakpoints View When Debugging"));
    item->setToolTip(tr("Checking this will show a column with address "
        "information in the breakpoint view during debugging."));
    item->setCheckable(true);
    item->setDefaultValue(false);
    instance->insertItem(UseAddressInBreakpointsView, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("UseAddressInStackView"));
    item->setText(tr("Show Address Data in Stack View When Debugging"));
    item->setToolTip(tr("Checking this will show a column with address "
        "information in the stack view during debugging."));
    item->setCheckable(true);
    item->setDefaultValue(false);
    instance->insertItem(UseAddressInStackView, item);
    item = new SavedAction(instance);

    item->setSettingsKey(debugModeGroup, QLatin1String("ListSourceFiles"));
    item->setText(tr("List Source Files"));
    item->setCheckable(true);
    item->setDefaultValue(false);
    instance->insertItem(ListSourceFiles, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("SkipKnownFrames"));
    item->setText(tr("Skip Known Frames"));
    item->setToolTip(tr("Selecting this results in well-known but usually "
      "not interesting frames belonging to reference counting and "
      "signal emission being skipped while single-stepping."));
    item->setCheckable(true);
    item->setDefaultValue(false);
    instance->insertItem(SkipKnownFrames, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("EnableReverseDebugging"));
    item->setText(tr("Enable Reverse Debugging"));
    item->setCheckable(true);
    item->setDefaultValue(false);
    instance->insertItem(EnableReverseDebugging, item);

#ifdef Q_OS_WIN
    item = new RegisterPostMortemAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("RegisterForPostMortem"));
    item->setText(tr("Register For Post-Mortem Debugging"));
    item->setCheckable(true);
    item->setDefaultValue(false);
    instance->insertItem(RegisterForPostMortem, item);
#endif

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("AllPluginBreakpoints"));
    item->setDefaultValue(true);
    instance->insertItem(AllPluginBreakpoints, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("SelectedPluginBreakpoints"));
    item->setDefaultValue(false);
    instance->insertItem(SelectedPluginBreakpoints, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("NoPluginBreakpoints"));
    item->setDefaultValue(false);
    instance->insertItem(NoPluginBreakpoints, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("SelectedPluginBreakpointsPattern"));
    item->setDefaultValue(QLatin1String(".*"));
    instance->insertItem(SelectedPluginBreakpointsPattern, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("MaximalStackDepth"));
    item->setDefaultValue(20);
    instance->insertItem(MaximalStackDepth, item);

    item = new SavedAction(instance);
    item->setText(tr("Reload Full Stack"));
    instance->insertItem(ExpandStack, item);

    item = new SavedAction(instance);
    item->setText(tr("Create Full Backtrace"));
    instance->insertItem(CreateFullBacktrace, item);

    item = new SavedAction(instance);
    item->setText(tr("Execute Line"));
    instance->insertItem(ExecuteCommand, item);

    item = new SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("WatchdogTimeout"));
    item->setDefaultValue(20);
    instance->insertItem(GdbWatchdogTimeout, item);

    // Language switching
    item = new Utils::SavedAction(instance);
    item->setSettingsKey(debugModeGroup, QLatin1String("ChangeLanguageAutomatically"));
    item->setText(tr("Change debugger language automatically"));
    item->setToolTip(tr("Changes the debugger language according to the currently opened file."));
    item->setCheckable(true);
    item->setDefaultValue(true);
    instance->insertItem(SwitchLanguageAutomatically, item);

    return instance;
}


//////////////////////////////////////////////////////////////////////////
//
// DebuggerActions
//
//////////////////////////////////////////////////////////////////////////

SavedAction *theDebuggerAction(int code)
{
    return DebuggerSettings::instance()->item(code);
}

bool theDebuggerBoolSetting(int code)
{
    return DebuggerSettings::instance()->item(code)->value().toBool();
}

QString theDebuggerStringSetting(int code)
{
    return DebuggerSettings::instance()->item(code)->value().toString();
}

} // namespace Internal
} // namespace Debugger

