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

#ifndef PROFILEREADER_H
#define PROFILEREADER_H

#include "profileevaluator.h"

#include <QtCore/QObject>
#include <QtCore/QMap>
#include <QtCore/QTimer>

namespace Qt4ProjectManager {
namespace Internal {

class ProFileReader : public QObject, public ProFileEvaluator
{
    Q_OBJECT

public:
    ProFileReader(ProFileOption *option);
    ~ProFileReader();

    // fileName is expected to be absolute and cleanPath()ed.
    bool readProFile(const QString &fileName);
    QList<ProFile*> includeFiles() const;

    QString value(const QString &variable) const;

    ProFile *proFileFor(const QString &name);
signals:
    void errorFound(const QString &error);

private:
    virtual void aboutToEval(ProFile *parent, ProFile *proFile, EvalFileType type);
    virtual void doneWithEval(ProFile *parent);
    virtual void logMessage(const QString &msg);
    virtual void fileMessage(const QString &msg);
    virtual void errorMessage(const QString &msg);

private:
    QMap<QString, ProFile *> m_includeFiles;
    QList<ProFile *> m_proFiles;
    int m_ignoreLevel;
};

class ProFileCacheManager : public QObject
{
    Q_OBJECT

public:
    static ProFileCacheManager *instance() { return s_instance; }
    ProFileCache *cache();
    void discardFiles(const QString &prefix);
    void discardFile(const QString &fileName);
    void incRefCount();
    void decRefCount();

private:
    ProFileCacheManager(QObject *parent);
    ~ProFileCacheManager();
    Q_SLOT void clear();
    ProFileCache *m_cache;
    int m_refCount;
    QTimer m_timer;

    static ProFileCacheManager *s_instance;

    friend class Qt4ProjectManagerPlugin;
};

} // namespace Internal
} // namespace Qt4ProjectManager

#endif // PROFILEREADER_H
