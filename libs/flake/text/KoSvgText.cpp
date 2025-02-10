/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSvgText.h"

#include <QDebug>
#include <array>

#include <kis_dom_utils.h>

#include <KoColorBackground.h>
#include <KoGradientBackground.h>
#include <KoVectorPatternBackground.h>
#include <KoShapeStroke.h>

#include <SvgLoadingContext.h>
#include <SvgUtil.h>

#include <KisStaticInitializer.h>

KIS_DECLARE_STATIC_INITIALIZER {
    qRegisterMetaType<KoSvgText::CssLengthPercentage>("KoSvgText::CssLengthPercentage");
    qRegisterMetaType<KoSvgText::AutoValue>("KoSvgText::AutoValue");
    qRegisterMetaType<KoSvgText::AutoLengthPercentage>("KoSvgText::AutoLengthPercentage");
    qRegisterMetaType<KoSvgText::StrokeProperty>("KoSvgText::StrokeProperty");
    qRegisterMetaType<KoSvgText::TextTransformInfo>("KoSvgText::TextTransformInfo");
    qRegisterMetaType<KoSvgText::TextIndentInfo>("KoSvgText::TextIndentInfo");
    qRegisterMetaType<KoSvgText::TabSizeInfo>("KoSvgText::TabSizeInfo");
    qRegisterMetaType<KoSvgText::LineHeightInfo>("KoSvgText::LineHeightInfo");
    qRegisterMetaType<KoSvgText::FontFamilyAxis>("KoSvgText::FontFamilyAxis");
    qRegisterMetaType<KoSvgText::FontFamilyStyleInfo>("KoSvgText::FontFamilyStyleInfo");
    qRegisterMetaType<KoSvgText::CssFontStyleData>("KoSvgText::CssSlantData");
    qRegisterMetaType<KoSvgText::BackgroundProperty>("KoSvgText::BackgroundProperty");
    qRegisterMetaType<KoSvgText::FontFeatureLigatures>("KoSvgText::FontFeatureLigatures");
    qRegisterMetaType<KoSvgText::FontFeatureNumeric>("KoSvgText::FontFeatureNumeric");
    qRegisterMetaType<KoSvgText::FontFeatureEastAsian>("KoSvgText::FontFeatureEastAsian");

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    qRegisterMetaTypeStreamOperators<KoSvgText::FontFamilyAxis>("KoSvgText::FontFamilyAxis");
    qRegisterMetaTypeStreamOperators<KoSvgText::FontFamilyStyleInfo>("KoSvgText::FontFamilyStyleInfo");

    QMetaType::registerEqualsComparator<KoSvgText::CssLengthPercentage>();
    QMetaType::registerDebugStreamOperator<KoSvgText::CssLengthPercentage>();

    QMetaType::registerEqualsComparator<KoSvgText::AutoValue>();
    QMetaType::registerDebugStreamOperator<KoSvgText::AutoValue>();


    QMetaType::registerEqualsComparator<KoSvgText::AutoLengthPercentage>();
    QMetaType::registerDebugStreamOperator<KoSvgText::AutoLengthPercentage>();

    QMetaType::registerEqualsComparator<KoSvgText::CssFontStyleData>();
    QMetaType::registerDebugStreamOperator<KoSvgText::CssFontStyleData>();
    
    QMetaType::registerEqualsComparator<KoSvgText::BackgroundProperty>();
    QMetaType::registerDebugStreamOperator<KoSvgText::BackgroundProperty>();


    QMetaType::registerEqualsComparator<KoSvgText::StrokeProperty>();
    QMetaType::registerDebugStreamOperator<KoSvgText::StrokeProperty>();


    QMetaType::registerEqualsComparator<KoSvgText::TextTransformInfo>();
    QMetaType::registerDebugStreamOperator<KoSvgText::TextTransformInfo>();


    QMetaType::registerEqualsComparator<KoSvgText::TextIndentInfo>();
    QMetaType::registerDebugStreamOperator<KoSvgText::TextIndentInfo>();


    QMetaType::registerEqualsComparator<KoSvgText::TabSizeInfo>();
    QMetaType::registerDebugStreamOperator<KoSvgText::TabSizeInfo>();

    QMetaType::registerEqualsComparator<KoSvgText::LineHeightInfo>();
    QMetaType::registerDebugStreamOperator<KoSvgText::LineHeightInfo>();
    
    QMetaType::registerEqualsComparator<KoSvgText::FontFamilyAxis>();
    QMetaType::registerDebugStreamOperator<KoSvgText::FontFamilyAxis>();
    
    QMetaType::registerEqualsComparator<KoSvgText::FontFamilyStyleInfo>();
    QMetaType::registerDebugStreamOperator<KoSvgText::FontFamilyStyleInfo>();

    QMetaType::registerEqualsComparator<KoSvgText::FontFeatureLigatures>();
    QMetaType::registerDebugStreamOperator<KoSvgText::FontFeatureLigatures>();

    QMetaType::registerEqualsComparator<KoSvgText::FontFeatureNumeric>();
    QMetaType::registerDebugStreamOperator<KoSvgText::FontFeatureNumeric>();

    QMetaType::registerEqualsComparator<KoSvgText::FontFeatureEastAsian>();
    QMetaType::registerDebugStreamOperator<KoSvgText::FontFeatureEastAsian>();
#endif
}

namespace KoSvgText {

AutoValue parseAutoValueX(const QString &value, const SvgLoadingContext &context, const QString &autoKeyword)
{
    return value == autoKeyword ? AutoValue() : SvgUtil::parseUnitX(context.currentGC(), context.resolvedProperties(), value);
}

AutoValue parseAutoValueY(const QString &value, const SvgLoadingContext &context, const QString &autoKeyword)
{
    return value == autoKeyword ? AutoValue() : SvgUtil::parseUnitY(context.currentGC(), context.resolvedProperties(), value);
}

AutoValue parseAutoValueXY(const QString &value, const SvgLoadingContext &context, const QString &autoKeyword)
{
    return value == autoKeyword ? AutoValue() : SvgUtil::parseUnitXY(context.currentGC(), context.resolvedProperties(), value);
}

AutoValue parseAutoValueAngular(const QString &value, const SvgLoadingContext &context, const QString &autoKeyword)
{
    return value == autoKeyword ? AutoValue() : SvgUtil::parseUnitAngular(context.currentGC(), value);
}

WritingMode parseWritingMode(const QString &value) {
    return (value == "tb-rl" || value == "tb" || value == "vertical-rl") ? VerticalRL : (value == "vertical-lr") ? VerticalLR : HorizontalTB;
}

Direction parseDirection(const QString &value) {
    return value == "rtl" ? DirectionRightToLeft : DirectionLeftToRight;
}

UnicodeBidi parseUnicodeBidi(const QString &value)
{
    return value == "embed"           ? BidiEmbed
        : value == "bidi-override"    ? BidiOverride
        : value == "isolate"          ? BidiIsolate
        : value == "isolate-override" ? BidiIsolateOverride
        : value == "plaintext"        ? BidiPlainText
                                      : BidiNormal;
}

TextOrientation parseTextOrientation(const QString &value)
{
    return value == "upright" ? OrientationUpright : value == "sideways" ? OrientationSideWays : OrientationMixed;
}
TextOrientation parseTextOrientationFromGlyphOrientation(AutoValue value)
{
    if (value.isAuto) {
        return OrientationMixed;
    } else if (value.customValue == 0) {
        return OrientationUpright;
    } else if (value.customValue == 90) {
        return OrientationSideWays;
    } else {
        return OrientationMixed;
    }
}

TextAnchor parseTextAnchor(const QString &value)
{
    return value == "middle" ? AnchorMiddle :
           value == "end" ? AnchorEnd :
           AnchorStart;
}

Baseline parseBaseline(const QString &value)
{
    return value == "use-script"                                                          ? BaselineUseScript
        : value == "no-change"                                                            ? BaselineNoChange
        : value == "reset-size"                                                           ? BaselineResetSize
        : value == "ideographic"                                                          ? BaselineIdeographic
        : value == "alphabetic"                                                           ? BaselineAlphabetic
        : value == "hanging"                                                              ? BaselineHanging
        : value == "mathematical"                                                         ? BaselineMathematical
        : value == "central"                                                              ? BaselineCentral
        : value == "middle"                                                               ? BaselineMiddle
        : value == "baseline"                                                             ? BaselineDominant
        : (value == "text-after-edge" || value == "after-edge" || value == "text-bottom") ? BaselineTextBottom
        : (value == "text-before-edge" || value == "before-edge" || value == "text-top")  ? BaselineTextTop
                                                                                          : BaselineAuto;
}

BaselineShiftMode parseBaselineShiftMode(const QString &value)
{
    return value == "baseline" ? ShiftNone :
           value == "sub" ? ShiftSub :
           value == "super" ? ShiftSuper :
           ShiftLengthPercentage;
}

LengthAdjust parseLengthAdjust(const QString &value)
{
    return value == "spacingAndGlyphs" ? LengthAdjustSpacingAndGlyphs : LengthAdjustSpacing;
}

QString writeAutoValue(const AutoValue &value, const QString &autoKeyword)
{
    return value.isAuto ? autoKeyword : KisDomUtils::toString(value.customValue);
}

QString writeWritingMode(WritingMode value, bool svg1_1)
{
    if (svg1_1) {
        return value == VerticalRL ? "tb" : "lr";
    } else {
        return value == VerticalRL ? "vertical-rl" : value == VerticalLR ? "vertical-lr" : "horizontal-tb";
    }
}

QString writeDirection(Direction value)
{
    return value == DirectionRightToLeft ? "rtl" : "ltr";
}

QString writeUnicodeBidi(UnicodeBidi value)
{
    return value == BidiEmbed          ? "embed"
        : value == BidiOverride        ? "bidi-override"
        : value == BidiIsolate         ? "isolate"
        : value == BidiIsolateOverride ? "isolate-override"
        : value == BidiPlainText       ? "plaintext"
                                       : "normal";
}

QString writeTextOrientation(TextOrientation orientation)
{
    return orientation == OrientationUpright ? "upright" : orientation == OrientationSideWays ? "sideways" : "mixed";
}

QString writeTextAnchor(TextAnchor value)
{
    return value == AnchorEnd ? "end" : value == AnchorMiddle ? "middle" : "start";
}

QString writeDominantBaseline(Baseline value)
{
    return value == BaselineUseScript   ? "use-script"
        : value == BaselineNoChange     ? "no-change"
        : value == BaselineResetSize    ? "reset-size"
        : value == BaselineIdeographic  ? "ideographic"
        : value == BaselineAlphabetic   ? "alphabetic"
        : value == BaselineHanging      ? "hanging"
        : value == BaselineMathematical ? "mathematical"
        : value == BaselineCentral      ? "central"
        : value == BaselineMiddle       ? "middle"
        : value == BaselineTextBottom   ? "text-bottom"
                                        : // text-after-edge in svg 1.1
        value == BaselineTextTop ? "text-top"
                                   : // text-before-edge in svg 1.1
        "auto";
}

QString writeAlignmentBaseline(Baseline value)
{
    return value == BaselineDominant    ? "baseline"
        : value == BaselineIdeographic  ? "ideographic"
        : value == BaselineAlphabetic   ? "alphabetic"
        : value == BaselineHanging      ? "hanging"
        : value == BaselineMathematical ? "mathematical"
        : value == BaselineCentral      ? "central"
        : value == BaselineMiddle       ? "middle"
        : value == BaselineTextBottom   ? "text-bottom"
                                        : // text-after-edge in svg 1.1
        value == BaselineTextTop ? "text-top"
                                   : // text-before-edge in svg 1.1
        "auto";
}

QString writeBaselineShiftMode(BaselineShiftMode value, CssLengthPercentage shift)
{
    return value == ShiftNone ? "baseline" :
           value == ShiftSub ? "sub" :
           value == ShiftSuper ? "super" :
           writeLengthPercentage(shift);
}

QString writeLengthAdjust(LengthAdjust value)
{
    return value == LengthAdjustSpacingAndGlyphs ? "spacingAndGlyphs" : "spacing";
}

QDebug operator<<(QDebug dbg, const KoSvgText::AutoValue &value)
{
    dbg.nospace() << (value.isAuto ? "auto" : QString::number(value.customValue));
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const KoSvgText::CssFontStyleData &value)
{
    if (value.style == QFont::StyleOblique) {
        dbg.nospace() << "oblique ";
        dbg.nospace() << value.slantValue;
    } else {
        dbg.nospace() << (value.style == QFont::StyleItalic? "italic": "roman");
    }


    return dbg.space();
}

void CharTransformation::mergeInParentTransformation(const CharTransformation &t)
{
    if (!xPos && t.xPos) {
        xPos = *t.xPos;
    }

    if (!yPos && t.yPos) {
        yPos = *t.yPos;
    }

    if (!dxPos && t.dxPos) {
        dxPos = *t.dxPos;
    }

    if (!dyPos && t.dyPos) {
        dyPos = *t.dyPos;
    }

    if (!rotate && t.rotate) {
        rotate = *t.rotate;
    }
}

bool CharTransformation::isNull() const
{
    return !xPos && !yPos && !dxPos && !dyPos && !rotate;
}

bool CharTransformation::startsNewChunk() const
{
    return xPos || yPos;
}

bool CharTransformation::hasRelativeOffset() const
{
    return dxPos || dyPos;
}

QPointF CharTransformation::absolutePos() const
{
    QPointF result;

    if (xPos) {
        result.rx() = *xPos;
    }

    if (yPos) {
        result.ry() = *yPos;
    }

    return result;
}

QPointF CharTransformation::relativeOffset() const
{
    QPointF result;

    if (dxPos) {
        result.rx() = *dxPos;
    }

    if (dyPos) {
        result.ry() = *dyPos;
    }

    return result;
}

bool CharTransformation::operator==(const CharTransformation &other) const {
    return
        xPos == other.xPos && yPos == other.yPos &&
        dxPos == other.dxPos && dyPos == other.dyPos &&
            rotate == other.rotate;
}

namespace {
QDebug addSeparator(QDebug dbg, bool hasPreviousContent) {
    return hasPreviousContent ? (dbg.nospace() << "; ") : dbg;
}
}

QDebug operator<<(QDebug dbg, const CharTransformation &t)
{
    dbg.nospace() << "CharTransformation(";

    bool hasContent = false;

    if (t.xPos) {
        dbg.nospace() << "xPos = " << *t.xPos;
        hasContent = true;
    }

    if (t.yPos) {
        dbg = addSeparator(dbg, hasContent);
        dbg.nospace() << "yPos = " << *t.yPos;
        hasContent = true;
    }

    if (t.dxPos) {
        dbg = addSeparator(dbg, hasContent);
        dbg.nospace() << "dxPos = " << *t.dxPos;
        hasContent = true;
    }

    if (t.dyPos) {
        dbg = addSeparator(dbg, hasContent);
        dbg.nospace() << "dyPos = " << *t.dyPos;
        hasContent = true;
    }

    if (t.rotate) {
        dbg = addSeparator(dbg, hasContent);
        dbg.nospace() << "rotate = " << *t.rotate;
        hasContent = true;
    }

    dbg.nospace() << ")";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const TextTransformInfo &t)
{
    dbg.nospace() << "TextTransformInfo(";
    dbg.nospace() << writeTextTransform(t);
    dbg.nospace() << ")";
    return dbg.space();
}
QDebug KRITAFLAKE_EXPORT operator<<(QDebug dbg, const KoSvgText::TextIndentInfo &value)
{
    dbg.nospace() << "TextIndentInfo(";
    dbg.nospace() << writeTextIndent(value);
    dbg.nospace() << ")";
    return dbg.space();
}

QDebug KRITAFLAKE_EXPORT operator<<(QDebug dbg, const KoSvgText::TabSizeInfo &value)
{
    dbg.nospace() << "TextIndentInfo(";
    dbg.nospace() << writeTabSize(value);
    if (value.isNumber) {
        dbg.nospace() << "x Spaces";
    }
    dbg.nospace() << ")";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const BackgroundProperty &prop)
{
    dbg.nospace() << "BackgroundProperty(";

    dbg.nospace() << prop.property.data();

    if (KoColorBackground *fill = dynamic_cast<KoColorBackground*>(prop.property.data())) {
        dbg.nospace() << ", color, " << fill->color();
    }

    if (KoGradientBackground *fill = dynamic_cast<KoGradientBackground*>(prop.property.data())) {
        dbg.nospace() << ", gradient, " << fill->gradient();
    }

    if (KoVectorPatternBackground *fill = dynamic_cast<KoVectorPatternBackground*>(prop.property.data())) {
        dbg.nospace() << ", pattern, num shapes: " << fill->shapes().size();
    }

    dbg.nospace() << ")";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const StrokeProperty &prop)
{
    dbg.nospace() << "StrokeProperty(";

    dbg.nospace() << prop.property.data();

    if (KoShapeStroke *stroke = dynamic_cast<KoShapeStroke*>(prop.property.data())) {
        dbg.nospace() << ", " << stroke->resultLinePen();
    }

    dbg.nospace() << ")";
    return dbg.space();
}

TextPathMethod parseTextPathMethod(const QString &value)
{
    return value == "stretch" ? TextPathStretch : TextPathAlign;
}

TextPathSpacing parseTextPathSpacing(const QString &value)
{
    return value == "auto" ? TextPathAuto : TextPathExact;
}

TextPathSide parseTextPathSide(const QString &value)
{
    return value == "left" ? TextPathSideLeft : TextPathSideRight;
}

QString writeTextPathMethod(TextPathMethod value)
{
    return value == TextPathAlign ? "align" : "stretch";
}

QString writeTextPathSpacing(TextPathSpacing value)
{
    return value == TextPathAuto ? "auto" : "exact";
}

QString writeTextPathSide(TextPathSide value)
{
    return value == TextPathSideLeft ? "left" : "right";
}

bool whiteSpaceValueToLongHands(const QString &value, TextSpaceCollapse &collapseMethod, TextWrap &wrapMethod, TextSpaceTrims &trimMethod)
{
    bool result = true;
    if (value == "pre") {
        collapseMethod = Preserve;
        wrapMethod = NoWrap;
        trimMethod = TrimNone;
    } else if (value == "nowrap") {
        collapseMethod = Collapse;
        wrapMethod = NoWrap;
        trimMethod = TrimNone;
    } else if (value == "pre-wrap") {
        collapseMethod = Preserve;
        wrapMethod = Wrap;
        trimMethod = TrimNone;
    } else if (value == "pre-line" || value == "break-spaces") {
        if (value == "break-spaces") {
            result = false;
        }
        collapseMethod = PreserveBreaks;
        wrapMethod = Wrap;
        trimMethod = TrimNone;
    } else { // "normal"
        if (value != "normal") {
            result = false;
        }
        collapseMethod = Collapse;
        wrapMethod = Wrap;
        trimMethod = TrimNone;
    }
    return result;
}

bool xmlSpaceToLongHands(const QString &value, TextSpaceCollapse &collapseMethod)
{
    bool result = true;

    if (value == "preserve") {
        /*
         * "When xml:space="preserve", the SVG user agent will do the following
         * using a copy of the original character data content. It will convert
         * all newline and tab characters into space characters. Then, it will
         * draw all space characters, including leading, trailing and multiple
         * contiguous space characters."
         */
        collapseMethod = PreserveSpaces;
    } else {
        /*
         * "When xml:space="default", the SVG user agent will do the following
         * using a copy of the original character data content. First, it will
         * remove all newline characters. Then it will convert all tab
         * characters into space characters. Then, it will strip off all leading
         * and trailing space characters. Then, all contiguous space characters
         * will be consolidated."
         */
        if (value != "default") {
            result = false;
        }
        collapseMethod = Collapse;
    }

    return result;
}

QString writeWhiteSpaceValue(TextSpaceCollapse collapseMethod, TextWrap wrapMethod, TextSpaceTrims trimMethod)
{
    Q_UNUSED(trimMethod);
    if (wrapMethod != NoWrap) {
        if (collapseMethod == Preserve) {
            return "pre-wrap";
        } else if (collapseMethod == PreserveBreaks) {
            return "pre-line";
        } else {
            return "normal";
        }

    } else {
        if (collapseMethod == Preserve) {
            return "pre";
        } else {
            return "nowrap";
        }
    }
}

QString writeXmlSpace(TextSpaceCollapse collapseMethod)
{
    return collapseMethod == PreserveSpaces ? "preserve" : "default";
}

WordBreak parseWordBreak(const QString &value)
{
    return value == "keep-all" ? WordBreakKeepAll : value == "break-all" ? WordBreakBreakAll : WordBreakNormal;
}

LineBreak parseLineBreak(const QString &value)
{
    return value == "loose"   ? LineBreakLoose
        : value == "normal"   ? LineBreakNormal
        : value == "strict"   ? LineBreakStrict
        : value == "anywhere" ? LineBreakAnywhere
                              : LineBreakAuto;
}

TextAlign parseTextAlign(const QString &value)
{
    return value == "end"         ? AlignEnd
        : value == "left"         ? AlignLeft
        : value == "right"        ? AlignRight
        : value == "center"       ? AlignCenter
        : value == "justify"      ? AlignJustify
        : value == "justify-all"  ? AlignJustify
        : value == "match-parent" ? AlignMatchParent
        : value == "auto"         ? AlignLastAuto
                                  : // only for text-align-last
        AlignStart;
}

QString writeWordBreak(WordBreak value)
{
    return value == WordBreakKeepAll ? "keep-all" : value == WordBreakBreakAll ? "break-all" : "normal";
}

QString writeLineBreak(LineBreak value)
{
    return value == LineBreakLoose   ? "loose"
        : value == LineBreakNormal   ? "normal"
        : value == LineBreakStrict   ? "strict"
        : value == LineBreakAnywhere ? "anywhere"
                                     : "auto";
}

QString writeTextAlign(TextAlign value)
{
    return value == AlignEnd        ? "end"
        : value == AlignLeft        ? "left"
        : value == AlignRight       ? "right"
        : value == AlignCenter      ? "center"
        : value == AlignJustify     ? "justify"
        : value == AlignMatchParent ? "match-parent"
        : value == AlignLastAuto    ? "auto"
                                    : // only for text-align-last
        "start";
}

TextTransformInfo parseTextTransform(const QString &value)
{
    TextTransformInfo textTransform;
    const QStringList values = value.toLower().split(" ");
    Q_FOREACH (const QString &param, values) {
        if (param == "capitalize") {
            textTransform.capitals = TextTransformCapitalize;
        } else if (param == "uppercase") {
            textTransform.capitals = TextTransformUppercase;
        } else if (param == "lowercase") {
            textTransform.capitals = TextTransformLowercase;
        } else if (param == "full-width") {
            textTransform.fullWidth = true;
        } else if (param == "full-size-kana") {
            textTransform.fullSizeKana = true;
        } else if (param == "none") {
            textTransform.capitals = TextTransformNone;
            textTransform.fullWidth = false;
            textTransform.fullSizeKana = false;
        } else {
            qWarning() << "Unknown parameter in text-transform" << param;
        }
    }
    return textTransform;
}

QString writeTextTransform(const TextTransformInfo textTransform)
{
    QStringList values;
    if (textTransform.capitals == TextTransformNone && !textTransform.fullWidth && !textTransform.fullSizeKana) {
        values.append("none");
    } else {
        if (textTransform.capitals == TextTransformLowercase) {
            values.append("lowercase");
        } else if (textTransform.capitals == TextTransformUppercase) {
            values.append("uppercase");
        } else if (textTransform.capitals == TextTransformCapitalize) {
            values.append("capitalize");
        }
        if (textTransform.fullWidth) {
            values.append("full-width");
        }
        if (textTransform.fullSizeKana) {
            values.append("full-size-kana");
        }
    }
    return values.join(" ");
}

TextIndentInfo parseTextIndent(const QString &value, const SvgLoadingContext &context)
{
    const QStringList values = value.toLower().split(" ");
    TextIndentInfo textIndent;
    Q_FOREACH (const QString &param, values) {
        if (param == "hanging") {
            textIndent.hanging = true;
        } else if (param == "each-line") {
            textIndent.eachLine = true;
        } else {
            textIndent.length = SvgUtil::parseTextUnitStruct(context.currentGC(), param);
            //ToDo: figure out how to detect value is number.
            //qWarning() << "Unknown parameter in text-indent" << param;
        }
    }
    return textIndent;
}

QString writeTextIndent(const TextIndentInfo textIndent)
{
    QStringList values;
    values.append(writeLengthPercentage(textIndent.length));
    if (textIndent.hanging) {
        values.append("hanging");
    }
    if (textIndent.eachLine) {
        values.append("each-line");
    }
    return values.join(" ");
}

TabSizeInfo parseTabSize(const QString &value, const SvgLoadingContext &context)
{
    TabSizeInfo tabSizeInfo;
    tabSizeInfo.value = KisDomUtils::toDouble(value, &tabSizeInfo.isNumber);
    if (!tabSizeInfo.isNumber) {
        tabSizeInfo.length = SvgUtil::parseTextUnitStruct(context.currentGC(), value);
    }
    return tabSizeInfo;
}

QString writeTabSize(const TabSizeInfo tabSize)
{
    QString val = KisDomUtils::toString(tabSize.value);
    if (!tabSize.isNumber) {

        // Tabsize does not support percentage, so if we accidentally set it somewhere, convert to em.
        val = writeLengthPercentage(tabSize.length, true);
        if (tabSize.length == CssLengthPercentage::Absolute) {
            val += "px"; // In SVG, due to browsers, the default unit is css px. Krita scales these to pt.
        }
    }
    return val;
}

int parseCSSFontStretch(const QString &value, int currentStretch)
{
    int newStretch = 100;

    static constexpr std::array<int, 9> fontStretches = {50, 62, 75, 87, 100, 112, 125, 150, 200};

    if (value == "wider") {
        const auto it = std::upper_bound(fontStretches.begin(), fontStretches.end(), currentStretch);

        newStretch = it != fontStretches.end() ? *it : fontStretches.back();
    } else if (value == "narrower") {
        const auto it =
            std::upper_bound(fontStretches.rbegin(), fontStretches.rend(), currentStretch, std::greater<int>());

        newStretch = it != fontStretches.rend() ? *it : fontStretches.front();
    } else {
        // try to read numerical stretch value
        bool ok = false;
        newStretch = value.toInt(&ok, 10);

        if (!ok) {
            auto it = std::find(fontStretchNames.begin(), fontStretchNames.end(), value);
            if (it != fontStretchNames.end()) {
                const auto index = std::distance(fontStretchNames.begin(), it);
                KIS_ASSERT(index >= 0);
                newStretch = fontStretches.at(static_cast<size_t>(index));
            }
        }
    }
    return newStretch;
}

int parseCSSFontWeight(const QString &value, int currentWeight)
{
    int weight = 400;

    // map svg weight to qt weight
    // svg value        qt value
    // 100,200,300      1, 17, 33
    // 400              50          (normal)
    // 500,600          58,66
    // 700              75          (bold)
    // 800,900          87,99
    static constexpr std::array<int, 9> svgFontWeights = {100, 200, 300, 400, 500, 600, 700, 800, 900};

    if (value == "bold")
        weight = 700;
    else if (value == "bolder") {
        const auto it = std::upper_bound(svgFontWeights.begin(), svgFontWeights.end(), currentWeight);

        weight = it != svgFontWeights.end() ? *it : svgFontWeights.back();
    } else if (value == "lighter") {
        const auto it =
            std::upper_bound(svgFontWeights.rbegin(), svgFontWeights.rend(), currentWeight, std::greater<int>());

        weight = it != svgFontWeights.rend() ? *it : svgFontWeights.front();
    } else {
        bool ok = false;

        // try to read numerical weight value
        const int parsed = value.toInt(&ok, 10);
        if (ok) {
            weight = qBound(0, parsed, 1000);
        }
    }
    return weight;
}

LineHeightInfo parseLineHeight(const QString &value, const SvgLoadingContext &context)
{
    LineHeightInfo lineHeight;
    lineHeight.isNormal = value == "normal";
    qreal parsed = value.toDouble(&lineHeight.isNumber);

    if (lineHeight.isNumber) {
        lineHeight.value = parsed;
    } else {
        lineHeight.length = SvgUtil::parseTextUnitStruct(context.currentGC(), value);
    }

    // Negative line-height is invalid
    if (!lineHeight.isNormal && (lineHeight.value < 0 || lineHeight.length.value < 0)) {
        lineHeight.isNormal = true;
        lineHeight.isNumber = false;
        lineHeight.value = 0;
        lineHeight.length.value = 0;
    }

    return lineHeight;
}

QString writeLineHeight(LineHeightInfo lineHeight)
{
    if (lineHeight.isNormal) return QString("normal");
    QString val = KisDomUtils::toString(lineHeight.value);
    if (!lineHeight.isNumber) {
        val = writeLengthPercentage(lineHeight.length);
        if (lineHeight.length.unit == CssLengthPercentage::Absolute) {
            val += "px"; // In SVG, due to browsers, the default unit is css px. Krita scales these to pt.
        }
    }
    return val;
}

QDebug operator<<(QDebug dbg, const LineHeightInfo &value)
{
    dbg.nospace() << "LineHeightInfo(";

    if (value.isNormal) {
        dbg.nospace() << "normal";
    } else if (!value.isNumber) {
        dbg.nospace() << value.value << "pt";
    } else {
        dbg.nospace() << value.value;
    }

    dbg.nospace() << ")";
    return dbg.space();
}

QDebug operator<<(QDebug dbg, const CssLengthPercentage &value)
{
    dbg.nospace() << "Length(";

    if (value.unit == CssLengthPercentage::Percentage) {
        dbg.nospace() << value.value << "%";
    } else if (value.unit == CssLengthPercentage::Em) {
        dbg.nospace() << value.value << "em";
    } else if (value.unit == CssLengthPercentage::Ex) {
        dbg.nospace() << value.value << "ex";
    } else {
        dbg.nospace() << value.value << "(pt)";
    }

    dbg.nospace() << ")";
    return dbg.space();
}

QString writeLengthPercentage(const CssLengthPercentage &length, bool percentageAsEm)
{
    QString val;
    if (length.unit == CssLengthPercentage::Percentage && !percentageAsEm) {
        val = KisDomUtils::toString(length.value*100.0) + "%";
    } else {
        val = KisDomUtils::toString(length.value);
        if (length.unit == CssLengthPercentage::Em || length.unit == CssLengthPercentage::Percentage) {
            val += "em";
        } else if (length.unit == CssLengthPercentage::Ex) {
            val += "ex";
        }
    }
    return val;
}

void CssLengthPercentage::convertToAbsolute(const qreal fontSizeInPt, const qreal fontXHeightInPt, const CssLengthPercentage::UnitType percentageUnit) {
    UnitType u = unit;
    if (u == Percentage) {
        u = percentageUnit;
    }
    if (u == Em) {
        value = value * fontSizeInPt;
    } else if (u == Ex) {
        value = value * fontXHeightInPt;
    }
    unit = Absolute;
}

AutoLengthPercentage parseAutoLengthPercentageXY(const QString &value, const SvgLoadingContext &context, const QString &autoKeyword, QRectF bbox, bool percentageIsViewPort)
{
    return value == autoKeyword ? AutoLengthPercentage()
                                : percentageIsViewPort? AutoLengthPercentage(SvgUtil::parseUnitStruct(context.currentGC(), value, true, true, bbox))
                                                      : AutoLengthPercentage(SvgUtil::parseTextUnitStruct(context.currentGC(), value));
}

QString writeAutoLengthPercentage(const AutoLengthPercentage &value, const QString &autoKeyword, bool percentageToEm)
{
    return value.isAuto ? autoKeyword : writeLengthPercentage(value.length, percentageToEm);
}

QDebug operator<<(QDebug dbg, const KoSvgText::AutoLengthPercentage &value)
{
    if (value.isAuto) {
        dbg.nospace() << "auto";
    } else {
        dbg.nospace() << value.length;
    }
    return dbg.space();
}


QDebug operator<<(QDebug dbg, const KoSvgText::FontFamilyAxis &axis)
{
    dbg.nospace() << axis.debugInfo();
    return dbg.space();
}

QDataStream &operator<<(QDataStream &out, const KoSvgText::FontFamilyAxis &axis) {

    QDomDocument doc;
    QDomElement root = doc.createElement("axis");
    root.setAttribute("tagName", axis.tag);
    root.setAttribute("min", axis.min);
    root.setAttribute("max", axis.max);
    root.setAttribute("default", axis.defaultValue);
    root.setAttribute("hidden", axis.axisHidden? "true": "false");
    root.setAttribute("variable", axis.variableAxis? "true": "false");
    for(auto it = axis.localizedLabels.begin(); it != axis.localizedLabels.end(); it++) {
        QDomElement name = doc.createElement("name");
        name.setAttribute("lang", it.key().bcp47Name());
        name.setAttribute("value", it.value());
        root.appendChild(name);
    }
    doc.appendChild(root);
    out << doc.toString(0);
    return out;
}
QDataStream &operator>>(QDataStream &in, KoSvgText::FontFamilyAxis &axis) {

    QString xml;
    in >> xml;

    QDomDocument doc;
    doc.setContent(xml);
    QDomElement root = doc.childNodes().at(0).toElement();
    axis.tag = root.attribute("tagName");
    axis.min = root.attribute("min").toDouble();
    axis.max = root.attribute("max").toDouble();
    axis.defaultValue = root.attribute("default").toDouble();
    axis.axisHidden = root.attribute("hidden") == "true"? true: false;
    axis.variableAxis = root.attribute("variable") == "true"? true: false;
    QDomNodeList names =  root.elementsByTagName("name");
    for(int i = 0; i < names.size(); i++) {
        QDomElement name = names.at(i).toElement();
        QString lang = name.attribute("lang");
        QString value = name.attribute("value");
        axis.localizedLabels.insert(QLocale(lang), value);
    }

    return in;
}

QDebug operator<<(QDebug dbg, const KoSvgText::FontFamilyStyleInfo &style)
{
    dbg.nospace() << style.debugInfo();
    return dbg.space();
}

QDataStream &operator<<(QDataStream &out, const KoSvgText::FontFamilyStyleInfo &style) {

    QDomDocument doc;
    QDomElement root = doc.createElement("style");
    root.setAttribute("italic", style.isItalic? "true": "false");
    root.setAttribute("oblique", style.isOblique? "true": "false");
    for(auto it = style.instanceCoords.begin(); it != style.instanceCoords.end(); it++) {
        QDomElement coord = doc.createElement("coord");
        coord.setAttribute("tag", it.key());
        coord.setAttribute("value", it.value());
        root.appendChild(coord);
    }
    for(auto it = style.localizedLabels.begin(); it != style.localizedLabels.end(); it++) {
        QDomElement name = doc.createElement("name");
        name.setAttribute("lang", it.key().bcp47Name());
        name.setAttribute("value", it.value());
        root.appendChild(name);
    }
    doc.appendChild(root);
    out << doc.toString(0);
    return out;
}
QDataStream &operator>>(QDataStream &in, KoSvgText::FontFamilyStyleInfo &style) {
    QString xml;
    in >> xml;

    QDomDocument doc;
    doc.setContent(xml);
    QDomElement root = doc.childNodes().at(0).toElement();
    style.isItalic = root.attribute("italic") == "true"? true: false;
    style.isOblique = root.attribute("oblique") == "true"? true: false;
    QDomNodeList names =  root.elementsByTagName("name");
    for(int i = 0; i < names.size(); i++) {
        QDomElement name = names.at(i).toElement();
        QString lang = name.attribute("lang");
        QString value = name.attribute("value");
        style.localizedLabels.insert(QLocale(lang), value);
    }
    QDomNodeList coords =  root.elementsByTagName("coord");
    for(int i = 0; i < coords.size(); i++) {
        QDomElement coord = coords.at(i).toElement();
        QString tag = coord.attribute("tag");
        double value = coord.attribute("value").toDouble();
        style.instanceCoords.insert(tag, value);
    }

    return in;
}

CssFontStyleData parseFontStyle(const QString &value)
{
    CssFontStyleData slant;
    QStringList params = value.split(" ");
    if (!params.isEmpty()) {
        QString style = params.first();
        slant.style = style == "italic"? QFont::StyleItalic: style == "oblique"? QFont::StyleOblique: QFont::StyleNormal;
    }
    if (params.size() > 1) {
        QString angle = params.last();
        if (angle.endsWith("deg")) {
            angle.chop(3);
            slant.slantValue.isAuto = false;
            slant.slantValue.customValue = angle.toDouble();
        }
    }
    return slant;
}

QString writeFontStyle(CssFontStyleData value)
{
    QString style =
        value.style == QFont::StyleItalic ? "italic" :
        value.style == QFont::StyleOblique ? "oblique" : "normal";
    if (value.style == QFont::StyleOblique && !value.slantValue.isAuto) {
        style.append(QString(" ")+QString::number(value.slantValue.customValue)+QString("deg"));
    }
    return style;
}

FontFeatureLigatures parseFontFeatureLigatures(const QString &value, FontFeatureLigatures features)
{
    if (value == "common-ligatures") {
        features.commonLigatures = true;
    } else if (value == "no-common-ligatures") {
        features.commonLigatures = false;
    } else if (value == "discretionary-ligatures") {
        features.discretionaryLigatures = true;
    } else if (value == "no-discretionary-ligatures") {
        features.discretionaryLigatures = false;
    } else if (value == "historical-ligatures") {
        features.historicalLigatures = true;
    } else if (value == "no-historical-ligatures") {
        features.historicalLigatures = false;
    } else if (value == "contextual") {
        features.contextualAlternates = true;
    } else if (value == "no-contextual") {
        features.contextualAlternates = false;
    } else if (value == "none") {
        features.commonLigatures = false;
        features.discretionaryLigatures = false;
        features.historicalLigatures = false;
        features.contextualAlternates = false;
    }
    return features;
}

QString writeFontFeatureLigatures(const FontFeatureLigatures &feature)
{
    if (feature.commonLigatures && !feature.discretionaryLigatures
            && !feature.historicalLigatures && feature.commonLigatures) {
        return "normal";
    }
    if (!feature.commonLigatures && !feature.discretionaryLigatures
            && !feature.historicalLigatures && !feature.commonLigatures) {
        return "none";
    }
    QStringList list;
    if (!feature.commonLigatures) {
        list << "no-common-ligatures";
    }
    if (feature.discretionaryLigatures) {
        list << "discretionary-ligatures";
    }
    if (feature.historicalLigatures) {
        list << "historical-ligatures";
    }
    if (!feature.contextualAlternates) {
        list << "no-contextual";
    }
    return list.join(" ");
}

QDebug operator<<(QDebug dbg, const KoSvgText::FontFeatureLigatures &feature)
{
    dbg.nospace() << "Ligatures("<< writeFontFeatureLigatures(feature) <<")";
    return dbg.space();
}

FontFeatureNumeric parseFontFeatureNumeric(const QString &value, FontFeatureNumeric features)
{
    if (value == "lining-nums") {
        features.style = NumericFigureStyleLining;
    } else if (value == "oldstyle-nums") {
        features.style = NumericFigureStyleOld;
    } else if (value == "proportional-nums") {
        features.spacing = NumericFigureSpacingProportional;
    } else if (value == "tabular-nums") {
        features.spacing = NumericFigureSpacingTabular;
    } else if (value == "diagonal-fractions") {
        features.fractions = NumericFractionsDiagonal;
    } else if (value == "stacked-fractions") {
        features.fractions = NumericFractionsStacked;
    } else if (value == "ordinal") {
        features.ordinals = true;
    } else if (value == "slashed-zero") {
        features.slashedZero = true;
    } else {
        features = FontFeatureNumeric();
    }
    return features;
}

QString writeFontFeatureNumeric(const FontFeatureNumeric &feature)
{
    if (feature == FontFeatureNumeric()) {
        return "normal";
    }
    QStringList list;

    if (feature.style == NumericFigureStyleLining) {
        list << "lining-nums";
    } else if (feature.style == NumericFigureStyleOld) {
        list << "oldstyle-nums";
    }

    if (feature.spacing == NumericFigureSpacingProportional) {
        list << "proportional-nums";
    } else if (feature.spacing == NumericFigureSpacingTabular) {
        list << "tabular-nums";
    }

    if (feature.fractions == NumericFractionsDiagonal) {
        list << "diagonal-fractions";
    } else if (feature.fractions == NumericFractionsStacked) {
        list << "stacked-fractions";
    }

    if (feature.ordinals) {
        list << "ordinal";
    }

    if (feature.slashedZero) {
        list << "slashed-zero";
    }

    return list.join(" ");
}

QDebug operator<<(QDebug dbg, const KoSvgText::FontFeatureNumeric &feature)
{
    dbg.nospace() << "NumericFeatures("<< writeFontFeatureNumeric(feature) <<")";
    return dbg.space();
}

FontFeatureEastAsian parseFontFeatureEastAsian(const QString &value, FontFeatureEastAsian features)
{
    if (value == "jis78") {
        features.variant = EastAsianJis78;
    } else if (value == "jis83") {
        features.variant = EastAsianJis83;
    } else if (value == "jis90") {
        features.variant = EastAsianJis90;
    } else if (value == "jis04") {
        features.variant = EastAsianJis04;
    } else if (value == "simplified") {
        features.variant = EastAsianSimplified;
    } else if (value == "traditional") {
        features.variant = EastAsianTraditional;
    } else if (value == "full-width") {
        features.width = EastAsianFullWidth;
    } else if (value == "proportional-width") {
        features.width = EastAsianProportionalWidth;
    } else if (value == "ruby") {
        features.ruby = true;
    } else {
        features = FontFeatureEastAsian();
    }
    return features;
}

QString writeFontFeatureEastAsian(const FontFeatureEastAsian &feature)
{
    if (feature == FontFeatureEastAsian()) {
        return "normal";
    }
    QStringList list;

    if (feature.variant == EastAsianJis78) {
        list << "jis78";
    } else if (feature.variant == EastAsianJis83) {
        list << "jis83";
    } else if (feature.variant == EastAsianJis90) {
        list << "jis90";
    } else if (feature.variant == EastAsianJis04) {
        list << "jis04";
    } else if (feature.variant == EastAsianSimplified) {
        list << "simplified";
    } else if (feature.variant == EastAsianTraditional) {
        list << "traditional";
    }

    if (feature.width == EastAsianFullWidth) {
        list << "full-width";
    } else if (feature.width == EastAsianProportionalWidth) {
        list << "proportional-width";
    }

    if (feature.ruby) {
        list << "ruby";
    }

    return list.join(" ");
}

QDebug operator<<(QDebug dbg, const KoSvgText::FontFeatureEastAsian &feature)
{
    dbg.nospace() << "EastAsianFeatures("<< writeFontFeatureEastAsian(feature) <<")";
    return dbg.space();
}

FontFeaturePosition parseFontFeaturePosition(const QString &value, FontFeaturePosition feature)
{
    return value == "super"? PositionSuper : value == "sub"? PositionSub : value == "normal"? PositionNormal: feature;
}

QString writeFontFeaturePosition(const FontFeaturePosition &value)
{
    return value == PositionSuper ? "super"
                                  : value == PositionSub? "sub" : "normal";
}

FontFeatureCaps parseFontFeatureCaps(const QString &value, FontFeatureCaps feature)
{
    return value == "small-caps"          ? CapsSmall
        : value == "all-small-caps"        ? CapsAllSmall
        : value == "petite-caps"         ? CapsPetite
        : value == "all-petite-caps" ? CapsAllPetite
        : value == "unicase"       ? CapsUnicase
        : value == "titling-caps"       ? CapsTitling
        : value == "normal"   ? CapsNormal: feature;
}

QString writeFontFeatureCaps(const FontFeatureCaps &value)
{
    return value == CapsSmall          ? "small-caps"
        : value == CapsAllSmall        ? "all-small-caps"
        : value == CapsPetite         ? "petite-caps"
        : value == CapsAllPetite ? "all-petite-caps"
        : value == CapsUnicase       ? "unicase"
        : value == CapsTitling       ? "titling-caps"
                                     : "normal";
}

QStringList fontFeaturesPosition(const FontFeaturePosition &feature, const int start, const int end)
{
    const QString length = QString("[%1:%2]").arg(start).arg(end);
    QString tag = feature == PositionSuper? "sups" : feature == PositionSub? "subs": QString();
    if (!tag.isEmpty()) {
        tag += length;
        tag += "=1";
    }
    return tag.isEmpty()? QStringList(): QStringList(tag);
}

QStringList fontFeaturesCaps(const FontFeatureCaps &feature, const int start, const int end)
{
    QStringList list;
    const QString length = QString("[%1:%2]").arg(start).arg(end);

    switch (feature) {
    case CapsSmall:
        list << "smcp" + length + "=1";
        break;
    case CapsAllSmall:
        list << "smcp" + length + "=1";
        list << "c2sc" + length + "=1";
        break;
    case CapsPetite:
        list << "pcap" + length + "=1";
        break;
    case CapsAllPetite:
        list << "pcap" + length + "=1";
        list << "c2pc" + length + "=1";
        break;
    case CapsUnicase:
        list << "unic" + length + "=1";
        break;
    case CapsTitling:
        list << "titl" + length + "=1";
        break;
    default:
        break;
    }

    return list;
}

} // namespace KoSvgText
