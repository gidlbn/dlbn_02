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

#ifndef QT4BUILDCONFIGURATION_H
#define QT4BUILDCONFIGURATION_H

#include "qtversionmanager.h"

#include <projectexplorer/buildconfiguration.h>
#include <projectexplorer/toolchain.h>

namespace Qt4ProjectManager {

class QMakeStep;
class MakeStep;

namespace Internal {
class Qt4ProFileNode;
class Qt4BuildConfigurationFactory;
class Qt4Target;

class Qt4BuildConfiguration : public ProjectExplorer::BuildConfiguration
{
    Q_OBJECT
    friend class Qt4BuildConfigurationFactory;

public:
    explicit Qt4BuildConfiguration(Qt4Target *target);
    virtual ~Qt4BuildConfiguration();

    Qt4Target *qt4Target() const;

    virtual ProjectExplorer::Environment baseEnvironment() const;

    virtual QString buildDirectory() const;
    bool shadowBuild() const;
    QString shadowBuildDirectory() const;
    void setShadowBuildAndDirectory(bool shadowBuild, const QString &buildDirectory);

    void setSubNodeBuild(Qt4ProjectManager::Internal::Qt4ProFileNode *node);
    Qt4ProjectManager::Internal::Qt4ProFileNode *subNodeBuild() const;

    // returns the qtVersion
    QtVersion *qtVersion() const;
    void setQtVersion(QtVersion *);

    ProjectExplorer::ToolChain *toolChain() const;
    void setToolChainType(ProjectExplorer::ToolChain::ToolChainType type);
    ProjectExplorer::ToolChain::ToolChainType toolChainType() const;

    QtVersion::QmakeBuildConfigs qmakeBuildConfiguration() const;
    void setQMakeBuildConfiguration(QtVersion::QmakeBuildConfigs config);
    // used by qmake step to notify that the qmake args have changed
    // not really nice, the build configuration should save the arguments
    // since they are needed for reevaluation
    void emitQMakeBuildConfigurationChanged();
    // used by qmake step to notify that the build directory was initialized
    // not really nice
    void emitBuildDirectoryInitialized();
    void getConfigCommandLineArguments(QStringList *addedUserConfigs, QStringList *removedUserConfigs) const;

    // Those functions are used in a few places.
    // The drawback is that we shouldn't actually depend on them being always there
    // That is generally the stuff that is asked should normally be transferred to
    // Qt4Project *
    // So that we can later enable people to build qt4projects the way they would like
    QMakeStep *qmakeStep() const;
    MakeStep *makeStep() const;

    QString makeCommand() const;
    QString defaultMakeTarget() const;

    bool compareToImportFrom(const QString &workingDirectory);
    static QStringList removeSpecFromArgumentList(const QStringList &old);
    static QString extractSpecFromArgumentList(const QStringList &list, QString directory, QtVersion *version);

    QVariantMap toMap() const;

signals:
    /// emitted if the qt version changes (either directly, or because the default qt version changed
    /// or because the user changed the settings for the qt version
    void qtVersionChanged();
    /// emitted iff the setToolChainType() function is called, not emitted for qtversion changes
    /// even if those result in a toolchain change
    void toolChainTypeChanged();
    /// emitted for setQMakeBuildConfig, not emitted for qt version changes, even
    /// if those change the qmakebuildconfig
    void qmakeBuildConfigurationChanged();

    /// emitted if the build configuration changed in a way that
    /// should trigger a reevaluation of all .pro files
    void proFileEvaluateNeeded(Qt4ProjectManager::Internal::Qt4BuildConfiguration *);

    void buildDirectoryInitialized();

private slots:
    void qtVersionsChanged(const QList<int> &changedVersions);

protected:
    Qt4BuildConfiguration(Qt4Target *target, Qt4BuildConfiguration *source);
    Qt4BuildConfiguration(Qt4Target *target, const QString &id);
    virtual bool fromMap(const QVariantMap &map);

private:
    void ctor();
    void pickValidQtVersion();

    bool m_shadowBuild;
    QString m_buildDirectory;
    int m_qtVersionId;
    int m_toolChainType;
    QtVersion::QmakeBuildConfigs m_qmakeBuildConfiguration;
    Qt4ProjectManager::Internal::Qt4ProFileNode *m_subNodeBuild;
};

class Qt4BuildConfigurationFactory : public ProjectExplorer::IBuildConfigurationFactory
{
    Q_OBJECT

public:
    explicit Qt4BuildConfigurationFactory(QObject *parent = 0);
    ~Qt4BuildConfigurationFactory();

    QStringList availableCreationIds(ProjectExplorer::Target *parent) const;
    QString displayNameForId(const QString &id) const;

    bool canCreate(ProjectExplorer::Target *parent, const QString &id) const;
    ProjectExplorer::BuildConfiguration *create(ProjectExplorer::Target *parent, const QString &id);
    bool canClone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *source) const;
    ProjectExplorer::BuildConfiguration *clone(ProjectExplorer::Target *parent, ProjectExplorer::BuildConfiguration *source);
    bool canRestore(ProjectExplorer::Target *parent, const QVariantMap &map) const;
    ProjectExplorer::BuildConfiguration *restore(ProjectExplorer::Target *parent, const QVariantMap &map);

private slots:
    void update();

private:
    struct VersionInfo {
        VersionInfo() {}
        VersionInfo(const QString &d, int v)
            : displayName(d), versionId(v) { }
        QString displayName;
        int versionId;
    };

    QMap<QString, VersionInfo> m_versions;
};

} // namespace Qt4ProjectManager
} // namespace Internal

#endif // QT4BUILDCONFIGURATION_H
