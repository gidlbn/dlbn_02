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

#ifndef BASEPROJECTWIZARDDIALOG_H
#define BASEPROJECTWIZARDDIALOG_H

#include "projectexplorer_export.h"
#include <utils/wizard.h>

#include <QtGui/QWizard>

namespace Utils {
    class ProjectIntroPage;
}

namespace ProjectExplorer {

struct BaseProjectWizardDialogPrivate;

/* BaseProjectWizardDialog: Presents the introductory
 * page and takes care of setting the directory as default
 * should the user wish to do that. */

class PROJECTEXPLORER_EXPORT BaseProjectWizardDialog : public Utils::Wizard
{
    Q_OBJECT

protected:
    explicit BaseProjectWizardDialog(Utils::ProjectIntroPage *introPage,
                                     int introId = -1,
                                     QWidget *parent = 0);

public:
    explicit BaseProjectWizardDialog(QWidget *parent = 0);

    virtual ~BaseProjectWizardDialog();

    QString projectName() const;
    QString path() const;

    // Generate a new project name (untitled<n>) in path.
    static QString uniqueProjectName(const QString &path);

public slots:
    void setIntroDescription(const QString &d);
    void setPath(const QString &path);
    void setProjectName(const QString &name);

signals:
    void introPageLeft(const QString &projectName, const QString &path);

protected:
    Utils::ProjectIntroPage *introPage() const;

private slots:
    void slotAccepted();
    void slotBaseCurrentIdChanged(int);

private:
    void init();

    BaseProjectWizardDialogPrivate *d;
};

} // namespace ProjectExplorer

#endif // BASEPROJECTWIZARDDIALOG_H
