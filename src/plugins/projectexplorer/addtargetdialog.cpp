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

#include "addtargetdialog.h"

#include "ui_addtargetdialog.h"

#include "project.h"

using namespace ProjectExplorer;
using namespace ProjectExplorer::Internal;

AddTargetDialog::AddTargetDialog(Project *project, QWidget *parent) :
    QDialog(parent),
    m_project(project),
    ui(new Ui::AddTargetDialog)
{
    ui->setupUi(this);

    foreach (const QString &id, m_project->possibleTargetIds()) {
        for (int i = 0; i <= ui->targetComboBox->count(); ++i) {
            const QString displayName = m_project->targetFactory()->displayNameForId(id);
            if (i == ui->targetComboBox->count() ||
                ui->targetComboBox->itemText(i) > displayName) {
                ui->targetComboBox->insertItem(i, displayName, id);
                break;
            }
        }
    }
    ui->targetComboBox->setCurrentIndex(0);

    connect(ui->buttonBox, SIGNAL(accepted()),
            this, SLOT(accept()));
}

AddTargetDialog::~AddTargetDialog()
{
    delete ui;
}

void AddTargetDialog::accept()
{
    int index = ui->targetComboBox->currentIndex();
    QString id(ui->targetComboBox->itemData(index).toString());
    Target *target(m_project->targetFactory()->create(m_project, id));
    if (!target)
        return;
    m_project->addTarget(target);

    done(QDialog::Accepted);
}

