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

#ifndef FORMWINDOWFILE_H
#define FORMWINDOWFILE_H

#include <coreplugin/ifile.h>

#include <QtCore/QPointer>

QT_BEGIN_NAMESPACE
class QDesignerFormWindowInterface;
class QFile;
QT_END_NAMESPACE

namespace Designer {
namespace Internal {

class FormWindowFile : public Core::IFile
{
    Q_OBJECT

public:
    explicit FormWindowFile(QDesignerFormWindowInterface *form, QObject *parent = 0);

    // IFile
    virtual bool save(const QString &fileName = QString());
    virtual QString fileName() const;
    virtual bool isModified() const;
    virtual bool isReadOnly() const;
    virtual bool isSaveAsAllowed() const;
    ReloadBehavior reloadBehavior(ChangeTrigger state, ChangeType type) const;
    void reload(ReloadFlag flag, ChangeType type);
    virtual QString defaultPath() const;
    virtual QString suggestedFileName() const;
    virtual QString mimeType() const;

    // Internal
    void setSuggestedFileName(const QString &fileName);

    bool writeFile(const QString &fileName, QString &errorString) const;
    bool writeFile(QFile &file, QString &errorString) const;

    QDesignerFormWindowInterface *formWindow() const;

signals:
    // Internal
    void saved();
    void reload(const QString &);
    void setDisplayName(const QString &);

public slots:
    void setFileName(const QString &);

private slots:
    void slotFormWindowRemoved(QDesignerFormWindowInterface *w);

private:
    const QString m_mimeType;

    QString m_fileName;
    QString m_suggestedName;
    // Might actually go out of scope before the IEditor due
    // to deleting the WidgetHost which owns it.
    QPointer<QDesignerFormWindowInterface> m_formWindow;
};

} // namespace Internal
} // namespace Designer

#endif // FORMWINDOWFILE_H
