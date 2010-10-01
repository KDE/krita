/* This file is part of the KDE project
   Copyright (C) 2010 KO GmbH <ben.martin@kogmbh.com>

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
#include "KoDocumentRdf_p.h"
#include "KoRdfPrefixMapping.h"
#include "RdfSemanticTreeWidgetSelectAction.h"

#include "../KoView.h"
#include "../KoDocument.h"
#include <KoToolManager.h>
#include <KoTextDocument.h>
#include <KoTextRdfCore.h>
#include "KoOdfWriteStore.h"
#include <KoXmlReader.h>
#include <KoXmlWriter.h>
#include <KoStoreDevice.h>
#include <KoCanvasBase.h>
#include <KoToolProxy.h>
#include <KoResourceManager.h>
#include <KoTextEditor.h>
#include <KoInlineObject.h>
#include <KoTextInlineRdf.h>
#include <KoInlineTextObjectManager.h>
#include <KoBookmark.h>
#include <KoTextMeta.h>

#include <kconfig.h>
#include <kdebug.h>
#include <kglobal.h>
#include <klocale.h>
#include <kuser.h>

#ifdef DEBUG_RDF
#define RDEBUG kDebug(30015)
#else
#define RDEBUG if(0) kDebug(30015)
#endif

using namespace Soprano;

KoDocumentRdfPrivate::KoDocumentRdfPrivate()
        : model(Soprano::createModel())
        , prefixMapping(0)
{
}

KoDocumentRdfPrivate::~KoDocumentRdfPrivate()
{
    delete prefixMapping;
    delete model;
}


KoDocumentRdf::KoDocumentRdf(KoDocument *parent)
        : KoDocumentRdfBase(parent)
        , d (new KoDocumentRdfPrivate())
{
    d->prefixMapping = new KoRdfPrefixMapping(this);
}

KoDocumentRdf::~KoDocumentRdf()
{
    RDEBUG;
    delete d;
}

Soprano::Model *KoDocumentRdf::model() const
{
    return d->model;
}

KoDocumentRdf *KoDocumentRdf::fromResourceManager(KoCanvasBase *host)
{
    KoResourceManager *rm = host->resourceManager();
    if (!rm->hasResource(KoText::DocumentRdf)) {
        return 0;
    }
    return static_cast<KoDocumentRdf*>(rm->resource(KoText::DocumentRdf).value<void*>());
}


KoDocument *KoDocumentRdf::document() const
{
    return qobject_cast<KoDocument *>(parent());
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
    return Node(QUrl("http://www.koffice.org/Rdf/inline-rdf"));
}

QString KoDocumentRdf::rdfInternalMetadataWithoutSubjectURI() const
{
    return "http://www.koffice.org/Rdf/internal/content.xml";
}

QString KoDocumentRdf::rdfPathContextPrefix() const
{
    return "http://www.koffice.org/Rdf/path/";
}

Soprano::Node KoDocumentRdf::manifestRdfNode() const
{
    return Node(QUrl(rdfPathContextPrefix() + "manifest.rdf"));
}

void KoDocumentRdf::freshenBNodes(Soprano::Model *m)
{
    Q_ASSERT(d->model);
    QList<Soprano::Statement> removeList;
    QList<Soprano::Statement> addList;
    QMap<QString, Soprano::Node> bnodeMap;
    StatementIterator it = m->listStatements();
    QList<Statement> allStatements = it.allElements();
    RDEBUG << "freshening model.sz:" << allStatements.size();
    foreach (Soprano::Statement s, allStatements) {
        Soprano::Node subj = s.subject();
        Soprano::Node obj = s.object();
        Soprano::Statement news;
        if (subj.type() == Soprano::Node::BlankNode) {
            QString nodeStr = subj.toString();
            Soprano::Node n = bnodeMap[ nodeStr ];
            if (!n.isValid()) {
                n = d->model->createBlankNode();
                bnodeMap[ nodeStr ] = n;
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
    Soprano::Model *tmpmodel(Soprano::createModel());
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
    Soprano::Node context(QUrl(rdfPathContextPrefix() + fileName));
    QUrl BaseURI = QUrl(QString());
    QString rdfxmlData(store->device()->readAll());
    Soprano::StatementIterator it = parser->parseString(rdfxmlData, BaseURI,
                                    Soprano::SerializationRdfXml);
    QList<Statement> allStatements = it.allElements();
    RDEBUG << "Found " << allStatements.size() << " triples..." << endl;
    foreach (Soprano::Statement s, allStatements) {
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

        QStringList classNames = KoRdfSemanticItem::classNames();
        foreach (const QString &klass, classNames) {
            KoRdfSemanticItem *si = KoRdfSemanticItem::createSemanticItem(this, this, klass);
            si->loadUserStylesheets(d->model);
        }
    }
    delete tmpmodel;
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
        QString sparqlQuery = "prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
          "prefix odf: <http://docs.oasis-open.org/opendocument/meta/package/odf#> \n"
          "prefix odfcommon: <http://docs.oasis-open.org/opendocument/meta/package/common#> \n"
          "select ?subj ?fileName \n"
          " where { \n"
          "  ?subj rdf:type odf:MetaDataFile . \n"
          "  ?subj odfcommon:path ?fileName  \n"
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
            if (!ok) break;
        }
    }
    return ok;
}

bool KoDocumentRdf::saveRdf(KoStore *store, KoXmlWriter *manifestWriter, Soprano::Node &context)
{
    bool ok = false;
    QString fileName("manifest.rdf");
    if (context.toString() == inlineRdfContext().toString()) {
        RDEBUG << "found some internal Rdf, this is handled by augmenting the DOM";
        return true;
    }
    if (!d->model) {
        kWarning(30003) << "No soprano model";
        return false;
    }
    //
    // The context contains the filename to save into
    //
    if (context.toString().startsWith(rdfPathContextPrefix())) {
        fileName = context.toString().mid(rdfPathContextPrefix().size());
    }
    RDEBUG << "saving external file:" << fileName;
    if (!store->open(fileName)) {
        document()->setErrorMessage(
            i18n("Not able to write '%1'. Partition full?", (fileName)));
        return false;
    }
    KoStoreDevice dev(store);
    QTextStream oss(&dev);
    if (fileName == "manifest.rdf" && d->prefixMapping) {
        d->prefixMapping->save(d->model, context);

        QStringList classNames = KoRdfSemanticItem::classNames();
        foreach (const QString &klass, classNames) {
            KoRdfSemanticItem *si = KoRdfSemanticItem::createSemanticItem(this, this, klass);
            si->saveUserStylesheets(d->model, context);
        }
    }
    Soprano::StatementIterator triples = d->model->listStatements(Soprano::Node(),
            Soprano::Node(), Soprano::Node(), context);
    QString serialization("application/rdf+xml");
    const Soprano::Serializer *serializer = Soprano::PluginManager::instance()->
        discoverSerializerForSerialization(Soprano::SerializationRdfXml);
    if (serializer) {
        QString data;
        QTextStream tss(&data);
        if (serializer->serialize(triples, tss, Soprano::SerializationRdfXml)) {
            tss.flush();
            oss << data;
            RDEBUG << "fileName:" << fileName << " data.sz:" << data.size();
            RDEBUG << "model.sz:" << d->model->statementCount();
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
    NodeIterator contextier = d->model->listContexts();
    QList<Node> contexts = contextier.allElements();
    foreach (Soprano::Node n, contexts) {
        saveRdf(store, manifestWriter, n);
    }
    return ok;
}

void KoDocumentRdf::updateXmlIdReferences(const QMap<QString, QString> &m)
{
    RDEBUG << "KoDocumentRdf::updateXmlIdReferences() m.size:" << m.size();
    Q_ASSERT(d->model);

    QList<Soprano::Statement> removeList;
    QList<Soprano::Statement> addList;
    StatementIterator it = d->model->listStatements(
                               Node(),
                               Node(QUrl("http://docs.oasis-open.org/opendocument/meta/package/common#idref")),
                               Node(),
                               Node());
    QList<Statement> allStatements = it.allElements();
    foreach (Soprano::Statement s, allStatements) {
        RDEBUG << "seeking obj:" << s.object();
        QMap<QString, QString>::const_iterator mi = m.find(s.object().toString());
        if (mi != m.end()) {
            QString oldID = mi.key();
            QString newID = mi.value();
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
            }
        }
    }
    // out with the old, in with the new
    // remove & add the triple lists.
    RDEBUG << "addStatements.size:" << addList.size();
    RDEBUG << " remove.size:" << removeList.size();
    KoTextRdfCore::removeStatementsIfTheyExist(d->model, removeList);
    d->model->addStatements(addList);
}

QList<KoRdfFoaF*> KoDocumentRdf::foaf(Soprano::Model *m)
{
    if (!m) {
        m = d->model;
        Q_ASSERT(m);
    }
    QString sparqlQuery = "prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
                          "prefix foaf: <http://xmlns.com/foaf/0.1/> \n"
                          "prefix pkg: <http://docs.oasis-open.org/opendocument/meta/package/common#> \n"
                          "select distinct ?graph ?person ?name ?nick ?homepage ?img ?phone \n"
                          "where { \n"
                          "  GRAPH ?graph { \n"
                          "    ?person rdf:type foaf:Person . \n"
                          "    ?person foaf:name ?name \n"
                          "    OPTIONAL { ?person foaf:phone ?phone } \n"
                          "    OPTIONAL { ?person foaf:nick ?nick } \n"
                          "    OPTIONAL { ?person foaf:homepage ?homepage } \n"
                          "    OPTIONAL { ?person foaf:img ?img } \n"
                          "    }\n"
                          "}\n";
    Soprano::QueryResultIterator it =
        m->executeQuery(sparqlQuery,
                        Soprano::Query::QueryLanguageSparql);
    RDEBUG << "1 model().sz:" << d->model->statementCount() << " m.sz:" << m->statementCount();
    // lastKnownObjects is used to perform a sematic set diff
    // at return time d->foafObjects will have any new objects and
    // ones that are no longer available will be removed.
    QList<KoRdfFoaF*> lastKnownObjects = d->foafObjects;
    // uniqfilter is needed because soprano is not honouring
    // the DISTINCT sparql keyword
    QSet<QString> uniqfilter;
    while (it.next()) {
        QString n = it.binding("name").toString();
        RDEBUG << "n:" << n;
        if (uniqfilter.contains(n))
            continue;
        uniqfilter += n;

        KoRdfFoaF *newItem = new KoRdfFoaF(this, this, it);
        QString newItemLs = newItem->linkingSubject().toString();
        foreach (KoRdfFoaF *semItem, lastKnownObjects) {
            if (newItemLs == semItem->linkingSubject().toString()) {
                lastKnownObjects.removeAll(semItem);
                delete newItem;
                newItem = 0;
                break;
            }
        }
        if (newItem) {
            d->foafObjects << newItem;
        }
    }
    foreach (KoRdfFoaF *semItem, lastKnownObjects) {
        d->foafObjects.removeAll(semItem);
    }
    RDEBUG << "foaf() size:" << d->foafObjects.size() << endl;
#ifndef NDEBUG
    if (d->foafObjects.empty() && m->statementCount()) {
      RDEBUG << "foaf() have data, but no foafs!" << endl;
        QList<Statement> allStatements = m->listStatements().allElements();
        foreach (Soprano::Statement s, allStatements) {
          RDEBUG << s;
        }
    }
#endif
    return d->foafObjects;
}

QList<KoRdfCalendarEvent*> KoDocumentRdf::calendarEvents(Soprano::Model *m)
{
    if (!m) {
        m = d->model;
        Q_ASSERT(m);
    }
    QString sparqlQuery = " prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
                          " prefix foaf: <http://xmlns.com/foaf/0.1/>  \n"
                          " prefix cal:  <http://www.w3.org/2002/12/cal/icaltzd#>  \n"
                          " select distinct ?graph ?ev ?uid ?dtstart ?dtend ?summary ?location ?geo ?long ?lat \n"
                          " where {  \n"
                          "  GRAPH ?graph { \n"
                          "    ?ev rdf:type cal:Vevent . \n"
                          "    ?ev cal:uid      ?uid . \n"
                          "    ?ev cal:dtstart  ?dtstart . \n"
                          "    ?ev cal:dtend    ?dtend \n"
                          "    OPTIONAL { ?ev cal:summary  ?summary  } \n"
                          "    OPTIONAL { ?ev cal:location ?location } \n"
                          "    OPTIONAL {  \n"
                          "               ?ev cal:geo ?geo \n"
                          "               ?geo rdf:first ?lat \n"
                          "               ?geo rdf:rest ?joiner \n"
                          "               ?joiner rdf:first ?long \n"
                          "              } \n"
                          "    } \n"
                          "  } \n";
    Soprano::QueryResultIterator it =
        m->executeQuery(sparqlQuery,
                        Soprano::Query::QueryLanguageSparql);
    QList<KoRdfCalendarEvent*> lastKnownObjects = d->calObjects;
    // uniqfilter is needed because soprano is not honouring
    // the DISTINCT sparql keyword
    QSet<QString> uniqfilter;
    while (it.next()) {
        QString n = it.binding("uid").toString();
        if (uniqfilter.contains(n))
            continue;
        uniqfilter += n;
        RDEBUG << " g:" << it.binding("g").toString();
        RDEBUG << " uid:" << it.binding("uid").toString();

        KoRdfCalendarEvent *newItem(new KoRdfCalendarEvent(this, this, it));
        QString newItemLs = newItem->linkingSubject().toString();
        foreach (KoRdfCalendarEvent *semItem, lastKnownObjects) {
            if (newItemLs == semItem->linkingSubject().toString()) {
                lastKnownObjects.removeAll(semItem);
                delete newItem;
                newItem = 0;
                break;
            }
        }
        if (newItem) {
            d->calObjects << newItem;
        }
    }
    foreach (KoRdfCalendarEvent *semItem, lastKnownObjects) {
        d->calObjects.removeAll(semItem);
    }
    RDEBUG << "calendarEvents() size:" << d->calObjects.size() << endl;
    return d->calObjects;
}

/**
 * The redland library is used for in memory Rdf by Soprano. Unfortunately
 * the distinct keyword doesn't always do what it should so a postprocess
 * has to be applied in some cases to ensure DISTINCT semantics in the results.
 */
struct SparqlDistinctPostprocess {
    QStringList m_bindingsThatMakeID;
    QSet<QString> m_uniqfilter;

    SparqlDistinctPostprocess(QString bindingForID) {
        m_bindingsThatMakeID << bindingForID;
    }
    bool shouldSkip(Soprano::QueryResultIterator it) {
        QString ID = uniqueID(it);
        bool ret = m_uniqfilter.contains(ID);
        m_uniqfilter << ID;
        return ret;
    }
    void addBindingToKeySet(QString n) {
        m_bindingsThatMakeID << n;
    }
protected:
    QString uniqueID(Soprano::QueryResultIterator it) {
        QString ret;

        foreach (const QString &b, m_bindingsThatMakeID) {
            QString n = it.binding(b).toString();
            ret += n;
        }
        return ret;
    }
};


void KoDocumentRdf::addLocations(Soprano::Model *m, QList<KoRdfLocation*> &ret, bool isGeo84,
                                 const QString &sparqlQuery)
{
    Soprano::QueryResultIterator it = m->executeQuery(sparqlQuery,
                        Soprano::Query::QueryLanguageSparql);
    SparqlDistinctPostprocess uniqFilter("lat");
    uniqFilter.addBindingToKeySet("long");
    while (it.next()) {
        if (uniqFilter.shouldSkip(it))
            continue;

        KoRdfLocation *semObj(new KoRdfLocation(this, this, it, isGeo84));
        ret << semObj;
    }
    RDEBUG << "addLocations() size:" << ret.size() << endl;
}

QList<KoRdfLocation*> KoDocumentRdf::locations(Soprano::Model *m)
{
    if (!m) {
        m = d->model;
        Q_ASSERT(m);
    }
    RDEBUG << "locations(top) full-model.sz:" << d->model->statementCount();
    RDEBUG << " passed model.size:" << m->statementCount();
    QList<KoRdfLocation*> currentKoRdfLocations;
    addLocations(m, currentKoRdfLocations, false,
        " prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#>  \n"
        " prefix foaf: <http://xmlns.com/foaf/0.1/>  \n"
        " prefix cal:  <http://www.w3.org/2002/12/cal/icaltzd#>  \n"
        " select distinct ?graph ?geo ?long ?lat ?joiner \n"
        " where {  \n"
        "  GRAPH ?graph { \n"
        "               ?ev cal:geo ?geo . \n"
        "               ?geo rdf:first ?lat . \n"
        "               ?geo rdf:rest ?joiner . \n"
        "               ?joiner rdf:first ?long \n"
        "               } \n"
        "  } \n");
    RDEBUG << "locations(1) currentKoRdfLocations.size:" << currentKoRdfLocations.size();
    addLocations(m, currentKoRdfLocations, true,
        " prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
        " prefix foaf: <http://xmlns.com/foaf/0.1/>  \n"
        " prefix geo84: <http://www.w3.org/2003/01/geo/wgs84_pos#> \n"
        "  \n"
        " select ?graph ?geo ?long ?lat ?type \n"
        " where {  \n"
        "  GRAPH ?graph { \n"
        "  \n"
        "        ?geo geo84:lat  ?lat . \n"
        "        ?geo geo84:long ?long \n"
        "        OPTIONAL { ?geo rdf:type ?type } \n"
        "  \n"
        "  } \n"
        " } \n");
    // add the new, remove the no longer existing between locObjects and currentKoRdfLocations.
    // The semantic items have a lifetime of this KoDocumentRDF.
    // If we could use smart pointers then we could just return the new list of locations,
    // As semantic items have a lifetime of this KoDocumentRDF,
    // we don't want to create any more than are needed.
    //
    // As currentKoRdfLocations contains all the location semitems we have found to be valid,
    // we need to transfer any new ones from that list to locObjects and delete what
    // remains (which are objects that existed in locObjects before and were rediscovered
    // during the query process).
    //
    // Creating a list of locations each time similifies the query and discovery process
    // at the expense of this little mess to merge the new and old with explicit pointer
    // and object lifetime handling
    QList<KoRdfLocation*> removeSet;
    foreach (KoRdfLocation *oldItem, d->locObjects) {
        QString oldItemLs = oldItem->linkingSubject().toString();
        bool found = false;
        foreach (KoRdfLocation *newItem, currentKoRdfLocations) {
            if (oldItemLs == newItem->linkingSubject().toString()) {
                found = true;
                break;
            }
        }
        if (!found) {
            removeSet << oldItem;
        }
    }
    foreach (KoRdfLocation *item, removeSet) {
        d->locObjects.removeAll(item);
    }
    QList<KoRdfLocation*> addedSet;
    foreach (KoRdfLocation *newItem, currentKoRdfLocations) {
        QString newItemLs = newItem->linkingSubject().toString();
        bool found = false;
        foreach (KoRdfLocation *oldItem, d->locObjects) {
            if (newItemLs == oldItem->linkingSubject().toString()) {
                found = true;
                break;
            }
        }
        if (!found) {
            d->locObjects << newItem;
            addedSet << newItem;
        }
    }
    foreach (KoRdfLocation *item, addedSet) {
        currentKoRdfLocations.removeAll(item);
    }
    RDEBUG << "locations(end) deleting duplicates size:" << currentKoRdfLocations.size() << endl;
    qDeleteAll(currentKoRdfLocations);
    RDEBUG << "locations(end) size:" << d->locObjects.size() << endl;
    return d->locObjects;
}

void KoDocumentRdf::dumpModel(const QString &msg, Soprano::Model *m) const
{
    if (!m) {
        return;
    }
    QList<Soprano::Statement> allStatements = m->listStatements().allElements();
    RDEBUG << "----- " << msg << " ----- model size:" << allStatements.size();
    foreach (Soprano::Statement s, allStatements) {
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
    Soprano::Statement ret(subj, pred, obj, inlineRdfContext());
    return ret;
}

void KoDocumentRdf::addStatements(Soprano::Model *model, const QString &xmlid)
{
    Q_ASSERT(model);
    Q_ASSERT(d->model);
    QString sparqlQuery;
    QTextStream queryss(&sparqlQuery);
    RDEBUG << "addStatements model.sz:" << d->model->statementCount() << " xmlid:" << xmlid;
    queryss << "prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
        << "prefix foaf: <http://xmlns.com/foaf/0.1/> \n"
        << "prefix pkg:  <http://docs.oasis-open.org/opendocument/meta/package/common#> \n"
        << ""
        << "select ?s ?p ?o \n"
        << "where { \n"
        << " ?s pkg:idref ?xmlid . \n"
        << " ?s ?p ?o . \n"
        << " filter( str(?xmlid) = \"" << xmlid << "\" ) \n"
        << "}\n";
    queryss.flush();
    RDEBUG << "sparql:" << sparqlQuery;
    Soprano::QueryResultIterator it = d->model->executeQuery(sparqlQuery,
                              Soprano::Query::QueryLanguageSparql);
    while (it.next()) {
        Statement s(it.binding("s"),
                    it.binding("p"),
                    it.binding("o"));
        model->addStatement(s);
        RDEBUG << "result, s:" << it.binding("s");
        RDEBUG << " p:" << it.binding("p");
        RDEBUG << " o:" << it.binding("o");
    }
}

void KoDocumentRdf::expandStatementsReferencingSubject(Soprano::Model *model)
{
    Q_ASSERT(model);
    Q_ASSERT(d->model);
    QList<Statement> addList;
    QList<Statement> allStatements = model->listStatements().allElements();
    foreach (Soprano::Statement s, allStatements) {
        QList<Statement> all = d->model->listStatements(Node(), Node(), s.subject()).allElements();
        foreach (Soprano::Statement e, all) {
            addList << e;
        }
    }
    model->addStatements(addList);
}

void KoDocumentRdf::expandStatementsSubjectPointsTo(Soprano::Model *model)
{
    Q_ASSERT(model);
    Q_ASSERT(d->model);
    QList<Statement> addList;
    QList<Statement> allStatements = model->listStatements().allElements();
    foreach (Soprano::Statement s, allStatements) {
        QList<Statement> all = d->model->listStatements(s.object(), Node(), Node()).allElements();
        foreach (Soprano::Statement e, all) {
            RDEBUG << "ADD obj:" << s.object() << " adding:" << e;
            addList << e;
        }
    }
    model->addStatements(addList);
}

void KoDocumentRdf::expandStatementsSubjectPointsTo(Soprano::Model *model, const Soprano::Node &n)
{
    Q_ASSERT(model);
    Q_ASSERT(d->model);
    QList<Statement> addList;
    QList<Statement> all = d->model->listStatements(n, Node(), Node()).allElements();
    foreach (Soprano::Statement e, all) {
        RDEBUG << "n:" << n << " adding:" << e;
        addList << e;
    }
    model->addStatements(addList);
}

void KoDocumentRdf::expandStatementsToIncludeRdfListsRecurse(Soprano::Model *model,
        QList<Statement> &addList, const Soprano::Node &n)
{
    Q_ASSERT(model);
    Q_ASSERT(d->model);
    Node rdfFirst = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#first"));
    Node rdfRest  = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"));
    QList<Statement> all;
    all = d->model->listStatements(n, rdfFirst, Node()).allElements();
    addList << all;
    all = d->model->listStatements(n, rdfRest, Node()).allElements();
    addList << all;
    foreach (Soprano::Statement s, all) {
        expandStatementsToIncludeRdfListsRecurse(model, addList, s.object());
    }
}


void KoDocumentRdf::expandStatementsToIncludeRdfLists(Soprano::Model *model)
{
    Q_ASSERT(model);
    RDEBUG << "model.sz:" << model->statementCount();
    QList<Statement> addList;
    QList<Statement> allStatements = model->listStatements().allElements();
    foreach (Soprano::Statement s, allStatements) {
        expandStatementsToIncludeRdfListsRecurse(model, addList, s.subject());
    }
    RDEBUG << "model.sz:" << model->statementCount();
    RDEBUG << "addList.sz:" << addList.size();
    model->addStatements(addList);
}

void KoDocumentRdf::expandStatementsToIncludeOtherPredicates(Soprano::Model *model)
{
    Q_ASSERT(model);
    Q_ASSERT(d->model);
    QList<Statement> addList;
    QList<Statement> allStatements = model->listStatements().allElements();
    foreach (Soprano::Statement s, allStatements) {
        QList<Statement> all = d->model->listStatements(s.subject(), Node(), Node()).allElements();
        foreach (Soprano::Statement e, all) {
            addList << e;
        }
    }
    model->addStatements(addList);
}

void KoDocumentRdf::expandStatements(Soprano::Model *model)
{
    Q_ASSERT(model);
    expandStatementsReferencingSubject(model);
    expandStatementsToIncludeOtherPredicates(model);
}

KAction *KoDocumentRdf::createInsertSemanticObjectReferenceAction(KoCanvasBase *host)
{
    KAction *ret = new InsertSemanticObjectReferenceAction(host, this, i18n("Reference"));
    RDEBUG << "createInsertSemanticObjectReferenceAction";
    return ret;
}

QList<KAction*> KoDocumentRdf::createInsertSemanticObjectNewActions(KoCanvasBase *host)
{
    QList<KAction*> ret;
    foreach (const QString &klass,  KoRdfSemanticItem::classNames()) {
        ret.append(new InsertSemanticObjectCreateAction(host, this, klass));
    }
    return ret;
}

QPair<int, int> KoDocumentRdf::findExtent(const QString &xmlid)
{
    KoTextInlineRdf *obj = findInlineRdfByID(xmlid);
    if (obj) {
        QPair<int, int> ret = obj->findExtent();
        RDEBUG << "have inline obj, extent:" << ret;
        return ret;
    }
    return QPair<int, int>(0, 0);
}

QPair<int, int> KoDocumentRdf::findExtent(QTextCursor &cursor)
{
    Q_ASSERT(d->model);
    QPair<int, int> ret(0, 0);
    RDEBUG << "model.sz:" << d->model->statementCount();
    //
    // Look backwards for enclosing text:meta and bookmark-start tags
    //
    if (KoInlineTextObjectManager *textObjectManager
            = KoTextDocument(cursor.document()).inlineTextObjectManager()) {
        long searchStartPosition = cursor.position();
        int limit = 500;
        for (QTextCursor tc = cursor;
                !tc.atStart() && limit;
                tc.movePosition(QTextCursor::Left), --limit) {
            KoInlineObject *inlineObject = textObjectManager->inlineTextObject(tc);
            if (inlineObject) {
                if (KoBookmark *bm = dynamic_cast<KoBookmark*>(inlineObject)) {
                    if (bm->type() == KoBookmark::EndBookmark) {
                        continue;
                    }
                    if (bm->type() == KoBookmark::StartBookmark) {
                        KoBookmark *e = bm->endBookmark();
                        if (e && e->position() < searchStartPosition)
                            continue;
                        else
                            return QPair<int, int>(bm->position(), e->position());
                    }
                }
                if (KoTextMeta *bm = dynamic_cast<KoTextMeta*>(inlineObject)) {
                    if (bm->type() == KoTextMeta::EndBookmark) {
                        continue;
                    }
                    if (bm->type() == KoTextMeta::StartBookmark) {
                        KoTextMeta *e = bm->endBookmark();
                        if (e && e->position() < searchStartPosition)
                            continue;
                        else
                            return QPair<int, int>(bm->position(), e->position());
                    }
                }
            }
        }
    }
    return ret;
}

QPair<int, int> KoDocumentRdf::findExtent(KoTextEditor *handler)
{
    Q_ASSERT(d->model);
    QPair<int, int> ret(0, 0);
    RDEBUG << "model.sz:" << d->model->statementCount();
    //
    // Look backwards for enclosing text:meta and bookmark-start tags
    //
    if (KoInlineTextObjectManager *textObjectManager
            = KoTextDocument(handler->document()).inlineTextObjectManager()) {
        long searchStartPosition = handler->position();
        KoTextEditor tc(handler->document());
        tc.setPosition(handler->position());
        for (int limit = 500; !tc.atStart() && limit;
                tc.movePosition(QTextCursor::Left), --limit) {
            QTextCursor qtc(handler->document());
            qtc.setPosition(tc.position());
            KoInlineObject *inlineObject = textObjectManager->inlineTextObject(qtc);
            if (inlineObject) {
                if (KoBookmark *bm = dynamic_cast<KoBookmark*>(inlineObject)) {
                    if (bm->type() == KoBookmark::EndBookmark) {
                        continue;
                    }
                    if (bm->type() == KoBookmark::StartBookmark) {
                        KoBookmark *e = bm->endBookmark();
                        if (e && e->position() < searchStartPosition)
                            continue;
                        else
                            return QPair<int, int>(bm->position(), e->position());
                    }
                }
                if (KoTextMeta *bm = dynamic_cast<KoTextMeta*>(inlineObject)) {
                    if (bm->type() == KoTextMeta::EndBookmark) {
                        continue;
                    }
                    if (bm->type() == KoTextMeta::StartBookmark) {
                        KoTextMeta *e = bm->endBookmark();
                        if (e && e->position() < searchStartPosition)
                            continue;
                        else
                            return QPair<int, int>(bm->position(), e->position());
                    }
                }
            }
        }
    }
    return ret;
}

QString KoDocumentRdf::findXmlId(KoTextEditor *handler)
{
    QString ret;
    KoTextInlineRdf *inlineRdf(0);
    //
    // Look backwards for enclosing text:meta and bookmark-start tags
    //
    if (KoInlineTextObjectManager *textObjectManager
            = KoTextDocument(handler->document()).inlineTextObjectManager()) {
        long searchStartPosition = handler->position();
        KoTextEditor tc(handler->document());
        tc.setPosition(handler->position());
        for (int limit = 500; !tc.atStart() && limit;
                tc.movePosition(QTextCursor::Left), --limit) {
            QTextCursor qtc(handler->document());
            qtc.setPosition(tc.position());
            KoInlineObject *inlineObject = textObjectManager->inlineTextObject(qtc);
            if (inlineObject) {
                if (KoBookmark *bm = dynamic_cast<KoBookmark*>(inlineObject)) {
                    if (bm->type() == KoBookmark::EndBookmark) {
                        continue;
                    }
                    if (bm->type() == KoBookmark::StartBookmark) {
                        KoBookmark *e = bm->endBookmark();
                        if (e && e->position() < searchStartPosition)
                            continue;
                    }
                }
                if (KoTextMeta *bm = dynamic_cast<KoTextMeta*>(inlineObject)) {
                    if (bm->type() == KoTextMeta::EndBookmark) {
                        continue;
                    }
                    if (bm->type() == KoTextMeta::StartBookmark) {
                        KoTextMeta *e = bm->endBookmark();
                        if (e && e->position() < searchStartPosition)
                            continue;
                    }
                }
                if (KoInlineObject *shape = dynamic_cast<KoInlineObject*>(inlineObject)) {
                    RDEBUG << "have KoInlineObject at:" <<  tc.position();
                    inlineRdf = shape->inlineRdf();
                    if (inlineRdf) {
                        break;
                    }
                }
            }
        }
    }
    if (!inlineRdf) {
        inlineRdf = KoTextInlineRdf::tryToGetInlineRdf(handler);
    }
    if (inlineRdf) {
        return inlineRdf->xmlId();
    }
    return ret;
}


QString KoDocumentRdf::findXmlId(QTextCursor &cursor)
{
    QString ret;
    KoTextInlineRdf *inlineRdf(0);
    //
    // Look backwards for enclosing text:meta and bookmark-start tags
    //
    if (KoInlineTextObjectManager *textObjectManager
            = KoTextDocument(cursor.document()).inlineTextObjectManager()) {
        long searchStartPosition = cursor.position();
        int limit = 500;
        for (QTextCursor tc = cursor; !tc.atStart() && limit;
                tc.movePosition(QTextCursor::Left), --limit) {
            KoInlineObject *inlineObject = textObjectManager->inlineTextObject(tc);
            if (inlineObject) {
                if (KoBookmark *bm = dynamic_cast<KoBookmark*>(inlineObject)) {
                    if (bm->type() == KoBookmark::EndBookmark) {
                        continue;
                    }
                    if (bm->type() == KoBookmark::StartBookmark) {
                        KoBookmark *e = bm->endBookmark();
                        if (e && e->position() < searchStartPosition)
                            continue;
                    }
                }
                if (KoTextMeta *bm = dynamic_cast<KoTextMeta*>(inlineObject)) {
                    if (bm->type() == KoTextMeta::EndBookmark) {
                        continue;
                    }
                    if (bm->type() == KoTextMeta::StartBookmark) {
                        KoTextMeta *e = bm->endBookmark();
                        if (e && e->position() < searchStartPosition)
                            continue;
                    }
                }
                if (KoInlineObject *shape = dynamic_cast<KoInlineObject*>(inlineObject)) {
                    RDEBUG << "have KoInlineObject at:" <<  tc.position();
                    inlineRdf = shape->inlineRdf();
                    if (inlineRdf) {
                        break;
                    }
                }
            }
        }
    }
    if (!inlineRdf) {
        inlineRdf = KoTextInlineRdf::tryToGetInlineRdf(cursor);
    }
    if (inlineRdf) {
        return inlineRdf->xmlId();
    }
    return ret;
}

Soprano::Model *KoDocumentRdf::findStatements(QTextCursor &cursor, int depth)
{
    Q_ASSERT(d->model);
    Soprano::Model *ret(Soprano::createModel());
    Q_ASSERT(ret);
    KoTextInlineRdf *inlineRdf(0);
    RDEBUG << "model.sz:" << d->model->statementCount();
    //
    // Look backwards for enclosing text:meta and bookmark-start tags
    //
    if (KoInlineTextObjectManager *textObjectManager
            = KoTextDocument(cursor.document()).inlineTextObjectManager()) {
        long searchStartPosition = cursor.position();
        int limit = 500;
        for (QTextCursor tc = cursor; !tc.atStart() && limit;
                tc.movePosition(QTextCursor::Left), --limit) {
            KoInlineObject *inlineObject = textObjectManager->inlineTextObject(tc);
            if (inlineObject) {
                if (KoBookmark *bm = dynamic_cast<KoBookmark*>(inlineObject)) {
                    RDEBUG << "have KoBookmark type:" << bm->type() << " at:" <<  tc.position() << endl;
                    if (bm->type() == KoBookmark::EndBookmark) {
                        continue;
                    }
                    if (bm->type() == KoBookmark::StartBookmark) {
                        KoBookmark *e = bm->endBookmark();
                        if (e && e->position() < searchStartPosition)
                            continue;
                    }
                }
                if (KoTextMeta *bm = dynamic_cast<KoTextMeta*>(inlineObject)) {
                    RDEBUG << "have KoMeta type:" << bm->type() << " at:" <<  tc.position() << endl;
                    if (bm->type() == KoTextMeta::EndBookmark) {
                        RDEBUG << "end text:meta, cursor:" << searchStartPosition;
                        RDEBUG << " end.pos:" << bm->position();
                        continue;
                    }
                    if (bm->type() == KoTextMeta::StartBookmark) {
                        KoTextMeta *e = bm->endBookmark();

                        RDEBUG << "start text:meta, cursor:" << searchStartPosition;
                        RDEBUG << " start.pos:" << bm->position();

                        if (e) {
                            RDEBUG << " e.pos:" << e->position() << endl;
                        }                         else {
                            RDEBUG << "no end marker!" << endl;
                        }

                        if (e && e->position() < searchStartPosition)
                            continue;
                    }
                }
                if (KoInlineObject *shape = dynamic_cast<KoInlineObject*>(inlineObject)) {
                    RDEBUG << "have KoInlineObject at:" <<  tc.position();
                    inlineRdf = shape->inlineRdf();
                    if (inlineRdf) {
                        break;
                    }
                }
            }
        }
    }
    RDEBUG << "1 model.sz:" << d->model->statementCount();
    RDEBUG << " ret.sz:" << ret->statementCount();
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
    inlineRdf = KoTextInlineRdf::tryToGetInlineRdf(cursor);
    if (inlineRdf) {
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

Soprano::Model *KoDocumentRdf::findStatements(const QString &xmlid, int depth)
{
    Soprano::Model *ret(Soprano::createModel());
    Q_ASSERT(ret);
    addStatements(ret, xmlid);
    for (int i = 1; i < depth; ++i) {
        expandStatements(ret);
    }
    return ret;
}

Soprano::Model *KoDocumentRdf::findStatements(KoTextEditor *handler, int depth)
{
    Q_ASSERT(d->model);
    Soprano::Model *ret(Soprano::createModel());
    Q_ASSERT(ret);
    KoTextInlineRdf *inlineRdf(0);
    RDEBUG << "model.sz:" << d->model->statementCount();
    //
    // Look backwards for enclosing text:meta and bookmark-start tags
    //
    if (KoInlineTextObjectManager *textObjectManager
            = KoTextDocument(handler->document()).inlineTextObjectManager()) {
        long searchStartPosition = handler->position();
        KoTextEditor tc(handler->document());
        tc.setPosition(handler->position());
        for (int limit = 500; !tc.atStart() && limit;
                tc.movePosition(QTextCursor::Left), --limit) {
            QTextCursor qtc(handler->document());
            qtc.setPosition(tc.position());
            KoInlineObject *inlineObject = textObjectManager->inlineTextObject(qtc);
            if (inlineObject) {
                if (KoBookmark *bm = dynamic_cast<KoBookmark*>(inlineObject)) {
                    RDEBUG << "have KoBookmark type:" << bm->type() << " at:" <<  tc.position() << endl;
                    if (bm->type() == KoBookmark::EndBookmark) {
                        continue;
                    }
                    if (bm->type() == KoBookmark::StartBookmark) {
                        KoBookmark *e = bm->endBookmark();
                        if (e && e->position() < searchStartPosition)
                            continue;
                    }
                }
                if (KoTextMeta *bm = dynamic_cast<KoTextMeta*>(inlineObject)) {
                    RDEBUG << "have KoMeta type:" << bm->type() << " at:" <<  tc.position() << endl;
                    if (bm->type() == KoTextMeta::EndBookmark) {
                        RDEBUG << "end text:meta, cursor:" << searchStartPosition;
                        RDEBUG << " end.pos:" << bm->position();
                        continue;
                    }
                    if (bm->type() == KoTextMeta::StartBookmark) {
                        KoTextMeta *e = bm->endBookmark();
                        RDEBUG << "start text:meta, cursor:" << searchStartPosition;
                        RDEBUG << " start.pos:" << bm->position();
                        if (e) {
                            RDEBUG << " e.pos:" << e->position() << endl;
                        }                         else {
                            RDEBUG << "no end marker!" << endl;
                        }
                        if (e && e->position() < searchStartPosition)
                            continue;
                    }
                }
                if (KoInlineObject *shape = dynamic_cast<KoInlineObject*>(inlineObject)) {
                    RDEBUG << "have KoInlineObject at:" <<  tc.position();
                    inlineRdf = shape->inlineRdf();
                    if (inlineRdf) {
                        break;
                    }
                }
            }
        }
    }
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

KoTextInlineRdf *KoDocumentRdf::findInlineRdfByID(const QString &xmlid)
{
    RDEBUG << "xxx xmlid:" << xmlid;
    foreach (KoTextInlineRdf *sp, d->inlineRdfObjects) {
        RDEBUG << "sp:" << (void*)sp;
        if (sp->xmlId() == xmlid) {
            return sp;
        }
    }
    return 0;
}


void KoDocumentRdf::rememberNewInlineRdfObject(KoTextInlineRdf *inlineRdf)
{
    if (!inlineRdf) {
        return;
    }
    d->inlineRdfObjects << inlineRdf;
}

void KoDocumentRdf::updateInlineRdfStatements(QTextDocument *qdoc)
{
    RDEBUG << "top";
    KoInlineTextObjectManager *textObjectManager = KoTextDocument(qdoc).inlineTextObjectManager();
    d->inlineRdfObjects.clear();
    if(!textObjectManager) {
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
    foreach (KoTextInlineRdf *sp, d->inlineRdfObjects) {
        Soprano::Statement st = toStatement(sp);
        if (st.isValid()) {
            d->model->addStatement(st);
        }
    }
    RDEBUG << "done";
}

void KoDocumentRdf::emitSemanticObjectAdded(KoRdfSemanticItem *item)
{
    emit semanticObjectAdded(item);
}

void KoDocumentRdf::emitSemanticObjectAddedConst(KoRdfSemanticItem *const item)
{
    emit semanticObjectAdded(item);
}

void KoDocumentRdf::emitSemanticObjectUpdated(KoRdfSemanticItem *item)
{
    RDEBUG << "item:" << item;
    if (item) {
        //
        // reflow the formatting for each view of the semanticItem, in reverse document order
        //
        QMap<int, reflowItem> col;
        QStringList xmlidlist = item->xmlIdList();
        foreach (const QString &xmlid, xmlidlist) {
            RDEBUG << "xmlid:" << xmlid << " reflow item:" << item->name();
            insertReflow(col, item);
        }
        applyReflow(col);
    }
    emit semanticObjectUpdated(item);
}

void KoDocumentRdf::emitSemanticObjectViewSiteUpdated(KoRdfSemanticItem *item, const QString &xmlid)
{
    RDEBUG << "item:" << item;
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

KoDocumentRdf::reflowItem::reflowItem(KoRdfSemanticItem *si, const QString &xmlid, KoSemanticStylesheet *ss, const QPair< int, int > &extent)
        : m_si(si)
        , m_ss(ss)
        , m_xmlid(xmlid)
        , m_extent(extent)
{
}

void KoDocumentRdf::insertReflow(QMap<int, reflowItem> &col, KoRdfSemanticItem *obj, KoSemanticStylesheet *ss)
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

void KoDocumentRdf::insertReflow(QMap<int, reflowItem> &col, KoRdfSemanticItem *obj,
                                 const QString &sheetType, const QString &stylesheetName)
{
    KoSemanticStylesheet *ss = obj->findStylesheetByName(sheetType, stylesheetName);
    insertReflow(col, obj, ss);
}

void KoDocumentRdf::insertReflow(QMap<int, reflowItem> &col, KoRdfSemanticItem *obj)
{
    RDEBUG << "reflowing object:" << obj->name();
    foreach (const QString &xmlid, obj->xmlIdList()) {
        QPair<int, int> extent = findExtent(xmlid);
        RDEBUG << "format(), adding reflow xmlid location:" << xmlid << " extent:" << extent;
        reflowItem item(obj, xmlid, 0, extent);
        col.insert(extent.first, item);
    }
}

void KoDocumentRdf::applyReflow(const QMap<int, reflowItem> &col)
{
    if (!document()) {
        return;
    }
    QMapIterator< int, reflowItem > i(col);
    i.toBack();
    while (i.hasPrevious()) {
        reflowItem item = i.previous().value();
        RDEBUG << "format(), extent:" << item.m_extent;
        RDEBUG << "xmlid:" << item.m_xmlid;
        RDEBUG << "format(), semitem:" << item.m_si;
        RDEBUG << "format(), semitem.name:" << item.m_si->name();
        if (item.m_ss) {
            KoRdfSemanticItemViewSite vs(item.m_si, item.m_xmlid);
            vs.setStylesheetWithoutReflow(item.m_ss);
        }
        emitSemanticObjectViewSiteUpdated(item.m_si, item.m_xmlid);
    }
}

QList<KoSemanticStylesheet*> KoDocumentRdf::userStyleSheetList(const QString& className)
{
    return d->userStylesheets[className];
}

void KoDocumentRdf::setUserStyleSheetList(const QString& className,const QList<KoSemanticStylesheet*>& l)
{
    d->userStylesheets[className] = l;
}

#define TextTool_ID "TextToolFactory_ID"

KoTextEditor *KoDocumentRdf::ensureTextTool(KoCanvasBase *host)
{
    KoToolManager::instance()->switchToolRequested(TextTool_ID);
    KoTextEditor *ret = qobject_cast<KoTextEditor*>(host->toolProxy()->selection());
    return ret;
}

void KoDocumentRdf::ensureTextTool()
{
    KoToolManager::instance()->switchToolRequested(TextTool_ID);
}
