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

#include "qmlprojectimportwizard.h"

#include "qmlprojectconstants.h"

#include <coreplugin/icore.h>
#include <coreplugin/mimedatabase.h>
#include <projectexplorer/projectexplorerconstants.h>
#include <projectexplorer/customwizard/customwizard.h>

#include <utils/filenamevalidatinglineedit.h>
#include <utils/filewizardpage.h>

#include <QtGui/QIcon>

#include <QtGui/QApplication>
#include <QtGui/QStyle>
#include <QtGui/QPainter>
#include <QtGui/QPixmap>

#include <QtCore/QDir>
#include <QtCore/QtDebug>
#include <QtCore/QCoreApplication>

using namespace Utils;

namespace QmlProjectManager {
namespace Internal {

//////////////////////////////////////////////////////////////////////////////
// QmlProjectImportWizardDialog
//////////////////////////////////////////////////////////////////////////////

QmlProjectImportWizardDialog::QmlProjectImportWizardDialog(QWidget *parent)
    : Utils::Wizard(parent)
{
    setWindowTitle(tr("Import Existing QML Directory"));

    // first page
    m_firstPage = new FileWizardPage;
    m_firstPage->setTitle(tr("Project Name and Location"));
    m_firstPage->setFileNameLabel(tr("Project name:"));
    m_firstPage->setPathLabel(tr("Location:"));

    const int firstPageId = addPage(m_firstPage);
    wizardProgress()->item(firstPageId)->setTitle(tr("Location"));
}

QmlProjectImportWizardDialog::~QmlProjectImportWizardDialog()
{ }

QString QmlProjectImportWizardDialog::path() const
{
    return m_firstPage->path();
}

void QmlProjectImportWizardDialog::setPath(const QString &path)
{
    m_firstPage->setPath(path);
}

QString QmlProjectImportWizardDialog::projectName() const
{
    return m_firstPage->fileName();
}

//////////////////////////////////////////////////////////////////////////////
// QmlProjectImportWizard
//////////////////////////////////////////////////////////////////////////////

QmlProjectImportWizard::QmlProjectImportWizard()
    : Core::BaseFileWizard(parameters())
{ }

QmlProjectImportWizard::~QmlProjectImportWizard()
{ }

Core::BaseFileWizardParameters QmlProjectImportWizard::parameters()
{
    Core::BaseFileWizardParameters parameters(ProjectWizard);
    // TODO do something about the ugliness of standard icons in sizes different than 16, 32, 64, 128
    {
        QPixmap icon(22, 22);
        icon.fill(Qt::transparent);
        QPainter p(&icon);
        p.drawPixmap(3, 3, 16, 16, qApp->style()->standardIcon(QStyle::SP_DirIcon).pixmap(16));
        parameters.setIcon(icon);
    }
    parameters.setDisplayName(tr("Import Existing QML Directory"));
    parameters.setId(QLatin1String("QI.QML Import"));
    parameters.setDescription(tr("Creates a QML project from an existing directory of QML files."));
#ifdef QTCREATOR_WITH_QML
    parameters.setCategory(QLatin1String(Constants::QML_WIZARD_CATEGORY));
#else
    parameters.setCategory(QLatin1String(ProjectExplorer::Constants::PROJECT_WIZARD_CATEGORY));
#endif
    parameters.setDisplayCategory(QCoreApplication::translate(Constants::QML_WIZARD_TR_SCOPE,
                                                              Constants::QML_WIZARD_TR_CATEGORY));
    return parameters;
}

QWizard *QmlProjectImportWizard::createWizardDialog(QWidget *parent,
                                                    const QString &defaultPath,
                                                    const WizardPageList &extensionPages) const
{
    QmlProjectImportWizardDialog *wizard = new QmlProjectImportWizardDialog(parent);
    setupWizard(wizard);

    wizard->setPath(defaultPath);

    foreach (QWizardPage *p, extensionPages)
        BaseFileWizard::applyExtensionPageShortTitle(wizard, wizard->addPage(p));

    return wizard;
}

Core::GeneratedFiles QmlProjectImportWizard::generateFiles(const QWizard *w,
                                                     QString *errorMessage) const
{
    Q_UNUSED(errorMessage)

    const QmlProjectImportWizardDialog *wizard = qobject_cast<const QmlProjectImportWizardDialog *>(w);
    const QString projectPath = wizard->path();
    const QDir dir(projectPath);
    const QString projectName = wizard->projectName();
    const QString creatorFileName = QFileInfo(dir, projectName + QLatin1String(".qmlproject")).absoluteFilePath();

    QString projectContents;
    {
        QTextStream out(&projectContents);

        out
            //: Comment added to generated .qmlproject file
            << "/* " << tr("File generated by QtCreator", "qmlproject Template") << " */" << endl
            << endl
            << "import QmlProject 1.0" << endl
            << endl
            << "Project {" << endl
            //: Comment added to generated .qmlproject file
            << "    /* " << tr("Include .qml, .js, and image files from current directory and subdirectories", "qmlproject Template") << " */" << endl
            << "    QmlFiles {" << endl
            << "        directory: \".\"" << endl
            << "    }" << endl
            << "    JavaScriptFiles {" << endl
            << "        directory: \".\"" << endl
            << "    }" << endl
            << "    ImageFiles {" << endl
            << "        directory: \".\"" << endl
            << "    }" << endl
            //: Comment added to generated .qmlproject file
            << "    /* " << tr("List of plugin directories passed to QML runtime", "qmlproject Template") << " */" << endl
            << "    // importPaths: [ \"../exampleplugin\" ]" << endl
            << "}" << endl;
    }
    Core::GeneratedFile generatedCreatorFile(creatorFileName);
    generatedCreatorFile.setContents(projectContents);
    generatedCreatorFile.setAttributes(Core::GeneratedFile::OpenProjectAttribute);

    Core::GeneratedFiles files;
    files.append(generatedCreatorFile);

    return files;
}

bool QmlProjectImportWizard::postGenerateFiles(const QWizard *, const Core::GeneratedFiles &l, QString *errorMessage)
{
    return ProjectExplorer::CustomProjectWizard::postGenerateOpen(l ,errorMessage);
}

} // namespace Internal
} // namespace QmlProjectManager
