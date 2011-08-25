/* This file is part of the KDE project
 *
 * Copyright (c) 2011 Boudewijn Rempt <boud@kogmbh.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "rdf_test.h"

#include <QtTest/QTest>
#include <QUuid>
#include <QString>
#include <QTextDocument>
#include <QTextTable>

#include <KoRdfSemanticItem.h>
#include <KoDocumentRdf.h>
#include <KoTextDocument.h>
#include <KoTextEditor.h>
#include <KoBookmark.h>
#include <KoTextInlineRdf.h>
#include <KoTextDocument.h>
#include <KoInlineTextObjectManager.h>

const QString lorem(
    "Lorem ipsum dolor sit amet, consectetur adipisicing elit, sed do eiusmod tempor"
    "incididunt ut labore et dolore magna aliqua. Ut enim ad minim veniam, quis nostrud"
    "exercitation ullamco laboris nisi ut aliquip ex ea commodo consequat.\n"
    "Duis aute irure dolor in reprehenderit in voluptate velit esse cillum dolore eu fugiat nulla"
    "pariatur. Excepteur sint occaecat cupidatat non proident, sunt in culpa qui officia"
    "deserunt mollit anim id est laborum.\n"
    );

class TestSemanticItem : public KoRdfSemanticItem
{
public:
    const QString PREDBASE;

    TestSemanticItem(QObject *parent, const KoDocumentRdf *rdf = 0)
        : KoRdfSemanticItem(const_cast<KoDocumentRdf*>(rdf), parent)
        , PREDBASE("http://calligra-suite.org/testrdf/")
    {
        m_uri = QUuid::createUuid().toString();
        setRdfType(PREDBASE + "testsamenticitem");
    }

    TestSemanticItem(QObject *parent, const KoDocumentRdf *rdf, Soprano::QueryResultIterator &it)
        : KoRdfSemanticItem(const_cast<KoDocumentRdf*>(rdf), it, parent)
    {
        m_uri = it.binding("object").toString();
        m_name = it.binding("name").toString();
    }

    virtual ~TestSemanticItem()
    {
    }

    virtual QWidget *createEditor(QWidget */*parent*/)
    {
        return 0;
    }

    virtual void updateFromEditorData()
    {
    }

    virtual void exportToFile(const QString &/*fileName*/ = QString()) const
    {
    }

    virtual void importFromData(const QByteArray &/*ba*/, KoDocumentRdf */*rdf*/ = 0, KoCanvasBase */*host*/ = 0)
    {
    }

    void setName(const QString &name)
    {
        updateTriple(m_name, name, PREDBASE + "name");
        if (documentRdf()) {
            const_cast<KoDocumentRdf*>(documentRdf())->emitSemanticObjectUpdated(this);
        }
    }

    virtual QString name() const
    {
        return m_name;
    }

    virtual QString className() const
    {
        return "TestSemanticItem";
    }

    virtual QList<KoSemanticStylesheet*> stylesheets() const
    {
        QList<KoSemanticStylesheet*> sheets;
        return sheets;
    }

    Soprano::Node linkingSubject() const
    {
        return Soprano::Node::createResourceNode(m_uri);
    }

    static QList<TestSemanticItem*> allObjects(KoDocumentRdf* rdf, Soprano::Model *model = 0)
    {
        QList<TestSemanticItem*> result;
        const Soprano::Model* m = model ? model : rdf->model();

        QString query =
                "prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
                "prefix testrdf: <http://calligra-suite.org/testrdf> \n"
                "select distinct ?name ?object ?metaDataparameterData ?typeOfObject \n"
                "where { \n"
                "    ?element rdf:type skf:table . \n"
                "    ?element testrdf:name ?name \n"
                "    ?element testrdf:metaDataparameterData ?metaDataparameterData \n"
                "    ?element testrdf:typeOfObject ?typeOfObject \n"
                "}\n"
                "    order by  DESC(?name) \n ";

        Soprano::QueryResultIterator it =m->executeQuery(query,
                                                         Soprano::Query::QueryLanguageSparql);


        while (it.next()) {
            TestSemanticItem* item = new TestSemanticItem(rdf, rdf ,it);
            result << item;
        }

        return result;
    }

private:

    QString m_uri;
    QString m_name;

};

void RdfTest::testCreateMarkers()
{

    QObject parent;

    // the rdf storage. In calligra, it's part of the KoDocument.
    KoDocumentRdf rdfDoc;

    // create a document
    QTextDocument doc;

    KoInlineTextObjectManager inlineObjectManager(&parent);
    KoTextDocument textDoc(&doc);
    textDoc.setInlineTextObjectManager(&inlineObjectManager);

    KoTextEditor editor(&doc);

    // enter some lorem ipsum
    editor.insertText(lorem);
    // enter a bit of marked text

    editor.insertTable(5,10);
    QTextTable *table = editor.cursor()->currentTable();
    table->setObjectName("Test Table");

    TestSemanticItem *testItem = new TestSemanticItem(&parent, &rdfDoc);
    testItem->setName(table->objectName());

    KoBookmark *startmark = new KoBookmark(editor.document());
    startmark->setType(KoBookmark::StartBookmark);

    KoTextInlineRdf *inlineRdf(new KoTextInlineRdf(editor.document(), startmark));
    QString newId = inlineRdf->createXmlId();
    inlineRdf->setXmlId(newId);

    startmark->setName(newId);
    startmark->setInlineRdf(inlineRdf);

    editor.setPosition(table->firstPosition());
    editor.movePosition(QTextCursor::PreviousCharacter);
    editor.insertInlineObject(startmark);

    Soprano::Statement st(
          testItem->linkingSubject(), // subject
          Soprano::Node::createResourceNode(QUrl("http://docs.oasis-open.org/opendocument/meta/package/common#idref")), // predicate
          Soprano::Node::createLiteralNode(newId), // object
          rdfDoc.manifestRdfNode()); // manifest datastore

    const_cast<Soprano::Model*>(rdfDoc.model())->addStatement(st);

    rdfDoc.rememberNewInlineRdfObject(inlineRdf);

    KoBookmark *endmark = new KoBookmark(editor.document());
    endmark->setName(newId);
    endmark->setType(KoBookmark::EndBookmark);
    startmark->setEndBookmark(endmark);

    editor.setPosition(table->lastPosition());
    editor.movePosition(QTextCursor::NextCharacter);
    editor.insertInlineObject(endmark);

    // enter some more lorem
    editor.insertText(lorem);

    // verify that the markers are there
}

void RdfTest::testFindMarkers()
{
}

void RdfTest::testEditAndFindMarkers()
{
}

void RdfTest::testRemoveMarkers()
{
}

QTEST_MAIN(RdfTest)

#include "rdf_test.moc"
