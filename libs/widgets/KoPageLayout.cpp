/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright 2002, 2003 David Faure <faure@kde.org>
   Copyright 2003 Nicolas GOUTTE <goutte@kde.org>
   Copyright 2007 Thomas Zander <zander@kde.org>
   Copyright 2009 Inge Wallin <inge@lysator.liu.se>

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

#include "KoPageLayout.h"

#include <KDebug>

#include "KoXmlNS.h"
#include "KoUnit.h"

KoGenStyle KoPageLayout::saveOdf() const
{
    KoGenStyle style(KoGenStyle::StylePageLayout);

    // Save page dimension.
    style.addPropertyPt("fo:page-width", width);
    style.addPropertyPt("fo:page-height", height);

    // Save margins. If all margins are the same, only one value needs to be saved.
    if (leftMargin == topMargin && leftMargin == rightMargin && leftMargin == bottomMargin) {
        style.addPropertyPt("fo:margin", leftMargin);
    }
    else {
        style.addPropertyPt("fo:margin-left", leftMargin);
        style.addPropertyPt("fo:margin-right", rightMargin);
        style.addPropertyPt("fo:margin-top", topMargin);
        style.addPropertyPt("fo:margin-bottom", bottomMargin);
    }

    // Save padding. If all paddings are the same, only one value needs to be saved.
    if (leftPadding == topPadding && leftPadding == rightPadding && leftPadding == bottomPadding) {
        style.addPropertyPt("fo:padding", leftPadding);
    }
    else {
        style.addPropertyPt("fo:padding-left", leftPadding);
        style.addPropertyPt("fo:padding-right", rightPadding);
        style.addPropertyPt("fo:padding-top", topPadding);
        style.addPropertyPt("fo:padding-bottom", bottomPadding);
    }

    // If there are any page borders, add them to the style.
    border.saveOdf(style);

    style.addProperty("style:print-orientation",
                      (orientation == KoPageFormat::Landscape
                       ? "landscape" : "portrait"));
    return style;
}

void KoPageLayout::loadOdf(const KoXmlElement &style)
{
    KoXmlElement  properties(KoXml::namedItemNS(style, KoXmlNS::style,
                                                "page-layout-properties"));

    if (!properties.isNull()) {
        KoPageLayout standard = standardLayout();

        // Page dimension -- width / height
        width = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "page-width"),
                                   standard.width);
        height = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "page-height"),
                                    standard.height);

        // Page orientation
        if (properties.attributeNS(KoXmlNS::style, "print-orientation", QString()) == "portrait")
            orientation = KoPageFormat::Portrait;
        else
            orientation = KoPageFormat::Landscape;

        // Margins.  Check if there is one "margin" attribute and use it for all
        // margins if there is.  Otherwise load the individual margins.
        if (properties.hasAttributeNS(KoXmlNS::fo, "margin")) {
            leftMargin  = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "margin"));
            topMargin = leftMargin;
            rightMargin = leftMargin;
            bottomMargin = leftMargin;
        }
        else {
            leftMargin   = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "margin-left"), MM_TO_POINT(20.0));
            topMargin    = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "margin-top"), MM_TO_POINT(20.0));
            rightMargin  = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "margin-right"), MM_TO_POINT(20.0));
            bottomMargin = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "margin-bottom"), MM_TO_POINT(20.0));
        }

        // Padding.  Same reasoning as for margins
        if (properties.hasAttributeNS(KoXmlNS::fo, "padding")) {
            leftPadding  = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "padding"));
            topPadding = leftPadding;
            rightPadding = leftPadding;
            bottomPadding = leftPadding;
        }
        else {
            leftPadding   = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "padding-left"));
            topPadding    = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "padding-top"));
            rightPadding  = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "padding-right"));
            bottomPadding = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "padding-bottom"));
        }

        // Parse border properties if there are any.
        border.loadOdf(properties);

        // guessFormat takes millimeters
        if (orientation == KoPageFormat::Landscape)
            format = KoPageFormat::guessFormat(POINT_TO_MM(height), POINT_TO_MM(width));
        else
            format = KoPageFormat::guessFormat(POINT_TO_MM(width), POINT_TO_MM(height));
    }
}

KoPageLayout KoPageLayout::standardLayout()
{
    KoPageLayout layout;

    layout.format = KoPageFormat::defaultFormat();

    // orientation and dimensions
    layout.orientation = KoPageFormat::Portrait;
    layout.width = MM_TO_POINT(KoPageFormat::width(layout.format, layout.orientation));
    layout.height = MM_TO_POINT(KoPageFormat::height(layout.format, layout.orientation));

    // margins
    layout.leftMargin = MM_TO_POINT(20.0);
    layout.rightMargin = MM_TO_POINT(20.0);
    layout.topMargin = MM_TO_POINT(20.0);
    layout.bottomMargin = MM_TO_POINT(20.0);

    // padding.  FIXME: Find the best real values.
    layout.leftPadding = MM_TO_POINT(20.0);
    layout.rightPadding = MM_TO_POINT(20.0);
    layout.topPadding = MM_TO_POINT(20.0);
    layout.bottomPadding = MM_TO_POINT(20.0);

    // borders
    layout.border.setLeftBorderStyle(KoBorder::BorderNone);
    layout.border.setTopBorderStyle(KoBorder::BorderNone);
    layout.border.setRightBorderStyle(KoBorder::BorderNone);
    layout.border.setBottomBorderStyle(KoBorder::BorderNone);

    layout.pageEdge = -1;
    layout.bindingSide = -1;

    return layout;
}

bool KoPageLayout::operator==(const KoPageLayout &l) const
{
    return qFuzzyCompare(width,l.width)
        && qFuzzyCompare(height,l.height)
        && qFuzzyCompare(leftMargin,l.leftMargin)
        && qFuzzyCompare(rightMargin,l.rightMargin)
        && qFuzzyCompare(topMargin,l.topMargin)
        && qFuzzyCompare(bottomMargin,l.bottomMargin)
        && qFuzzyCompare(pageEdge,l.pageEdge)
        && qFuzzyCompare(bindingSide,l.bindingSide)
        && border == l.border;
}

bool KoPageLayout::operator!=(const KoPageLayout& l) const
{
    return !((*this) == l);
}
