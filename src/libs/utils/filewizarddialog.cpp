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

#include "filewizarddialog.h"
#include "filewizardpage.h"

#include <QtGui/QAbstractButton>

namespace Utils {

FileWizardDialog::FileWizardDialog(QWidget *parent) :
    Wizard(parent),
    m_filePage(new FileWizardPage)
{
    setWindowFlags(windowFlags() & ~Qt::WindowContextHelpButtonHint);
    setOption(QWizard::NoCancelButton, false);
    setOption(QWizard::NoDefaultButton, false);
    const int filePageId = addPage(m_filePage);
    wizardProgress()->item(filePageId)->setTitle(tr("Location"));
    connect(m_filePage, SIGNAL(activated()), button(QWizard::FinishButton), SLOT(animateClick()));
}

QString FileWizardDialog::fileName() const
{
    return m_filePage->fileName();
}

QString FileWizardDialog::path() const
{
    return m_filePage->path();
}

void FileWizardDialog::setPath(const QString &path)
{
    m_filePage->setPath(path);

}

void FileWizardDialog::setFileName(const QString &name)
{
    m_filePage->setFileName(name);
}

} // namespace Utils
