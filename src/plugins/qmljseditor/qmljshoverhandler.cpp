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

#include "qmljseditor.h"
#include "qmlexpressionundercursor.h"
#include "qmljshoverhandler.h"

#include <coreplugin/icore.h>
#include <coreplugin/helpmanager.h>
#include <coreplugin/uniqueidmanager.h>
#include <coreplugin/editormanager/editormanager.h>
#include <debugger/debuggerconstants.h>
#include <extensionsystem/pluginmanager.h>
#include <qmljs/qmljsbind.h>
#include <qmljs/qmljsevaluate.h>
#include <qmljs/qmljsinterpreter.h>
#include <qmljs/qmljslink.h>
#include <qmljs/qmljsscopebuilder.h>
#include <qmljs/parser/qmljsast_p.h>
#include <texteditor/itexteditor.h>
#include <texteditor/basetexteditor.h>

#include <QtCore/QDebug>
#include <QtCore/QDir>
#include <QtCore/QFileInfo>
#include <QtCore/QSettings>
#include <QtGui/QToolTip>
#include <QtGui/QTextCursor>
#include <QtGui/QTextBlock>

using namespace Core;
using namespace QmlJS;
using namespace QmlJSEditor;
using namespace QmlJSEditor::Internal;

HoverHandler::HoverHandler(QObject *parent)
    : QObject(parent)
{
    m_modelManager = ExtensionSystem::PluginManager::instance()->getObject<ModelManagerInterface>();

    // Listen for editor opened events in order to connect to tooltip/helpid requests
    connect(ICore::instance()->editorManager(), SIGNAL(editorOpened(Core::IEditor *)),
            this, SLOT(editorOpened(Core::IEditor *)));
}

void HoverHandler::editorOpened(IEditor *editor)
{
    QmlJSEditorEditable *qmlEditor = qobject_cast<QmlJSEditorEditable *>(editor);
    if (!qmlEditor)
        return;

    connect(qmlEditor, SIGNAL(tooltipRequested(TextEditor::ITextEditor*, QPoint, int)),
            this, SLOT(showToolTip(TextEditor::ITextEditor*, QPoint, int)));

    connect(qmlEditor, SIGNAL(contextHelpIdRequested(TextEditor::ITextEditor*, int)),
            this, SLOT(updateContextHelpId(TextEditor::ITextEditor*, int)));
}

void HoverHandler::showToolTip(TextEditor::ITextEditor *editor, const QPoint &point, int pos)
{
    if (! editor)
        return;

    ICore *core = ICore::instance();
    const int dbgcontext = core->uniqueIDManager()->uniqueIdentifier(Debugger::Constants::C_DEBUGMODE);

    if (core->hasContext(dbgcontext))
        return;

    updateHelpIdAndTooltip(editor, pos);

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

void HoverHandler::updateContextHelpId(TextEditor::ITextEditor *editor, int pos)
{
    updateHelpIdAndTooltip(editor, pos);
}

void HoverHandler::updateHelpIdAndTooltip(TextEditor::ITextEditor *editor, int pos)
{
    m_helpId.clear();
    m_toolTip.clear();

    if (!m_modelManager)
        return;

    QmlJSTextEditor *edit = qobject_cast<QmlJSTextEditor *>(editor->widget());
    if (!edit)
        return;

    const SemanticInfo semanticInfo = edit->semanticInfo();

    if (semanticInfo.revision() != edit->documentRevision())
        return;

    const Snapshot snapshot = semanticInfo.snapshot;
    const Document::Ptr qmlDocument = semanticInfo.document;

    // We only want to show F1 if the tooltip matches the help id
    bool showF1 = true;

    foreach (const QTextEdit::ExtraSelection &sel, edit->extraSelections(TextEditor::BaseTextEditor::CodeWarningsSelection)) {
        if (pos >= sel.cursor.selectionStart() && pos <= sel.cursor.selectionEnd()) {
            showF1 = false;
            m_toolTip = sel.format.toolTip();
        }
    }

    QString symbolName = QLatin1String("<unknown>");
    if (m_helpId.isEmpty() && m_toolTip.isEmpty()) {
        AST::Node *node = semanticInfo.nodeUnderCursor(pos);
        if (node && !(AST::cast<AST::StringLiteral *>(node) != 0 || AST::cast<AST::NumericLiteral *>(node) != 0)) {
            QList<AST::Node *> astPath = semanticInfo.astPath(pos);

            Interpreter::Engine interp;
            Interpreter::Context context(&interp);
            Link link(&context, qmlDocument, snapshot, m_modelManager->importPaths());
            ScopeBuilder scopeBuilder(qmlDocument, &context);
            scopeBuilder.push(astPath);

            Evaluate check(&context);
            const Interpreter::Value *value = check(node);

            QStringList baseClasses;
            m_toolTip = prettyPrint(value, &context, &baseClasses);

            foreach (const QString &baseClass, baseClasses) {
                QString helpId = QLatin1String("QML.");
                helpId += baseClass;

                if (!Core::HelpManager::instance()->linksForIdentifier(helpId).isEmpty()) {
                    m_helpId = helpId;
                    break;
                }
            }
        }
    }

    if (!m_toolTip.isEmpty())
        m_toolTip = Qt::escape(m_toolTip);

    if (!m_helpId.isEmpty()) {
        if (showF1) {
            m_toolTip = QString::fromUtf8("<table><tr><td valign=middle><nobr>%1</td>"
                                            "<td><img src=\":/cppeditor/images/f1.png\"></td></tr></table>")
                    .arg(m_toolTip);
        }
        editor->setContextHelpId(m_helpId);
    } else if (!m_toolTip.isEmpty()) {
        m_toolTip = QString::fromUtf8("<nobr>%1").arg(m_toolTip);
    } else if (!m_helpId.isEmpty()) {
        m_toolTip = QString::fromUtf8("<nobr>No help available for \"%1\"").arg(symbolName);
    }
}

QString HoverHandler::prettyPrint(const QmlJS::Interpreter::Value *value, QmlJS::Interpreter::Context *context,
                                     QStringList *baseClasses) const
{
    if (! value)
        return QString();

    if (const Interpreter::ObjectValue *objectValue = value->asObjectValue()) {
        do {
            const QString className = objectValue->className();

            if (! className.isEmpty())
                baseClasses->append(className);

            objectValue = objectValue->prototype(context);
        } while (objectValue);

        if (! baseClasses->isEmpty())
            return baseClasses->first();
    } else if (const Interpreter::QmlEnumValue *enumValue = dynamic_cast<const Interpreter::QmlEnumValue *>(value)) {
        return enumValue->name();
    }

    QString typeId = context->engine()->typeId(value);

    if (typeId == QLatin1String("undefined"))
        typeId.clear();

    return typeId;
}
