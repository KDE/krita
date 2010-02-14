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

#include "rdf/RdfSemanticItemViewSite.h"
#include "rdf/KoDocumentRdf.h"
#include "rdf/KoDocumentRdf_p.h"
#include "KoCanvasBase.h"
#include "KoToolProxy.h"
#include "KoResourceManager.h"
#include "KoText.h"
#include "KoTextEditor.h"
#include <kdebug.h>

using namespace Soprano;

RdfSemanticItemViewSite::RdfSemanticItemViewSite(RdfSemanticItem *si, const QString &xmlid)
        : m_xmlid(xmlid)
        , m_semItem(si)
{
    KoDocumentRdf *rdf = m_semItem->documentRdf();
    Q_UNUSED(rdf);
}

Soprano::Node RdfSemanticItemViewSite::linkingSubject()
{
    KoDocumentRdf *rdf = m_semItem->documentRdf();
    Soprano::Model *m = rdf->model();
    Node pred(QUrl("http://kogmbh.net/rdf/site/package/common#idref"));
    Node obj = Node::createLiteralNode(m_xmlid);
    Node context = rdf->manifestRdfNode();
    // try to find it if it already exists
    StatementIterator it = m->listStatements(Node(), pred, obj, context);
    QList<Statement> allStatements = it.allElements();
    foreach (Soprano::Statement s, allStatements) {
        return s.subject();
    }
    Node ret = m->createBlankNode();
    m->addStatement(ret, pred, obj, context);
    return ret;
}

QString RdfSemanticItemViewSite::getProperty(const QString &prop, const QString &defval)
{
    Soprano::Node ls = linkingSubject();
    QString fqprop = "http://kogmbh.net/rdf/site#" + prop;
    KoDocumentRdf *rdf = m_semItem->documentRdf();
    Soprano::Model *m = rdf->model();
    StatementIterator it = m->listStatements(ls, Node::createResourceNode(QUrl(fqprop)),
                               Node(), rdf->manifestRdfNode());
    QList<Statement> allStatements = it.allElements();
    foreach (Soprano::Statement s, allStatements) {
        return s.object().toString();
    }
    return defval;
}

void RdfSemanticItemViewSite::setProperty(const QString &prop, const QString &v)
{
    QString fqprop = "http://kogmbh.net/rdf/site#" + prop;
    KoDocumentRdf *rdf = m_semItem->documentRdf();
    Soprano::Model *m = rdf->model();
    Soprano::Node ls = linkingSubject();
    Soprano::Node pred = Node::createResourceNode(QUrl(fqprop));
    m->removeAllStatements(Statement(ls, pred, Node()));
    m->addStatement(ls, pred,Node::createLiteralNode(v), rdf->manifestRdfNode());
}

SemanticStylesheet *RdfSemanticItemViewSite::stylesheet()
{
    QString name = getProperty("stylesheet", "name");
    QString type = getProperty("stylesheet-type", SemanticStylesheet::TYPE_SYSTEM);
    QString uuid = getProperty("stylesheet-uuid", "");
    kDebug(30015) << "stylesheet at site, format(), xmlid:" << m_xmlid;
    kDebug(30015) << " sheet:" << name << " type:" << type;
    SemanticStylesheet *ret(0);
    if (!uuid.isEmpty()) {
        ret = m_semItem->findStylesheetByUuid(uuid);
    }
    if (!ret) {
        ret = m_semItem->findStylesheetByName(type, name);
    }
    if (!ret) {
        // safety first, there will always be a default stylesheet
        ret = m_semItem->defaultStylesheet();
    }
    Q_ASSERT(ret);
    return ret;
}

void RdfSemanticItemViewSite::applyStylesheet(KoTextEditor *editor, SemanticStylesheet *ss)
{
    // Save the stylesheet property and cause a reflow.
    kDebug(30015) << "apply stylesheet at site. format(), xmlid:" << m_xmlid << " sheet:" << ss->name();
    setStylesheetWithoutReflow(ss);
    reflowUsingCurrentStylesheet(editor);
}

void RdfSemanticItemViewSite::disassociateStylesheet()
{
    kDebug(30015) << "stylesheet at site. xmlid:" << m_xmlid;
    setProperty("stylesheet", "");
    setProperty("stylesheet-type", "");
    setProperty("stylesheet-uuid", "");
}

void RdfSemanticItemViewSite::setStylesheetWithoutReflow(SemanticStylesheet *ss)
{
    // Save the stylesheet property
    kDebug(30015) << "apply stylesheet at site. format(), xmlid:" << m_xmlid << " sheet:" << ss->name();
    setProperty("stylesheet", ss->name());
    setProperty("stylesheet-type", ss->type());
    setProperty("stylesheet-uuid", ss->uuid());
}

void RdfSemanticItemViewSite::reflowUsingCurrentStylesheet(KoTextEditor *editor)
{
    SemanticStylesheet *ss = stylesheet();
    ss->format(m_semItem, editor, m_xmlid);
}

void RdfSemanticItemViewSite::selectRange(KoResourceManager *provider, int startpos, int endpos)
{
    kDebug(30015) << " startpos:" << startpos << " endpos:" << endpos;
    if (endpos > startpos) {
        provider->setResource(KoText::CurrentTextPosition, startpos);
        provider->setResource(KoText::CurrentTextAnchor, endpos + 1);
        provider->clearResource(KoText::SelectedTextPosition);
        provider->clearResource(KoText::SelectedTextAnchor);
    } else {
        provider->setResource(KoText::CurrentTextPosition, startpos);
    }
}

void RdfSemanticItemViewSite::select(KoCanvasBase *host)
{
    Q_ASSERT(m_semItem);
    Q_ASSERT(m_semItem->documentRdf());
    Q_ASSERT(host);
    KoTextEditor *editor = KoDocumentRdf::ensureTextTool(host);
    KoResourceManager *provider = host->resourceManager();
    KoDocumentRdf *rdf = m_semItem->documentRdf();
    QPair<int, int> p = p = rdf->findExtent(m_xmlid);
    int startpos = p.first;
    int endpos = p.second + 1;
    if (!endpos) {
        kDebug(30015) << "No end point found for semantic item:" << m_semItem->name();
        kDebug(30015) << "xmlid:" << m_xmlid;
        return;
    }
    kDebug(30015) << "xmlid:" << m_xmlid;
    kDebug(30015) << "start:" << startpos << " endpos:" << endpos;
    selectRange(provider, startpos, endpos);
    kDebug(30015) << "selected text:" << editor->selectedText();
}
