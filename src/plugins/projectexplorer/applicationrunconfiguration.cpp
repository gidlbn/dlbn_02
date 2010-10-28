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

#include "applicationrunconfiguration.h"

#include "environment.h"

#include <projectexplorer/projectexplorerconstants.h>
#include <utils/qtcassert.h>

#include <QtCore/QDir>
#include <QtGui/QLabel>

using namespace ProjectExplorer;
using namespace ProjectExplorer::Internal;

/// LocalApplicationRunConfiguration

LocalApplicationRunConfiguration::LocalApplicationRunConfiguration(Target *target, const QString &id) :
    RunConfiguration(target, id)
{
}

LocalApplicationRunConfiguration::LocalApplicationRunConfiguration(Target *target, LocalApplicationRunConfiguration *rc) :
    RunConfiguration(target, rc)
{
}

LocalApplicationRunConfiguration::~LocalApplicationRunConfiguration()
{
}

/// LocalApplicationRunControlFactory

LocalApplicationRunControlFactory::LocalApplicationRunControlFactory()
{
}

LocalApplicationRunControlFactory::~LocalApplicationRunControlFactory()
{
}

bool LocalApplicationRunControlFactory::canRun(ProjectExplorer::RunConfiguration *runConfiguration, const QString &mode) const
{
    return (mode == ProjectExplorer::Constants::RUNMODE)
            && (qobject_cast<LocalApplicationRunConfiguration *>(runConfiguration) != 0);
}

QString LocalApplicationRunControlFactory::displayName() const
{
    return tr("Run");
}

RunControl *LocalApplicationRunControlFactory::create(ProjectExplorer::RunConfiguration *runConfiguration, const QString &mode)
{
    QTC_ASSERT(canRun(runConfiguration, mode), return 0);
    return new LocalApplicationRunControl(qobject_cast<LocalApplicationRunConfiguration *>(runConfiguration));
}

QWidget *LocalApplicationRunControlFactory::createConfigurationWidget(RunConfiguration *runConfiguration)
{
    Q_UNUSED(runConfiguration)
    return new QLabel("TODO add Configuration widget");
}

// ApplicationRunControl

LocalApplicationRunControl::LocalApplicationRunControl(LocalApplicationRunConfiguration *runConfiguration)
    : RunControl(runConfiguration)
{
    m_applicationLauncher.setEnvironment(runConfiguration->environment().toStringList());
    m_applicationLauncher.setWorkingDirectory(runConfiguration->workingDirectory());

    m_executable = runConfiguration->executable();
    m_runMode = static_cast<ApplicationLauncher::Mode>(runConfiguration->runMode());
    m_commandLineArguments = runConfiguration->commandLineArguments();

    connect(&m_applicationLauncher, SIGNAL(appendMessage(QString,bool)),
            this, SLOT(slotAppendMessage(QString,bool)));
    connect(&m_applicationLauncher, SIGNAL(appendOutput(QString, bool)),
            this, SLOT(slotAddToOutputWindow(QString, bool)));
    connect(&m_applicationLauncher, SIGNAL(processExited(int)),
            this, SLOT(processExited(int)));
    connect(&m_applicationLauncher, SIGNAL(bringToForegroundRequested(qint64)),
            this, SLOT(bringApplicationToForeground(qint64)));
}

LocalApplicationRunControl::~LocalApplicationRunControl()
{
}

void LocalApplicationRunControl::start()
{
    m_applicationLauncher.start(m_runMode, m_executable, m_commandLineArguments);
    emit started();

    emit appendMessage(this, tr("Starting %1...").arg(QDir::toNativeSeparators(m_executable)), false);
}

void LocalApplicationRunControl::stop()
{
    m_applicationLauncher.stop();
}

bool LocalApplicationRunControl::isRunning() const
{
    return m_applicationLauncher.isRunning();
}

void LocalApplicationRunControl::slotAppendMessage(const QString &err,
                                                   bool isError)
{
    emit appendMessage(this, err, isError);
    emit finished();
}

void LocalApplicationRunControl::slotAddToOutputWindow(const QString &line,
                                                       bool isError)
{
    emit addToOutputWindowInline(this, line, isError);
}

void LocalApplicationRunControl::processExited(int exitCode)
{
    emit appendMessage(this, tr("%1 exited with code %2").arg(QDir::toNativeSeparators(m_executable)).arg(exitCode), false);
    emit finished();
}

