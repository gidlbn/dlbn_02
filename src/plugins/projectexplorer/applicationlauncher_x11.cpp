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

#include "applicationlauncher.h"
#include "consoleprocess.h"

#include <coreplugin/icore.h>

#include <QtCore/QTimer>

using namespace ProjectExplorer;
using namespace Utils;

ApplicationLauncher::ApplicationLauncher(QObject *parent)
    : QObject(parent)
{
    m_outputCodec = QTextCodec::codecForLocale();
    m_currentMode = Gui;
    m_guiProcess = new QProcess(this);
    m_guiProcess->setReadChannelMode(QProcess::SeparateChannels);
    connect(m_guiProcess, SIGNAL(error(QProcess::ProcessError)),
        this, SLOT(guiProcessError()));
    connect(m_guiProcess, SIGNAL(readyReadStandardOutput()),
        this, SLOT(readStandardOutput()));
    connect(m_guiProcess, SIGNAL(readyReadStandardError()),
        this, SLOT(readStandardError()));
    connect(m_guiProcess, SIGNAL(finished(int, QProcess::ExitStatus)),
            this, SLOT(processDone(int, QProcess::ExitStatus)));
    connect(m_guiProcess, SIGNAL(started()),
            this, SLOT(bringToForeground()));

    m_consoleProcess = new ConsoleProcess(this);
    m_consoleProcess->setSettings(Core::ICore::instance()->settings());
    connect(m_consoleProcess, SIGNAL(processMessage(QString,bool)),
            this, SIGNAL(appendMessage(QString,bool)));
    connect(m_consoleProcess, SIGNAL(processStopped()),
            this, SLOT(processStopped()));
}

void ApplicationLauncher::setWorkingDirectory(const QString &dir)
{
    m_guiProcess->setWorkingDirectory(dir);
    m_consoleProcess->setWorkingDirectory(dir);
}

void ApplicationLauncher::setEnvironment(const QStringList &env)
{
    m_guiProcess->setEnvironment(env);
    m_consoleProcess->setEnvironment(env);
}

void ApplicationLauncher::start(Mode mode, const QString &program, const QStringList &args)
{
    m_currentMode = mode;
    if (mode == Gui) {
        m_guiProcess->start(program, args);
    } else {
        m_consoleProcess->start(program, args);
    }
}

void ApplicationLauncher::stop()
{
    if (m_currentMode == Gui) {
        m_guiProcess->terminate();
        if (!m_guiProcess->waitForFinished(1000)) { // This is blocking, so be fast.
            m_guiProcess->kill();
            m_guiProcess->waitForFinished();
        }
    } else {
        m_consoleProcess->stop();
    }
}

bool ApplicationLauncher::isRunning() const
{
    if (m_currentMode == Gui)
        return m_guiProcess->state() != QProcess::NotRunning;
    else
        return m_consoleProcess->isRunning();
}

qint64 ApplicationLauncher::applicationPID() const
{
    qint64 result = 0;
    if (!isRunning())
        return result;

    if (m_currentMode == Console) {
        result = m_consoleProcess->applicationPID();
    } else {
        result = (qint64)m_guiProcess->pid();
    }
    return result;
}

void ApplicationLauncher::guiProcessError()
{
    QString error;
    switch (m_guiProcess->error()) {
    case QProcess::FailedToStart:
        error = tr("Failed to start program. Path or permissions wrong?");
        break;
    case QProcess::Crashed:
        error = tr("The program has unexpectedly finished.");
        break;
    default:
        error = tr("Some error has occurred while running the program.");
    }
    emit appendMessage(error, true);
}

void ApplicationLauncher::readStandardOutput()
{
    QByteArray data = m_guiProcess->readAllStandardOutput();
    emit appendOutput(m_outputCodec->toUnicode(
            data.constData(), data.length(), &m_outputCodecState),
                      false);
}

void ApplicationLauncher::readStandardError()
{
    QByteArray data = m_guiProcess->readAllStandardError();
    emit appendOutput(m_outputCodec->toUnicode(
            data.constData(), data.length(), &m_errorCodecState),
                      true);
}

void ApplicationLauncher::processStopped()
{
    emit processExited(0);
}

void ApplicationLauncher::processDone(int exitCode, QProcess::ExitStatus)
{
    emit processExited(exitCode);
}

void ApplicationLauncher::bringToForeground()
{
    emit bringToForegroundRequested(applicationPID());
}
