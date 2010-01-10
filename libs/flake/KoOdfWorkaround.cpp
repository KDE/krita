/* This file is part of the KDE project
   Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Johannes Simon <johannes.simon@gmail.com>

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
#include <KoColorBackground.h>
#include <KoStyleStack.h>

#include <QPen>
#include <QColor>

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

    if ( element.prefix() == "chart" ) {
        KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
        styleStack.save();

        bool hasStyle = element.hasAttributeNS(KoXmlNS::chart, "style-name");
        if (hasStyle) {
            context.odfLoadingContext().fillStyleStack(element, KoXmlNS::chart, "style-name", "chart");
            styleStack.setTypeProperties("graphic");
        }

        if (context.odfLoadingContext().generator().startsWith("OpenOffice.org")) {
            if (hasStyle && !styleStack.hasProperty(KoXmlNS::draw, "fill") &&
                             styleStack.hasProperty(KoXmlNS::draw, "fill-color")) {
                color = QColor(styleStack.property(KoXmlNS::draw, "fill-color"));
            } else if (!hasStyle || (!styleStack.hasProperty(KoXmlNS::draw, "fill")
                                    && !styleStack.hasProperty(KoXmlNS::draw, "fill-color")) ) {
                KoXmlElement plotAreaElement = element.parentNode().toElement();
                KoXmlElement chartElement = plotAreaElement.parentNode().toElement();

                if (element.tagName() == "wall") {
                    if (chartElement.hasAttributeNS(KoXmlNS::chart, "class")) {
                        QString chartType = chartElement.attributeNS(KoXmlNS::chart, "class");
                        // TODO: Check what default backgrounds for surface, stock and gantt charts are
                        if ( chartType == "chart:line" ||
                             chartType == "chart:area" ||
                             chartType == "chart:bar" ||
                             chartType == "chart:scatter" )
                        color = QColor(0xe0e0e0);
                    }
                } else if (element.tagName() == "series") {
                    if (chartElement.hasAttributeNS(KoXmlNS::chart, "class")) {
                        QString chartType = chartElement.attributeNS(KoXmlNS::chart, "class");
                        // TODO: Check what default backgrounds for surface, stock and gantt charts are
                        if ( chartType == "chart:area" ||
                             chartType == "chart:bar" )
                            color = QColor(0x99ccff);
                    }
                }
                else if (element.tagName() == "chart")
                    color = QColor(0xffffff);
            }
        }

        styleStack.restore();
    }

    return color;
}

QColor KoOdfWorkaround::fixMissingStrokeColor(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    // Default to an invalid color
    QColor color;

    if ( element.prefix() == "chart" ) {
        KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
        styleStack.save();

        bool hasStyle = element.hasAttributeNS(KoXmlNS::chart, "style-name");
        if (hasStyle) {
            context.odfLoadingContext().fillStyleStack(element, KoXmlNS::chart, "style-name", "chart");
            styleStack.setTypeProperties("graphic");
        }

        if (context.odfLoadingContext().generator().startsWith("OpenOffice.org")) {
            if (hasStyle && styleStack.hasProperty(KoXmlNS::draw, "stroke") &&
                            !styleStack.hasProperty(KoXmlNS::draw, "stroke-color")) {
                color = Qt::black;
            } else if (!hasStyle) {
                KoXmlElement plotAreaElement = element.parentNode().toElement();
                KoXmlElement chartElement = plotAreaElement.parentNode().toElement();

                if (element.tagName() == "series") {
                    if (chartElement.hasAttributeNS(KoXmlNS::chart, "class")) {
                        QString chartType = chartElement.attributeNS(KoXmlNS::chart, "class");
                        // TODO: Check what default backgrounds for surface, stock and gantt charts are
                        if ( chartType == "chart:line" ||
                             chartType == "chart:scatter" )
                            color = QColor(0x99ccff);
                    }
                }
            }
        }

        styleStack.restore();
    }

    return color;
}

bool KoOdfWorkaround::fixMissingStyle_DisplayLabel(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    // If no axis style is specified, OpenOffice.org hides the axis' data labels
    if (context.odfLoadingContext().generator().startsWith("OpenOffice.org"))
        return false;

    // In all other cases, they're visible
    return true;
}
