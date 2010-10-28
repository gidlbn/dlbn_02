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

#ifndef FIRSTDEFINITIONFINDER_H
#define FIRSTDEFINITIONFINDER_H

#include <qmljs/parser/qmljsastvisitor_p.h>
#include <qmljs/qmljsdocument.h>

namespace QmlDesigner {

class FirstDefinitionFinder: protected QmlJS::AST::Visitor
{
public:
    FirstDefinitionFinder(const QString &text);

    quint32 operator()(quint32 offset);

protected:
    using QmlJS::AST::Visitor::visit;

    virtual bool visit(QmlJS::AST::UiObjectBinding *ast);
    virtual bool visit(QmlJS::AST::UiObjectDefinition *ast);

    void extractFirstObjectDefinition(QmlJS::AST::UiObjectInitializer* ast);

private:
    QmlJS::Document::Ptr m_doc;
    quint32 m_offset;
    QmlJS::AST::UiObjectDefinition *m_firstObjectDefinition;

};

} // namespace QmlDesigner

#endif // FIRSTDEFINITIONFINDER_H
