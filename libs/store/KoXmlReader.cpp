/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2005-2006 Ariya Hidayat <ariya@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "KoXmlReader.h"
#include "KoXmlNS.h"

#include <QStringList>

// ==================================================================
//
//         functions in KoXml namespace
//
// ==================================================================

QDomElement KoXml::namedItemNS(const QDomNode& node, const QString& nsURI,
                                const QString& localName)
{
    // David's solution for namedItemNS, only for QDom stuff
    QDomNode n = node.firstChild();
    for (; !n.isNull(); n = n.nextSibling()) {
        if (n.isElement() && n.localName() == localName &&
                n.namespaceURI() == nsURI)
            return n.toElement();
    }
    return QDomElement();
}

QDomElement KoXml::namedItemNS(const QDomNode& node, const QString& nsURI,
                                const QString& localName, KoXmlNamedItemType type)
{
    Q_UNUSED(type);
    return namedItemNS(node, nsURI, localName);
}

int KoXml::childNodesCount(const QDomNode& node)
{
    return node.childNodes().count();
}

void KoXml::asQDomNode(QDomDocument& ownerDoc, const QDomNode& node)
{
    Q_ASSERT(!node.isDocument());
    ownerDoc.appendChild(ownerDoc.importNode(node, true));
}

void KoXml::asQDomElement(QDomDocument &ownerDoc, const QDomElement& element)
{
    KoXml::asQDomNode(ownerDoc, element);
}

QDomDocument KoXml::asQDomDocument(const QDomDocument& document)
{
    return document;
}

