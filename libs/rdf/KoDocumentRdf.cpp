/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>
   Copyright (C) 2011,2012 Ben Martin <monkeyiq@users.sourceforge.net> hacking for fun!

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include "KoDocumentRdf.h"

#include "KoRdfPrefixMapping.h"
#include "RdfSemanticTreeWidgetSelectAction.h"
#include "InsertSemanticObjectActionBase.h"
#include "InsertSemanticObjectCreateAction.h"
#include "InsertSemanticObjectReferenceAction.h"
#include "KoRdfSemanticItemRegistry.h"

#include <KoTextDocument.h>
#include <KoTextRdfCore.h>
#include <KoXmlWriter.h>
#include <KoStoreDevice.h>
#include <KoCanvasBase.h>
#include <KoTextEditor.h>
#include <KoInlineObject.h>
#include <KoTextInlineRdf.h>
#include <KoInlineTextObjectManager.h>
#include <KoTextRangeManager.h>
#include <KoTextMeta.h>

#include <kdebug.h>
#include <klocalizedstring.h>
#include <QAction>

#include <QWeakPointer>

// #define DEBUG_RDF

#ifdef DEBUG_RDF
#define RDEBUG kDebug(30015)
#else
#define RDEBUG if(0) kDebug(30015)
#endif

using namespace Soprano;

class KoDocumentRdfPrivate
{
public:

    KoDocumentRdfPrivate()
        : model(Soprano::createModel())
        , prefixMapping(0)
    {
    }

    ~KoDocumentRdfPrivate()
    {
        prefixMapping->deleteLater();
        model->deleteLater();
    }

    QSharedPointer<Soprano::Model> model; ///< Main Model containing all Rdf for doc
    QMap<QString, QWeakPointer<KoTextInlineRdf> > inlineRdfObjects;  ///< Cache of weak pointers to inline Rdf
    KoRdfPrefixMapping *prefixMapping;     ///< prefix -> URI mapping

    QMap<QString, QList<hKoRdfBasicSemanticItem> > semanticItems;
    QMap<QString, QList<hKoSemanticStylesheet> > userStylesheets;
};

const QString KoDocumentRdf::RDF_PATH_CONTEXT_PREFIX = "http://www.calligra.org/Rdf/path/";

KoDocumentRdf::KoDocumentRdf(QObject *parent)
    : KoDocumentRdfBase(parent)
    , d (new KoDocumentRdfPrivate())
{
//    if (!backendIsSane()) {
//        kWarning() << "Looks like the backend is not sane!";
//    }
    d->prefixMapping = new KoRdfPrefixMapping(this);
}

KoDocumentRdf::~KoDocumentRdf()
{
    RDEBUG; //FIXME: looks like unused call to kDebug()
    delete d;
}

QSharedPointer<Soprano::Model> KoDocumentRdf::model() const
{
    return d->model;
}

KoRdfPrefixMapping *KoDocumentRdf::prefixMapping() const
{
    return d->prefixMapping;
}

/**
 * Graph context used for Rdf stored inline in content.xml
 * in an Rdfa like fashion.
 */
Soprano::Node KoDocumentRdf::inlineRdfContext() const
{
    return Node(QUrl("http://www.calligra.org/Rdf/inline-rdf"));
}

QString KoDocumentRdf::rdfInternalMetadataWithoutSubjectURI() const
{
    return "http://www.calligra.org/Rdf/internal/content.xml";
}

Soprano::Node KoDocumentRdf::manifestRdfNode() const
{
    return Node(QUrl(RDF_PATH_CONTEXT_PREFIX + "manifest.rdf"));
}

void KoDocumentRdf::freshenBNodes(QSharedPointer<Soprano::Model> m)
{
    Q_ASSERT(m);
    Q_ASSERT(d->model);

    QList<Soprano::Statement> removeList;
    QList<Soprano::Statement> addList;
    QMap<QString, Soprano::Node> bnodeMap;
    StatementIterator it = m->listStatements();
    QList<Statement> allStatements = it.allElements();

    RDEBUG << "freshening model.sz:" << allStatements.size();

    foreach (const Soprano::Statement &s, allStatements) {
        Soprano::Node subj = s.subject();
        Soprano::Node obj = s.object();
        Soprano::Statement news;

        if (subj.type() == Soprano::Node::BlankNode) {
            QString nodeStr = subj.toString();
            Soprano::Node n = bnodeMap[nodeStr];
            if (!n.isValid()) {
                n = d->model->createBlankNode();
                bnodeMap[nodeStr] = n;
            }
            removeList << s;
            subj = n;
            news = Statement(subj, s.predicate(), obj, s.context());
        }

        if (obj.type() == Soprano::Node::BlankNode) {
            QString nodeStr = obj.toString();
            Soprano::Node n = bnodeMap[ nodeStr ];
            if (!n.isValid()) {
                n = d->model->createBlankNode();
                bnodeMap[ nodeStr ] = n;
            }
            removeList << s;
            obj = n;
            news = Statement(subj, s.predicate(), obj, s.context());
        }

        if (news.isValid()) {
            addList << news;
        }
    }
    RDEBUG << "remove count:" << removeList.size();
    RDEBUG << "add count:" << addList.size();
    // Note that as of Jan 2010 you couldn't rely on
    // Soprano::Model::removeStatements() if every entry
    // in removeList did not exist exactly once in the model.
    KoTextRdfCore::removeStatementsIfTheyExist(m, removeList);
    RDEBUG << "after remove, model.sz:" << m->statementCount();
    m->addStatements(addList);
    RDEBUG << "after add,    model.sz:" << m->statementCount();
}

bool KoDocumentRdf::loadRdf(KoStore *store, const Soprano::Parser *parser, const QString &fileName)
{
    QSharedPointer<Soprano::Model> tmpmodel(Soprano::createModel());
    if (!d->model || !tmpmodel) {
        kWarning(30003) << "No soprano model";
        return false;
    }

    bool ok = true;
    if (!store->open(fileName)) {
        RDEBUG << "Entry " << fileName << " not found!"; // not a warning as embedded stores don't have to have all files
        return false;
    }

    RDEBUG << "Loading external Rdf/XML from:" << fileName;

    Soprano::Node context(QUrl(RDF_PATH_CONTEXT_PREFIX + fileName));
    QUrl BaseURI = QUrl(QString());
    QString rdfxmlData(store->device()->readAll());
    Soprano::StatementIterator it = parser->parseString(rdfxmlData, BaseURI, Soprano::SerializationRdfXml);
    QList<Statement> allStatements = it.allElements();
    RDEBUG << "Found " << allStatements.size() << " triples..." << endl;
    foreach (const Soprano::Statement &s, allStatements) {
        Soprano::Node subj = s.subject();
        Soprano::Node pred = s.predicate();
        Soprano::Node obj  = s.object();
        Error::ErrorCode err = tmpmodel->addStatement(subj, pred, obj, context);
        if (err != Error::ErrorNone) {
            RDEBUG << "Error adding triple! s:" << subj << " p:" << pred << " o:" << obj << endl;
            ok = false;
            break;
        }
    }
    RDEBUG << "calling freshenBNodes(), tmpmodel.sz:" << tmpmodel->statementCount();

#ifdef DEBUG_RDF
    dumpModel(fileName, tmpmodel);
#endif
    freshenBNodes(tmpmodel);
#ifdef DEBUG_RDF
    dumpModel(fileName, tmpmodel);
#endif

    RDEBUG << "done with freshenBNodes(), tmpmodel.sz:" << tmpmodel->statementCount();

    d->model->addStatements(tmpmodel->listStatements().allElements());
    if (fileName == "manifest.rdf" && d->prefixMapping) {
        d->prefixMapping->load(d->model);

        foreach (const QString &semanticClass, KoRdfSemanticItemRegistry::instance()->classNames()) {
            if (!KoRdfSemanticItemRegistry::instance()->isBasic(semanticClass)) {
                hKoRdfSemanticItem si(static_cast<KoRdfSemanticItem *>(
		    KoRdfSemanticItemRegistry::instance()->createSemanticItem(semanticClass, this, this).data()
		));
                si->loadUserStylesheets(d->model);
            }
        }
    }
    store->close();
    return ok;
}

bool KoDocumentRdf::loadOasis(KoStore *store)
{
    if (!store) {
        kWarning(30003) << "No store backend";
        return false;
    }

    if (!d->model) {
        kWarning(30003) << "No soprano model";
        return false;
    }

    const Soprano::Parser *parser =
        Soprano::PluginManager::instance()->discoverParserForSerialization(
            Soprano::SerializationRdfXml);
    bool ok = loadRdf(store, parser, "manifest.rdf");
    if (ok) {
        QString sparqlQuery =
            "prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
            "prefix odf: <http://docs.oasis-open.org/ns/office/1.2/meta/odf#> \n"
            "prefix pkg: <http://docs.oasis-open.org/ns/office/1.2/meta/pkg#> \n"
            "select ?subj ?fileName \n"
            " where { \n"
            "  ?subj rdf:type odf:MetaDataFile . \n"
            "  ?subj pkg:path ?fileName  \n"
            " } \n";
        Soprano::QueryResultIterator it =
            d->model->executeQuery(sparqlQuery,
                                  Soprano::Query::QueryLanguageSparql);

        QList< QString > externalRdfFiles;
        //
        // This is a bit tricky, loadRdf() might block if the
        // sparql query is still being iterated, so we have to
        // store the fileNames and exhaust the binding result
        // iterator first.
        //
        while (it.next()) {
            QString fileName = it.binding("fileName").toString();
            externalRdfFiles << fileName;
        }

        foreach (const QString &fileName, externalRdfFiles) {
            ok = loadRdf(store, parser, fileName);
            if (!ok) {
                break;
            }
        }
    }
    return ok;
}

bool KoDocumentRdf::saveRdf(KoStore *store, KoXmlWriter *manifestWriter, const Soprano::Node &context) const
{
    bool ok = false;
    QString fileName("manifest.rdf");
    if (context.toString() == inlineRdfContext().toString()) {
        RDEBUG << "found some internal Rdf, this is handled by augmenting the DOM";
        return true;
    }

    if (!model()) {
        kWarning(30003) << "No soprano model";
        return false;
    }

    //
    // The context contains the filename to save into
    //
    if (context.toString().startsWith(RDF_PATH_CONTEXT_PREFIX)) {
        fileName = context.toString().mid(RDF_PATH_CONTEXT_PREFIX.size());
    }

    RDEBUG << "saving external file:" << fileName;
    if (!store->open(fileName)) {
        return false;
    }

    KoStoreDevice dev(store);
    QTextStream oss(&dev);
    if (fileName == "manifest.rdf" && d->prefixMapping) {
        d->prefixMapping->save(d->model, context);

        foreach (const QString &semanticClass, KoRdfSemanticItemRegistry::instance()->classNames()) {
            if (!KoRdfSemanticItemRegistry::instance()->isBasic(semanticClass)) {
                hKoRdfSemanticItem si(static_cast<KoRdfSemanticItem *>(
		    KoRdfSemanticItemRegistry::instance()->createSemanticItem(
			semanticClass,
			this,
			const_cast<KoDocumentRdf*>(this)
		    ).data()
		));
                si->saveUserStylesheets(d->model, context);
            }
        }
    }
    Soprano::StatementIterator triples = model()->listStatements(Soprano::Node(),
        Soprano::Node(), Soprano::Node(), context);

    const Soprano::Serializer *serializer = Soprano::PluginManager::instance()->
        discoverSerializerForSerialization(Soprano::SerializationRdfXml);

    if (serializer) {
        QString data;
        QTextStream tss(&data);
        if (serializer->serialize(triples, tss, Soprano::SerializationRdfXml)) {
            tss.flush();
            oss << data;
            RDEBUG << "fileName:" << fileName << " data.sz:" << data.size();
            RDEBUG << "model.sz:" << model()->statementCount();
            ok = true;
        } else {
            RDEBUG << "serialization of Rdf failed!";
        }
    }
    oss.flush();
    store->close();
    manifestWriter->addManifestEntry(fileName, "application/rdf+xml");
    return ok;
}

bool KoDocumentRdf::saveOasis(KoStore *store, KoXmlWriter *manifestWriter)
{
    RDEBUG << "saveOasis() generic";
    if (!d->model) {
        kWarning(30003) << "No soprano model";
        return false;
    }
    bool ok = true;
    NodeIterator contextIter = model()->listContexts();
    QList<Node> contexts = contextIter.allElements();
    foreach (const Soprano::Node &node, contexts) {
        if (!saveRdf(store, manifestWriter, node)) {
            ok = false;
        }
    }
    return ok;
}

void KoDocumentRdf::updateXmlIdReferences(const QMap<QString, QString> &m)
{
    Q_ASSERT(d->model);

    QList<Soprano::Statement> removeList;
    QList<Soprano::Statement> addList;
    StatementIterator it = model()->listStatements(
                               Node(),
                               Node(QUrl("http://docs.oasis-open.org/ns/office/1.2/meta/pkg#idref")),
                               Node(),
                               Node());
    if (!it.isValid())
        return;

    // new xmlid->inlinerdfobject mapping
    QMap<QString, QWeakPointer<KoTextInlineRdf> > inlineRdfObjects;

    QList<Statement> allStatements = it.allElements();
    foreach (const Soprano::Statement &s, allStatements) {
        RDEBUG << "seeking obj:" << s.object();
        QMap<QString, QString>::const_iterator mi = m.find(s.object().toString());
        if (mi != m.end()) {
            const QString &oldID = mi.key();
            const QString &newID = mi.value();
            removeList << s;
            Statement n(s.subject(),
                        s.predicate(),
                        Node(LiteralValue::createPlainLiteral(newID)),
                        s.context());
            addList << n;
            RDEBUG << "looking for inlineRdf object for ID:" << oldID;
            if (KoTextInlineRdf *inlineRdf = findInlineRdfByID(oldID)) {
                RDEBUG << "updating the xmlid of the inline object";
                RDEBUG << "old:" << oldID << " new:" << newID;
                inlineRdf->setXmlId(newID);
                inlineRdfObjects[newID] = inlineRdf;
            }
        }
    }
    // out with the old, in with the new
    // remove & add the triple lists.
    RDEBUG << "addStatements.size:" << addList.size();
    RDEBUG << " remove.size:" << removeList.size();
    KoTextRdfCore::removeStatementsIfTheyExist(d->model, removeList);
    d->model->addStatements(addList);
    d->inlineRdfObjects = inlineRdfObjects;

}

QList<hKoRdfBasicSemanticItem> KoDocumentRdf::semanticItems(const QString &className, QSharedPointer<Soprano::Model> m)
{
    if (!m) {
        m = d->model;
        Q_ASSERT(m);
    }

    QList<hKoRdfBasicSemanticItem> items;
    // TODO: improve double lookup
    if (KoRdfSemanticItemRegistry::instance()->classNames().contains(className)) {
        KoRdfSemanticItemRegistry::instance()->updateSemanticItems(d->semanticItems[className], this, className, m);
        items = d->semanticItems[className];
    }

    return items;
}

hKoRdfBasicSemanticItem KoDocumentRdf::createSemanticItem(const QString &semanticClass, QObject *parent) const
{
    return KoRdfSemanticItemRegistry::instance()->createSemanticItem(semanticClass, this, parent);
}

void KoDocumentRdf::dumpModel(const QString &msg, QSharedPointer<Soprano::Model> m) const
{
    if (!m) {
        return;
    }

    QList<Soprano::Statement> allStatements = m->listStatements().allElements();
    RDEBUG << "----- " << msg << " ----- model size:" << allStatements.size();
    foreach (const Soprano::Statement &s, allStatements) {
        RDEBUG << s;
    }
}

Soprano::Statement KoDocumentRdf::toStatement(KoTextInlineRdf *inlineRdf) const
{
    if (!inlineRdf) {
        return Soprano::Statement();
    }
    if (inlineRdf->predicate().isEmpty())  {
        return Soprano::Statement();
    }

    Soprano::Node subj = Soprano::Node::createResourceNode(QUrl(inlineRdf->subject()));
    Soprano::Node pred = Soprano::Node::createResourceNode(QUrl(inlineRdf->predicate()));
    Soprano::Node obj;

    switch (inlineRdf->sopranoObjectType()) {
    case Node::ResourceNode:
        obj = Soprano::Node::createResourceNode(inlineRdf->object());
        break;
    case Node::LiteralNode:
        obj = Soprano::Node::createLiteralNode(inlineRdf->object());
        break;
    case Node::BlankNode:
        obj = Soprano::Node::createBlankNode(inlineRdf->object());
        break;
    }

    if (!inlineRdf->subject().size()) {
        subj = inlineRdfContext();
    }

    RDEBUG << "subj:"  << subj;
    RDEBUG << " pred:" << pred;
    RDEBUG << " obj:"  << obj;
    return Soprano::Statement(subj, pred, obj, inlineRdfContext());
}

void KoDocumentRdf::addStatements(QSharedPointer<Soprano::Model> model, const QString &xmlid)
{
    Q_ASSERT(model);
    Q_ASSERT(d->model);
    QString sparqlQuery;

    RDEBUG << "addStatements model.sz:" << d->model->statementCount() << " xmlid:" << xmlid;

    sparqlQuery = "prefix pkg:  <http://docs.oasis-open.org/ns/office/1.2/meta/pkg#> \n"
                  "\n"
                  "select ?s ?p ?o ?g \n"
                  "where { \n"
                  " graph ?g { ?s ?p ?o } .  ?s pkg:idref ?xmlid  \n"
                  " filter( str(?xmlid) = \"" + xmlid + "\" ) \n"
                  "}\n";

    RDEBUG << "sparql:" << sparqlQuery;
    Soprano::QueryResultIterator it = d->model->executeQuery(sparqlQuery,
                              Soprano::Query::QueryLanguageSparql);

    while (it.next()) {
        Statement s(it.binding("s"),
                    it.binding("p"),
                    it.binding("o"),
                    it.binding("g"));
        model->addStatement(s);
        RDEBUG << "result, s:" << it.binding("s");
        RDEBUG << " p:" << it.binding("p");
        RDEBUG << " o:" << it.binding("o");
    }
}

void KoDocumentRdf::expandStatementsReferencingSubject(QSharedPointer<Soprano::Model> _model) const
{
    Q_ASSERT(_model);
    Q_ASSERT(d->model);
    QList<Statement> addList;
    QList<Statement> allStatements = _model->listStatements().allElements();

    foreach (const Soprano::Statement &s, allStatements) {
        QList<Statement> all = d->model->listStatements(Node(), Node(), s.subject()).allElements();
        addList.append(all);
    }
    _model->addStatements(addList);
}

void KoDocumentRdf::expandStatementsSubjectPointsTo(QSharedPointer<Soprano::Model> _model) const
{
    Q_ASSERT(_model);
    Q_ASSERT(d->model);
    QList<Statement> addList;
    QList<Statement> allStatements = _model->listStatements().allElements();

    foreach (const Soprano::Statement &s, allStatements) {
        QList<Statement> all = d->model->listStatements(s.object(), Node(), Node()).allElements();
        addList.append(all);
    }
    _model->addStatements(addList);
}

void KoDocumentRdf::expandStatementsSubjectPointsTo(QSharedPointer<Soprano::Model> _model, const Soprano::Node &n) const
{
    Q_ASSERT(_model);
    Q_ASSERT(d->model);
    QList<Statement> addList = d->model->listStatements(n, Node(), Node()).allElements();

    _model->addStatements(addList);
}

void KoDocumentRdf::expandStatementsToIncludeRdfListsRecurse(QSharedPointer<Soprano::Model> _model,
        QList<Statement> &addList, const Soprano::Node &n) const
{
    Q_ASSERT(_model);
    Q_ASSERT(d->model);
    Node rdfFirst = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#first"));
    Node rdfRest  = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"));
    QList<Statement> all;

    all = model()->listStatements(n, rdfFirst, Node()).allElements();
    addList << all;
    all = model()->listStatements(n, rdfRest, Node()).allElements();
    addList << all;
    foreach (const Soprano::Statement &s, all) {
        expandStatementsToIncludeRdfListsRecurse(_model, addList, s.object());
    }
}

void KoDocumentRdf::expandStatementsToIncludeRdfLists(QSharedPointer<Soprano::Model> model) const
{
    Q_ASSERT(model);
    RDEBUG << "model.sz:" << model->statementCount();
    QList<Statement> addList;
    QList<Statement> allStatements = model->listStatements().allElements();

    foreach (const Soprano::Statement &s, allStatements) {
        expandStatementsToIncludeRdfListsRecurse(model, addList, s.subject());
    }
    RDEBUG << "model.sz:" << model->statementCount();
    RDEBUG << "addList.sz:" << addList.size();
    model->addStatements(addList);
}

void KoDocumentRdf::expandStatementsToIncludeOtherPredicates(QSharedPointer<Soprano::Model> _model) const
{
    Q_ASSERT(_model);
    Q_ASSERT(d->model);
    QList<Statement> addList;
    QList<Statement> allStatements = _model->listStatements().allElements();

    foreach (const Soprano::Statement &s, allStatements) {
        QList<Statement> all = model()->listStatements(s.subject(), Node(), Node()).allElements();
        addList.append(all);
    }
    _model->addStatements(addList);
}

void KoDocumentRdf::expandStatements(QSharedPointer<Soprano::Model> model) const
{
    Q_ASSERT(model);
    expandStatementsReferencingSubject(model);
    expandStatementsToIncludeOtherPredicates(model);
}

QAction *KoDocumentRdf::createInsertSemanticObjectReferenceAction(KoCanvasBase *host)
{
    QAction *ret = new InsertSemanticObjectReferenceAction(host, this, i18n("Reference"));
    RDEBUG << "createInsertSemanticObjectReferenceAction";
    return ret;
}

QList<QAction *> KoDocumentRdf::createInsertSemanticObjectNewActions(KoCanvasBase *host)
{
    QList<QAction *> ret;
    foreach (const QString &semanticClass, KoRdfSemanticItemRegistry::instance()->classNames()) {
        if (!KoRdfSemanticItemRegistry::instance()->isBasic(semanticClass)) {
            ret.append(new InsertSemanticObjectCreateAction(host, this, semanticClass));
        }
    }
    return ret;
}

QPair<int, int> KoDocumentRdf::findExtent(const QString &xmlid) const
{
    KoTextInlineRdf *obj = findInlineRdfByID(xmlid);
    if (obj) {
        QPair<int, int> ret = obj->findExtent();
        RDEBUG << "(Semantic) have inline obj, extent:" << ret;
        return ret;
    }
    return QPair<int, int>(-1, 0);
}

QPair<int, int> KoDocumentRdf::findExtent(KoTextEditor *handler) const
{
    Q_ASSERT(d->model);
    RDEBUG << "model.sz:" << d->model->statementCount();

    const QTextDocument *document = handler->document();

    // first check for bookmarks
    KoTextRangeManager *mgr = KoTextDocument(document).textRangeManager();
    Q_ASSERT(mgr);
    QHash<int, KoTextRange *> textRanges = mgr->textRangesChangingWithin(handler->document(), 0, handler->selectionEnd(), handler->selectionStart(), -1);
    foreach (const KoTextRange *range, textRanges) {
        return QPair<int,int>(range->rangeStart(), range->rangeEnd());
    }
/*
    // find the text:meta inline objects
    int startPosition = handler->position();
    KoInlineTextObjectManager *inlineObjectManager
                = KoTextDocument(handler->document()).inlineTextObjectManager();
    Q_ASSERT(inlineObjectManager);

    QTextCursor cursor = document->find(QString(QChar::ObjectReplacementCharacter),
                                        startPosition,
                                        QTextDocument::FindBackward);
    while(!cursor.isNull()) {
        RDEBUG <<  "findXmlId" << cursor.position();
        QTextCharFormat fmt = cursor.charFormat();
        KoInlineObject *obj = inlineObjectManager->inlineTextObject(fmt);

        if (KoTextMeta *metamark = dynamic_cast<KoTextMeta*>(obj)) {
            if (metamark->type() == KoTextMeta::StartBookmark) {
                KoTextMeta *endmark = metamark->endBookmark();
                Q_ASSERT(endmark);
                if (endmark->position() > startPosition) {
                    return QPair<int, int>(metamark->position(), endmark->position());
                }
            }
        }
        cursor = document->find(QString(QChar::ObjectReplacementCharacter),
                                cursor.position(),
                                QTextDocument::FindBackward);
    }
    */
    return QPair<int, int>(-1, 0);
}

QString KoDocumentRdf::findXmlId(KoTextEditor *handler) const
{
    int startPosition = handler->position();
    Q_UNUSED(startPosition);

    KoTextInlineRdf *inlineRdf = 0;

    const QTextDocument *document = handler->document();

    // first check for bookmarks
    KoTextRangeManager *mgr = KoTextDocument(document).textRangeManager();
    Q_ASSERT(mgr);
    QHash<int, KoTextRange *> textRanges = mgr->textRangesChangingWithin(document, 0, handler->selectionEnd(), handler->selectionStart(), -1);
    foreach (const KoTextRange *range, textRanges) {
        inlineRdf = range->inlineRdf();
        if (inlineRdf) {
            return inlineRdf->xmlId();
        }
    }

/*
    // find the text:meta inline objects
    KoInlineTextObjectManager *inlineObjectManager
                = KoTextDocument(document).inlineTextObjectManager();
    Q_ASSERT(inlineObjectManager);
    QTextCursor cursor = document->find(QString(QChar::ObjectReplacementCharacter),
                                        startPosition,
                                        QTextDocument::FindBackward);
    while(!cursor.isNull()) {
        RDEBUG << "Cursor position" << cursor.position();
        QTextCharFormat fmt = cursor.charFormat();
        KoInlineObject *obj = inlineObjectManager->inlineTextObject(fmt);
        RDEBUG << "obj" << obj;

        if (KoTextMeta *metamark = dynamic_cast<KoTextMeta*>(obj)) {
            if (metamark->type() == KoTextMeta::StartBookmark) {
                KoTextMeta *endmark = metamark->endBookmark();
                // we used to assert on endmark, but we cannot keep people from
                // inserting a startbookmark and only then creating and inserting
                // the endmark
                if (endmark && endmark->position() > startPosition) {
                    inlineRdf = metamark->inlineRdf();
                }
            }
        }

        // if we've got inline rdf, we've found the nearest xmlid wrapping our current position
        if (inlineRdf) {
            break;
        }

        if( cursor.position() <= 0 )
            break;

        // else continue with the next inline object
        cursor = document->find(QString(QChar::ObjectReplacementCharacter),
                                cursor.position()-1,
                                QTextDocument::FindBackward);
    }
*/
    // we couldn't find inline rdf object... So try to see whether there's
    // inlineRdf in the charformat for the current cursor position. It's
    // unlikely, of course. Maybe this should be the first check, though?
    if (!inlineRdf) {
        inlineRdf = KoTextInlineRdf::tryToGetInlineRdf(handler);
    }

    if (inlineRdf) {
        return inlineRdf->xmlId();
    }

    return QString();
}

QSharedPointer<Soprano::Model> KoDocumentRdf::findStatements(const QString &xmlid, int depth)
{
    QSharedPointer<Soprano::Model> ret(Soprano::createModel());
    Q_ASSERT(ret);
    addStatements(ret, xmlid);
    for (int i = 1; i < depth; ++i) {
        expandStatements(ret);
    }
    return ret;
}

QSharedPointer<Soprano::Model> KoDocumentRdf::findStatements(KoTextEditor *handler, int depth)
{
    Q_ASSERT(d->model);

    QSharedPointer<Soprano::Model> ret(Soprano::createModel());
    Q_ASSERT(ret);

    QString xmlid = findXmlId(handler);
    KoTextInlineRdf *inlineRdf = findInlineRdfByID(xmlid);

    RDEBUG << "1 model.sz:" << d->model->statementCount()
        << " ret.sz:" << ret->statementCount();
    if (inlineRdf) {
        RDEBUG << "have inlineRdf1...xmlid:" << inlineRdf->xmlId();
        RDEBUG << " ret.sz:" << ret->statementCount();
        ret->addStatement(toStatement(inlineRdf));
        RDEBUG << "have inlineRdf2...xmlid:" << inlineRdf->xmlId();
        RDEBUG << " ret.sz:" << ret->statementCount();
        QString xmlid = inlineRdf->xmlId();
        addStatements(ret, xmlid);
    }

    RDEBUG << "2 ret.sz:" << ret->statementCount();
    RDEBUG << "checking for block inlineRdf...";
    inlineRdf = KoTextInlineRdf::tryToGetInlineRdf(handler);

    if (inlineRdf) {
        RDEBUG << "inlineRdf:" << (void*)inlineRdf;
        ret->addStatement(toStatement(inlineRdf));
        QString xmlid = inlineRdf->xmlId();
        addStatements(ret, xmlid);
        RDEBUG << "have block inlineRdf...xmlid:" << inlineRdf->xmlId();
    }

    RDEBUG << "3 ret.sz:" << ret->statementCount();
    RDEBUG << "expanding statements...";

    for (int i = 1; i < depth; ++i) {
        expandStatements(ret);
    }

    return ret;
}

KoTextInlineRdf *KoDocumentRdf::findInlineRdfByID(const QString &xmlid) const
{
    if (d->inlineRdfObjects.contains(xmlid)) {
        QWeakPointer<KoTextInlineRdf> inlineRdf = d->inlineRdfObjects[xmlid];
        if (!inlineRdf.isNull()) {
            return inlineRdf.data();
        }
        else {
            d->inlineRdfObjects.remove(xmlid);
        }
    }
    return 0;
}


void KoDocumentRdf::rememberNewInlineRdfObject(KoTextInlineRdf *inlineRdf)
{
    if (!inlineRdf) {
        return;
    }
    d->inlineRdfObjects[inlineRdf->xmlId()] = inlineRdf;
}

void KoDocumentRdf::updateInlineRdfStatements(const QTextDocument *qdoc)
{
    RDEBUG << "top";
    KoInlineTextObjectManager *textObjectManager = KoTextDocument(qdoc).inlineTextObjectManager();
    KoTextRangeManager *textRangeManager = KoTextDocument(qdoc).textRangeManager();
    d->inlineRdfObjects.clear();
    if(!textObjectManager || !textRangeManager) {
        return;
    }
    //
    // Rdf from inline objects
    //
    QList<KoInlineObject*> kiocol = textObjectManager->inlineTextObjects();
    foreach (KoInlineObject *kio, kiocol) {
        if (KoTextInlineRdf *inlineRdf = kio->inlineRdf()) {
            rememberNewInlineRdfObject(inlineRdf);
        }
    }
    //
    // Rdf from textranges
    //
    QList<KoTextRange *> rangelist = textRangeManager->textRanges();
    foreach (KoTextRange *range, rangelist) {
        if (KoTextInlineRdf *inlineRdf = range->inlineRdf()) {
            rememberNewInlineRdfObject(inlineRdf);
        }
    }
    //
    // Browse the blocks and see if any of them have Rdf attached
    //
    QVector<QTextFormat> formats = qdoc->allFormats();
    foreach (const QTextFormat& tf, formats) {
        if (KoTextInlineRdf *inlineRdf = KoTextInlineRdf::tryToGetInlineRdf(tf)) {
            rememberNewInlineRdfObject(inlineRdf);
        }
    }
    if (!d->model) {
        return;
    }
    Soprano::Node context = inlineRdfContext();
    RDEBUG << "removing";
    // delete all inline Rdf statements from model
    d->model->removeAllStatements(Soprano::Node(), Soprano::Node(), Soprano::Node(), context);
    RDEBUG << "adding, count:" << d->inlineRdfObjects.size();

    // add statements from inlineRdfObjects to model
    foreach (const QString &xmlid, d->inlineRdfObjects.keys()) {
        QWeakPointer<KoTextInlineRdf> sp = d->inlineRdfObjects[xmlid];
        if (sp.isNull()) {
            d->inlineRdfObjects.remove(xmlid);
        }
        else {
            Soprano::Statement st = toStatement(sp.data());
            if (st.isValid()) {
                d->model->addStatement(st);
            }
        }
    }
    RDEBUG << "done";
}

void KoDocumentRdf::emitSemanticObjectAdded(hKoRdfBasicSemanticItem item) const
{
    emit semanticObjectAdded(item);
}

void KoDocumentRdf::emitSemanticObjectAddedConst(const hKoRdfBasicSemanticItem item) const
{
    emit semanticObjectAdded(item);
}

void KoDocumentRdf::emitSemanticObjectUpdated(hKoRdfBasicSemanticItem item)
{
    if (item && !KoRdfSemanticItemRegistry::instance()->isBasic(item->className())) {
        //
        // reflow the formatting for each view of the semanticItem, in reverse document order
        //
        QMap<int, reflowItem> col;
	hKoRdfSemanticItem si(static_cast<KoRdfSemanticItem *>(item.data()));
        RDEBUG << "xmlids:" << item->xmlIdList() << " reflow item:" << si->name();
        insertReflow(col, si);
        applyReflow(col);
    }
    emit semanticObjectUpdated(item);
}

void KoDocumentRdf::emitSemanticObjectViewSiteUpdated(hKoRdfBasicSemanticItem baseItem, const QString &xmlid)
{
    if (KoRdfSemanticItemRegistry::instance()->isBasic(baseItem->className())) {
        return;
    }

    hKoRdfSemanticItem item(static_cast<KoRdfSemanticItem *>(baseItem.data()));
    if (item) {
	RDEBUG << "xmlid:" << xmlid << " reflow item:" << item->name();
	emit semanticObjectViewSiteUpdated(item, xmlid);
    }
}


bool KoDocumentRdf::completeLoading(KoStore *)
{
    return true;
}

bool KoDocumentRdf::completeSaving(KoStore *, KoXmlWriter *, KoShapeSavingContext *)
{
    return true;
}

KoDocumentRdf::reflowItem::reflowItem(hKoRdfSemanticItem si, const QString &xmlid, hKoSemanticStylesheet ss, const QPair< int, int > &extent)
        : m_si(si)
        , m_ss(ss)
        , m_xmlid(xmlid)
        , m_extent(extent)
{
}

void KoDocumentRdf::insertReflow(QMap<int, reflowItem> &col, hKoRdfSemanticItem obj, hKoSemanticStylesheet ss)
{
    RDEBUG << "reflowing object:" << obj->name();
    QStringList xmlidlist = obj->xmlIdList();
    foreach (const QString &xmlid, xmlidlist) {
        QPair< int, int > extent = findExtent(xmlid);
        RDEBUG << "format(), adding reflow xmlid location:" << xmlid << " extent:" << extent;
        reflowItem item(obj, xmlid, ss, extent);
        col.insert(extent.first, item);
    }
}

void KoDocumentRdf::insertReflow(QMap<int, reflowItem> &col, hKoRdfSemanticItem obj,
                                 const QString &sheetType, const QString &stylesheetName)
{
    hKoSemanticStylesheet ss = obj->findStylesheetByName(sheetType, stylesheetName);
    insertReflow(col, obj, ss);
}

void KoDocumentRdf::insertReflow(QMap<int, reflowItem> &col, hKoRdfSemanticItem obj)
{
    RDEBUG << "reflowing object:" << obj->name();
    foreach (const QString &xmlid, obj->xmlIdList()) {
        QPair<int, int> extent = findExtent(xmlid);
        RDEBUG << "format(), adding reflow xmlid location:" << xmlid << " extent:" << extent;
        reflowItem item(obj, xmlid, hKoSemanticStylesheet(0), extent);
        col.insert(extent.first, item);
    }
}

void KoDocumentRdf::applyReflow(const QMap<int, reflowItem> &col)
{
    QMapIterator< int, reflowItem > i(col);
    i.toBack();
    while (i.hasPrevious()) {
        const reflowItem &item = i.previous().value();
        RDEBUG << "format(), extent:" << item.m_extent;
        RDEBUG << "xmlid:" << item.m_xmlid;
        RDEBUG << "format(), semitem:" << item.m_si.constData();
        RDEBUG << "format(), semitem.name:" << item.m_si->name();
        if (item.m_ss) {
            KoRdfSemanticItemViewSite vs(item.m_si, item.m_xmlid);
            vs.setStylesheetWithoutReflow(item.m_ss);
        }
        emitSemanticObjectViewSiteUpdated(item.m_si, item.m_xmlid);
    }
}

QList<hKoSemanticStylesheet> KoDocumentRdf::userStyleSheetList(const QString& className) const
{
    return d->userStylesheets[className];
}

void KoDocumentRdf::setUserStyleSheetList(const QString& className,const QList<hKoSemanticStylesheet>& l)
{
    d->userStylesheets[className] = l;
}

bool KoDocumentRdf::backendIsSane()
{
    const Soprano::Backend *backend = Soprano::discoverBackendByFeatures(
            Soprano::BackendFeatureContext |
            Soprano::BackendFeatureQuery |
            Soprano::BackendFeatureStorageMemory);
    if (!backend) {
        // without a backend with the desired features, this test fails
        kWarning() << "No suitable backend found.";
        return false;
    }
    kWarning() << "Found a backend: " << backend->pluginName();

    Soprano::setUsedBackend(backend);
    Soprano::BackendSettings backendSettings;
    backendSettings << Soprano::BackendOptionStorageMemory;
    Soprano::StorageModel* model = backend->createModel(backendSettings);
    if (!model) {
        // if model creation failed, this test fails
        kWarning() << "No model could be created.";
        return false;
    }

    model->addStatement(Soprano::Statement(
            QUrl("subject"), QUrl("predicate"), QUrl("object"),
            QUrl("context")));

    Soprano::QueryResultIterator it = model->executeQuery(
            "SELECT ?g ?s ?p ?o WHERE { GRAPH ?g { ?s ?p ?o } }",
            Soprano::Query::QueryLanguageSparql);

    if (!it.next()) {
        // there should be exactly one result statement
        kWarning() << "Query returned too few results.";
        delete model;
        return false;
    }
    if (it.next()) {
        // there should be exactly one result statement
        kWarning() << "Query returned too many results.";
        delete model;
        return false;
    }

    delete model;
    return true;

}

QStringList KoDocumentRdf::idrefList() const
{
    Q_ASSERT(d->model);
    QStringList idrefs;

    StatementIterator it = model()->listStatements(
                               Node(),
                               Node(QUrl("http://docs.oasis-open.org/ns/office/1.2/meta/pkg#idref")),
                               Node(),
                               Node());
    if (!it.isValid()) {
        return idrefs;
    }

    QList<Statement> allStatements = it.allElements();
    foreach (const Soprano::Statement &s, allStatements) {
        idrefs << s.object().toString();
    }
    return idrefs;
}
