/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Inge Wallin <inge@lysator.liu.se>

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
 * or created (e.g. on the stack) and given to KoGenStyles::lookup.
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
        StylePageLayout,             ///< style:page-layout as in odf 14.3 Page Layout
        StyleUser,                   ///< style:style with style:paragraph-properties as in odf 14.1 Style Element (office:styles)
        StyleAuto,                   ///< style:style with style:paragraph-properties as in odf 14.1 Style Element (office:automatic-styles)
        StyleText,                   ///< style:style with style:text-properties as in odf 14.8.1 Text Styles (office:styles)
        StyleTextAuto,               ///< style:style with style:text-properties as in odf 14.8.1 Text Styles (office:automatic-styles)
        StyleChart,                  ///< style:style with style:chart-properties
        StyleChartAuto,              ///< style:style with style:chart-properties
        StyleList,                   ///< text:list-style as in odf 14.10 List Style (office:styles)
        StyleListAuto,               ///< text:list-style as in odf 14.10 List Style (office:automatic-styles)
        StyleSection,                ///< style:style with style:section-properties as in odf 14.8.3 Section Styles (office:styles)
        StyleSectionAuto,            ///< style:style with style:section-properties as in odf 14.8.3 Section Styles (office:automatic-styles)
        StyleNumericNumber,          ///< number:number-style as in odf 14.7.1 Number Style
        StyleNumericDate,            ///< number:date-style as in odf 14.7.4 Date Style
        StyleNumericTime,            ///< number:time-style as in odf 14.7.5 Time Style
        StyleNumericFraction,        ///< number:number-style as in odf 14.7.1 Number Style
        StyleNumericPercentage,      ///< number:percentage-style as in odf 14.7.3 Percentage Style
        StyleNumericScientific,      ///< number:number-style as in odf 14.7.1 Number Style
        StyleNumericCurrency,        ///< number:currency-style as in odf 14.7.2 Currency Style
        StyleNumericText,            ///< number:text-style 14.7.7 Text Style not used
        StyleHatch,                  ///< draw:hatch as in odf 14.14.3 Hatch (office:styles)
        StyleGraphicAuto,            ///< style:style with style:graphic-properties as in 14.13.1 Graphic and Presentation Styles (office:automatic-styles)
        StylePresentationAuto,       ///< style:style with style:graphic-properties as in 14.13.1 Graphic and Presentation Styles (office:automatic-styles)
        StyleStrokeDash,             ///< draw:stroke-dash as in odf 14.14.7 Stroke Dash (office:styles)
        StyleGradient,               ///< draw:gradient as in odf 14.14.1 Gradient (office:styles)
        StyleGradientLinear,         ///< svg:linearGradient as in odf 14.14.2 SVG Gradients (office:styles)
        StyleGradientRadial,         ///< svg:radialGradient as in odf 14.14.2 SVG Gradients (office:styles)
        StyleGradientConical,        ///< koffice:conicalGradient koffice extension for conical gradients 
        StyleFillImage,              ///< draw:fill-image as in odf 14.14.4 Fill Image (office:styles)
        StyleDrawingPage,            ///< style:drawing-page-properties as in odf 14.13.2 Drawing Page Style
        StyleNumericBoolean,         ///< number:boolean 14.7.6 Boolean Style not used
        StyleOpacity,                ///< draw:opacity as in odf 14.14.5 Opacity Gradient not used
        StyleMarker,                 ///< draw:marker as in odf 14.14.6 Marker
        StylePresentationPageLayout, ///< style:presentation-page-layout as in odf 14.15 Presentation Page Layouts
        StyleAutoTable,              ///< style:table-properties as in odf 15.8 Table Formatting Properties (office:automatic-styles)
// office:style needed?
        StyleTableColumn,            ///< style:table-column-properties as in odf 15.9 Column Formatting Properties (office:style)
        StyleAutoTableColumn,        ///< style:table-column-properties as in odf 15.9 Column Formatting Properties (office:automatic-styles)
        StyleTableRow,               ///< style:table-row-properties as in odf 15.10 Table Row Formatting Properties (office:style)
        StyleAutoTableRow,           ///< style:table-row-properties as in odf 15.10 Table Row Formatting Properties (office:automatic-styles)
        StyleTableCell,              ///< style:table-cell-properties as in odf 15.11 Table Cell Formatting Properties (office:style)
        StyleAutoTableCell,          ///< style:table-cell-properties as in odf 15.11 Table Cell Formatting Properties (office:automatic-styles)
        StyleTable,                  ///< style:table-properties as in odf1.2 16.9 Table Formatting Properties (office:style)
        //   TODO differently
        StyleMaster                  ///< 14.4 Master Pages
        /// style:default-style as in odf 14.2 Default Styles
        /// 14.5 Table Templates
        /// 14.6 Font Face Declaration
    };

    /**
     * Start the definition of a new style. Its name will be set later by KoGenStyles::lookup(),
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
    explicit KoGenStyle(Type type = StylePageLayout, const char* familyName = 0,
                        const QString& parentName = QString());
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
    void setParentName(const QString& name) {
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
        DefaultType = 0,
        /// TextType is always text-properties.
        TextType,
        /// ParagraphType is always paragraph-properties.
        ParagraphType,
        /// GraphicType is always graphic-properties.
        GraphicType,
        Reserved1, ///< @internal for binary compatible extensions
        /// For elements that are children of the style itself, not any of the properties
        StyleChildElement,
        ChildElement, ///< @internal
        N_NumTypes ///< @internal - warning, adding items here affects binary compatibility (size of KoGenStyle)
    };

    /// Add a property to the style
    void addProperty(const QString& propName, const QString& propValue, PropertyType type = DefaultType) {
        m_properties[type].insert(propName, propValue);
    }
    /// Overloaded version of addProperty that takes a char*, usually for "..."
    void addProperty(const QString& propName, const char* propValue, PropertyType type = DefaultType) {
        m_properties[type].insert(propName, QString::fromUtf8(propValue));
    }
    /// Overloaded version of addProperty that converts an int to a string
    void addProperty(const QString& propName, int propValue, PropertyType type = DefaultType) {
        m_properties[type].insert(propName, QString::number(propValue));
    }
    /// Overloaded version of addProperty that converts a bool to a string (false/true)
    void addProperty(const QString& propName, bool propValue, PropertyType type = DefaultType) {
        m_properties[type].insert(propName, propValue ? "true" : "false");
    }

    /**
     *  Add a property which represents a distance, measured in pt
     *  The number is written out with the highest possible precision
     *  (unlike QString::number and setNum, which default to 6 digits),
     *  and the unit name ("pt") is appended to it.
     */
    void addPropertyPt(const QString& propName, qreal propValue, PropertyType type = DefaultType);

    /**
     *  Add an attribute to the style
     *  The difference between property and attributes is a bit oasis-format-specific:
     *  attributes are for the style element itself, and properties are in the style:properties child element
     */
    void addAttribute(const QString& attrName, const QString& attrValue) {
        m_attributes.insert(attrName, attrValue);
    }
    /// Overloaded version of addAttribute that takes a char*, usually for "..."
    void addAttribute(const QString& attrName, const char* attrValue) {
        m_attributes.insert(attrName, QString::fromUtf8(attrValue));
    }
    /// Overloaded version of addAttribute that converts an int to a string
    void addAttribute(const QString& attrName, int attrValue) {
        m_attributes.insert(attrName, QString::number(attrValue));
    }

    /// Overloaded version of addAttribute that converts a bool to a string
    void addAttribute(const QString& attrName, bool attrValue) {
        m_attributes.insert(attrName, attrValue ? "true" : "false");
    }

    /**
     *  Add an attribute which represents a distance, measured in pt
     *  The number is written out with the highest possible precision
     *  (unlike QString::number and setNum, which default to 6 digits),
     *  and the unit name ("pt") is appended to it.
     */
    void addAttributePt(const QString& attrName, qreal attrValue);

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
    void addChildElement(const QString& elementName, const QString& elementContents) {
        m_properties[ChildElement].insertMulti(elementName, elementContents);
    }

    /**
     * @brief Add a style:map to the style.
     * @param styleMap the attributes for the map, associated as (name,value).
     */
    void addStyleMap(const QMap<QString, QString>& styleMap);

    /**
     * @return true if the style has no attributes, no properties, no style map etc.
     * This can be used by applications which do not save all attributes unconditionally,
     * but only those that differ from the parent. But note that lookup() can't find this out...
     */
    bool isEmpty() const {
        if (!m_attributes.isEmpty() || ! m_maps.isEmpty())
            return false;
        for (uint i = 0 ; i < N_NumTypes ; ++i)
            if (! m_properties[i].isEmpty())
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
    void writeStyle(KoXmlWriter* writer, const KoGenStyles& styles, const char* elementName, const QString& name,
                    const char* propertiesElementName, bool closeElement = true, bool drawElement = false) const;

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
    QString property(const QString& propName, PropertyType type) const {
        QMap<QString, QString>::const_iterator it = m_properties[type].find(propName);
        if (it != m_properties[type].end())
            return it.value();
        return QString();
    }

    QString attribute(const QString& propName) const {
        QMap<QString, QString>::const_iterator it = m_attributes.find(propName);
        if (it != m_attributes.end())
            return it.value();
        return QString();
    }

    void writeStyleProperties(KoXmlWriter* writer, PropertyType i,
                              const char* elementName, const KoGenStyle* parentStyle) const;

#ifndef NDEBUG
    void printDebug() const;
#endif

private:
    // Note that the copy constructor and assignment operator are allowed.
    // Better not use pointers below!
    // TODO turn this into a QSharedData class
    Type m_type;
    QByteArray m_familyName;
    QString m_parentName;
    /// We use QMaps since they provide automatic sorting on the key (important for unicity!)
    typedef QMap<QString, QString> StyleMap;
    StyleMap m_properties[N_NumTypes];
    StyleMap m_attributes;
    QList<StyleMap> m_maps; // we can't really sort the maps between themselves...

    bool m_autoStyleInStylesDotXml;
    bool m_defaultStyle;
    short m_unused2;

    // For lookup
    friend class KoGenStyles;
};

#endif /* KOGENSTYLE_H */
