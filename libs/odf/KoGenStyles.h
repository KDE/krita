/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>
   Copyright (C) 2010 Jarosław Staniek <staniek@kde.org>

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
#include <QtCore/QFlags>
#include <KoGenStyle.h>

class KoStore;
class KoFontFace;

/**
 * @brief Repository of styles used during saving ODF documents.
 *
 * Each instance of KoGenStyles is a collection of styles whose names
 * are in the same "namespace".
 * This means there should be one instance for all styles in &lt;office:styles&gt;,
 * and &lt;office:automatic-styles&gt;, another instance for number formats, another
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
 * @note The use of KoGenStyles isn't mandatory, of course. If the application
 * is already designed with user and automatic styles in mind for a given
 * set of properties, it can go ahead and save all styles directly (after
 * ensuring they have unique names).
 *
 * @author David Faure <faure@kde.org>
 */
class KOODF_EXPORT KoGenStyles
{
public:
    /// Single style with assigned name
    struct NamedStyle {
        const KoGenStyle* style; ///< @note owned by the collection
        QString name;
    };

    typedef QMultiMap<KoGenStyle, QString> StyleMap;

    KoGenStyles();
    ~KoGenStyles();

    /**
     * Those are flags for the insert() call.
     *
     * By default (NoFlag), the generated style names will look like "name1", "name2".
     * If DontAddNumberToName is set, the first name that will be tried is "name", and only if
     * that one exists, then "name1" is tried. Set DontAddNumberToName if the name given as
     * argument is supposed to be the full style name.
     * If AllowDuplicates is set, a unique style name is generated even if a similar KoGenStyle
     * already exists. In other words, the collection will now contain two equal KoGenStyle
     * and generate them with different style names.
     */
    enum InsertionFlag {
        NoFlag = 0,
        DontAddNumberToName = 1,
        AllowDuplicates = 2
    };
    Q_DECLARE_FLAGS(InsertionFlags, InsertionFlag)

    /**
     * Look up a style in the collection, inserting it if necessary.
     * This assigns a name to the style and returns it.
     *
     * @param style the style to look up.
     * @param baseName proposed (base) name for the style. Note that with the ODF,
     * the style name is never shown to the user (there's a separate display-name
     * attribute for that). So there are little reasons to use named styles anyway.
     * But this attribute can be used for clarity of the files.
     * If this name is already in use (for another style), then a number is appended
     * to it until unused name is found.
     * @param flags see Flags
     *
     * @return the name that has been assigned for the inserted style
     */
    QString insert(const KoGenStyle &style, const QString &baseName = QString(), InsertionFlags flags = NoFlag);

    /**
     * Return the entire collection of styles
     * Use this for saving the styles
     */
    StyleMap styles() const;

    /**
     * Return all styles of a given type (NOT marked for styles.xml).
     * Use this for saving the styles.
     *
     * @param type the style type, see the KoGenStyle constructor
     * @see insert()
     */
    QList<KoGenStyles::NamedStyle> styles(KoGenStyle::Type type) const;

    /**
     * Return styles of a given type, marked for styles.xml,
     * Use this for saving the styles.
     *
     * @param type the style type, see the KoGenStyle constructor
     * @see insert()
     */
    QList<KoGenStyles::NamedStyle> stylesForStylesXml(KoGenStyle::Type type) const;

    /**
     * @return an existing style by name. If no such style exists, 0 is returned.
     */
    const KoGenStyle *style(const QString& name) const;

    /**
     * @return an existing style by name, which can be modified.
     * If no such style exists, 0 is returned.
     * @warning This is DANGEROUS.
     * It basically defeats the purpose of insert()!
     * Only do this if you know for sure no other 'user' of that style will
     * be affected.
     */
    KoGenStyle* styleForModification(const QString &name);

    /**
     * Mark a given automatic style as being needed in styles.xml.
     * For instance styles used by headers and footers need to go there, since
     * they are saved in styles.xml, and styles.xml must be independent from content.xml.
     *
     * Equivalent to using KoGenStyle::setAutoStyleInStylesDotXml() but this can be done after insert().
     *
     * This operation can't be undone; once styles are promoted they can't go back
     * to being content.xml-only.
     *
     * @see styles, KoGenStyle::setAutoStyleInStylesDotXml
     */
    void markStyleForStylesXml(const QString &name);

    /**
     * Insert a font face declaration.
     * @a face should have non-empty "name" parameter, i.e. should not be null.
     *
     * Declaration with given name replaces previously inserted declaration with the same name.
     *
     * See odf 2.6 Font Face Declarations
     * and odf 14.6 Font Face Declaration.
     */
    void insertFontFace(const KoFontFace &face);

    /**
     * @return font face declaration for name @a name
     *         or null font face (see KoFontFace::isNull()) if there is no such font face.
     *
     * See odf 2.6 Font Face Declarations
     * and odf 14.6 Font Face Declaration.
     */
    KoFontFace fontFace(const QString &name) const;

    /**
     * Save the styles into the styles.xml file
     *
     * This saves all styles and font face declarations to the styles.xml file which
     * belong there. This creates the file and creates an entry in the manifest.
     *
     * @param store
     * @param mainfestwriter
     * @return true on success
     */
    bool saveOdfStylesDotXml(KoStore *store, KoXmlWriter *manifestWriter) const;

    /**
     * Placement of styles saved in saveOdfStyles() or inserted in insertRawOdfStyles().
     */
    enum StylesPlacement {
        /**
         * Creates document's office:styles tag and saves all document styles there
         * or inserts raw styles into document's office:styles.
         */
        DocumentStyles,
        /**
         * Creates styles.xml's office:master-styles tag and saves all master styles there
         * or inserts raw styles into styles.xml's office:automatic-styles.
         */
        MasterStyles,
        /**
         * Creates document's office:automatic-styles tag and saves all automatic styles there
         * or inserts raw styles into document's office:automatic-styles.
         */
        DocumentAutomaticStyles,
        /**
         * Creates styles.xml's office:automatic-styles tag and saves all automatic styles there
         * or inserts raw styles into style.xml's office:automatic-styles.
         */
        StylesXmlAutomaticStyles,
        /**
         * Creates document's office:font-face-decls tag and saves all document styles there
         * or inserts raw styles into document's office:font-face-decls.
         */
        FontFaceDecls
    };

    /**
     * Save styles of given type.
     *
     * @param placement see StylesPlacement
     * @param xmlWriter target writer
     */
    void saveOdfStyles(StylesPlacement placement, KoXmlWriter *xmlWriter) const;

    /**
     * Insert extra styles of given type.
     *
     * This inserts extra styles as raw xml into a given placement.
     * The information is collected and written back when saveOdfStyles() is called.
     * This method is useful for testing purposes.
     *
     * @param placement see StylesPlacement
     * @param xml the raw xml string
     */
    void insertRawOdfStyles(StylesPlacement placement, const QByteArray &xml);

    /**
     * register a relation for a previously inserted style to a previously inserted target style.
     * This allows you to insert a style relation based on generated names.
     */
    void insertStyleRelation(const QString &source, const QString &target, const char *tagName);

private:
    friend KOODF_EXPORT QDebug operator<<(QDebug dbg, const KoGenStyles& styles);

    class Private;
    Private * const d;
};

Q_DECLARE_OPERATORS_FOR_FLAGS(KoGenStyles::InsertionFlags)

//! Debug stream operator.
KOODF_EXPORT QDebug operator<<(QDebug dbg, const KoGenStyles& styles);

#endif /* KOGENSTYLES_H */
