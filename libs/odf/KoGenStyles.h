/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>

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

#ifndef KOGENSTYLES_H
#define KOGENSTYLES_H

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QMultiMap>
#include <QtCore/QSet>
#include <QtCore/QString>
#include "koodf_export.h"
#include "KoGenStyle.h"

class KoStore;
class KoFontFace;

/**
 * @brief Repository of styles used during saving of OASIS/OOo file.
 *
 * Each instance of KoGenStyles is a collection of styles whose names
 * are in the same "namespace".
 * This means there should be one instance for all styles in <office:styles>,
 * and automatic-styles, another instance for number formats, another
 * one for draw styles, and another one for list styles.
 *
 * "Style" in this context only means "a collection of properties".
 * The "Gen" means both "Generic" and "Generated" :)
 *
 * The basic design principle is the flyweight pattern: if you need
 * a style with the same properties from two different places, you
 * get the same object. Here it means rather you get the same name for the style,
 * and it will get saved only once to the file.
 *
 * KoGenStyles features sharing, creation on demand, and name generation.
 * Since this is used for saving only, it doesn't feature refcounting, nor
 * removal of individual styles.
 *
 * NOTE: the use of KoGenStyles isn't mandatory, of course. If the application
 * is already designed with user and automatic styles in mind for a given
 * set of properties, it can go ahead and save all styles directly (after
 * ensuring they have unique names).
 *
 * @author David Faure <faure@kde.org>
 */
class KOODF_EXPORT KoGenStyles
{
public:
    struct NamedStyle {
        const KoGenStyle* style; ///< @note owned by the collection
        QString name;
    };

    typedef QMultiMap<KoGenStyle, QString> StyleMap;
    typedef QSet<QString> NameMap;
    typedef QList<NamedStyle> StyleArray;

    KoGenStyles();
    ~KoGenStyles();

    /**
     * Those are flags for the lookup() call.
     *
     * By default, the generated style names will look like "name1", "name2".
     * If DontForceNumbering is set, the first name that will be tried is "name", and only if
     * that one exists, then "name1" is tried. Set DontForceNumbering if the name given as
     * argument is supposed to be the full style name.
     * If AllowDuplicates is set, a unique style name is generated even if a similar KoGenStyle
     * already exists. In other words, the collection will now contain two equal KoGenStyle
     * and generate them with different style names.
     */
    enum Flags { // bitfield
        NoFlag = 0,
        ForceNumbering = 0, // it's the default anyway
        DontForceNumbering = 1,
        AllowDuplicates = 2
    };
    // KDE4 TODO: use QFlags and change the arg type in lookup

    /**
     * Look up a style in the collection, inserting it if necessary.
     * This assigns a name to the style and returns it.
     *
     * @param style the style to look up.
     * @param name proposed (base) name for the style. Note that with the OASIS format,
     * the style name is never shown to the user (there's a separate display-name
     * attribute for that). So there are little reasons to use named styles anyway.
     * But this attribute can be used for clarity of the files.
     * If this name is already in use (for another style), then a number is appended
     * to it until unique.
     * @param flags see Flags
     *
     * @return the name for this style
     * @todo ### rename lookup to insert
     */
    QString lookup(const KoGenStyle& style, const QString& name = QString(), int flags = NoFlag);

    /**
     * Return the entire collection of styles
     * Use this for saving the styles
     */
    StyleMap styles() const;

    /**
     * Return all styles of a given type
     * Use this for saving the styles
     *
     * @param type the style type, see the KoGenStyle constructor
     * @param markedForStylesXml if true, return only style marked for styles.xml,
     * otherwise only those NOT marked for styles.xml.
     * @see lookup
     */
    QList<NamedStyle> styles(int type, bool markedForStylesXml = false) const;

    /**
     * @return an existing style by name
     */
    const KoGenStyle* style(const QString& name) const;

    /**
     * @return an existing style by name, which can be modified.
     * @warning This is DANGEROUS.
     * It basically defeats the purpose of lookup()!
     * Only do this if you know for sure no other 'user' of that style will
     * be affected.
     */
    KoGenStyle* styleForModification(const QString& name);

    /**
     * Mark a given automatic style as being needed in styles.xml.
     * For instance styles used by headers and footers need to go there, since
     * they are saved in styles.xml, and styles.xml must be independent from content.xml.
     *
     * Equivalent to using KoGenStyle::setAutoStyleInStylesDotXml() but this can be done after lookup.
     *
     * This operation can't be undone; once styles are promoted they can't go back
     * to being content.xml-only.
     *
     * @see styles, KoGenStyle::setAutoStyleInStylesDotXml
     */
    void markStyleForStylesXml(const QString& name);

    /**
     * Add a font face declaration.
     * @a face should have non-empty "name" parameter, i.e. should not be null.
     *
     * Declaration with given name replaces previously added declaration with the same name.
     *
     * See odf 2.6 Font Face Declarations
     * and odf 14.6 Font Face Declaration.
     */
    void addFontFace(const KoFontFace& face);

    /**
     * @return font face declaration for name @a name
     *         or null font face (see KoFontFace::isNull()) if there is no such font face.
     *
     * See odf 2.6 Font Face Declarations
     * and odf 14.6 Font Face Declaration.
     */
    KoFontFace fontFace(const QString& name) const;

    /**
     * Outputs debug information
     */
    void dump();

    /**
     * Save the styles into the styles.xml file
     *
     * This saves all styles and font face declarations to the styles.xml file which
     * belong there. This creates the file and creates an entry in the manifest.
     *
     * @param store
     * @param mainfestwriter
     * @return if it was successful
     */
    bool saveOdfStylesDotXml(KoStore* store, KoXmlWriter* manifestWriter);

    /**
     * Save automatic styles.
     *
     * This creates the office:automatic-styles tag containing all
     * automatic styles.
     *
     * @param xmlWriter
     * @param stylesDotXml
     */
    void saveOdfAutomaticStyles(KoXmlWriter* xmlWriter, bool stylesDotXml) const;

    /**
     * Save document styles.
     *
     * This creates the office:styles tag containing all document styles.
     *
     * @param xmlWriter
     */
    void saveOdfDocumentStyles(KoXmlWriter* xmlWriter) const;

    /**
     * Save master styles.
     *
     * This creates the office:master-styles tag containing all master styles.
     *
     * @param xmlWriter
     */
    void saveOdfMasterStyles(KoXmlWriter* xmlWriter) const;

    /**
     * Save font face declarations
     *
     * This creates the office:font-face-decls tag containing all font face
     * declarations
     */
    void saveOdfFontFaceDecls(KoXmlWriter* xmlWriter) const;

    /**
     * register a relation for a previously inserted style to a previously inserted target style.
     * This allows you to add a style relation based on generated names.
     */
    void insertStyleRelation(const QString &source, const QString &target, const char *tagName);

    /**
     * Adds extra document styles.
     *
     * This adds extra document styles as raw xml within the office:styles tag.
     * The information is collected and written back when saveOdfDocumentStyles() is called.
     * This method is useful for testing purposes.
     *
     * @param xml the raw xml string
     */
    void addRawOdfDocumentStyles(const QByteArray& xml);

    /**
     * Adds extra master styles.
     *
     * This adds extra master styles as raw xml within the office:master-styles tag.
     * The information is collected and written back when saveOdfMasterStyles() is called.
     * This method is useful for testing purposes.
     *
     * @param xml the raw xml string
     */
    void addRawOdfMasterStyles(const QByteArray& xml);

    /**
     * Adds extra automatic styles.
     *
     * This adds extra master automatic as raw xml within the office:automatic-styles tag.
     * The information is collected and written back when saveOdfAutomaticStyles() is called.
     * This method is useful for testing purposes.
     *
     * @param xml the raw xml string
     * @param stylesDotXml true if the xml should go to styles.xml instead of content.xml
     */
    void addRawOdfAutomaticStyles(const QByteArray& xml, bool stylesDotXml);

private:
    QString makeUniqueName(const QString& base, int flags) const;

    class Private;
    Private * const d;
};

#endif /* KOGENSTYLES_H */
