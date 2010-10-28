#include "qmljslink.h"

#include "parser/qmljsast_p.h"
#include "qmljsdocument.h"
#include "qmljsbind.h"
#include "qmljsscopebuilder.h"

#include <QtCore/QFileInfo>
#include <QtCore/QDir>
#include <QtCore/QDebug>

using namespace QmlJS;
using namespace QmlJS::Interpreter;
using namespace QmlJS::AST;

Link::Link(Context *context, const Document::Ptr &doc, const Snapshot &snapshot,
           const QStringList &importPaths)
    : _doc(doc)
    , _snapshot(snapshot)
    , _context(context)
    , _importPaths(importPaths)
{
    linkImports();
    initializeScopeChain();
}

Link::~Link()
{
}

Interpreter::Engine *Link::engine()
{
    return _context->engine();
}

QList<DiagnosticMessage> Link::diagnosticMessages() const
{
    return _diagnosticMessages;
}

void Link::initializeScopeChain()
{
    ScopeChain &scopeChain = _context->scopeChain();

    // ### TODO: This object ought to contain the global namespace additions by QML.
    scopeChain.globalScope = engine()->globalObject();

    if (! _doc) {
        scopeChain.update();
        return;
    }

    Bind *bind = _doc->bind();
    QHash<Document *, ScopeChain::QmlComponentChain *> componentScopes;

    if (_doc->qmlProgram()) {
        scopeChain.qmlComponentScope.clear();
        componentScopes.insert(_doc.data(), &scopeChain.qmlComponentScope);
        makeComponentChain(_doc, &scopeChain.qmlComponentScope, &componentScopes);

        if (const ObjectValue *typeEnvironment = _context->typeEnvironment(_doc.data()))
            scopeChain.qmlTypes = typeEnvironment;
    } else {
        // add scope chains for all components that import this file
        foreach (Document::Ptr otherDoc, _snapshot) {
            foreach (const QString &fileImport, otherDoc->bind()->fileImports()) {
                if (_doc->fileName() == fileImport) {
                    ScopeChain::QmlComponentChain *component = new ScopeChain::QmlComponentChain;
                    componentScopes.insert(otherDoc.data(), component);
                    scopeChain.qmlComponentScope.instantiatingComponents += component;
                    makeComponentChain(otherDoc, component, &componentScopes);
                }
            }
        }

        // ### TODO: Which type environment do scripts see?

        if (bind->rootObjectValue())
            scopeChain.jsScopes += bind->rootObjectValue();
    }

    scopeChain.update();
}

void Link::makeComponentChain(
        Document::Ptr doc,
        ScopeChain::QmlComponentChain *target,
        QHash<Document *, ScopeChain::QmlComponentChain *> *components)
{
    if (!doc->qmlProgram())
        return;

    Bind *bind = doc->bind();

    // add scopes for all components instantiating this one
    foreach (Document::Ptr otherDoc, _snapshot) {
        if (otherDoc == doc)
            continue;
        if (otherDoc->bind()->usesQmlPrototype(bind->rootObjectValue(), _context)) {
            if (components->contains(otherDoc.data())) {
//                target->instantiatingComponents += components->value(otherDoc.data());
            } else {
                ScopeChain::QmlComponentChain *component = new ScopeChain::QmlComponentChain;
                components->insert(otherDoc.data(), component);
                target->instantiatingComponents += component;

                makeComponentChain(otherDoc, component, components);
            }
        }
    }

    // build this component scope
    if (bind->rootObjectValue())
        target->rootObject = bind->rootObjectValue();

    target->ids = bind->idEnvironment();
}

void Link::linkImports()
{
    foreach (Document::Ptr doc, _snapshot) {
        ObjectValue *typeEnv = engine()->newObject(/*prototype =*/0); // ### FIXME

        // Populate the _typeEnvironment with imports.
        populateImportedTypes(typeEnv, doc);

        _context->setTypeEnvironment(doc.data(), typeEnv);
    }
}

void Link::populateImportedTypes(Interpreter::ObjectValue *typeEnv, Document::Ptr doc)
{
    if (! (doc->qmlProgram() && doc->qmlProgram()->imports))
        return;

    // Add the implicitly available Script type
    const ObjectValue *scriptValue = engine()->metaTypeSystem().staticTypeForImport("Script");
    if (scriptValue)
        typeEnv->setProperty("Script", scriptValue);

    // implicit imports:
    // qml files in the same directory are available without explicit imports
    foreach (Document::Ptr otherDoc, _snapshot.documentsInDirectory(doc->path())) {
        if (otherDoc == doc || !otherDoc->bind()->rootObjectValue())
            continue;

        typeEnv->setProperty(otherDoc->componentName(),
                             otherDoc->bind()->rootObjectValue());
    }

    // explicit imports, whether directories or files
    for (UiImportList *it = doc->qmlProgram()->imports; it; it = it->next) {
        if (! it->import)
            continue;

        if (it->import->fileName) {
            importFile(typeEnv, doc, it->import);
        } else if (it->import->importUri) {
            importNonFile(typeEnv, doc, it->import);
        }
    }
}

/*
    import "content"
    import "content" as Xxx
    import "content" 4.6
    import "content" 4.6 as Xxx

    import "http://www.ovi.com/" as Ovi
*/
void Link::importFile(Interpreter::ObjectValue *typeEnv, Document::Ptr doc,
                      AST::UiImport *import)
{
    Q_UNUSED(doc)

    if (!import->fileName)
        return;

    QString path = doc->path();
    path += QLatin1Char('/');
    path += import->fileName->asString();
    path = QDir::cleanPath(path);

    ObjectValue *importNamespace = typeEnv;

    // directory import
    QList<Document::Ptr> documentsInDirectory = _snapshot.documentsInDirectory(path);
    if (! documentsInDirectory.isEmpty()) {
        if (import->importId) {
            importNamespace = engine()->newObject(/*prototype =*/0);
            typeEnv->setProperty(import->importId->asString(), importNamespace);
        }

        foreach (Document::Ptr importedDoc, documentsInDirectory) {
            if (importedDoc->bind()->rootObjectValue()) {
                const QString targetName = importedDoc->componentName();
                importNamespace->setProperty(targetName, importedDoc->bind()->rootObjectValue());
            }
        }
    }
    // file import
    else if (Document::Ptr importedDoc = _snapshot.document(path)) {
        if (importedDoc->bind()->rootObjectValue()) {
            QString targetName;
            if (import->importId) {
                targetName = import->importId->asString();
            } else {
                targetName = importedDoc->componentName();
            }

            importNamespace->setProperty(targetName, importedDoc->bind()->rootObjectValue());
        }
    } else {
        error(doc, import->fileNameToken,
              tr("could not find file or directory"));
    }
}

static SourceLocation locationFromRange(const SourceLocation &start,
                                        const SourceLocation &end)
{
    return SourceLocation(start.offset,
                          end.end() - start.begin(),
                          start.startLine,
                          start.startColumn);
}

/*
  import Qt 4.6
  import Qt 4.6 as Xxx
  (import com.nokia.qt is the same as the ones above)
*/
void Link::importNonFile(Interpreter::ObjectValue *typeEnv, Document::Ptr doc, AST::UiImport *import)
{
    if (! import->importUri)
        return;

    ObjectValue *namespaceObject = 0;

    if (import->importId) { // with namespace we insert an object in the type env. to hold the imported types
        namespaceObject = engine()->newObject(/*prototype */ 0);
        typeEnv->setProperty(import->importId->asString(), namespaceObject);

    } else { // without namespace we insert all types directly into the type env.
        namespaceObject = typeEnv;
    }

    const QString packageName = Bind::toString(import->importUri, '.');
    int majorVersion = QmlObjectValue::NoVersion;
    int minorVersion = QmlObjectValue::NoVersion;

    if (import->versionToken.isValid()) {
        const QString versionString = doc->source().mid(import->versionToken.offset, import->versionToken.length);
        const int dotIdx = versionString.indexOf(QLatin1Char('.'));
        if (dotIdx == -1) {
            error(doc, import->versionToken,
                  tr("expected two numbers separated by a dot"));
            return;
        } else {
            majorVersion = versionString.left(dotIdx).toInt();
            minorVersion = versionString.mid(dotIdx + 1).toInt();
        }
    } else {
        error(doc, locationFromRange(import->firstSourceLocation(), import->lastSourceLocation()),
              tr("package import requires a version number"));
        return;
    }

    // if the package is in the meta type system, use it
    if (engine()->metaTypeSystem().hasPackage(packageName)) {
        foreach (QmlObjectValue *object, engine()->metaTypeSystem().staticTypesForImport(packageName, majorVersion, minorVersion)) {
            namespaceObject->setProperty(object->className(), object);
        }
        return;
    } else {
        // check the filesystem
        const QString packagePath = Bind::toString(import->importUri, QDir::separator());
        foreach (const QString &importPath, _importPaths) {
            QDir dir(importPath);
            if (!dir.cd(packagePath))
                continue;

            const LibraryInfo libraryInfo = _snapshot.libraryInfo(dir.path());
            if (!libraryInfo.isValid())
                continue;

            if (!libraryInfo.plugins().isEmpty())
                _context->setDocumentImportsPlugins(doc.data());

            QSet<QString> importedTypes;
            foreach (const QmlDirParser::Component &component, libraryInfo.components()) {
                if (importedTypes.contains(component.typeName))
                    continue;

                if (component.majorVersion > majorVersion
                    || (component.majorVersion == majorVersion
                        && component.minorVersion > minorVersion))
                    continue;

                importedTypes.insert(component.typeName);
                if (Document::Ptr importedDoc = _snapshot.document(dir.filePath(component.fileName))) {
                    if (importedDoc->bind()->rootObjectValue())
                        namespaceObject->setProperty(component.typeName, importedDoc->bind()->rootObjectValue());
                }
            }

            return;
        }
    }

    error(doc, locationFromRange(import->firstSourceLocation(), import->lastSourceLocation()),
          tr("package not found"));
}

UiQualifiedId *Link::qualifiedTypeNameId(Node *node)
{
    if (UiObjectBinding *binding = AST::cast<UiObjectBinding *>(node))
        return binding->qualifiedTypeNameId;
    else if (UiObjectDefinition *binding = AST::cast<UiObjectDefinition *>(node))
        return binding->qualifiedTypeNameId;
    else
        return 0;
}

void Link::error(const Document::Ptr &doc, const AST::SourceLocation &loc, const QString &message)
{
    if (doc->fileName() == _doc->fileName())
        _diagnosticMessages.append(DiagnosticMessage(DiagnosticMessage::Error, loc, message));
}
