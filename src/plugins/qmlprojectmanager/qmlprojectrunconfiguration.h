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

#ifndef QMLPROJECTRUNCONFIGURATION_H
#define QMLPROJECTRUNCONFIGURATION_H

#include "qmlprojectmanager_global.h"
#include <projectexplorer/runconfiguration.h>
#include <QWeakPointer>
#include <QComboBox>
#include <QLabel>

QT_FORWARD_DECLARE_CLASS(QStringListModel);

namespace Core {
    class IEditor;
}

namespace QmlProjectManager {

namespace Internal {
class QmlProjectTarget;
class QmlProjectRunConfigurationFactory;

}

const char * const CURRENT_FILE  = QT_TRANSLATE_NOOP("QmlManager", "<Current File>");
const char * const M_CURRENT_FILE  = "CurrentFile";


class QMLPROJECTMANAGER_EXPORT QmlProjectRunConfigurationDebugData {
public:
    QmlProjectRunConfigurationDebugData();
    QString serverAddress;
    quint16 serverPort;
};

class QMLPROJECTMANAGER_EXPORT QmlProjectRunConfiguration : public ProjectExplorer::RunConfiguration
{
    Q_OBJECT
    friend class Internal::QmlProjectRunConfigurationFactory;

public:
    QmlProjectRunConfiguration(Internal::QmlProjectTarget *parent);
    virtual ~QmlProjectRunConfiguration();

    Internal::QmlProjectTarget *qmlTarget() const;

    bool isEnabled(ProjectExplorer::BuildConfiguration *bc) const;

    QString viewerPath() const;
    QStringList viewerArguments() const;
    QString workingDirectory() const;
    QString debugServerAddress() const;
    uint debugServerPort() const;

    // RunConfiguration
    virtual QWidget *createConfigurationWidget();

    QVariantMap toMap() const;

public slots:
    void changeCurrentFile(Core::IEditor*);

private slots:
    QString mainScript() const;
    void setMainScript(const QString &scriptFile);
    void updateFileComboBox();

    void updateEnabled();

    void onViewerChanged();
    void onViewerArgsChanged();
    void onDebugServerAddressChanged();
    void onDebugServerPortChanged();

protected:
    QString viewerDefaultPath() const;
    QmlProjectRunConfiguration(Internal::QmlProjectTarget *parent, QmlProjectRunConfiguration *source);
    virtual bool fromMap(const QVariantMap &map);
    void setEnabled(bool value);

private:
    void ctor();

    // absolute path to current file (if being used)
    QString m_currentFileFilename;
    // absolute path to selected main script (if being used)
    QString m_mainScriptFilename;

    QString m_scriptFile;
    QString m_qmlViewerCustomPath;
    QString m_qmlViewerArgs;
    QmlProjectRunConfigurationDebugData m_debugData;

    QStringListModel *m_fileListModel;
    // weakpointer is used to make sure we don't try to manipulate
    // widget which was deleted already, as can be the case here.
    QWeakPointer<QLabel> m_qmlViewerExecutable;
    QWeakPointer<QComboBox> m_fileListCombo;

    Internal::QmlProjectTarget *m_projectTarget;

    bool m_usingCurrentFile;
    bool m_isEnabled;

};

} // namespace QmlProjectManager

#endif // QMLPROJECTRUNCONFIGURATION_H
