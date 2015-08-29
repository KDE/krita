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

#include "KoRdfBasicSemanticItem.h"

//Calligra
#include <KoDocumentRdf.h>

//KDE
#include <KDebug>

//QT
#include <QUuid>
#include <QDateTime>

using namespace Soprano;

KoRdfBasicSemanticItem::KoRdfBasicSemanticItem(QObject *parent)
    : QObject(parent)
    , m_rdf(0)
{
}

KoRdfBasicSemanticItem::KoRdfBasicSemanticItem(QObject *parent, const KoDocumentRdf *rdf)
    : QObject(parent)
    , m_rdf(rdf)
{
}

KoRdfBasicSemanticItem::KoRdfBasicSemanticItem(QObject *parent, const KoDocumentRdf *rdf, Soprano::QueryResultIterator &it)
    : QObject(parent)
    , m_rdf(rdf)
{
    m_context = it.binding("graph");
    kDebug(30015) << "KoRdfBasicSemanticItem() context:" << m_context.toString();
}

KoRdfBasicSemanticItem::~KoRdfBasicSemanticItem()
{
}

const KoDocumentRdf *KoRdfBasicSemanticItem::documentRdf() const
{
    return m_rdf;
}

QStringList KoRdfBasicSemanticItem::xmlIdList() const
{
    QStringList ret;

    StatementIterator it = documentRdf()->model()->listStatements(
        linkingSubject(),
        Node::createResourceNode(QUrl("http://docs.oasis-open.org/ns/office/1.2/meta/pkg#idref")),
        Node(),
        documentRdf()->manifestRdfNode()
    );
    QList<Statement> allStatements = it.allElements();
    foreach (const Soprano::Statement &s, allStatements) {
        QString xmlid = s.object().toString();
        ret << xmlid;
    }
    return ret;
}

void KoRdfBasicSemanticItem::updateTriple_remove(const Soprano::LiteralValue &toModify,
        const QString &predString,
        const Soprano::Node &explicitLinkingSubject)
{
    QSharedPointer<Soprano::Model> m = documentRdf()->model();
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
    foreach (const Soprano::Statement &s, allStatements) {
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

void KoRdfBasicSemanticItem::updateTriple_add(const Soprano::LiteralValue &toModify,
                                       const QString &predString,
                                       const Soprano::Node &explicitLinkingSubject)
{
    QSharedPointer<Soprano::Model> m = documentRdf()->model();
    Node pred = Node::createResourceNode(QUrl(predString));

    kDebug(30015) << "Rdf.add subj:" << explicitLinkingSubject;
    kDebug(30015) << "Rdf.add pred:" << pred;
    kDebug(30015) << "Rdf.add  obj:" << Node::createLiteralNode(toModify);
    kDebug(30015) << "Rdf.add  ctx:" << context();
    m->addStatement(explicitLinkingSubject, pred, Node::createLiteralNode(toModify), context());
}


void KoRdfBasicSemanticItem::updateTriple(QString &toModify, const QString &newValue, const QString &predString)
{
    kDebug(30015) << "tomod:" << toModify << " newV:" << newValue << " pred:" << predString;
    updateTriple_remove(toModify, predString, linkingSubject());
    toModify = newValue;
    updateTriple_add(toModify, predString, linkingSubject());
}

void KoRdfBasicSemanticItem::updateTriple(KDateTime &toModify, const KDateTime &newValue, const QString &predString)
{
    updateTriple_remove(Soprano::LiteralValue(toModify.dateTime()),
                        predString, linkingSubject());
    toModify = newValue;
    updateTriple_add(Soprano::LiteralValue(toModify.dateTime()),
                     predString, linkingSubject());
}

void KoRdfBasicSemanticItem::updateTriple(double &toModify,
                                   double newValue,
                                   const QString &predString,
                                   const Soprano::Node &explicitLinkingSubject)
{
    updateTriple_remove(Soprano::LiteralValue(toModify), predString, explicitLinkingSubject);
    toModify = newValue;
    updateTriple_add(Soprano::LiteralValue(toModify), predString, explicitLinkingSubject);
}

void KoRdfBasicSemanticItem::setRdfType(const QString &t)
{
    QSharedPointer<Soprano::Model> m = documentRdf()->model();
    Q_ASSERT(m);
    Node pred = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#type"));
    m->addStatement(linkingSubject(), pred, Node::createResourceNode(t), context());
}

Soprano::Node KoRdfBasicSemanticItem::linkingSubject() const
{
    return Node::createEmptyNode();
}

Soprano::Node KoRdfBasicSemanticItem::context() const
{
    if (m_context.isValid()) {
        return m_context;
    }
    return documentRdf()->manifestRdfNode();
}

Soprano::Node KoRdfBasicSemanticItem::createNewUUIDNode() const
{
    QString uuid = QUuid::createUuid().toString();
    uuid.remove('{');
    uuid.remove('}');
    QString nodestr = "http://calligra.org/uuidnode/" + uuid;
    return Node::createResourceNode(nodestr);
}
