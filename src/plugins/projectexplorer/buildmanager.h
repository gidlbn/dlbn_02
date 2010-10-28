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

#ifndef BUILDMANAGER_H
#define BUILDMANAGER_H

#include "projectexplorer_export.h"
#include "taskwindow.h"

#include <QtCore/QObject>
#include <QtCore/QStringList>
#include <QtCore/QList>
#include <QtCore/QHash>
#include <QtCore/QFutureWatcher>

namespace ProjectExplorer {

namespace Internal {
    class CompileOutputWindow;
    class BuildProgressFuture;
}

class BuildStep;
class Project;
class ProjectExplorerPlugin;
class BuildConfiguration;

class PROJECTEXPLORER_EXPORT BuildManager
  : public QObject
{
    Q_OBJECT

    //NBS TODO this class has too many different variables which hold state:
    // m_buildQueue, m_running, m_canceled, m_progress, m_maxProgress, m_activeBuildSteps and ...
    // I might need to reduce that

public:
    BuildManager(ProjectExplorerPlugin *parent);
    ~BuildManager();

    bool isBuilding() const;

    bool tasksAvailable() const;
    //shows with focus
    void gotoTaskWindow();

    void buildProject(BuildConfiguration *bc);
    void buildProjects(const QList<BuildConfiguration *> &configurations);
    void cleanProject(BuildConfiguration *configuration);
    void cleanProjects(const QList<BuildConfiguration *> &configurations);
    bool isBuilding(Project *p);
    bool isBuilding(BuildStep *step);

    // Append any build step to the list of build steps (currently only used to add the QMakeStep)
    void appendStep(BuildStep *step);

public slots:
    void cancel();
    // Shows without focus
    void showTaskWindow();
    void toggleTaskWindow();
    void toggleOutputWindow();
    void aboutToRemoveProject(ProjectExplorer::Project *p);

signals:
    void buildStateChanged(ProjectExplorer::Project *pro);
    void buildQueueFinished(bool success);
    void tasksChanged();
    void taskAdded(const ProjectExplorer::Task &task);
    void tasksCleared();

private slots:
    void addToTaskWindow(const ProjectExplorer::Task &task);
    void addToOutputWindow(const QString &string, const QTextCharFormat &textCharFormat);

    void nextBuildQueue();
    void progressChanged();
    void emitCancelMessage();
    void showBuildResults();
    void updateTaskCount();
    void finish();

private:
    void startBuildQueue();
    void nextStep();
    void clearBuildQueue();
    bool buildQueueAppend(QList<BuildStep *> steps);
    void incrementActiveBuildSteps(Project *pro);
    void decrementActiveBuildSteps(Project *pro);

    Internal::CompileOutputWindow *m_outputWindow;
    TaskWindow *m_taskWindow;

    QList<BuildStep *> m_buildQueue;
    QStringList m_configurations; // the corresponding configuration to the m_buildQueue
    ProjectExplorerPlugin *m_projectExplorerPlugin;
    bool m_running;
    QFutureWatcher<bool> m_watcher;
    BuildStep *m_currentBuildStep;
    QString m_currentConfiguration;
    // used to decide if we are building a project to decide when to emit buildStateChanged(Project *)
    QHash<Project *, int>  m_activeBuildSteps;
    Project *m_previousBuildStepProject;
    // is set to true while canceling, so that nextBuildStep knows that the BuildStep finished because of canceling
    bool m_canceling;

    // Progress reporting to the progress manager
    int m_progress;
    int m_maxProgress;
    QFutureInterface<void> *m_progressFutureInterface;
    QFutureWatcher<void> m_progressWatcher;
};

} // namespace ProjectExplorer

#endif // BUILDMANAGER_H
