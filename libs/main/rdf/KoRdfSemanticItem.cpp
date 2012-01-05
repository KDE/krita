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

#include "KoRdfSemanticItem.h"
#include "KoDocumentRdf.h"
#include "KoRdfFoaF.h"
#include "KoRdfCalendarEvent.h"
#include "KoRdfLocation.h"
#include "KoDocumentRdf.h"

#include <KoInlineObject.h>
#include <KoTextInlineRdf.h>
#include <KoTextRdfCore.h>
#include <KoCanvasBase.h>
#include <KoTextDocument.h>
#include <KoTextEditor.h>
#include <KoBookmark.h>
#include <KoTextMeta.h>
#include <KoTextDocument.h>

#include <kdebug.h>
#include <QUuid>

#include "KoChangeTrackerDisabledRAII.h"

using namespace Soprano;

KoRdfSemanticItem::KoRdfSemanticItem(const KoDocumentRdf *rdf, QObject *parent)
        : QObject(parent)
        , m_rdf(rdf)
{
}


KoRdfSemanticItem::KoRdfSemanticItem(QObject *parent)
        : QObject(parent)
        , m_rdf(0)
{
}

KoRdfSemanticItem::KoRdfSemanticItem(const KoDocumentRdf *rdf, Soprano::QueryResultIterator &it, QObject *parent)
    : QObject(parent)
    , m_rdf(rdf)
{
    m_context = it.binding("graph");
    kDebug(30015) << "KoRdfSemanticItem() context:" << m_context.toString();
}

KoRdfSemanticItem::~KoRdfSemanticItem()
{
}

const KoDocumentRdf *KoRdfSemanticItem::documentRdf() const
{
    return m_rdf;
}


QStringList KoRdfSemanticItem::xmlIdList() const
{
    QStringList ret;

    Soprano::Node linksubj = linkingSubject();
    StatementIterator it = documentRdf()->model()->listStatements(
                               linksubj,
                               Node::createResourceNode(QUrl("http://docs.oasis-open.org/opendocument/meta/package/common#idref")),
                               Node(),
                               documentRdf()->manifestRdfNode());
    QList<Statement> allStatements = it.allElements();
    foreach (Soprano::Statement s, allStatements) {
        QString xmlid = s.object().toString();
        ret << xmlid;
    }
    return ret;
}

KoRdfSemanticTreeWidgetItem *KoRdfSemanticItem::createQTreeWidgetItem(QTreeWidgetItem *parent)
{
    Q_UNUSED(parent);
    return 0;
}

void KoRdfSemanticItem::updateTriple_remove(const Soprano::LiteralValue &toModify,
        const QString &predString,
        const Soprano::Node &explicitLinkingSubject)
{
    Soprano::Model *m = const_cast<Soprano::Model*>(documentRdf()->model());
    Node pred = Node::createResourceNode(QUrl(predString));
    m->removeStatement(explicitLinkingSubject,pred, Node::createLiteralNode(toModify));
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
    foreach (Soprano::Statement s, allStatements) {
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

void KoRdfSemanticItem::updateTriple_add(const Soprano::LiteralValue &toModify,
                                       const QString &predString,
                                       const Soprano::Node &explicitLinkingSubject)
{
    Soprano::Model *m = const_cast<Soprano::Model*>(documentRdf()->model());
    Node pred = Node::createResourceNode(QUrl(predString));

    kDebug(30015) << "Rdf.add subj:" << explicitLinkingSubject;
    kDebug(30015) << "Rdf.add pred:" << pred;
    kDebug(30015) << "Rdf.add  obj:" << Node::createLiteralNode(toModify);
    kDebug(30015) << "Rdf.add  ctx:" << context();
    m->addStatement(explicitLinkingSubject, pred, Node::createLiteralNode(toModify), context());
}


void KoRdfSemanticItem::updateTriple(QString &toModify, const QString &newValue, const QString &predString)
{
    kDebug(30015) << "tomod:" << toModify << " newV:" << newValue << " pred:" << predString;
    updateTriple_remove(toModify, predString, linkingSubject());
    toModify = newValue;
    updateTriple_add(toModify, predString, linkingSubject());
}

void KoRdfSemanticItem::updateTriple(KDateTime &toModify, const KDateTime &newValue, const QString &predString)
{
    updateTriple_remove(Soprano::LiteralValue(toModify.dateTime()),
                        predString, linkingSubject());
    toModify = newValue;
    updateTriple_add(Soprano::LiteralValue(toModify.dateTime()),
                     predString, linkingSubject());
}

void KoRdfSemanticItem::updateTriple(double &toModify,
                                   double newValue,
                                   const QString &predString,
                                   const Soprano::Node &explicitLinkingSubject)
{
    updateTriple_remove(Soprano::LiteralValue(toModify), predString, explicitLinkingSubject);
    toModify = newValue;
    updateTriple_add(Soprano::LiteralValue(toModify), predString, explicitLinkingSubject);
}

void KoRdfSemanticItem::setRdfType(const QString &t)
{
    Soprano::Model *m = const_cast<Soprano::Model*>(documentRdf()->model());
    Q_ASSERT(m);
    Node pred = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#type"));
    m->addStatement(linkingSubject(), pred, Node::createResourceNode(t), context());
}

void KoRdfSemanticItem::importFromDataComplete(const QByteArray &ba, const KoDocumentRdf *rdf, KoCanvasBase *host)
{
    Q_UNUSED(ba);

    // Create and populate and editor with the current data,
    // then update the Rdf from that editor.
    QWidget *parent = 0;
    QWidget *objectEditor = createEditor(parent);
    updateFromEditorData();

    if (host) {
        insert(host);
    }
    delete objectEditor;

    if (rdf) {
        rdf->emitSemanticObjectAdded(this);
    }
}

Soprano::Node KoRdfSemanticItem::linkingSubject() const
{
    return Node::createEmptyNode();
}

Soprano::Node KoRdfSemanticItem::context() const
{
    if (m_context.isValid()) {
        return m_context;
    }
    return documentRdf()->manifestRdfNode();
}

void KoRdfSemanticItem::setupStylesheetReplacementMapping(QMap<QString, QString> &m)
{
    Q_UNUSED(m);
}

void KoRdfSemanticItem::exportToMime(QMimeData *md) const
{
    Q_UNUSED(md);
}

void KoRdfSemanticItem::insert(KoCanvasBase *host)
{
    Q_UNUSED(host);
    kDebug(30015) << "insert...";
    KoTextEditor *editor = KoTextEditor::getTextEditorFromCanvas(host);

    Q_ASSERT(editor);
    Q_ASSERT(editor->document());

    KoTextDocument ktd(editor->document());
    KoChangeTrackerDisabledRAII disableChangeTracker(ktd.changeTracker());
    Q_UNUSED(disableChangeTracker);

    KoTextMeta *startmark = new KoTextMeta(editor->document());
    editor->insertInlineObject(startmark);
    KoTextInlineRdf *inlineRdf(new KoTextInlineRdf((QTextDocument*)editor->document(), startmark));

    // generate an xml:id for inlineRdf
    // set it and also insert some Rdf into manifest.rdf to link
    // the new xml:id to the foaf Rdf data.
    QString newID = inlineRdf->createXmlId();
    inlineRdf->setXmlId(newID);
    startmark->setInlineRdf(inlineRdf);

    if (documentRdf()) {
        Soprano::Statement st(
            linkingSubject(),
            Node::createResourceNode(QUrl("http://docs.oasis-open.org/opendocument/meta/package/common#idref")),
            Node::createLiteralNode(newID),
            documentRdf()->manifestRdfNode());
        const_cast<Soprano::Model*>(documentRdf()->model())->addStatement(st);
        kDebug(30015) << "KoRdfSemanticItem::insert() HAVE documentRdf(), linking statement:" << st;

        Soprano::Model* model(Soprano::createModel());
        const_cast<KoDocumentRdf*>(documentRdf())->addStatements(model, newID);
        kDebug(30015) << "KoRdfSemanticItem::insert() returned model size:" << model->statementCount();
        const_cast<KoDocumentRdf*>(documentRdf())->rememberNewInlineRdfObject(inlineRdf);
        delete model;
    } else {
        kDebug(30015) << "KoRdfSemanticItem::insert() documentRdf() is not set!";
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

    KoTextMeta *endmark = new KoTextMeta(editor->document());
    editor->insertInlineObject(endmark);
    startmark->setEndBookmark(endmark);

    editor->setPosition(editor->position() - 1, QTextCursor::MoveAnchor);
    KoSemanticStylesheet *ss = defaultStylesheet();
    KoRdfSemanticItemViewSite vs(this, newID);
    vs.applyStylesheet(editor, ss);

}

QStringList KoRdfSemanticItem::classNames()
{
    QStringList ret;
    ret << "Contact";
    ret << "Event";
    ret << "Location";
    return ret;
}

KoRdfSemanticItem *KoRdfSemanticItem::createSemanticItem(QObject *parent, const KoDocumentRdf *m_rdf, const QString &semanticClass)
{
    if (semanticClass == "Contact") {
        return new KoRdfFoaF(parent, m_rdf);
    }
    if (semanticClass == "Event") {
        return new KoRdfCalendarEvent(parent, m_rdf);
    }
    if (semanticClass == "Location") {
        return new KoRdfLocation(parent, m_rdf);
    }
    return 0;
}

QList<KoSemanticStylesheet*> KoRdfSemanticItem::userStylesheets() const
{
    return documentRdf()->userStyleSheetList(className());
}


KoSemanticStylesheet *KoRdfSemanticItem::findStylesheetByUuid(const QString &id) const
{
    KoSemanticStylesheet *ret = 0;
    if (id.isEmpty()) {
        return ret;
    }
    foreach (KoSemanticStylesheet *ss, stylesheets()) {
        if (ss->uuid() == id) {
            return ss;
        }
    }
    foreach (KoSemanticStylesheet *ss, userStylesheets()) {
        if (ss->uuid() == id) {
            return ss;
        }
    }
    return ret;
}

KoSemanticStylesheet *KoRdfSemanticItem::findStylesheetByName(const QList<KoSemanticStylesheet*> &ssheets,
        const QString &n) const
{
    KoSemanticStylesheet *ret = 0;
    foreach (KoSemanticStylesheet *ss, ssheets) {
        if (ss->name() == n) {
            return ss;
        }
    }
    return ret;
}

KoSemanticStylesheet *KoRdfSemanticItem::findStylesheetByName(const QString &sheetType, const QString &n) const
{
    if (sheetType == "System") {
        return findStylesheetByName(stylesheets(), n);
    }
    return findStylesheetByName(userStylesheets(), n);
}

KoSemanticStylesheet *KoRdfSemanticItem::defaultStylesheet() const
{
    QString semanticClass = metaObject()->className();
    Soprano::Model *m = const_cast<Soprano::Model*>(documentRdf()->model());
    QString name = KoTextRdfCore::getProperty(m,
                                              Node::createResourceNode(QUrl("http://calligra.org/rdf/document/" + semanticClass)),
                                              Node::createResourceNode(QUrl("http://calligra.org/rdf/stylesheet")),
                                              "name");
    QString type = KoTextRdfCore::getProperty(m,
                                              Node::createResourceNode(QUrl("http://calligra.org/rdf/document/" + semanticClass)),
                                              Node::createResourceNode(QUrl("http://calligra.org/rdf/stylesheet-type")),
                                              KoSemanticStylesheet::stylesheetTypeSystem());
    QString uuid = KoTextRdfCore::getProperty(m,
                                              Node::createResourceNode(QUrl("http://calligra.org/rdf/document/" + semanticClass)),
                                              Node::createResourceNode(QUrl("http://calligra.org/rdf/stylesheet-uuid")),
                                              QString());
    kDebug(30015) << "name:" << name << " type:" << type << "\n uuid:" << uuid;
    KoSemanticStylesheet *ret = findStylesheetByUuid(uuid);
    if (!ret) {
        ret = findStylesheetByName(type, name);
    }
    if (!ret) {
        // The "name" stylesheet should always exist
        ret = findStylesheetByName(KoSemanticStylesheet::stylesheetTypeSystem(), "name");
    }
    Q_ASSERT(ret);
    return ret;
}

void KoRdfSemanticItem::defaultStylesheet(KoSemanticStylesheet *ss)
{
    const KoDocumentRdf *rdf = documentRdf();
    Soprano::Model *m = const_cast<Soprano::Model*>(documentRdf()->model());
    QString uuid = ss->uuid();
    QString name = ss->name();
    QString semanticClass = metaObject()->className();
    m->removeAllStatements(
        Statement(Node::createResourceNode(QUrl("http://calligra.org/rdf/document/" + semanticClass)),
                  Node::createResourceNode(QUrl("http://calligra.org/rdf/stylesheet")),
                  Node()));
    m->addStatement(Node::createResourceNode(QUrl("http://calligra.org/rdf/document/" + semanticClass)),
                    Node::createResourceNode(QUrl("http://calligra.org/rdf/stylesheet")),
                    Node::createLiteralNode(name),
                    rdf->manifestRdfNode());
    m->removeAllStatements(
        Statement(Node::createResourceNode(QUrl("http://calligra.org/rdf/document/" + semanticClass)),
                  Node::createResourceNode(QUrl("http://calligra.org/rdf/stylesheet-type")),
                  Node()));
    m->addStatement(Node::createResourceNode(QUrl("http://calligra.org/rdf/document/" + semanticClass)),
                    Node::createResourceNode(QUrl("http://calligra.org/rdf/stylesheet-type")),
                    Node::createLiteralNode(name),
                    rdf->manifestRdfNode());
    m->removeAllStatements(
        Statement(Node::createResourceNode(QUrl("http://calligra.org/rdf/document/" + semanticClass)),
                  Node::createResourceNode(QUrl("http://calligra.org/rdf/stylesheet-uuid")),
                  Node()));
    m->addStatement(Node::createResourceNode(QUrl("http://calligra.org/rdf/document/" + semanticClass)),
                    Node::createResourceNode(QUrl("http://calligra.org/rdf/stylesheet-uuid")),
                    Node::createLiteralNode(uuid),
                    rdf->manifestRdfNode());
}

KoSemanticStylesheet *KoRdfSemanticItem::createUserStylesheet(const QString &name, const QString &templateString)
{
    bool isMutable = true;
    KoSemanticStylesheet *ss =
        new KoSemanticStylesheet(QUuid::createUuid().toString(),
                                 name, templateString,
                                 KoSemanticStylesheet::stylesheetTypeUser(),
                                 isMutable);
    QList<KoSemanticStylesheet*> userSheets = userStylesheets();
    userSheets << ss;
    const_cast<KoDocumentRdf*>(documentRdf())->setUserStyleSheetList(className(),userSheets);
    connect(ss, SIGNAL(nameChanging(KoSemanticStylesheet*, QString, QString)),
            this, SLOT(onUserStylesheetRenamed(KoSemanticStylesheet*, QString, QString)));
    return ss;
}

void KoRdfSemanticItem::onUserStylesheetRenamed(KoSemanticStylesheet *ss, const QString &oldName, const QString &newName)
{
    Q_UNUSED(ss);
    Q_UNUSED(oldName);
    Q_UNUSED(newName);
}

void KoRdfSemanticItem::destroyUserStylesheet(KoSemanticStylesheet *ss)
{
    QList<KoSemanticStylesheet*> userSheets = userStylesheets();
    userSheets.removeAll(ss);
    const_cast<KoDocumentRdf*>(documentRdf())->setUserStyleSheetList(className(),userSheets);
}

void KoRdfSemanticItem::loadUserStylesheets(Soprano::Model *model)
{
    QString semanticClass = metaObject()->className();
    QString nodePrefix = "http://calligra.org/rdf/user-stylesheets/" + semanticClass + "/";
    Node rdfNil = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"));
    Node rdfFirst = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#first"));
    Node rdfRest = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"));

    Soprano::Node ListHeadSubject = Node::createResourceNode(QUrl(nodePrefix + "list"));
    QList<Statement> listNodes = KoTextRdfCore::loadList(model, ListHeadSubject);
    kDebug(30015) << "semanticClass:" << semanticClass << " listNodes.sz:" << listNodes.size();
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
        KoSemanticStylesheet *ss =
            new KoSemanticStylesheet(uuid, name, templateString,
                                   KoSemanticStylesheet::stylesheetTypeUser(),
                                   isMutable);
        QList<KoSemanticStylesheet*> userSheets = userStylesheets();
        userSheets << ss;
        const_cast<KoDocumentRdf*>(documentRdf())->setUserStyleSheetList(className(),userSheets);
        connect(ss, SIGNAL(nameChanging(KoSemanticStylesheetPtr, QString, QString)),
                this, SLOT(onUserStylesheetRenamed(KoSemanticStylesheetPtr, QString, QString)));
    }
}

void KoRdfSemanticItem::saveUserStylesheets(Soprano::Model *model, const Soprano::Node &context) const
{
    QString semanticClass = metaObject()->className();
    QString nodePrefix = "http://calligra.org/rdf/user-stylesheets/" + semanticClass + "/";
    Node rdfNil = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"));
    Node rdfFirst = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#first"));
    Node rdfRest = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"));

    Soprano::Node dataBNode = model->createBlankNode();
    QList< Soprano::Node > dataBNodeList;

    QList<KoSemanticStylesheet*> ssl = userStylesheets();
    foreach (KoSemanticStylesheet *ss, ssl) {
        kDebug(30015) << "saving sheet:" << ss->name();

        dataBNode = model->createBlankNode();
        model->addStatement(dataBNode, Node::createResourceNode(QUrl(nodePrefix + "uuid")),
            Node::createLiteralNode(ss->uuid()), context);
        model->addStatement(dataBNode, Node::createResourceNode(QUrl(nodePrefix + "name")),
            Node::createLiteralNode(ss->name()), context);
        model->addStatement(dataBNode, Node::createResourceNode(QUrl(nodePrefix + "template")),
            Node::createLiteralNode(ss->templateString()), context);
        dataBNodeList << dataBNode;
    }

    kDebug(30015) << "saving list, size:" << dataBNodeList.size();
    Soprano::Node ListHeadSubject = Node::createResourceNode(QUrl(nodePrefix + "list"));
    KoTextRdfCore::saveList(model, ListHeadSubject, dataBNodeList, context);
}

Soprano::Node KoRdfSemanticItem::createNewUUIDNode() const
{
    QString uuid = QUuid::createUuid().toString();
    uuid.remove('{');
    uuid.remove('}');
    QString nodestr = "http://calligra.org/uuidnode/" + uuid;
    return Node::createResourceNode(nodestr);
}

