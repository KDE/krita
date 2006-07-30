/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KOOASISLOADINGCONTEXT_H
#define KOOASISLOADINGCONTEXT_H

class KoXmlWriter;
class QDomElement;
class KoDocument;
class KoOasisStyles;
class KoPictureCollection;
class KoStore;

#include <QMap>
#include <koffice_export.h>
#include <QStringList>
#include <KoStyleStack.h>
#include <KoXmlReader.h>

/**
 * Used during loading of Oasis format (and discarded at the end of the loading).
 *
 * @author David Faure <faure@kde.org>
 */
class KOFFICECORE_EXPORT KoOasisLoadingContext
{
public:
    /**
     * Stores reference to the KoOasisStyles and stored passed by KoDocument.
     * Make sure that the KoOasisStyles instance outlives this KoOasisLoadingContext instance.
     * (This is the case during loading, when using the KoOasisStyles given by KoDocument)
     *
     * @param doc the KoDocument being loaded
     * @param styles reference to the KoOasisStyles parsed by KoDocument
     * @param store pointer to store, if available, for e.g. loading images.
     */
    KoOasisLoadingContext( KoDocument* doc, KoOasisStyles& styles, KoStore* store );
    ~KoOasisLoadingContext();

    KoDocument* koDocument() { return m_doc; }
    KoStore* store() { return m_store; }

    KoOasisStyles& oasisStyles() { return m_styles; }
    KoStyleStack& styleStack() { return m_styleStack; }

    const KoXmlDocument& manifestDocument() const { return m_manifestDoc; }

    /// Return the <meta:generator> of the document, e.g. "KOffice/1.4.0a"
    QString generator() const;

    /**
     * Convenience method for loading the style of an object
     * before loading that object.
     *
     * Read attribute (nsURI,attrName) from the given dom element,
     * treat that attribute as a style name, and load that style
     * including all its parent styles.
     * @param element the dom element to read the attribute from
     * @param nsURI the namespace URI of the attribute to read
     * @param attrName the name of the attribute to read
     * @param family the style family used for this object
     */
    void fillStyleStack( const KoXmlElement& element, const char* nsURI, const char* attrName, const char* family );

    /**
     * Add @p style to the stack, as well as all its parent styles
     * and the default style for this style family.
     *
     * @param style the dom element containing the style to add to the stack
     * @param family the family to use when looking up parent styles
     * @param usingStylesAutoStyles if true, the parent styles are looked up
     *   in the automatic styles from styles.xml, instead of looking up
     *   in the automatic styles from content.xml as we usually do.
     *   This is useful for loading headers and footers for instance.
     *   See setUseStylesAutoStyles(), which makes fillStyleStack() set this bool.
     *
     * Usually you would call fillStyleStack() instead.
     */
    void addStyles( const KoXmlElement* style, const char* family, bool usingStylesAutoStyles = false );

    /// Set to true while loading headers and footers, to remember to use auto styles
    /// from styles.xml
    void setUseStylesAutoStyles( bool useStylesAutoStyles ) { m_useStylesAutoStyles = useStylesAutoStyles; }
    //bool useStylesAutoStyles() const { return m_useStylesAutoStyles; }

private:
    void parseMeta() const;

private:
    KoDocument* m_doc;
    KoStore* m_store;
    KoOasisStyles& m_styles;
    KoStyleStack m_styleStack;

    mutable QString m_generator;
    mutable bool m_metaXmlParsed;
    bool m_useStylesAutoStyles;
    bool m_unused1; // padding, can be used later
    bool m_unused2; // padding, can be used later

    KoXmlDocument m_manifestDoc;

    class Private;
    Private *d;
};

#endif /* KOOASISLOADINGCONTEXT_H */

