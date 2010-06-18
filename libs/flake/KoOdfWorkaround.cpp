/* This file is part of the KDE project
   Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Johannes Simon <johannes.simon@gmail.com>
   Copyright (C) 2010 Jan Hambrecht <jaham@gmx.net>

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
#include "KoShape.h"
#include <KoPathShape.h>
#include <KoOdfLoadingContext.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoColorBackground.h>
#include <KoStyleStack.h>

#include <QPen>
#include <QColor>

#include <kdebug.h>

static bool s_workaroundPresentationPlaceholderBug = false;

void KoOdfWorkaround::fixPenWidth(QPen & pen, KoShapeLoadingContext &context)
{
    if (context.odfLoadingContext().generatorType() == KoOdfLoadingContext::OpenOffice && pen.widthF() == 0.0) {
        pen.setWidthF(0.5);
        kDebug(30003) << "Work around OO bug with pen width 0";
    }
}

void KoOdfWorkaround::fixEnhancedPath(QString & path, const KoXmlElement &element, KoShapeLoadingContext &context)
{
    if (context.odfLoadingContext().generatorType() == KoOdfLoadingContext::OpenOffice) {
        if (path.isEmpty() && element.attributeNS(KoXmlNS::draw, "type", "") == "ellipse" ) {
            path = "U 10800 10800 10800 10800 0 360 Z N";
        }
    }
}

void KoOdfWorkaround::fixEnhancedPathPolarHandlePosition(QString &position, const KoXmlElement &element, KoShapeLoadingContext &context)
{
    if (context.odfLoadingContext().generatorType() == KoOdfLoadingContext::OpenOffice) {
        if (element.hasAttributeNS(KoXmlNS::draw, "handle-polar")) {
            QStringList tokens = position.simplified().split(' ');
            if (tokens.count() == 2) {
                position = tokens[1] + ' ' + tokens[0];
            }
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

        if (context.odfLoadingContext().generatorType() == KoOdfLoadingContext::OpenOffice) {
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

bool KoOdfWorkaround::fixMissingStroke(QPen &pen, const KoXmlElement &element, KoShapeLoadingContext &context)
{
    bool fixed = false;

    if ( element.prefix() == "chart" ) {
        KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
        styleStack.save();

        bool hasStyle = element.hasAttributeNS(KoXmlNS::chart, "style-name");
        if (hasStyle) {
            context.odfLoadingContext().fillStyleStack(element, KoXmlNS::chart, "style-name", "chart");
            styleStack.setTypeProperties("graphic");
        }

        if (context.odfLoadingContext().generatorType() == KoOdfLoadingContext::OpenOffice) {
            if (hasStyle && styleStack.hasProperty(KoXmlNS::draw, "stroke") &&
                            !styleStack.hasProperty(KoXmlNS::draw, "stroke-color")) {
                fixed = true;
                pen.setColor( Qt::black );
            } else if (!hasStyle) {
                KoXmlElement plotAreaElement = element.parentNode().toElement();
                KoXmlElement chartElement = plotAreaElement.parentNode().toElement();

                if (element.tagName() == "series") {
                    if (chartElement.hasAttributeNS(KoXmlNS::chart, "class")) {
                        QString chartType = chartElement.attributeNS(KoXmlNS::chart, "class");
                        // TODO: Check what default backgrounds for surface, stock and gantt charts are
                        if ( chartType == "chart:line" ||
                             chartType == "chart:scatter" ) {
                            fixed = true;
                            pen = QPen( 0x99ccff );
                        }
                    }
                } else if (element.tagName() == "legend") {
                    fixed = true;
                    pen = QPen( Qt::black );
                }
            }
        }

        styleStack.restore();
    }

    return fixed;
}

bool KoOdfWorkaround::fixMissingStyle_DisplayLabel(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    Q_UNUSED(element);
    // If no axis style is specified, OpenOffice.org hides the axis' data labels
    if (context.odfLoadingContext().generatorType() == KoOdfLoadingContext::OpenOffice)
        return false;

    // In all other cases, they're visible
    return true;
}

void KoOdfWorkaround::setFixPresentationPlaceholder(bool fix, KoShapeLoadingContext &context)
{
    if (context.odfLoadingContext().generatorType() == KoOdfLoadingContext::OpenOffice) {
        s_workaroundPresentationPlaceholderBug = fix;
    }
}

bool KoOdfWorkaround::fixPresentationPlaceholder()
{
    return s_workaroundPresentationPlaceholderBug;
}

void KoOdfWorkaround::fixPresentationPlaceholder(KoShape *shape)
{
    if (s_workaroundPresentationPlaceholderBug && !shape->hasAdditionalAttribute("presentation:placeholder")) {
        shape->setAdditionalAttribute("presentation:placeholder", "true");
    }
}

KoColorBackground *KoOdfWorkaround::fixBackgroundColor(const KoShape *shape, KoShapeLoadingContext &context)
{
    KoColorBackground *colorBackground = 0;
    KoOdfLoadingContext &odfContext = context.odfLoadingContext();
    if (odfContext.generatorType() == KoOdfLoadingContext::OpenOffice) {
        const KoPathShape *pathShape = dynamic_cast<const KoPathShape*>(shape);
        //check shape type
        if (pathShape) {
            KoStyleStack &styleStack = odfContext.styleStack();
            const QString color(styleStack.property(KoXmlNS::draw, "fill-color"));
            if (color.isEmpty()) {
                colorBackground = new KoColorBackground(Qt::cyan);
            } else {
                colorBackground = new KoColorBackground(color);
            }
        }
    }
    return colorBackground;
}
