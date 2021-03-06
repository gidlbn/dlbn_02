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

#include "commonsettingspage.h"
#include "vcsbaseconstants.h"
#include "nicknamedialog.h"

#include "ui_commonsettingspage.h"

#include <coreplugin/icore.h>
#include <extensionsystem/pluginmanager.h>

#include <QtCore/QDebug>
#include <QtCore/QCoreApplication>
#include <QtGui/QMessageBox>

namespace VCSBase {
namespace Internal {

// ------------------ VCSBaseSettingsWidget

CommonSettingsWidget::CommonSettingsWidget(QWidget *parent) :
    QWidget(parent),
    m_ui(new Ui::CommonSettingsPage)
{
    m_ui->setupUi(this);
    m_ui->submitMessageCheckScriptChooser->setExpectedKind(Utils::PathChooser::Command);
    m_ui->nickNameFieldsFileChooser->setExpectedKind(Utils::PathChooser::File);
    m_ui->nickNameMailMapChooser->setExpectedKind(Utils::PathChooser::File);
}

CommonSettingsWidget::~CommonSettingsWidget()
{
    delete m_ui;
}

CommonVcsSettings CommonSettingsWidget::settings() const
{
    CommonVcsSettings rc;
    rc.nickNameMailMap = m_ui->nickNameMailMapChooser->path();
    rc.nickNameFieldListFile = m_ui->nickNameFieldsFileChooser->path();
    rc.submitMessageCheckScript = m_ui->submitMessageCheckScriptChooser->path();
    rc.lineWrap= m_ui->lineWrapCheckBox->isChecked();
    rc.lineWrapWidth = m_ui->lineWrapSpinBox->value();
    return rc;
}

void CommonSettingsWidget::setSettings(const CommonVcsSettings &s)
{
    m_ui->nickNameMailMapChooser->setPath(s.nickNameMailMap);
    m_ui->nickNameFieldsFileChooser->setPath(s.nickNameFieldListFile);
    m_ui->submitMessageCheckScriptChooser->setPath(s.submitMessageCheckScript);
    m_ui->lineWrapCheckBox->setChecked(s.lineWrap);
    m_ui->lineWrapSpinBox->setValue(s.lineWrapWidth);
}

QString CommonSettingsWidget::searchKeyWordMatchString() const
{
    const QChar blank = QLatin1Char(' ');
    QString rc = m_ui->submitMessageCheckScriptLabel->text();
    rc += blank;
    rc += m_ui->nickNameMailMapLabel->text();
    rc += blank;
    rc += m_ui->nickNameFieldsFileLabel->text();
    rc.remove(QLatin1Char('&')); // Strip buddy markers.
    return rc;
}

// --------------- VCSBaseSettingsPage
CommonOptionsPage::CommonOptionsPage(QObject *parent) :
    VCSBaseOptionsPage(parent)
{
    m_settings.fromSettings(Core::ICore::instance()->settings());
}

void CommonOptionsPage::updateNickNames()
{
}

CommonOptionsPage::~CommonOptionsPage()
{
}

QString CommonOptionsPage::id() const
{
    return QLatin1String(Constants::VCS_COMMON_SETTINGS_ID);
}

QString CommonOptionsPage::displayName() const
{
    return QCoreApplication::translate("VCSBase", Constants::VCS_COMMON_SETTINGS_NAME);
}

QWidget *CommonOptionsPage::createPage(QWidget *parent)
{
    m_widget = new CommonSettingsWidget(parent);
    m_widget->setSettings(m_settings);
    if (m_searchKeyWords.isEmpty())
        m_searchKeyWords = m_widget->searchKeyWordMatchString();
    return m_widget;
}

void CommonOptionsPage::apply()
{
    if (m_widget) {
        const CommonVcsSettings newSettings = m_widget->settings();
        if (newSettings != m_settings) {
            m_settings = newSettings;
            m_settings.toSettings(Core::ICore::instance()->settings());
            emit settingsChanged(m_settings);
        }
    }
}

bool CommonOptionsPage::matches(const QString &key) const
{
    return m_searchKeyWords.contains(key, Qt::CaseInsensitive);
}

} // namespace Internal
} // namespace VCSBase
