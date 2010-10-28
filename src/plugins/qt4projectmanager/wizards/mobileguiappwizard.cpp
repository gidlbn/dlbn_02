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

#include "mobileguiappwizard.h"

#include "qt4projectmanagerconstants.h"

#include <QtGui/QIcon>

namespace Qt4ProjectManager {
namespace Internal {

MobileGuiAppWizard::MobileGuiAppWizard() :
    GuiAppWizard(QLatin1String("C.Qt4GuiMobile"),
                 QLatin1String(Constants::QT_APP_WIZARD_CATEGORY),
                 QLatin1String(Constants::QT_APP_WIZARD_TR_SCOPE),
                 QLatin1String(Constants::QT_APP_WIZARD_TR_CATEGORY),
                 tr("Mobile Qt Application"),
                 tr("Creates a Qt application optimized for mobile devices "
                    "with a Qt Designer-based main window.\n\n"
                    "Preselects Qt for Simulator and mobile targets if available"),
                 QIcon(QLatin1String(":/projectexplorer/images/SymbianDevice.png")),
                 true)
{
}

} // namespace Internal
} // namespace Qt4ProjectManager
