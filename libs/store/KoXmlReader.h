/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2005-2006 Ariya Hidayat <ariya@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef KO_XMLREADER_H
#define KO_XMLREADER_H

#include "kritastore_export.h"

#include <QDomDocument>
#include <QIODevice>
#include <QPair>
#include <QString>


/**
 * The office-text-content-prelude type.
 */
enum KoXmlNamedItemType {
    QDomTextContentPrelude ///< office-text-content-prelude
    //QDomTextContentMain, ///< office-text-content-main
    //QDomTextContentEpilogue ///< office-text-content-epilogue
};

/**
 * This namespace contains a few convenience functions to simplify code using QDom
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
KRITASTORE_EXPORT QDomElement namedItemNS(const QDomNode& node,
                                          const QString& nsURI, const QString& localName);

/**
 * A namespace-aware version of QDomNode::namedItem().
 * which also takes care of casting to a QDomElement.
 *
 * Use this when you like to return the first or an invalid
 * QDomElement with a known type.
 *
 * This is an optimized version of the namedItemNS above to
 * give fast access to certain sections of the document using
 * the office-text-content-prelude condition as @a KoXmlNamedItemType .
 */
KRITASTORE_EXPORT QDomElement namedItemNS(const QDomNode& node,
                                          const QString& nsURI, const QString& localName,
                                          KoXmlNamedItemType type);

/**
 * Get the number of child nodes of specified node.
 */
KRITASTORE_EXPORT int childNodesCount(const QDomNode& node);

/**
 * Convert QDomNode classes to the corresponding QDom classes, which has
 * @p ownerDoc as the owner document (QDomDocument instance).
 * The converted @p node (and its children) are added to ownerDoc.
 *
 * NOTE:
 * - If ownerDoc is not empty, this may fail, @see QDomDocument
 * - @p node must not be a QDomDocument, use asQDomDocument()
 *
 * @see asQDomDocument, asQDomElement
 */
KRITASTORE_EXPORT void asQDomNode(QDomDocument& ownerDoc, const QDomNode& node);

/**
 * Convert QDomNode classes to the corresponding QDom classes, which has
 * @p ownerDoc as the owner document (QDomDocument instance).
 * The converted @p element (and its children) is added to ownerDoc.
 *
 * NOTE: If ownerDoc is not empty, this may fail, @see QDomDocument
 *
 */
KRITASTORE_EXPORT void asQDomElement(QDomDocument& ownerDoc, const QDomElement& element);

/**
 * Converts the whole @p document into a QDomDocument
 */
KRITASTORE_EXPORT QDomDocument asQDomDocument(const QDomDocument& document);

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
    for ( QDomNode _node = parent.firstChild(); !_node.isNull(); _node = _node.nextSibling() ) \
    if ( ( elem = _node.toElement() ).isNull() ) {} else

}
#endif // KO_XMLREADER_H
