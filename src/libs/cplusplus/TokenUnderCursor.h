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
#ifndef TOKENUNDERCURSOR_H
#define TOKENUNDERCURSOR_H

#include "SimpleLexer.h"
#include <QList>

QT_BEGIN_NAMESPACE
class QString;
class QTextCursor;
class QTextBlock;
QT_END_NAMESPACE

namespace CPlusPlus {

class CPLUSPLUS_EXPORT TokenUnderCursor
{
public:
    TokenUnderCursor();
    ~TokenUnderCursor();

    SimpleToken operator()(const QTextCursor &cursor, QTextBlock *block = 0);

    const QList<SimpleToken> &tokens() const
    { return _tokens; }

private:
    QList<SimpleToken> _tokens;
    QString _text;
};

} // end of namespace CPlusPlus

#endif // TOKENUNDERCURSOR_H
