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

#include <q3dict.h>
#include <qmap.h>
#include <q3valuevector.h>
//Added by qt3to4:
#include <Q3ValueList>
#include <Q3CString>
#include <QString>
#include <koffice_export.h>

class KoXmlWriter;
class KoGenStyle;

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

/**
 * A generic style, i.e. basically a collection of properties and a name.
 * Instances of KoGenStyle can either be held in the KoGenStyles collection,
 * or created (e.g. on the stack) and given to KoGenStyles::lookup.
 *
 * @author David Faure <faure@kde.org>
 */
class KOFFICECORE_EXPORT KoGenStyle
{
public:
    /**
     * Possible values for the "type" argument of the KoGenStyle constructor.
     * Those values can be extended by applications (starting at number 20),
     * it's for their own consumption anyway.
     * (The reason for having the very common ones here, is to make it possible to
     * use them from libkotext).
     */
    enum { STYLE_PAGELAYOUT = 0,
           STYLE_USER = 1,
           STYLE_AUTO = 2,
           STYLE_MASTER = 3,
           STYLE_LIST = 4,
           STYLE_AUTO_LIST = 5,
           // 6 is available
           STYLE_NUMERIC_DATE = 7,
           STYLE_NUMERIC_TIME = 8,
           STYLE_NUMERIC_FRACTION = 9,
           STYLE_NUMERIC_PERCENTAGE = 10,
           STYLE_NUMERIC_SCIENTIFIC = 11,
           STYLE_NUMERIC_CURRENCY = 12,
           STYLE_NUMERIC_TEXT = 13,
           STYLE_HATCH = 14,
           STYLE_GRAPHICAUTO = 15};

    /**
     * Start the definition of a new style. Its name will be set later by KoGenStyles::lookup(),
     * but first you must define its properties and attributes.
     *
     * @param type this is a hook for the application to categorize styles
     * See the STYLE_* enum. Ignored when writing out the style.
     *
     * @param familyName The value for style:family, e.g. text, paragraph, graphic etc.
     * The family is for style:style elements only; number styles and list styles don't have one.
     *
     * @param parentName If set, name of the parent style from which this one inherits.
     */
    explicit KoGenStyle( int type = 0, const char* familyName = 0,
                         const QString& parentName = QString::null );
    ~KoGenStyle();

    /*
     * setAutoStyleInStylesDotXml(true) marks a given automatic style as being needed in styles.xml.
     * For instance styles used by headers and footers need to go there, since
     * they are saved in styles.xml, and styles.xml must be independent from content.xml.
     *
     * The application should use KoGenStyles::styles( type, true ) in order to retrieve
     * those styles and save them separately.
     */
    void setAutoStyleInStylesDotXml( bool b ) { m_autoStyleInStylesDotXml = b; }
    /// @return the value passed to setAutoStyleInStylesDotXml; false by default
    bool autoStyleInStylesDotXml() const { return m_autoStyleInStylesDotXml; }

    /*
     * setDefaultStyle(true) marks a given style as being the default style.
     * This means we expect that you will call writeStyle( ...,"style:default-style"),
     * and its name will be ommitted in the output.
     */
    void setDefaultStyle( bool b ) { m_defaultStyle = b; }
    /// @return the value passed to setDefaultStyle; false by default
    bool isDefaultStyle() const { return m_defaultStyle; }

    /// Return the type of this style, as set in the constructor
    int type() const { return m_type; }

    /// Return the family name
    const char* familyName() const { return m_familyName.data(); }

    /// Return the name of style's parent, if set
    QString parentName() const { return m_parentName; }

    /**
     *  @brief The types of properties
     *
     *  Simple styles only write one foo-properties tag, in which case they can just use DefaultType.
     *  However a given style might want to write several kinds of properties, in which case it would
     *  need to use other property types than the default one.
     *
     *  For instance this style:
     *  @code
     *  <style:style style:family="chart">
     *    <style:chart-properties .../>
     *    <style:graphic-properties .../>
     *    <style:text-properties .../>
     *  </style:style>
     *  @endcode
     *  would use DefaultType for chart-properties (and would pass "style:chart-properties" to writeStyle(),
     *  and would use GraphicType and TextType.
     */
    enum PropertyType
    {
        /**
         *  DefaultType depends on family: e.g. paragraph-properties if family=paragraph
         *  or on the type of style (e.g. page-layout -> page-layout-properties).
         *  (In fact that tag name is the one passed to writeStyle)
         */
        DefaultType = 0,
        /// TextType is always text-properties.
        TextType,
        /// ParagraphType is always paragraph-properties.
        ParagraphType,
        /// GraphicType is always graphic-properties.
        GraphicType,
        Reserved1, ///< @internal for binary compatible extensions
        Reserved2, ///< @internal for binary compatible extensions
        ChildElement, ///< @internal
        N_NumTypes ///< @internal - warning, adding items here affects binary compatibility (size of KoGenStyle)
    };

    /// Add a property to the style
    void addProperty( const QString& propName, const QString& propValue, PropertyType type = DefaultType ) {
        m_properties[type].insert( propName, propValue );
    }
    /// Overloaded version of addProperty that takes a char*, usually for "..."
    void addProperty( const QString& propName, const char* propValue, PropertyType type = DefaultType ) {
        m_properties[type].insert( propName, QString::fromUtf8( propValue ), type );
    }
    /// Overloaded version of addProperty that converts an int to a string
    void addProperty( const QString& propName, int propValue, PropertyType type = DefaultType ) {
        m_properties[type].insert( propName, QString::number( propValue ), type );
    }
    /// Overloaded version of addProperty that converts a bool to a string (false/true)
    void addProperty( const QString& propName, bool propValue, PropertyType type = DefaultType ) {
        m_properties[type].insert( propName, propValue ? "true" : "false", type );
    }

    /**
     *  Add a property which represents a distance, measured in pt
     *  The number is written out with the highest possible precision
     *  (unlike QString::number and setNum, which default to 6 digits),
     *  and the unit name ("pt") is appended to it.
     */
    void addPropertyPt( const QString& propName, double propValue, PropertyType type = DefaultType );

    /**
     *  Add an attribute to the style
     *  The difference between property and attributes is a bit oasis-format-specific:
     *  attributes are for the style element itself, and properties are in the style:properties child element
     */
    void addAttribute( const QString& attrName, const QString& attrValue ) {
        m_attributes.insert( attrName, attrValue );
    }
    /// Overloaded version of addAttribute that takes a char*, usually for "..."
    void addAttribute( const QString& attrName, const char* attrValue ) {
        m_attributes.insert( attrName, QString::fromUtf8( attrValue ) );
    }
    /// Overloaded version of addAttribute that converts an int to a string
    void addAttribute( const QString& attrName, int attrValue ) {
        m_attributes.insert( attrName, QString::number( attrValue ) );
    }

    /// Overloaded version of addAttribute that converts a bool to a string
    void addAttribute( const QString& attrName, bool attrValue ) {
        m_attributes.insert( attrName, attrValue ? "true" : "false" );
    }

    /**
     *  Add an attribute which represents a distance, measured in pt
     *  The number is written out with the highest possible precision
     *  (unlike QString::number and setNum, which default to 6 digits),
     *  and the unit name ("pt") is appended to it.
     */
    void addAttributePt( const QString& attrName, double attrValue );

    /**
     * @brief Add a child element to the style properties.
     *
     * What is meant here is that the contents of the QString
     * will be written out literally. This means you should use
     * KoXmlWriter to generate it:
     * @code
     * QBuffer buffer;
     * buffer.open( QIODevice::WriteOnly );
     * KoXmlWriter elementWriter( &buffer );  // TODO pass indentation level
     * elementWriter.startElement( "..." );
     * ...
     * elementWriter.endElement();
     * QString elementContents = QString::fromUtf8( buffer.buffer(), buffer.buffer().size() );
     * gs.addChildElement( "...", elementContents );
     * @endcode
     *
     * The value of @p elementName isn't used, except that it must be unique.
     */
    void addChildElement( const QString& elementName, const QString& elementContents ) {
        m_properties[ChildElement].insert( elementName, elementContents );
    }

    /**
     * @brief Add a style:map to the style.
     * @param styleMap the attributes for the map, associated as (name,value).
     */
    void addStyleMap( const QMap<QString, QString>& styleMap ) {
        m_maps.append( styleMap );
    }

    /**
     * @return true if the style has no attributes, no properties, no style map etc.
     * This can be used by applications which do not save all attributes unconditionally,
     * but only those that differ from the parent. But note that lookup() can't find this out...
     */
    bool isEmpty() const {
        if ( !m_attributes.isEmpty() || ! m_maps.isEmpty() )
            return false;
        for ( uint i = 0 ; i < N_NumTypes ; ++i )
            if ( ! m_properties[i].isEmpty() )
                return false;
        return true;
    }

    /**
     *  Write the definition of this style to @p writer, using the OASIS format.
     *  @param writer the KoXmlWriter in which @p elementName will be created and filled in
     *  @param styles the styles collection, used to look up the parent style
     *  @param elementName the name of the XML element, e.g. "style:style". Don't forget to
     *  pass style:default-style if isDefaultStyle().
     *  @param name must come from the collection. It will be ignored if isDefaultStyle() is true.
     *  @param propertiesElementName the name of the XML element with the style properties,
     *  e.g. "style:text-properties". Can be 0 in special cases where there should be no such item,
     *  in which case the attributes and elements are added under the style itself.
     *  @param closeElement set it to false to be able to add more child elements to the style element
     *  @param drawElement set it to true to add "draw:name" (used for gradient/hatch style) otherwise add "style:name"
     */
    void writeStyle( KoXmlWriter* writer, KoGenStyles& styles, const char* elementName, const QString& name,
                     const char* propertiesElementName, bool closeElement = true, bool drawElement = false ) const;

    /**
     *  QMap requires a complete sorting order.
     *  Another solution would have been a qdict and a key() here, a la KoTextFormat,
     *  but the key was difficult to generate.
     *  Solutions with only a hash value (not representative of the whole data)
     *  require us to write a hashtable by hand....
     */
    bool operator<( const KoGenStyle &other ) const;

    /// Not needed for QMap, but can still be useful
    bool operator==( const KoGenStyle &other ) const;

private:
    QString property( const QString& propName, PropertyType type ) const {
        QMap<QString, QString>::const_iterator it = m_properties[type].find( propName );
        if ( it != m_properties[type].end() )
            return it.data();
        return QString::null;
    }

    QString attribute( const QString& propName ) const {
        QMap<QString, QString>::const_iterator it = m_attributes.find( propName );
        if ( it != m_attributes.end() )
            return it.data();
        return QString::null;
    }

    void writeStyleProperties( KoXmlWriter* writer, PropertyType i,
                               const char* elementName, const KoGenStyle* parentStyle ) const;

#ifndef NDEBUG
    void printDebug() const;
#endif

private:
    // Note that the copy constructor and assignment operator are allowed.
    // Better not use pointers below!
    int m_type;
    Q3CString m_familyName;
    QString m_parentName;
    /// We use QMaps since they provide automatic sorting on the key (important for unicity!)
    QMap<QString, QString> m_properties[N_NumTypes];
    QMap<QString, QString> m_attributes;
    typedef QMap<QString, QString> StyleMap;
    Q3ValueVector<StyleMap> m_maps; // we can't really sort the maps between themselves...

    bool m_autoStyleInStylesDotXml;
    bool m_defaultStyle;
    short m_unused2;

    // For lookup
    friend class KoGenStyles;
};

#endif /* KOGENSTYLES_H */
