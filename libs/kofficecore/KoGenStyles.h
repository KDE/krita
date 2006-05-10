/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>

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
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOGENSTYLES_H
#define KOGENSTYLES_H

#include <QMap>
#include <q3valuevector.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <QString>
#include <koffice_export.h>
#include "KoGenStyle.h"

class KoXmlWriter;

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
class KOFFICECORE_EXPORT KoGenStyles
{
public:
    KoGenStyles();
    ~KoGenStyles();

    /**
     * Those are flags for the lookup() call.
     *
     * By default, the generated style names will look like "name1", "name2".
     * If DontForceNumbering is set, the first name that will be tried is "name", and only if
     * that one exists, then "name1" is tried. Set DontForceNumbering if the name given as
     * argument is supposed to be the full style name.
     *
     */
    enum Flags { // bitfield
        NoFlag = 0,
        ForceNumbering = 0, // it's the default anyway
        DontForceNumbering = 1
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
    QString lookup( const KoGenStyle& style, const QString& name = QString::null, int flags = NoFlag );

    typedef QMap<KoGenStyle, QString> StyleMap;
    /**
     * Return the entire collection of styles
     * Use this for saving the styles
     */
    const StyleMap& styles() const { return m_styleMap; }

    struct NamedStyle {
        const KoGenStyle* style; ///< @note owned by the collection
        QString name;
    };
    /**
     * Return all styles of a given type
     * Use this for saving the styles
     *
     * @param type the style type, see the KoGenStyle constructor
     * @param markedForStylesXml if true, return only style marked for styles.xml,
     * otherwise only those NOT marked for styles.xml.
     * @see lookup
     */
    Q3ValueList<NamedStyle> styles( int type, bool markedForStylesXml = false ) const;

    /**
     * @return an existing style by name
     */
    const KoGenStyle* style( const QString& name ) const;

    /**
     * @return an existing style by name, which can be modified.
     * @warning This is DANGEROUS.
     * It basically defeats the purpose of lookup()!
     * Only do this if you know for sure no other 'user' of that style will
     * be affected.
     */
    KoGenStyle* styleForModification( const QString& name );

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
    void markStyleForStylesXml( const QString& name );

    /**
     * Outputs debug information
     */
    void dump();

private:
    QString makeUniqueName( const QString& base, int flags ) const;

    /// style definition -> name
    StyleMap m_styleMap;

    /// Map with the style name as key.
    /// This map is mainly used to check for name uniqueness
    /// The value of the bool doesn't matter.
    typedef QMap<QString, bool> NameMap; // KDE4: QSet
    NameMap m_styleNames;
    NameMap m_autoStylesInStylesDotXml;

    /// List of styles (used to preserve ordering)
    typedef Q3ValueVector<NamedStyle> StyleArray;
    StyleArray m_styleArray;

    class Private;
    Private *d;
};

#endif /* KOGENSTYLES_H */
