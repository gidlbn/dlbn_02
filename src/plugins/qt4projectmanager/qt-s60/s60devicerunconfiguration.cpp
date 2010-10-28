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

#include "s60devicerunconfiguration.h"
#include "s60devicerunconfigurationwidget.h"
#include "qt4project.h"
#include "qt4target.h"
#include "qtversionmanager.h"
#include "profilereader.h"
#include "s60manager.h"
#include "s60devices.h"
#include "s60runconfigbluetoothstarter.h"
#include "bluetoothlistener_gui.h"
#include "symbiandevicemanager.h"
#include "qt4buildconfiguration.h"
#include "qt4projectmanagerconstants.h"

#include <coreplugin/icore.h>
#include <coreplugin/messagemanager.h>
#include <coreplugin/progressmanager/progressmanager.h>
#include <utils/qtcassert.h>
#include <utils/pathchooser.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/project.h>
#include <projectexplorer/buildconfiguration.h>

#include <debugger/debuggermanager.h>

#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>
#include <QtCore/QCoreApplication>

using namespace ProjectExplorer;
using namespace Qt4ProjectManager;
using namespace Qt4ProjectManager::Internal;

namespace {
const char * const S60_DEVICE_RC_ID("Qt4ProjectManager.S60DeviceRunConfiguration");
const char * const S60_DEVICE_RC_PREFIX("Qt4ProjectManager.S60DeviceRunConfiguration.");

const char * const PRO_FILE_KEY("Qt4ProjectManager.S60DeviceRunConfiguration.ProFile");
const char * const SERIAL_PORT_NAME_KEY("Qt4ProjectManager.S60DeviceRunConfiguration.SerialPortName");
const char * const COMMUNICATION_TYPE_KEY("Qt4ProjectManager.S60DeviceRunConfiguration.CommunicationType");
const char * const COMMAND_LINE_ARGUMENTS_KEY("Qt4ProjectManager.S60DeviceRunConfiguration.CommandLineArguments");

const int    PROGRESS_DEPLOYBASE = 0;
const int    PROGRESS_PACKAGEDEPLOYED = 100;
const int    PROGRESS_PACKAGEINSTALLED = 200;
const int    PROGRESS_MAX = 200;

enum { debug = 0 };

// Format information about a file
static inline QString msgListFile(const QString &f)
{
    QString rc;
    const QFileInfo fi(f);
    QTextStream str(&rc);
    if (fi.exists()) {
        str << fi.size() << ' ' << fi.lastModified().toString(Qt::ISODate) << ' ' << QDir::toNativeSeparators(fi.absoluteFilePath());
    } else {
        str << "<non-existent> " << QDir::toNativeSeparators(fi.absoluteFilePath());
    }
    return rc;
}

QString pathFromId(const QString &id)
{
    if (!id.startsWith(QLatin1String(S60_DEVICE_RC_PREFIX)))
        return QString();
    return id.mid(QString::fromLatin1(S60_DEVICE_RC_PREFIX).size());
}

QString pathToId(const QString &path)
{
    return QString::fromLatin1(S60_DEVICE_RC_PREFIX) + path;
}

}

// ======== S60DeviceRunConfiguration

S60DeviceRunConfiguration::S60DeviceRunConfiguration(Target *parent, const QString &proFilePath) :
    RunConfiguration(parent,  QLatin1String(S60_DEVICE_RC_ID)),
    m_proFilePath(proFilePath),
#ifdef Q_OS_WIN
    m_serialPortName(QLatin1String("COM5"))
#else
    m_serialPortName(QLatin1String(SymbianUtils::SymbianDeviceManager::linuxBlueToothDeviceRootC) + QLatin1Char('0'))
#endif
{
    ctor();
}

S60DeviceRunConfiguration::S60DeviceRunConfiguration(Target *target, S60DeviceRunConfiguration *source) :
    RunConfiguration(target, source),
    m_proFilePath(source->m_proFilePath),
    m_serialPortName(source->m_serialPortName)
{
    ctor();
}

void S60DeviceRunConfiguration::ctor()
{
    if (!m_proFilePath.isEmpty())
        setDisplayName(tr("%1 on Symbian Device").arg(QFileInfo(m_proFilePath).completeBaseName()));
    else
        setDisplayName(tr("QtS60DeviceRunConfiguration"));

    connect(qt4Target()->qt4Project(), SIGNAL(proFileUpdated(Qt4ProjectManager::Internal::Qt4ProFileNode*)),
            this, SLOT(proFileUpdate(Qt4ProjectManager::Internal::Qt4ProFileNode*)));
}

void S60DeviceRunConfiguration::proFileUpdate(Qt4ProjectManager::Internal::Qt4ProFileNode *pro)
{
    if (m_proFilePath == pro->path())
        emit targetInformationChanged();
}

S60DeviceRunConfiguration::~S60DeviceRunConfiguration()
{
}

Qt4Target *S60DeviceRunConfiguration::qt4Target() const
{
    return static_cast<Qt4Target *>(target());
}

ProjectExplorer::ToolChain::ToolChainType S60DeviceRunConfiguration::toolChainType(
        ProjectExplorer::BuildConfiguration *configuration) const
{
    if (Qt4BuildConfiguration *bc = qobject_cast<Qt4BuildConfiguration *>(configuration))
        return bc->toolChainType();
    return ProjectExplorer::ToolChain::INVALID;
}

ProjectExplorer::ToolChain::ToolChainType S60DeviceRunConfiguration::toolChainType() const
{
    if (Qt4BuildConfiguration *bc = qobject_cast<Qt4BuildConfiguration *>(target()->activeBuildConfiguration()))
        return bc->toolChainType();
    return ProjectExplorer::ToolChain::INVALID;
}

bool S60DeviceRunConfiguration::isEnabled(ProjectExplorer::BuildConfiguration *configuration) const
{
    const Qt4BuildConfiguration *qt4bc = static_cast<const Qt4BuildConfiguration *>(configuration);
    switch (qt4bc->toolChainType()) {
    case ToolChain::GCCE:
    case ToolChain::RVCT_ARMV5:
    case ToolChain::RVCT_ARMV6:
    case ToolChain::GCCE_GNUPOC:
    case ToolChain::RVCT_ARMV5_GNUPOC:
        return true;
    default:
        break;
    }
    return false;
}

QWidget *S60DeviceRunConfiguration::createConfigurationWidget()
{
    return new S60DeviceRunConfigurationWidget(this);
}

QVariantMap S60DeviceRunConfiguration::toMap() const
{
    QVariantMap map(ProjectExplorer::RunConfiguration::toMap());
    const QDir projectDir = QDir(target()->project()->projectDirectory());

    map.insert(QLatin1String(PRO_FILE_KEY), projectDir.relativeFilePath(m_proFilePath));
    map.insert(QLatin1String(SERIAL_PORT_NAME_KEY), m_serialPortName);
    map.insert(QLatin1String(COMMAND_LINE_ARGUMENTS_KEY), m_commandLineArguments);

    return map;
}

bool S60DeviceRunConfiguration::fromMap(const QVariantMap &map)
{
    const QDir projectDir = QDir(target()->project()->projectDirectory());

    m_proFilePath = projectDir.filePath(map.value(QLatin1String(PRO_FILE_KEY)).toString());
    m_serialPortName = map.value(QLatin1String(SERIAL_PORT_NAME_KEY)).toString().trimmed();
    m_commandLineArguments = map.value(QLatin1String(COMMAND_LINE_ARGUMENTS_KEY)).toStringList();

    return RunConfiguration::fromMap(map);
}

QString S60DeviceRunConfiguration::serialPortName() const
{
    return m_serialPortName;
}

void S60DeviceRunConfiguration::setSerialPortName(const QString &name)
{
    const QString &candidate = name.trimmed();
    if (m_serialPortName == candidate)
        return;
    m_serialPortName = candidate;
    emit serialPortNameChanged();
}

QString S60DeviceRunConfiguration::targetName() const
{
    TargetInformation ti = qt4Target()->qt4Project()->rootProjectNode()->targetInformation(m_proFilePath);
    if (!ti.valid)
        return QString();
    return ti.target;
}

static inline QString fixBaseNameTarget(const QString &in)
{
    if (in == QLatin1String("udeb"))
        return QLatin1String("debug");
    if (in == QLatin1String("urel"))
        return QLatin1String("release");
    return in;
}

QString S60DeviceRunConfiguration::packageFileNameWithTargetInfo() const
{
    TargetInformation ti = qt4Target()->qt4Project()->rootProjectNode()->targetInformation(m_proFilePath);
    if (!ti.valid)
        return QString();
    QString baseFileName = ti.buildDir + QLatin1Char('/') + ti.target;
    baseFileName += QLatin1Char('_')
                    + (isDebug() ? QLatin1String("debug") : QLatin1String("release"))
                    + QLatin1Char('-') + symbianPlatform() + QLatin1String(".sis");
    return baseFileName;
}

QString S60DeviceRunConfiguration::symbianPlatform() const
{
    Qt4BuildConfiguration *qt4bc = qt4Target()->qt4Project()->activeTarget()->activeBuildConfiguration();
    switch (qt4bc->toolChainType()) {
    case ToolChain::GCCE:
    case ToolChain::GCCE_GNUPOC:
        return QLatin1String("gcce");
    case ToolChain::RVCT_ARMV5:
        return QLatin1String("armv5");
    default: // including ToolChain::RVCT_ARMV6_GNUPOC:
        return QLatin1String("armv6");
    }
}

bool S60DeviceRunConfiguration::isDebug() const
{
    const Qt4BuildConfiguration *qt4bc = qt4Target()->qt4Project()->activeTarget()->activeBuildConfiguration();
    return (qt4bc->qmakeBuildConfiguration() & QtVersion::DebugBuild);
}

QString S60DeviceRunConfiguration::symbianTarget() const
{
    return isDebug() ? QLatin1String("udeb") : QLatin1String("urel");
}

QString S60DeviceRunConfiguration::packageTemplateFileName() const
{
    TargetInformation ti = qt4Target()->qt4Project()->rootProjectNode()->targetInformation(m_proFilePath);
    if (!ti.valid)
        return QString();
    return ti.buildDir + QLatin1Char('/') + ti.target + QLatin1String("_template.pkg");
}

/* Grep a package file for the '.exe' file. Curently for use on Linux only
 * as the '.pkg'-files on Windows do not contain drive letters, which is not
 * handled here. \code
; Executable and default resource files
"./foo.exe"    - "!:\sys\bin\foo.exe"
\endcode  */

static inline QString executableFromPackageUnix(const QString &packageFileName)
{
    QFile packageFile(packageFileName);
    if (!packageFile.open(QIODevice::ReadOnly|QIODevice::Text))
        return QString();
    QRegExp pattern(QLatin1String("^\"(.*.exe)\" *- \"!:.*.exe\"$"));
    QTC_ASSERT(pattern.isValid(), return QString());
    foreach(const QString &line, QString::fromLocal8Bit(packageFile.readAll()).split(QLatin1Char('\n')))
        if (pattern.exactMatch(line)) {
            // Expand relative paths by package file paths
            QString rc = pattern.cap(1);
            if (rc.startsWith(QLatin1String("./")))
                rc.remove(0, 2);
            const QFileInfo fi(rc);
            if (fi.isAbsolute())
                return rc;
            return QFileInfo(packageFileName).absolutePath() + QLatin1Char('/') + rc;
        }
    return QString();
}

const QtVersion *S60DeviceRunConfiguration::qtVersion() const
{
    if (const BuildConfiguration *bc = target()->activeBuildConfiguration())
        if (const Qt4BuildConfiguration *qt4bc = qobject_cast<const Qt4BuildConfiguration *>(bc))
            return qt4bc->qtVersion();
    return 0;
}

QString S60DeviceRunConfiguration::localExecutableFileName() const
{
    QString localExecutable;
    switch (toolChainType()) {
    case ToolChain::GCCE_GNUPOC:
    case ToolChain::RVCT_ARMV5_GNUPOC:
        localExecutable = executableFromPackageUnix(packageTemplateFileName());
        break;
    default: {
            const QtVersion *qtv = qtVersion();
            QTC_ASSERT(qtv, return QString());
            const S60Devices::Device device = S60Manager::instance()->deviceForQtVersion(qtv);
            QTextStream(&localExecutable) << device.epocRoot << "/epoc32/release/"
                    << symbianPlatform() << '/' << symbianTarget() << '/' << targetName()
                    << ".exe";
        }
        break;
    }
    if (debug)
        qDebug() << "Local executable" << localExecutable;
    return QDir::toNativeSeparators(localExecutable);
}

QString S60DeviceRunConfiguration::signedPackage() const
{
    TargetInformation ti = qt4Target()->qt4Project()->rootProjectNode()->targetInformation(m_proFilePath);
    if (!ti.valid)
        return QString();
    return ti.buildDir + QLatin1Char('/') + ti.target + QLatin1String(".sis");
}

QStringList S60DeviceRunConfiguration::commandLineArguments() const
{
    return m_commandLineArguments;
}

void S60DeviceRunConfiguration::setCommandLineArguments(const QStringList &args)
{
    m_commandLineArguments = args;
}

// ======== S60DeviceRunConfigurationFactory

S60DeviceRunConfigurationFactory::S60DeviceRunConfigurationFactory(QObject *parent) :
    IRunConfigurationFactory(parent)
{
}

S60DeviceRunConfigurationFactory::~S60DeviceRunConfigurationFactory()
{
}

QStringList S60DeviceRunConfigurationFactory::availableCreationIds(Target *parent) const
{
    Qt4Target *target = qobject_cast<Qt4Target *>(parent);
    if (!target ||
        target->id() != QLatin1String(Constants::S60_DEVICE_TARGET_ID))
        return QStringList();

    return target->qt4Project()->applicationProFilePathes(QLatin1String(S60_DEVICE_RC_PREFIX));
}

QString S60DeviceRunConfigurationFactory::displayNameForId(const QString &id) const
{
    if (!pathFromId(id).isEmpty())
        return tr("%1 on Symbian Device").arg(QFileInfo(pathFromId(id)).completeBaseName());
    return QString();
}

bool S60DeviceRunConfigurationFactory::canCreate(Target *parent, const QString &id) const
{
    Qt4Target * t(qobject_cast<Qt4Target *>(parent));
    if (!t ||
        t->id() != QLatin1String(Constants::S60_DEVICE_TARGET_ID))
        return false;
    return t->qt4Project()->hasApplicationProFile(pathFromId(id));
}

RunConfiguration *S60DeviceRunConfigurationFactory::create(Target *parent, const QString &id)
{
    if (!canCreate(parent, id))
        return 0;

    Qt4Target *t(static_cast<Qt4Target *>(parent));
    return new S60DeviceRunConfiguration(t, pathFromId(id));
}

bool S60DeviceRunConfigurationFactory::canRestore(Target *parent, const QVariantMap &map) const
{
    Qt4Target * t(qobject_cast<Qt4Target *>(parent));
    if (!t ||
        t->id() != QLatin1String(Constants::S60_DEVICE_TARGET_ID))
        return false;
    QString id(ProjectExplorer::idFromMap(map));
    return id == QLatin1String(S60_DEVICE_RC_ID);
}

RunConfiguration *S60DeviceRunConfigurationFactory::restore(Target *parent, const QVariantMap &map)
{
    if (!canRestore(parent, map))
        return 0;
    Qt4Target *t(static_cast<Qt4Target *>(parent));
    S60DeviceRunConfiguration *rc(new S60DeviceRunConfiguration(t, QString()));
    if (rc->fromMap(map))
        return rc;

    delete rc;
    return 0;
}

bool S60DeviceRunConfigurationFactory::canClone(Target *parent, RunConfiguration *source) const
{
    if (!qobject_cast<Qt4Target *>(parent))
        return false;
    return source->id() == QLatin1String(S60_DEVICE_RC_ID);
}

RunConfiguration *S60DeviceRunConfigurationFactory::clone(Target *parent, RunConfiguration *source)
{
    if (!canClone(parent, source))
        return 0;
    Qt4Target *t = static_cast<Qt4Target *>(parent);
    S60DeviceRunConfiguration * old(static_cast<S60DeviceRunConfiguration *>(source));
    return new S60DeviceRunConfiguration(t, old);
}

// ======== S60DeviceRunControlBase

S60DeviceRunControlBase::S60DeviceRunControlBase(RunConfiguration *runConfiguration) :
    RunControl(runConfiguration),
    m_toolChain(ProjectExplorer::ToolChain::INVALID),
    m_releaseDeviceAfterLauncherFinish(false),
    m_handleDeviceRemoval(true),
    m_launcher(0)
{
    // connect for automatically reporting the "finished deploy" state to the progress manager
    connect(this, SIGNAL(finished()), this, SLOT(reportDeployFinished()));

    S60DeviceRunConfiguration *s60runConfig = qobject_cast<S60DeviceRunConfiguration *>(runConfiguration);
    const Qt4BuildConfiguration *activeBuildConf = s60runConfig->qt4Target()->activeBuildConfiguration();

    QTC_ASSERT(s60runConfig, return);
    m_toolChain = s60runConfig->toolChainType();
    m_serialPortName = s60runConfig->serialPortName();
    m_serialPortFriendlyName = SymbianUtils::SymbianDeviceManager::instance()->friendlyNameForPort(m_serialPortName);
    m_targetName = s60runConfig->targetName();
    m_packageFileNameWithTarget = s60runConfig->packageFileNameWithTargetInfo();
    m_signedPackage = s60runConfig->signedPackage();
    m_commandLineArguments = s60runConfig->commandLineArguments();
    m_workingDirectory = QFileInfo(m_signedPackage).absolutePath();
    m_qtDir = activeBuildConf->qtVersion()->versionInfo().value("QT_INSTALL_DATA");
    if (const QtVersion *qtv = s60runConfig->qtVersion())
        m_qtBinPath = qtv->versionInfo().value(QLatin1String("QT_INSTALL_BINS"));
    QTC_ASSERT(!m_qtBinPath.isEmpty(), return);
    m_executableFileName = s60runConfig->localExecutableFileName();
    if (debug)
        qDebug() << "S60DeviceRunControlBase::CT" << m_targetName << ProjectExplorer::ToolChain::toolChainName(m_toolChain)
                 << m_serialPortName << m_workingDirectory;
}

S60DeviceRunControlBase::~S60DeviceRunControlBase()
{
    if (m_launcher) {
        m_launcher->deleteLater();
        m_launcher = 0;
    }
}

void S60DeviceRunControlBase::setReleaseDeviceAfterLauncherFinish(bool v)
{
    m_releaseDeviceAfterLauncherFinish = v;
}

static inline bool ensureDeleteFile(const QString &fileName, QString *errorMessage)
{
    QFile file(fileName);
    if (file.exists() && !file.remove()) {
        *errorMessage = S60DeviceRunControlBase::tr("Unable to remove existing file '%1': %2").arg(fileName, file.errorString());
        return false;
    }
    return true;
}

static inline bool renameFile(const QString &sourceName, const QString &targetName,
                              QString *errorMessage)
{
    if (sourceName == targetName)
        return true;
    if (!ensureDeleteFile(targetName, errorMessage))
        return false;
    QFile source(sourceName);
    if (!source.rename(targetName)) {
        *errorMessage = S60DeviceRunControlBase::tr("Unable to rename file '%1' to '%2': %3")
                        .arg(sourceName, targetName, source.errorString());
        return false;
    }
    return true;
}

void S60DeviceRunControlBase::start()
{
    m_deployProgress = new QFutureInterface<void>;
    Core::ICore::instance()->progressManager()->addTask(m_deployProgress->future(),
                                                        tr("Deploying"),
                                                        QLatin1String("Symbian.Deploy"));
    m_deployProgress->setProgressRange(0, PROGRESS_MAX);
    m_deployProgress->setProgressValue(0);
    m_deployProgress->reportStarted();
    emit started();
    if (m_serialPortName.isEmpty()) {
        m_deployProgress->reportCanceled();
        appendMessage(this, tr("There is no device plugged in."), true);
        emit finished();
        return;
    }

    emit appendMessage(this, tr("Executable file: %1").arg(msgListFile(m_executableFileName)), false);

    QString errorMessage;
    QString settingsCategory;
    QString settingsPage;
    if (!checkConfiguration(&errorMessage, &settingsCategory, &settingsPage)) {
        m_deployProgress->reportCanceled();
        appendMessage(this, errorMessage, true);
        emit finished();
        Core::ICore::instance()->showWarningWithOptions(tr("Debugger for Symbian Platform"),
                                                        errorMessage, QString(),
                                                        settingsCategory, settingsPage);
        return;
    }

    // make sure we have the right name of the sis package
    bool ok = true;
    QFileInfo packageInfo(m_signedPackage);
    {
        // support for 4.6.1 and pre, where make sis creates 'targetname_armX_udeb.sis' instead of 'targetname.sis'
        QFileInfo packageWithTargetInfo(m_packageFileNameWithTarget);
        // does the 4.6.1 version exist?
        if (packageWithTargetInfo.exists() && packageWithTargetInfo.isFile()) {
            // is the 4.6.1 version newer? (to guard against behavior change Qt Creator 1.3 --> 2.0)
            if (!packageInfo.exists()
                    || packageInfo.lastModified() < packageWithTargetInfo.lastModified()) {
                // the 'targetname_armX_udeb.sis' crap exists and is new, rename it
                emit appendMessage(this, tr("Renaming new package '%1' to '%2'")
                                   .arg(QDir::toNativeSeparators(m_packageFileNameWithTarget),
                                        QDir::toNativeSeparators(m_signedPackage)), false);
                ok = renameFile(m_packageFileNameWithTarget, m_signedPackage, &errorMessage);
            } else {
                // the 'targetname_armX_udeb.sis' crap exists but is old, remove it
                emit appendMessage(this, tr("Removing old package '%1'")
                                   .arg(QDir::toNativeSeparators(m_packageFileNameWithTarget)),
                                   false);
                QFile::remove(m_packageFileNameWithTarget);
            }
        }
    }
    if (ok) {
        if (!packageInfo.exists() || !packageInfo.isFile()) {
            errorMessage = tr("Package file not found");
            ok = false;
        }
    }
    if (ok) {
        startDeployment();
    } else {
        m_deployProgress->reportCanceled();
        errorMessage = tr("Failed to find package '%1': %2").arg(m_signedPackage, errorMessage);
        appendMessage(this, errorMessage, true);
        stop();
        emit finished();
    }
}

static inline void stopProcess(QProcess *p)
{
    const int timeOutMS = 200;
    if (p->state() != QProcess::Running)
        return;
    p->terminate();
    if (p->waitForFinished(timeOutMS))
        return;
    p->kill();
}

void S60DeviceRunControlBase::stop()
{
    if (m_launcher)
        m_launcher->terminate();
}

bool S60DeviceRunControlBase::isRunning() const
{
    //TODO !!!
    return false;
}

void S60DeviceRunControlBase::startDeployment()
{
    QString errorMessage;
    bool success = false;
    do {
        connect(SymbianUtils::SymbianDeviceManager::instance(), SIGNAL(deviceRemoved(const SymbianUtils::SymbianDevice)),
                this, SLOT(deviceRemoved(SymbianUtils::SymbianDevice)));
        m_launcher = trk::Launcher::acquireFromDeviceManager(m_serialPortName, 0, &errorMessage);
        if (!m_launcher)
            break;

        connect(m_launcher, SIGNAL(finished()), this, SLOT(launcherFinished()));
        connect(m_launcher, SIGNAL(canNotConnect(QString)), this, SLOT(printConnectFailed(QString)));
        connect(m_launcher, SIGNAL(copyingStarted()), this, SLOT(printCopyingNotice()));
        connect(m_launcher, SIGNAL(canNotCreateFile(QString,QString)), this, SLOT(printCreateFileFailed(QString,QString)));
        connect(m_launcher, SIGNAL(canNotWriteFile(QString,QString)), this, SLOT(printWriteFileFailed(QString,QString)));
        connect(m_launcher, SIGNAL(canNotCloseFile(QString,QString)), this, SLOT(printCloseFileFailed(QString,QString)));
        connect(m_launcher, SIGNAL(installingStarted()), this, SLOT(printInstallingNotice()));
        connect(m_launcher, SIGNAL(canNotInstall(QString,QString)), this, SLOT(printInstallFailed(QString,QString)));
        connect(m_launcher, SIGNAL(installingFinished()), this, SLOT(printInstallingFinished()));
        connect(m_launcher, SIGNAL(copyProgress(int)), this, SLOT(printCopyProgress(int)));
        connect(m_launcher, SIGNAL(stateChanged(int)), this, SLOT(slotLauncherStateChanged(int)));
        connect(m_launcher, SIGNAL(processStopped(uint,uint,uint,QString)),
                this, SLOT(processStopped(uint,uint,uint,QString)));

        //TODO sisx destination and file path user definable
        if (!m_commandLineArguments.isEmpty())
            m_launcher->setCommandLineArgs(m_commandLineArguments);
        const QString copyDst = QString::fromLatin1("C:\\Data\\%1").arg(QFileInfo(m_signedPackage).fileName());
        const QString runFileName = QString::fromLatin1("C:\\sys\\bin\\%1.exe").arg(m_targetName);
        m_launcher->setCopyFileName(m_signedPackage, copyDst);
        m_launcher->setInstallFileName(copyDst);
        initLauncher(runFileName, m_launcher);
        emit appendMessage(this, tr("Package: %1\nDeploying application to '%2'...").arg(msgListFile(m_signedPackage), m_serialPortFriendlyName), false);
        // Prompt the user to start up the Blue tooth connection
        const trk::PromptStartCommunicationResult src =
            S60RunConfigBluetoothStarter::startCommunication(m_launcher->trkDevice(),
                                                             0, &errorMessage);
        if (src != trk::PromptStartCommunicationConnected)
            break;
        if (!m_launcher->startServer(&errorMessage)) {
            errorMessage = tr("Could not connect to phone on port '%1': %2\n"
                              "Check if the phone is connected and App TRK is running.").arg(m_serialPortName, errorMessage);
            break;
        }
        success = true;
    } while (false);

    if (!success) {
        if (!errorMessage.isEmpty())
            appendMessage(this, errorMessage, true);
        stop();
        emit finished();
    }
}

void S60DeviceRunControlBase::printCreateFileFailed(const QString &filename, const QString &errorMessage)
{
    emit appendMessage(this, tr("Could not create file %1 on device: %2").arg(filename, errorMessage), true);
}

void S60DeviceRunControlBase::printWriteFileFailed(const QString &filename, const QString &errorMessage)
{
    emit appendMessage(this, tr("Could not write to file %1 on device: %2").arg(filename, errorMessage), true);
}

void S60DeviceRunControlBase::printCloseFileFailed(const QString &filename, const QString &errorMessage)
{
    const QString msg = tr("Could not close file %1 on device: %2. It will be closed when App TRK is closed.");
    emit appendMessage(this, msg.arg(filename, errorMessage), true);
}

void S60DeviceRunControlBase::printConnectFailed(const QString &errorMessage)
{
    emit appendMessage(this, tr("Could not connect to App TRK on device: %1. Restarting App TRK might help.").arg(errorMessage), true);
}

void S60DeviceRunControlBase::printCopyingNotice()
{
    emit appendMessage(this, tr("Copying installation file..."), false);
}

void S60DeviceRunControlBase::printCopyProgress(int progress)
{
    m_deployProgress->setProgressValue(PROGRESS_DEPLOYBASE + progress);
}

void S60DeviceRunControlBase::printInstallingNotice()
{
    m_deployProgress->setProgressValue(PROGRESS_PACKAGEDEPLOYED);
    emit appendMessage(this, tr("Installing application..."), false);
}

void S60DeviceRunControlBase::printInstallingFinished()
{
    m_deployProgress->setProgressValue(PROGRESS_PACKAGEINSTALLED);
    m_deployProgress->reportFinished();
    delete m_deployProgress;
    m_deployProgress = 0;
}

void S60DeviceRunControlBase::printInstallFailed(const QString &filename, const QString &errorMessage)
{
    QTC_ASSERT(m_deployProgress, ;)
    if (m_deployProgress)
        m_deployProgress->reportCanceled();
    emit appendMessage(this, tr("Could not install from package %1 on device: %2").arg(filename, errorMessage), true);
}

void S60DeviceRunControlBase::launcherFinished()
{
    if (m_releaseDeviceAfterLauncherFinish) {
        m_handleDeviceRemoval = false;
        trk::Launcher::releaseToDeviceManager(m_launcher);
    }
    m_launcher->deleteLater();
    m_launcher = 0;
    handleLauncherFinished();
}

void S60DeviceRunControlBase::reportDeployFinished()
{
    if (m_deployProgress) {
        m_deployProgress->reportFinished();
        delete m_deployProgress;
        m_deployProgress = 0;
    }
}

void S60DeviceRunControlBase::processStopped(uint pc, uint pid, uint tid, const QString& reason)
{
    emit addToOutputWindow(this, trk::Launcher::msgStopped(pid, tid, pc, reason), false);
    m_launcher->terminate();
}

QMessageBox *S60DeviceRunControlBase::createTrkWaitingMessageBox(const QString &port, QWidget *parent)
{
    const QString title  = QCoreApplication::translate("Qt4ProjectManager::Internal::S60DeviceRunControlBase",
                                                       "Waiting for App TRK");
    const QString text = QCoreApplication::translate("Qt4ProjectManager::Internal::S60DeviceRunControlBase",
                                                     "Please start App TRK on %1.").arg(port);
    QMessageBox *rc = new QMessageBox(QMessageBox::Information, title, text,
                                      QMessageBox::Cancel, parent);
    return rc;
}

void S60DeviceRunControlBase::slotLauncherStateChanged(int s)
{
    if (s == trk::Launcher::WaitingForTrk) {
        QMessageBox *mb = S60DeviceRunControlBase::createTrkWaitingMessageBox(m_launcher->trkServerName(),
                                                     Core::ICore::instance()->mainWindow());
        connect(m_launcher, SIGNAL(stateChanged(int)), mb, SLOT(close()));
        connect(mb, SIGNAL(finished(int)), this, SLOT(slotWaitingForTrkClosed()));
        mb->open();
    }
}

void S60DeviceRunControlBase::slotWaitingForTrkClosed()
{
    if (m_launcher && m_launcher->state() == trk::Launcher::WaitingForTrk) {
        stop();
        appendMessage(this, tr("Canceled."), true);
        emit finished();
    }
}

void S60DeviceRunControlBase::printApplicationOutput(const QString &output)
{
    printApplicationOutput(output, false);
}

void S60DeviceRunControlBase::printApplicationOutput(const QString &output, bool onStdErr)
{
    emit addToOutputWindowInline(this, output, onStdErr);
}

void S60DeviceRunControlBase::deviceRemoved(const SymbianUtils::SymbianDevice &d)
{
    if (m_handleDeviceRemoval && d.portName() == m_serialPortName) {
        appendMessage(this, tr("The device '%1' has been disconnected").arg(d.friendlyName()), true);
        emit finished();
    }
}

bool S60DeviceRunControlBase::checkConfiguration(QString * /* errorMessage */,
                                                 QString * /* settingsCategory */,
                                                 QString * /* settingsPage */) const
{
    return true;
}

// =============== S60DeviceRunControl

S60DeviceRunControl::S60DeviceRunControl(ProjectExplorer::RunConfiguration *runConfiguration) :
    S60DeviceRunControlBase(runConfiguration)
{
}

void S60DeviceRunControl::initLauncher(const QString &executable, trk::Launcher *launcher)
{
     connect(launcher, SIGNAL(startingApplication()), this, SLOT(printStartingNotice()));
     connect(launcher, SIGNAL(applicationRunning(uint)), this, SLOT(printRunNotice(uint)));
     connect(launcher, SIGNAL(canNotRun(QString)), this, SLOT(printRunFailNotice(QString)));
     connect(launcher, SIGNAL(applicationOutputReceived(QString)), this, SLOT(printApplicationOutput(QString)));
     launcher->addStartupActions(trk::Launcher::ActionCopyInstallRun);
     launcher->setFileName(executable);
}

void S60DeviceRunControl::handleLauncherFinished()
{
     emit finished();
     emit appendMessage(this, tr("Finished."), false);
 }

void S60DeviceRunControl::printStartingNotice()
{
    emit appendMessage(this, tr("Starting application..."), false);
}

void S60DeviceRunControl::printRunNotice(uint pid)
{
    emit appendMessage(this, tr("Application running with pid %1.").arg(pid), false);
}

void S60DeviceRunControl::printRunFailNotice(const QString &errorMessage) {
    emit appendMessage(this, tr("Could not start application: %1").arg(errorMessage), true);
}

// ======== S60DeviceDebugRunControl

S60DeviceDebugRunControl::S60DeviceDebugRunControl(S60DeviceRunConfiguration *runConfiguration) :
    S60DeviceRunControlBase(runConfiguration),
    m_startParams(new Debugger::DebuggerStartParameters)
{
    setReleaseDeviceAfterLauncherFinish(true); // Debugger controls device after install
    Debugger::DebuggerManager *dm = Debugger::DebuggerManager::instance();
    S60DeviceRunConfiguration *rc = qobject_cast<S60DeviceRunConfiguration *>(runConfiguration);
    QTC_ASSERT(dm && rc, return);

    connect(dm, SIGNAL(debuggingFinished()),
            this, SLOT(debuggingFinished()), Qt::QueuedConnection);
    connect(dm, SIGNAL(applicationOutputAvailable(QString, bool)),
            this, SLOT(printApplicationOutput(QString, bool)),
            Qt::QueuedConnection);

    m_startParams->remoteChannel = rc->serialPortName();
    m_startParams->processArgs = rc->commandLineArguments();
    m_startParams->startMode = Debugger::StartInternal;
    m_startParams->toolChainType = rc->toolChainType();

    m_localExecutableFileName = rc->localExecutableFileName();
    const int lastDotPos = m_localExecutableFileName.lastIndexOf(QLatin1Char('.'));
    if (lastDotPos != -1) {
        m_startParams->symbolFileName = m_localExecutableFileName.mid(0, lastDotPos) + QLatin1String(".sym");
    }
}

void S60DeviceDebugRunControl::stop()
{
    S60DeviceRunControlBase::stop();
    Debugger::DebuggerManager *dm = Debugger::DebuggerManager::instance();
    QTC_ASSERT(dm, return)
    if (dm->state() == Debugger::DebuggerNotReady)
        dm->exitDebugger();
}

S60DeviceDebugRunControl::~S60DeviceDebugRunControl()
{
}

void S60DeviceDebugRunControl::initLauncher(const QString &executable, trk::Launcher *launcher)
{
    // No setting an executable on the launcher causes it to deploy only
    m_startParams->executable = executable;
    // Prefer the '*.sym' file over the '.exe', which should exist at the same
    // location in debug builds

    if (!QFileInfo(m_startParams->symbolFileName).isFile()) {
        m_startParams->symbolFileName.clear();
        emit appendMessage(this, tr("Warning: Cannot locate the symbol file belonging to %1.").arg(m_localExecutableFileName), true);
    }

    launcher->addStartupActions(trk::Launcher::ActionCopyInstall);
    // Avoid close/open sequence in quick succession, which may cause crashs
    launcher->setCloseDevice(false);
}

void S60DeviceDebugRunControl::handleLauncherFinished()
{
    emit appendMessage(this, tr("Launching debugger..."), false);
    Debugger::DebuggerManager::instance()->startNewDebugger(m_startParams);
}

void S60DeviceDebugRunControl::debuggingFinished()
{
    emit appendMessage(this, tr("Debugging finished."), false);
    emit finished();
}

bool S60DeviceDebugRunControl::checkConfiguration(QString *errorMessage,
                                                  QString *settingsCategory,
                                                  QString *settingsPage) const
{
    return Debugger::DebuggerManager::instance()->checkDebugConfiguration(m_startParams->toolChainType,
                                                                          errorMessage,
                                                                          settingsCategory,
                                                                          settingsPage);
}
