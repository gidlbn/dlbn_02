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

#include "cpphoverhandler.h"
#include "cppeditor.h"
#include "cppplugin.h"

#include <coreplugin/icore.h>
#include <coreplugin/helpmanager.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/editormanager/editormanager.h>
#include <cpptools/cppmodelmanagerinterface.h>
#include <extensionsystem/pluginmanager.h>
#include <texteditor/itexteditor.h>
#include <texteditor/basetexteditor.h>
#include <debugger/debuggerconstants.h>

#include <CoreTypes.h>
#include <FullySpecifiedType.h>
#include <Literals.h>
#include <Control.h>
#include <Names.h>
#include <Scope.h>
#include <Symbol.h>
#include <Symbols.h>
#include <cplusplus/ExpressionUnderCursor.h>
#include <cplusplus/Overview.h>
#include <cplusplus/TypeOfExpression.h>
#include <cplusplus/SimpleLexer.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtGui/QToolTip>
#include <QtGui/QTextCursor>
#include <QtGui/QTextBlock>
#include <QtGui/QApplication>
#include <QtGui/QDesktopWidget>

using namespace CppEditor::Internal;
using namespace CPlusPlus;
using namespace Core;

CppHoverHandler::CppHoverHandler(QObject *parent)
    : QObject(parent)
{
    m_modelManager = ExtensionSystem::PluginManager::instance()->getObject<CppTools::CppModelManagerInterface>();

    // Listen for editor opened events in order to connect to tooltip/helpid requests
    connect(ICore::instance()->editorManager(), SIGNAL(editorOpened(Core::IEditor *)),
            this, SLOT(editorOpened(Core::IEditor *)));
}

void CppHoverHandler::updateContextHelpId(TextEditor::ITextEditor *editor, int pos)
{
    updateHelpIdAndTooltip(editor, pos);
}

void CppHoverHandler::editorOpened(IEditor *editor)
{
    CPPEditorEditable *cppEditor = qobject_cast<CPPEditorEditable *>(editor);
    if (!cppEditor)
        return;

    connect(cppEditor, SIGNAL(tooltipRequested(TextEditor::ITextEditor*, QPoint, int)),
            this, SLOT(showToolTip(TextEditor::ITextEditor*, QPoint, int)));

    connect(cppEditor, SIGNAL(contextHelpIdRequested(TextEditor::ITextEditor*, int)),
            this, SLOT(updateContextHelpId(TextEditor::ITextEditor*, int)));
}

void CppHoverHandler::showToolTip(TextEditor::ITextEditor *editor, const QPoint &point, int pos)
{
    if (!editor)
        return;

    ICore *core = ICore::instance();
    const int dbgcontext = core->uniqueIDManager()->uniqueIdentifier(Debugger::Constants::C_DEBUGMODE);

    if (core->hasContext(dbgcontext))
        return;

    updateHelpIdAndTooltip(editor, pos, QApplication::desktop()->screenNumber(point));

    if (m_toolTip.isEmpty())
        QToolTip::hideText();
    else {
        const QPoint pnt = point - QPoint(0,
#ifdef Q_WS_WIN
        24
#else
        16
#endif
        );

        QToolTip::showText(pnt, m_toolTip);
    }
}

static QString buildHelpId(Symbol *symbol, const Name *name)
{
    Scope *scope = 0;

    if (symbol) {
        scope = symbol->scope();
        name = symbol->name();
    }

    if (! name)
        return QString();

    Overview overview;
    overview.setShowArgumentNames(false);
    overview.setShowReturnTypes(false);

    QStringList qualifiedNames;
    qualifiedNames.prepend(overview.prettyName(name));

    for (; scope; scope = scope->enclosingScope()) {
        Symbol *owner = scope->owner();

        if (owner && owner->name() && ! scope->isEnumScope()) {
            const Name *name = owner->name();
            const Identifier *id = 0;

            if (const NameId *nameId = name->asNameId())
                id = nameId->identifier();

            else if (const TemplateNameId *nameId = name->asTemplateNameId())
                id = nameId->identifier();

            if (id)
                qualifiedNames.prepend(QString::fromLatin1(id->chars(), id->size()));
        }
    }

    return qualifiedNames.join(QLatin1String("::"));
}

// ### move me
static FullySpecifiedType resolve(const FullySpecifiedType &ty,
                                  const LookupContext &context,
                                  Symbol **resolvedSymbol,
                                  const Name **resolvedName)
{
    Control *control = context.control();

    if (const PointerType *ptrTy = ty->asPointerType()) {
        return control->pointerType(resolve(ptrTy->elementType(), context,
                                            resolvedSymbol, resolvedName));

    } else if (const ReferenceType *refTy = ty->asReferenceType()) {
        return control->referenceType(resolve(refTy->elementType(), context,
                                              resolvedSymbol, resolvedName));

    } else if (const PointerToMemberType *ptrToMemTy = ty->asPointerToMemberType()) {
        return control->pointerToMemberType(ptrToMemTy->memberName(),
                                            resolve(ptrToMemTy->elementType(), context,
                                                    resolvedSymbol, resolvedName));

    } else if (const NamedType *namedTy = ty->asNamedType()) {
        if (resolvedName)
            *resolvedName = namedTy->name();

        const QList<Symbol *> candidates = context.resolve(namedTy->name());

        foreach (Symbol *c, candidates) {
            if (c->isClass() || c->isEnum()) {
                if (resolvedSymbol)
                    *resolvedSymbol = c;

                return c->type();
            }
        }

    } else if (const Namespace *nsTy = ty->asNamespaceType()) {
        if (resolvedName)
            *resolvedName = nsTy->name();

        if (resolvedSymbol)
            *resolvedSymbol = const_cast<Namespace *>(nsTy);

    } else if (const Class *classTy = ty->asClassType()) {
        if (resolvedName)
            *resolvedName = classTy->name();

        if (resolvedSymbol)
            *resolvedSymbol = const_cast<Class *>(classTy);

    } else if (const ForwardClassDeclaration *fwdClassTy = ty->asForwardClassDeclarationType()) {
        if (resolvedName)
            *resolvedName = fwdClassTy->name();

        if (resolvedSymbol)
            *resolvedSymbol = const_cast<ForwardClassDeclaration *>(fwdClassTy);

    } else if (const Enum *enumTy = ty->asEnumType()) {
        if (resolvedName)
            *resolvedName = enumTy->name();

        if (resolvedSymbol)
            *resolvedSymbol = const_cast<Enum *>(enumTy);

    } else if (const Function *funTy = ty->asFunctionType()) {
        if (resolvedName)
            *resolvedName = funTy->name();

        if (resolvedSymbol)
            *resolvedSymbol = const_cast<Function *>(funTy);

    }

    return ty;
}

void CppHoverHandler::updateHelpIdAndTooltip(TextEditor::ITextEditor *editor,
                                             int pos,
                                             const int screen)
{
    m_helpId.clear();
    m_toolTip.clear();

    if (!m_modelManager)
        return;

    TextEditor::BaseTextEditor *edit = qobject_cast<TextEditor::BaseTextEditor *>(editor->widget());
    if (!edit)
        return;

    const Snapshot documents = m_modelManager->snapshot();
    const QString fileName = editor->file()->fileName();
    Document::Ptr doc = documents.document(fileName);
    if (!doc)
        return; // nothing to do

    QString formatTooltip = edit->extraSelectionTooltip(pos);
    QTextCursor tc(edit->document());
    tc.setPosition(pos);

    const unsigned lineNumber = tc.block().blockNumber() + 1;

    // Find the last symbol up to the cursor position
    int line = 0, column = 0;
    editor->convertPosition(tc.position(), &line, &column);
    Symbol *lastSymbol = doc->findSymbolAt(line, column);

    TypeOfExpression typeOfExpression;
    typeOfExpression.setSnapshot(documents);

    // We only want to show F1 if the tooltip matches the help id
    bool showF1 = true;

    foreach (const Document::DiagnosticMessage &m, doc->diagnosticMessages()) {
        if (m.line() == lineNumber) {
            m_toolTip = m.text();
            showF1 = false;
            break;
        }
    }

    QMap<QString, QUrl> helpLinks;
    if (m_toolTip.isEmpty()) {
        foreach (const Document::Include &incl, doc->includes()) {
            if (incl.line() == lineNumber) {
                m_toolTip = QDir::toNativeSeparators(incl.fileName());
                m_helpId = QFileInfo(incl.fileName()).fileName();
                helpLinks = Core::HelpManager::instance()->linksForIdentifier(m_helpId);
                break;
            }
        }
    }

    if (m_helpId.isEmpty()) {
        // Move to the end of a qualified name
        bool stop = false;
        while (!stop) {
            const QChar ch = editor->characterAt(tc.position());
            if (ch.isLetterOrNumber() || ch == QLatin1Char('_'))
                tc.setPosition(tc.position() + 1);
            else if (ch == QLatin1Char(':') && editor->characterAt(tc.position() + 1) == QLatin1Char(':')) {
                tc.setPosition(tc.position() + 2);
            } else {
                stop = true;
            }
        }

        // Fetch the expression's code
        ExpressionUnderCursor expressionUnderCursor;
        const QString expression = expressionUnderCursor(tc);

        const QList<LookupItem> types = typeOfExpression(expression, doc, lastSymbol);

        if (!types.isEmpty()) {
            const LookupItem result = types.first();

            FullySpecifiedType firstType = result.type(); // result of `type of expression'.
            Symbol *lookupSymbol = result.lastVisibleSymbol(); // lookup symbol

            Symbol *resolvedSymbol = lookupSymbol;
            const Name *resolvedName = lookupSymbol ? lookupSymbol->name() : 0;
            firstType = resolve(firstType, typeOfExpression.lookupContext(),
                                &resolvedSymbol, &resolvedName);

            if (resolvedSymbol && resolvedSymbol->scope()
                && resolvedSymbol->scope()->isClassScope()) {
                Class *enclosingClass = resolvedSymbol->scope()->owner()->asClass();
                if (const Identifier *id = enclosingClass->identifier()) {
                    if (id->isEqualTo(resolvedSymbol->identifier()))
                        resolvedSymbol = enclosingClass;
                }
            }

            m_helpId = buildHelpId(resolvedSymbol, resolvedName);

            if (m_toolTip.isEmpty()) {
                Symbol *symbol = result.lastVisibleSymbol();
                if (resolvedSymbol)
                    symbol = resolvedSymbol;

                Overview overview;
                overview.setShowArgumentNames(true);
                overview.setShowReturnTypes(true);
                overview.setShowFullyQualifiedNamed(true);

                if (symbol && symbol == resolvedSymbol && symbol->isClass()) {
                    m_toolTip = m_helpId;

                } else if (lookupSymbol && (lookupSymbol->isDeclaration() || lookupSymbol->isArgument())) {
                    m_toolTip = overview.prettyType(firstType, buildHelpId(lookupSymbol, lookupSymbol->name()));

                } else if (firstType->isClassType() || firstType->isEnumType() ||
                           firstType->isForwardClassDeclarationType()) {
                    m_toolTip = m_helpId;

                } else {
                    m_toolTip = overview.prettyType(firstType, m_helpId);

                }
            }

            // Some docs don't contain the namespace in the documentation pages, for instance
            // there is QtMobility::QContactManager but the help page is for QContactManager.
            // To show their help anyway, try stripping scopes until we find something.
            const QString startHelpId = m_helpId;
            while (!m_helpId.isEmpty()) {
                helpLinks = Core::HelpManager::instance()->linksForIdentifier(m_helpId);
                if (!helpLinks.isEmpty())
                    break;

                int coloncolonIndex = m_helpId.indexOf(QLatin1String("::"));
                if (coloncolonIndex == -1) {
                    m_helpId = startHelpId;
                    break;
                }

                m_helpId.remove(0, coloncolonIndex + 2);
            }
        }
    }

    if (m_toolTip.isEmpty()) {
        foreach (const Document::MacroUse &use, doc->macroUses()) {
            if (use.contains(pos)) {
                const Macro m = use.macro();
                m_toolTip = m.toString();
                m_helpId = m.name();
                break;
            }
        }
    }

    if (!formatTooltip.isEmpty())
        m_toolTip = formatTooltip;

    const int tipWidth = QFontMetrics(QToolTip::font()).width(m_toolTip);
    bool preventWrapping = true;

    if (screen != -1) {
#ifdef Q_WS_MAC
        int screenWidth = QApplication::desktop()->availableGeometry(screen).width();
#else
        int screenWidth = QApplication::desktop()->screenGeometry(screen).width();
#endif
        if (tipWidth > screenWidth * .8)
            preventWrapping = false;
    }

    if (!m_helpId.isEmpty() && !helpLinks.isEmpty()) {
        if (showF1) {
            if (preventWrapping) {
                m_toolTip = QString(QLatin1String("<table><tr><td valign=middle width=%2>%1</td>"
                                                  "<td><img src=\":/cppeditor/images/f1.png\"></td></tr></table>"))
                            .arg(Qt::escape(m_toolTip)).arg(tipWidth);
            } else {
                m_toolTip = QString(QLatin1String("<table><tr><td valign=middle>%1</td>"
                                                  "<td><img src=\":/cppeditor/images/f1.png\"></td></tr></table>"))
                            .arg(Qt::escape(m_toolTip));
            }
        }
        editor->setContextHelpId(m_helpId);
    } else if (!m_toolTip.isEmpty()) {
        if (preventWrapping)
            m_toolTip = QString(QLatin1String("<table><tr><td width=%2>%1</td></tr></table>")).arg(Qt::escape(m_toolTip)).arg(tipWidth);
        else if (!Qt::mightBeRichText(m_toolTip))
            m_toolTip = QString(QLatin1String("<p>%1</p>")).arg(Qt::escape(m_toolTip));
    }
}
