/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOSVGTEXT_H
#define KOSVGTEXT_H

#include <QFont>
#include <QList>
#include <QPainterPath>
#include <QTextCharFormat>
#include <QVariant>
#include <QVector>
#include <array>
#include <boost/operators.hpp>
#include <boost/optional.hpp>

#include <QSharedPointer>
#include <KoShapeBackground.h>
#include <KoShapeStrokeModel.h>

#include <QDomDocument>

#include <kritaflake_export.h>

class SvgLoadingContext;
class QDebug;

#include <KoShape.h>
class KoSvgTextChunkShape;

namespace KoSvgText
{
    Q_NAMESPACE_EXPORT(KRITAFLAKE_EXPORT)

enum WritingMode {
    HorizontalTB, ///< Left to right, lay out new lines bottom of previous. RTL
                  ///< scripts use this with BIDI reordering.
    VerticalRL, ///< Top to bottom, lay out new lines right of the previous.
                ///< used for CJK scripts.
    VerticalLR ///< Top to bottom, lay out new lines left of the previous. Used
               ///< for Mongolian.
};
Q_ENUM_NS(WritingMode)

/// Base direction used by Bidi algorithm.
enum Direction {
    DirectionLeftToRight,
    DirectionRightToLeft
};
Q_ENUM_NS(Direction)

/// These values control the type of bidi-controls we'll inject into the final
/// text.
enum UnicodeBidi {
    BidiNormal, ///< No new bidi-level is started.
    BidiEmbed, ///< Opens an additional Bidi-reordering level.
    BidiOverride, ///< Opens an additional Bidi-reordering level, implicit part
                  ///< of the algorithm is ignored.
    BidiIsolate, ///< Content is ordered as if in a seperate paragraph.
    BidiIsolateOverride, ///< Ordered like a directional override inside an
                         ///< isolated paragraph.
    BidiPlainText ///< Behaves like isolate, except using heuristics defined in
                  ///< P2 and P3 of the unicode bidirectional algorithm.
};
Q_ENUM_NS(UnicodeBidi)

/// Orientation of the glyphs, used for vertical writing modes.
enum TextOrientation {
    OrientationMixed, ///< Use UA50 to determine whether a character should be
                      ///< sideways.
    OrientationUpright, ///< Set all characters upright.
    OrientationSideWays ///< Set all characters sideways.
};
Q_ENUM_NS(TextOrientation)

/// Where the text is anchored for SVG 1.1 text and 'inline-size'.
enum TextAnchor {
    AnchorStart, ///< Anchor left for LTR, right for RTL.
    AnchorMiddle, ///< Anchor to the middle.
    AnchorEnd ///< Anchor right for LTR, left for RTL.
};
Q_ENUM_NS(TextAnchor)

/*
 * CSS-Text-3 defines the white space property, and SVG 2 adopts this, except,
 * CSS-Text-3 doesn't have a concept of 'xml:space="preserve"'. CSS-Text-4
 * *does*, however, that works by splitting the white-space property into the
 * the following three enums. Officially the SVG2 spec says to use 'white-space'
 * of CSS-Text-4 because of this.
 */

/// Part of "white-space", NOTE: white-space:break-spaces; is not really covered
/// by this new method yet.
enum TextSpaceCollapse {
    Collapse, ///< Collapse white space sequences into a single character.
    Discard, ///< Discard all Spaces
    Preserve, ///< Do not collapse any space
    PreserveBreaks, ///< Preserve segment breaks like /n, but otherwise collapse
                    ///< all whitespace.
    PreserveSpaces ///< Preserve spaces, convert tabs and linebreaks to spaces,
                   ///< required for 'xml:space="preserve"' emulation.
};

/// Part of "white-space", in practice we only support wrap and nowrap.
enum TextWrap {
    Wrap, ///< Do any kind of text wrapping at soft wrapping opportunities,
          ///< typically greedy.
    NoWrap, ///< Do not do any text wrapping.
    Balance, ///< Select algorithm that tries to ensure white space is balanced
             ///< instead of as much text as possible on each line.
    Stable, ///< Select algorithm that doesn't change the text before the cursor
            ///< when adjusting text.
    Pretty ///< select algorithm that gives the best looking result, may require
           ///< looking ahead.
};

/// Part of "white-space"
enum TextSpaceTrim {
    TrimNone = 0x0, ///< No trimming.
    TrimInner = 0x1, ///< Discard white space at the beginning and end of element.
    DiscardBefore = 0x2, ///< Trim white space before the start of the element.
    DiscardAfter = 0x4 ///< Trim white space after the end of the element.
};

/// Whether to break words.
enum WordBreak {
    WordBreakNormal, ///< Set according to script. Also "break-word"
    WordBreakKeepAll, ///< Never break inside words.
    WordBreakBreakAll, ///< Always break inside words.
};
Q_ENUM_NS(WordBreak)

/// Line breaking strictness. A number of these values are values to be handed
/// over to the line/word breaking algorithm.
enum LineBreak {
    LineBreakAuto, ///< Use preferred method.
    LineBreakLoose, ///< Use loose method, language specific.
    LineBreakNormal, ///< Use normal method, language specific.
    LineBreakStrict, ///< Use strict method, language specific.
    LineBreakAnywhere ///< Break between any typographic clusters.
};
Q_ENUM_NS(LineBreak)

/// What to do with words that cannot be broken, but still overflow.
enum OverflowWrap {
    OverflowWrapNormal, ///< Do nothing besides 'relaxing' the strictness of
                        ///< 'wordbreak'.
    OverflowWrapAnywhere, ///< Break anywhere as soon as overflow happens.
    OverflowWrapBreakWord ///< Break previous soft-break before breaking the
                          ///< word.
};

/// TextAlign values, see
/// https://www.w3.org/TR/css-writing-modes-4/#logical-to-physical for
/// interaction with writing mode and direction.
enum TextAlign {
    AlignLastAuto, ///< TextAlignLast: same as text-align-all, unless that's
                   ///< justify, then this is AlignStart.
    AlignStart, ///< Align text to left side of line with LTR, right with RTL,
                ///< top with the vertical writing modes.
    AlignEnd, ///< Align text to right side of line with LTR, left with RTL,
              ///< bottom with the vertical writing modes.
    AlignLeft, ///< Align text to left side of line. Top with the vertical
               ///< writing modes, bottom in sideways.
    AlignRight, ///< Align text to right side of line. Bottom with the vertical
                ///< writing modes, top in sideways
    AlignCenter, ///< Center text in line.
    AlignJustify, ///< Justify text, so that the end and start both touch the
                  ///< end and start of the line.
    AlignMatchParent ///< Inherit, except Start and End are matched against the
                     ///< parent values... We don't support this.
};
Q_ENUM_NS(TextAlign)

/// Whether and how to transform text. Not strictly necessary according to SVG2.
/// Fullwidth and FullSizeKana are inside the textTransform Struct.
enum TextTransform {
    TextTransformNone = 0x0, ///< No transforms.
    TextTransformCapitalize = 0x1, ///< Convert first letter in word of bicarmel
                                   ///< text to upper-case, locale dependant.
    TextTransformUppercase = 0x2, ///< Convert all bicarmel text to upper-case, locale dependant.
    TextTransformLowercase = 0x4, ///< Convert all bicarmel text to lower-case, locale dependant.
};
Q_ENUM_NS(TextTransform)

/// How to handle overflow.
enum TextOverflow {
    OverFlowVisible, ///< Determined by 'overflow' property, not by
                     ///< text-overflow. In svg all the non-visible values
                     ///< compute to 'clip'.
    OverFlowClip, ///< Clip the rendered content.
    OverFlowEllipse ///< Replace the last characters with "U+2026"
};

/// Flags. Whether and how to hang punctuation. Not strictly necessary according
/// to SVG2, marked as 'at-risk' in CSS-Text-3, though this feature is useful
/// for East-Asian text layout.
enum HangingPunctuation {
    HangNone = 0x0, ///< Hang nothing.
    HangFirst = 0x1, ///< Hang opening brackets and quotes.
    HangLast = 0x2, ///< Hang closing brackets and quotes.
    HangEnd = 0x4, ///< Hang stops and commas. Force/Allow is a seperate boolean.
    HangForce = 0x8 ///< Whether to force hanging stops or commas.
};

/// Baseline values used by dominant-baseline and baseline-align.
enum Baseline {
    BaselineAuto, ///< Use the preferred baseline for the writing-mode and
                  ///< text-orientation.
    BaselineUseScript, ///< SVG 1.1 feature, Deprecated in CSS-Inline-3. Use the
                       ///< preferred baseline for the given script.
    BaselineDominant, ///< alignment-baseline has the same value as
                      ///< dominant-baseline.
    BaselineNoChange, ///< Use parent baseline table.
    BaselineResetSize, ///< Use parent baseline table, but adjust to current
                       ///< fontsize.
    BaselineIdeographic, ///< Use 'ideo' or 'ideographic under/left' baselines.
                         ///< Used for CJK.
    BaselineAlphabetic, ///< Use 'romn' or the baseline for LCG scripts.
    BaselineHanging, ///< Use 'hang', or the baseline for scripts that hang like
                     ///< Tibetan and Devanagari.
    BaselineMathematical, ///< Use 'math' or the mathematical baseline, used for
                          ///< aligning numbers and mathematical symbols
                          ///< correctly.
    BaselineCentral, ///< Use the center between the ideographic over and under.
    BaselineMiddle, ///< Use the center between the alphabetical and x-height,
                    ///< or central in vertical.
    BaselineTextBottom, ///< Bottom side of the inline line-box.
    BaselineTextTop ///< Top side of the inline line-box.
};
Q_ENUM_NS(Baseline)

/// Mode of the baseline shift.
enum BaselineShiftMode {
    ShiftNone, ///< No shift.
    ShiftSub, ///< Use parent font metric for 'subscript'.
    ShiftSuper, ///< Use parent font metric for 'superscript'.
    ShiftLengthPercentage ///< Css Length Percentage, percentage is em.
};
Q_ENUM_NS(BaselineShiftMode)

enum LengthAdjust {
    LengthAdjustSpacing, ///< Only stretch the spaces.
    LengthAdjustSpacingAndGlyphs ///< Stretches the glyphs as well.
};
Q_ENUM_NS(LengthAdjust)

/// Flags for text-decoration, for underline, overline and strikethrough.
enum TextDecoration {
    DecorationNone = 0x0,
    DecorationUnderline = 0x1,
    DecorationOverline = 0x2,
    DecorationLineThrough = 0x4
};

/// Style of the text-decoration.
enum TextDecorationStyle {
    Solid, ///< Draw a solid line.Ex: -----
    Double, ///< Draw two lines. Ex: =====
    Dotted, ///< Draw a dotted line. Ex: .....
    Dashed, ///< Draw a dashed line. Ex: - - - - -
    Wavy ///< Draw a wavy line. We currently make a zigzag, ex: ^^^^^
};
Q_ENUM_NS(TextDecorationStyle)

/// Which location to choose for the underline.
enum TextDecorationUnderlinePosition {
    UnderlineAuto, ///< Use Font metrics.
    UnderlineUnder, ///< Use the bottom of the text decoration bounding box.
    UnderlineLeft, ///< Put the underline on the left of the text decoration
                   ///< bounding box, overline right.
    UnderlineRight ///< Put the underline on the right of the text decoration
                   ///< bounding box, overline left.
};
Q_ENUM_NS(TextDecorationUnderlinePosition)

/// Whether to stretch the glyphs along a path.
enum TextPathMethod {
    TextPathAlign, ///< Only align position and rotation of glyphs to the path.
    TextPathStretch ///< Align position and rotation and stretch glyphs' path
                    ///< points along the path as well.
};

/// Currently not used, this value suggest using either the default values or
/// 'better' ones. Currently not used.
enum TextPathSpacing {
    TextPathAuto,
    TextPathExact
};

/// Whether to reverse the path before laying out text.
enum TextPathSide {
    TextPathSideRight,
    TextPathSideLeft
};

/// CSS defines a number of font features as CSS properties. Not all Opentype
/// features are part of this.
enum FontVariantFeature {
    FontVariantNormal, ///< Use default features.
    FontVariantNone, ///< All features are disabled.
    /// font-variant-ligatures, common and contextual are on by default.
    CommonLigatures,
    NoCommonLigatures,
    DiscretionaryLigatures,
    NoDiscretionaryLigatures,
    HistoricalLigatures,
    NoHistoricalLigatures,
    ContextualAlternates,
    NoContextualAlternates,
    /// font-variant-position, neither values are on by default.
    PositionSub,
    PositionSuper,
    /// font-variant-caps, none of the values applicable are on by default.
    SmallCaps,
    AllSmallCaps,
    PetiteCaps,
    AllPetiteCaps,
    Unicase,
    TitlingCaps,
    /// font-variant-numeric, none of the values applicable are on by default.
    LiningNums,
    OldStyleNums,
    ProportionalNums,
    TabularNums,
    DiagonalFractions,
    StackedFractions,
    Ordinal,
    SlashedZero,
    /// font-variant-alternates
    HistoricalForms,
    StylisticAlt,
    StyleSet,
    CharacterVariant,
    Swash,
    Ornaments,
    Annotation,
    /// font-variant-east-asian, none of the values applicable are on by
    /// default.
    EastAsianJis78,
    EastAsianJis83,
    EastAsianJis90,
    EastAsianJis04,
    EastAsianSimplified,
    EastAsianTraditional,
    EastAsianFullWidth,
    EastAsianProportionalWidth,
    EastAsianRuby
};

Q_DECLARE_FLAGS(TextDecorations, TextDecoration)
Q_DECLARE_OPERATORS_FOR_FLAGS(TextDecorations)

Q_DECLARE_FLAGS(TextSpaceTrims, TextSpaceTrim)
Q_DECLARE_OPERATORS_FOR_FLAGS(TextSpaceTrims)

Q_DECLARE_FLAGS(HangingPunctuations, HangingPunctuation)
Q_DECLARE_OPERATORS_FOR_FLAGS(HangingPunctuations)

/**
 * CssLengthPercentage is a struct that represents the CSS length-percentage,
 * which is Css' way of saying that an attribute can be either a length (which
 * are unit-based, including font-relative units) or a percentage (which is
 * relative to a value on a case-by-case basis).
 *
 * Most percentages in SVG are viewport based, which we just resolve during
 * parsing, because KoShapes do not know anything about the doc they're in. A
 * handful are font-size based, which we resolve to em, and this is also what
 * this struct defaults to. A very small subset (text-indent and text path
 * start-offset) are relative to other things, and these are solved during
 * text-layout.
 *
 * We currently use this for both Lengths and LengthPercentages (always saving
 * as EM when it is a Length, in the case of TabSize). Whether and how a
 * percentage should be resolved is defined in the CSS specs in the section a
 * given property is specified.
 */

struct CssLengthPercentage : public boost::equality_comparable<CssLengthPercentage> {
    enum UnitType {
        Absolute, ///< Pt, everything needs to be converted to pt for this to work.
        Percentage, /// 0 to 1.0
        Em, /// multiply by Font-size
        Ex, /// multiply by font-x-height.
    };

    CssLengthPercentage() {}
    CssLengthPercentage(qreal value, UnitType unit = Absolute): value(value), unit(unit) {}

    qreal value = 0.0;
    UnitType unit = Absolute;

    void convertToAbsolute(const qreal fontSizeInPt, const qreal fontXHeightInPt, const UnitType percentageUnit = Em);

    bool operator==(const CssLengthPercentage & other) const {
        return qFuzzyCompare(value, other.value) && unit == other.unit;
    }
};

QDebug KRITAFLAKE_EXPORT operator<<(QDebug dbg, const KoSvgText::CssLengthPercentage &value);

// Not all properties support percentage, so this ensures % is saved as em as a fallback.
QString writeLengthPercentage(const CssLengthPercentage &length, bool percentageAsEm = false);

/**
 * AutoValue represents the "auto-or-real" values used in SVG
 *
 * Some SVG attributes can be set to either "auto" or some floating point
 * value. E.g. 'kerning' attribute. If its value is "auto", the kerning is
 * defined by the kerning tables of the font. And if its value is a real
 * number, e.g. 0 or 5.5, the font kerning is set to this particular number.
 */
struct AutoValue : public boost::equality_comparable<AutoValue>
{
    AutoValue() {}
    AutoValue(qreal _customValue) : isAuto(false), customValue(_customValue) {}

    bool isAuto = true;
    qreal customValue = 0.0;

    bool operator==(const AutoValue & other) const {
        return isAuto == other.isAuto && (isAuto || qFuzzyCompare(customValue, other.customValue));
    }
};

struct AutoLengthPercentage : public boost::equality_comparable<AutoLengthPercentage>
{
    AutoLengthPercentage() {}
    AutoLengthPercentage(CssLengthPercentage _length)
        : isAuto(false), length(_length) {}
    AutoLengthPercentage(qreal _customValue, CssLengthPercentage::UnitType unit = CssLengthPercentage::Absolute)
        : isAuto(false), length(_customValue, unit) {}

    bool isAuto = true;
    CssLengthPercentage length;

    bool operator==(const AutoLengthPercentage & other) const {
        return isAuto == other.isAuto && (isAuto || length == other.length);
    }
};



QDebug KRITAFLAKE_EXPORT operator<<(QDebug dbg, const KoSvgText::AutoValue &value);
QDebug KRITAFLAKE_EXPORT operator<<(QDebug dbg, const KoSvgText::AutoLengthPercentage &value);

inline QVariant fromAutoValue(const KoSvgText::AutoValue &value) {
    return QVariant::fromValue(value);
}

AutoValue parseAutoValueX(const QString &value, const SvgLoadingContext &context, const QString &autoKeyword = "auto");
AutoValue parseAutoValueY(const QString &value, const SvgLoadingContext &context, const QString &autoKeyword = "auto");
AutoValue parseAutoValueXY(const QString &value, const SvgLoadingContext &context, const QString &autoKeyword = "auto");
AutoValue parseAutoValueAngular(const QString &value, const SvgLoadingContext &context, const QString &autoKeyword = "auto");

AutoLengthPercentage parseAutoLengthPercentageXY(const QString &value, const SvgLoadingContext &context, const QString &autoKeyword = "auto", QRectF bbox = QRectF(), bool percentageIsViewPort = true);

WritingMode parseWritingMode(const QString &value);
Direction parseDirection(const QString &value);
UnicodeBidi parseUnicodeBidi(const QString &value);
TextOrientation parseTextOrientation(const QString &value);
TextOrientation parseTextOrientationFromGlyphOrientation(AutoValue value);
TextAnchor parseTextAnchor(const QString &value);

/**
 * @brief whiteSpaceValueToLongHands
 * CSS-Text-4 takes CSS-Text-3 whitespace values and treats them as a shorthand
 * for three more specific properties. This method sets the three properties
 * according to the white space value given.
 *
 * @return whether the value could be parsed.
 * Some CSS-Text-3 whitespace values ("break-spaces") have no CSS-Text-4
 * variants yet, and thus will be interpreted as "Normal" while this boolean
 * returns false.
 */
bool whiteSpaceValueToLongHands(const QString &value, TextSpaceCollapse &collapseMethod, TextWrap &wrapMethod, TextSpaceTrims &trimMethod);
/**
 * @brief xmlSpaceToLongHands
 * This takes xml:space values and converts them to CSS-Text-4 properties.
 * @see whiteSpaceValueToLongHands
 */
bool xmlSpaceToLongHands(const QString &value, TextSpaceCollapse &collapseMethod);

WordBreak parseWordBreak(const QString &value);
LineBreak parseLineBreak(const QString &value);
TextAlign parseTextAlign(const QString &value);

Baseline parseBaseline(const QString &value);
BaselineShiftMode parseBaselineShiftMode(const QString &value);

LengthAdjust parseLengthAdjust(const QString &value);

static const std::array<const char *, 9> fontStretchNames = {"ultra-condensed",
                                                             "extra-condensed",
                                                             "condensed",
                                                             "semi-condensed",
                                                             "normal",
                                                             "semi-expanded",
                                                             "expanded",
                                                             "extra-expanded",
                                                             "ultra-expanded"};
static const std::array<const char *, 7> fontSizeNames =
    {"xx-small", "x-small", "small", "medium", "large", "x-large", "xx-large"};
/**
 * @brief parseCSSFontStretch
 * For CSS3, the font-stretches were only given as keywords. In Css 4 and above,
 * they also allow values, except in the "font"-shorthand. The css3 bool will
 * restrict parsing to this value for this reason.
 */
int parseCSSFontStretch(const QString &value, int currentStretch);

int parseCSSFontWeight(const QString &value, int currentWeight);

QMap<QString, FontVariantFeature> fontVariantStrings();
QStringList fontVariantOpentypeTags(FontVariantFeature feature);

TextPathMethod parseTextPathMethod(const QString &value);
TextPathSpacing parseTextPathSpacing(const QString &value);
TextPathSide parseTextPathSide(const QString &value);

QString writeAutoValue(const AutoValue &value, const QString &autoKeyword = "auto");

// Not all properties support percentages, so in that case, save percentage as em.
QString writeAutoLengthPercentage(const AutoLengthPercentage &value, const QString &autoKeyword = "auto", bool percentageToEm = false);

QString writeWritingMode(WritingMode value, bool svg1_1 = false);
QString writeDirection(Direction value);
QString writeUnicodeBidi(UnicodeBidi value);
QString writeTextOrientation(TextOrientation orientation);
QString writeTextAnchor(TextAnchor value);
QString writeDominantBaseline(Baseline value);
QString writeAlignmentBaseline(Baseline value);
QString writeBaselineShiftMode(BaselineShiftMode value, CssLengthPercentage shift);
QString writeLengthAdjust(LengthAdjust value);

QString writeTextPathMethod(TextPathMethod value);
QString writeTextPathSpacing(TextPathSpacing value);
QString writeTextPathSide(TextPathSide value);

QString writeWordBreak(WordBreak value);
QString writeLineBreak(LineBreak value);
QString writeTextAlign(TextAlign value);

/**
 * @brief writeWhiteSpaceValue
 * determine the CSS-3-Whitespace shorthand value.
 * @see whiteSpaceValueToLongHands
 */
QString writeWhiteSpaceValue(TextSpaceCollapse collapseMethod, TextWrap wrapMethod, KoSvgText::TextSpaceTrims trimMethod);
QString writeXmlSpace(TextSpaceCollapse collapseMethod);

struct CharTransformation : public boost::equality_comparable<CharTransformation>
{
    boost::optional<qreal> xPos;
    boost::optional<qreal> yPos;
    boost::optional<qreal> dxPos;
    boost::optional<qreal> dyPos;
    boost::optional<qreal> rotate;

    void mergeInParentTransformation(const CharTransformation &t);
    bool isNull() const;
    bool startsNewChunk() const;
    bool hasRelativeOffset() const;

    QPointF absolutePos() const;
    QPointF relativeOffset() const;

    bool operator==(const CharTransformation & other) const;
};
QDebug KRITAFLAKE_EXPORT operator<<(QDebug dbg, const KoSvgText::CharTransformation &t);

struct TextOnPathInfo {
    qreal startOffset = 0.0;
    bool startOffsetIsPercentage = false;
    TextPathMethod method = TextPathAlign;
    TextPathSpacing spacing = TextPathAuto;
    TextPathSide side = TextPathSideLeft;
};

/// "This property transforms text for styling purposes.
///  It has no effect on the underlying content, and must not affect the content
///  of a plain text copy & paste operation."
/// -- CSS-Text-3
struct TextTransformInfo : public boost::equality_comparable<TextTransformInfo> {
    TextTransformInfo() = default;
    TextTransform capitals = TextTransformNone; ///< Text transform upper/lower/capitalize.
    bool fullWidth = false; ///< Convert proportional or half-width text to
                            ///< full-width text. 'at-risk'
    bool fullSizeKana = false; ///< Convert Japanese Katakana and Hiragana to
                               ///< their 'fullsize' equivelants. 'at-risk'
    bool operator==(const TextTransformInfo &rhs) const
    {
        return (capitals == rhs.capitals) && (fullWidth == rhs.fullWidth) && (fullSizeKana == rhs.fullSizeKana);
    }
};
QDebug KRITAFLAKE_EXPORT operator<<(QDebug dbg, const KoSvgText::TextTransformInfo &t);
TextTransformInfo parseTextTransform(const QString &value);
QString writeTextTransform(TextTransformInfo textTransform);

/// "This property specifies the indentation applied to lines of inline content
/// in a block.
///  The indent is treated as a margin applied to the start edge of the line
///  box." -- CSS-Text-3
struct TextIndentInfo : public boost::equality_comparable<TextIndentInfo> {
    TextIndentInfo() = default;

    CssLengthPercentage length;
    bool hanging = false; ///< Flip the lines to which text-indent is applied.
    bool eachLine = false; ///< Apply the text-indent to each line following a hardbreak.
    bool operator==(const TextIndentInfo &rhs) const
    {
        return (length == rhs.length) && (hanging == rhs.hanging) && (eachLine == rhs.eachLine);
    }
};

TextIndentInfo parseTextIndent(const QString &value, const SvgLoadingContext &context);
QString writeTextIndent(TextIndentInfo textIndent);

QDebug KRITAFLAKE_EXPORT operator<<(QDebug dbg, const KoSvgText::TextIndentInfo &value);

/// "This property determines the tab size used to render preserved tab
/// characters (U+0009)." -- CSS-Text-3
struct TabSizeInfo : public boost::equality_comparable<TabSizeInfo> {
    qreal value = 8; ///< A length or a number. Length is currently marked 'at-risk'.
    bool isNumber = true; ///< Multiply by width of 'space' character, including
                          ///< word- and letter-spacing.
    CssLengthPercentage length;
    qreal extraSpacing = 0.0; ///< Extra spacing due word or letter-spacing. Not
                              ///< written to css and only used during layout.
    bool operator==(const TabSizeInfo &rhs) const
    {
        bool val = isNumber? qFuzzyCompare(value, rhs.value): length == rhs.length;
        return (val) && (isNumber == rhs.isNumber);
    }
};
TabSizeInfo parseTabSize(const QString &value, const SvgLoadingContext &context);
QString writeTabSize(TabSizeInfo tabSize);
QDebug KRITAFLAKE_EXPORT operator<<(QDebug dbg, const KoSvgText::TabSizeInfo &value);


struct LineHeightInfo : public boost::equality_comparable<LineHeightInfo> {
    CssLengthPercentage length;
    qreal value = 1.0; /// Length or number.
    bool isNumber = false; /// It's a number indicating the lineHeight;
    bool isNormal = true; /// The 'auto' value.

    bool operator==(const LineHeightInfo &rhs) const
    {
        bool toggles = (isNumber == rhs.isNumber && isNormal == rhs.isNormal);
        bool val = isNumber? qFuzzyCompare(value, rhs.value): length == rhs.length;
        return (toggles && val);
    }
};

LineHeightInfo parseLineHeight(const QString &value, const SvgLoadingContext &context);
QString writeLineHeight(LineHeightInfo lineHeight);
QDebug KRITAFLAKE_EXPORT operator<<(QDebug dbg, const KoSvgText::LineHeightInfo &value);

/**
 * @brief BackgroundProperty is a special wrapper around KoShapeBackground for managing it in KoSvgTextProperties
 */
struct BackgroundProperty : public boost::equality_comparable<BackgroundProperty>
{
    BackgroundProperty() {}
    BackgroundProperty(QSharedPointer<KoShapeBackground> p) : property(p) {}

    bool operator==(const BackgroundProperty &rhs) const {
        return (!property && !rhs.property) ||
                (property && rhs.property &&
                 property->compareTo(rhs.property.data()));
    }

    QSharedPointer<KoShapeBackground> property;
};

QDebug KRITAFLAKE_EXPORT operator<<(QDebug dbg, const KoSvgText::BackgroundProperty &prop);

/**
 * @brief StrokeProperty is a special wrapper around KoShapeStrokeModel for managing it in KoSvgTextProperties
 */
struct StrokeProperty : public boost::equality_comparable<StrokeProperty>
{
    StrokeProperty() {}
    StrokeProperty(QSharedPointer<KoShapeStrokeModel> p) : property(p) {}

    bool operator==(const StrokeProperty &rhs) const {
        return (!property && !rhs.property) ||
                (property && rhs.property &&
                 property->compareFillTo(rhs.property.data()) && property->compareStyleTo(rhs.property.data()));
    }

    QSharedPointer<KoShapeStrokeModel> property;
};

QDebug KRITAFLAKE_EXPORT operator<<(QDebug dbg, const KoSvgText::StrokeProperty &prop);

} // namespace KoSvgText

Q_DECLARE_METATYPE(KoSvgText::CssLengthPercentage)
Q_DECLARE_METATYPE(KoSvgText::AutoValue)
Q_DECLARE_METATYPE(KoSvgText::AutoLengthPercentage)
Q_DECLARE_METATYPE(KoSvgText::TextDecorations)
Q_DECLARE_METATYPE(KoSvgText::HangingPunctuations)
Q_DECLARE_METATYPE(KoSvgText::TextSpaceTrims)
Q_DECLARE_METATYPE(KoSvgText::BackgroundProperty)
Q_DECLARE_METATYPE(KoSvgText::StrokeProperty)
Q_DECLARE_METATYPE(KoSvgText::TextTransformInfo)
Q_DECLARE_METATYPE(KoSvgText::TextIndentInfo)
Q_DECLARE_METATYPE(KoSvgText::TabSizeInfo)
Q_DECLARE_METATYPE(KoSvgText::LineHeightInfo)

#endif // KOSVGTEXT_H
