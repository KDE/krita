/* This file is part of the KDE project
   Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
   Copyright (C) 2009 Johannes Simon <johannes.simon@gmail.com>
   Copyright (C) 2010,2011 Jan Hambrecht <jaham@gmx.net>
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
 * Boston, MA 02110-1301, USA.
*/

#include "KoOdfWorkaround.h"

#include "KoShapeLoadingContext.h"
#include "KoShape.h"
#include "KoPathShape.h"
#include "KoColorBackground.h"
#include <KoOdfLoadingContext.h>
#include <KoXmlReader.h>
#include <KoXmlNS.h>
#include <KoStyleStack.h>
#include <KoUnit.h>

#include <QPen>
#include <QColor>

#include <FlakeDebug.h>

static bool s_workaroundPresentationPlaceholderBug = false;

void KoOdfWorkaround::fixPenWidth(QPen & pen, KoShapeLoadingContext &context)
{
    if (context.odfLoadingContext().generatorType() == KoOdfLoadingContext::OpenOffice && pen.widthF() == 0.0) {
        pen.setWidthF(0.5);
        debugFlake << "Work around OO bug with pen width 0";
    }
}

void KoOdfWorkaround::fixEnhancedPath(QString & path, const KoXmlElement &element, KoShapeLoadingContext &context)
{
    if (context.odfLoadingContext().generatorType() == KoOdfLoadingContext::OpenOffice) {
        if (path.isEmpty() && element.attributeNS(KoXmlNS::draw, "type", "") == "ellipse") {
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

    if (element.prefix() == "chart") {
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
                                    && !styleStack.hasProperty(KoXmlNS::draw, "fill-color"))) {
                KoXmlElement plotAreaElement = element.parentNode().toElement();
                KoXmlElement chartElement = plotAreaElement.parentNode().toElement();

                if (element.tagName() == "wall") {
                    if (chartElement.hasAttributeNS(KoXmlNS::chart, "class")) {
                        QString chartType = chartElement.attributeNS(KoXmlNS::chart, "class");
                        // TODO: Check what default backgrounds for surface, stock and gantt charts are
                        if (chartType == "chart:line" ||
                             chartType == "chart:area" ||
                             chartType == "chart:bar" ||
                             chartType == "chart:scatter")
                        color = QColor(0xe0e0e0);
                    }
                } else if (element.tagName() == "series") {
                    if (chartElement.hasAttributeNS(KoXmlNS::chart, "class")) {
                        QString chartType = chartElement.attributeNS(KoXmlNS::chart, "class");
                        // TODO: Check what default backgrounds for surface, stock and gantt charts are
                        if (chartType == "chart:area" ||
                             chartType == "chart:bar")
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

bool KoOdfWorkaround::fixMissingStroke(QPen &pen, const KoXmlElement &element, KoShapeLoadingContext &context, const KoShape *shape)
{
    bool fixed = false;

    if (context.odfLoadingContext().generatorType() == KoOdfLoadingContext::OpenOffice) {
        KoStyleStack &styleStack = context.odfLoadingContext().styleStack();
        if (element.prefix() == "chart") {
            styleStack.save();

            bool hasStyle = element.hasAttributeNS(KoXmlNS::chart, "style-name");
            if (hasStyle) {
                context.odfLoadingContext().fillStyleStack(element, KoXmlNS::chart, "style-name", "chart");
                styleStack.setTypeProperties("graphic");
            }

            if (hasStyle && styleStack.hasProperty(KoXmlNS::draw, "stroke") &&
                            !styleStack.hasProperty(KoXmlNS::svg, "stroke-color")) {
                fixed = true;
                pen.setColor(Qt::black);
            } else if (!hasStyle) {
                KoXmlElement plotAreaElement = element.parentNode().toElement();
                KoXmlElement chartElement = plotAreaElement.parentNode().toElement();

                if (element.tagName() == "series") {
                    QString chartType = chartElement.attributeNS(KoXmlNS::chart, "class");
                    if (!chartType.isEmpty()) {
                        // TODO: Check what default backgrounds for surface, stock and gantt charts are
                        if (chartType == "chart:line" ||
                             chartType == "chart:scatter") {
                            fixed = true;
                            pen = QPen(0x99ccff);
                        }
                    }
                } else if (element.tagName() == "legend") {
                    fixed = true;
                    pen = QPen(Qt::black);
                }
            }
            styleStack.restore();
        }
        else {
            const KoPathShape *pathShape = dynamic_cast<const KoPathShape*>(shape);
            if (pathShape) {
                const QString strokeColor(styleStack.property(KoXmlNS::svg, "stroke-color"));
                if (strokeColor.isEmpty()) {
                    pen.setColor(Qt::black);
                } else {
                    pen.setColor(strokeColor);
                }
                fixed = true;
            }
        }
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
    KoOdfLoadingContext::GeneratorType type(context.odfLoadingContext().generatorType());
    if (type == KoOdfLoadingContext::OpenOffice || type == KoOdfLoadingContext::MicrosoftOffice) {
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

QSharedPointer<KoColorBackground> KoOdfWorkaround::fixBackgroundColor(const KoShape *shape, KoShapeLoadingContext &context)
{
    QSharedPointer<KoColorBackground> colorBackground;
    KoOdfLoadingContext &odfContext = context.odfLoadingContext();
    if (odfContext.generatorType() == KoOdfLoadingContext::OpenOffice) {
        const KoPathShape *pathShape = dynamic_cast<const KoPathShape*>(shape);
        //check shape type
        if (pathShape) {
            KoStyleStack &styleStack = odfContext.styleStack();
            const QString color(styleStack.property(KoXmlNS::draw, "fill-color"));
            if (color.isEmpty()) {
                colorBackground = QSharedPointer<KoColorBackground>(new KoColorBackground(QColor(153, 204, 255)));
            } else {
                colorBackground = QSharedPointer<KoColorBackground>(new KoColorBackground(color));
            }
        }
    }
    return colorBackground;
}

void KoOdfWorkaround::fixGluePointPosition(QString &positionString, KoShapeLoadingContext &context)
{
    KoOdfLoadingContext::GeneratorType type(context.odfLoadingContext().generatorType());
    if (type == KoOdfLoadingContext::OpenOffice && !positionString.endsWith('%')) {
        const qreal pos = KoUnit::parseValue(positionString);
        positionString = QString("%1%%").arg(KoUnit(KoUnit::Millimeter).toUserValue(pos));
    }
}

void KoOdfWorkaround::fixMissingFillRule(Qt::FillRule& fillRule, KoShapeLoadingContext& context)
{
    if ((context.odfLoadingContext().generatorType() == KoOdfLoadingContext::OpenOffice)) {
        fillRule = Qt::OddEvenFill;
    }
}

bool KoOdfWorkaround::fixAutoGrow(KoTextShapeDataBase::ResizeMethod method, KoShapeLoadingContext &context)
{
    bool fix = false;
    if (context.odfLoadingContext().generatorType() == KoOdfLoadingContext::OpenOffice) {
        if (method == KoTextShapeDataBase::AutoGrowWidth || method == KoTextShapeDataBase::AutoGrowHeight || method == KoTextShapeDataBase::AutoGrowWidthAndHeight) {
            fix = true;
        }
    }
    return fix;
}

bool KoOdfWorkaround::fixEllipse(const QString &kind, KoShapeLoadingContext &context)
{
    bool radiusGiven = false;
    if (context.odfLoadingContext().generatorType() == KoOdfLoadingContext::OpenOffice) {
        if (kind == "section" || kind == "arc") {
            radiusGiven = true;
        }
    }
    return radiusGiven;
}

void KoOdfWorkaround::fixBadFormulaHiddenForStyleCellProtect(QString& value)
{
    if (value.endsWith(QLatin1String("Formula.hidden"))) {
        const int length = value.length();
        value[length-14] = QLatin1Char('f');
        value[length-7] = QLatin1Char('-');
    }
}

void KoOdfWorkaround::fixBadDateForTextTime(QString &value)
{
    if (value.startsWith(QLatin1String("0-00-00T"))) {
        value.remove(0, 8);
    }
}

void KoOdfWorkaround::fixClipRectOffsetValuesString(QString &offsetValuesString)
{
    if (! offsetValuesString.contains(QLatin1Char(','))) {
        // assumes no spaces existing between values and units
        offsetValuesString = offsetValuesString.simplified().replace(QLatin1Char(' '), QLatin1Char(','));
    }
}

QString KoOdfWorkaround::fixTableTemplateName(const KoXmlElement &e)
{
    return e.attributeNS(KoXmlNS::text, "style-name", QString());
}

QString KoOdfWorkaround::fixTableTemplateCellStyleName(const KoXmlElement &e)
{
    return e.attributeNS(KoXmlNS::text, "style-name", QString());
}

static const struct {
    const char* oldPath;
    const char* newPath;
} markerPathMapping[] = {
    // Arrow
    {"m10 0-10 30h20z",
     "M10 0l-10 30h20z"},
    // Square
    {"m0 0h10v10h-10",
     "M0 0h10v10h-10z"},
    // Small Arrow
    {"m1321 3493h-1321l702-3493z",
     "M1321 3493h-1321l702-3493z"},
     // Dimension Lines
    {"M0 0h278 278 280v36 36 38h-278-278-280v-36-36z",
     "m0 0h278 278 280v36 36 38h-278-278-280v-36-36z"},
    // Double Arrow
    {"m737 1131h394l-564-1131-567 1131h398l-398 787h1131z",
     "M737 1131h394l-564-1131-567 1131h398l-398 787h1131z"},
    // Rounded short Arrow
    {"m1009 1050-449-1008-22-30-29-12-34 12-21 26-449 1012-5 13v8l5 21 12 21 17 13 21 4h903l21-4 21-13 9-21 4-21v-8z",
     "M1009 1050l-449-1008-22-30-29-12-34 12-21 26-449 1012-5 13v8l5 21 12 21 17 13 21 4h903l21-4 21-13 9-21 4-21v-8z"},
    // Symmetric Arrow
    {"m564 0-564 902h1131z",
     "M564 0l-564 902h1131z"},
    // Line Arrow
    {"m0 2108v17 17l12 42 30 34 38 21 43 4 29-8 30-21 25-26 13-34 343-1532 339 1520 13 42 29 34 39 21 42 4 42-12 34-30 21-42v-39-12l-4 4-440-1998-9-42-25-39-38-25-43-8-42 8-38 25-26 39-8 42z",
     "M0 2108v17 17l12 42 30 34 38 21 43 4 29-8 30-21 25-26 13-34 343-1532 339 1520 13 42 29 34 39 21 42 4 42-12 34-30 21-42v-39-12l-4 4-440-1998-9-42-25-39-38-25-43-8-42 8-38 25-26 39-8 42z"},
    // Rounded large Arrow
    {"m1127 2120-449-2006-9-42-25-39-38-25-38-8-43 8-38 25-25 39-9 42-449 2006v13l-4 9 9 42 25 38 38 25 42 9h903l42-9 38-25 26-38 8-42v-9z",
     "M1127 2120l-449-2006-9-42-25-39-38-25-38-8-43 8-38 25-25 39-9 42-449 2006v13l-4 9 9 42 25 38 38 25 42 9h903l42-9 38-25 26-38 8-42v-9z"},
    // Circle
    {"m462 1118-102-29-102-51-93-72-72-93-51-102-29-102-13-105 13-102 29-106 51-102 72-89 93-72 102-50 102-34 106-9 101 9 106 34 98 50 93 72 72 89 51 102 29 106 13 102-13 105-29 102-51 102-72 93-93 72-98 51-106 29-101 13z",
     "M462 1118l-102-29-102-51-93-72-72-93-51-102-29-102-13-105 13-102 29-106 51-102 72-89 93-72 102-50 102-34 106-9 101 9 106 34 98 50 93 72 72 89 51 102 29 106 13 102-13 105-29 102-51 102-72 93-93 72-98 51-106 29-101 13z"},
    // Square 45
    {"m0 564 564 567 567-567-567-564z",
     "M0 564l564 567 567-567-567-564z"},
    // Arrow concave
    {"m1013 1491 118 89-567-1580-564 1580 114-85 136-68 148-46 161-17 161 13 153 46z",
     "M1013 1491l118 89-567-1580-564 1580 114-85 136-68 148-46 161-17 161 13 153 46z"},
    // Short line Arrow
    {"m1500 0 1500 2789v211h-114l-1286-2392v2392h-200v-2392l-1286 2392h-114v-211z",
     "M1500 0l1500 2789v211h-114l-1286-2392v2392h-200v-2392l-1286 2392h-114v-211z"},
    // Triangle unfilled
    {"m1500 0 1500 3000h-3000zm1500-2553-1176 2353h2353z",
     "M1500 0l1500 3000h-3000zM1500 447l-1176 2353h2353z"},
    // Diamond unfilled
    {"m1500 0 1500 3000-1500 3000-1500-3000zm1500-2553-1276 2553 1276 2553 1276-2553z",
     "M1500 0l1500 3000-1500 3000-1500-3000zM1500 447l-1276 2553 1276 2553 1276-2553z"},
    // Diamond
    {"m1500 0 1500 3000-1500 3000-1500-3000z",
     "M1500 0l1500 3000-1500 3000-1500-3000z"},
    // Circle unfilled
    {"m1500 3000c-276 0-511-63-750-201s-411-310-549-549-201-474-201-750 63-511 201-750 310-411 549-549 474-201 750-201 511 63 750 201 411 310 549 549 201 474 201 750-63 511-201 750-310 411-549 549-474 201-750 201zm0-200c-239 0-443-55-650-174s-356-269-476-476-174-411-174-650 55-443 174-650 269-356 476-476c207-119 411-174 650-174s443 55 650 174c207 120 356 269 476 476s174 411 174 650-55 443-174 650-269 356-476 476c-207 119-411 174-650 174z",
     "M1500 3000c-276 0-511-63-750-201s-411-310-549-549-201-474-201-750 63-511 201-750 310-411 549-549 474-201 750-201 511 63 750 201 411 310 549 549 201 474 201 750-63 511-201 750-310 411-549 549-474 201-750 201zM1500 2800c-239 0-443-55-650-174s-356-269-476-476-174-411-174-650 55-443 174-650 269-356 476-476c207-119 411-174 650-174s443 55 650 174c207 120 356 269 476 476s174 411 174 650-55 443-174 650-269 356-476 476c-207 119-411 174-650 174z"},
    // Square 45 unfilled
    {"m1500 3000-1500-1500 1500-1500 1500 1500zm-1500 1215-1215-1215 1215-1215 1215 1215z",
     "M1500 3000l-1500-1500 1500-1500 1500 1500zM1500 2715l-1215-1215 1215-1215 1215 1215z"},
    // Square unfilled
    {"m0 0h300v300h-300zm20-280h260v260h-260z",
     "M0 0h300v300h-300zM20 20h260v260h-260z"},
    // Half Circle unfilled
    {"m14971 0c21 229 29 423 29 653 0 690-79 1328-244 1943-165 614-416 1206-761 1804-345 597-733 1110-1183 1560-451 450-964 837-1562 1182-598 345-1190 596-1806 760-600 161-1223 240-1894 244v600h-100v-600c-671-4-1294-83-1894-244-616-164-1208-415-1806-760-598-345-1111-732-1562-1182-450-450-838-963-1183-1560-345-598-596-1190-761-1804-165-615-244-1253-244-1943 0-230 8-424 29-653l298 26 299 26c-18 211-26 390-26 601 0 635 72 1222 224 1787 151 566 383 1110 700 1659 318 550 674 1022 1088 1437 415 414 888 769 1438 1087 550 317 1095 548 1661 700 566 151 1154 223 1789 223s1223-72 1789-223c566-152 1111-383 1661-700 550-318 1023-673 1438-1087 414-415 770-887 1088-1437 317-549 549-1093 700-1659 152-565 224-1152 224-1787 0-211-8-390-26-601l299-26z",
     "M14971 0c21 229 29 423 29 653 0 690-79 1328-244 1943-165 614-416 1206-761 1804-345 597-733 1110-1183 1560-451 450-964 837-1562 1182s-1190 596-1806 760c-600 161-1223 240-1894 244v600h-100v-600c-671-4-1294-83-1894-244-616-164-1208-415-1806-760s-1111-732-1562-1182c-450-450-838-963-1183-1560-345-598-596-1190-761-1804-165-615-244-1253-244-1943 0-230 8-424 29-653l298 26 299 26c-18 211-26 390-26 601 0 635 72 1222 224 1787 151 566 383 1110 700 1659 318 550 674 1022 1088 1437 415 414 888 769 1438 1087 550 317 1095 548 1661 700 566 151 1154 223 1789 223s1223-72 1789-223c566-152 1111-383 1661-700 550-318 1023-673 1438-1087 414-415 770-887 1088-1437 317-549 549-1093 700-1659 152-565 224-1152 224-1787 0-211-8-390-26-601l299-26z"}
};
static const int markerPathMappingSize = sizeof(markerPathMapping)/sizeof(markerPathMapping[0]);

void KoOdfWorkaround::fixMarkerPath(QString& path)
{
    for (int i = 0; i < markerPathMappingSize; ++i) {
        if (path == QLatin1String(markerPathMapping[i].oldPath)) {
            path = QLatin1String(markerPathMapping[i].newPath);
            break;
        }
    }
}
