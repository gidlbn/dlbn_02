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

#ifndef QMLJSCHECK_H
#define QMLJSCHECK_H

#include <qmljs/qmljsdocument.h>
#include <qmljs/qmljsinterpreter.h>
#include <qmljs/qmljslink.h>
#include <qmljs/qmljsscopebuilder.h>
#include <qmljs/parser/qmljsastvisitor_p.h>

#include <QtCore/QCoreApplication>

namespace QmlJS {

class QMLJS_EXPORT Check: protected AST::Visitor
{
    Q_DECLARE_TR_FUNCTIONS(QmlJS::Check)

public:
    Check(Document::Ptr doc, const Snapshot &snapshot, const QStringList &importPaths);
    virtual ~Check();

    QList<DiagnosticMessage> operator()();

protected:
    virtual bool visit(AST::UiProgram *ast);
    virtual bool visit(AST::UiObjectDefinition *ast);
    virtual bool visit(AST::UiObjectBinding *ast);
    virtual bool visit(AST::UiScriptBinding *ast);
    virtual bool visit(AST::UiArrayBinding *ast);

private:
    void visitQmlObject(AST::Node *ast, AST::UiQualifiedId *typeId,
                        AST::UiObjectInitializer *initializer);
    const Interpreter::Value *checkScopeObjectMember(const AST::UiQualifiedId *id);

    void warning(const AST::SourceLocation &loc, const QString &message);
    void error(const AST::SourceLocation &loc, const QString &message);
    static AST::SourceLocation locationFromRange(const AST::SourceLocation &start,
                                                 const AST::SourceLocation &end);

    Document::Ptr _doc;
    Snapshot _snapshot;

    Interpreter::Engine _engine;
    Interpreter::Context _context;
    Link _link;
    ScopeBuilder _scopeBuilder;

    QList<DiagnosticMessage> _messages;

    bool _ignoreTypeErrors;
};

} // namespace QmlJS

#endif // QMLJSCHECK_H
