/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOSVGTEXTPROPERTIES_H
#define KOSVGTEXTPROPERTIES_H

#include "kritaflake_export.h"
#include "KoFlakeTypes.h"
#include "KoSvgText.h"

#include <QScopedPointer>
#include <QVariant>
#include <QList>

#include <boost/operators.hpp>

#include "KoCSSFontInfo.h"

class SvgLoadingContext;
class KoShapeBackground;

/**
 * KoSvgTextProperties represents the text attributes defined in SVG DOM tree
 *
 * There is a limitation in flake: it doesn't support the inheritance of shape
 * properties: every shape stores all the properties that were defined at the
 * loading/creation stage. KoSvgTextProperties allows the user to compare
 * the properties of the two shapes and distinguish, which properties were
 * inherited by text shape, and which are its own. It is needed to generate a
 * correct and clean SVG/markup code that can be edited by the user easily.
 * Otherwise, every \<tspan\> block will contain the full list of 20+ attributes,
 * which are not interesting for the user, since they are inherited or default.
 *
 * To achieve the goal, KoSvgTextProperties wraps all the SVG attributes into a
 * map of QVariants. When the user need to find a set of unique properties
 * of the shape, it iterates through the map and compares values with standard
 * QVariant-based comparison operator. If the property value in a child and a
 * parent is not the same, then it is not inherited.
 */
class KRITAFLAKE_EXPORT KoSvgTextProperties : public boost::equality_comparable<KoSvgTextProperties>
{
public:
    /**
     * Defines a set of supported properties. See SVG 1.1 for details.
     */
    enum PropertyId {
        WritingModeId, ///< KoSvgText::WritingMode
        DirectionId, ///< KoSvgText::Direction
        UnicodeBidiId, ///< KoSvgText::UnicodeBidi
        TextAnchorId, ///< KoSvgText::TextAnchor
        DominantBaselineId, ///< KoSvgText::Baseline
        AlignmentBaselineId, ///< KoSvgText::Baseline
        BaselineShiftModeId, ///< KoSvgText::BaselineShiftMode
        BaselineShiftValueId, ///< Double
        KerningId, ///< KoSvgText::AutoValue
        TextOrientationId, ///< KoSvgText::TextOrientation
        LetterSpacingId, ///< KoSvgText::AutoLengthPercentage
        WordSpacingId, ///< KoSvgText::AutoLengthPercentage

        FontFamiliesId, ///< QStringList
        FontStyleId, ///< KoSvgText::CssSlantData
        FontStretchId, ///< Int
        FontWeightId, ///< Int
        FontSizeId, ///< Double
        FontSizeAdjustId, ///< KoSvgText::AutoValue

        /// KoSvgText::FontVariantFeature
        FontVariantLigatureId,
        FontVariantPositionId,
        FontVariantCapsId,
        FontVariantNumericId,
        FontVariantEastAsianId,

        FontFeatureSettingsId, ///< QStringList
        FontOpticalSizingId, ///< Bool
        FontVariationSettingsId, ///< QStringList

        TextDecorationLineId, ///< Flags, KoSvgText::TextDecorations
        TextDecorationStyleId, ///< KoSvgText::TextDecorationStyle
        TextDecorationColorId, ///< QColor
        TextDecorationPositionId, ///< KoSvgText::TextDecorationUnderlinePosition
        FillId, ///< KoSvgText::BackgroundProperty
        StrokeId, ///< KoSvgText::StrokeProperty
        Opacity, ///< Double, SVG shape opacity.
        PaintOrder, ///< QVector<KoShape::PaintOrder>
        Visiblity, ///< Bool, CSS visibility

        TextLanguage, ///< a language string.

        TextCollapseId, ///< KoSvgText::TextSpaceCollapse
        TextWrapId, ///< KoSvgText::TextWrap
        TextTrimId, ///< Flags, KoSvgText::TextSpaceTrims
        LineBreakId, ///< KoSvgText::LineBreak
        WordBreakId, ///< KoSvgText::WordBreak
        TextAlignAllId, ///< KoSvgText::TextAlign
        TextAlignLastId, ///< KoSvgText::TextAlign
        TextTransformId, ///< KoSvgText::TextTransformInfo Struct
        TextOverFlowId, ///< KoSvgText::WordBreak
        OverflowWrapId, ///<
        InlineSizeId, ///< KoSvgText::AutoValue
        LineHeightId, ///< KoSvgText::AutoValue
        TextIndentId, ///< KoSvgText::TextIndentInfo Struct.
        HangingPunctuationId, ///< Flags, KoSvgText::HangingPunctuations
        TabSizeId, ///< Int

        ShapePaddingId, ///< Double
        ShapeMarginId,  ///< Double

        FontSynthesisBoldId, ///< Bool
        FontSynthesisItalicId, ///< Bool
        FontSynthesisSmallCapsId, ///< Bool
        FontSynthesisSuperSubId, ///< Bool

        TextRenderingId, ///< Enum

        KraTextVersionId, ///< Int, used for handling incorrectly saved files.
        KraTextStyleType, ///< string, used to identify the style preset type (character or paragraph).
        KraTextStyleResolution, ///< Int, used to scale style presets to be pixel-relative.
    };

    KoSvgTextProperties();
    ~KoSvgTextProperties();

    KoSvgTextProperties(const KoSvgTextProperties &rhs);
    KoSvgTextProperties& operator=(const KoSvgTextProperties &rhs);
    bool operator==(const KoSvgTextProperties &rhs) const;

    /**
     * Set the property \p id to \p value
     */
    void setProperty(PropertyId id, const QVariant &value);

    /**
     * Check if property \p id is present in this properties set
     */
    bool hasProperty(PropertyId id) const;

    /**
     * Return the value of property \p id. If the property doesn't exist in
     * the shape, return \p defaultValue instead.
     */
    QVariant property(PropertyId id, const QVariant &defaultValue = QVariant()) const;

    /**
     * Remove property \p id from the set
     */
    void removeProperty(PropertyId id);

    /**
     * Return the value of property \p id. If the property doesn't exist in the
     * shape, return the default value define in SVG 1.1.
     */
    QVariant propertyOrDefault(PropertyId id) const;

    /**
     * Return a list of properties contained in this set
     */
    QList<PropertyId> properties() const;

    /**
     * Return true if the set contains no properties
     */
    bool isEmpty() const;

    /**
     * Reset all non-inheritable properties to default values. The set of
     * non-inheritable properties is define by SVG 1.1. Used by the loading
     * code for resetting state automata's properties on entering a \<tspan\>.
     */
    void resetNonInheritableToDefault();


    /**
     * Apply properties from the parent shape. The property is set **iff** the
     * property is inheritable according to SVG and this set does not define
     * it.
     */
    void inheritFrom(const KoSvgTextProperties &parentProperties, bool resolve = false);

    /**
     * @brief resolveRelativeValues
     * resolve the font-relative values.
     * @param fontSize -- fontsize to resolve 'em' to.
     * @param xHeight -- xHeight to resolve 'ex' to.
     */
    void resolveRelativeValues(const KoSvgText::FontMetrics metrics = KoSvgText::FontMetrics(12.0, true), const qreal fontSize = 12.0);

    /**
     * Return true if the property \p id is inherited from \p parentProperties.
     * The property is considered "inherited" **iff* it is inheritable
     * according to SVG and the parent defined the same property with the same
     * value.
     */
    bool inheritsProperty(PropertyId id, const KoSvgTextProperties &parentProperties) const;

    /// Test whether it has non-inheritable properties set.
    bool hasNonInheritableProperties() const;

    /// Used to merge child properties into parent properties
    void setAllButNonInheritableProperties(const KoSvgTextProperties &properties);

    /**
     * @brief scaleAbsoluteValues
     * This scales all absolute values stored in these text properties. Relative
     * values don't need to be scaled. This can be used to scale styles in setSize
     * as well as scale the text properties for Style Presets.
     * @param scaleInline -- affects inline-direction values, like letter-spacing,
     * inline-size, tab-size, text-indent, etc.
     * @param scaleBlock -- affects block-direction values, like font-size,
     * line-height, and baseline-shift.
     */
    void scaleAbsoluteValues(const double scaleInline = 1.0, const double scaleBlock = 1.0);

    /**
     * Return a set of properties that ar **not** inherited from \p
     * parentProperties. The property is considered "inherited" **iff* it is
     * inheritable according to SVG and the parent defined the same property
     * with the same value.
     * @param keepFontSize whether to keep the font size, use for root nodes
     * so that it won't be omitted and inheriting from the "default", which may
     * not be deterministic.
     */
    KoSvgTextProperties ownProperties(const KoSvgTextProperties &parentProperties, bool keepFontSize = false) const;

    /**
     * @brief parseSvgTextAttribute add a property according to an XML attribute value.
     * @param context shared loading context
     * @param command XML attribute name
     * @param value attribute value
     *
     * @see supportedXmlAttributes for a list of supported attributes
     */
    void parseSvgTextAttribute(const SvgLoadingContext &context, const QString &command, const QString &value);

    /**
     * Convert all the properties of the set into a map of XML attribute/value
     * pairs.
     */
    QMap<QString, QString> convertToSvgTextAttributes() const;

    /**
     * @brief convertParagraphProperties
     * some properties only apply to the root shape, so we write those separately.
     * @return
     */
    QMap<QString, QString> convertParagraphProperties() const;

    QFont generateFont() const;

    qreal xHeight() const;

    /**
     * @brief metrics
     * Return the metrics of the first available font.
     * @param withResolvedLineHeight -- apply the lineheight into the linegap property.
     * @return metrics for the current font.
     */
    KoSvgText::FontMetrics metrics(const bool withResolvedLineHeight = true) const;

    /**
     * @brief applyLineHeight
     * Calculate the linegap for the current linegap property.
     * @param metrics the metrics to apply this to.
     * @return metrics with the linegap adjusted for the lineheight.
     */
    KoSvgText::FontMetrics applyLineHeight(KoSvgText::FontMetrics metrics) const;

    /**
     * @brief fontFeaturesForText
     * Returns a harfbuzz friendly list of opentype font-feature settings using
     * the various font-variant and font-feature-settings values.
     * @param start the start pos of the text.
     * @param length the length of the text.
     * @return a list of strings for font-features and their ranges that can be
     * understood by harfbuzz.
     */
    QStringList fontFeaturesForText(int start, int length) const;

    /**
     * @brief cssFontInfo
     * @return this collects all the CSS Font properties into
     * a KoCSSFontInfo struct for usage with the KoFontRegistry.
     */
    KoCSSFontInfo cssFontInfo() const;

    QSharedPointer<KoShapeBackground> background() const;
    KoShapeStrokeModelSP stroke() const;

    KoSvgText::CssLengthPercentage fontSize() const;
    void setFontSize(const KoSvgText::CssLengthPercentage length);

    /**
     * Return a list of supported XML attribute names (defined in SVG)
     */
    static QStringList supportedXmlAttributes();

    /**
     * Return a static object that defines default values for all the supported
     * properties according to SVG
     */
    static const KoSvgTextProperties& defaultProperties();

    /**
     * Returns whether the property only applies to paragraphs (what CSS calls blocks).
     * Within SVG, all paragraphs are blocks and all text inside is inline. There are
     * some potential caveats to this (text-combine-upright, ruby), but those are also
     * unique in other ways.
     */
    static bool propertyIsBlockOnly(KoSvgTextProperties::PropertyId id);

    /**
     * returns whether a property can be inherited.
     */
    bool propertyIsInheritable(KoSvgTextProperties::PropertyId id) const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KOSVGTEXTPROPERTIES_H
