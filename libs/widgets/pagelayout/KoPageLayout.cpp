/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>
   Copyright 2002, 2003 David Faure <faure@kde.org>
   Copyright 2003 Nicolas GOUTTE <goutte@kde.org>
   Copyright 2007 Thomas Zander <zander@kde.org>

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

#include "KoXmlNS.h"
#include "KoUnit.h"

KoGenStyle KoPageLayout::saveOdf() const
{
    KoGenStyle style(KoGenStyle::StylePageLayout);
    style.addPropertyPt("fo:page-width", width);
    style.addPropertyPt("fo:page-height", height);
    style.addPropertyPt("fo:margin-left", left);
    style.addPropertyPt("fo:margin-right", right);
    style.addPropertyPt("fo:margin-top", top);
    style.addPropertyPt("fo:margin-bottom", bottom);
    style.addProperty("style:print-orientation", (orientation == KoPageFormat::Landscape ? "landscape" : "portrait"));
    return style;
}

void KoPageLayout::loadOdf(const KoXmlElement &style)
{
    KoXmlElement properties(KoXml::namedItemNS(style, KoXmlNS::style, "page-layout-properties"));
    if (!properties.isNull()) {
        width = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "page-width"));
        height = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "page-height"));
        KoPageLayout standard = standardLayout();
        if (width == 0)
            width = standard.width;
        if (height == 0)
            height = standard.height;
        if (properties.attributeNS(KoXmlNS::style, "print-orientation", QString()) == "portrait")
            orientation = KoPageFormat::Portrait;
        else
            orientation = KoPageFormat::Landscape;
        right = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "margin-right"));
        bottom = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "margin-bottom"));
        left = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "margin-left"));
        top = KoUnit::parseValue(properties.attributeNS(KoXmlNS::fo, "margin-top"));
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
    layout.orientation = KoPageFormat::Portrait;
    layout.width = MM_TO_POINT(KoPageFormat::width(layout.format, layout.orientation));
    layout.height = MM_TO_POINT(KoPageFormat::height(layout.format, layout.orientation));
    layout.left = MM_TO_POINT(20.0);
    layout.right = MM_TO_POINT(20.0);
    layout.top = MM_TO_POINT(20.0);
    layout.bottom = MM_TO_POINT(20.0);
    layout.pageEdge = -1;
    layout.bindingSide = -1;
    return layout;
}
