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

#ifndef FILEICONPROVIDER_H
#define FILEICONPROVIDER_H

#include <coreplugin/core_global.h>

#include <QtGui/QStyle>
#include <QtGui/QFileIconProvider>

QT_BEGIN_NAMESPACE
class QFileInfo;
class QIcon;
class QPixmap;
class QString;
QT_END_NAMESPACE

namespace Core {

class MimeType;
struct FileIconProviderPrivate;

class CORE_EXPORT FileIconProvider : public QFileIconProvider
{
    Q_DISABLE_COPY(FileIconProvider)
    FileIconProvider();

public:
    virtual ~FileIconProvider();

    // Implement QFileIconProvider
    virtual QIcon icon(const QFileInfo &info) const;
    using QFileIconProvider::icon;

    // Register additional overlay icons
    static QPixmap overlayIcon(QStyle::StandardPixmap baseIcon, const QIcon &overlayIcon, const QSize &size);
    void registerIconOverlayForSuffix(const QIcon &icon, const QString &suffix);
    void registerIconOverlayForMimeType(const QIcon &icon, const MimeType &mimeType);

    static FileIconProvider *instance();

private:
    FileIconProviderPrivate *d;
};

} // namespace Core

#endif // FILEICONPROVIDER_H
