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

//Calligra
#include <KoCanvasBase.h>
#include <KoCanvasResourceManager.h>
#include <KoChangeTrackerDisabledRAII.h>
#include <KoDocumentRdf.h>
#include <KoTextDocument.h>
#include <KoTextEditor.h>
#include <KoText.h>
#include <KoTextInlineRdf.h>
#include <KoTextMeta.h>
#include <KoTextRdfCore.h>

//KDE
#include <KDebug>

//QT
#include <QUuid>
#include <QWidget>

using namespace Soprano;

KoRdfSemanticItem::KoRdfSemanticItem(QObject *parent)
    : KoRdfBasicSemanticItem(parent)
{
}

KoRdfSemanticItem::KoRdfSemanticItem(QObject *parent, const KoDocumentRdf *rdf)
    : KoRdfBasicSemanticItem(parent, rdf)
{
}

KoRdfSemanticItem::KoRdfSemanticItem(QObject *parent, const KoDocumentRdf *rdf, Soprano::QueryResultIterator &it)
    : KoRdfBasicSemanticItem(parent, rdf, it)
{
}

KoRdfSemanticItem::~KoRdfSemanticItem()
{
}

KoRdfSemanticTreeWidgetItem *KoRdfSemanticItem::createQTreeWidgetItem(QTreeWidgetItem *parent)
{
    Q_UNUSED(parent);
    return 0;
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
        rdf->emitSemanticObjectAdded(hKoRdfSemanticItem(this));
    }
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

//    int originalpos = editor->position();
    KoTextMeta *startmark = new KoTextMeta(editor->document());
    editor->insertInlineObject(startmark);
    KoTextInlineRdf *inlineRdf(new KoTextInlineRdf((QTextDocument*)editor->document(), startmark));

    // generate an xml:id for inlineRdf
    // set it and also insert some Rdf into manifest.rdf to link
    // the new xml:id to the foaf Rdf data.
    QString newID = inlineRdf->createXmlId();
    inlineRdf->setXmlId(newID);
    startmark->setInlineRdf(inlineRdf);

    // we could do a paragraph relayout to update the position() values
    // of the start and end, but this is more efficient.
    startmark->updatePosition( (QTextDocument*)editor->document(),
                               startmark->position()-1,
                               QTextCharFormat() );
    if (documentRdf()) {
        Soprano::Statement st(
            linkingSubject(),
            Node::createResourceNode(QUrl("http://docs.oasis-open.org/ns/office/1.2/meta/pkg#idref")),
            Node::createLiteralNode(newID),
            documentRdf()->manifestRdfNode());
        documentRdf()->model()->addStatement(st);
        kDebug(30015) << "KoRdfSemanticItem::insert() HAVE documentRdf(), linking statement:" << st;

        QSharedPointer<Soprano::Model> model(Soprano::createModel());
        const_cast<KoDocumentRdf*>(documentRdf())->addStatements(model, newID);
        kDebug(30015) << "KoRdfSemanticItem::insert() returned model size:" << model->statementCount();
        const_cast<KoDocumentRdf*>(documentRdf())->rememberNewInlineRdfObject(inlineRdf);
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
    // let the RDF docker know about this new object too.
    KoCanvasResourceManager *provider = host->resourceManager();
    provider->setResource(KoText::CurrentTextPosition, editor->position() - 1);

    hKoSemanticStylesheet ss = defaultStylesheet();
    KoRdfSemanticItemViewSite vs(hKoRdfSemanticItem(this), newID);
    vs.applyStylesheet(editor, ss);

}

QList<hKoSemanticStylesheet> KoRdfSemanticItem::userStylesheets() const
{
    return documentRdf()->userStyleSheetList(className());
}

hKoSemanticStylesheet KoRdfSemanticItem::findStylesheetByUuid(const QString &id) const
{
    hKoSemanticStylesheet ret = hKoSemanticStylesheet(0);
    if (id.isEmpty()) {
        return ret;
    }
    foreach (hKoSemanticStylesheet ss, stylesheets()) {
        if (ss->uuid() == id) {
            return ss;
        }
    }
    foreach (hKoSemanticStylesheet ss, userStylesheets()) {
        if (ss->uuid() == id) {
            return ss;
        }
    }
    return ret;
}

hKoSemanticStylesheet KoRdfSemanticItem::findStylesheetByName(const QList<hKoSemanticStylesheet> &ssheets,
        const QString &n) const
{
    hKoSemanticStylesheet ret = hKoSemanticStylesheet(0);
    foreach (hKoSemanticStylesheet ss, ssheets) {
        if (ss->name() == n) {
            return ss;
        }
    }
    return ret;
}

hKoSemanticStylesheet KoRdfSemanticItem::findStylesheetByName(const QString &sheetType, const QString &n) const
{
    if (sheetType == "System") {
        return findStylesheetByName(stylesheets(), n);
    }
    return findStylesheetByName(userStylesheets(), n);
}

hKoSemanticStylesheet KoRdfSemanticItem::defaultStylesheet() const
{
    QString semanticClass = metaObject()->className();
    QSharedPointer<Soprano::Model> m = documentRdf()->model();
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
    hKoSemanticStylesheet ret = findStylesheetByUuid(uuid);
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

void KoRdfSemanticItem::defaultStylesheet(hKoSemanticStylesheet ss)
{
    const KoDocumentRdf *rdf = documentRdf();
    QSharedPointer<Soprano::Model> m = documentRdf()->model();
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

hKoSemanticStylesheet KoRdfSemanticItem::createUserStylesheet(const QString &name, const QString &templateString)
{
    bool isMutable = true;
    hKoSemanticStylesheet ss =
        hKoSemanticStylesheet(
            new KoSemanticStylesheet(QUuid::createUuid().toString(),
                                     name, templateString,
                                     KoSemanticStylesheet::stylesheetTypeUser(),
                                     isMutable));
    QList<hKoSemanticStylesheet> userSheets = userStylesheets();
    userSheets << ss;
    const_cast<KoDocumentRdf*>(documentRdf())->setUserStyleSheetList(className(),userSheets);
    connect(ss.data(), SIGNAL(nameChanging(hKoSemanticStylesheet, QString, QString)),
            this, SLOT(onUserStylesheetRenamed(hKoSemanticStylesheet, QString, QString)));
    return ss;
}

hKoSemanticStylesheet KoRdfSemanticItem::createSystemStylesheet(const QString &uuid, const QString &name, const QString &templateString) const
{
    const bool isNotMutable = false;
    hKoSemanticStylesheet ss =
        hKoSemanticStylesheet(new KoSemanticStylesheet(uuid, name, templateString, KoSemanticStylesheet::stylesheetTypeSystem(), isNotMutable));
    return ss;
}

void KoRdfSemanticItem::onUserStylesheetRenamed(hKoSemanticStylesheet ss, const QString &oldName, const QString &newName)
{
    Q_UNUSED(ss);
    Q_UNUSED(oldName);
    Q_UNUSED(newName);
}

void KoRdfSemanticItem::destroyUserStylesheet(hKoSemanticStylesheet ss)
{
    QList<hKoSemanticStylesheet> userSheets = userStylesheets();
    userSheets.removeAll(ss);
    const_cast<KoDocumentRdf*>(documentRdf())->setUserStyleSheetList(className(),userSheets);
}

void KoRdfSemanticItem::loadUserStylesheets(QSharedPointer<Soprano::Model> model)
{
    QString semanticClass = metaObject()->className();
    QString nodePrefix = "http://calligra.org/rdf/user-stylesheets/" + semanticClass + "/";
    Node rdfNil = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"));
    Node rdfFirst = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#first"));
    Node rdfRest = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"));

    Soprano::Node ListHeadSubject = Node::createResourceNode(QUrl(nodePrefix + "list"));
    QList<Statement> listNodes = KoTextRdfCore::loadList(model, ListHeadSubject);
    kDebug(30015) << "semanticClass:" << semanticClass << " listNodes.sz:" << listNodes.size();
    foreach (const Soprano::Statement &s, listNodes) {
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
        hKoSemanticStylesheet ss =
            hKoSemanticStylesheet(
                new KoSemanticStylesheet(uuid, name, templateString,
                                         KoSemanticStylesheet::stylesheetTypeUser(),
                                         isMutable));
        QList<hKoSemanticStylesheet> userSheets = userStylesheets();
        userSheets << ss;
        const_cast<KoDocumentRdf*>(documentRdf())->setUserStyleSheetList(className(),userSheets);
        connect(ss.data(), SIGNAL(nameChanging(hKoSemanticStylesheet, QString, QString)),
                this, SLOT(onUserStylesheetRenamed(KoSemanticStylesheetPtr, QString, QString)));
    }
}

void KoRdfSemanticItem::saveUserStylesheets(QSharedPointer<Soprano::Model> model, const Soprano::Node &context) const
{
    QString semanticClass = metaObject()->className();
    QString nodePrefix = "http://calligra.org/rdf/user-stylesheets/" + semanticClass + "/";
    Node rdfNil = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"));
    Node rdfFirst = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#first"));
    Node rdfRest = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"));

    Soprano::Node dataBNode = model->createBlankNode();
    QList< Soprano::Node > dataBNodeList;

    QList<hKoSemanticStylesheet> ssl = userStylesheets();
    foreach (hKoSemanticStylesheet ss, ssl) {
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

QList<hKoRdfSemanticItem> KoRdfSemanticItem::fromList(const QList< hKoRdfBasicSemanticItem > &lst)
{
    QList<hKoRdfSemanticItem> res;
    foreach (const hKoRdfBasicSemanticItem &l, lst) {
        KoRdfSemanticItem *newl = dynamic_cast<KoRdfSemanticItem *>(l.data());
        if (newl) {
            res << hKoRdfSemanticItem(newl);
        }
    }
    return res;
}
