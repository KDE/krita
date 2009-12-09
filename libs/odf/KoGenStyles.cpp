/* This file is part of the KDE project
   Copyright (C) 2004-2006 David Faure <faure@kde.org>
   Copyright (C) 2007-2008 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Thomas Zander <zander@kde.org>
   Copyright (C) 2008 Girish Ramakrishnan <girish@forwardbias.in>
   Copyright (C) 2009 Inge Wallin <inge@lysator.liu.se>

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

#include "KoGenStyles.h"

#include <KoStore.h>
#include <KoStoreDevice.h>
#include <KoXmlWriter.h>
#include "KoOdfWriteStore.h"
#include "KoFontFace.h"
#include <float.h>
#include <kdebug.h>

static const struct {
    KoGenStyle::Type m_type;
    const char * m_elementName;
    const char * m_propertiesElementName;
    bool m_drawElement;
} styleData[] = {
    { KoGenStyle::StyleUser, "style:style", "style:paragraph-properties", false  },
    { KoGenStyle::StyleText, "style:style", "style:text-properties", false  },
    { KoGenStyle::StyleChart, "style:style", "style:chart-properties", false  },
    { KoGenStyle::StyleTableColumn, "style:style", "style:table-column-properties", false  },
    { KoGenStyle::StyleTableRow, "style:style", "style:table-row-properties", false  },
    { KoGenStyle::StyleTableCell, "style:style", "style:table-cell-properties", false  },
    { KoGenStyle::StyleList, "text:list-style", 0, false  },
    { KoGenStyle::StyleGradientLinear, "svg:linearGradient", 0, true  },
    { KoGenStyle::StyleGradientRadial, "svg:radialGradient", 0, true  },
    { KoGenStyle::StyleGradientConical, "koffice:conicalGradient", 0, true  },
    { KoGenStyle::StyleStrokeDash, "draw:stroke-dash", 0, true  },
    { KoGenStyle::StyleFillImage, "draw:fill-image", 0, true  },
    { KoGenStyle::StyleHatch, "draw:hatch", "style:graphic-properties", true  },
    { KoGenStyle::StyleGradient, "draw:gradient", "style:graphic-properties", true  },
    { KoGenStyle::StyleMarker, "draw:marker", "style:graphic-properties", true  },
    { KoGenStyle::StylePresentationPageLayout, "style:presentation-page-layout", 0, false  }
};

static const unsigned int numStyleData = sizeof(styleData) / sizeof(*styleData);

static const struct {
    KoGenStyle::Type m_type;
    const char * m_elementName;
    const char * m_propertiesElementName;
    bool m_drawElement;
} autoStyleData[] = {
    { KoGenStyle::StyleAuto, "style:style", "style:paragraph-properties", false  },
    { KoGenStyle::StyleTextAuto, "style:style", "style:text-properties", false  },
    { KoGenStyle::StyleGraphicAuto, "style:style", "style:graphic-properties", false  },
    { KoGenStyle::StyleChartAuto, "style:style", "style:chart-properties", false  },
    { KoGenStyle::StylePresentationAuto, "style:style", "style:graphic-properties", false  },
    { KoGenStyle::StyleDrawingPage, "style:style", "style:drawing-page-properties", false  },
    { KoGenStyle::StyleAutoTable, "style:style", "style:table-properties", false  },
    { KoGenStyle::StyleAutoTableColumn, "style:style", "style:table-column-properties", false  },
    { KoGenStyle::StyleAutoTableRow, "style:style", "style:table-row-properties", false  },
    { KoGenStyle::StyleAutoTableCell, "style:style", "style:table-cell-properties", false  },
    { KoGenStyle::StyleSectionAuto, "style:style", "style:section-properties", false  },
    { KoGenStyle::StylePageLayout, "style:page-layout", "style:page-layout-properties", false  },
    { KoGenStyle::StyleListAuto, "text:list-style", 0, false  },
    { KoGenStyle::StyleNumericNumber, "number:number-style", 0, false  },
    { KoGenStyle::StyleNumericFraction, "number:number-style", 0, false  },
    { KoGenStyle::StyleNumericScientific, "number:number-style", 0, false  },
    { KoGenStyle::StyleNumericDate, "number:date-style", 0, false  },
    { KoGenStyle::StyleNumericTime, "number:time-style", 0, false  },
    { KoGenStyle::StyleNumericPercentage, "number:percentage-style", 0, false  },
    { KoGenStyle::StyleNumericCurrency, "number:currency-style", 0, false  },
    { KoGenStyle::StyleNumericBoolean, "number:boolean-style", 0, false  },
    { KoGenStyle::StyleNumericText, "number:text-style", 0, false  }
};

static const unsigned int numAutoStyleData = sizeof(autoStyleData) / sizeof(*autoStyleData);

static void addRawOdfStyles(const QByteArray& xml, QByteArray& styles)
{
    if (xml.isEmpty())
        return;
    if (!styles.isEmpty() && !styles.endsWith('\n') && !xml.startsWith('\n')) {
        styles.append('\n');
    }
    styles.append(xml);
}

class KoGenStyles::Private
{
public:
    Private(KoGenStyles *q) : q(q)
    {
    }

    ~Private()
    {
    }

    /// style definition -> name
    StyleMap styleMap;

    /// Map with the style name as key.
    /// This map is mainly used to check for name uniqueness
    NameMap styleNames;
    NameMap autoStylesInStylesDotXml;

    /// List of styles (used to preserve ordering)
    StyleArray styleArray;

    /// map for saving default styles
    QMap<int, KoGenStyle> defaultStyles;

    /// font faces
    QMap<QString, KoFontFace> fontFaces;

    StyleMap::iterator insertStyle(const KoGenStyle &style, const QString &name, int flags);

    struct RelationTarget {
        QString target; // the style we point to
        QString attribute; // the attribute name used for the relation
    };
    QHash<QString, RelationTarget> relations; // key is the name of the source style

    QByteArray& rawOdfAutomaticStyles(bool stylesDotXml) {
        return stylesDotXml ? rawOdfAutomaticStyles_stylesDotXml : rawOdfAutomaticStyles_contentDotXml;
    }

    QByteArray rawOdfDocumentStyles;
    QByteArray rawOdfAutomaticStyles_stylesDotXml;
    QByteArray rawOdfAutomaticStyles_contentDotXml;
    QByteArray rawOdfMasterStyles;

    KoGenStyles *q;
};

KoGenStyles::KoGenStyles()
        : d(new Private(this))
{
}

KoGenStyles::~KoGenStyles()
{
    delete d;
}

QString KoGenStyles::lookup(const KoGenStyle& style, const QString& name, int flags)
{
    // if it is a default style it has to be saved differently
    if (style.isDefaultStyle()) {
        // we can have only one default style per type
        Q_ASSERT(!d->defaultStyles.contains(style.type()));
        // default style is only possible for style:style in office:style types
        Q_ASSERT(style.type() == KoGenStyle::StyleUser ||
                 style.type() == KoGenStyle::StyleTableColumn ||
                 style.type() == KoGenStyle::StyleTableRow ||
                 style.type() == KoGenStyle::StyleTableCell);
        d->defaultStyles.insert(style.type(), style);
        // default styles don't have a name
        return QString();
    }

    if (flags & AllowDuplicates) {
        StyleMap::iterator it = d->insertStyle(style, name, flags);
        return it.value();
    }

    StyleMap::iterator it = d->styleMap.find(style);
    if (it == d->styleMap.end()) {
        // Not found, try if this style is in fact equal to its parent (the find above
        // wouldn't have found it, due to m_parentName being set).
        if (!style.parentName().isEmpty()) {
            KoGenStyle testStyle(style);
            const KoGenStyle* parentStyle = this->style(style.parentName());   // ## linear search
            if (!parentStyle) {
                kDebug(30003) << "KoGenStyles::lookup(" << name << "): parent style '" << style.parentName() << "' not found in collection";
            } else {
                if (testStyle.m_familyName != parentStyle->m_familyName) {
                    kWarning(30003) << "KoGenStyles::lookup(" << name << ", family=" << testStyle.m_familyName << ") parent style '" << style.parentName() << "' has a different family: " << parentStyle->m_familyName;
                }

                testStyle.m_parentName = parentStyle->m_parentName;
                // Exclude the type from the comparison. It's ok for an auto style
                // to have a user style as parent; they can still be identical
                testStyle.m_type = parentStyle->m_type;
                // Also it's ok to not have the display name of the parent style
                // in the auto style
                QMap<QString, QString>::const_iterator it = parentStyle->m_attributes.find("style:display-name");
                if (it != parentStyle->m_attributes.end())
                    testStyle.addAttribute("style:display-name", *it);

                if (*parentStyle == testStyle)
                    return style.parentName();
            }
        }

        it = d->insertStyle(style, name, flags);
    }
    return it.value();
}

KoGenStyles::StyleMap::iterator KoGenStyles::Private::insertStyle(const KoGenStyle &style, const QString &name, int flags)
{
    QString styleName(name);
    if (styleName.isEmpty()) {
        switch (style.type()) {
        case KoGenStyle::StyleAuto: styleName = 'P'; break;
        case KoGenStyle::StyleListAuto: styleName = 'L'; break;
        case KoGenStyle::StyleTextAuto: styleName = 'T'; break;
        default:
            styleName = 'A'; // for "auto".
        }
        flags &= ~DontForceNumbering; // i.e. force numbering
    }
    styleName = q->makeUniqueName(styleName, flags);
    if (style.autoStyleInStylesDotXml())
        autoStylesInStylesDotXml.insert(styleName);
    else
        styleNames.insert(styleName);
    KoGenStyles::StyleMap::iterator it = styleMap.insert(style, styleName);
    NamedStyle s;
    s.style = &it.key();
    s.name = styleName;
    styleArray.append(s);
    return it;
}

KoGenStyles::StyleMap KoGenStyles::styles() const
{
    return d->styleMap;
}

QString KoGenStyles::makeUniqueName(const QString& base, int flags) const
{
    // If this name is not used yet, and numbering isn't forced, then the given name is ok.
    if ((flags & DontForceNumbering)
            && ! d->autoStylesInStylesDotXml.contains(base)
            && ! d->styleNames.contains(base))
        return base;
    int num = 1;
    QString name;
    do {
        name = base;
        name += QString::number(num++);
    } while (d->autoStylesInStylesDotXml.contains(name)
             || d->styleNames.contains(name));
    return name;
}

QList<KoGenStyles::NamedStyle> KoGenStyles::styles(int type, bool markedForStylesXml) const
{
    QList<KoGenStyles::NamedStyle> lst;
    const NameMap& nameMap = markedForStylesXml ? d->autoStylesInStylesDotXml : d->styleNames;
    StyleArray::const_iterator it = d->styleArray.constBegin();
    const StyleArray::const_iterator end = d->styleArray.constEnd();
    for (; it != end ; ++it) {
        // Look up if it's marked for styles.xml or not by looking up in the corresponding style map.
        if ((*it).style->type() == type && nameMap.find((*it).name) != nameMap.end()) {
            lst.append(*it);
        }
    }
    return lst;
}

const KoGenStyle* KoGenStyles::style(const QString& name) const
{
    StyleArray::const_iterator it = d->styleArray.constBegin();
    const StyleArray::const_iterator end = d->styleArray.constEnd();
    for (; it != end ; ++it) {
        if ((*it).name == name)
            return (*it).style;
    }
    return 0;
}

KoGenStyle* KoGenStyles::styleForModification(const QString& name)
{
    return const_cast<KoGenStyle *>(style(name));
}

void KoGenStyles::markStyleForStylesXml(const QString& name)
{
    Q_ASSERT(d->styleNames.contains(name));
    d->styleNames.remove(name);
    d->autoStylesInStylesDotXml.insert(name);
    styleForModification(name)->setAutoStyleInStylesDotXml(true);
}

void KoGenStyles::dump()
{
    kDebug(30003) << "Style array:";
    StyleArray::const_iterator it = d->styleArray.constBegin();
    const StyleArray::const_iterator end = d->styleArray.constEnd();
    for (; it != end ; ++it) {
        kDebug(30003) << (*it).name;
    }
    for (NameMap::const_iterator it = d->styleNames.constBegin(); it != d->styleNames.constEnd(); ++it) {
        kDebug(30003) << "style:" << *it;
    }
    for (NameMap::const_iterator it = d->autoStylesInStylesDotXml.constBegin(); it != d->autoStylesInStylesDotXml.constEnd(); ++it) {
#ifndef NDEBUG
        kDebug(30003) << "auto style for style.xml:" << *it;
        const KoGenStyle* s = style(*it);
        Q_ASSERT(s);
        Q_ASSERT(s->autoStyleInStylesDotXml());
#endif
    }
}

void KoGenStyles::addFontFace(const KoFontFace &face)
{
    Q_ASSERT(!face.isNull());
    if (face.isNull()) {
        kWarning() << "This font face is null and will not be added to styles: set at least the name";
        return;
    }
    d->fontFaces.insert(face.name(), face); // replaces prev item
}

KoFontFace KoGenStyles::fontFace(const QString& name) const
{
    return d->fontFaces.value(name);
}

bool KoGenStyles::saveOdfStylesDotXml(KoStore* store, KoXmlWriter* manifestWriter)
{
    if (!store->open("styles.xml"))
        return false;

    manifestWriter->addManifestEntry("styles.xml",  "text/xml");

    KoStoreDevice stylesDev(store);
    KoXmlWriter* stylesWriter = KoOdfWriteStore::createOasisXmlWriter(&stylesDev, "office:document-styles");

    saveOdfFontFaceDecls(stylesWriter);
    saveOdfDocumentStyles(stylesWriter);
    saveOdfAutomaticStyles(stylesWriter, true);
    saveOdfMasterStyles(stylesWriter);

    stylesWriter->endElement(); // root element (office:document-styles)
    stylesWriter->endDocument();
    delete stylesWriter;

    if (!store->close())   // done with styles.xml
        return false;

    return true;
}

void KoGenStyles::saveOdfAutomaticStyles(KoXmlWriter* xmlWriter, bool stylesDotXml) const
{
    xmlWriter->startElement("office:automatic-styles");

    for (uint i = 0; i < numAutoStyleData; ++i) {
        QList<KoGenStyles::NamedStyle> stylesList = styles(int(autoStyleData[i].m_type), stylesDotXml);
        QList<KoGenStyles::NamedStyle>::const_iterator it = stylesList.constBegin();
        for (; it != stylesList.constEnd() ; ++it) {
            (*it).style->writeStyle(xmlWriter, *this, autoStyleData[i].m_elementName, (*it).name,
                                    autoStyleData[i].m_propertiesElementName, true, autoStyleData[i].m_drawElement);
        }
    }

    const QByteArray &rawOdfAutomaticStyles = d->rawOdfAutomaticStyles(stylesDotXml);
    if (!rawOdfAutomaticStyles.isEmpty()) {
        xmlWriter->addCompleteElement(rawOdfAutomaticStyles.constData());
    }

    xmlWriter->endElement(); // office:automatic-styles
}


void KoGenStyles::saveOdfDocumentStyles(KoXmlWriter* xmlWriter) const
{
    xmlWriter->startElement("office:styles");

    for (uint i = 0; i < numStyleData; ++i) {
        QMap<int, KoGenStyle>::iterator it(d->defaultStyles.find(styleData[i].m_type));
        if (it != d->defaultStyles.end()) {
            it.value().writeStyle(xmlWriter, *this, "style:default-style", "",
                                  styleData[i].m_propertiesElementName, true, styleData[i].m_drawElement);
        }
    }

    for (uint i = 0; i < numStyleData; ++i) {
        QList<KoGenStyles::NamedStyle> stylesList = styles(int(styleData[i].m_type));
        QList<KoGenStyles::NamedStyle>::const_iterator it = stylesList.constBegin();
        for (; it != stylesList.constEnd() ; ++it) {
            if (d->relations.contains(it->name)) {
                KoGenStyles::Private::RelationTarget relation = d->relations.value(it->name);
                KoGenStyle styleCopy = *(*it).style;
                styleCopy.addAttribute(relation.attribute, relation.target);
                styleCopy.writeStyle(xmlWriter, *this, styleData[i].m_elementName, (*it).name,
                                    styleData[i].m_propertiesElementName, true, styleData[i].m_drawElement);
            } else {
                (*it).style->writeStyle(xmlWriter, *this, styleData[i].m_elementName, (*it).name,
                                    styleData[i].m_propertiesElementName, true, styleData[i].m_drawElement);
            }
        }
    }

    if (!d->rawOdfDocumentStyles.isEmpty()) {
        xmlWriter->addCompleteElement(d->rawOdfDocumentStyles.constData());
    }

    xmlWriter->endElement(); // office:styles
}

void KoGenStyles::saveOdfMasterStyles(KoXmlWriter* xmlWriter) const
{
    xmlWriter->startElement("office:master-styles");

    QList<KoGenStyles::NamedStyle> stylesList = styles(KoGenStyle::StyleMaster);
    QList<KoGenStyles::NamedStyle>::const_iterator it = stylesList.constBegin();
    for (; it != stylesList.constEnd() ; ++it) {
        (*it).style->writeStyle(xmlWriter, *this, "style:master-page", (*it).name, 0);
    }

    if (!d->rawOdfMasterStyles.isEmpty()) {
        xmlWriter->addCompleteElement(d->rawOdfMasterStyles.constData());
    }

    xmlWriter->endElement(); // office:master-styles
}

void KoGenStyles::addRawOdfDocumentStyles(const QByteArray& xml)
{
    addRawOdfStyles(xml, d->rawOdfDocumentStyles);
}

void KoGenStyles::addRawOdfAutomaticStyles(const QByteArray& xml, bool stylesDotXml)
{
    addRawOdfStyles(xml, d->rawOdfAutomaticStyles(stylesDotXml));
}

void KoGenStyles::addRawOdfMasterStyles(const QByteArray& xml)
{
    addRawOdfStyles(xml, d->rawOdfMasterStyles);
}

void KoGenStyles::saveOdfFontFaceDecls(KoXmlWriter* xmlWriter) const
{
    if (d->fontFaces.isEmpty())
        return;

    xmlWriter->startElement("office:font-face-decls");
    for (QMap<QString, KoFontFace>::ConstIterator it(d->fontFaces.constBegin());
         it != d->fontFaces.constEnd(); ++it)
    {
        it.value().saveOdf(xmlWriter);
    }

    xmlWriter->endElement(); // office:font-face-decls
}

void KoGenStyles::insertStyleRelation(const QString &source, const QString &target, const char *tagName)
{
    KoGenStyles::Private::RelationTarget relation;
    relation.target = target;
    relation.attribute = QString(tagName);
    d->relations.insert(source, relation);
}
