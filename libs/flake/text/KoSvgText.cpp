/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSvgText.h"
#include <SvgUtil.h>
#include <SvgLoadingContext.h>
#include <QDebug>
#include "kis_dom_utils.h"

#include <KoColorBackground.h>
#include <KoGradientBackground.h>
#include <KoVectorPatternBackground.h>
#include <KoShapeStroke.h>

#include <KoSvgTextChunkShape.h>
#include <KoSvgTextChunkShapeLayoutInterface.h>


namespace {

struct TextPropertiesStaticRegistrar {
    TextPropertiesStaticRegistrar() {
        qRegisterMetaType<KoSvgText::AutoValue>("KoSvgText::AutoValue");
        QMetaType::registerEqualsComparator<KoSvgText::AutoValue>();
        QMetaType::registerDebugStreamOperator<KoSvgText::AutoValue>();

        qRegisterMetaType<KoSvgText::BackgroundProperty>("KoSvgText::BackgroundProperty");
        QMetaType::registerEqualsComparator<KoSvgText::BackgroundProperty>();
        QMetaType::registerDebugStreamOperator<KoSvgText::BackgroundProperty>();

        qRegisterMetaType<KoSvgText::StrokeProperty>("KoSvgText::StrokeProperty");
        QMetaType::registerEqualsComparator<KoSvgText::StrokeProperty>();
        QMetaType::registerDebugStreamOperator<KoSvgText::StrokeProperty>();

        qRegisterMetaType<KoSvgText::AssociatedShapeWrapper>("KoSvgText::AssociatedShapeWrapper");
    }
};

static TextPropertiesStaticRegistrar textPropertiesStaticRegistrar;

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
    return (value == "tb-rl" || value == "tb" || value == "vertical-rl")
        ? VerticalRL
        : (value == "vertical-lr") ? VerticalLR
                                   : HorizontalTB;
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
    return value == "upright" ? OrientationUpright
        : value == "sideways" ? OrientationSideWays
                              : OrientationMixed;
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
    return value == "use-script"  ? BaselineUseScript
        : value == "no-change"    ? BaselineNoChange
        : value == "reset-size"   ? BaselineResetSize
        : value == "ideographic"  ? BaselineIdeographic
        : value == "alphabetic"   ? BaselineAlphabetic
        : value == "hanging"      ? BaselineHanging
        : value == "mathematical" ? BaselineMathematical
        : value == "central"      ? BaselineCentral
        : value == "middle"       ? BaselineMiddle
        : value == "baseline"     ? BaselineDominant
        : (value == "text-after-edge" || value == "after-edge"
           || value == "text-bottom")
        ? BaselineTextBottom
        : (value == "text-before-edge" || value == "before-edge"
           || value == "text-top")
        ? BaselineTextTop
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
        return value == VerticalRL ? "vertical-rl"
            : value == VerticalLR  ? "vertical-lr"
                                   : "horizontal-tb";
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
    return orientation == OrientationUpright ? "upright"
        : orientation == OrientationSideWays ? "sideways"
                                             : "mixed";
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
        value == BaselineTextTop ? "text-before-edge"
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
        value == BaselineTextTop ? "text-before-edge"
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
        return QStringList();
    }
}
}


