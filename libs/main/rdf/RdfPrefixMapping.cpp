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

#include "RdfPrefixMapping.h"
#include "KoTextRdfCore.h"
#include <Soprano/Soprano>
#include <kdebug.h>
using namespace Soprano;

RdfPrefixMapping::RdfPrefixMapping(KoDocumentRdf* m_rdf)
        : m_rdf(m_rdf)
{
    insert("pkg", "http://docs.oasis-open.org/opendocument/meta/package/common#");
    insert("odf", "http://docs.oasis-open.org/opendocument/meta/package/odf#");
    insert("rdf", "http://www.w3.org/1999/02/22-rdf-syntax-ns#");
    insert("xhtml", "http://www.w3.org/1999/xhtml");
    insert("xsd", "http://www.w3.org/2001/XMLSchema");
    insert("xsi", "http://www.w3.org/2001/XMLSchema-instance");
    insert("foaf", "http://xmlns.com/foaf/0.1/");
    insert("geo84", "http://www.w3.org/2003/01/geo/wgs84_pos#");
    insert("dcterms", "http://dublincore.org/documents/dcmi-terms/#");
    insert("cite", "http://docs.oasis-open.org/prototype/opendocument/citation#");
    insert("cal", "http://www.w3.org/2002/12/cal/icaltzd#");
}

RdfPrefixMapping::~RdfPrefixMapping()
{
}

QString RdfPrefixMapping::canonPrefix(QString pname) const
{
    int idx = pname.indexOf(':');
    if (idx >= 0) {
        pname = pname.left(idx + 1);
    }
    if (!pname.endsWith(":")) {
        pname += ":";
    }
    return pname;
}


QString RdfPrefixMapping::URItoPrefexedLocalname(QString uri) const
{
    for (m_mappings_t::const_iterator mi = m_mappings.begin(); mi != m_mappings.end(); ++mi) {
        if (uri.startsWith(mi.value())) {
            QString ret = mi.key() + uri.mid(mi.value().length());
            return ret;
        }
    }
    return uri;
}

QString RdfPrefixMapping::PrefexedLocalnameToURI(QString pname) const
{
    QString pfx = canonPrefix(pname);
    if (pfx.isEmpty()) {
        return pname;
    }
    m_mappings_t::const_iterator mi = m_mappings.find(pfx);
    if (mi == m_mappings.end())
        return pname;
    return mi.value() + pname.mid(mi.key().length());
}

QString RdfPrefixMapping::prefexToURI(QString pfx) const
{
    pfx = canonPrefix(pfx);
    m_mappings_t::const_iterator mi = m_mappings.find(pfx);
    if (mi == m_mappings.end()) {
        return "";
    }
    return mi.value();
}

void RdfPrefixMapping::dump() const
{
    kDebug(30015) << m_mappings;
}

void RdfPrefixMapping::insert(QString prefix, QString url)
{
    prefix = canonPrefix(prefix);
    kDebug(30015) << " prefix:" << prefix << " url:" << url;
    m_mappings.insert(prefix, url);
}

void RdfPrefixMapping::remove(QString prefix)
{
    prefix = canonPrefix(prefix);
    kDebug(30015) << " prefix:" << prefix;
    m_mappings.remove(prefix);
}

void RdfPrefixMapping::load(Soprano::Model* model)
{
    QString nodePrefix = "http://kogmbh.net/rdf/prefixmapping/";
    Node rdfNil = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"));
    Node rdfFirst = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#first"));
    Node rdfRest = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"));
    Soprano::Node ListHeadSubject = Node::createResourceNode(QUrl(nodePrefix + "list"));
    QList<Statement> listNodes = KoTextRdfCore::loadList(model, ListHeadSubject);
    kDebug(30015) << "found count:" << listNodes.size();
    foreach (Soprano::Statement s, listNodes) {
        Soprano::Node dataBNode = s.object();
        QString prefix = KoTextRdfCore::getObject(model, dataBNode,
                         Node::createResourceNode(QUrl(nodePrefix + "prefix"))).toString();
        QString url = KoTextRdfCore::getObject(model, dataBNode,
                                               Node::createResourceNode(QUrl(nodePrefix + "url"))).toString();
        kDebug(30015) << "found prefix:" << prefix << " url:" << url;
        insert(prefix, url);
    }
}

void RdfPrefixMapping::save(Soprano::Model* model, Soprano::Node context) const
{
    QString nodePrefix = "http://kogmbh.net/rdf/prefixmapping/";
    Node rdfNil = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#nil"));
    Node rdfFirst = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#first"));
    Node rdfRest = Node::createResourceNode(QUrl("http://www.w3.org/1999/02/22-rdf-syntax-ns#rest"));
    Soprano::Node dataBNode = model->createBlankNode();
    QList< Soprano::Node > dataBNodeList;
    m_mappings_t::const_iterator mi = m_mappings.begin();
    m_mappings_t::const_iterator me = m_mappings.end();
    for (; mi != me; ++mi) {
        kDebug(30015) << "saving prefix:" << mi.key() << " url:" << mi.value();
        dataBNode = model->createBlankNode();
        model->addStatement(
            dataBNode,
            Node::createResourceNode(QUrl(nodePrefix + "prefix")),
            Node::createLiteralNode(mi.key()),
            context);
        model->addStatement(
            dataBNode,
            Node::createResourceNode(QUrl(nodePrefix + "url")),
            Node::createLiteralNode(mi.value()),
            context);
        dataBNodeList << dataBNode;
    }
    Soprano::Node ListHeadSubject = Node::createResourceNode(QUrl(nodePrefix + "list"));
    KoTextRdfCore::saveList(model, ListHeadSubject, dataBNodeList, context);
}
