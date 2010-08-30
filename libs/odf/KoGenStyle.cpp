/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007 Thorsten Zachmann <zachmann@kde.org>
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
#include "KoGenStyle.h"
#include "KoGenStyles.h"

#include <KoXmlWriter.h>

#include <float.h>

#include <kdebug.h>

// Returns -1, 0 (equal) or 1
static int compareMap(const QMap<QString, QString>& map1, const QMap<QString, QString>& map2)
{
    QMap<QString, QString>::const_iterator it = map1.constBegin();
    QMap<QString, QString>::const_iterator oit = map2.constBegin();
    for (; it != map1.constEnd(); ++it, ++oit) {   // both maps have been checked for size already
        if (it.key() != oit.key())
            return it.key() < oit.key() ? -1 : + 1;
        if (it.value() != oit.value())
            return it.value() < oit.value() ? -1 : + 1;
    }
    return 0; // equal
}


KoGenStyle::KoGenStyle(Type type, const char* familyName,
                       const QString& parentName)
        : m_type(type), m_familyName(familyName), m_parentName(parentName),
        m_autoStyleInStylesDotXml(false), m_defaultStyle(false)
{
    switch (type) {
    case TextStyle:
    case TextAutoStyle:
        m_propertyType = TextType;
        break;
    case ParagraphStyle:
    case ParagraphAutoStyle:
        m_propertyType = ParagraphType;
        break;
    case GraphicStyle:
    case GraphicAutoStyle:
        m_propertyType = GraphicType;
        break;
    case SectionStyle:
    case SectionAutoStyle:
        m_propertyType = SectionType;
        break;
    case RubyStyle:
    case RubyAutoStyle:
        m_propertyType = RubyType;
        break;
    case TableStyle:
    case TableAutoStyle:
        m_propertyType = TableType;
        break;
    case TableColumnStyle:
    case TableColumnAutoStyle:
        m_propertyType = TableColumnType;
        break;
    case TableRowStyle:
    case TableRowAutoStyle:
        m_propertyType = TableRowType;
        break;
    case TableCellStyle:
    case TableCellAutoStyle:
        m_propertyType = TableCellType;
        break;
    case PresentationStyle:
    case PresentationAutoStyle:
        m_propertyType = PresentationType;
        break;
    case DrawingPageStyle:
    case DrawingPageAutoStyle:
        m_propertyType = DrawingPageType;
        break;
    case ChartStyle:
    case ChartAutoStyle:
        m_propertyType = ChartType;
        break;
    default:
        m_propertyType =  DefaultType;
        break;
    }
}

KoGenStyle::~KoGenStyle()
{
}

/*
 * The order of this list is important; e.g. a graphic-properties must
 * precede a text-properties always. See the Relax NG to check the order.
 */
static KoGenStyle::PropertyType s_propertyTypes[] = {
    KoGenStyle::DefaultType,
    KoGenStyle::SectionType,
    KoGenStyle::RubyType,
    KoGenStyle::TableType,
    KoGenStyle::TableColumnType,
    KoGenStyle::TableRowType,
    KoGenStyle::TableCellType,
    KoGenStyle::DrawingPageType,
    KoGenStyle::ChartType,
    KoGenStyle::GraphicType,
    KoGenStyle::ParagraphType,
    KoGenStyle::TextType,
};

static const char* s_propertyNames[] = {
    0,
    "style:section-properties",
    "style:ruby-properties",
    "style:table-properties",
    "style:table-column-properties",
    "style:table-row-properties",
    "style:table-cell-properties",
    "style:drawing-page-properties",
    "style:chart-properties",
    "style:graphic-properties",
    "style:paragraph-properties",
    "style:text-properties"
};

static const int s_propertyNamesCount = sizeof(s_propertyNames) / sizeof(*s_propertyNames);

static KoGenStyle::PropertyType propertyTypeByElementName(const char* propertiesElementName)
{
    for (int i = 0; i < s_propertyNamesCount; ++i) {
        if (qstrcmp(s_propertyNames[i], propertiesElementName) == 0) {
            return s_propertyTypes[i];
        }
    }
    return KoGenStyle::DefaultType;
}

void KoGenStyle::writeStyleProperties(KoXmlWriter* writer, PropertyType type,
                                      const KoGenStyle* parentStyle) const
{
    const char* elementName = 0;
    for (int i=0; i<s_propertyNamesCount; ++i) {
        if (s_propertyTypes[i] == type) {
            elementName = s_propertyNames[i];
        }
    }
    Q_ASSERT(elementName);
    const StyleMap& map = m_properties[type];
    if (!map.isEmpty()) {
        writer->startElement(elementName);
        QMap<QString, QString>::const_iterator it = map.constBegin();
        const QMap<QString, QString>::const_iterator end = map.constEnd();
        for (; it != end; ++it) {
            if (!parentStyle || parentStyle->property(it.key(), type) != it.value())
                writer->addAttribute(it.key().toUtf8(), it.value().toUtf8());
        }
        writer->endElement();
    }
}

void KoGenStyle::writeStyle(KoXmlWriter* writer, const KoGenStyles& styles, const char* elementName, const QString& name, const char* propertiesElementName, bool closeElement, bool drawElement) const
{
    //kDebug(30003) <<"writing out style" << name <<" display-name=" << m_attributes["style:display-name"] <<" family=" << m_familyName;
    writer->startElement(elementName);
    const KoGenStyle* parentStyle = 0;
    if (!m_defaultStyle) {
        if (!drawElement)
            writer->addAttribute("style:name", name);
        else
            writer->addAttribute("draw:name", name);
        if (!m_parentName.isEmpty()) {
            parentStyle = styles.style(m_parentName);
            if (parentStyle && m_familyName.isEmpty()) {
                // get family from parent style, just in case
                // Note: this is saving code, don't convert to attributeNS!
                const_cast<KoGenStyle *>(this)->
                m_familyName = parentStyle->attribute("style:family").toLatin1();
                //kDebug(30003) <<"Got familyname" << m_familyName <<" from parent";
            }
            if (parentStyle && !parentStyle->isDefaultStyle())
                writer->addAttribute("style:parent-style-name", m_parentName);
        }
    } else { // default-style
        Q_ASSERT(qstrcmp(elementName, "style:default-style") == 0);
        Q_ASSERT(m_parentName.isEmpty());
    }
    if (!m_familyName.isEmpty())
        const_cast<KoGenStyle *>(this)->
        addAttribute("style:family", QString::fromLatin1(m_familyName));
    else {
        if (qstrcmp(elementName, "style:style") == 0)
            kWarning(30003) << "User style " << name << " is without family - invalid. m_type=" << m_type;
    }

#if 0 // #ifndef NDEBUG
    kDebug(30003) << "style:" << name;
    printDebug();
    if (parentStyle) {
        kDebug(30003) << " parent:" << m_parentName;
        parentStyle->printDebug();
    }
#endif

    // Write attributes [which differ from the parent style]
    // We only look at the direct parent style because we assume
    // that styles are fully specified, i.e. the inheritance is
    // only in the final file, not in the caller's code.
    QMap<QString, QString>::const_iterator it = m_attributes.constBegin();
    for (; it != m_attributes.constEnd(); ++it) {
        bool writeit = true;
        if (parentStyle && it.key() != "style:family"  // always write the family out
                && parentStyle->attribute(it.key()) == it.value())
            writeit = false;
        if (writeit)
            writer->addAttribute(it.key().toUtf8(), it.value().toUtf8());
    }
    bool createPropertiesTag = propertiesElementName && propertiesElementName[0] != '\0';
    KoGenStyle::PropertyType i = KoGenStyle::DefaultType;
    KoGenStyle::PropertyType defaultPropertyType = KoGenStyle::DefaultType;
    if (createPropertiesTag)
        defaultPropertyType = propertyTypeByElementName(propertiesElementName);
    if (!m_properties[i].isEmpty() ||
            !m_properties[KoGenStyle::ChildElement].isEmpty() ||
            !m_properties[defaultPropertyType].isEmpty()) {
        if (createPropertiesTag)
            writer->startElement(propertiesElementName);   // e.g. paragraph-properties
        it = m_properties[i].constBegin();
        for (; it != m_properties[i].constEnd(); ++it) {
            if (!parentStyle || parentStyle->property(it.key(), i) != it.value())
                writer->addAttribute(it.key().toUtf8(), it.value().toUtf8());
        }
        //write the explicitly-defined properties that are the same type as the default,
        //but only if defaultPropertyType is Text, Paragraph, or GraphicType
        if (defaultPropertyType != 0) {
            it = m_properties[defaultPropertyType].constBegin();
            for (; it != m_properties[defaultPropertyType].constEnd(); ++it) {
                if (!parentStyle || parentStyle->property(it .key(), defaultPropertyType) != it.value())
                    writer->addAttribute(it.key().toUtf8(), it.value().toUtf8());
            }
        }
        //write child elements of the properties elements
        i = KoGenStyle::ChildElement;
        it = m_properties[i].constBegin();
        for (; it != m_properties[i].constEnd(); ++it) {
            if (!parentStyle || parentStyle->property(it.key(), i) != it.value()) {
                writer->addCompleteElement(it.value().toUtf8());
            }
        }
        if (createPropertiesTag)
            writer->endElement();
    }

    // now write out any other properties elements
    //start with i=1 to skip the defaultType that we already took care of
    for (int i = 1; i < s_propertyNamesCount; ++i) {
        //skip any properties that are the same as the defaultType
        if (s_propertyTypes[i] != defaultPropertyType) {
            writeStyleProperties(writer, s_propertyTypes[i], parentStyle);
        }
    }

    //write child elements that aren't in any of the properties elements
    i = KoGenStyle::StyleChildElement;
    it = m_properties[i].constBegin();
    for (; it != m_properties[i].constEnd(); ++it) {
        if (!parentStyle || parentStyle->property(it.key(), i) != it.value()) {
            writer->addCompleteElement(it.value().toUtf8());
        }
    }

    // And now the style maps
    for (int i = 0; i < m_maps.count(); ++i) {
        bool writeit = true;
        if (parentStyle && compareMap(m_maps[i], parentStyle->m_maps[i]) == 0)
            writeit = false;
        if (writeit) {
            writer->startElement("style:map");
            QMap<QString, QString>::const_iterator it = m_maps[i].constBegin();
            for (; it != m_maps[i].constEnd(); ++it) {
                writer->addAttribute(it.key().toUtf8(), it.value().toUtf8());
            }
            writer->endElement(); // style:map
        }
    }
    if (closeElement)
        writer->endElement();
}

void KoGenStyle::addPropertyPt(const QString& propName, qreal propValue, PropertyType type)
{
    if (type == DefaultType) {
        type = m_propertyType;
    }
    QString str;
    str.setNum(propValue, 'f', DBL_DIG);
    str += "pt";
    m_properties[type].insert(propName, str);
}

void KoGenStyle::addAttributePt(const QString& attrName, qreal attrValue)
{
    QString str;
    str.setNum(attrValue, 'f', DBL_DIG);
    str += "pt";
    m_attributes.insert(attrName, str);
}

void KoGenStyle::addStyleMap(const QMap<QString, QString>& styleMap)
{
    // check, if already present
    for (int i = 0 ; i < m_maps.count() ; ++i) {
        if (m_maps[i].count() == styleMap.count()) {
            int comp = compareMap(m_maps[i], styleMap);
            if (comp == 0)
                return;
        }
    }
    m_maps.append(styleMap);
}


#ifndef NDEBUG
void KoGenStyle::printDebug() const
{
    int i = DefaultType;
    kDebug(30003) << m_properties[i].count() << " properties.";
    for (QMap<QString, QString>::ConstIterator it = m_properties[i].constBegin(); it != m_properties[i].constEnd(); ++it) {
        kDebug(30003) << "" << it.key() << " =" << it.value();
    }
    i = TextType;
    kDebug(30003) << m_properties[i].count() << " text properties.";
    for (QMap<QString, QString>::ConstIterator it = m_properties[i].constBegin(); it != m_properties[i].constEnd(); ++it) {
        kDebug(30003) << "" << it.key() << " =" << it.value();
    }
    i = ParagraphType;
    kDebug(30003) << m_properties[i].count() << " paragraph properties.";
    for (QMap<QString, QString>::ConstIterator it = m_properties[i].constBegin(); it != m_properties[i].constEnd(); ++it) {
        kDebug(30003) << "" << it.key() << " =" << it.value();
    }
    i = ChildElement;
    kDebug(30003) << m_properties[i].count() << " child elements.";
    for (QMap<QString, QString>::ConstIterator it = m_properties[i].constBegin(); it != m_properties[i].constEnd(); ++it) {
        kDebug(30003) << "" << it.key() << " =" << it.value();
    }
    kDebug(30003) << m_attributes.count() << " attributes.";
    for (QMap<QString, QString>::ConstIterator it = m_attributes.constBegin(); it != m_attributes.constEnd(); ++it) {
        kDebug(30003) << "" << it.key() << " =" << it.value();
    }
    kDebug(30003) << m_maps.count() << " maps.";
    for (int i = 0; i < m_maps.count(); ++i) {
        kDebug(30003) << "map" << i << ":";
        for (QMap<QString, QString>::ConstIterator it = m_maps[i].constBegin(); it != m_maps[i].constEnd(); ++it) {
            kDebug(30003) << "" << it.key() << " =" << it.value();
        }
    }
    kDebug(30003);
}
#endif

bool KoGenStyle::operator<(const KoGenStyle &other) const
{
    if (m_type != other.m_type) return m_type < other.m_type;
    if (m_parentName != other.m_parentName) return m_parentName < other.m_parentName;
    if (m_familyName != other.m_familyName) return m_familyName < other.m_familyName;
    if (m_autoStyleInStylesDotXml != other.m_autoStyleInStylesDotXml) return m_autoStyleInStylesDotXml;
    for (uint i = 0 ; i <= LastPropertyType; ++i)
        if (m_properties[i].count() != other.m_properties[i].count())
            return m_properties[i].count() < other.m_properties[i].count();
    if (m_attributes.count() != other.m_attributes.count()) return m_attributes.count() < other.m_attributes.count();
    if (m_maps.count() != other.m_maps.count()) return m_maps.count() < other.m_maps.count();
    // Same number of properties and attributes, no other choice than iterating
    for (uint i = 0 ; i <= LastPropertyType; ++i) {
        int comp = compareMap(m_properties[i], other.m_properties[i]);
        if (comp != 0)
            return comp < 0;
    }
    int comp = compareMap(m_attributes, other.m_attributes);
    if (comp != 0)
        return comp < 0;
    for (int i = 0 ; i < m_maps.count() ; ++i) {
        int comp = compareMap(m_maps[i], other.m_maps[i]);
        if (comp != 0)
            return comp < 0;
    }
    return false;
}

bool KoGenStyle::operator==(const KoGenStyle &other) const
{
    if (m_type != other.m_type) return false;
    if (m_parentName != other.m_parentName) return false;
    if (m_familyName != other.m_familyName) return false;
    if (m_autoStyleInStylesDotXml != other.m_autoStyleInStylesDotXml) return false;
    for (uint i = 0 ; i <= LastPropertyType; ++i)
        if (m_properties[i].count() != other.m_properties[i].count())
            return false;
    if (m_attributes.count() != other.m_attributes.count()) return false;
    if (m_maps.count() != other.m_maps.count()) return false;
    // Same number of properties and attributes, no other choice than iterating
    for (uint i = 0 ; i <= LastPropertyType; ++i) {
        int comp = compareMap(m_properties[i], other.m_properties[i]);
        if (comp != 0)
            return false;
    }
    int comp = compareMap(m_attributes, other.m_attributes);
    if (comp != 0)
        return false;
    for (int i = 0 ; i < m_maps.count() ; ++i) {
        int comp = compareMap(m_maps[i], other.m_maps[i]);
        if (comp != 0)
            return false;
    }
    return true;
}

bool KoGenStyle::isEmpty() const
{
    if (!m_attributes.isEmpty() || ! m_maps.isEmpty())
        return false;
    for (uint i = 0 ; i <= LastPropertyType; ++i)
        if (! m_properties[i].isEmpty())
            return false;
    return true;
}
