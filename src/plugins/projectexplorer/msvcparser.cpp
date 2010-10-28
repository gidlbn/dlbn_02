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

#include "msvcparser.h"
#include "projectexplorerconstants.h"

using namespace ProjectExplorer;

MsvcParser::MsvcParser()
{
    m_compileRegExp.setPattern("^([^\\(]+)\\((\\d+)\\)+\\s:[^:\\d]+(\\d+):(.*)$");
    m_compileRegExp.setMinimal(true);
    m_linkRegExp.setPattern("^([^\\(]+)\\s:[^:\\d]+(\\d+):(.*)$");
    m_linkRegExp.setMinimal(true);
}

void MsvcParser::stdOutput(const QString &line)
{
    QString lne = line.trimmed();
    if (m_compileRegExp.indexIn(lne) > -1 && m_compileRegExp.numCaptures() == 4) {
        emit addTask(Task(toType(m_compileRegExp.cap(3).toInt()) /* task type */,
                          m_compileRegExp.cap(4) /* description */,
                          m_compileRegExp.cap(1) /* filename */,
                          m_compileRegExp.cap(2).toInt() /* linenumber */,
                          Constants::TASK_CATEGORY_COMPILE));
        return;
    }
    if (m_linkRegExp.indexIn(lne) > -1 && m_linkRegExp.numCaptures() == 3) {
        QString fileName = m_linkRegExp.cap(1);
        if (fileName.contains(QLatin1String("LINK"), Qt::CaseSensitive))
            fileName.clear();

        emit addTask(Task(toType(m_linkRegExp.cap(2).toInt()) /* task type */,
                          m_linkRegExp.cap(3) /* description */,
                          fileName /* filename */,
                          -1 /* line number */,
                          Constants::TASK_CATEGORY_COMPILE));
        return;
    }
    IOutputParser::stdError(line);
}

Task::TaskType MsvcParser::toType(int number)
{
    if (number == 0)
        return Task::Unknown;
    else if (number > 4000 && number < 5000)
        return Task::Warning;
    else
        return Task::Error;
}
