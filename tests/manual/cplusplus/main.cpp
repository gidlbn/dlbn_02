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

#include <AST.h>
#include <ASTVisitor.h>
#include <ASTPatternBuilder.h>
#include <ASTMatcher.h>
#include <Control.h>
#include <Scope.h>
#include <Semantic.h>
#include <TranslationUnit.h>
#include <Literals.h>
#include <Symbols.h>
#include <Names.h>
#include <CoreTypes.h>
#include <CppDocument.h>

#include <QFile>
#include <QList>
#include <QCoreApplication>
#include <QStringList>
#include <QFileInfo>
#include <QTime>
#include <QtDebug>

#include <cstdio>
#include <cstdlib>
#include <iostream>

using namespace CPlusPlus;

class ForEachNode: protected ASTVisitor
{
    Document::Ptr doc;
    AST *pattern;

public:
    ForEachNode(Document::Ptr doc)
        : ASTVisitor(doc->translationUnit()),
          matcher() {}

    void operator()() { accept(doc->translationUnit()->ast()); }

protected:
    using ASTVisitor::visit;

    virtual bool preVisit(AST *ast)
    {
        ir.reset();
        IfStatementAST *pattern = ir.IfStatement(ir.SimpleName());

        //CompoundStatementAST *pattern = ir.CompoundStatement();

        if (ast->match(ast, pattern, &matcher))
            translationUnit()->warning(ast->firstToken(), "matched");

        return true;
    }


    ASTPatternBuilder ir;
    ASTMatcher matcher;
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    QStringList files = app.arguments();
    files.removeFirst();

    foreach (const QString &fileName, files) {
        QFile file(fileName);
        if (! file.open(QFile::ReadOnly))
            continue;

        const QByteArray source = file.readAll();
        file.close();

        Document::Ptr doc = Document::create(fileName);
        doc->control()->setDiagnosticClient(0);
        doc->setSource(source);
        doc->parse();

        ForEachNode forEachNode(doc);
        forEachNode();
    }

    return EXIT_SUCCESS;
}
