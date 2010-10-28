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

#ifndef RELOADPROMPTUTILS_H
#define RELOADPROMPTUTILS_H

#include "utils_global.h"

QT_BEGIN_NAMESPACE
class QString;
class QWidget;
QT_END_NAMESPACE

namespace Utils {

enum ReloadPromptAnswer { ReloadCurrent, ReloadAll, ReloadSkipCurrent, ReloadNone };

QTCREATOR_UTILS_EXPORT ReloadPromptAnswer reloadPrompt(const QString &fileName, bool modified, QWidget *parent);
QTCREATOR_UTILS_EXPORT ReloadPromptAnswer reloadPrompt(const QString &title, const QString &prompt, const QString &details, QWidget *parent);

enum FileDeletedPromptAnswer { FileDeletedClose, FileDeletedSaveAs, FileDeletedSave };

QTCREATOR_UTILS_EXPORT FileDeletedPromptAnswer fileDeletedPrompt(const QString &fileName, QWidget *parent);

} // namespace Utils

#endif // RELOADPROMPTUTILS_H