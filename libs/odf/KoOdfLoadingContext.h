/* This file is part of the KDE project
   Copyright (C) 2005 David Faure <faure@kde.org>
   Copyright (C) 2010 Inge Wallin <inge@lysator.liu.se>

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

#ifndef KOODFLOADINGCONTEXT_H
#define KOODFLOADINGCONTEXT_H

#include "koodf_export.h"
#include "KoOdfStylesReader.h"


class KoStore;
class KoOdfManifestEntry;
class KoStyleStack;


/**
 * Used during loading of Oasis format (and discarded at the end of the loading).
 *
 * @author David Faure <faure@kde.org>
 */
class KOODF_EXPORT KoOdfLoadingContext
{
public:
    enum GeneratorType { Unknown, Calligra, OpenOffice, MicrosoftOffice };
    /**
     * Stores reference to the KoOdfStylesReader and stored passed by
     * KoDocument. Make sure that the KoOdfStylesReader instance outlives
     * this KoOdfLoadingContext instance. (This is the case during
     * loading, when using the KoOdfStylesReader given by KoDocument)
     *
     * @param styles reference to the KoOdfStylesReader parsed by KoDocument
     * @param store pointer to store, if available, for e.g. loading images.
     * @param defaultStylesResourcePath resource path to "defaultstyles.xml", empty if none
     */
    explicit KoOdfLoadingContext(KoOdfStylesReader &stylesReader, KoStore *store, const QString &defaultStylesResourcePath = QString());
    virtual ~KoOdfLoadingContext();

    /**
    * Set different manifest
    * @param fileName file name of the manifest file
    */
    void setManifestFile(const QString &fileName);

    KoStore *store() const;

    KoOdfStylesReader &stylesReader();

    /**
    * Get the application default styles styleReader
    */
    KoOdfStylesReader &defaultStylesReader();

    KoStyleStack &styleStack() const;

    /// Return the <meta:generator> of the document, e.g. "Calligra/1.4.0a"
    QString generator() const;
    /// Return the GeneratorType of the document, e.g. Calligra
    GeneratorType generatorType() const;

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
    void fillStyleStack(const KoXmlElement &element, const QString &nsURI, const QString &attrName, const QString &family);

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
    void addStyles(const KoXmlElement *style, const QString &family, bool usingStylesAutoStyles = false);

    /// Set to true while loading headers and footers, to remember to use auto styles
    /// from styles.xml
    void setUseStylesAutoStyles(bool useStylesAutoStyles);
    bool useStylesAutoStyles() const;


    /**
     * @return the mimetype for the document in the given path using the manifest
     * The mimetype is defined in the manifest.xml document.
     * @param path The path to get the mimetpye for
     * @param guess If set to true it tries to guess the mimetype in case it is not provided in the manifest
                    Note: Be sure to call this function with a closed store when guess is set to true as otherwise
                    it will fail.
     */
    QString mimeTypeForPath(const QString& path, bool guess = false) const;

    /**
     * @return the full list of entries from the manifest file
     */
    QList<KoOdfManifestEntry*> manifestEntries() const;

private:
    class Private;
    Private * const d;
    /// Parse and set generator and generatorType attributes from <meta:generator> attribute of meta.xml file
    void parseGenerator() const;
    bool parseManifest(const KoXmlDocument &manifestDocument);
};

#endif /* KOODFLOADINGCONTEXT_H */
