/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Inge Wallin <inge@lysator.liu.se>
   Copyright (C) 2010 KO GmbH <jos.van.den.oever@kogmbh.com>
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

#ifndef KOGENSTYLE_H
#define KOGENSTYLE_H

#include <QtCore/QList>
#include <QtCore/QMap>
#include <QtCore/QString>
#include <iostream>
#include "koodf_export.h"

class KoGenStyles;
class KoXmlWriter;

/**
 * A generic style, i.e. basically a collection of properties and a name.
 * Instances of KoGenStyle can either be held in the KoGenStyles collection,
 * or created (e.g. on the stack) and given to KoGenStyles::insert().
 *
 * @author David Faure <faure@kde.org>
 */
class KOODF_EXPORT KoGenStyle
{
public:
    /**
     * Possible values for the "type" argument of the KoGenStyle constructor.
     * @note If there is still something missing, add it here so that it is possible to use the same
     *       saving code in all applications.
     */
    enum Type {
        PageLayoutStyle,             ///< style:page-layout as in odf 14.3 Page Layout
        TextStyle,                   ///< style:style from family "text" as in odf 14.8.1 Text Styles
                                     ///<  (office:styles)
        TextAutoStyle,               ///< style:style from family "text" as in odf 14.8.1 Text Styles
                                     ///<  (office:automatic-styles)
        ParagraphStyle,              ///< style:style from family "paragraph" as in odf 14.1 Style Element
                                     ///<  (office:styles)
        ParagraphAutoStyle,          ///< style:style from family "paragraph" as in odf 14.1 Style Element
                                     ///<  (office:automatic-styles)
        SectionStyle,                ///< style:style from family "section" as in odf 14.8.3 Section Styles
                                     ///<  (office:styles)
        SectionAutoStyle,            ///< style:style from family "section" as in odf 14.8.3 Section Styles
                                     ///<  (office:automatic-styles)
        RubyStyle,                   ///< style:style from family "ruby" as in odf 14.8.4 Ruby Style
                                     ///<  (office:styles)
        RubyAutoStyle,               ///< style:style from family "ruby" as in odf 14.8.4 Ruby Style
                                     ///<  (office:automatic-styles)
        TableStyle,                  ///< style:style from family "table" as in odf 14.12 Table Formatting
                                     ///<  Properties (office:styles)
        TableAutoStyle,              ///< style:style from family "table" as in odf 14.12 Table Formatting Properties
                                     ///<  (office:automatic-styles)
        TableColumnStyle,            ///< style:style from family "table-column" as in odf 15.9 Column Formatting
                                     ///<  Properties (office:styles)
        TableColumnAutoStyle,        ///< style:style from family "table-column" as in odf 15.9 Column Formatting
                                     ///<  Properties (office:automatic-styles)
        TableRowStyle,               ///< style:style from family "table-row" as in odf 15.10 Table Row Formatting
                                     ///<  Properties (office:styles)
        TableRowAutoStyle,           ///< style:style from family "table-row" as in odf 15.10 Table Row Formatting
                                     ///<  Properties (office:automatic-styles)
        TableCellStyle,              ///< style:style from family "table-cell" as in odf 15.11 Table Cell Formatting
                                     ///<  Properties (office:styles)
        TableCellAutoStyle,          ///< style:style from family "table-cell" as in odf 15.11 Table Cell Formatting
                                     ///<  Properties (office:automatic-styles)
        GraphicStyle,                ///< style:style from family "graphic" as in 14.13.1 Graphic and Presentation
                                     ///<  Styles (office:automatic-styles)
        GraphicAutoStyle,            ///< style:style from family "graphic" as in 14.13.1 Graphic and Presentation
                                     ///<  Styles (office:automatic-styles)
        PresentationStyle,           ///< style:style from family "presentation" as in 14.13.1 Graphic and
                                     ///<  Presentation Styles (office:styles)
        PresentationAutoStyle,       ///< style:style from family "presentation" as in 14.13.1 Graphic and
                                     ///<  Presentation Styles (office:automatic-styles)
        DrawingPageStyle,            ///< style:style from family "drawing-page" as in odf 14.13.2 Drawing Page Style
                                     ///<  (office:styles)
        DrawingPageAutoStyle,        ///< style:style from family "drawing-page" as in odf 14.13.2 Drawing Page Style
                                     ///<  (office:automatic-styles)
        ChartStyle,                  ///< style:style from family "chart" as in odf 14.16 Chart Styles
                                     ///<  (office:styles)
        ChartAutoStyle,              ///< style:style from family "chart" as in odf 14.16 Chart Styles
                                     ///<  (office:automatic-styles)

        ListStyle,                   ///< text:list-style as in odf 14.10 List Style (office:styles)
        ListAutoStyle,               ///< text:list-style as in odf 14.10 List Style (office:automatic-styles)
        NumericNumberStyle,          ///< number:number-style as in odf 14.7.1 Number Style
        NumericDateStyle,            ///< number:date-style as in odf 14.7.4 Date Style
        NumericTimeStyle,            ///< number:time-style as in odf 14.7.5 Time Style
        NumericFractionStyle,        ///< number:number-style as in odf 14.7.1 Number Style
        NumericPercentageStyle,      ///< number:percentage-style as in odf 14.7.3 Percentage Style
        NumericScientificStyle,      ///< number:number-style as in odf 14.7.1 Number Style
        NumericCurrencyStyle,        ///< number:currency-style as in odf 14.7.2 Currency Style
        NumericTextStyle,            ///< number:text-style 14.7.7 Text Style
                                     ///<  @note unused
        HatchStyle,                  ///< draw:hatch as in odf 14.14.3 Hatch (office:styles)
        StrokeDashStyle,             ///< draw:stroke-dash as in odf 14.14.7 Stroke Dash (office:styles)
        GradientStyle,               ///< draw:gradient as in odf 14.14.1 Gradient (office:styles)
        LinearGradientStyle,         ///< svg:linearGradient as in odf 14.14.2 SVG Gradients (office:styles)
        RadialGradientStyle,         ///< svg:radialGradient as in odf 14.14.2 SVG Gradients (office:styles)
        ConicalGradientStyle,        ///< koffice:conicalGradient koffice extension for conical gradients
        FillImageStyle,              ///< draw:fill-image as in odf 14.14.4 Fill Image (office:styles)
        NumericBooleanStyle,         ///< number:boolean 14.7.6 Boolean Style
                                     ///<  @note unused
        OpacityStyle,                ///< draw:opacity as in odf 14.14.5 Opacity Gradient
                                     ///<  @note unused
        MarkerStyle,                 ///< draw:marker as in odf 14.14.6 Marker
        PresentationPageLayoutStyle, ///< style:presentation-page-layout as in odf 14.15 Presentation Page Layouts
        //   TODO differently
        MasterPageStyle,             ///< style:master-page as in odf 14.4 14.4 Master Pages (office:master-styles)
        // style:default-style as in odf 14.2 Default Styles
        // 14.5 Table Templates
        /// @internal @note always update when adding values to this enum
        LastStyle = MasterPageStyle
    };

    /**
     * Start the definition of a new style. Its name will be set later by KoGenStyles::insert(),
     * but first you must define its properties and attributes.
     *
     * @param type this is a hook for the application to categorize styles
     * See the Style* enum. Ignored when writing out the style.
     *
     * @param familyName The value for style:family, e.g. text, paragraph, graphic etc.
     * The family is for style:style elements only; number styles and list styles don't have one.
     *
     * @param parentName If set, name of the parent style from which this one inherits.
     */
    explicit KoGenStyle(Type type = PageLayoutStyle, const char *familyName = 0,
                        const QString &parentName = QString());
    ~KoGenStyle();

    /**
     * setAutoStyleInStylesDotXml(true) marks a given automatic style as being needed in styles.xml.
     * For instance styles used by headers and footers need to go there, since
     * they are saved in styles.xml, and styles.xml must be independent from content.xml.
     *
     * The application should use KoGenStyles::styles( type, true ) in order to retrieve
     * those styles and save them separately.
     */
    void setAutoStyleInStylesDotXml(bool b) {
        m_autoStyleInStylesDotXml = b;
    }
    /// @return the value passed to setAutoStyleInStylesDotXml; false by default
    bool autoStyleInStylesDotXml() const {
        return m_autoStyleInStylesDotXml;
    }

    /**
     * setDefaultStyle(true) marks a given style as being the default style.
     * This means we expect that you will call writeStyle( ...,"style:default-style"),
     * and its name will be ommitted in the output.
     */
    void setDefaultStyle(bool b) {
        m_defaultStyle = b;
    }
    /// @return the value passed to setDefaultStyle; false by default
    bool isDefaultStyle() const {
        return m_defaultStyle;
    }

    /// Return the type of this style, as set in the constructor
    Type type() const {
        return m_type;
    }

    /// Return the family name
    const char* familyName() const {
        return m_familyName.data();
    }

    /// Sets the name of style's parent.
    void setParentName(const QString &name) {
        m_parentName = name;
    }

    /// Return the name of style's parent, if set
    QString parentName() const {
        return m_parentName;
    }

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
    enum PropertyType {
        /**
         *  DefaultType depends on family: e.g. paragraph-properties if family=paragraph
         *  or on the type of style (e.g. page-layout -> page-layout-properties).
         *  (In fact that tag name is the one passed to writeStyle)
         */
        DefaultType,
        /// TextType is always text-properties.
        TextType,
        /// ParagraphType is always paragraph-properties.
        ParagraphType,
        /// GraphicType is always graphic-properties.
        GraphicType,
        /// SectionType is always section-properties.
        SectionType,
        /// RubyType is always ruby-properties.
        RubyType,
        /// TableType is always table-properties.
        TableType,
        /// TableColumnType is always table-column-properties
        TableColumnType,
        /// TableRowType is always table-row-properties.
        TableRowType,
        /// TableCellType is always for table-cell-properties.
        TableCellType,
        /// PresentationType is always for presentation-properties.
        PresentationType,
        /// DrawingPageType is always for presentation-properties.
        DrawingPageType,
        /// ChartType is always for presentation-properties.
        ChartType,
        Reserved1, ///< @internal for binary compatible extensions
        /// For elements that are children of the style itself, not any of the properties
        StyleChildElement,
        ChildElement, ///< @internal
        /// @internal @note always update when adding values to this enum
        LastPropertyType = ChildElement
    };

    /// Add a property to the style
    void addProperty(const QString &propName, const QString &propValue, PropertyType type = DefaultType) {
        if (type == DefaultType) {
            type = m_propertyType;
        }
        m_properties[type].insert(propName, propValue);
    }
    /// Overloaded version of addProperty that takes a char*, usually for "..."
    void addProperty(const QString &propName, const char *propValue, PropertyType type = DefaultType) {
        if (type == DefaultType) {
            type = m_propertyType;
        }
        m_properties[type].insert(propName, QString::fromUtf8(propValue));
    }
    /// Overloaded version of addProperty that converts an int to a string
    void addProperty(const QString &propName, int propValue, PropertyType type = DefaultType) {
        if (type == DefaultType) {
            type = m_propertyType;
        }
        m_properties[type].insert(propName, QString::number(propValue));
    }
    /// Overloaded version of addProperty that converts a bool to a string (false/true)
    void addProperty(const QString &propName, bool propValue, PropertyType type = DefaultType) {
        if (type == DefaultType) {
            type = m_propertyType;
        }
        m_properties[type].insert(propName, propValue ? "true" : "false");
    }

    /**
     *  Add a property which represents a distance, measured in pt
     *  The number is written out with the highest possible precision
     *  (unlike QString::number and setNum, which default to 6 digits),
     *  and the unit name ("pt") is appended to it.
     */
    void addPropertyPt(const QString &propName, qreal propValue, PropertyType type = DefaultType);

    /**
     *  Add an attribute to the style
     *  The difference between property and attributes is a bit oasis-format-specific:
     *  attributes are for the style element itself, and properties are in the style:properties child element
     */
    void addAttribute(const QString &attrName, const QString& attrValue) {
        m_attributes.insert(attrName, attrValue);
    }
    /// Overloaded version of addAttribute that takes a char*, usually for "..."
    void addAttribute(const QString &attrName, const char* attrValue) {
        m_attributes.insert(attrName, QString::fromUtf8(attrValue));
    }
    /// Overloaded version of addAttribute that converts an int to a string
    void addAttribute(const QString &attrName, int attrValue) {
        m_attributes.insert(attrName, QString::number(attrValue));
    }

    /// Overloaded version of addAttribute that converts a bool to a string
    void addAttribute(const QString &attrName, bool attrValue) {
        m_attributes.insert(attrName, attrValue ? "true" : "false");
    }

    /**
     *  Add an attribute which represents a distance, measured in pt
     *  The number is written out with the highest possible precision
     *  (unlike QString::number and setNum, which default to 6 digits),
     *  and the unit name ("pt") is appended to it.
     */
    void addAttributePt(const QString &attrName, qreal attrValue);

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
    void addChildElement(const QString &elementName, const QString& elementContents) {
        m_properties[ChildElement].insertMulti(elementName, elementContents);
    }

    /**
     * @brief Add a style:map to the style.
     * @param styleMap the attributes for the map, associated as (name,value).
     */
    void addStyleMap(const QMap<QString, QString> &styleMap);

    /**
     * @return true if the style has no attributes, no properties, no style map etc.
     * This can be used by applications which do not save all attributes unconditionally,
     * but only those that differ from the parent. But note that KoGenStyles::insert() can't find this out...
     */
    bool isEmpty() const;

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
    void writeStyle(KoXmlWriter *writer, const KoGenStyles &styles, const char *elementName, const QString &name,
                    const char *propertiesElementName, bool closeElement = true, bool drawElement = false) const;

    /**
     *  Write the definition of these style properties to @p writer, using the OASIS format.
     *  @param writer the KoXmlWriter in which @p elementName will be created and filled in
     *  @param type the type of properties to write
     *  @param parentStyle the parent to this style
     */
    void writeStyleProperties(KoXmlWriter *writer, PropertyType type,
                              const KoGenStyle *parentStyle = 0) const;

    /**
     *  QMap requires a complete sorting order.
     *  Another solution would have been a qdict and a key() here, a la KoTextFormat,
     *  but the key was difficult to generate.
     *  Solutions with only a hash value (not representative of the whole data)
     *  require us to write a hashtable by hand....
     */
    bool operator<(const KoGenStyle &other) const;

    /// Not needed for QMap, but can still be useful
    bool operator==(const KoGenStyle &other) const;

private:
    QString property(const QString &propName, PropertyType type) const {
        const QMap<QString, QString>::const_iterator it = m_properties[type].constFind(propName);
        if (it != m_properties[type].constEnd())
            return it.value();
        return QString();
    }

    QString attribute(const QString &propName) const {
        const QMap<QString, QString>::const_iterator it = m_attributes.constFind(propName);
        if (it != m_attributes.constEnd())
            return it.value();
        return QString();
    }


#ifndef NDEBUG
    void printDebug() const;
#endif

private:
    // Note that the copy constructor and assignment operator are allowed.
    // Better not use pointers below!
    // TODO turn this into a QSharedData class
    PropertyType m_propertyType;
    Type m_type;
    QByteArray m_familyName;
    QString m_parentName;
    /// We use QMaps since they provide automatic sorting on the key (important for unicity!)
    typedef QMap<QString, QString> StyleMap;
    StyleMap m_properties[LastPropertyType+1];
    StyleMap m_attributes;
    QList<StyleMap> m_maps; // we can't really sort the maps between themselves...

    bool m_autoStyleInStylesDotXml;
    bool m_defaultStyle;
    short m_unused2;

    // For insert()
    friend class KoGenStyles;
};

#endif /* KOGENSTYLE_H */
