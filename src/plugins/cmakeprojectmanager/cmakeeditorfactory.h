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

#ifndef CMAKEEDITORFACTORY_H
#define CMAKEEDITORFACTORY_H

#include <coreplugin/editormanager/ieditorfactory.h>
#include "cmakeprojectmanager.h"

#include <QtCore/QStringList>

QT_BEGIN_NAMESPACE
class QAction;
QT_END_NAMESPACE

namespace TextEditor {
class TextEditorActionHandler;
}

namespace CMakeProjectManager {

namespace Internal {

class CMakeEditorFactory : public Core::IEditorFactory
{
    Q_OBJECT

public:
    CMakeEditorFactory(CMakeManager *parent, TextEditor::TextEditorActionHandler *handler);
    ~CMakeEditorFactory();

    virtual QStringList mimeTypes() const;
    virtual QString id() const;
    virtual QString displayName() const;
    Core::IFile *open(const QString &fileName);
    Core::IEditor *createEditor(QWidget *parent);

private:
    const QStringList m_mimeTypes;
    CMakeManager *m_manager;
    TextEditor::TextEditorActionHandler *m_actionHandler;
};

} // namespace Internal
} // namespace CMakeProjectManager

#endif // CMAKEEDITORFACTORY_H
