/* This file is part of the KDE project
 * Copyright (C) 2002-2005,2007 Rob Buis <buis@kde.org>
 * Copyright (C) 2002-2004 Nicolas Goutte <nicolasg@snafu.de>
 * Copyright (C) 2005-2006 Tim Beaulen <tbscope@gmail.com>
 * Copyright (C) 2005-2009 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2005,2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006-2007 Inge Wallin <inge@lysator.liu.se>
 * Copyright (C) 2007-2008,2010 Thorsten Zachmann <zachmann@kde.org>

 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "SvgStyleParser.h"
#include "SvgLoadingContext.h"
#include "SvgGraphicContext.h"
#include "SvgUtil.h"
#include <text/KoSvgTextProperties.h>

#include <QStringList>
#include <QColor>
#include <QGradientStops>

class Q_DECL_HIDDEN SvgStyleParser::Private
{
public:
    Private(SvgLoadingContext &loadingContext)
        : context(loadingContext)
    {
        textAttributes << KoSvgTextProperties::supportedXmlAttributes();

        // the order of the font attributes is important, don't change without reason !!!
        fontAttributes << "font-family" << "font-size" << "font-weight" << "font-style"
                       << "font-variant" << "font-stretch" << "font-size-adjust" << "font"
                       << "text-decoration" << "letter-spacing" << "word-spacing" << "baseline-shift";
        // the order of the style attributes is important, don't change without reason !!!
        styleAttributes << "color" << "display" << "visibility";
        styleAttributes << "fill" << "fill-rule" << "fill-opacity";
        styleAttributes << "stroke" << "stroke-width" << "stroke-linejoin" << "stroke-linecap";
        styleAttributes << "stroke-dasharray" << "stroke-dashoffset" << "stroke-opacity" << "stroke-miterlimit";
        styleAttributes << "opacity" << "filter" << "clip-path" << "clip-rule" << "mask";
        styleAttributes << "marker" << "marker-start" << "marker-mid" << "marker-end" << "krita:marker-fill-method";
    }

    SvgLoadingContext &context;
    QStringList textAttributes; ///< text related attributes
    QStringList fontAttributes; ///< font related attributes
    QStringList styleAttributes; ///< style related attributes
};

SvgStyleParser::SvgStyleParser(SvgLoadingContext &context)
    : d(new Private(context))
{

}

SvgStyleParser::~SvgStyleParser()
{
    delete d;
}

void SvgStyleParser::parseStyle(const SvgStyles &styles)
{
    SvgGraphicsContext *gc = d->context.currentGC();
    if (!gc)
        return;

    // make sure we parse the style attributes in the right order
    Q_FOREACH (const QString & command, d->styleAttributes) {
        const QString &params = styles.value(command);
        if (params.isEmpty())
            continue;
        parsePA(gc, command, params);
    }
}

void SvgStyleParser::parseFont(const SvgStyles &styles)
{
    SvgGraphicsContext *gc = d->context.currentGC();
    if (!gc)
        return;

    // make sure to only parse font attributes here
    Q_FOREACH (const QString & command, d->fontAttributes) {
        const QString &params = styles.value(command);
        if (params.isEmpty())
            continue;
        parsePA(gc, command, params);
    }

    Q_FOREACH (const QString & command, d->textAttributes) {
        const QString &params = styles.value(command);
        if (params.isEmpty())
            continue;
        parsePA(gc, command, params);
    }
}
#include <kis_debug.h>
void SvgStyleParser::parsePA(SvgGraphicsContext *gc, const QString &command, const QString &params)
{
    QColor fillcolor = gc->fillColor;
    QColor strokecolor = gc->stroke->color();

    if (params == "inherit")
        return;

    if (command == "fill") {
        if (params == "none") {
            gc->fillType = SvgGraphicsContext::None;
        } else if (params.startsWith(QLatin1String("url("))) {
            unsigned int start = params.indexOf('#') + 1;
            unsigned int end = params.indexOf(')', start);
            gc->fillId = params.mid(start, end - start);
            gc->fillType = SvgGraphicsContext::Complex;
            // check if there is a fallback color
            parseColor(fillcolor, params.mid(end + 1).trimmed());
        } else {
            // great we have a solid fill
            gc->fillType = SvgGraphicsContext::Solid;
            parseColor(fillcolor,  params);
        }
    } else if (command == "fill-rule") {
        if (params == "nonzero")
            gc->fillRule = Qt::WindingFill;
        else if (params == "evenodd")
            gc->fillRule = Qt::OddEvenFill;
    } else if (command == "stroke") {
        if (params == "none") {
            gc->strokeType = SvgGraphicsContext::None;
        } else if (params.startsWith(QLatin1String("url("))) {
            unsigned int start = params.indexOf('#') + 1;
            unsigned int end = params.indexOf(')', start);
            gc->strokeId = params.mid(start, end - start);
            gc->strokeType = SvgGraphicsContext::Complex;
            // check if there is a fallback color
            parseColor(strokecolor, params.mid(end + 1).trimmed());
        } else {
            // great we have a solid stroke
            gc->strokeType = SvgGraphicsContext::Solid;
            parseColor(strokecolor, params);
        }
    } else if (command == "stroke-width") {
        gc->stroke->setLineWidth(SvgUtil::parseUnitXY(gc, params));
    } else if (command == "stroke-linejoin") {
        if (params == "miter")
            gc->stroke->setJoinStyle(Qt::MiterJoin);
        else if (params == "round")
            gc->stroke->setJoinStyle(Qt::RoundJoin);
        else if (params == "bevel")
            gc->stroke->setJoinStyle(Qt::BevelJoin);
    } else if (command == "stroke-linecap") {
        if (params == "butt")
            gc->stroke->setCapStyle(Qt::FlatCap);
        else if (params == "round")
            gc->stroke->setCapStyle(Qt::RoundCap);
        else if (params == "square")
            gc->stroke->setCapStyle(Qt::SquareCap);
    } else if (command == "stroke-miterlimit") {
        gc->stroke->setMiterLimit(params.toFloat());
    } else if (command == "stroke-dasharray") {
        QVector<qreal> array;
        if (params != "none") {
            QString dashString = params;
            QStringList dashes = dashString.replace(',', ' ').simplified().split(' ');
            for (QStringList::Iterator it = dashes.begin(); it != dashes.end(); ++it) {
                array.append(SvgUtil::parseUnitXY(gc, *it));
            }

            // if the array is odd repeat it according to the standard
            if (array.size() & 1) {
                array << array;
            }
        }
        gc->stroke->setLineStyle(Qt::CustomDashLine, array);
    } else if (command == "stroke-dashoffset") {
        gc->stroke->setDashOffset(params.toFloat());
    }
    // handle opacity
    else if (command == "stroke-opacity")
        strokecolor.setAlphaF(SvgUtil::fromPercentage(params));
    else if (command == "fill-opacity") {
        float opacity = SvgUtil::fromPercentage(params);
        if (opacity < 0.0)
            opacity = 0.0;
        if (opacity > 1.0)
            opacity = 1.0;
        fillcolor.setAlphaF(opacity);
    } else if (command == "opacity") {
        gc->opacity = SvgUtil::fromPercentage(params);
    } else if (command == "font-family") {
        QStringList familiesList = params.split(',', QString::SkipEmptyParts);
        for (QString &family : familiesList) {
            family = family.trimmed();
            if ((family.startsWith('\"') && family.endsWith('\"')) ||
                (family.startsWith('\'') && family.endsWith('\''))) {

                family = family.mid(1, family.size() - 2);
            }
        }

        gc->font.setFamily(familiesList.first());
        gc->fontFamiliesList = familiesList;
    } else if (command == "font-size") {
        float pointSize = SvgUtil::parseUnitY(gc, params);
        if (pointSize > 0.0f)
            gc->font.setPointSizeF(pointSize);
    } else if (command == "font-style") {
        gc->font.setStyle(
            params == "italic" ? QFont::StyleItalic :
            params == "oblique" ? QFont::StyleOblique :
            QFont::StyleNormal);
    } else if (command == "font-variant") {
        gc->font.setCapitalization(
            params == "small-caps" ? QFont::SmallCaps :
            QFont::MixedCase);
    } else if (command == "font-stretch") {
        int newStretch = 100;

        static const std::vector<int> fontStretches = {50, 62, 75, 87, 100, 112, 125, 150, 200};

        if (params == "wider") {
            auto it = std::upper_bound(fontStretches.begin(),
                                       fontStretches.end(),
                                       gc->font.stretch());

            newStretch = it != fontStretches.end() ? *it : fontStretches.back();
        } else if (params == "narrower") {
            auto it = std::upper_bound(fontStretches.rbegin(),
                                       fontStretches.rend(),
                                       gc->font.stretch(),
                                       std::greater<int>());

            newStretch = it != fontStretches.rend() ? *it : fontStretches.front();

        } else if (params == "ultra-condensed") {
            newStretch = QFont::UltraCondensed;
        } else if (params == "extra-condensed") {
            newStretch = QFont::ExtraCondensed;
        } else if (params == "condensed") {
            newStretch = QFont::Condensed;
        } else if (params == "semi-condensed") {
            newStretch = QFont::SemiCondensed;
        } else if (params == "semi-expanded") {
            newStretch = QFont::SemiExpanded;
        } else if (params == "expanded") {
            newStretch = QFont::Expanded;
        } else if (params == "extra-expanded") {
            newStretch = QFont::ExtraExpanded;
        } else if (params == "ultra-expanded") {
            newStretch = QFont::UltraExpanded;
        }

        gc->font.setStretch(newStretch);

    } else if (command == "font-weight") {
        int weight = QFont::Normal;

        // map svg weight to qt weight
        // svg value        qt value
        // 100,200,300      1, 17, 33
        // 400              50          (normal)
        // 500,600          58,66
        // 700              75          (bold)
        // 800,900          87,99

        static const std::vector<int> fontWeights = {1,17,33,50,58,66,75,87,99};

        if (params == "bold")
            weight = QFont::Bold;
        else if (params == "bolder") {
            auto it = std::upper_bound(fontWeights.begin(),
                                       fontWeights.end(),
                                       gc->font.weight());

            weight = it != fontWeights.end() ? *it : fontWeights.back();
        } else if (params == "lighter") {
            auto it = std::upper_bound(fontWeights.rbegin(),
                                       fontWeights.rend(),
                                       gc->font.weight(),
                                       std::greater<int>());

            weight = it != fontWeights.rend() ? *it : fontWeights.front();
        } else {
            bool ok;
            // try to read numerical weight value
            weight = params.toInt(&ok, 10);

            if (!ok)
                return;

            switch (weight) {
            case 100: weight = 1; break;
            case 200: weight = 17; break;
            case 300: weight = 33; break;
            case 400: weight = 50; break;
            case 500: weight = 58; break;
            case 600: weight = 66; break;
            case 700: weight = 75; break;
            case 800: weight = 87; break;
            case 900: weight = 99; break;
            }
        }
        gc->font.setWeight(weight);
    } else if (command == "font-size-adjust") {
        warnFile << "WARNING: \'font-size-adjust\' SVG attribute is not supported!";
    } else if (command == "font") {
        warnFile << "WARNING: \'font\' SVG attribute is not yet implemented! Please report a bug!";
    } else if (command == "text-decoration") {
        gc->font.setStrikeOut(false);
        gc->font.setUnderline(false);
        gc->font.setOverline(false);

        Q_FOREACH (const QString &param, params.split(' ', QString::SkipEmptyParts)) {
            if (param == "line-through") {
                gc->font.setStrikeOut(true);
            } else if (param == "underline") {
                gc->font.setUnderline(true);
            } else if (param == "overline") {
                gc->font.setOverline(true);
            }
        }
    } else if (command == "color") {
        QColor color;
        parseColor(color, params);
        gc->currentColor = color;
    } else if (command == "display") {
        if (params == "none")
            gc->display = false;
    } else if (command == "visibility") {
        // visible is inherited!
        gc->visible = params == "visible";
    } else if (command == "filter") {
        if (params != "none" && params.startsWith("url(")) {
            unsigned int start = params.indexOf('#') + 1;
            unsigned int end = params.indexOf(')', start);
            gc->filterId = params.mid(start, end - start);
        }
    } else if (command == "clip-path") {
        if (params != "none" && params.startsWith("url(")) {
            unsigned int start = params.indexOf('#') + 1;
            unsigned int end = params.indexOf(')', start);
            gc->clipPathId = params.mid(start, end - start);
        }
    } else if (command == "clip-rule") {
        if (params == "nonzero")
            gc->clipRule = Qt::WindingFill;
        else if (params == "evenodd")
            gc->clipRule = Qt::OddEvenFill;
    } else if (command == "mask") {
        if (params != "none" && params.startsWith("url(")) {
            unsigned int start = params.indexOf('#') + 1;
            unsigned int end = params.indexOf(')', start);
            gc->clipMaskId = params.mid(start, end - start);
        }
    } else if (command == "marker-start") {
           if (params != "none" && params.startsWith("url(")) {
               unsigned int start = params.indexOf('#') + 1;
               unsigned int end = params.indexOf(')', start);
               gc->markerStartId = params.mid(start, end - start);
           }
    } else if (command == "marker-end") {
        if (params != "none" && params.startsWith("url(")) {
            unsigned int start = params.indexOf('#') + 1;
            unsigned int end = params.indexOf(')', start);
            gc->markerEndId = params.mid(start, end - start);
        }
    } else if (command == "marker-mid") {
        if (params != "none" && params.startsWith("url(")) {
            unsigned int start = params.indexOf('#') + 1;
            unsigned int end = params.indexOf(')', start);
            gc->markerMidId = params.mid(start, end - start);
        }
    } else if (command == "marker") {
        if (params != "none" && params.startsWith("url(")) {
            unsigned int start = params.indexOf('#') + 1;
            unsigned int end = params.indexOf(')', start);
            gc->markerStartId = params.mid(start, end - start);
            gc->markerMidId = gc->markerStartId;
            gc->markerEndId = gc->markerStartId;
        }
    } else if (command == "krita:marker-fill-method") {
        gc->autoFillMarkers = params == "auto";
    } else if (d->textAttributes.contains(command)) {
        gc->textProperties.parseSVGTextAttribute(d->context, command, params);
    }

    gc->fillColor = fillcolor;
    gc->stroke->setColor(strokecolor);
}

bool SvgStyleParser::parseColor(QColor &color, const QString &s)
{
    if (s.isEmpty() || s == "none")
        return false;

    if (s.startsWith(QLatin1String("rgb("))) {
        QString parse = s.trimmed();
        QStringList colors = parse.split(',');
        QString r = colors[0].right((colors[0].length() - 4));
        QString g = colors[1];
        QString b = colors[2].left((colors[2].length() - 1));

        if (r.contains('%')) {
            r = r.left(r.length() - 1);
            r = QString::number(int((double(255 * r.toDouble()) / 100.0)));
        }

        if (g.contains('%')) {
            g = g.left(g.length() - 1);
            g = QString::number(int((double(255 * g.toDouble()) / 100.0)));
        }

        if (b.contains('%')) {
            b = b.left(b.length() - 1);
            b = QString::number(int((double(255 * b.toDouble()) / 100.0)));
        }

        color = QColor(r.toInt(), g.toInt(), b.toInt());
    } else if (s == "currentColor") {
        color = d->context.currentGC()->currentColor;
    } else {
        // QColor understands #RRGGBB and svg color names
        color.setNamedColor(s.trimmed());
    }

    return true;
}

void SvgStyleParser::parseColorStops(QGradient *gradient,
                                     const KoXmlElement &e,
                                     SvgGraphicsContext *context,
                                     const QGradientStops &defaultStops)
{
    QGradientStops stops;

    qreal previousOffset = 0.0;

    KoXmlElement stop;
    forEachElement(stop, e) {
        if (stop.tagName() == "stop") {
            qreal offset = 0.0;
            QString offsetStr = stop.attribute("offset").trimmed();
            if (offsetStr.endsWith('%')) {
                offsetStr = offsetStr.left(offsetStr.length() - 1);
                offset = offsetStr.toFloat() / 100.0;
            } else {
                offset = offsetStr.toFloat();
            }

            // according to SVG the value must be within [0; 1] interval
            offset = qBound(0.0, offset, 1.0);

            // according to SVG the stops' offset must be non-decreasing
            offset = qMax(offset, previousOffset);
            previousOffset = offset;

            QColor color;

            QString stopColorStr = stop.attribute("stop-color");
            QString stopOpacityStr = stop.attribute("stop-opacity");

            const QStringList attributes({"stop-color", "stop-opacity"});
            SvgStyles styles = parseOneCssStyle(stop.attribute("style"), attributes);

            // SVG: CSS values have precedence over presentation attributes!
            if (styles.contains("stop-color")) {
                stopColorStr = styles.value("stop-color");
            }

            if (styles.contains("stop-opacity")) {
                stopOpacityStr = styles.value("stop-opacity");
            }

            if (stopColorStr.isEmpty() && stopColorStr == "inherit") {
                color = context->currentColor;
            } else {
                parseColor(color, stopColorStr);
            }

            if (!stopOpacityStr.isEmpty() && stopOpacityStr != "inherit") {
                color.setAlphaF(qBound(0.0, stopOpacityStr.toDouble(), 1.0));
            }

            stops.append(QPair<qreal, QColor>(offset, color));
        }
    }

    if (!stops.isEmpty()) {
        gradient->setStops(stops);
    } else {
        gradient->setStops(defaultStops);
    }
}

SvgStyles SvgStyleParser::parseOneCssStyle(const QString &style, const QStringList &interestingAttributes)
{
    SvgStyles parsedStyles;
    if (style.isEmpty()) return parsedStyles;

    QStringList substyles = style.simplified().split(';', QString::SkipEmptyParts);
    if (!substyles.count()) return parsedStyles;

    for (QStringList::Iterator it = substyles.begin(); it != substyles.end(); ++it) {
        QStringList substyle = it->split(':');
        if (substyle.count() != 2)
            continue;
        QString command = substyle[0].trimmed();
        QString params  = substyle[1].trimmed();

        if (interestingAttributes.isEmpty() || interestingAttributes.contains(command)) {
            parsedStyles[command] = params;
        }
    }

    return parsedStyles;
}

SvgStyles SvgStyleParser::collectStyles(const KoXmlElement &e)
{
    SvgStyles styleMap;

    // collect individual presentation style attributes which have the priority 0
    // according to SVG standard
    // NOTE: font attributes should be parsed the first, because they defines 'em' and 'ex'
    Q_FOREACH (const QString & command, d->fontAttributes) {
        const QString attribute = e.attribute(command);
        if (!attribute.isEmpty())
            styleMap[command] = attribute;
    }
    Q_FOREACH (const QString &command, d->styleAttributes) {
        const QString attribute = e.attribute(command);
        if (!attribute.isEmpty())
            styleMap[command] = attribute;
    }
    Q_FOREACH (const QString & command, d->textAttributes) {
        const QString attribute = e.attribute(command);
        if (!attribute.isEmpty())
            styleMap[command] = attribute;
    }

    // match css style rules to element
    QStringList cssStyles = d->context.matchingCssStyles(e);

    // collect all css style attributes
    Q_FOREACH (const QString &style, cssStyles) {
        QStringList substyles = style.split(';', QString::SkipEmptyParts);
        if (!substyles.count())
            continue;
        for (QStringList::Iterator it = substyles.begin(); it != substyles.end(); ++it) {
            QStringList substyle = it->split(':');
            if (substyle.count() != 2)
                continue;
            QString command = substyle[0].trimmed();
            QString params  = substyle[1].trimmed();

            // toggle the namespace selector into the xml-like one
            command.replace("|", ":");

            // only use style and font attributes
            if (d->styleAttributes.contains(command) ||
                d->fontAttributes.contains(command) ||
                d->textAttributes.contains(command)) {

                styleMap[command] = params;
            }
        }
    }

    // FIXME: if 'inherit' we should just remove the property and use the one from the context!

    // replace keyword "inherit" for style values
    QMutableMapIterator<QString, QString> it(styleMap);
    while (it.hasNext()) {
        it.next();
        if (it.value() == "inherit") {
            it.setValue(inheritedAttribute(it.key(), e));
        }
    }

    return styleMap;
}

SvgStyles SvgStyleParser::mergeStyles(const SvgStyles &referencedBy, const SvgStyles &referencedStyles)
{
    // 1. use all styles of the referencing styles
    SvgStyles mergedStyles = referencedBy;
    // 2. use all styles of the referenced style which are not in the referencing styles
    SvgStyles::const_iterator it = referencedStyles.constBegin();
    for (; it != referencedStyles.constEnd(); ++it) {
        if (!referencedBy.contains(it.key())) {
            mergedStyles.insert(it.key(), it.value());
        }
    }
    return mergedStyles;
}

SvgStyles SvgStyleParser::mergeStyles(const KoXmlElement &e1, const KoXmlElement &e2)
{
    return mergeStyles(collectStyles(e1), collectStyles(e2));
}

QString SvgStyleParser::inheritedAttribute(const QString &attributeName, const KoXmlElement &e)
{
    KoXmlNode parent = e.parentNode();
    while (!parent.isNull()) {
        KoXmlElement currentElement = parent.toElement();
        if (currentElement.hasAttribute(attributeName)) {
            return currentElement.attribute(attributeName);
        }
        parent = currentElement.parentNode();
    }
    return QString();
}
