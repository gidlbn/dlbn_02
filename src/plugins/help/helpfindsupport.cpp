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

#include "helpfindsupport.h"
#include "helpviewer.h"

#include <utils/qtcassert.h>

using namespace Help::Internal;

HelpFindSupport::HelpFindSupport(CentralWidget *centralWidget)
    : m_centralWidget(centralWidget)
{
}

HelpFindSupport::~HelpFindSupport()
{
}

bool HelpFindSupport::isEnabled() const
{
    return true;
}

Find::IFindSupport::FindFlags HelpFindSupport::supportedFindFlags() const
{
    return Find::IFindSupport::FindBackward | Find::IFindSupport::FindCaseSensitively
        | Find::IFindSupport::FindWholeWords;
}

QString HelpFindSupport::currentFindString() const
{
    QTC_ASSERT(m_centralWidget, return QString());
    HelpViewer *viewer = m_centralWidget->currentHelpViewer();
    if (!viewer)
        return QString();
    return viewer->selectedText();
}

QString HelpFindSupport::completedFindString() const
{
    return QString();
}

Find::IFindSupport::Result HelpFindSupport::findIncremental(const QString &txt,
    Find::IFindSupport::FindFlags findFlags)
{
    QTC_ASSERT(m_centralWidget, return NotFound);
    findFlags &= ~Find::IFindSupport::FindBackward;
    return m_centralWidget->find(txt, findFlags, true) ? Found : NotFound;
}

Find::IFindSupport::Result HelpFindSupport::findStep(const QString &txt,
    Find::IFindSupport::FindFlags findFlags)
{
    QTC_ASSERT(m_centralWidget, return NotFound);
    return m_centralWidget->find(txt, findFlags, false) ? Found : NotFound;
}

// -- HelpViewerFindSupport

HelpViewerFindSupport::HelpViewerFindSupport(HelpViewer *viewer)
    : m_viewer(viewer)
{
}

Find::IFindSupport::FindFlags HelpViewerFindSupport::supportedFindFlags() const
{
    return Find::IFindSupport::FindBackward | Find::IFindSupport::FindCaseSensitively
        | Find::IFindSupport::FindWholeWords;
}

QString HelpViewerFindSupport::currentFindString() const
{
    QTC_ASSERT(m_viewer, return QString());
    return m_viewer->selectedText();
}

Find::IFindSupport::Result HelpViewerFindSupport::findIncremental(const QString &txt,
    Find::IFindSupport::FindFlags findFlags)
{
    QTC_ASSERT(m_viewer, return NotFound);
    findFlags &= ~Find::IFindSupport::FindBackward;
    return find(txt, findFlags, true) ? Found : NotFound;
}

Find::IFindSupport::Result HelpViewerFindSupport::findStep(const QString &txt,
    Find::IFindSupport::FindFlags findFlags)
{
    QTC_ASSERT(m_viewer, return NotFound);
    return find(txt, findFlags, false) ? Found : NotFound;
}

bool HelpViewerFindSupport::find(const QString &txt,
    Find::IFindSupport::FindFlags findFlags, bool incremental)
{
    QTC_ASSERT(m_viewer, return false);
    return m_viewer->findText(txt, findFlags, incremental, false);
}
