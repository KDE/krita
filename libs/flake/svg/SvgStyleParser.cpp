/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2002-2005, 2007 Rob Buis <buis@kde.org>
 * SPDX-FileCopyrightText: 2002-2004 Nicolas Goutte <nicolasg@snafu.de>
 * SPDX-FileCopyrightText: 2005-2006 Tim Beaulen <tbscope@gmail.com>
 * SPDX-FileCopyrightText: 2005-2009 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2005, 2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2006-2007 Inge Wallin <inge@lysator.liu.se>
 * SPDX-FileCopyrightText: 2007-2008, 2010 Thorsten Zachmann <zachmann@kde.org>

 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "SvgStyleParser.h"
#include "SvgLoadingContext.h"
#include "SvgGraphicContext.h"
#include "SvgUtil.h"

#include "kis_dom_utils.h"

#include <text/KoSvgText.h>
#include <text/KoSvgTextProperties.h>

#include <QStringList>
#include <QColor>
#include <QGradientStops>
#include <KoColor.h>

class Q_DECL_HIDDEN SvgStyleParser::Private
{
public:
    Private(SvgLoadingContext &loadingContext)
        : context(loadingContext)
    {
        textAttributes << KoSvgTextProperties::supportedXmlAttributes();
        textAttributes.removeAll("xml:space");

        // the order of the font attributes is important, don't change without reason !!!
        fontAttributes << "font-family"
                       << "font-size"
                       << "font-weight"
                       << "font-style"
                       << "font-variant-caps"
                       << "font-variant-alternates"
                       << "font-variant-ligatures"
                       << "font-variant-numeric"
                       << "font-variant-east-asian"
                       << "font-variant-position"
                       << "font-variant"
                       << "font-feature-settings"
                       << "font-stretch"
                       << "font-size-adjust"
                       << "font" // why are we doing this after the rest?
                       << "font-optical-sizing"
                       << "font-variation-settings"
                       << "font-synthesis-weight"
                       << "font-synthesis-style"
                       << "font-synthesis-small-caps"
                       << "font-synthesis-position"
                       << "font-synthesis"
                       << "text-decoration"
                       << "text-decoration-line"
                       << "text-decoration-style"
                       << "text-decoration-color"
                       << "text-decoration-position"
                       << "font-kerning"
                       << "letter-spacing"
                       << "word-spacing"
                       << "baseline-shift"
                       << "vertical-align"
                       << "line-height"
                       << "xml:space"
                       << "white-space"
                       << "text-transform"
                       << "text-indent"
                       << "word-break"
                       << "line-break"
                       << "hanging-punctuation"
                       << "text-align"
                       << "text-align-all"
                       << "text-align-last"
                       << "inline-size"
                       << "overflow"
                       << "text-overflow"
                       << "tab-size"
                       << "overflow-wrap"
                       << "word-wrap"
                       << "text-orientation";
        // the order of the style attributes is important, don't change without reason !!!
        styleAttributes << "color" << "display" << "visibility";
        styleAttributes << "fill" << "fill-rule" << "fill-opacity";
        styleAttributes << "stroke" << "stroke-width" << "stroke-linejoin" << "stroke-linecap";
        styleAttributes << "stroke-dasharray" << "stroke-dashoffset" << "stroke-opacity" << "stroke-miterlimit";
        styleAttributes << "opacity" << "paint-order" << "filter" << "clip-path" << "clip-rule" << "mask";
        styleAttributes << "shape-inside" << "shape-subtract" << "shape-padding" << "shape-margin";
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

void SvgStyleParser::parseStyle(const SvgStyles &styles, const bool inheritByDefault)
{
    SvgGraphicsContext *gc = d->context.currentGC();
    if (!gc) return;

    if (inheritByDefault) {
        gc->fillType = SvgGraphicsContext::Inherit;
        gc->strokeType = SvgGraphicsContext::Inherit;
    }

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
        gc->stroke->setLineWidth(SvgUtil::parseUnitXY(gc, d->context.resolvedProperties(), params));
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
                array.append(SvgUtil::parseUnitXY(gc, d->context.resolvedProperties(), *it));
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
    } else if (command == "paint-order") {
        gc->paintOrder = params;
    } else if (command == "font-family") {
        gc->textProperties.parseSvgTextAttribute(d->context, command, params);
    } else if (command == "font-size") {
        gc->textProperties.parseSvgTextAttribute(d->context, command, params);
    } else if (command == "font-style") {
        gc->textProperties.parseSvgTextAttribute(d->context, command, params);

    } else if (command == "font-variant" || command == "font-variant-caps" || command == "font-variant-alternates" || command == "font-variant-ligatures"
               || command == "font-variant-numeric" || command == "font-variant-east-asian" || command == "font-variant-position") {
        gc->textProperties.parseSvgTextAttribute(d->context, command, params);

    } else if (command == "font-feature-settings") {
        gc->textProperties.parseSvgTextAttribute(d->context, command, params);
    } else if (command == "font-stretch") {
        gc->textProperties.parseSvgTextAttribute(d->context, command, params);
    } else if (command == "font-weight") {
        gc->textProperties.parseSvgTextAttribute(d->context, command, params);
    } else if (command == "font-variation-settings") {
        gc->textProperties.parseSvgTextAttribute(d->context, command, params);
    } else if (command == "font-optical-sizing") {
        gc->textProperties.parseSvgTextAttribute(d->context, command, params);
    } else if (command == "font-size-adjust") {
        gc->textProperties.parseSvgTextAttribute(d->context, command, params);
    } else if (command == "font-synthesis"
               || command == "font-synthesis-weight" || command == "font-synthesis-style"
               || command == "font-synthesis-small-caps" || command == "font-synthesis-position") {
        gc->textProperties.parseSvgTextAttribute(d->context, command, params);
    } else if (command == "font") {
        qWarning() << "Krita does not support the 'font' shorthand";
    } else if (command == "text-decoration" || command == "text-decoration-line" || command == "text-decoration-style" || command == "text-decoration-color"
               || command == "text-decoration-position") {
        gc->textProperties.parseSvgTextAttribute(d->context, command, params);

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
    } else if (command == "shape-inside") {
        gc->shapeInsideValue = params;
    } else if (command == "shape-subtract") {
        gc->shapeSubtractValue = params;
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
    } else if (command == "font-kerning" || command == "line-height" || command == "white-space" || command == "xml:space" || command == "text-transform" || command == "text-indent"
               || command == "word-break" || command == "line-break" || command == "hanging-punctuation" || command == "text-align"
               || command == "text-align-all" || command == "text-align-last" || command == "inline-size" || command == "overflow" || command == "text-overflow"
               || command == "tab-size" || command == "overflow-wrap" || command == "word-wrap" || command == "vertical-align"
               || command ==  "shape-padding" || command ==   "shape-margin" || command == "text-orientation" || command == "text-rendering") {
        gc->textProperties.parseSvgTextAttribute(d->context, command, params);
    } else if (command == "krita:marker-fill-method") {
        gc->autoFillMarkers = params == "auto";
    } else if (d->textAttributes.contains(command)) {
        gc->textProperties.parseSvgTextAttribute(d->context, command, params);
    }

    gc->fillColor = fillcolor;
    gc->stroke->setColor(strokecolor);
}

bool SvgStyleParser::parseColor(QColor &color, const QString &s)
{
    if (s.isEmpty() || s == "none")
        return false;

    KoColor current = KoColor();
    current.fromQColor(d->context.currentGC()->currentColor);
    KoColor c = KoColor::fromSVG11(s, d->context.profiles(), current);
    c.toQColor(&color);

    return true;
}

QPair<qreal, QColor> SvgStyleParser::parseColorStop(const QDomElement& stop,
                                    SvgGraphicsContext *context,
                                    qreal& previousOffset)
{
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
        color.setAlphaF(qBound(0.0, KisDomUtils::toDouble(stopOpacityStr), 1.0));
    }
    return QPair<qreal, QColor>(offset, color);
}

#define forEachElement( elem, parent ) \
    for ( QDomNode _node = parent.firstChild(); !_node.isNull(); _node = _node.nextSibling() ) \
    if ( ( elem = _node.toElement() ).isNull() ) {} else

void SvgStyleParser::parseColorStops(QGradient *gradient,
                                     const QDomElement &e,
                                     SvgGraphicsContext *context,
                                     const QGradientStops &defaultStops)
{
    QGradientStops stops;

    qreal previousOffset = 0.0;

    QDomElement stop;
    forEachElement(stop, e) {
        if (stop.tagName() == "stop") {
            stops.append(parseColorStop(stop, context, previousOffset));
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

    QStringList substyles = style.simplified().split(';', Qt::SkipEmptyParts);
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

SvgStyles SvgStyleParser::collectStyles(const QDomElement &e)
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
        QStringList substyles = style.split(';', Qt::SkipEmptyParts);
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

SvgStyles SvgStyleParser::mergeStyles(const QDomElement &e1, const QDomElement &e2)
{
    return mergeStyles(collectStyles(e1), collectStyles(e2));
}

QString SvgStyleParser::inheritedAttribute(const QString &attributeName, const QDomElement &e)
{
    QDomNode parent = e.parentNode();
    while (!parent.isNull()) {
        QDomElement currentElement = parent.toElement();
        if (currentElement.hasAttribute(attributeName)) {
            return currentElement.attribute(attributeName);
        }
        parent = currentElement.parentNode();
    }
    return QString();
}
