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
#ifndef TEST_SEMANTIC_ITEM
#define TEST_SEMANTIC_ITEM

#include <QTest>
#include <QUuid>
#include <QString>
#include <QTextDocument>
#include <QTextTable>
#include <QTextCharFormat>

#include <KoRdfSemanticItem.h>
#include <KoDocumentRdf.h>
#include <KoTextEditor.h>
#include <KoTextInlineRdf.h>
#include <KoTextDocument.h>
#include <KoInlineTextObjectManager.h>

class TestSemanticItem;
typedef QExplicitlySharedDataPointer<TestSemanticItem> hTestSemanticItem;

class TestSemanticItem : public KoRdfSemanticItem
{
public:
    const QString PREDBASE;

    TestSemanticItem(QObject *parent, const KoDocumentRdf *rdf = 0)
        : KoRdfSemanticItem(parent, const_cast<KoDocumentRdf*>(rdf))
        , PREDBASE("http://calligra.org/testrdf/")
        , m_uri(QUuid::createUuid().toString())
    {
        Q_ASSERT(!m_uri.isEmpty());

        m_linkingSubject = Soprano::Node::createResourceNode(m_uri);

        Q_ASSERT(context().isValid());
        Q_ASSERT(m_linkingSubject.isResource());
        setRdfType(PREDBASE + "testitem");
        Q_ASSERT(documentRdf()->model()->statementCount() > 0);

        updateTriple(m_payload, "payload, payload, payload",  PREDBASE + "payload");
    }

    TestSemanticItem(QObject *parent, const KoDocumentRdf *rdf, Soprano::QueryResultIterator &it)
        : KoRdfSemanticItem(parent, const_cast<KoDocumentRdf*>(rdf), it)
    {
        m_uri = it.binding("object").toString();
        Q_ASSERT(!m_uri.isNull());
        m_linkingSubject = Soprano::Node::createResourceNode(m_uri);
        Q_ASSERT(m_linkingSubject.isResource());
        m_name = it.binding("name").toString();
        Q_ASSERT(!m_name.isNull());
        m_payload = it.binding("payload").toString();
        Q_ASSERT(!m_payload.isNull());
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

    virtual void importFromData(const QByteArray &/*ba*/, const KoDocumentRdf */*rdf*/ = 0, KoCanvasBase */*host*/ = 0)
    {
    }

    void setName(const QString &name)
    {
        updateTriple(m_name, name, PREDBASE + "name");
        if (documentRdf()) {
            const_cast<KoDocumentRdf*>(documentRdf())->emitSemanticObjectUpdated(hKoRdfSemanticItem(this));
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

    virtual QList<hKoSemanticStylesheet> stylesheets() const
    {
        QList<hKoSemanticStylesheet> sheets;
        return sheets;
    }

    Soprano::Node linkingSubject() const
    {
        return m_linkingSubject;
    }

    static QList<hTestSemanticItem> allObjects(KoDocumentRdf* rdf, QSharedPointer<Soprano::Model> model = QSharedPointer<Soprano::Model>(0))
    {
        QList<hTestSemanticItem> result;
        QSharedPointer<Soprano::Model> m = model ? model : rdf->model();

        QString query =
                "prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
                "prefix testrdf: <http://calligra.org/testrdf/> \n"
                "select distinct ?name ?object ?payload \n"
                "where { \n"
                "    ?object rdf:type testrdf:testitem . \n"
                "    ?object testrdf:name ?name . \n"
                "    ?object testrdf:payload ?payload \n"
                "}\n"
                "    order by  DESC(?name) \n ";

        Soprano::QueryResultIterator it = m->executeQuery(query,
                                                          Soprano::Query::QueryLanguageSparql);

        while (it.next()) {
            hTestSemanticItem item(new TestSemanticItem(0, rdf, it));
            result << item;
        }
        return result;
    }

    static QList<hTestSemanticItem> findItemsByName(const QString name, KoDocumentRdf* rdf, QSharedPointer<Soprano::Model> model = QSharedPointer<Soprano::Model>(0))
    {
        QList<hTestSemanticItem> result;
        QSharedPointer<Soprano::Model> m = model ? model : rdf->model();

        QString query(
                    "prefix rdf: <http://www.w3.org/1999/02/22-rdf-syntax-ns#> \n"
                    "prefix testrdf: <http://calligra.org/testrdf/> \n"
                    "select distinct ?name ?object ?payload \n"
                    "where { \n"
                    "    ?object rdf:type testrdf:testitem . \n"
                    "    ?object testrdf:name ?name . \n"
                    "    ?object testrdf:payload ?payload . \n"
                    "    filter (?name = %1) "
                    "}\n"
                    "    order by  DESC(?name) \n ");

        Soprano::QueryResultIterator it = m->executeQuery(query.arg(Soprano::Node::literalToN3(name)),
                                                          Soprano::Query::QueryLanguageSparql);

        while (it.next()) {
            hTestSemanticItem item(new TestSemanticItem(0, rdf, it));
            result << item;
        }
        return result;
    }

private:
    Soprano::Node m_linkingSubject;
    QString m_name;
    QString m_uri;
    QString m_payload;
};

#endif
