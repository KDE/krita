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

#include <KoSvgTextChunkShape.h>
#include <KoSvgTextChunkShapeLayoutInterface.h>
#include <KisStaticInitializer.h>

KIS_DECLARE_STATIC_INITIALIZER {
    qRegisterMetaType<KoSvgText::AutoValue>("KoSvgText::AutoValue");
    QMetaType::registerEqualsComparator<KoSvgText::AutoValue>();
    QMetaType::registerDebugStreamOperator<KoSvgText::AutoValue>();

    qRegisterMetaType<KoSvgText::BackgroundProperty>("KoSvgText::BackgroundProperty");
    QMetaType::registerEqualsComparator<KoSvgText::BackgroundProperty>();
    QMetaType::registerDebugStreamOperator<KoSvgText::BackgroundProperty>();

    qRegisterMetaType<KoSvgText::StrokeProperty>("KoSvgText::StrokeProperty");
    QMetaType::registerEqualsComparator<KoSvgText::StrokeProperty>();
    QMetaType::registerDebugStreamOperator<KoSvgText::StrokeProperty>();

    qRegisterMetaType<KoSvgText::TextTransformInfo>("KoSvgText::TextTransformInfo");
    QMetaType::registerEqualsComparator<KoSvgText::TextTransformInfo>();
    QMetaType::registerDebugStreamOperator<KoSvgText::TextTransformInfo>();

    qRegisterMetaType<KoSvgText::TextIndentInfo>("KoSvgText::TextIndentInfo");
    QMetaType::registerEqualsComparator<KoSvgText::TextIndentInfo>();
    QMetaType::registerDebugStreamOperator<KoSvgText::TextIndentInfo>();

    qRegisterMetaType<KoSvgText::TabSizeInfo>("KoSvgText::TabSizeInfo");
    QMetaType::registerEqualsComparator<KoSvgText::TabSizeInfo>();
    QMetaType::registerDebugStreamOperator<KoSvgText::TabSizeInfo>();

    qRegisterMetaType<KoSvgText::LineHeightInfo>("KoSvgText::LineHeightInfo");
    QMetaType::registerEqualsComparator<KoSvgText::LineHeightInfo>();
    QMetaType::registerDebugStreamOperator<KoSvgText::LineHeightInfo>();

    qRegisterMetaType<KoSvgText::AssociatedShapeWrapper>("KoSvgText::AssociatedShapeWrapper");
}

namespace KoSvgText {

AutoValue parseAutoValueX(const QString &value, const SvgLoadingContext &context, const QString &autoKeyword)
{
    return value == autoKeyword ? AutoValue() : SvgUtil::parseUnitX(context.currentGC(), value);
}

AutoValue parseAutoValueY(const QString &value, const SvgLoadingContext &context, const QString &autoKeyword)
{
    return value == autoKeyword ? AutoValue() : SvgUtil::parseUnitY(context.currentGC(), value);
}

AutoValue parseAutoValueXY(const QString &value, const SvgLoadingContext &context, const QString &autoKeyword)
{
    return value == autoKeyword ? AutoValue() : SvgUtil::parseUnitXY(context.currentGC(), value);
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
           ShiftPercentage;
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

QString writeBaselineShiftMode(BaselineShiftMode value, qreal portion)
{
    return value == ShiftNone ? "baseline" :
           value == ShiftSub ? "sub" :
           value == ShiftSuper ? "super" :
           SvgUtil::toPercentage(portion);
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

AssociatedShapeWrapper::AssociatedShapeWrapper()
{
}

AssociatedShapeWrapper::AssociatedShapeWrapper(KoSvgTextChunkShape *shape)
    : m_shape(shape)
{
    if (m_shape) {
        m_shape->addShapeChangeListener(this);
    }
}

AssociatedShapeWrapper::AssociatedShapeWrapper(const AssociatedShapeWrapper &rhs)
    : AssociatedShapeWrapper(rhs.m_shape)
{
}

AssociatedShapeWrapper &AssociatedShapeWrapper::operator=(const AssociatedShapeWrapper &rhs)
{
    if (m_shape) {
        m_shape->removeShapeChangeListener(this);
        m_shape = 0;
    }

    m_shape = rhs.m_shape;

    if (m_shape) {
        m_shape->addShapeChangeListener(this);
    }

    return *this;
}

bool AssociatedShapeWrapper::isValid() const
{
    return m_shape;
}

void AssociatedShapeWrapper::notifyShapeChanged(KoShape::ChangeType type, KoShape *shape)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(shape == m_shape);

    if (type == KoShape::Deleted) {
        m_shape = 0;
    }
}

void AssociatedShapeWrapper::addCharacterRect(const QRectF &rect)
{
    if (m_shape) {
        m_shape->layoutInterface()->addAssociatedOutline(rect);
    }
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

QMap<QString, FontVariantFeature> fontVariantStrings()
{
    QMap<QString, FontVariantFeature> features;
    features.insert("normal", FontVariantNormal);
    features.insert("none", FontVariantNone);

    features.insert("common-ligatures", CommonLigatures);
    features.insert("no-common-ligatures", NoCommonLigatures);
    features.insert("discretionary-ligatures", DiscretionaryLigatures);
    features.insert("no-discretionary-ligatures", NoDiscretionaryLigatures);
    features.insert("historical-ligatures", HistoricalLigatures);
    features.insert("no-historical-ligatures", NoHistoricalLigatures);
    features.insert("contextual", ContextualAlternates);
    features.insert("no-contextual", NoContextualAlternates);

    features.insert("sub", PositionSub);
    features.insert("super", PositionSuper);

    features.insert("small-caps", SmallCaps);
    features.insert("all-small-caps", AllSmallCaps);
    features.insert("petite-caps", PetiteCaps);
    features.insert("all-petite-caps", AllPetiteCaps);
    features.insert("unicase", Unicase);
    features.insert("titling-caps", TitlingCaps);

    features.insert("lining-nums", LiningNums);
    features.insert("oldstyle-nums", OldStyleNums);
    features.insert("proportional-nums", ProportionalNums);
    features.insert("tabular-nums", TabularNums);
    features.insert("diagonal-fractions", DiagonalFractions);
    features.insert("stacked-fractions", StackedFractions);
    features.insert("ordinal", Ordinal);
    features.insert("slashed-zero", SlashedZero);

    features.insert("historical-forms", HistoricalForms);
    features.insert("stylistic", StylisticAlt);
    features.insert("styleset", StyleSet);
    features.insert("character-variant", CharacterVariant);
    features.insert("swash", Swash);
    features.insert("ornaments", Ornaments);
    features.insert("annotation", Annotation);

    features.insert("jis78", EastAsianJis78);
    features.insert("jis83", EastAsianJis83);
    features.insert("jis90", EastAsianJis90);
    features.insert("jis04", EastAsianJis04);
    features.insert("simplified", EastAsianSimplified);
    features.insert("traditional", EastAsianTraditional);
    features.insert("full-width", EastAsianFullWidth);
    features.insert("proportional-width", EastAsianProportionalWidth);
    features.insert("ruby", EastAsianRuby);

    return features;
}

QStringList fontVariantOpentypeTags(FontVariantFeature feature)
{
    switch (feature) {
    case CommonLigatures:
    case NoCommonLigatures:
        return {"clig", "liga"};
    case DiscretionaryLigatures:
    case NoDiscretionaryLigatures:
        return {"dlig"};
    case HistoricalLigatures:
    case NoHistoricalLigatures:
        return {"hlig"};
    case ContextualAlternates:
    case NoContextualAlternates:
        return {"calt"};

    case PositionSub:
        return {"subs"};
    case PositionSuper:
        return {"sups"};

    case SmallCaps:
        return {"smcp"};
    case AllSmallCaps:
        return {"smcp", "c2sc"};
    case PetiteCaps:
        return {"pcap"};
    case AllPetiteCaps:
        return {"pcap", "c2pc"};
    case Unicase:
        return {"unic"};
    case TitlingCaps:
        return {"titl"};

    case LiningNums:
        return {"lnum"};
    case OldStyleNums:
        return {"onum"};
    case ProportionalNums:
        return {"pnum"};
    case TabularNums:
        return {"tnum"};
    case DiagonalFractions:
        return {"frac"};
    case StackedFractions:
        return {"afrc"};
    case Ordinal:
        return {"ordn"};
    case SlashedZero:
        return {"zero"};

    case HistoricalForms:
        return {"hist"};
    case StylisticAlt:
        return {"salt"};
    case StyleSet: // add 01 to 99 at the end
        return {"ss"};
    case CharacterVariant: // add 01 to 99 at the end
        return {"cv"};
    case Swash:
        return {"swsh", "cswh"};
    case Ornaments:
        return {"ornm"};
    case Annotation:
        return {"nalt"};

    case EastAsianJis78:
        return {"jp78"};
    case EastAsianJis83:
        return {"jp83"};
    case EastAsianJis90:
        return {"jp90"};
    case EastAsianJis04:
        return {"jp04"};
    case EastAsianSimplified:
        return {"smpl"};
    case EastAsianTraditional:
        return {"trad"};
    case EastAsianFullWidth:
        return {"fwid"};
    case EastAsianProportionalWidth:
        return {"pwid"};
    case EastAsianRuby:
        return {"ruby"};
    default:
        return {};
    }
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
        bool ok = false;
        qreal parsed = 0.0;
        if (param.endsWith("%")) {
            parsed = SvgUtil::fromPercentage(param, &ok);
            textIndent.isPercentage = true;
        } else {
            parsed = SvgUtil::parseUnitXY(context.currentGC(), param);
            ok = true;
        }

        if (param == "hanging") {
            textIndent.hanging = true;
        } else if (param == "each-line") {
            textIndent.eachLine = true;
        } else if (ok) {
            textIndent.value = parsed;
        } else {
            qWarning() << "Unknown parameter in text-indent" << param;
        }
    }
    return textIndent;
}

QString writeTextIndent(const TextIndentInfo textIndent)
{
    QStringList values;
    QString indentVal = QString::number(textIndent.value);
    if (textIndent.isPercentage) {
        indentVal += "%";
    }
    values.append(indentVal);
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
    bool ok = false;
    qreal parsed = KisDomUtils::toDouble(value, &ok);
    if (!ok) {
        parsed = SvgUtil::parseUnitXY(context.currentGC(), value);
    }
    tabSizeInfo.value = parsed;
    tabSizeInfo.isNumber = ok;
    return tabSizeInfo;
}

QString writeTabSize(const TabSizeInfo tabSize)
{
    QString val = KisDomUtils::toString(tabSize.value);
    if (!tabSize.isNumber) {
        val += "pt";
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
        if (value.endsWith("%")) {
            qreal percent = SvgUtil::fromPercentage(value);
            lineHeight.value = SvgUtil::parseUnitXY(context.currentGC(), QString::number(percent)+"em");
        } else {
            lineHeight.value = SvgUtil::parseUnitXY(context.currentGC(), value);
        }
    }

    // Negative line-height is invalid
    if (!lineHeight.isNormal && lineHeight.value < 0) {
        lineHeight.isNormal = true;
        lineHeight.isNumber = false;
        lineHeight.value = 0;
    }

    return lineHeight;
}

QString writeLineHeight(LineHeightInfo lineHeight)
{
    if (lineHeight.isNormal) return QString("normal");
    QString val = KisDomUtils::toString(lineHeight.value);
    if (!lineHeight.isNumber) {
        val += "pt";
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

} // namespace KoSvgText
