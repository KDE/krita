/* This file is part of the KDE project
   Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>

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

#include "KoOdfWorkaround.h"

#include "KoShapeLoadingContext.h"
#include <KoOdfLoadingContext.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <QPen>
#include <QColor>
#include <KoColorBackground.h>

#include <kdebug.h>

// TODO only parse the generator string once so we don't have a string compare for every loaded shape
void KoOdfWorkaround::fixPenWidth(QPen & pen, KoShapeLoadingContext &context)
{
    if (context.odfLoadingContext().generator().startsWith("OpenOffice.org") && pen.widthF() == 0.0) {
        pen.setWidthF(0.5);
        kDebug(30003) << "Work around OO bug with pen width 0";
    }
}

void KoOdfWorkaround::fixEnhancedPath(QString & path, const KoXmlElement &element, KoShapeLoadingContext &context)
{
    if (context.odfLoadingContext().generator().startsWith("OpenOffice.org") ) {
        if (path.isEmpty() && element.attributeNS(KoXmlNS::draw, "type", "") == "ellipse" ) {
            path = "U 10800 10800 10800 10800 0 360 Z N";
        }
    }
}

QColor KoOdfWorkaround::fixMissingFillColor(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    // Default to an invalid color
    QColor color;
    if (context.odfLoadingContext().generator().startsWith("OpenOffice.org")) {
        if (element.prefix() == "chart" && element.tagName() == "wall")
            color = QColor("#e0e0e0");
        if (element.prefix() == "chart" && element.tagName() == "series")
            color = QColor("#99ccff");
        if (element.prefix() == "chart" && element.tagName() == "chart")
            color = QColor("#ffffff");
    }
    return color;
}
