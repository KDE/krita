/* This file is part of the KDE project
   Copyright (C) 2005-2006 Ariya Hidayat <ariya@kde.org>

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
   Boston, MA 02110-1301, USA.
*/

#include "KoXmlReader.h"
#include "KoXmlNS.h"

#include <QTextCodec>
#include <QTextDecoder>
#include <QXmlStreamReader>


// ==================================================================
//
//         functions in KoXml namespace
//
// ==================================================================

KoXmlElement KoXml::namedItemNS(const KoXmlNode& node, const QString& nsURI,
                                const QString& localName)
{
    // David's solution for namedItemNS, only for QDom stuff
    KoXmlNode n = node.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        if (n.isElement() && n.localName() == localName &&
                n.namespaceURI() == nsURI)
            return n.toElement();
    }
    return KoXmlElement();
}

KoXmlElement KoXml::namedItemNS(const KoXmlNode& node, const QString& nsURI,
                                const QString& localName, KoXmlNamedItemType type)
{
    Q_UNUSED(type)
    return namedItemNS(node, nsURI, localName);
}

void KoXml::load(KoXmlNode& node, int depth)
{
    // do nothing, QDom has no on-demand loading
    Q_UNUSED(node);
    Q_UNUSED(depth);
}


void KoXml::unload(KoXmlNode& node)
{
    // do nothing, QDom has no on-demand unloading
    Q_UNUSED(node);
}

int KoXml::childNodesCount(const KoXmlNode& node)
{
    return node.childNodes().count();
}

QStringList KoXml::attributeNames(const KoXmlNode& node)
{
    QStringList result;

    QDomNamedNodeMap attrMap = node.attributes();
    for (int i = 0; i < attrMap.count(); ++i)
        result += attrMap.item(i).toAttr().name();

    return result;
}

void KoXml::asQDomNode(QDomDocument& ownerDoc, const KoXmlNode& node)
{
    Q_ASSERT(!node.isDocument());
    ownerDoc.appendChild(ownerDoc.importNode(node, true));
}

void KoXml::asQDomElement(QDomDocument &ownerDoc, const KoXmlElement& element)
{
    KoXml::asQDomNode(ownerDoc, element);
}

QDomDocument KoXml::asQDomDocument(const KoXmlDocument& document)
{
    return document;
}

bool KoXml::setDocument(KoXmlDocument& doc, QIODevice *device,
                        bool namespaceProcessing,
                        QString *errorMsg, int *errorLine, int *errorColumn)
{
    bool result = doc.setContent(device, namespaceProcessing, errorMsg, errorLine, errorColumn);
    return result;
}
