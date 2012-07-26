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

#include "KoTextRdfCore.h"
#include <kdebug.h>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>
#include <QFile>
using namespace Soprano;

bool KoTextRdfCore::saveRdf( QSharedPointer<Soprano::Model> model, Soprano::StatementIterator triples, KoStore *store, KoXmlWriter *manifestWriter, const QString &fileName)
{
    bool ok = false;

    if (! store->open(fileName)) {
        return false;
    }

    KoStoreDevice dev(store);
    QTextStream oss(&dev);

    QString serialization = "application/rdf+xml";
    const Soprano::Serializer* serializer
            = Soprano::PluginManager::instance()->discoverSerializerForSerialization(
                    Soprano::SerializationRdfXml);

    if (serializer) {
        QString data;
        QTextStream tss(&data);
        if (serializer->serialize(triples, tss, Soprano::SerializationRdfXml)) {
            tss.flush();
            oss << data;
            kDebug(30015) << "fileName:" << fileName << " data.sz:" << data.size();
            kDebug(30015) << "model.sz:" << model->statementCount();
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

bool KoTextRdfCore::createAndSaveManifest(QSharedPointer<Soprano::Model> docmodel, const QMap<QString, QString> &idmap, KoStore *store, KoXmlWriter *manifestWriter)
{
    QSharedPointer<Soprano::Model> tmpmodel(Soprano::createModel());
    QMap<QString, QString>::const_iterator iditer = idmap.constBegin();
    QMap<QString, QString>::const_iterator idend = idmap.constEnd();
    for (; iditer != idend; ++iditer) {
        QString oldID = iditer.key();
        QString newID = iditer.value();
        kDebug(30015) << "oldID:" << oldID << " newID:" << newID;
        QString sparqlQuery;
        QTextStream queryss(&sparqlQuery);
        queryss << ""
            << "prefix rdf:  <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
            << "prefix foaf: <http://xmlns.com/foaf/0.1/> \n"
            << "prefix pkg:  <http://docs.oasis-open.org/opendocument/meta/package/common#> \n"
            << ""
            << "select ?s ?p ?o \n"
            << "where { \n"
            << " ?s pkg:idref ?xmlid . \n"
            << " ?s ?p ?o . \n"
            << " filter( str(?xmlid) = \"" << oldID << "\" ) \n"
            << "}\n";

        Soprano::QueryResultIterator it =
            docmodel->executeQuery(sparqlQuery,
                                   Soprano::Query::QueryLanguageSparql);
        while (it.next()) {
            Soprano::Node pred = it.binding("p");
            Soprano::Node obj  = it.binding("o");
            if (pred.toString() == "http://docs.oasis-open.org/opendocument/meta/package/common#idref") {
                kDebug(30015) << "changing idref, oldID:" << oldID << " newID:" << newID;
                obj = Node::createLiteralNode(newID);
            }
            Statement s(it.binding("s"), pred, obj);
            tmpmodel->addStatement(s);
        }
    }
    kDebug(30015) << "exporting triples model.sz:" << tmpmodel->statementCount();
    // save tmpmodel as manifest.rdf in C+P ODF file.
    Soprano::StatementIterator triples = tmpmodel->listStatements();
    bool ret = saveRdf(tmpmodel, triples, store, manifestWriter, "manifest.rdf");
    return ret;
}

bool KoTextRdfCore::loadManifest(KoStore *store, QSharedPointer<Soprano::Model> model)
{
    bool ok = true;
    QString fileName = "manifest.rdf";
    if (!store->open(fileName)) {
        kDebug(30003) << "Entry " << fileName << " not found!";
        return false;
    }
    Soprano::Node context(QUrl("http://www.calligra.org/Rdf/path/" + fileName));
    QUrl BaseURI = QUrl("");
    kDebug(30015) << "Loading external Rdf/XML from:" << fileName;

    QString rdfxmlData(store->device()->readAll());
    const Soprano::Parser *parser =
        Soprano::PluginManager::instance()->discoverParserForSerialization(
            Soprano::SerializationRdfXml);
    Soprano::StatementIterator it = parser->parseString(rdfxmlData,
                                    BaseURI,
                                    Soprano::SerializationRdfXml);
    QList<Statement> allStatements = it.allElements();
    kDebug(30015) << "Found " << allStatements.size() << " triples...";
    foreach (Soprano::Statement s, allStatements) {
        Error::ErrorCode err = model->addStatement(s.subject(), s.predicate(),
                s.object(), context);
        if (err != Error::ErrorNone) {
            kDebug(30015) << "Error adding triple! s:" << s.subject()
                << " p:" << s.predicate()
                << " o:" << s.object();
            ok = false;
            break;
        }
    }
    store->close();
    return ok;
}

void KoTextRdfCore::dumpModel(const QString &msg, QSharedPointer<Soprano::Model> m)
{
#ifndef NDEBUG
    QList<Soprano::Statement> allStatements = m->listStatements().allElements();
    kDebug(30015) << "----- " << msg << " ----- model size:" << allStatements.size() << endl;
    foreach (Soprano::Statement s, allStatements) {
        kDebug(30015) << s;
    }
#else
    Q_UNUSED(msg);
    Q_UNUSED(m);
#endif
}

QList<Soprano::Statement> KoTextRdfCore::loadList(QSharedPointer<Soprano::Model> model, Soprano::Node ListHeadSubject)
{
    Node rdfNil = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"));
    Node rdfFirst = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#first"));
    Node rdfRest = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"));

    Soprano::Node listBNode = ListHeadSubject;
    QList<Statement> ret;
    kDebug(30015) << "finding all nodes in the list...";
    while (true) {
        ret << model->listStatements(listBNode, rdfFirst, Node()).allElements();
        Soprano::Node obj = KoTextRdfCore::getObject(model, listBNode, rdfRest);
        kDebug(30015) << "ret:" << ret;
        kDebug(30015) << "rest:" << obj;
        if (!obj.isValid()) {
            break;
        }
        if (obj.toString() == rdfNil.toString()) {
            break;
        }
        listBNode = obj;
    }
    return ret;
}

static void removeList(QSharedPointer<Soprano::Model> model, Soprano::Node ListHeadSubject)
{
    Node rdfNil = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"));
    Node rdfFirst = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#first"));
    Node rdfRest = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"));
    //
    // Chain down the list recursively so we delete from
    // the list tail back to the head.
    //
    Soprano::Node obj = KoTextRdfCore::getObject(model, ListHeadSubject, rdfRest);
    if (obj.isValid()) {
        if (obj.toString() != rdfNil.toString()) {
            removeList(model, obj);
        }
    }
    model->removeAllStatements(ListHeadSubject, rdfFirst, Node());
    model->removeAllStatements(ListHeadSubject, rdfRest, Node());
}

void KoTextRdfCore::saveList(QSharedPointer<Soprano::Model> model, Soprano::Node ListHeadSubject, QList<Soprano::Node> &dataBNodeList, Soprano::Node context)
{
    Node rdfNil = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"));
    Node rdfFirst = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#first"));
    Node rdfRest = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"));

    kDebug(30015) << "header:" << ListHeadSubject.toString();
    kDebug(30015) << "list.sz:" << dataBNodeList.size();
    kDebug(30015) << "context:" << context.toString();

    removeList(model, ListHeadSubject);

    Soprano::Node listBNode = ListHeadSubject;
    Soprano::Node prevListBNode;

    foreach (Soprano::Node dataBNode, dataBNodeList) {
        // Link the list in Rdf
        model->addStatement(listBNode, rdfFirst, dataBNode, context);
        if (prevListBNode.isValid()) {
            kDebug(30015) << "prev:" << prevListBNode << " current:" << listBNode;
            model->addStatement(prevListBNode, rdfRest, listBNode, context);
        }
        prevListBNode = listBNode;
        listBNode = model->createBlankNode();
    }

    kDebug(30015) << "at end, prev.isValid:" << prevListBNode.isValid();
    if (prevListBNode.isValid()) {
        model->addStatement(prevListBNode, rdfRest, listBNode, context);
    }
    model->addStatement(listBNode, rdfRest, rdfNil, context);
}

void KoTextRdfCore::removeStatementsIfTheyExist( QSharedPointer<Soprano::Model> m, const QList<Soprano::Statement> &removeList)
{
    foreach (Soprano::Statement s, removeList) {
        StatementIterator it = m->listStatements(s.subject(), s.predicate(), s.object(), s.context());
        QList<Statement> allStatements = it.allElements();
        Q_FOREACH(Soprano::Statement z, allStatements) {
            kDebug(30015) << "found:" << z;
            m->removeStatement(z);
        }
    }
}

Soprano::Node KoTextRdfCore::getObject(QSharedPointer<Soprano::Model> model, Soprano::Node s, Soprano::Node p)
{
    QList<Statement> all;
    all = model->listStatements(s, p, Node()).allElements();
    if (all.isEmpty()) {
        return Soprano::Node();
    }
    return all.first().object();
}

QByteArray KoTextRdfCore::fileToByteArray(const QString &fileName)
{
    QFile t(fileName);
    t.open(QIODevice::ReadOnly);
    return t.readAll();
}

QString KoTextRdfCore::getProperty(QSharedPointer<Soprano::Model> m, Soprano::Node subj, Soprano::Node pred, const QString &defval)
{
    StatementIterator it = m->listStatements(subj, pred, Node());
    QList<Statement> allStatements = it.allElements();
    foreach (Soprano::Statement s, allStatements) {
        return s.object().toString();
    }
    return defval;
}

QString KoTextRdfCore::optionalBindingAsString(Soprano::QueryResultIterator &it, const QString &bindingName, const QString &def)
{
    if (it.binding(bindingName).isValid()) {
        return it.binding(bindingName).toString();
    }
    return def;
}
