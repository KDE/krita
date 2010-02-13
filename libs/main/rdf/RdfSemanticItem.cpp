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

#include "rdf/RdfSemanticItem.h"
#include "rdf/KoDocumentRdf.h"
#include "rdf/KoDocumentRdf_p.h"
#include "KoInlineObject.h"
#include "KoTextInlineRdf.h"
#include "KoTextRdfCore.h"
#include "KoTextEditor.h"
#include "KoCanvasBase.h"
#include "KoToolProxy.h"
#include "KoBookmark.h"
#include "KoTextMeta.h"
#include "KoTextDocument.h"

#include <kdebug.h>
#include <QUuid>

#include "KoChangeTrackerDisabledRAII.h"

using namespace Soprano;

RdfSemanticItem::RdfSemanticItem(QObject* parent, KoDocumentRdf* m_rdf)
        : QObject(parent),
        m_rdf(m_rdf)
{
}

RdfSemanticItem::RdfSemanticItem(QObject* parent, KoDocumentRdf* m_rdf, Soprano::QueryResultIterator& it)
        : QObject(parent),
        m_rdf(m_rdf)
{
    m_context = it.binding("graph");
    kDebug(30015) << "RdfSemanticItem() context:" << m_context.toString();
}

RdfSemanticItem::~RdfSemanticItem()
{
}

KoDocumentRdf* RdfSemanticItem::DocumentRdf() const
{
    return m_rdf;
}


QStringList RdfSemanticItem::xmlIdList() const
{
    QStringList ret;

    Soprano::Node linksubj = linkingSubject();
    StatementIterator it = m_rdf->model()->listStatements(
                               linksubj,
                               Node::createResourceNode(QUrl("http://docs.oasis-open.org/opendocument/meta/package/common#idref")),
                               Node(),
                               m_rdf->manifestRdfNode());
    QList<Statement> allStatements = it.allElements();
    Q_FOREACH(Soprano::Statement s, allStatements) {
        QString xmlid = s.object().toString();
        ret << xmlid;
    }
    return ret;
}

RdfSemanticTreeWidgetItem* RdfSemanticItem::createQTreeWidgetItem(QTreeWidgetItem* parent)
{
    Q_UNUSED(parent);
    return 0;
}

void RdfSemanticItem::updateTriple_remove(const Soprano::LiteralValue& toModify,
        const QString& predString,
        const Soprano::Node& explicitLinkingSubject)
{
    Soprano::Model* m = m_rdf->model();
    Node pred = Node::createResourceNode(QUrl(predString));
    m->removeStatement(
        explicitLinkingSubject,
        pred,
        Node::createLiteralNode(toModify)
    );
    kDebug(30015) << "Rdf.del subj:" << explicitLinkingSubject;
    kDebug(30015) << "Rdf.del pred:" << pred;
    kDebug(30015) << "Rdf.del  obj:" << Node::createLiteralNode(toModify);
    kDebug(30015) << "Rdf.del  ctx:" << context();
    //
    // Typeless remove, I found that if a object literal did not
    // stipulate its type in the input Rdf, just using
    // removeStatement() above might not pick it up. So the below code
    // looks through all statements with subj+pred and checks typeless
    // string identity of the object() and removes it if strings match.
    //
    StatementIterator it = m->listStatements(explicitLinkingSubject, pred, Node());
    QList<Statement> removeList;
    QList<Statement> allStatements = it.allElements();
    Q_FOREACH(Soprano::Statement s, allStatements) {
        kDebug(30015) << "typeless remove,  s:" << s.object().toString();
        kDebug(30015) << "typeless remove, tm:" << Node::createLiteralNode(toModify).toString();

        if (s.object().toString() == Node::createLiteralNode(toModify).toString()) {
            removeList << s;
        }
        //
        // Sometimes the object value is serialized as 51.47026 or
        // 5.1470260000e+01. There are also slight rounding errors
        // which are introduced that complicate comparisons.
        //
        if (toModify.isDouble()) {
            removeList << s;
        }
    }
    m->removeStatements(removeList);
}

void RdfSemanticItem::updateTriple_add(const Soprano::LiteralValue& toModify,
                                       const QString& predString,
                                       const Soprano::Node& explicitLinkingSubject)
{
    Soprano::Model* m = m_rdf->model();
    Node pred = Node::createResourceNode(QUrl(predString));

    kDebug(30015) << "Rdf.add subj:" << explicitLinkingSubject;
    kDebug(30015) << "Rdf.add pred:" << pred;
    kDebug(30015) << "Rdf.add  obj:" << Node::createLiteralNode(toModify);
    kDebug(30015) << "Rdf.add  ctx:" << context();
    m->addStatement(
        explicitLinkingSubject,
        pred,
        Node::createLiteralNode(toModify),
        context());
}


void RdfSemanticItem::updateTriple(QString& toModify, const QString& newValue, const QString& predString)
{
    kDebug(30015) << "tomod:" << toModify << " newV:" << newValue << " pred:" << predString;
    updateTriple_remove(toModify, predString, linkingSubject());
    toModify = newValue;
    updateTriple_add(toModify, predString, linkingSubject());
}

void RdfSemanticItem::updateTriple(KDateTime& toModify, const KDateTime& newValue, const QString& predString)
{
    updateTriple_remove(Soprano::LiteralValue(toModify.dateTime()),
                        predString, linkingSubject());
    toModify = newValue;
    updateTriple_add(Soprano::LiteralValue(toModify.dateTime()),
                     predString, linkingSubject());
}

void RdfSemanticItem::updateTriple(double& toModify,
                                   double    newValue,
                                   const QString& predString,
                                   const Soprano::Node& explicitLinkingSubject)
{
    updateTriple_remove(Soprano::LiteralValue(toModify), predString, explicitLinkingSubject);
    toModify = newValue;
    updateTriple_add(Soprano::LiteralValue(toModify), predString, explicitLinkingSubject);
}

void RdfSemanticItem::setRdfType(const QString& t)
{
    Soprano::Model* m = m_rdf->model();
    Node pred = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#type"));
    m->addStatement(
        linkingSubject(),
        pred,
        Node::createResourceNode(t),
        context());
}

void RdfSemanticItem::importFromDataComplete(const QByteArray& ba, KoDocumentRdf* m_rdf, KoCanvasBase* host)
{
    Q_UNUSED(ba);

    // Create and populate and editor with the current data,
    // then update the Rdf from that editor.
    QWidget* parent = 0;
    QWidget* objectEditor = createEditor(parent);
    updateFromEditorData();

    if (host) {
        insert(host);
    }
    delete objectEditor;

    if (m_rdf) {
        m_rdf->emitSemanticObjectAdded(this);
    }
}

Soprano::Node RdfSemanticItem::linkingSubject() const
{
    return Node::createEmptyNode();
}

Soprano::Node RdfSemanticItem::context()
{
    if (m_context.isValid()) {
        return m_context;
    }
    return m_rdf->manifestRdfNode();
}

void RdfSemanticItem::setupStylesheetReplacementMapping(QMap< QString, QString >& m)
{
    Q_UNUSED(m);
}

void RdfSemanticItem::exportToMime(QMimeData* md)
{
    Q_UNUSED(md);
}

void RdfSemanticItem::insert(KoCanvasBase* host)
{
    kDebug(30015) << "insert...";
    KoTextEditor* editor = KoDocumentRdf::ensureTextTool(host);
    Q_ASSERT(editor);
    Q_ASSERT(editor->document());

    KoTextDocument ktd(editor->document());
    KoChangeTrackerDisabledRAII disableChangeTracker(ktd.changeTracker());

    KoTextMeta* startmark = new KoTextMeta(editor->document());
    editor->insertInlineObject(startmark);
    KoTextInlineRdf* inlineRdf(
        new KoTextInlineRdf((QTextDocument*)editor->document(), startmark));

    // generate an xml:id for inlineRdf
    // set it and also insert some Rdf into manifest.rdf to link
    // the new xml:id to the foaf Rdf data.
    QString newID = inlineRdf->createXmlId();
    inlineRdf->setXmlId(newID);
    startmark->setInlineRdf(inlineRdf);

    if (m_rdf) {
        Soprano::Statement st(
            linkingSubject(),
            Node::createResourceNode(QUrl("http://docs.oasis-open.org/opendocument/meta/package/common#idref")),
            Node::createLiteralNode(newID),
            m_rdf->manifestRdfNode());
        m_rdf->model()->addStatement(st);
        kDebug(30015) << "RdfSemanticItem::insert() HAVE m_rdf, linking statement:" << st;

        Soprano::Model* model(Soprano::createModel());
        m_rdf->addStatements(model, newID);
        kDebug(30015) << "RdfSemanticItem::insert() returned model size:" << model->statementCount();
        m_rdf->rememberNewInlineRdfObject(inlineRdf);
        delete model;
    } else {
        kDebug(30015) << "RdfSemanticItem::insert() m_rdf is not set!";
    }

    //
    // Use stylesheets to format the display of the newly inserted
    // semitem. To do this, there needs to be a start and end KoTextMeta
    // and they have to be properly linked. So first the name() is inserted
    // and the endmark inserted so that the stylesheet system has a valid
    // begin <-> end range. To make the docker happy, we move the cursor back
    // over the end marker so that the cursor is within the newly inserted
    // semitem.
    //
    editor->insertText(name());

    KoTextMeta* endmark = new KoTextMeta(editor->document());
    editor->insertInlineObject(endmark);
    startmark->setEndBookmark(endmark);

    editor->setPosition(editor->position() - 1, QTextCursor::MoveAnchor);
    SemanticStylesheet *ss = defaultStylesheet();
    RdfSemanticItemViewSite vs(this, newID);
    vs.applyStylesheet(editor, ss);
}

QStringList RdfSemanticItem::classNames()
{
    QStringList ret;
    ret << "Contact";
    ret << "Event";
    ret << "Location";
    return ret;
}

RdfSemanticItem* RdfSemanticItem::createSemanticItem(QObject* parent, KoDocumentRdf* m_rdf, const QString& klass)
{
    if (klass == "Contact") {
        return new RdfFoaF(parent, m_rdf);
    }
    if (klass == "Event") {
        return new RdfCalendarEvent(parent, m_rdf);
    }
    if (klass == "Location") {
        return new RdfLocation(parent, m_rdf);
    }
    return 0;
}

SemanticStylesheet *RdfSemanticItem::findStylesheetByUuid(const QString& id)
{
    SemanticStylesheet *ret = 0;
    if (id.isEmpty()) {
        return ret;
    }
    foreach (SemanticStylesheet *ss, stylesheets()) {
        if (ss->uuid() == id) {
            return ss;
        }
    }
    foreach (SemanticStylesheet *ss, userStylesheets()) {
        if (ss->uuid() == id) {
            return ss;
        }
    }
    return ret;
}

SemanticStylesheet *RdfSemanticItem::findStylesheetByName(const QList<SemanticStylesheet*>& ssheets,
        const QString& n)
{
    SemanticStylesheet *ret = 0;
    foreach (SemanticStylesheet *ss, ssheets) {
        if (ss->name() == n) {
            return ss;
        }
    }
    return ret;
}

SemanticStylesheet *RdfSemanticItem::findStylesheetByName(const QString& sheetType, const QString& n)
{
    if (sheetType == "System") {
        return findStylesheetByName(stylesheets(), n);
    }
    return findStylesheetByName(userStylesheets(), n);
}


SemanticStylesheet *RdfSemanticItem::defaultStylesheet()
{
    QString klass = metaObject()->className();
    QString name = getProperty(DocumentRdf()->model(),
                               Node::createResourceNode(QUrl("http://kogmbh.net/rdf/document/" + klass)),
                               Node::createResourceNode(QUrl("http://kogmbh.net/rdf/stylesheet")),
                               "name");
    QString type = getProperty(DocumentRdf()->model(),
                               Node::createResourceNode(QUrl("http://kogmbh.net/rdf/document/" + klass)),
                               Node::createResourceNode(QUrl("http://kogmbh.net/rdf/stylesheet-type")),
                               SemanticStylesheet::TYPE_SYSTEM);
    QString uuid = getProperty(DocumentRdf()->model(),
                               Node::createResourceNode(QUrl("http://kogmbh.net/rdf/document/" + klass)),
                               Node::createResourceNode(QUrl("http://kogmbh.net/rdf/stylesheet-uuid")),
                               "");
    kDebug(30015) << "name:" << name << " type:" << type << "\n uuid:" << uuid;
    SemanticStylesheet *ret = findStylesheetByUuid(uuid);
    if (!ret) {
        ret = findStylesheetByName(type, name);
    }
    if (!ret) {
        // The "name" stylesheet should always exist
        ret = findStylesheetByName(SemanticStylesheet::TYPE_SYSTEM, "name");
    }
    Q_ASSERT(ret);
    return ret;
}

void RdfSemanticItem::defaultStylesheet(SemanticStylesheet *ss)
{
    KoDocumentRdf* rdf = DocumentRdf();
    Soprano::Model* m = rdf->model();
    QString uuid = ss->uuid();
    QString name = ss->name();
    QString klass = metaObject()->className();
    m->removeAllStatements(
        Statement(Node::createResourceNode(QUrl("http://kogmbh.net/rdf/document/" + klass)),
                  Node::createResourceNode(QUrl("http://kogmbh.net/rdf/stylesheet")),
                  Node()));
    m->addStatement(Node::createResourceNode(QUrl("http://kogmbh.net/rdf/document/" + klass)),
                    Node::createResourceNode(QUrl("http://kogmbh.net/rdf/stylesheet")),
                    Node::createLiteralNode(name),
                    rdf->manifestRdfNode());
    m->removeAllStatements(
        Statement(Node::createResourceNode(QUrl("http://kogmbh.net/rdf/document/" + klass)),
                  Node::createResourceNode(QUrl("http://kogmbh.net/rdf/stylesheet-type")),
                  Node()));
    m->addStatement(Node::createResourceNode(QUrl("http://kogmbh.net/rdf/document/" + klass)),
                    Node::createResourceNode(QUrl("http://kogmbh.net/rdf/stylesheet-type")),
                    Node::createLiteralNode(name),
                    rdf->manifestRdfNode());
    m->removeAllStatements(
        Statement(Node::createResourceNode(QUrl("http://kogmbh.net/rdf/document/" + klass)),
                  Node::createResourceNode(QUrl("http://kogmbh.net/rdf/stylesheet-uuid")),
                  Node()));
    m->addStatement(Node::createResourceNode(QUrl("http://kogmbh.net/rdf/document/" + klass)),
                    Node::createResourceNode(QUrl("http://kogmbh.net/rdf/stylesheet-uuid")),
                    Node::createLiteralNode(uuid),
                    rdf->manifestRdfNode());
}

SemanticStylesheet *RdfSemanticItem::createUserStylesheet(const QString& name, const QString& templateString)
{
    bool isMutable = true;
    SemanticStylesheet *ss =
        new SemanticStylesheet(QUuid::createUuid().toString(),
                               name, templateString,
                               SemanticStylesheet::TYPE_USER,
                               isMutable);
    userStylesheets() << ss;
    connect(ss, SIGNAL(nameChanging(SemanticStylesheet*, QString, QString)),
            this, SLOT(onUserStylesheetRenamed(SemanticStylesheet*, QString, QString)));
    return ss;
}

void RdfSemanticItem::onUserStylesheetRenamed(SemanticStylesheet *ss, QString oldName, QString newName)
{
    Q_UNUSED(ss);
    Q_UNUSED(oldName);
    Q_UNUSED(newName);
}

void RdfSemanticItem::destroyUserStylesheet(SemanticStylesheet *ss)
{
    userStylesheets().removeAll(ss);
    ss = 0;
}

void RdfSemanticItem::loadUserStylesheets(Soprano::Model* model)
{
    QString klass = metaObject()->className();
    QString nodePrefix = "http://kogmbh.net/rdf/user-stylesheets/" + klass + "/";
    Node rdfNil = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"));
    Node rdfFirst = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#first"));
    Node rdfRest = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"));

    Soprano::Node ListHeadSubject = Node::createResourceNode(QUrl(nodePrefix + "list"));
    QList<Statement> listNodes = KoTextRdfCore::loadList(model, ListHeadSubject);
    kDebug(30015) << "klass:" << klass << " listNodes.sz:" << listNodes.size();
    foreach (Soprano::Statement s, listNodes) {
        Soprano::Node dataBNode = s.object();

        QString uuid = KoTextRdfCore::getObject(model, dataBNode,
                                                Node::createResourceNode(QUrl(nodePrefix + "uuid"))).toString();
        QString name = KoTextRdfCore::getObject(model, dataBNode,
                                                Node::createResourceNode(QUrl(nodePrefix + "name"))).toString();
        QString templateString = KoTextRdfCore::getObject(model, dataBNode,
                                 Node::createResourceNode(QUrl(nodePrefix + "template"))).toString();
        kDebug(30015) << "dataBNode:" << dataBNode;
        kDebug(30015) << "loading name:" << name << " template:" << templateString;

        if (findStylesheetByName(userStylesheets(), name)) {
            continue;
        }

        bool isMutable = true;
        SemanticStylesheet *ss =
            new SemanticStylesheet(uuid, name, templateString,
                                   SemanticStylesheet::TYPE_USER,
                                   isMutable);
        userStylesheets() << ss;
        connect(ss, SIGNAL(nameChanging(SemanticStylesheetPtr, QString, QString)),
                this, SLOT(onUserStylesheetRenamed(SemanticStylesheetPtr, QString, QString)));
    }
}

void RdfSemanticItem::saveUserStylesheets(Soprano::Model* model, const Soprano::Node& context)
{
    QString klass = metaObject()->className();
    QString nodePrefix = "http://kogmbh.net/rdf/user-stylesheets/" + klass + "/";
    Node rdfNil = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"));
    Node rdfFirst = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#first"));
    Node rdfRest = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"));

    Soprano::Node dataBNode = model->createBlankNode();
    QList< Soprano::Node > dataBNodeList;

    QList<SemanticStylesheet*>& ssl = userStylesheets();
    foreach (SemanticStylesheet *ss, ssl) {
        kDebug(30015) << "saving sheet:" << ss->name();

        dataBNode = model->createBlankNode();
        model->addStatement(
            dataBNode,
            Node::createResourceNode(QUrl(nodePrefix + "uuid")),
            Node::createLiteralNode(ss->uuid()),
            context);
        model->addStatement(
            dataBNode,
            Node::createResourceNode(QUrl(nodePrefix + "name")),
            Node::createLiteralNode(ss->name()),
            context);
        model->addStatement(
            dataBNode,
            Node::createResourceNode(QUrl(nodePrefix + "template")),
            Node::createLiteralNode(ss->templateString()),
            context);
        dataBNodeList << dataBNode;
    }

    kDebug(30015) << "saving list, size:" << dataBNodeList.size();
    Soprano::Node ListHeadSubject = Node::createResourceNode(QUrl(nodePrefix + "list"));
    KoTextRdfCore::saveList(model, ListHeadSubject, dataBNodeList, context);
}

Soprano::Node RdfSemanticItem::createNewUUIDNode()
{
    QString uuid = QUuid::createUuid().toString();
    uuid.replace("{", "");
    uuid.replace("}", "");
    QString nodestr = "http://kogmbh.net/uuidnode/" + uuid;
    return Node::createResourceNode(nodestr);
}
