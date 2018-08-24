/* This file is part of the KDE project
   Copyright 2012 Friedrich W. H. Kossebau <kossebau@kde.org>

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

#include "KoColumns.h"

#include <QBuffer>

#include "KoXmlWriter.h"
#include "KoGenStyle.h"
#include "KoXmlReader.h"
#include "KoXmlNS.h"
#include "KoUnit.h"
// KDE
#include <OdfDebug.h>

static const int defaultColumnCount = 1;
static const KoColumns::SeparatorStyle defaultSeparatorStyle = KoColumns::None;
static const int defaultSeparatorHeight = 100;
static const  Qt::GlobalColor defaultSeparatorColor = Qt::black;
static const KoColumns::SeparatorVerticalAlignment defaultSeparatorVerticalAlignment = KoColumns::AlignTop;
static const int defaultColumnGapWidth = 17; // in pt, ~ 6mm



const char * KoColumns::separatorStyleString(KoColumns::SeparatorStyle separatorStyle)
{
    const char * result;

    //  skip KoColumns::None, is default
    if (separatorStyle == Solid) {
        result = "solid";
    } else if (separatorStyle == Dotted) {
        result = "dotted";
    } else if (separatorStyle == Dashed) {
        result = "dashed";
    } else if (separatorStyle == DotDashed) {
        result = "dot-dashed";
    } else {
        result = "none";
    }

    return result;
}

const char * KoColumns::separatorVerticalAlignmentString(KoColumns::SeparatorVerticalAlignment separatorVerticalAlignment)
{
    const char * result;

    //  skip KoColumns::AlignTop, is default
    if (separatorVerticalAlignment == AlignVCenter) {
        result = "middle";
    } else if (separatorVerticalAlignment == AlignBottom) {
        result = "bottom";
    } else {
        result = "top";
    }

    return result;
}

KoColumns::SeparatorVerticalAlignment KoColumns::parseSeparatorVerticalAlignment(const QString &value)
{
    // default to AlignTop
    SeparatorVerticalAlignment result = defaultSeparatorVerticalAlignment;

    if (! value.isEmpty()) {
        // skip "top", is default
        if (value == QLatin1String("middle")) {
            result = AlignVCenter;
        } else if (value == QLatin1String("bottom")) {
            result = AlignBottom;
        }
    }

    return result;
}

QColor KoColumns::parseSeparatorColor(const QString &value)
{
    QColor result(value);

    if (! result.isValid())
        // default is black, cmp. ODF 1.2 §19.467
        result = QColor(defaultSeparatorColor);

    return result;
}

// TODO: see if there is another parse method somewhere which can be used here
int KoColumns::parseSeparatorHeight(const QString &value)
{
    int result = defaultSeparatorHeight;

    // only try to convert if it ends with a %, so is also not empty
    if (value.endsWith(QLatin1Char('%'))) {
        bool ok = false;
        // try to convert
        result = value.left(value.length()-1).toInt(&ok);
        // reset to 100% if conversion failed (which sets result to 0)
        if (! ok) {
            result = defaultSeparatorHeight;
        }
    }

    return result;
}

KoColumns::SeparatorStyle KoColumns::parseSeparatorStyle(const QString &value)
{
    SeparatorStyle result = None;
    if (! value.isEmpty()) {
        //  skip "none", is default
        if (value == QLatin1String("solid")) {
            result = Solid;
        } else if (value == QLatin1String("dotted")) {
            result = Dotted;
        } else if (value == QLatin1String("dashed")) {
            result = Dashed;
        } else if (value == QLatin1String("dot-dashed")) {
            result = DotDashed;
        }
    }

    return result;
}

int KoColumns::parseRelativeWidth(const QString &value)
{
    int result = 0;

    // only try to convert if it ends with a *, so is also not empty
    if (value.endsWith(QLatin1Char('*'))) {
        bool ok = false;
        // try to convert
        result = value.left(value.length()-1).toInt(&ok);
        if (! ok) {
            result = 0;
        }
    }

    return result;
}

KoColumns::KoColumns()
  : count(defaultColumnCount)
  , gapWidth(defaultColumnGapWidth)
  , separatorStyle(defaultSeparatorStyle)
  , separatorColor(defaultSeparatorColor)
  , separatorVerticalAlignment(defaultSeparatorVerticalAlignment)
  , separatorHeight(defaultSeparatorHeight)
{
}

void KoColumns::reset()
{
    count = defaultColumnCount;
    gapWidth = defaultColumnGapWidth;
    separatorStyle = defaultSeparatorStyle;
    separatorColor = QColor(defaultSeparatorColor);
    separatorVerticalAlignment = defaultSeparatorVerticalAlignment;
    separatorHeight = defaultSeparatorHeight;
}

bool KoColumns::operator==(const KoColumns& rhs) const {
    return
        count == rhs.count &&
        (columnData.isEmpty() && rhs.columnData.isEmpty() ?
             (qAbs(gapWidth - rhs.gapWidth) <= 1E-10) :
             (columnData == rhs.columnData));
}

bool KoColumns::operator!=(const KoColumns& rhs) const {
    return
        count != rhs.count ||
        (columnData.isEmpty() && rhs.columnData.isEmpty() ?
             qAbs(gapWidth - rhs.gapWidth) > 1E-10 :
             ! (columnData == rhs.columnData));
}

void KoColumns::loadOdf(const KoXmlElement &style)
{
    KoXmlElement columnsElement = KoXml::namedItemNS(style, KoXmlNS::style, "columns");
    if (!columnsElement.isNull()) {
        count = columnsElement.attributeNS(KoXmlNS::fo, "column-count").toInt();
        if (count < 1)
            count = 1;
        gapWidth = KoUnit::parseValue(columnsElement.attributeNS(KoXmlNS::fo, "column-gap"));

        KoXmlElement columnSep = KoXml::namedItemNS(columnsElement, KoXmlNS::style, "column-sep");
        if (! columnSep.isNull()) {
            separatorStyle = parseSeparatorStyle(columnSep.attributeNS(KoXmlNS::style, "style"));
            separatorWidth = KoUnit::parseValue(columnSep.attributeNS(KoXmlNS::style, "width"));
            separatorHeight = parseSeparatorHeight(columnSep.attributeNS(KoXmlNS::style, "height"));
            separatorColor = parseSeparatorColor(columnSep.attributeNS(KoXmlNS::style, "color"));
            separatorVerticalAlignment =
                parseSeparatorVerticalAlignment(columnSep.attributeNS(KoXmlNS::style, "vertical-align"));
        }

        KoXmlElement columnElement;
        forEachElement(columnElement, columnsElement) {
            if(columnElement.localName() != QLatin1String("column") ||
               columnElement.namespaceURI() != KoXmlNS::style)
                continue;

            ColumnDatum datum;
            datum.leftMargin = KoUnit::parseValue(columnElement.attributeNS(KoXmlNS::fo, "start-indent"), 0.0);
            datum.rightMargin = KoUnit::parseValue(columnElement.attributeNS(KoXmlNS::fo, "end-indent"), 0.0);
            datum.topMargin = KoUnit::parseValue(columnElement.attributeNS(KoXmlNS::fo, "space-before"), 0.0);
            datum.bottomMargin = KoUnit::parseValue(columnElement.attributeNS(KoXmlNS::fo, "space-after"), 0.0);
            datum.relativeWidth = parseRelativeWidth(columnElement.attributeNS(KoXmlNS::style, "rel-width"));
            // on a bad relativeWidth just drop all data
            if (datum.relativeWidth <= 0) {
                columnData.clear();
                break;
            }

            columnData.append(datum);
        }

        if (! columnData.isEmpty() && count != columnData.count()) {
            warnOdf << "Found not as many <style:column> elements as attribute fo:column-count has set:"<< count;
            columnData.clear();
        }
    } else {
        reset();
    }
}

void KoColumns::saveOdf(KoGenStyle &style) const
{
    if (count > 1) {
        QBuffer buffer;
        buffer.open(QIODevice::WriteOnly);
        KoXmlWriter writer(&buffer);

        writer.startElement("style:columns");
        writer.addAttribute("fo:column-count", count);
        if (columnData.isEmpty()) {
            writer.addAttribute("fo:column-gap", gapWidth);
        }

        if (separatorStyle != KoColumns::None) {
            writer.startElement("style:column-sep");
            writer.addAttribute("style:style", separatorStyleString(separatorStyle));
            writer.addAttribute("style:width", separatorWidth);
            writer.addAttribute("style:height", QString::number(separatorHeight)+QLatin1Char('%'));
            writer.addAttribute("style:color", separatorColor.name());
            writer.addAttribute("style:vertical-align", separatorVerticalAlignmentString(separatorVerticalAlignment));
            writer.endElement(); // style:column-sep
        }

        Q_FOREACH (const ColumnDatum &cd, columnData) {
            writer.startElement("style:column");
            writer.addAttribute("fo:start-indent", cd.leftMargin);
            writer.addAttribute("fo:end-indent", cd.rightMargin);
            writer.addAttribute("fo:space-before", cd.topMargin);
            writer.addAttribute("fo:space-after", cd.bottomMargin);
            writer.addAttribute("style:rel-width", QString::number(cd.relativeWidth)+QLatin1Char('*'));
            writer.endElement(); // style:column
        }

        writer.endElement(); // style:columns

        QString contentElement = QString::fromUtf8(buffer.buffer(), buffer.buffer().size());
        style.addChildElement("style:columns", contentElement);
    }
}
