/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSvgTextContentElement.h"

#include "KoCssTextUtils.h"
#include <kis_dom_utils.h>
#include "SvgUtil.h"

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
        result << SvgUtil::parseUnitX(context.currentGC(), str);
    }

    return result;
}

QVector<qreal> parseListAttributeY(const QString &value, SvgLoadingContext &context)
{
    QVector<qreal> result;

    QStringList list = SvgUtil::simplifyList(value);
    Q_FOREACH (const QString &str, list) {
        result << SvgUtil::parseUnitY(context.currentGC(), str);
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
}

bool KoSvgTextContentElement::loadSvg(const QDomElement &e, SvgLoadingContext &context)
{
    SvgGraphicsContext *gc = context.currentGC();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(gc, false);

    properties = gc->textProperties;

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
                textPathInfo.startOffset = SvgUtil::parseUnit(context.currentGC(), offset);
            }
        }
    }

    return true;
}

bool KoSvgTextContentElement::loadSvgTextNode(const QDomText &text, SvgLoadingContext &context)
{
    SvgGraphicsContext *gc = context.currentGC();
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(gc, false);

    properties = gc->textProperties;

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

int KoSvgTextContentElement::numChars(bool withControls) const
{
    int result = 0;
    if (withControls) {
        KoSvgText::UnicodeBidi bidi = KoSvgText::UnicodeBidi(properties.propertyOrDefault(KoSvgTextProperties::UnicodeBidiId).toInt());
        KoSvgText::Direction direction = KoSvgText::Direction(properties.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
        KoSvgText::TextTransformInfo textTransformInfo =
            properties.propertyOrDefault(KoSvgTextProperties::TextTransformId).value<KoSvgText::TextTransformInfo>();
        QString lang = properties.property(KoSvgTextProperties::TextLanguage).toString().toUtf8();
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


QString KoSvgTextContentElement::getTransformedString(QVector<QPair<int, int> > positions) const
{
    KoSvgText::TextTransformInfo textTransformInfo =
        properties.propertyOrDefault(KoSvgTextProperties::TextTransformId).value<KoSvgText::TextTransformInfo>();
    QString lang = properties.property(KoSvgTextProperties::TextLanguage).toString().toUtf8();
    return transformText(text, textTransformInfo, lang, positions);
}

void KoSvgTextContentElement::removeText(int &start, int length)
{
    KoCssTextUtils::removeText(text, start, length);
}
