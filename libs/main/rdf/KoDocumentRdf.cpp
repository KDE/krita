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
#include "RdfPrefixMapping.h"
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

using namespace Soprano;

QByteArray fileToByteArray(const QString &fileName)
{
    QFile t(fileName);
    t.open(QIODevice::ReadOnly);
    return t.readAll();
}

QString getProperty(Soprano::Model *m, Soprano::Node subj, Soprano::Node pred, QString defval)
{
    StatementIterator it = m->listStatements(subj, pred, Node());
    QList<Statement> allStatements = it.allElements();
    foreach (Soprano::Statement s, allStatements) {
        return s.object().toString();
    }
    return defval;
}

QString optionalBindingAsString(Soprano::QueryResultIterator &it, QString bindingName, QString def)
{
    if (it.binding(bindingName).isValid()) {
        return it.binding(bindingName).toString();
    }
    return def;
}

KoDocumentRdf::KoDocumentRdf(KoDocument *parent)
        : KoDocumentRdfBase(parent)
        , m_model(Soprano::createModel())
        , m_prefixMapping(0)
{
    m_prefixMapping = new RdfPrefixMapping(this);
}

KoDocumentRdf::~KoDocumentRdf()
{
    kDebug(30015);
    delete m_prefixMapping;
    if (m_model) {
        delete m_model;
    }
}

Soprano::Model *KoDocumentRdf::model() const
{
    return m_model;
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
    return (KoDocument*)parent();
}


RdfPrefixMapping *KoDocumentRdf::getPrefixMapping() const
{
    return m_prefixMapping;
}

/**
 * Graph context used for Rdf stored inline in content.xml
 * in an Rdfa like fashion.
 */
Soprano::Node KoDocumentRdf::getInlineRdfContext() const
{
    return Node(QUrl("http://www.koffice.org/Rdf/inline-rdf"));
}

QString KoDocumentRdf::getRdfInternalMetadataWithoutSubjectURI() const
{
    return "http://www.koffice.org/Rdf/internal/content.xml";
}

QString KoDocumentRdf::getRdfPathContextPrefix() const
{
    return "http://www.koffice.org/Rdf/path/";
}

Soprano::Node KoDocumentRdf::manifestRdfNode() const
{
    return Node(QUrl(getRdfPathContextPrefix() + "manifest.rdf"));
}

void KoDocumentRdf::freshenBNodes(Soprano::Model *m)
{
    QList<Soprano::Statement> removeList;
    QList<Soprano::Statement> addList;
    QMap<QString, Soprano::Node> bnodeMap;
    StatementIterator it = m->listStatements();
    QList<Statement> allStatements = it.allElements();
    kDebug(30015) << "freshening model.sz:" << allStatements.size();
    foreach (Soprano::Statement s, allStatements) {
        Soprano::Node subj = s.subject();
        Soprano::Node obj = s.object();
        Soprano::Statement news;
        if (subj.type() == Soprano::Node::BlankNode) {
            QString nodeStr = subj.toString();
            Soprano::Node n = bnodeMap[ nodeStr ];
            if (!n.isValid()) {
                n = m_model->createBlankNode();
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
                n = m_model->createBlankNode();
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
    kDebug(30015) << "remove count:" << removeList.size();
    kDebug(30015) << "add count:" << addList.size();
    // Note that as of Jan 2010 you couldn't rely on
    // Soprano::Model::removeStatements() if every entry
    // in removeList did not exist exactly once in the model.
    KoTextRdfCore::removeStatementsIfTheyExist(m, removeList);
    kDebug(30015) << "after remove, model.sz:" << m->statementCount();
    m->addStatements(addList);
    kDebug(30015) << "after add,    model.sz:" << m->statementCount();
}

bool KoDocumentRdf::loadRdf(KoStore *store, const Soprano::Parser *parser, const QString &fileName)
{
    bool ok = true;
    if (!store->open(fileName)) {
        kDebug(30003) << "Entry " << fileName << " not found!"; // not a warning as embedded stores don't have to have all files
        return false;
    }
    kDebug(30015) << "Loading external Rdf/XML from:" << fileName;
    Soprano::Node context(QUrl(getRdfPathContextPrefix() + fileName));
    QUrl BaseURI = QUrl(QString());
    QString rdfxmlData(store->device()->readAll());
    Soprano::StatementIterator it = parser->parseString(rdfxmlData, BaseURI,
                                    Soprano::SerializationRdfXml);
    QList<Statement> allStatements = it.allElements();
    kDebug(30015) << "Found " << allStatements.size() << " triples..." << endl;
    Soprano::Model *tmpmodel(Soprano::createModel());
    foreach (Soprano::Statement s, allStatements) {
        Soprano::Node subj = s.subject();
        Soprano::Node pred = s.predicate();
        Soprano::Node obj  = s.object();
        Error::ErrorCode err = tmpmodel->addStatement(subj, pred, obj, context);
        if (err != Error::ErrorNone) {
            kDebug(30015) << "Error adding triple! s:" << subj << " p:" << pred << " o:" << obj << endl;
            ok = false;
            break;
        }
    }
    kDebug(30015) << "calling freshenBNodes(), tmpmodel.sz:" << tmpmodel->statementCount();
    dumpModel(fileName, tmpmodel);
    freshenBNodes(tmpmodel);
    dumpModel(fileName, tmpmodel);
    kDebug(30015) << "done with freshenBNodes(), tmpmodel.sz:" << tmpmodel->statementCount();
    m_model->addStatements(tmpmodel->listStatements().allElements());
    if (fileName == "manifest.rdf" && m_prefixMapping) {
        m_prefixMapping->load(m_model);

        QStringList classNames = RdfSemanticItem::classNames();
        foreach (const QString &klass, classNames) {
            RdfSemanticItem *si = RdfSemanticItem::createSemanticItem(this, this, klass);
            si->loadUserStylesheets(m_model);
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
                              "  ?subj rdf:type odf:MetaDataFile  \n"
                              "  ?subj odfcommon:path ?fileName  \n"
                              " } \n";
        Soprano::QueryResultIterator it =
            m_model->executeQuery(sparqlQuery,
                                  Soprano::Query::QueryLanguageSparql);
        QList< QString > externalRdfFiles;
        //
        // This is a bit tricky, loadRdf() might block if the
        // sparql query is still being iterated, so we have to
        // store the fileNames and exhaust the binding result
        // iterator first.
        //
        while (ok && it.next()) {
            QString fileName = it.binding("fileName").toString();
            externalRdfFiles << fileName;
        }
        foreach (const QString &fileName, externalRdfFiles) {
            ok = loadRdf(store, parser, fileName);
        }
    }
    return ok;
}

bool KoDocumentRdf::saveRdf(KoStore *store, KoXmlWriter *manifestWriter, Soprano::Node &context)
{
    bool ok = false;
    QString fileName = "manifest.rdf";
    if (context.toString() == getInlineRdfContext().toString()) {
        kDebug(30015) << "found some internal Rdf, this is handled by augmenting the DOM";
        return true;
    }
    //
    // The context contains the filename to save into
    //
    if (context.toString().startsWith(getRdfPathContextPrefix())) {
        fileName = context.toString().mid(getRdfPathContextPrefix().size());
    }
    kDebug(30015) << "saving external file:" << fileName;
    if (!store->open(fileName)) {
        document()->setErrorMessage(
            i18n("Not able to write '%1'. Partition full?", (fileName)));
        return false;
    }
    KoStoreDevice dev(store);
    QTextStream oss(&dev);
    if (fileName == "manifest.rdf" && m_prefixMapping) {
        m_prefixMapping->save(m_model, context);

        QStringList classNames = RdfSemanticItem::classNames();
        foreach (const QString &klass, classNames) {
            RdfSemanticItem *si = RdfSemanticItem::createSemanticItem(this, this, klass);
            si->saveUserStylesheets(m_model, context);
        }
    }
    Soprano::StatementIterator triples = m_model->listStatements(Soprano::Node(),
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
            kDebug(30015) << "fileName:" << fileName << " data.sz:" << data.size();
            kDebug(30015) << "model.sz:" << m_model->statementCount();
            ok = true;
        } else {
            kDebug(30015) << "serialization of Rdf failed!";
        }
    }
    oss.flush();
    store->close();
    manifestWriter->addManifestEntry(fileName, "application/rdf+xml");
    return ok;
}

bool KoDocumentRdf::saveOasis(KoStore *store, KoXmlWriter *manifestWriter)
{
    kDebug(30015) << "saveOasis() generic";
    bool ok = true;
    NodeIterator contextier = m_model->listContexts();
    QList<Node> contexts = contextier.allElements();
    foreach (Soprano::Node n, contexts) {
        saveRdf(store, manifestWriter, n);
    }
    return ok;
}

void KoDocumentRdf::updateXmlIdReferences(const QMap<QString, QString> &m)
{
    kDebug(30015) << "KoDocumentRdf::updateXmlIdReferences() m.size:" << m.size();

    QList<Soprano::Statement> removeList;
    QList<Soprano::Statement> addList;
    StatementIterator it = m_model->listStatements(
                               Node(),
                               Node(QUrl("http://docs.oasis-open.org/opendocument/meta/package/common#idref")),
                               Node(),
                               Node());
    QList<Statement> allStatements = it.allElements();
    foreach (Soprano::Statement s, allStatements) {
        kDebug(30015) << "seeking obj:" << s.object();
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
            kDebug(30015) << "looking for inlineRdf object for ID:" << oldID;
            if (KoTextInlineRdf *inlineRdf = findInlineRdfByID(oldID)) {
                kDebug(30015) << "updating the xmlid of the inline object";
                kDebug(30015) << "old:" << oldID << " new:" << newID;
                inlineRdf->setXmlId(newID);
            }
        }
    }
    // out with the old, in with the new
    // remove & add the triple lists.
    kDebug(30015) << "addStatements.size:" << addList.size();
    kDebug(30015) << " remove.size:" << removeList.size();
    KoTextRdfCore::removeStatementsIfTheyExist(m_model, removeList);
    m_model->addStatements(addList);
}

QList<RdfFoaF*> KoDocumentRdf::foaf(Soprano::Model *m)
{
    if (!m) {
        m = model();
    }
    QString sparqlQuery = "prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
                          "prefix foaf: <http://xmlns.com/foaf/0.1/> \n"
                          "prefix pkg: <http://docs.oasis-open.org/opendocument/meta/package/common#> \n"
                          "select distinct ?graph ?person ?name ?nick ?homepage ?img ?phone \n"
                          "where { \n"
                          "  GRAPH ?graph { \n"
                          "    ?person rdf:type foaf:Person \n"
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
    kDebug(30015) << "1 model().sz:" << model()->statementCount() << " m.sz:" << m->statementCount();
    // lastKnownObjects is used to perform a sematic set diff
    // at return time m_foafObjects will have any new objects and
    // ones that are no longer available will be removed.
    QList<RdfFoaF*> lastKnownObjects = m_foafObjects;
    // uniqfilter is needed because soprano is not honouring
    // the DISTINCT sparql keyword
    QSet<QString> uniqfilter;
    while (it.next()) {
        QString n = it.binding("name").toString();
        kDebug(30015) << "n:" << n;
        if (uniqfilter.contains(n))
            continue;
        uniqfilter += n;

        RdfFoaF *newItem = new RdfFoaF(this, this, it);
        QString newItemLs = newItem->linkingSubject().toString();
        foreach (RdfFoaF *semItem, lastKnownObjects) {
            if (newItemLs == semItem->linkingSubject().toString()) {
                lastKnownObjects.removeAll(semItem);
                delete newItem;
                newItem = 0;
                break;
            }
        }
        if (newItem) {
            m_foafObjects << newItem;
        }
    }
    foreach (RdfFoaF *semItem, lastKnownObjects) {
        m_foafObjects.removeAll(semItem);
    }
    kDebug(30015) << "foaf() size:" << m_foafObjects.size() << endl;
    if (m_foafObjects.empty() && m->statementCount()) {
        kDebug(30015) << "foaf() have data, but no foafs!" << endl;
        QList<Statement> allStatements = m->listStatements().allElements();
        foreach (Soprano::Statement s, allStatements) {
            kDebug(30015) << s;
        }
    }
    return m_foafObjects;
}

QList<RdfCalendarEvent*> KoDocumentRdf::calendarEvents(Soprano::Model *m)
{
    if (!m) {
        m = model();
    }
    QString sparqlQuery = " prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
                          " prefix foaf: <http://xmlns.com/foaf/0.1/>  \n"
                          " prefix cal:  <http://www.w3.org/2002/12/cal/icaltzd#>  \n"
                          " select distinct ?graph ?ev ?uid ?dtstart ?dtend ?summary ?location ?geo ?long ?lat \n"
                          " where {  \n"
                          "  GRAPH ?graph { \n"
                          "    ?ev rdf:type cal:Vevent  \n"
                          "    ?ev cal:uid      ?uid  \n"
                          "    ?ev cal:dtstart  ?dtstart \n"
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
    QList<RdfCalendarEvent*> lastKnownObjects = m_calObjects;
    // uniqfilter is needed because soprano is not honouring
    // the DISTINCT sparql keyword
    QSet<QString> uniqfilter;
    while (it.next()) {
        QString n = it.binding("uid").toString();
        if (uniqfilter.contains(n))
            continue;
        uniqfilter += n;
        kDebug(30015) << " g:" << it.binding("g").toString();
        kDebug(30015) << " uid:" << it.binding("uid").toString();

        RdfCalendarEvent *newItem(new RdfCalendarEvent(this, this, it));
        QString newItemLs = newItem->linkingSubject().toString();
        foreach (RdfCalendarEvent *semItem, lastKnownObjects) {
            if (newItemLs == semItem->linkingSubject().toString()) {
                lastKnownObjects.removeAll(semItem);
                delete newItem;
                newItem = 0;
                break;
            }
        }
        if (newItem) {
            m_calObjects << newItem;
        }
    }
    foreach (RdfCalendarEvent *semItem, lastKnownObjects) {
        m_calObjects.removeAll(semItem);
    }
    kDebug(30015) << "calendarEvents() size:" << m_calObjects.size() << endl;
    return m_calObjects;
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

        foreach (const QString &b, m_bindingsThatMakeID) { // TODO why loop and not use the var?
            QString n = it.binding("lat").toString();
            ret += n;
        }
        return ret;
    }
};


void KoDocumentRdf::addLocations(Soprano::Model *m, QList<RdfLocation*> &ret, bool isGeo84,
                                 const QString &sparqlQuery)
{
    Soprano::QueryResultIterator it = m->executeQuery(sparqlQuery,
                        Soprano::Query::QueryLanguageSparql);
    SparqlDistinctPostprocess uniqFilter("lat");
    uniqFilter.addBindingToKeySet("long");
    while (it.next()) {
        if (uniqFilter.shouldSkip(it))
            continue;

        RdfLocation *semObj(new RdfLocation(this, this, it, isGeo84));
        ret << semObj;
    }
    kDebug(30015) << "addLocations() size:" << ret.size() << endl;
}

QList<RdfLocation*> KoDocumentRdf::locations(Soprano::Model *m)
{
    if (!m) {
        m = model();
    }
    kDebug(30015) << "locations(top) full-model.sz:" << m_model->statementCount();
    kDebug(30015) << " passed model.size:" << m->statementCount();
    QList<RdfLocation*> currentRdfLocations;
    addLocations(m, currentRdfLocations, false,
        " prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#>  \n"
        " prefix foaf: <http://xmlns.com/foaf/0.1/>  \n"
        " prefix cal:  <http://www.w3.org/2002/12/cal/icaltzd#>  \n"
        " select distinct ?graph ?geo ?long ?lat ?joiner \n"
        " where {  \n"
        "  GRAPH ?graph { \n"
        "               ?ev cal:geo ?geo \n"
        "               ?geo rdf:first ?lat \n"
        "               ?geo rdf:rest ?joiner \n"
        "               ?joiner rdf:first ?long \n"
        "               } \n"
        "  } \n");
    kDebug(30015) << "locations(1) currentRdfLocations.size:" << currentRdfLocations.size();
    addLocations(m, currentRdfLocations, true,
        " prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
        " prefix foaf: <http://xmlns.com/foaf/0.1/>  \n"
        " prefix geo84: <http://www.w3.org/2003/01/geo/wgs84_pos#> \n"
        "  \n"
        " select ?graph ?geo ?long ?lat ?type \n"
        " where {  \n"
        "  GRAPH ?graph { \n"
        "  \n"
        "        ?geo geo84:lat  ?lat \n"
        "        ?geo geo84:long ?long \n"
        "        OPTIONAL { ?geo rdf:type ?type } \n"
        "  \n"
        "  } \n"
        " } \n");
    // add the new, remove the no longer existing between m_locObjects and currentRdfLocations.
    // The semantic items have a lifetime of this KoDocumentRDF.
    // If we could use smart pointers then we could just return the new list of locations,
    // As semantic items have a lifetime of this KoDocumentRDF,
    // we don't want to create any more than are needed.
    //
    // As currentRdfLocations contains all the location semitems we have found to be valid,
    // we need to transfer any new ones from that list to m_locObjects and delete what
    // remains (which are objects that existed in m_locObjects before and were rediscovered
    // during the query process).
    //
    // Creating a list of locations each time similifies the query and discovery process
    // at the expense of this little mess to merge the new and old with explicit pointer
    // and object lifetime handling
    QList<RdfLocation*> removeSet;
    foreach (RdfLocation *oldItem, m_locObjects) {
        QString oldItemLs = oldItem->linkingSubject().toString();
        bool found = false;
        foreach (RdfLocation *newItem, currentRdfLocations) {
            if (oldItemLs == newItem->linkingSubject().toString()) {
                found = true;
                break;
            }
        }
        if (!found) {
            removeSet << oldItem;
        }
    }
    foreach (RdfLocation *item, removeSet) {
        m_locObjects.removeAll(item);
    }
    QList<RdfLocation*> addedSet;
    foreach (RdfLocation *newItem, currentRdfLocations) {
        QString newItemLs = newItem->linkingSubject().toString();
        bool found = false;
        foreach (RdfLocation *oldItem, m_locObjects) {
            if (newItemLs == oldItem->linkingSubject().toString()) {
                found = true;
                break;
            }
        }
        if (!found) {
            m_locObjects << newItem;
            addedSet << newItem;
        }
    }
    foreach (RdfLocation *item, addedSet) {
        currentRdfLocations.removeAll(item);
    }
    kDebug(30015) << "locations(end) deleting duplicates size:" << currentRdfLocations.size() << endl;
    qDeleteAll(currentRdfLocations);
    kDebug(30015) << "locations(end) size:" << m_locObjects.size() << endl;
    return m_locObjects;
}

void KoDocumentRdf::dumpModel(const QString &msg, Soprano::Model *m) const
{
    if (!m) {
        return;
    }
    QList<Soprano::Statement> allStatements = m->listStatements().allElements();
    kDebug(30015) << "----- " << msg << " ----- model size:" << allStatements.size();
    foreach (Soprano::Statement s, allStatements) {
        kDebug(30015) << s;
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
        subj = getInlineRdfContext();
    }
    kDebug(30015) << "subj:"  << subj;
    kDebug(30015) << " pred:" << pred;
    kDebug(30015) << " obj:"  << obj;
    Soprano::Statement ret(subj, pred, obj, getInlineRdfContext());
    return ret;
}

void KoDocumentRdf::addStatements(Soprano::Model *model, const QString &xmlid)
{
    QString sparqlQuery;
    QTextStream queryss(&sparqlQuery);
    kDebug(30015) << "addStatements model.sz:" << m_model->statementCount() << " xmlid:" << xmlid;
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
    kDebug(30015) << "sparql:" << sparqlQuery;
    Soprano::QueryResultIterator it = m_model->executeQuery(sparqlQuery,
                              Soprano::Query::QueryLanguageSparql);
    while (it.next()) {
        Statement s(it.binding("s"),
                    it.binding("p"),
                    it.binding("o"));
        model->addStatement(s);
        kDebug(30015) << "result, s:" << it.binding("s");
        kDebug(30015) << " p:" << it.binding("p");
        kDebug(30015) << " o:" << it.binding("o");
    }
}

void KoDocumentRdf::expandStatementsReferencingSubject(Soprano::Model *model)
{
    QList<Statement> addList;
    QList<Statement> allStatements = model->listStatements().allElements();
    foreach (Soprano::Statement s, allStatements) {
        QList<Statement> all = m_model->listStatements(Node(), Node(), s.subject()).allElements();
        foreach (Soprano::Statement e, all) {
            addList << e;
        }
    }
    model->addStatements(addList);
}

void KoDocumentRdf::expandStatementsSubjectPointsTo(Soprano::Model *model)
{
    QList<Statement> addList;
    QList<Statement> allStatements = model->listStatements().allElements();
    foreach (Soprano::Statement s, allStatements) {
        QList<Statement> all = m_model->listStatements(s.object(), Node(), Node()).allElements();
        foreach (Soprano::Statement e, all) {
            kDebug(30015) << "ADD obj:" << s.object() << " adding:" << e;
            addList << e;
        }
    }
    model->addStatements(addList);
}

void KoDocumentRdf::expandStatementsSubjectPointsTo(Soprano::Model *model, const Soprano::Node &n)
{
    QList<Statement> addList;
    QList<Statement> all = m_model->listStatements(n, Node(), Node()).allElements();
    foreach (Soprano::Statement e, all) {
        kDebug(30015) << "n:" << n << " adding:" << e;
        addList << e;
    }
    model->addStatements(addList);
}

void KoDocumentRdf::expandStatementsToIncludeRdfListsRecurse(Soprano::Model *model,
        QList<Statement> &addList, const Soprano::Node &n)
{
    Node rdfFirst = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#first"));
    Node rdfRest  = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"));
    QList<Statement> all;
    all = m_model->listStatements(n, rdfFirst, Node()).allElements();
    addList << all;
    all = m_model->listStatements(n, rdfRest, Node()).allElements();
    addList << all;
    foreach (Soprano::Statement s, all) {
        expandStatementsToIncludeRdfListsRecurse(model, addList, s.object());
    }
}


void KoDocumentRdf::expandStatementsToIncludeRdfLists(Soprano::Model *model)
{
    kDebug(30015) << "model.sz:" << model->statementCount();
    QList<Statement> addList;
    QList<Statement> allStatements = model->listStatements().allElements();
    foreach (Soprano::Statement s, allStatements) {
        expandStatementsToIncludeRdfListsRecurse(model, addList, s.subject());
    }
    kDebug(30015) << "model.sz:" << model->statementCount();
    kDebug(30015) << "addList.sz:" << addList.size();
    model->addStatements(addList);
}

void KoDocumentRdf::expandStatementsToIncludeOtherPredicates(Soprano::Model *model)
{
    QList<Statement> addList;
    QList<Statement> allStatements = model->listStatements().allElements();
    foreach (Soprano::Statement s, allStatements) {
        QList<Statement> all = m_model->listStatements(s.subject(), Node(), Node()).allElements();
        foreach (Soprano::Statement e, all) {
            addList << e;
        }
    }
    model->addStatements(addList);
}

void KoDocumentRdf::expandStatements(Soprano::Model *model)
{
    expandStatementsReferencingSubject(model);
    expandStatementsToIncludeOtherPredicates(model);
}

KAction *KoDocumentRdf::createInsertSemanticObjectReferenceAction(KoCanvasBase *host)
{
    KAction *ret = new InsertSemanticObjectReferenceAction(host, this, i18n("Reference"));
    kDebug(30015) << "createInsertSemanticObjectReferenceAction";
    return ret;
}

QList<KAction*> KoDocumentRdf::createInsertSemanticObjectNewActions(KoCanvasBase *host)
{
    QList<KAction*> ret;
    foreach (const QString &klass,  RdfSemanticItem::classNames()) {
        ret.append(new InsertSemanticObjectCreateAction(host, this, klass));
    }
    return ret;
}

QPair<int, int> KoDocumentRdf::findExtent(const QString &xmlid)
{
    KoTextInlineRdf *obj = findInlineRdfByID(xmlid);
    if (obj) {
        QPair<int, int> ret = obj->findExtent();
        kDebug(30015) << "have inline obj, extent:" << ret;
        return ret;
    }
    return QPair<int, int>(0, 0);
}

QPair<int, int> KoDocumentRdf::findExtent(QTextCursor &cursor)
{
    QPair<int, int> ret(0, 0);
    kDebug(30015) << "model.sz:" << m_model->statementCount();
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
    QPair<int, int> ret(0, 0);
    kDebug(30015) << "model.sz:" << m_model->statementCount();
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
                    kDebug(30015) << "have KoInlineObject at:" <<  tc.position();
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
                    kDebug(30015) << "have KoInlineObject at:" <<  tc.position();
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
    Soprano::Model *ret(Soprano::createModel());
    KoTextInlineRdf *inlineRdf(0);
    kDebug(30015) << "model.sz:" << m_model->statementCount();
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
                    kDebug(30015) << "have KoBookmark type:" << bm->type() << " at:" <<  tc.position() << endl;
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
                    kDebug(30015) << "have KoMeta type:" << bm->type() << " at:" <<  tc.position() << endl;
                    if (bm->type() == KoTextMeta::EndBookmark) {
                        kDebug(30015) << "end text:meta, cursor:" << searchStartPosition;
                        kDebug(30015) << " end.pos:" << bm->position();
                        continue;
                    }
                    if (bm->type() == KoTextMeta::StartBookmark) {
                        KoTextMeta *e = bm->endBookmark();

                        kDebug(30015) << "start text:meta, cursor:" << searchStartPosition;
                        kDebug(30015) << " start.pos:" << bm->position();

                        if (e) {
                            kDebug(30015) << " e.pos:" << e->position() << endl;
                        }                         else {
                            kDebug(30015) << "no end marker!" << endl;
                        }

                        if (e && e->position() < searchStartPosition)
                            continue;
                    }
                }
                if (KoInlineObject *shape = dynamic_cast<KoInlineObject*>(inlineObject)) {
                    kDebug(30015) << "have KoInlineObject at:" <<  tc.position();
                    inlineRdf = shape->inlineRdf();
                    if (inlineRdf) {
                        break;
                    }
                }
            }
        }
    }
    kDebug(30015) << "1 model.sz:" << m_model->statementCount();
    kDebug(30015) << " ret.sz:" << ret->statementCount();
    if (inlineRdf) {
        kDebug(30015) << "have inlineRdf1...xmlid:" << inlineRdf->xmlId();
        kDebug(30015) << " ret.sz:" << ret->statementCount();
        ret->addStatement(toStatement(inlineRdf));
        kDebug(30015) << "have inlineRdf2...xmlid:" << inlineRdf->xmlId();
        kDebug(30015) << " ret.sz:" << ret->statementCount();
        QString xmlid = inlineRdf->xmlId();
        addStatements(ret, xmlid);
    }
    kDebug(30015) << "2 ret.sz:" << ret->statementCount();
    kDebug(30015) << "checking for block inlineRdf...";
    inlineRdf = KoTextInlineRdf::tryToGetInlineRdf(cursor);
    if (inlineRdf) {
        ret->addStatement(toStatement(inlineRdf));
        QString xmlid = inlineRdf->xmlId();
        addStatements(ret, xmlid);
        kDebug(30015) << "have block inlineRdf...xmlid:" << inlineRdf->xmlId();
    }
    kDebug(30015) << "3 ret.sz:" << ret->statementCount();
    kDebug(30015) << "expanding statements...";
    for (int i = 1; i < depth; ++i) {
        expandStatements(ret);
    }
    return ret;
}

Soprano::Model *KoDocumentRdf::findStatements(const QString &xmlid, int depth)
{
    Soprano::Model *ret(Soprano::createModel());
    addStatements(ret, xmlid);
    for (int i = 1; i < depth; ++i) {
        expandStatements(ret);
    }
    return ret;
}

Soprano::Model *KoDocumentRdf::findStatements(KoTextEditor *handler, int depth)
{
    Soprano::Model *ret(Soprano::createModel());
    KoTextInlineRdf *inlineRdf(0);
    kDebug(30015) << "model.sz:" << m_model->statementCount();
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
                    kDebug(30015) << "have KoBookmark type:" << bm->type() << " at:" <<  tc.position() << endl;
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
                    kDebug(30015) << "have KoMeta type:" << bm->type() << " at:" <<  tc.position() << endl;
                    if (bm->type() == KoTextMeta::EndBookmark) {
                        kDebug(30015) << "end text:meta, cursor:" << searchStartPosition;
                        kDebug(30015) << " end.pos:" << bm->position();
                        continue;
                    }
                    if (bm->type() == KoTextMeta::StartBookmark) {
                        KoTextMeta *e = bm->endBookmark();
                        kDebug(30015) << "start text:meta, cursor:" << searchStartPosition;
                        kDebug(30015) << " start.pos:" << bm->position();
                        if (e) {
                            kDebug(30015) << " e.pos:" << e->position() << endl;
                        }                         else {
                            kDebug(30015) << "no end marker!" << endl;
                        }
                        if (e && e->position() < searchStartPosition)
                            continue;
                    }
                }
                if (KoInlineObject *shape = dynamic_cast<KoInlineObject*>(inlineObject)) {
                    kDebug(30015) << "have KoInlineObject at:" <<  tc.position();
                    inlineRdf = shape->inlineRdf();
                    if (inlineRdf) {
                        break;
                    }
                }
            }
        }
    }
    kDebug(30015) << "1 model.sz:" << m_model->statementCount()
        << " ret.sz:" << ret->statementCount();
    if (inlineRdf) {
        kDebug(30015) << "have inlineRdf1...xmlid:" << inlineRdf->xmlId();
        kDebug(30015) << " ret.sz:" << ret->statementCount();
        ret->addStatement(toStatement(inlineRdf));
        kDebug(30015) << "have inlineRdf2...xmlid:" << inlineRdf->xmlId();
        kDebug(30015) << " ret.sz:" << ret->statementCount();
        QString xmlid = inlineRdf->xmlId();
        addStatements(ret, xmlid);
    }
    kDebug(30015) << "2 ret.sz:" << ret->statementCount();
    kDebug(30015) << "checking for block inlineRdf...";
    inlineRdf = KoTextInlineRdf::tryToGetInlineRdf(handler);
    if (inlineRdf) {
        kDebug(30015) << "inlineRdf:" << (void*)inlineRdf;
        ret->addStatement(toStatement(inlineRdf));
        QString xmlid = inlineRdf->xmlId();
        addStatements(ret, xmlid);
        kDebug(30015) << "have block inlineRdf...xmlid:" << inlineRdf->xmlId();
    }
    kDebug(30015) << "3 ret.sz:" << ret->statementCount();
    kDebug(30015) << "expanding statements...";
    for (int i = 1; i < depth; ++i) {
        expandStatements(ret);
    }
    return ret;
}

KoTextInlineRdf *KoDocumentRdf::findInlineRdfByID(const QString &xmlid)
{
    kDebug(30015) << "xxx xmlid:" << xmlid;
    foreach (KoTextInlineRdf *sp, m_inlineRdfObjects) {
        kDebug(30015) << "sp:" << (void*)sp;
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
    m_inlineRdfObjects << inlineRdf;
}

void KoDocumentRdf::updateInlineRdfStatements(QTextDocument *qdoc)
{
    kDebug(30015) << "top";
    KoInlineTextObjectManager *textObjectManager = KoTextDocument(qdoc).inlineTextObjectManager();
    m_inlineRdfObjects.clear();
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
    foreach (QTextFormat tf, formats) { // TODO make this use a const ref
        if (KoTextInlineRdf *inlineRdf = KoTextInlineRdf::tryToGetInlineRdf(tf)) {
            rememberNewInlineRdfObject(inlineRdf);
        }
    }
    if (!m_model) {
        return;
    }
    Soprano::Node context = getInlineRdfContext();
    kDebug(30015) << "removing";
    // delete all inline Rdf statements from model
    m_model->removeAllStatements(Soprano::Node(), Soprano::Node(), Soprano::Node(), context);
    kDebug(30015) << "adding, count:" << m_inlineRdfObjects.size();
    // add statements from m_inlineRdfObjects to model
    foreach (KoTextInlineRdf *sp, m_inlineRdfObjects) {
        Soprano::Statement st = toStatement(sp);
        if (st.isValid()) {
            m_model->addStatement(st);
        }
    }
    kDebug(30015) << "done";
}

void KoDocumentRdf::emitSemanticObjectAdded(RdfSemanticItem *item)
{
    emit semanticObjectAdded(item);
}

void KoDocumentRdf::emitSemanticObjectAddedConst(RdfSemanticItem *const item)
{
    emit semanticObjectAdded(item);
}

void KoDocumentRdf::emitSemanticObjectUpdated(RdfSemanticItem *item)
{
    kDebug(30015) << "item:" << item;
    if (item) {
        //
        // reflow the formatting for each view of the semanticItem, in reverse document order
        //
        QMap<int, reflowItem> col;
        QStringList xmlidlist = item->xmlIdList();
        foreach (const QString &xmlid, xmlidlist) {
            kDebug(30015) << "xmlid:" << xmlid << " reflow item:" << item->name();
            insertReflow(col, item);
        }
        applyReflow(col);
    }
    emit semanticObjectUpdated(item);
}

void KoDocumentRdf::emitSemanticObjectViewSiteUpdated(RdfSemanticItem *item, const QString &xmlid)
{
    kDebug(30015) << "item:" << item;
    if (item) {
        kDebug(30015) << "xmlid:" << xmlid << " reflow item:" << item->name();
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

KoDocumentRdf::reflowItem::reflowItem(RdfSemanticItem *si, const QString &xmlid, SemanticStylesheet *ss, const QPair< int, int > &extent)
        : m_si(si)
        , m_ss(ss)
        , m_xmlid(xmlid)
        , m_extent(extent)
{
}

void KoDocumentRdf::insertReflow(QMap<int, reflowItem> &col, RdfSemanticItem *obj, SemanticStylesheet *ss)
{
    kDebug(30015) << "reflowing object:" << obj->name();
    QStringList xmlidlist = obj->xmlIdList();
    foreach (const QString &xmlid, xmlidlist) {
        QPair< int, int > extent = findExtent(xmlid);
        kDebug(30015) << "format(), adding reflow xmlid location:" << xmlid << " extent:" << extent;
        reflowItem item(obj, xmlid, ss, extent);
        col.insert(extent.first, item);
    }
}

void KoDocumentRdf::insertReflow(QMap<int, reflowItem> &col, RdfSemanticItem *obj,
                                 const QString &sheetType, const QString &stylesheetName)
{
    SemanticStylesheet *ss = obj->findStylesheetByName(sheetType, stylesheetName);
    insertReflow(col, obj, ss);
}

void KoDocumentRdf::insertReflow(QMap<int, reflowItem> &col, RdfSemanticItem *obj)
{
    kDebug(30015) << "reflowing object:" << obj->name();
    foreach (const QString &xmlid, obj->xmlIdList()) {
        QPair<int, int> extent = findExtent(xmlid);
        kDebug(30015) << "format(), adding reflow xmlid location:" << xmlid << " extent:" << extent;
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
        kDebug(30015) << "format(), extent:" << item.m_extent;
        kDebug(30015) << "xmlid:" << item.m_xmlid;
        kDebug(30015) << "format(), semitem:" << item.m_si;
        kDebug(30015) << "format(), semitem.name:" << item.m_si->name();
        if (item.m_ss) {
            RdfSemanticItemViewSite vs(item.m_si, item.m_xmlid);
            vs.setStylesheetWithoutReflow(item.m_ss);
        }
        emitSemanticObjectViewSiteUpdated(item.m_si, item.m_xmlid);
    }
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
