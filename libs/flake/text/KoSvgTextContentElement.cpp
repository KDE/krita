/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSvgTextContentElement.h"

#include "KoCssTextUtils.h"
#include <kis_dom_utils.h>
#include "SvgUtil.h"
#include "KoXmlWriter.h"
#include "SvgStyleWriter.h"
#include <kis_global.h>

#include "SvgGraphicContext.h"

#include <QRegularExpression>

KoSvgTextContentElement::KoSvgTextContentElement()
{

}

namespace {
void appendLazy(QVector<qreal> *list, boost::optional<qreal> value, int iteration, bool hasDefault = true, qreal defaultValue = 0.0)
{
    if (!value) return;
    if (value && *value == defaultValue && hasDefault == true && list->isEmpty()) return;

    while (list->size() < iteration) {
        list->append(defaultValue);
    }

    list->append(*value);
}

void fillTransforms(QVector<qreal> *xPos, QVector<qreal> *yPos, QVector<qreal> *dxPos, QVector<qreal> *dyPos, QVector<qreal> *rotate,
                    QVector<KoSvgText::CharTransformation> localTransformations)
{
    for (int i = 0; i < localTransformations.size(); i++) {
        const KoSvgText::CharTransformation &t = localTransformations[i];
        appendLazy(xPos, t.xPos, i, false);
        appendLazy(yPos, t.yPos, i, false);
        appendLazy(dxPos, t.dxPos, i);
        appendLazy(dyPos, t.dyPos, i);
        appendLazy(rotate, t.rotate, i);
    }
}



QVector<qreal> parseListAttributeX(const QString &value, SvgLoadingContext &context)
{
    QVector<qreal> result;

    QStringList list = SvgUtil::simplifyList(value);
    Q_FOREACH (const QString &str, list) {
        result << SvgUtil::parseUnitX(context.currentGC(), context.resolvedProperties(), str);
    }

    return result;
}

QVector<qreal> parseListAttributeY(const QString &value, SvgLoadingContext &context)
{
    QVector<qreal> result;

    QStringList list = SvgUtil::simplifyList(value);
    Q_FOREACH (const QString &str, list) {
        result << SvgUtil::parseUnitY(context.currentGC(), context.resolvedProperties(), str);
    }

    return result;
}

QVector<qreal> parseListAttributeAngular(const QString &value, SvgLoadingContext &context)
{
    QVector<qreal> result;

    QStringList list = SvgUtil::simplifyList(value);
    Q_FOREACH (const QString &str, list) {
        result << SvgUtil::parseUnitAngular(context.currentGC(), str);
    }

    return result;
}

QString convertListAttribute(const QVector<qreal> &values) {
    QStringList stringValues;

    Q_FOREACH (qreal value, values) {
        stringValues.append(KisDomUtils::toString(value));
    }

    return stringValues.join(',');
}

void writeTextListAttribute(const QString &attribute, const QVector<qreal> &values, KoXmlWriter &writer)
{
    const QString value = convertListAttribute(values);
    if (!value.isEmpty()) {
        writer.addAttribute(attribute.toLatin1().data(), value);
    }
}
}

#include <ksharedconfig.h>
#include <kconfiggroup.h>

/**
 * HACK ALERT: this is a function from a private Qt's header qfont_p.h,
 * we don't include the whole header, because it is painful in the
 * environments we don't fully control, e.g. in distribution packages.
 */
Q_GUI_EXPORT int qt_defaultDpi();

namespace {
int forcedDpiForQtFontBugWorkaround() {
    KConfigGroup cfg(KSharedConfig::openConfig(), "");
    int value = cfg.readEntry("forcedDpiForQtFontBugWorkaround", qt_defaultDpi());

    if (value < 0) {
        value = qt_defaultDpi();
    }

    return value;
}


KoSvgTextProperties adjustPropertiesForFontSizeWorkaround(const KoSvgTextProperties &properties)
{
    if (!properties.hasProperty(KoSvgTextProperties::FontSizeId) || !properties.hasProperty(KoSvgTextProperties::FontSizeAdjustId))
        return properties;

    KoSvgTextProperties result = properties;

    const int forcedFontDPI = forcedDpiForQtFontBugWorkaround();

    if (result.hasProperty(KoSvgTextProperties::KraTextVersionId) &&
        result.property(KoSvgTextProperties::KraTextVersionId).toInt() < 2 &&
        forcedFontDPI > 0) {

        qreal fontSize = result.fontSize().value;
        fontSize *= qreal(forcedFontDPI) / 72.0;
        result.setFontSize(KoSvgText::CssLengthPercentage(fontSize));
    }
    if (result.hasProperty(KoSvgTextProperties::KraTextVersionId) && result.property(KoSvgTextProperties::KraTextVersionId).toInt() < 3
        && result.hasProperty(KoSvgTextProperties::FontSizeAdjustId)) {
        result.setProperty(KoSvgTextProperties::FontSizeAdjustId, KoSvgText::fromAutoValue(KoSvgText::AutoValue()));
    }

    result.setProperty(KoSvgTextProperties::KraTextVersionId, 3);

    return result;
}

}

const QString TEXT_STYLE_TYPE = "krita:style-type";
const QString TEXT_STYLE_RES = "krita:style-resolution";

bool KoSvgTextContentElement::loadSvg(const QDomElement &e, SvgLoadingContext &context, bool rootNode)
{
    SvgGraphicsContext *gc = context.currentGC();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(gc, false);

    KoSvgTextProperties props = rootNode? context.resolvedProperties(): gc->textProperties;


    QVector<KoSvgTextProperties::PropertyId> generic = {KoSvgTextProperties::FillId,
                                                        KoSvgTextProperties::StrokeId,
                                                        KoSvgTextProperties::PaintOrder,
                                                        KoSvgTextProperties::Opacity,
                                                        KoSvgTextProperties::Visibility};
    for (int i = 0; i < generic.size(); i++) {
        auto id = generic[i];
        if (properties.hasProperty(id)) {
            props.setProperty(id, properties.property(id));
        }
    }
    properties = props;

    textLength = KoSvgText::parseAutoValueXY(e.attribute("textLength", ""), context, "");
    lengthAdjust = KoSvgText::parseLengthAdjust(e.attribute("lengthAdjust", "spacing"));

    QVector<qreal> xPos = parseListAttributeX(e.attribute("x", ""), context);
    QVector<qreal> yPos = parseListAttributeY(e.attribute("y", ""), context);
    QVector<qreal> dxPos = parseListAttributeX(e.attribute("dx", ""), context);
    QVector<qreal> dyPos = parseListAttributeY(e.attribute("dy", ""), context);
    QVector<qreal> rotate = parseListAttributeAngular(e.attribute("rotate", ""), context);

    const int numLocalTransformations =
        std::max({xPos.size(), yPos.size(),
                  dxPos.size(), dyPos.size(),
                  rotate.size()});

    localTransformations.resize(numLocalTransformations);
    for (int i = 0; i < numLocalTransformations; i++) {
        if (i < xPos.size()) {
            localTransformations[i].xPos = xPos[i];
        }
        if (i < yPos.size()) {
            localTransformations[i].yPos = yPos[i];
        }
        if (i < dxPos.size() && dxPos[i] != 0.0) {
            localTransformations[i].dxPos = dxPos[i];
        }
        if (i < dyPos.size() && dyPos[i] != 0.0) {
            localTransformations[i].dyPos = dyPos[i];
        }
        if (i < rotate.size()) {
            localTransformations[i].rotate = rotate[i];
        }
    }

    if (e.tagName() == "textPath") {
        // we'll read the value 'path' later.

        textPathInfo.side = KoSvgText::parseTextPathSide(e.attribute("side", "left"));
        textPathInfo.method = KoSvgText::parseTextPathMethod(e.attribute("method", "align"));
        textPathInfo.spacing = KoSvgText::parseTextPathSpacing(e.attribute("spacing", "auto"));
        // This depends on pathLength;
        if (e.hasAttribute("startOffset")) {
            QString offset = e.attribute("startOffset", "0");
            if (offset.endsWith("%")) {
                textPathInfo.startOffset = SvgUtil::parseNumber(offset.left(offset.size() - 1));
                textPathInfo.startOffsetIsPercentage = true;
            } else {
                textPathInfo.startOffset = SvgUtil::parseUnit(context.currentGC(), context.resolvedProperties(), offset);
            }
        }
    }

    if (e.hasAttribute(TEXT_STYLE_TYPE.toLatin1().data())) {
        properties.setProperty(KoSvgTextProperties::KraTextStyleType, e.attribute(TEXT_STYLE_TYPE.toLatin1().data()));
        if (e.hasAttribute(TEXT_STYLE_RES.toLatin1().data())) {
            QString resolution = e.attribute(TEXT_STYLE_RES.toLatin1().data()).toLower();
            if (resolution.endsWith("dpi")) {
                resolution.chop(3);
            }
            properties.setProperty(KoSvgTextProperties::KraTextStyleResolution, KisDomUtils::toInt(resolution));
        }
    }

    return true;
}

bool KoSvgTextContentElement::loadSvgTextNode(const QDomText &text, SvgLoadingContext &context)
{
    SvgGraphicsContext *gc = context.currentGC();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(gc, false);

    // In theory, the XML spec requires XML parsers to normalize line endings to
    // LF. However, QXmlInputSource + QXmlSimpleReader do not do this, so we can
    // end up with CR in the text. The SVG spec explicitly calls out that all
    // newlines in SVG are to be represented by a single LF (U+000A) character,
    // so we can replace all CRLF and CR into LF here for simplicity.
    static const QRegularExpression s_regexCrlf(R"==((?:\r\n|\r(?!\n)))==");
    QString content = text.data();
    content.replace(s_regexCrlf, QStringLiteral("\n"));

    this->text = std::move(content);

    return true;
}

bool KoSvgTextContentElement::saveSvg(SvgSavingContext &context,
                                      bool rootText,
                                      bool saveText,
                                      QMap<QString, QString> shapeSpecificAttributes,
                                      KoShape *textPath)
{
    if (textPath) {
        if (textPath) {
            // we'll always save as an embedded shape as "path" is an svg 2.0
            // feature.
            QString id = textPath->isVisible(false)? context.getID(textPath): SvgStyleWriter::embedShape(textPath, context);
            // inkscape can only read 'xlink:href'
            if (!id.isEmpty()) {
                context.shapeWriter().addAttribute("xlink:href", "#" + id);
            }
        }
        if (textPathInfo.startOffset != 0) {
            QString offset = KisDomUtils::toString(textPathInfo.startOffset);
            if (textPathInfo.startOffsetIsPercentage) {
                offset += "%";
            }
            context.shapeWriter().addAttribute("startOffset", offset);
        }
        if (textPathInfo.method != KoSvgText::TextPathAlign) {
            context.shapeWriter().addAttribute("method", KoSvgText::writeTextPathMethod(textPathInfo.method));
        }
        if (textPathInfo.side != KoSvgText::TextPathSideLeft) {
            context.shapeWriter().addAttribute("side", KoSvgText::writeTextPathSide(textPathInfo.side));
        }
        if (textPathInfo.spacing != KoSvgText::TextPathAuto) {
            context.shapeWriter().addAttribute("spacing", KoSvgText::writeTextPathSpacing(textPathInfo.spacing));
        }
    }

    if (!localTransformations.isEmpty()) {

        QVector<qreal> xPos;
        QVector<qreal> yPos;
        QVector<qreal> dxPos;
        QVector<qreal> dyPos;
        QVector<qreal> rotate;

        fillTransforms(&xPos, &yPos, &dxPos, &dyPos, &rotate, localTransformations);

        for (int i = 0; i < rotate.size(); i++) {
            rotate[i] = kisRadiansToDegrees(rotate[i]);
        }

        writeTextListAttribute("x", xPos, context.shapeWriter());
        writeTextListAttribute("y", yPos, context.shapeWriter());
        writeTextListAttribute("dx", dxPos, context.shapeWriter());
        writeTextListAttribute("dy", dyPos, context.shapeWriter());
        writeTextListAttribute("rotate", rotate, context.shapeWriter());
    }

    if (!textLength.isAuto) {
        context.shapeWriter().addAttribute("textLength", KisDomUtils::toString(textLength.customValue));

        if (lengthAdjust == KoSvgText::LengthAdjustSpacingAndGlyphs) {
            context.shapeWriter().addAttribute("lengthAdjust", "spacingAndGlyphs");
        }
    }

    KoSvgTextProperties ownProperties = rootText? properties.ownProperties(KoSvgTextProperties::defaultProperties(), true): properties;

    ownProperties = adjustPropertiesForFontSizeWorkaround(ownProperties);

    // we write down stroke/fill if they are different from the parent's value
    if (!rootText) {
        if (ownProperties.hasProperty(KoSvgTextProperties::FillId)) {
            SvgStyleWriter::saveSvgFill(properties.background(),
                                        false,
                                        this->associatedOutline.boundingRect(),
                                        associatedOutline.boundingRect().size(),
                                        QTransform(),
                                        context);
        }

        if (ownProperties.hasProperty(KoSvgTextProperties::StrokeId)) {
            SvgStyleWriter::saveSvgStroke(properties.stroke(), context);
        }
    }

    QMap<QString, QString> attributes = ownProperties.convertToSvgTextAttributes();
    QStringList allowedAttributes = properties.supportedXmlAttributes();
    QString styleString;

    for (auto it = shapeSpecificAttributes.constBegin(); it != shapeSpecificAttributes.constEnd(); ++it) {
        styleString.append(it.key().toLatin1().data()).append(": ").append(it.value()).append(";");
    }
    for (auto it = attributes.constBegin(); it != attributes.constEnd(); ++it) {
        if (allowedAttributes.contains(it.key())) {
            context.shapeWriter().addAttribute(it.key().toLatin1().data(), it.value());
        } else {
            styleString.append(it.key().toLatin1().data()).append(": ").append(it.value()).append(";");
        }
    }
    if (!styleString.isEmpty()) {
        context.shapeWriter().addAttribute("style", styleString);
    }

    if (properties.hasProperty(KoSvgTextProperties::KraTextStyleType)) {
        context.shapeWriter().addAttribute(TEXT_STYLE_TYPE.toLatin1().data(), properties.property(KoSvgTextProperties::KraTextStyleType).toString());
        if (properties.hasProperty(KoSvgTextProperties::KraTextStyleResolution)) {
            context.shapeWriter().addAttribute(TEXT_STYLE_RES.toLatin1().data(), QString::number(properties.property(KoSvgTextProperties::KraTextStyleResolution).toInt())+"dpi");
        }
    }

    if (saveText) {
        context.shapeWriter().addTextNode(text);
    }
    return true;
}

static QString transformText(QString text, KoSvgText::TextTransformInfo textTransformInfo, const QString &lang, QVector<QPair<int, int>> &positions)
{
    if (textTransformInfo.capitals == KoSvgText::TextTransformCapitalize) {
        text = KoCssTextUtils::transformTextCapitalize(text, lang, positions);
    } else if (textTransformInfo.capitals == KoSvgText::TextTransformUppercase) {
        text = KoCssTextUtils::transformTextToUpperCase(text, lang, positions);
    } else if (textTransformInfo.capitals == KoSvgText::TextTransformLowercase) {
        text = KoCssTextUtils::transformTextToLowerCase(text, lang, positions);
    } else {
        positions.clear();
        for (int i = 0; i < text.size(); i++) {
            positions.append(QPair<int, int>(i, i));
        }
    }

    if (textTransformInfo.fullWidth) {
        text = KoCssTextUtils::transformTextFullWidth(text);
    }
    if (textTransformInfo.fullSizeKana) {
        text = KoCssTextUtils::transformTextFullSizeKana(text);
    }
    return text;
}

int KoSvgTextContentElement::numChars(bool withControls, KoSvgTextProperties resolvedProps) const
{
    int result = 0;
    if (withControls) {
        KoSvgText::UnicodeBidi bidi = KoSvgText::UnicodeBidi(resolvedProps.propertyOrDefault(KoSvgTextProperties::UnicodeBidiId).toInt());
        KoSvgText::Direction direction = KoSvgText::Direction(resolvedProps.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
        KoSvgText::TextTransformInfo textTransformInfo =
            resolvedProps.propertyOrDefault(KoSvgTextProperties::TextTransformId).value<KoSvgText::TextTransformInfo>();
        QString lang = resolvedProps.property(KoSvgTextProperties::TextLanguage).toString().toUtf8();
        QVector<QPair<int, int>> positions;

        result = KoCssTextUtils::getBidiOpening(direction == KoSvgText::DirectionLeftToRight, bidi).size();
        result += transformText(text, textTransformInfo, lang, positions).size();
        result += KoCssTextUtils::getBidiClosing(bidi).size();
    } else {
        result = text.size();
    }
    return result;
}

void KoSvgTextContentElement::insertText(int start, QString insertText)
{
    if (start >= text.size()) {
        text.append(insertText);
    } else {
        text.insert(start, insertText);
    }
}


QString KoSvgTextContentElement::getTransformedString(QVector<QPair<int, int> > &positions, KoSvgTextProperties resolvedProps) const
{
    KoSvgText::TextTransformInfo textTransformInfo =
        resolvedProps.propertyOrDefault(KoSvgTextProperties::TextTransformId).value<KoSvgText::TextTransformInfo>();
    QString lang = resolvedProps.property(KoSvgTextProperties::TextLanguage).toString().toUtf8();
    return transformText(text, textTransformInfo, lang, positions);
}

void KoSvgTextContentElement::removeText(int &start, int length)
{
    KoCssTextUtils::removeText(text, start, length);
}
