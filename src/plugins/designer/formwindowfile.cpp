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

#include "formwindowfile.h"
#include "designerconstants.h"

#include <coreplugin/icore.h>
#include <utils/reloadpromptutils.h>
#include <utils/qtcassert.h>

#include <QtDesigner/QDesignerFormWindowInterface>
#include <QtDesigner/QDesignerFormWindowManagerInterface>
#include <QtDesigner/QDesignerFormEditorInterface>

#include <QtGui/QMessageBox>
#include <QtGui/QMainWindow>

#include <QtCore/QFile>
#include <QtCore/QFileInfo>
#include <QtCore/QByteArray>
#include <QtCore/QDebug>

namespace Designer {
namespace Internal {

FormWindowFile::FormWindowFile(QDesignerFormWindowInterface *form, QObject *parent)
  : Core::IFile(parent),
    m_mimeType(QLatin1String(Designer::Constants::FORM_MIMETYPE)),
    m_formWindow(form)
{
    connect(m_formWindow->core()->formWindowManager(), SIGNAL(formWindowRemoved(QDesignerFormWindowInterface*)),
            this, SLOT(slotFormWindowRemoved(QDesignerFormWindowInterface*)));
}

bool FormWindowFile::save(const QString &name /*= QString()*/)
{
    const QString actualName = name.isEmpty() ? fileName() : name;

    if (Designer::Constants::Internal::debug)
        qDebug() << Q_FUNC_INFO << name << "->" << actualName;

    QTC_ASSERT(m_formWindow, return false);

    if (actualName.isEmpty())
        return false;

    const QFileInfo fi(actualName);
    const QString oldFormName = m_formWindow->fileName();
    const QString formName = fi.absoluteFilePath();
    m_formWindow->setFileName(formName);

    QString errorString;
    if (!writeFile(actualName, errorString)) {
        QMessageBox::critical(0, tr("Error saving %1").arg(formName), errorString);
        m_formWindow->setFileName(oldFormName);
        return false;
    }

    m_fileName = fi.absoluteFilePath();
    emit setDisplayName(fi.fileName());
    m_formWindow->setDirty(false);
    emit changed();
    emit saved();

    return true;
}

QString FormWindowFile::fileName() const
{
    return m_fileName;
}

bool FormWindowFile::isModified() const
{
    return m_formWindow && m_formWindow->isDirty();
}

bool FormWindowFile::isReadOnly() const
{
    if (m_fileName.isEmpty())
        return false;
    const QFileInfo fi(m_fileName);
    return !fi.isWritable();
}

bool FormWindowFile::isSaveAsAllowed() const
{
    return true;
}

Core::IFile::ReloadBehavior FormWindowFile::reloadBehavior(ChangeTrigger state, ChangeType type) const
{
    if (type == TypePermissions)
        return BehaviorSilent;
    if (type == TypeContents) {
        if (state == TriggerInternal && !isModified())
            return BehaviorSilent;
        return BehaviorAsk;
    }
    return BehaviorAsk;
}

void FormWindowFile::reload(ReloadFlag flag, ChangeType type)
{
    if (flag == FlagIgnore)
        return;
    if (type == TypePermissions) {
        emit changed();
    } else {
        emit aboutToReload();
        emit reload(m_fileName);
        emit reloaded();
    }
}

QString FormWindowFile::defaultPath() const
{
    return QString();
}

void FormWindowFile::setSuggestedFileName(const QString &fileName)
{
    if (Designer::Constants::Internal::debug)
        qDebug() << Q_FUNC_INFO << m_fileName << fileName;

    m_suggestedName = fileName;
}

QString FormWindowFile::suggestedFileName() const
{
    return m_suggestedName;
}

QString FormWindowFile::mimeType() const
{
    return m_mimeType;
}

bool FormWindowFile::writeFile(const QString &fileName, QString &errorString) const
{
    if (Designer::Constants::Internal::debug)
        qDebug() << Q_FUNC_INFO << m_fileName << fileName;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate)) {
        errorString = tr("Unable to open %1: %2").arg(fileName, file.errorString());
        return false;
    }
    const bool rc = writeFile(file, errorString);
    file.close();
    return rc;
}

bool FormWindowFile::writeFile(QFile &file, QString &errorString) const
{
    const QByteArray content = m_formWindow->contents().toUtf8();
    if (!file.write(content)) {
        errorString = tr("Unable to write to %1: %2").arg(file.fileName(), file.errorString());
        return false;
    }
    return true;
}

void FormWindowFile::setFileName(const QString &fname)
{
    m_fileName = fname;
}

QDesignerFormWindowInterface *FormWindowFile::formWindow() const
{
    return m_formWindow;
}

void FormWindowFile::slotFormWindowRemoved(QDesignerFormWindowInterface *w)
{
    // Release formwindow as soon as the FormWindowManager removes
    // as calls to isDirty() are triggered at arbitrary times
    // while building.
    if (w == m_formWindow)
        m_formWindow = 0;
}

} // namespace Internal
} // namespace Designer
