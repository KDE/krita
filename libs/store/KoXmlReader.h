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

#ifndef KO_XMLREADER_H
#define KO_XMLREADER_H

#include "KoXmlReaderForward.h"

#include "kritastore_export.h"

#include <QPair>
#include <QString>

class QIODevice;


/**
 * The office-text-content-prelude type.
 */
enum KoXmlNamedItemType {
    KoXmlTextContentPrelude ///< office-text-content-prelude
    //KoXmlTextContentMain, ///< office-text-content-main
    //KoXmlTextContentEpilogue ///< office-text-content-epilogue
};

/**
 * This namespace contains a few convenience functions to simplify code using QDom
 * (when loading OASIS documents, in particular).
 *
 * To find the child element with a given name, use KoXml::namedItemNS.
 *
 * To find all child elements with a given name, use
 * QDomElement e;
 * forEachElement( e, parent )
 * {
 *     if ( e.localName() == "..." && e.namespaceURI() == KoXmlNS::... )
 *     {
 *         ...
 *     }
 * }
 * Note that this means you don't ever need to use QDomNode nor toElement anymore!
 * Also note that localName is the part without the prefix, this is the whole point
 * of namespace-aware methods.
 *
 * To find the attribute with a given name, use QDomElement::attributeNS.
 *
 * Do not use getElementsByTagNameNS, it's recursive (which is never needed in Calligra).
 * Do not use tagName() or nodeName() or prefix(), since the prefix isn't fixed.
 *
 * @author David Faure <faure@kde.org>
 */
namespace KoXml
{

/**
 * A namespace-aware version of QDomNode::namedItem(),
 * which also takes care of casting to a QDomElement.
 *
 * Use this when a domelement is known to have only *one* child element
 * with a given tagname.
 *
 * Note: do *NOT* use getElementsByTagNameNS, it's recursive!
 */
KRITASTORE_EXPORT KoXmlElement namedItemNS(const KoXmlNode& node,
                                        const QString& nsURI, const QString& localName);

/**
 * A namespace-aware version of QDomNode::namedItem().
 * which also takes care of casting to a QDomElement.
 *
 * Use this when you like to return the first or an invalid
 * KoXmlElement with a known type.
 *
 * This is an optimized version of the namedItemNS above to
 * give fast access to certain sections of the document using
 * the office-text-content-prelude condition as @a KoXmlNamedItemType .
 */
KRITASTORE_EXPORT KoXmlElement namedItemNS(const KoXmlNode& node,
                                      const QString& nsURI, const QString& localName,
                                      KoXmlNamedItemType type);

/**
 * Explicitly load child nodes of specified node, up to given depth.
 * This function has no effect if QDom is used.
 */
KRITASTORE_EXPORT void load(KoXmlNode& node, int depth = 1);

/**
 * Unload child nodes of specified node.
 * This function has no effect if QDom is used.
 */
KRITASTORE_EXPORT void unload(KoXmlNode& node);

/**
 * Get the number of child nodes of specified node.
 */
KRITASTORE_EXPORT int childNodesCount(const KoXmlNode& node);

/**
 * Return the name of all attributes of specified node.
 */
KRITASTORE_EXPORT QStringList attributeNames(const KoXmlNode& node);

/**
 * Convert KoXmlNode classes to the corresponding QDom classes, which has
 * @p ownerDoc as the owner document (QDomDocument instance).
 * The converted @p node (and its children) are added to ownerDoc.
 *
 * NOTE:
 * - If ownerDoc is not empty, this may fail, @see QDomDocument
 * - @p node must not be a KoXmlDocument, use asQDomDocument()
 *
 * @see asQDomDocument, asQDomElement
 */
KRITASTORE_EXPORT void asQDomNode(QDomDocument& ownerDoc, const KoXmlNode& node);

/**
 * Convert KoXmlNode classes to the corresponding QDom classes, which has
 * @p ownerDoc as the owner document (QDomDocument instance).
 * The converted @p element (and its children) is added to ownerDoc.
 *
 * NOTE: If ownerDoc is not empty, this may fail, @see QDomDocument
 *
 */
KRITASTORE_EXPORT void asQDomElement(QDomDocument& ownerDoc, const KoXmlElement& element);

/**
 * Converts the whole @p document into a QDomDocument
 */
KRITASTORE_EXPORT QDomDocument asQDomDocument(const KoXmlDocument& document);

/*
 * Load an XML document from specified device to a document. You can of
 * course use it with QFile (which inherits QIODevice).
 * This is much more memory efficient than standard QDomDocument::setContent
 * because the data from the device is buffered, unlike
 * QDomDocument::setContent which just loads everything in memory.
 *
 * Note: it is assumed that the XML uses UTF-8 encoding.
 */
KRITASTORE_EXPORT bool setDocument(KoXmlDocument& doc, QIODevice* device,
                                bool namespaceProcessing, QString* errorMsg = 0,
                                int* errorLine = 0, int* errorColumn = 0);
}

/**
 * \def forEachElement( elem, parent )
 * \brief Loop through all child elements of \p parent.
 * This convenience macro is used to implement the forEachElement loop.
 * The \p elem parameter is a name of a QDomElement variable and the \p parent
 * is the name of the parent element. For example:
 *
 * \code
 * QDomElement e;
 * forEachElement( e, parent )
 * {
 *     qDebug() << e.localName() << " element found.";
 *     ...
 * }
 * \endcode
 */
#define forEachElement( elem, parent ) \
    for ( KoXmlNode _node = parent.firstChild(); !_node.isNull(); _node = _node.nextSibling() ) \
        if ( ( elem = _node.toElement() ).isNull() ) {} else


#endif // KO_XMLREADER_H
