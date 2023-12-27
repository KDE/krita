/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSvgTextShape.h"
#include "KoSvgTextShape_p.h"
#include "KoSvgTextShapeLayoutFunc.h"

#include "KoCssTextUtils.h"
#include "KoFontLibraryResourceUtils.h"
#include "KoFontRegistry.h"
#include "KoSvgTextChunkShapeLayoutInterface.h"
#include "KoSvgTextProperties.h"
#include "KoColorBackground.h"

#include <FlakeDebug.h>
#include <KoPathShape.h>

#include <kis_global.h>

#include <QPainterPath>
#include <QtMath>

#include <variant>

#include <graphemebreak.h>
#include <wordbreak.h>
#include <linebreak.h>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_TRUETYPE_TABLES_H

#include <hb.h>
#include <hb-ft.h>
#include <hb-ot.h>

#include <raqm.h>

using raqm_t_up = KisLibraryResourcePointer<raqm_t, raqm_destroy>;

using KoSvgTextShapeLayoutFunc::calculateLineHeight;
using KoSvgTextShapeLayoutFunc::breakLines;
using KoSvgTextShapeLayoutFunc::getShapes;
using KoSvgTextShapeLayoutFunc::flowTextInShapes;


/**
 * @brief logicalToVisualCursorPositions
 * Create a map that sorts the cursor positions by the visual index of the cluster.
 */
static QMap<int, int> logicalToVisualCursorPositions(const QVector<CursorPos> &cursorPos
                                              , const QVector<CharacterResult> &result
                                              , const QVector<LineBox> &lines
                                              , const bool &ltr = false)  {
    QMap<int, int> logicalToVisual;
    for (int i = 0; i < lines.size(); i++) {
        Q_FOREACH(const LineChunk chunk, lines.at(i).chunks) {
            QMap<int, int> visualToLogical;
            QVector<int> visual;
            Q_FOREACH(const int j, chunk.chunkIndices) {
                visualToLogical.insert(result.at(j).visualIndex, j);
            }
            Q_FOREACH(const int j, visualToLogical.values()) {
                QMap<int, int> relevant;
                for (int k = 0; k < cursorPos.size(); k++) {
                    if (j == cursorPos.at(k).cluster) {
                        relevant.insert(cursorPos.at(k).offset, k);
                    }
                }
                Q_FOREACH(const int k, relevant.keys()) {
                    int final = result.at(j).cursorInfo.rtl? relevant.size()-1-k: k;
                    visual.append(relevant.value(final));
                }
            }

            if (ltr) {
                for (int k = 0; k < visual.size(); k++) {
                    logicalToVisual.insert(visual.at(k), logicalToVisual.size());
                }
            } else {
                for (int k = visual.size()-1; k > -1; k--) {
                    logicalToVisual.insert(visual.at(k), logicalToVisual.size());
                }
            }
        }
    }

    return logicalToVisual;
}


// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void KoSvgTextShape::Private::relayout(const KoSvgTextShape *q)
{
    clearAssociatedOutlines(q);
    this->initialTextPosition = QPointF();
    this->result.clear();
    this->cursorPos.clear();
    this->logicalToVisualCursorPos.clear();

    // The following is based on the text-layout algorithm in SVG 2.
    KoSvgText::WritingMode writingMode = KoSvgText::WritingMode(q->textProperties().propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());
    KoSvgText::Direction direction = KoSvgText::Direction(q->textProperties().propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
    KoSvgText::AutoValue inlineSize = q->textProperties().propertyOrDefault(KoSvgTextProperties::InlineSizeId).value<KoSvgText::AutoValue>();
    QString lang = q->textProperties().property(KoSvgTextProperties::TextLanguage).toString().toUtf8();

    const bool isHorizontal = writingMode == KoSvgText::HorizontalTB;

    FT_Int32 loadFlags = 0;
    if (this->textRendering == GeometricPrecision || this->textRendering == Auto) {
        // without load_no_hinting, the advance and offset will be rounded
        // to nearest pixel, which we don't want as we're using the vector
        // outline.
        loadFlags |= FT_LOAD_NO_HINTING;

        // Disable embedded bitmaps because they _do not_ follow geometric
        // precision, but is focused on legibility.
        // This does not affect bitmap-only fonts.
        loadFlags |= FT_LOAD_NO_BITMAP;
    } else {
        // When using hinting, sometimes the bounding box does not encompass the
        // drawn glyphs properly.
        // The default hinting works best for vertical, while the 'light'
        // hinting mode works best for horizontal.
        if (isHorizontal) {
            loadFlags |= FT_LOAD_TARGET_LIGHT;
        }
    }
    const auto loadFlagsForFace = [loadFlags, isHorizontal](FT_Face face) -> FT_Int32 {
        FT_Int32 faceLoadFlags = loadFlags;
        if (FT_HAS_COLOR(face)) {
            faceLoadFlags |= FT_LOAD_COLOR;
        }
        if (!isHorizontal && FT_HAS_VERTICAL(face)) {
            faceLoadFlags |= FT_LOAD_VERTICAL_LAYOUT;
        }
        if (!FT_IS_SCALABLE(face)) {
            // This is needed for the CBDT version of Noto Color Emoji
            faceLoadFlags &= ~FT_LOAD_NO_BITMAP;
        }
        return faceLoadFlags;
    };

    // Whenever the freetype docs talk about a 26.6 floating point unit, they
    // mean a 1/64 value.
    const qreal ftFontUnit = 64.0;
    const qreal ftFontUnitFactor = 1 / ftFontUnit;
    const qreal finalRes = qMin(this->xRes, this->yRes);
    const qreal scaleToPT = 72. / finalRes;
    const qreal scaleToPixel = finalRes / 72.;
    const QTransform dpiScale = QTransform::fromScale(scaleToPT, scaleToPT);
    const QTransform ftTF = QTransform::fromScale(ftFontUnitFactor, -ftFontUnitFactor) * dpiScale;

    // Some fonts have a faulty underline thickness,
    // so we limit the minimum to be a single pixel wide.
    const qreal minimumDecorationThickness = scaleToPT;

    // First, get text. We use the subChunks because that handles bidi-insertion for us.

    bool _ignore = false;
    const QVector<KoSvgTextChunkShapeLayoutInterface::SubChunk> textChunks =
        q->layoutInterface()->collectSubChunks(false, _ignore);
    QString text;
    QVector<QPair<int, int>> clusterToOriginalString;
    QString plainText;
    Q_FOREACH (const KoSvgTextChunkShapeLayoutInterface::SubChunk &chunk, textChunks) {
        for (int i = 0; i < chunk.newToOldPositions.size(); i++) {
            QPair pos = chunk.newToOldPositions.at(i);
            int a = pos.second < 0? -1: text.size()+pos.second;
            int b = pos.first < 0? -1: plainText.size()+pos.first;
            QPair newPos = QPair<int, int> (a, b);
            clusterToOriginalString.append(newPos);
        }
        text.append(chunk.text);
        plainText.append(chunk.originalText);
    }
    debugFlake << "Laying out the following text: " << text;

    // 1. Setup.

    // KoSvgText::TextSpaceTrims trims =
    // q->textProperties().propertyOrDefault(KoSvgTextProperties::TextTrimId).value<KoSvgText::TextSpaceTrims>();
    KoSvgText::TextWrap wrap = KoSvgText::TextWrap(q->textProperties().propertyOrDefault(KoSvgTextProperties::TextWrapId).toInt());
    KoSvgText::TextSpaceCollapse collapse = KoSvgText::TextSpaceCollapse(q->textProperties().propertyOrDefault(KoSvgTextProperties::TextCollapseId).toInt());
    KoSvgText::LineBreak linebreakStrictness = KoSvgText::LineBreak(q->textProperties().property(KoSvgTextProperties::LineBreakId).toInt());
    QVector<bool> collapseChars = KoCssTextUtils::collapseSpaces(&text, collapse);
    if (!lang.isEmpty()) {
        // Libunibreak currently only has support for strict, and even then only
        // for very specific cases.
        if (linebreakStrictness == KoSvgText::LineBreakStrict) {
            lang += "-strict";
        }
    }
    QVector<QPair<bool, bool>> justify;
    QVector<char> lineBreaks(text.size());
    QVector<char> wordBreaks(text.size());
    QVector<char> graphemeBreaks(text.size());
    if (text.size() > 0) {
        // TODO: Figure out how to gracefully skip all the next steps when the text-size is 0.
        // can't currently remember if removing the associated outlines was all that is necessary.
        set_linebreaks_utf16(text.utf16(), static_cast<size_t>(text.size()), lang.toUtf8().data(), lineBreaks.data());
        set_wordbreaks_utf16(text.utf16(), static_cast<size_t>(text.size()), lang.toUtf8().data(), wordBreaks.data());
        set_graphemebreaks_utf16(text.utf16(), static_cast<size_t>(text.size()), lang.toUtf8().data(), graphemeBreaks.data());
        justify = KoCssTextUtils::justificationOpportunities(text, lang);
    }


    int globalIndex = 0;
    QVector<CharacterResult> result(text.size());
    // HACK ALERT!
    // Apparantly feeding a bidi algorithm a hardbreak makes it go 'ok, not doing any
    // bidi', which makes sense, Bidi is supossed to be done 'after' line breaking.
    // Without replacing hardbreaks with spaces, hardbreaks in rtl will break the bidi.
    for (int i = 0; i < text.size(); i++) {
        if (lineBreaks[i] == LINEBREAK_MUSTBREAK) {
            text[i] = QChar::Space;
        }
    }
    for (int i=0; i < clusterToOriginalString.size(); i++) {
        QPair mapping = clusterToOriginalString.at(i);
        if (mapping.first < 0) {
            continue;
        } else {
            if (mapping.first < result.size()) {
                result[mapping.first].plaintTextIndex = mapping.second;
            }
        }
    }


    // 3. Resolve character positioning.
    // According to SVG 2.0 algorithm, you'd first put everything into a css-compatible-renderer,
    // so, apply https://www.w3.org/TR/css-text-3/#order and then the rest of the SVG 2 text algorithm.
    // However, SVG 1.1 requires Textchunks to have separate shaping (and separate bidi), so you need to
    // resolve the transforms first to find the absolutely positioned chunks, but because that relies on
    // white-space collapse, we need to do that first, and then apply the collapse.
    // https://github.com/w3c/svgwg/issues/631 and https://github.com/w3c/svgwg/issues/635
    // argue shaping across multiple text-chunks is undefined behaviour, but it breaks SVG 1.1 text
    // to consider it anything but required to have both shaping and bidi-reorder break.
    QVector<KoSvgText::CharTransformation> resolvedTransforms(text.size());
    globalIndex = 0;
    bool wrapped = !(inlineSize.isAuto && q->shapesInside().isEmpty());
    if (!resolvedTransforms.isEmpty()) {
        resolvedTransforms[0].xPos = 0;
        resolvedTransforms[0].yPos = 0;
    }
    this->resolveTransforms(q, text, result, globalIndex, isHorizontal, wrapped, false, resolvedTransforms, collapseChars);

    QMap<int, KoSvgText::TabSizeInfo> tabSizeInfo;

    // pass everything to a css-compatible text-layout algortihm.
    raqm_t_up layout(raqm_create());

    if (raqm_set_text_utf16(layout.data(), text.utf16(), static_cast<size_t>(text.size()))) {
        if (writingMode == KoSvgText::VerticalRL || writingMode == KoSvgText::VerticalLR) {
            raqm_set_par_direction(layout.data(), raqm_direction_t::RAQM_DIRECTION_TTB);
        } else if (direction == KoSvgText::DirectionRightToLeft) {
            raqm_set_par_direction(layout.data(), raqm_direction_t::RAQM_DIRECTION_RTL);
        } else {
            raqm_set_par_direction(layout.data(), raqm_direction_t::RAQM_DIRECTION_LTR);
        }

        int start = 0;
        Q_FOREACH (const KoSvgTextChunkShapeLayoutInterface::SubChunk &chunk, textChunks) {
            int length = chunk.text.size();
            KoSvgTextProperties properties = chunk.format.associatedShapeWrapper().shape()->textProperties();

            // In this section we retrieve the resolved transforms and
            // direction/anchoring that we can get from the subchunks.
            KoSvgText::TextAnchor anchor = KoSvgText::TextAnchor(properties.propertyOrDefault(KoSvgTextProperties::TextAnchorId).toInt());
            KoSvgText::Direction direction = KoSvgText::Direction(properties.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
            KoSvgText::WordBreak wordBreakStrictness = KoSvgText::WordBreak(properties.propertyOrDefault(KoSvgTextProperties::WordBreakId).toInt());
            KoSvgText::HangingPunctuations hang =
                properties.propertyOrDefault(KoSvgTextProperties::HangingPunctuationId).value<KoSvgText::HangingPunctuations>();
            KoSvgText::TabSizeInfo tabInfo = properties.propertyOrDefault(KoSvgTextProperties::TabSizeId).value<KoSvgText::TabSizeInfo>();
            KoSvgText::AutoValue letterSpacing = properties.propertyOrDefault(KoSvgTextProperties::LetterSpacingId).value<KoSvgText::AutoValue>();
            KoSvgText::AutoValue wordSpacing = properties.propertyOrDefault(KoSvgTextProperties::WordSpacingId).value<KoSvgText::AutoValue>();
            KoSvgText::LineHeightInfo lineHeight = properties.propertyOrDefault(KoSvgTextProperties::LineHeightId).value<KoSvgText::LineHeightInfo>();
            bool overflowWrap = KoSvgText::OverflowWrap(properties.propertyOrDefault(KoSvgTextProperties::OverflowWrapId).toInt()) != KoSvgText::OverflowWrapNormal;

            KoColorBackground *b = dynamic_cast<KoColorBackground *>(chunk.format.associatedShapeWrapper().shape()->background().data());
            QColor fillColor;
            if (b)
            {
                fillColor = b->color();
            }
            if (!letterSpacing.isAuto) {
                tabInfo.extraSpacing += letterSpacing.customValue;
            }
            if (!wordSpacing.isAuto) {
                tabInfo.extraSpacing += wordSpacing.customValue;
            }

            for (int i = 0; i < length; i++) {
                CharacterResult cr = result[start + i];
                cr.anchor = anchor;
                cr.direction = direction;
                QPair<bool, bool> canJustify = justify.value(start + i, QPair<bool, bool>(false, false));
                cr.justifyBefore = canJustify.first;
                cr.justifyAfter = canJustify.second;
                cr.overflowWrap = overflowWrap;
                if (lineBreaks[start + i] == LINEBREAK_MUSTBREAK) {
                    cr.breakType = BreakType::HardBreak;
                    cr.lineEnd = LineEdgeBehaviour::Collapse;
                    cr.lineStart = LineEdgeBehaviour::Collapse;
                } else if (lineBreaks[start + i] == LINEBREAK_ALLOWBREAK && wrap != KoSvgText::NoWrap) {
                    cr.breakType = BreakType::SoftBreak;

                    if (KoCssTextUtils::collapseLastSpace(text.at(start + i), collapse)) {
                        cr.lineEnd = LineEdgeBehaviour::Collapse;
                        cr.lineStart = LineEdgeBehaviour::Collapse;
                    }
                }
                if (cr.lineEnd != LineEdgeBehaviour::Collapse) {
                    const auto isFollowedByForcedLineBreak = [&]() {
                        if (result.size() <= start + i + 1) {
                            // End of the text block, consider it a forced line break
                            return true;
                        }
                        if (lineBreaks[start + i+ 1] == LINEBREAK_MUSTBREAK) {
                            // Next character is a forced line break
                            return true;
                        }
                        if (resolvedTransforms.at(start + i + 1).startsNewChunk()) {
                            // Next character is another chunk, consider it a forced line break
                            return true;
                        }
                        return false;
                    };
                    bool forceHang = false;
                    if (KoCssTextUtils::hangLastSpace(text.at(start + i), collapse, wrap, forceHang, isFollowedByForcedLineBreak())) {
                        cr.lineEnd = forceHang? LineEdgeBehaviour::ForceHang: LineEdgeBehaviour::ConditionallyHang;

                    }
                }

                if ((wordBreakStrictness == KoSvgText::WordBreakBreakAll ||
                     linebreakStrictness == KoSvgText::LineBreakAnywhere)
                        && wrap != KoSvgText::NoWrap) {
                    if (graphemeBreaks[start + i] == GRAPHEMEBREAK_BREAK && cr.breakType == BreakType::NoBreak) {
                        cr.breakType = BreakType::SoftBreak;
                    }
                }
                if (cr.lineStart != LineEdgeBehaviour::Collapse && hang.testFlag(KoSvgText::HangFirst)) {
                    cr.lineStart = KoCssTextUtils::characterCanHang(text.at(start + i), KoSvgText::HangFirst)
                        ? LineEdgeBehaviour::ForceHang
                        : cr.lineEnd;
                }
                if (cr.lineEnd != LineEdgeBehaviour::Collapse) {
                    if (hang.testFlag(KoSvgText::HangLast)) {
                        cr.lineEnd = KoCssTextUtils::characterCanHang(text.at(start + i), KoSvgText::HangLast)
                            ? LineEdgeBehaviour::ForceHang
                            : cr.lineEnd;
                    }
                    if (hang.testFlag(KoSvgText::HangEnd)) {
                        LineEdgeBehaviour edge = hang.testFlag(KoSvgText::HangForce)
                            ? LineEdgeBehaviour::ForceHang
                            : LineEdgeBehaviour::ConditionallyHang;
                        cr.lineEnd = KoCssTextUtils::characterCanHang(text.at(start + i), KoSvgText::HangEnd) ? edge : cr.lineEnd;
                    }
                }

                cr.cursorInfo.isWordBoundary = (wordBreaks[start + i] == WORDBREAK_BREAK);
                cr.cursorInfo.color = fillColor;

                if (text.at(start + i) == QChar::Tabulation) {
                    tabSizeInfo.insert(start + i, tabInfo);
                }

                if (resolvedTransforms.at(start + i).startsNewChunk()) {
                    raqm_set_arbitrary_run_break(layout.data(), static_cast<size_t>(start + i), true);
                }

                if (chunk.firstTextInPath && i == 0) {
                    cr.anchored_chunk = true;
                }
                result[start + i] = cr;
            }

            QVector<int> lengths;
            QStringList fontFeatures = properties.fontFeaturesForText(start, length);

            qreal fontSize = properties.property(KoSvgTextProperties::FontSizeId).toReal();
            const QFont::Style style = QFont::Style(properties.propertyOrDefault(KoSvgTextProperties::FontStyleId).toInt());
            KoSvgText::AutoValue fontSizeAdjust = properties.propertyOrDefault(KoSvgTextProperties::FontSizeAdjustId).value<KoSvgText::AutoValue>();
            if (properties.hasProperty(KoSvgTextProperties::KraTextVersionId)) {
                fontSizeAdjust.isAuto = (properties.property(KoSvgTextProperties::KraTextVersionId).toInt() < 3);
            }
            const std::vector<FT_FaceUP> faces = KoFontRegistry::instance()->facesForCSSValues(
                properties.property(KoSvgTextProperties::FontFamiliesId).toStringList(),
                lengths,
                properties.fontAxisSettings(),
                chunk.text,
                static_cast<quint32>(finalRes),
                static_cast<quint32>(finalRes),
                fontSize,
                fontSizeAdjust.isAuto ? 1.0 : fontSizeAdjust.customValue,
                properties.propertyOrDefault(KoSvgTextProperties::FontWeightId).toInt(),
                properties.propertyOrDefault(KoSvgTextProperties::FontStretchId).toInt(),
                style != QFont::StyleNormal);
            if (properties.hasProperty(KoSvgTextProperties::TextLanguage)) {
                raqm_set_language(layout.data(),
                                  properties.property(KoSvgTextProperties::TextLanguage).toString().toUtf8(),
                                  static_cast<size_t>(start),
                                  static_cast<size_t>(length));
            }
            Q_FOREACH (const QString &feature, fontFeatures) {
                debugFlake << "adding feature" << feature;
                raqm_add_font_feature(layout.data(), feature.toUtf8(), feature.toUtf8().size());
            }

            if (!letterSpacing.isAuto) {
                raqm_set_letter_spacing_range(layout.data(),
                                              static_cast<int>(letterSpacing.customValue * ftFontUnit * scaleToPixel),
                                              static_cast<size_t>(start),
                                              static_cast<size_t>(length));
            }

            if (!wordSpacing.isAuto) {
                raqm_set_word_spacing_range(layout.data(),
                                            static_cast<int>(wordSpacing.customValue * ftFontUnit * scaleToPixel),
                                            static_cast<size_t>(start),
                                            static_cast<size_t>(length));
            }

            for (int i = 0; i < lengths.size(); i++) {
                length = lengths.at(i);
                const FT_FaceUP &face = faces.at(static_cast<size_t>(i));
                const FT_Int32 faceLoadFlags = loadFlagsForFace(face.data());
                if (start == 0) {
                    raqm_set_freetype_face(layout.data(), face.data());
                    raqm_set_freetype_load_flags(layout.data(), faceLoadFlags);
                }
                if (length > 0) {
                    raqm_set_freetype_face_range(layout.data(),
                                                 face.data(),
                                                 static_cast<size_t>(start),
                                                 static_cast<size_t>(length));
                    raqm_set_freetype_load_flags_range(layout.data(),
                                                       faceLoadFlags,
                                                       static_cast<size_t>(start),
                                                       static_cast<size_t>(length));
                }

                hb_font_t_up font(hb_ft_font_create_referenced(face.data()));
                hb_position_t ascender = 0;
                hb_position_t descender = 0;
                hb_position_t lineGap = 0;

                if (isHorizontal) {
                    /**
                     * There's 3 different definitions of the so-called vertical metrics, that is,
                     * the ascender and descender for horizontally laid out script. WinAsc & Desc,
                     * HHAE asc&desc, and OS/2... we need the last one, but harfbuzz doesn't return
                     * it unless there's a flag set in the font, which is missing in a lot of fonts
                     * that were from the transitional period, like Deja Vu Sans. Hence we need to get
                     * the OS/2 table and calculate the values manually (and fall back in various ways).
                     *
                     * https://www.w3.org/TR/css-inline-3/#ascent-descent
                     * https://www.w3.org/TR/CSS2/visudet.html#sTypoAscender
                     * https://wiki.inkscape.org/wiki/Text_Rendering_Notes#Ascent_and_Descent
                     *
                     * Related HB issue: https://github.com/harfbuzz/harfbuzz/issues/1920
                     */
                    TT_OS2 *os2Table = nullptr;
                    os2Table = (TT_OS2*)FT_Get_Sfnt_Table(face.data(), FT_SFNT_OS2);
                    if (os2Table) {
                        int yscale = face.data()->size->metrics.y_scale;

                        ascender = FT_MulFix(os2Table->sTypoAscender, yscale);
                        descender = FT_MulFix(os2Table->sTypoDescender, yscale);
                        lineGap = FT_MulFix(os2Table->sTypoLineGap, yscale);
                    }

                    constexpr unsigned USE_TYPO_METRICS = 1u << 7;
                    if (!os2Table || os2Table->version == 0xFFFFU || !(os2Table->fsSelection & USE_TYPO_METRICS)) {
                        hb_position_t altAscender = 0;
                        hb_position_t altDescender = 0;
                        hb_position_t altLineGap = 0;
                        if (!hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_HORIZONTAL_ASCENDER, &altAscender)) {
                            altAscender = face.data()->ascender;
                        }
                        if (!hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_HORIZONTAL_DESCENDER, &altDescender)) {
                            altDescender = face.data()->descender;
                        }
                        if (!hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_HORIZONTAL_LINE_GAP, &altLineGap)) {
                            altLineGap = face.data()->height - (altAscender-altDescender);
                        }

                        // Some fonts have sTypo metrics that are too small compared
                        // to the HHEA values which make the default line height too
                        // tight (e.g. Microsoft JhengHei, Source Han Sans), so we
                        // compare them and take the ones that are larger.
                        if (!os2Table || (altAscender - altDescender + altLineGap) > (ascender - descender + lineGap)) {
                            ascender = altAscender;
                            descender = altDescender;
                            lineGap = altLineGap;
                        }
                    }
                } else {
                    hb_font_extents_t fontExtends;
                    hb_font_get_extents_for_direction (font.data(), HB_DIRECTION_TTB, &fontExtends);
                    if (!hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_VERTICAL_ASCENDER, &ascender)) {
                        ascender = fontExtends.ascender;
                    }
                    if (!hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_VERTICAL_DESCENDER, &descender)) {
                        descender = fontExtends.descender;
                    }
                    if (!hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_VERTICAL_LINE_GAP, &lineGap)) {
                        lineGap = 0;
                    }
                    // Default microsoft CJK fonts have the vertical ascent and descent be the same as the horizontal
                    // ascent and descent, so we 'normalize' the ascender and descender to be half the total height.
                    qreal height = ascender - fontExtends.descender;
                    ascender = height*0.5;
                    descender = -ascender;
                }
                if (ascender == 0 && descender == 0) {
                    ascender = face->size->metrics.ascender;
                    descender = face->size->metrics.descender;
                    qreal height = ascender - descender;
                    lineGap = face->size->metrics.height - height;
                    if (!isHorizontal) {
                        ascender = height * 0.5;
                        descender = -ascender;
                    }
                }

                for (int j=start; j<start+length; j++) {
                    result[j].fontAscent = ascender;
                    result[j].fontDescent = descender;
                    qreal leading = lineGap;

                    if (!lineHeight.isNormal) {
                        if (lineHeight.isNumber) {
                            leading = (fontSize*scaleToPixel*ftFontUnit)*lineHeight.value;
                            leading -= (ascender-descender);
                        } else {
                            QPointF val = ftTF.inverted().map(QPointF(lineHeight.value, lineHeight.value));
                            leading = isHorizontal? val.x(): val.y();
                            leading -= (ascender-descender);
                        }
                    }
                    result[j].fontHalfLeading = leading * 0.5;
                    result[j].fontStyle = style;
                    result[j].fontWeight = properties.propertyOrDefault(KoSvgTextProperties::FontWeightId).toInt();
                }

                start += length;
            }
        }
        debugFlake << "text-length:" << text.size();
    }
    // set very first character as anchored chunk.
    if (!result.empty()) {
        result[0].anchored_chunk = true;
    }

    if (raqm_layout(layout.data())) {
        debugFlake << "layout succeeded";
    }

    // 2. Set flags and assign initial positions
    // We also retreive a glyph path here.
    size_t count = 0;
    const raqm_glyph_t *glyphs = raqm_get_glyphs(layout.data(), &count);
    if (!glyphs) {
        return;
    }

    QPointF totalAdvanceFTFontCoordinates;
    QMap<int, int> logicalToVisual;
    this->isBidi = false;


    KIS_ASSERT(count <= INT32_MAX);

    for (int i = 0; i < static_cast<int>(count); i++) {
        raqm_glyph_t currentGlyph = glyphs[i];
        KIS_ASSERT(currentGlyph.cluster <= INT32_MAX);
        const int cluster = static_cast<int>(currentGlyph.cluster);
        if (!result[cluster].addressable) {
            continue;
        }
        CharacterResult charResult = result[cluster];

        const FT_Int32 faceLoadFlags = loadFlagsForFace(currentGlyph.ftface);

        const auto getUcs4At = [](const QString &s, int i) -> char32_t {
            const QChar high = s.at(i);
            if (!high.isSurrogate()) {
                return high.unicode();
            }
            if (high.isHighSurrogate() && s.length() > i + 1) {
                const QChar low = s[i + 1];
                if (low.isLowSurrogate()) {
                    return QChar::surrogateToUcs4(high, low);
                }
            }
            // Don't return U+FFFD replacement character but return the
            // unpaired surrogate itself, so that if we want to we can draw
            // a tofu block for it.
            return high.unicode();
        };
        const char32_t codepoint = getUcs4At(text, cluster);
        debugFlake << "glyph" << i << "cluster" << cluster << currentGlyph.index << codepoint;

        charResult.cursorInfo.rtl = raqm_get_direction_at_index(layout.data(), cluster) == RAQM_DIRECTION_RTL;
        if (charResult.cursorInfo.rtl != (charResult.direction == KoSvgText::DirectionRightToLeft)) {
            this->isBidi = true;
        }

        if (!this->loadGlyph(ftTF,
                             tabSizeInfo,
                             faceLoadFlags,
                             isHorizontal,
                             codepoint,
                             currentGlyph,
                             charResult,
                             totalAdvanceFTFontCoordinates)) {
            continue;
        }

        charResult.visualIndex = i;
        logicalToVisual.insert(cluster, i);

        charResult.middle = false;

        result[cluster] = charResult;
    }

    // fix it so that characters that are in the 'middle' due to either being
    // surrogates or part of a ligature, are marked as such. Also set the css
    // position so that anchoring will work correctly later.
    int firstCluster = -1;
    bool graphemeBreakNext = false;
    for (int i = 0; i < result.size(); i++) {
        result[i].middle = result.at(i).visualIndex == -1;
        if (result[i].addressable && !result.at(i).middle) {
            if (result.at(i).plaintTextIndex > -1 && firstCluster > -1) {
                CursorInfo info = result.at(firstCluster).cursorInfo;
                // ensure the advance gets added to the ligature carets if we found them,
                // so they don't get overwritten by the synthesis code.
                if (!info.offsets.isEmpty()) {
                    info.offsets.append(result.at(firstCluster).advance);
                }
                info.graphemeIndices.append(result.at(i).plaintTextIndex);
                result[firstCluster].cursorInfo = info;
            }
            firstCluster = i;
        } else {
            int fC = qMax(0, firstCluster);
            if (text[fC].isSpace() == text[i].isSpace()) {
                if (result[fC].breakType != BreakType::HardBreak) {
                    result[fC].breakType = result.at(i).breakType;
                }
                if (result[fC].lineStart == LineEdgeBehaviour::NoChange) {
                    result[fC].lineStart = result.at(i).lineStart;
                }
                if (result[fC].lineEnd == LineEdgeBehaviour::NoChange) {
                    result[fC].lineEnd = result.at(i).lineEnd;
                }
            }
            if (graphemeBreakNext && result[i].addressable && result.at(i).plaintTextIndex > -1) {
                result[fC].cursorInfo.graphemeIndices.append(result.at(i).plaintTextIndex);
            }
            result[i].cssPosition = result.at(fC).cssPosition + result.at(fC).advance;
            result[i].hidden = true;
        }
        graphemeBreakNext = graphemeBreaks[i] == GRAPHEMEBREAK_BREAK;
    }
    int fC = qMax(0, firstCluster);
    if (result.at(fC).cursorInfo.graphemeIndices.isEmpty() || graphemeBreakNext) {
        result[fC].cursorInfo.graphemeIndices.append(plainText.size());
    }

    // Add a dummy charResult at the end when the last non-collapsed position
    // is a hard breaks, so the new line is laid out.
    int dummyIndex = -1;
    if (result.at(fC).breakType == BreakType::HardBreak) {
        CharacterResult hardbreak = result.at(fC);
        dummyIndex = fC +1;
        CharacterResult dummy;
        //dummy.hidden = true;
        dummy.addressable = true;
        dummy.visualIndex = hardbreak.visualIndex + 1;
        dummy.scaledAscent = hardbreak.scaledAscent;
        dummy.scaledDescent = hardbreak.scaledDescent;
        dummy.scaledHalfLeading = hardbreak.scaledHalfLeading;
        dummy.cssPosition = hardbreak.cssPosition + hardbreak.advance;
        dummy.finalPosition = dummy.cssPosition;
        dummy.boundingBox = hardbreak.boundingBox;
        dummy.lineHeightBox = hardbreak.lineHeightBox;
        if (isHorizontal) {
            dummy.boundingBox.setWidth(0);
            dummy.lineHeightBox.setWidth(0);
        } else {
            dummy.boundingBox.setHeight(0);
            dummy.lineHeightBox.setHeight(0);
        }
        dummy.plaintTextIndex = hardbreak.cursorInfo.graphemeIndices.last();
        dummy.cursorInfo.caret = hardbreak.cursorInfo.caret;
        dummy.cursorInfo.rtl = hardbreak.cursorInfo.rtl;
        dummy.direction = hardbreak.direction;
        result.insert(dummyIndex, dummy);
        logicalToVisual.insert(dummyIndex, dummy.visualIndex);
        resolvedTransforms.insert(dummyIndex, KoSvgText::CharTransformation());
    }

    debugFlake << "Glyphs retrieved";

    // Compute baseline alignment.
    globalIndex = 0;
    this->computeFontMetrics(q, QMap<int, int>(), 0, QPointF(), QPointF(), result, globalIndex, finalRes, isHorizontal);

    // Handle linebreaking.
    QPointF startPos = resolvedTransforms[0].absolutePos();
    if (!this->shapesInside.isEmpty()) {
        QList<QPainterPath> shapes = getShapes(this->shapesInside, this->shapesSubtract, q->textProperties());
        this->lineBoxes = flowTextInShapes(q->textProperties(), logicalToVisual, result, shapes, startPos);
    } else {
        this->lineBoxes = breakLines(q->textProperties(), logicalToVisual, result, startPos);
    }

    // Handle baseline alignment.
    globalIndex = 0;
    this->handleLineBoxAlignment(q, result, this->lineBoxes, globalIndex, isHorizontal);

    if (inlineSize.isAuto && this->shapesInside.isEmpty()) {
        debugFlake << "Starting with SVG 1.1 specific portion";
        debugFlake << "4. Adjust positions: dx, dy";
        // 4. Adjust positions: dx, dy
        QPointF shift = QPointF();
        bool setAnchoredChunk = false;
        for (int i = 0; i < result.size(); i++) {
            if (result.at(i).addressable) {
                KoSvgText::CharTransformation transform = resolvedTransforms[i];
                if (transform.hasRelativeOffset()) {
                    shift += transform.relativeOffset();
                }
                CharacterResult charResult = result[i];
                if (transform.rotate) {
                    charResult.rotate = *transform.rotate;
                }
                charResult.finalPosition = charResult.cssPosition + shift;

                // ensure that anchored chunks aren't set in the middle of a ligature.
                if (setAnchoredChunk) {
                    charResult.anchored_chunk = true;
                    setAnchoredChunk = false;
                }
                if (transform.startsNewChunk()) {
                    if(charResult.middle) {
                        setAnchoredChunk = true;
                    } else {
                        charResult.anchored_chunk = true;
                    }
                }
                result[i] = charResult;
            }
        }

        // 5. Apply ‘textLength’ attribute
        debugFlake << "5. Apply ‘textLength’ attribute";
        globalIndex = 0;
        int resolved = 0;
        this->applyTextLength(q, result, globalIndex, resolved, isHorizontal);

        // 6. Adjust positions: x, y
        debugFlake << "6. Adjust positions: x, y";
        // https://github.com/w3c/svgwg/issues/617
        shift = QPointF();
        for (int i = 0; i < result.size(); i++) {
            if (result.at(i).addressable) {
                KoSvgText::CharTransformation transform = resolvedTransforms[i];
                CharacterResult charResult = result[i];
                if (transform.xPos) {
                    const qreal delta = transform.dxPos ? *transform.dxPos : 0.0;
                    shift.setX(*transform.xPos + (delta - charResult.finalPosition.x()));
                }
                if (transform.yPos) {
                    const qreal delta = transform.dyPos ? *transform.dyPos : 0.0;
                    shift.setY(*transform.yPos + (delta - charResult.finalPosition.y()));
                }
                charResult.finalPosition += shift;
                if (charResult.middle && i-1 >=0) {
                        charResult.finalPosition = result.at(i-1).finalPosition;
                }

                result[i] = charResult;
            }
        }

        // 7. Apply anchoring
        debugFlake << "7. Apply anchoring";
        applyAnchoring(result, isHorizontal);

        // Computing the textDecorations needs to happen before applying the
        // textPath to the results, as we need the unapplied result vector for
        // positioning.
        debugFlake << "Now Computing text-decorations";
        globalIndex = 0;
        this->computeTextDecorations(q,
                                  result,
                                  logicalToVisual,
                                  minimumDecorationThickness,
                                  nullptr,
                                  0.0,
                                  false,
                                  globalIndex,
                                  isHorizontal,
                                  direction == KoSvgText::DirectionLeftToRight,
                                  false);

        // 8. Position on path

        debugFlake << "8. Position on path";
        this->applyTextPath(q, result, isHorizontal, startPos);
    } else {
        globalIndex = 0;
        debugFlake << "Computing text-decorationsfor inline-size";
        this->computeTextDecorations(q,
                                  result,
                                  logicalToVisual,
                                  minimumDecorationThickness,
                                  nullptr,
                                  0.0,
                                  false,
                                  globalIndex,
                                  isHorizontal,
                                  direction == KoSvgText::DirectionLeftToRight,
                                  true);
    }

    // 9. return result.
    debugFlake << "9. return result.";
    globalIndex = 0;
    QVector<CursorPos> cursorPos;
    Q_FOREACH (const KoSvgTextChunkShapeLayoutInterface::SubChunk &chunk, textChunks) {
        KoSvgText::AssociatedShapeWrapper wrapper = chunk.format.associatedShapeWrapper();
        const int j = chunk.text.size();
        for (int i = globalIndex; i < globalIndex + j; i++) {
            if (result.at(i).addressable && !result.at(i).middle) {

                if (result.at(i).plaintTextIndex > -1) {
                    QVector<QPointF> positions;
                    bool insertFirst = false;
                    if (result.at(i).anchored_chunk) {
                        CursorPos pos;
                        pos.cluster = i;
                        pos.index = result.at(i).plaintTextIndex;
                        insertFirst = true;
                        QPointF newOffset = result.at(i).cursorInfo.rtl? result.at(i).advance: QPointF();
                        result[i].cursorInfo.offsets.insert(0, newOffset);
                        positions.append(newOffset);
                        pos.offset = 0;
                        pos.synthetic = true;
                        cursorPos.append(pos);
                    }

                    int graphemes = result.at(i).cursorInfo.graphemeIndices.size();
                    for (int k = 0; k < graphemes; k++) {
                        if (result.at(i).breakType == BreakType::HardBreak && k+1 == graphemes) {
                            continue;
                        }
                        CursorPos pos;
                        pos.cluster = i;
                        pos.index = result.at(i).cursorInfo.graphemeIndices.at(k);
                        pos.offset = insertFirst? k+1: k;
                        cursorPos.append(pos);
                        QPointF offset = (k+1) * (result.at(i).advance/graphemes);
                        positions.append(result.at(i).cursorInfo.rtl? result.at(i).advance - offset: offset);
                    }
                    if (insertFirst) {
                        result[i].cursorInfo.graphemeIndices.insert(0, result[i].plaintTextIndex);
                    }
                    if (result.at(i).cursorInfo.offsets.size() < positions.size()) {
                        result[i].cursorInfo.offsets = positions;
                    }
                }


                if (!result.at(i).hidden) {
                    const QTransform tf = result.at(i).finalTransform();
                    wrapper.addCharacterRect(tf.mapRect(result.at(i).boundingBox));
                }
            }
        }
        globalIndex += j;
    }
    // figure out if we added a dummy, and if so add a pos for it.
    if (dummyIndex > -1 && dummyIndex < result.size()) {
        if (result.at(dummyIndex).anchored_chunk) {
            CursorPos pos;
            pos.cluster = dummyIndex;
            pos.index = result.at(dummyIndex).plaintTextIndex;
            result[dummyIndex].plaintTextIndex -= 1;
            result[dummyIndex].cursorInfo.offsets.insert(0, QPointF());
            pos.offset = 0;
            pos.synthetic = true;
            cursorPos.append(pos);
            if (!textChunks.isEmpty()) {
                textChunks.last().format.associatedShapeWrapper().addCharacterRect(result.at(dummyIndex).finalTransform().mapRect(result[dummyIndex].boundingBox));
            }
        }
    }
    this->initialTextPosition = startPos;
    this->plainText = plainText;
    this->result = result;
    this->cursorPos = cursorPos;
    this->logicalToVisualCursorPos = logicalToVisualCursorPositions(cursorPos, result, this->lineBoxes, direction == KoSvgText::DirectionLeftToRight);
}

void KoSvgTextShape::Private::clearAssociatedOutlines(const KoShape *rootShape)
{
    const KoSvgTextChunkShape *chunkShape = dynamic_cast<const KoSvgTextChunkShape *>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);

    chunkShape->layoutInterface()->clearAssociatedOutline();
    chunkShape->layoutInterface()->clearTextDecorations();

    Q_FOREACH (KoShape *child, chunkShape->shapes()) {
        clearAssociatedOutlines(child);
    }
}

/**
 * @brief KoSvgTextShape::Private::resolveTransforms
 * This resolves transforms and applies whitespace collapse.
 */
void KoSvgTextShape::Private::resolveTransforms(const KoShape *rootShape, QString text, QVector<CharacterResult> &result, int &currentIndex, bool isHorizontal, bool wrapped, bool textInPath, QVector<KoSvgText::CharTransformation> &resolved, QVector<bool> collapsedChars) {
    const KoSvgTextChunkShape *chunkShape = dynamic_cast<const KoSvgTextChunkShape *>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);

    QVector<KoSvgText::CharTransformation> local = chunkShape->layoutInterface()->localCharTransformations();

    int i = 0;

    int index = currentIndex;
    int j = index + chunkShape->layoutInterface()->numChars(true);

    if (chunkShape->layoutInterface()->textPath()) {
        textInPath = true;
    } else {
        for (int k = index; k < j; k++ ) {
            bool bidi = (text.at(k).unicode() >= 8234 && text.at(k).unicode() <= 8238)
                    || (text.at(k).unicode() >= 8294 && text.at(k).unicode() <= 8297);
            bool softHyphen = text.at(k) == QChar::SoftHyphen;

            // Apparantly when there's bidi controls in the text, they participate in line-wrapping,
            // so we don't check for it when wrapping.
            if (collapsedChars[k] || (bidi && !wrapped) || softHyphen) {
                result[k].addressable = false;
                continue;
            }

            if (i < local.size()) {
                KoSvgText::CharTransformation newTransform = local.at(i);
                newTransform.mergeInParentTransformation(resolved[k]);
                resolved[k] = newTransform;
                i += 1;
            } else if (k > 0) {
                if (resolved[k - 1].rotate) {
                    resolved[k].rotate = resolved[k - 1].rotate;
                }
            }
        }
    }

    Q_FOREACH (KoShape *child, chunkShape->shapes()) {
        resolveTransforms(child, text, result, currentIndex, isHorizontal, false, textInPath, resolved, collapsedChars);

    }

    if (chunkShape->layoutInterface()->textPath()) {
        bool first = true;
        for (int k = index; k < j; k++ ) {

            if (!result[k].addressable) {
                continue;
            }

            //  Also unset the first transform on a textPath to avoid breakage with rtl text.
            if (first) {
                if (isHorizontal) {
                    resolved[k].xPos = 0.0;
                } else {
                    resolved[k].yPos = 0.0;
                }
                first = false;
            }
            // x and y attributes are officially 'ignored' for text on path, though the algorithm
            // suggests this is only if a child of a path... In reality, not resetting this will
            // break text-on-path with rtl.
            if (isHorizontal) {
                resolved[k].yPos.reset();
            } else {
                resolved[k].xPos.reset();
            }
        }
    }

    currentIndex = j;

}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void KoSvgTextShape::Private::applyTextLength(const KoShape *rootShape,
                                              QVector<CharacterResult> &result,
                                              int &currentIndex,
                                              int &resolvedDescendentNodes,
                                              bool isHorizontal)
{
    const KoSvgTextChunkShape *chunkShape = dynamic_cast<const KoSvgTextChunkShape *>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);

    int i = currentIndex;
    int j = i + chunkShape->layoutInterface()->numChars(true);
    int resolvedChildren = 0;

    Q_FOREACH (KoShape *child, chunkShape->shapes()) {
        applyTextLength(child, result, currentIndex, resolvedChildren, isHorizontal);
    }
    // Raqm handles bidi reordering for us, but this algorithm does not
    // anticipate that, so we need to keep track of which typographic item
    // belongs where.
    QMap<int, int> visualToLogical;
    if (!chunkShape->layoutInterface()->textLength().isAuto) {
        qreal a = 0.0;
        qreal b = 0.0;
        int n = 0;
        for (int k = i; k < j; k++) {
            if (result.at(k).addressable) {
                if (result.at(k).visualIndex > -1) {
                    visualToLogical.insert(result.at(k).visualIndex, k);
                }
                // if character is linebreak, return;

                qreal pos = result.at(k).finalPosition.x();
                qreal advance = result.at(k).advance.x();
                if (!isHorizontal) {
                    pos = result.at(k).finalPosition.y();
                    advance = result.at(k).advance.y();
                }
                if (k == i) {
                    a = qMin(pos, pos + advance);
                    b = qMax(pos, pos + advance);
                } else {
                    a = qMin(a, qMin(pos, pos + advance));
                    b = qMax(b, qMax(pos, pos + advance));
                }
                if (!result.at(k).textLengthApplied) {
                    n += 1;
                }
            }
        }
        n += resolvedChildren;
        bool spacingAndGlyphs = (chunkShape->layoutInterface()->lengthAdjust() == KoSvgText::LengthAdjustSpacingAndGlyphs);
        if (!spacingAndGlyphs) {
            n -= 1;
        }
        const qreal delta = chunkShape->layoutInterface()->textLength().customValue - (b - a);

        const QPointF d = isHorizontal ? QPointF(delta / n, 0) : QPointF(0, delta / n);

        QPointF shift;
        bool secondTextLengthApplied = false;
        Q_FOREACH (int k, visualToLogical.keys()) {
            CharacterResult cr = result[visualToLogical.value(k)];
            if (cr.addressable) {
                cr.finalPosition += shift;
                if (spacingAndGlyphs) {
                    QPointF scale(d.x() != 0 ? (d.x() / cr.advance.x()) + 1 : 1.0, d.y() != 0 ? (d.y() / cr.advance.y()) + 1 : 1.0);
                    QTransform tf = QTransform::fromScale(scale.x(), scale.y());
                    // FIXME: What about other glyph formats?
                    if (auto *outlineGlyph = std::get_if<Glyph::Outline>(&cr.glyph)) {
                        outlineGlyph->path = tf.map(outlineGlyph->path);
                    }
                    cr.advance = tf.map(cr.advance);
                    cr.boundingBox = tf.mapRect(cr.boundingBox);
                }
                bool last = spacingAndGlyphs ? false : k == visualToLogical.keys().last();

                if (!(cr.textLengthApplied && secondTextLengthApplied) && !last) {
                    shift += d;
                }
                secondTextLengthApplied = cr.textLengthApplied;
                cr.textLengthApplied = true;
            }
            result[visualToLogical.value(k)] = cr;
        }
        resolvedDescendentNodes += 1;

        // apply the shift to all consequetive chars as long as they don't start
        // a new chunk.
        int lastVisualValue = visualToLogical.keys().last();
        visualToLogical.clear();

        for (int k = j; k < result.size(); k++) {
            if (result.at(k).anchored_chunk) {
                break;
            }
            visualToLogical.insert(result.at(k).visualIndex, k);
        }
        // And also backwards for rtl.
        for (int k = i; k > -1; k--) {
            visualToLogical.insert(result.at(k).visualIndex, k);
            if (result.at(k).anchored_chunk) {
                break;
            }
        }
        Q_FOREACH (int k, visualToLogical.keys()) {
            if (k > lastVisualValue) {
                result[visualToLogical.value(k)].finalPosition += shift;
            }
        }
    }

    currentIndex = j;
}

/**
 * @brief KoSvgTextShape::Private::computeFontMetrics
 * This function handles computing the baselineOffsets
 */

void KoSvgTextShape::Private::computeFontMetrics( // NOLINT(readability-function-cognitive-complexity)
    const KoShape *rootShape,
    const QMap<int, int> &parentBaselineTable,
    qreal parentFontSize,
    QPointF superScript,
    QPointF subScript,
    QVector<CharacterResult> &result,
    int &currentIndex,
    qreal res,
    bool isHorizontal)
{
    const KoSvgTextChunkShape *chunkShape = dynamic_cast<const KoSvgTextChunkShape *>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);

    QMap<int, int> baselineTable;
    const int i = currentIndex;
    const int j = qMin(i + chunkShape->layoutInterface()->numChars(true), result.size());

    KoSvgTextProperties properties = chunkShape->textProperties();

    const qreal fontSize = properties.propertyOrDefault(KoSvgTextProperties::FontSizeId).toReal();
    const qreal baselineShift = properties.property(KoSvgTextProperties::BaselineShiftValueId).toReal() * fontSize;
    QPointF baselineShiftTotal;
    KoSvgText::BaselineShiftMode baselineShiftMode = KoSvgText::BaselineShiftMode(properties.property(KoSvgTextProperties::BaselineShiftModeId).toInt());

    if (baselineShiftMode == KoSvgText::ShiftSuper) {
        baselineShiftTotal = isHorizontal ? superScript : QPointF(-superScript.y(), superScript.x());
    } else if (baselineShiftMode == KoSvgText::ShiftSub) {
        baselineShiftTotal = isHorizontal ? subScript : QPointF(-subScript.y(), subScript.x());
    } else if (baselineShiftMode == KoSvgText::ShiftPercentage) {
        // Positive baseline-shift goes up in the inline-direction, which is up in horizontal and right in vertical.
        baselineShiftTotal = isHorizontal ? QPointF(0, -baselineShift) : QPointF(baselineShift, 0);
    }

    QVector<int> lengths;
    const QFont::Style style = QFont::Style(properties.propertyOrDefault(KoSvgTextProperties::FontStyleId).toInt());
    KoSvgText::AutoValue fontSizeAdjust = properties.propertyOrDefault(KoSvgTextProperties::FontSizeAdjustId).value<KoSvgText::AutoValue>();
    if (properties.hasProperty(KoSvgTextProperties::KraTextVersionId)) {
        fontSizeAdjust.isAuto = (properties.property(KoSvgTextProperties::KraTextVersionId).toInt() < 3);
    }
    const std::vector<FT_FaceUP> faces = KoFontRegistry::instance()->facesForCSSValues(
        properties.property(KoSvgTextProperties::FontFamiliesId).toStringList(),
        lengths,
        properties.fontAxisSettings(),
        QString(),
        static_cast<quint32>(res),
        static_cast<quint32>(res),
        fontSize,
        fontSizeAdjust.isAuto ? 1.0 : fontSizeAdjust.customValue,
        properties.propertyOrDefault(KoSvgTextProperties::FontWeightId).toInt(),
        properties.propertyOrDefault(KoSvgTextProperties::FontStretchId).toInt(),
        style != QFont::StyleNormal);

    hb_font_t_up font(hb_ft_font_create_referenced(faces.front().data()));
    const qreal freetypePixelsToPt = (1.0 / 64.0) * (72. / res);

    hb_direction_t dir = HB_DIRECTION_LTR;
    if (!isHorizontal) {
        dir = HB_DIRECTION_TTB;
    }
    hb_script_t script = HB_SCRIPT_UNKNOWN;
    KoSvgText::Baseline dominantBaseline = KoSvgText::Baseline(properties.property(KoSvgTextProperties::DominantBaselineId).toInt());

    hb_position_t baseline = 0;
    KoSvgText::Baseline defaultBaseline = isHorizontal? KoSvgText::BaselineAlphabetic: KoSvgText::BaselineCentral;
    if (dominantBaseline == KoSvgText::BaselineResetSize && parentFontSize > 0) {
        baselineTable = parentBaselineTable;
        qreal multiplier = 1.0 / parentFontSize * fontSize;
        Q_FOREACH (int key, baselineTable.keys()) {
            baselineTable.insert(key, static_cast<int>(baselineTable.value(key) * multiplier));
        }
        dominantBaseline = KoSvgText::BaselineAuto;
    } else if (dominantBaseline == KoSvgText::BaselineNoChange) {
        baselineTable = parentBaselineTable;
        dominantBaseline = KoSvgText::BaselineAuto;
    } else {
        QMap<hb_ot_layout_baseline_tag_t, KoSvgText::Baseline> baselineList;
        baselineList.insert(HB_OT_LAYOUT_BASELINE_TAG_ROMAN, KoSvgText::BaselineAlphabetic);
        baselineList.insert(HB_OT_LAYOUT_BASELINE_TAG_MATH, KoSvgText::BaselineMathematical);
        baselineList.insert(HB_OT_LAYOUT_BASELINE_TAG_HANGING, KoSvgText::BaselineHanging);
        baselineList.insert(HB_OT_LAYOUT_BASELINE_TAG_IDEO_EMBOX_CENTRAL, KoSvgText::BaselineCentral);
        baselineList.insert(HB_OT_LAYOUT_BASELINE_TAG_IDEO_EMBOX_BOTTOM_OR_LEFT, KoSvgText::BaselineIdeographic);

        if (hb_version_atleast(4, 0, 0)) {
            hb_position_t origin = 0;
            hb_ot_layout_get_baseline_with_fallback(font.data(), baselineList.key(defaultBaseline), dir, script, HB_TAG_NONE, &origin);
            Q_FOREACH(hb_ot_layout_baseline_tag_t tag, baselineList.keys()) {
                hb_ot_layout_get_baseline_with_fallback(font.data(), tag, dir, script, HB_TAG_NONE, &baseline);
                baselineTable.insert(baselineList.value(tag), baseline - origin);
            }

            if (isHorizontal) {
                hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_X_HEIGHT, &baseline);
                baselineTable.insert(KoSvgText::BaselineMiddle, static_cast<int>((baseline - baselineTable.value(KoSvgText::BaselineAlphabetic)) * 0.5));
            } else {
                baselineTable.insert(KoSvgText::BaselineMiddle, baselineTable.value(KoSvgText::BaselineCentral));
            }
        } else {
            hb_position_t origin = 0;
            if (!isHorizontal) {
                // we'll need to calculate the central baseline manually, because there's no opentype tag associated with it, and
                // the harfbuzz tag is specific to HB 4.0 and up.

                hb_position_t over = 0.0;
                hb_position_t under = 0.0;
                bool hasOver = hb_ot_layout_get_baseline(font.data(), HB_OT_LAYOUT_BASELINE_TAG_IDEO_EMBOX_TOP_OR_RIGHT, dir, script, HB_TAG_NONE, &over);
                bool hasUnder = hb_ot_layout_get_baseline(font.data(), HB_OT_LAYOUT_BASELINE_TAG_IDEO_EMBOX_BOTTOM_OR_LEFT, dir, script, HB_TAG_NONE, &under);
                if (!hasOver || !hasUnder) {
                    hb_font_extents_t font_extents;
                    hb_font_get_extents_for_direction (font.data(), dir, &font_extents);
                    if (!hasOver) { over = font_extents.ascender;}
                    if (!hasUnder) { under = font_extents.descender;}
                }
                origin = (over + under) / 2;
            }

            Q_FOREACH(hb_ot_layout_baseline_tag_t tag, baselineList.keys()) {
                if (baselineList.value(tag) == defaultBaseline) {
                    baselineTable.insert(defaultBaseline, 0);
                } else {
                    baseline = 0;
                    hb_ot_layout_get_baseline(font.data(), tag, dir, script, HB_TAG_NONE, &baseline);
                    baselineTable.insert(baselineList.value(tag), baseline-origin);
                }
            }

            if (isHorizontal) {
                hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_X_HEIGHT, &baseline);
                baselineTable.insert(KoSvgText::BaselineMiddle, static_cast<int>((baseline - baselineTable.value(KoSvgText::BaselineAlphabetic)) * 0.5));
            } else {
                baselineTable.insert(KoSvgText::BaselineMiddle, baselineTable.value(KoSvgText::BaselineCentral));
            }
        }
    }

    // Get underline and super/subscripts.
    QPointF newSuperScript;
    QPointF newSubScript;
    if (hb_version_atleast(4, 0, 0)) {
        hb_position_t baseline2 = 0;
        hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_SUPERSCRIPT_EM_X_OFFSET, &baseline);
        hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_SUPERSCRIPT_EM_Y_OFFSET, &baseline2);
        newSuperScript = QPointF(baseline * freetypePixelsToPt, baseline2 * -freetypePixelsToPt);
        hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_SUBSCRIPT_EM_X_OFFSET, &baseline);
        hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_SUBSCRIPT_EM_Y_OFFSET, &baseline2);
        newSubScript = QPointF(baseline * freetypePixelsToPt, baseline2 * freetypePixelsToPt);

        qreal width = 0;
        qreal offset = 0;
        hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_UNDERLINE_SIZE, &baseline);
        width = baseline;
        hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_UNDERLINE_OFFSET, &baseline);
        offset = baseline;
        offset *= -freetypePixelsToPt;
        width *= freetypePixelsToPt;

        chunkShape->layoutInterface()->setTextDecorationFontMetrics(KoSvgText::DecorationUnderline, offset, width);
        chunkShape->layoutInterface()->setTextDecorationFontMetrics(KoSvgText::DecorationOverline, 0, width);

        hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_STRIKEOUT_SIZE, &baseline);
        width = baseline;
        hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_STRIKEOUT_OFFSET, &baseline);
        width *= freetypePixelsToPt;
        offset *= -freetypePixelsToPt;
        chunkShape->layoutInterface()->setTextDecorationFontMetrics(KoSvgText::DecorationLineThrough, offset, width);
    } else {
        baseline = 0;
        hb_position_t baseline2 = 0;
        hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_SUPERSCRIPT_EM_X_OFFSET, &baseline);
        hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_SUPERSCRIPT_EM_Y_OFFSET, &baseline2);
        if (baseline2 == 0) {
            newSuperScript = QPointF(0, 0.6 * -fontSize);
        } else {
            newSuperScript = QPointF(baseline * freetypePixelsToPt, baseline2 * -freetypePixelsToPt);
        }
        baseline = 0;
        baseline2 = 0;
        hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_SUBSCRIPT_EM_X_OFFSET, &baseline);
        hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_SUBSCRIPT_EM_Y_OFFSET, &baseline2);
        // Subscript should be 'added' onto the baseline'.
        if (baseline2 == 0) {
            newSubScript = QPointF(0, 0.2 * fontSize);
        } else {
            newSubScript = QPointF(baseline * freetypePixelsToPt, baseline2 * freetypePixelsToPt);
        }

        qreal width = 0;
        qreal offset = 0;
        const double fallbackThickness =
            faces.front()->underline_thickness * (faces.front()->size->metrics.y_scale / 65535.0);
        hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_UNDERLINE_SIZE, &baseline);
        width = qMax<double>(baseline, fallbackThickness);

        hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_UNDERLINE_OFFSET, &baseline);
        offset = baseline;
        offset *= -freetypePixelsToPt;
        width *= freetypePixelsToPt;

        chunkShape->layoutInterface()->setTextDecorationFontMetrics(KoSvgText::DecorationUnderline, offset, width);
        chunkShape->layoutInterface()->setTextDecorationFontMetrics(KoSvgText::DecorationOverline, 0, width);

        hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_STRIKEOUT_SIZE, &baseline);
        width = qMax<double>(baseline, fallbackThickness);
        hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_STRIKEOUT_OFFSET, &baseline);
        if (baseline == 0) {
            offset = baselineTable.value(KoSvgText::BaselineCentral);
        }
        width *= freetypePixelsToPt;
        offset *= -freetypePixelsToPt;

        chunkShape->layoutInterface()->setTextDecorationFontMetrics(KoSvgText::DecorationLineThrough, offset, width);
    }

    Q_FOREACH (KoShape *child, chunkShape->shapes()) {
        computeFontMetrics(child, baselineTable, fontSize, newSuperScript, newSubScript, result, currentIndex, res, isHorizontal);
    }

    KoSvgText::Baseline baselineAdjust = KoSvgText::Baseline(properties.property(KoSvgTextProperties::AlignmentBaselineId).toInt());

    if (baselineAdjust == KoSvgText::BaselineDominant) {
        baselineAdjust = dominantBaseline;
    }
    if (baselineAdjust == KoSvgText::BaselineAuto || baselineAdjust == KoSvgText::BaselineUseScript) {
        // UseScript got deprecated in CSS-Inline-3.
        baselineAdjust = defaultBaseline;
    }

    LineBox relevantLine;
    Q_FOREACH(LineBox lineBox, lineBoxes) {
        Q_FOREACH(LineChunk chunk, lineBox.chunks) {
            if (chunk.chunkIndices.contains(i)) {
                relevantLine = lineBox;
            }
        }
    }

    const int offset = parentBaselineTable.value(baselineAdjust, 0) - baselineTable.value(baselineAdjust, 0);
    QPointF shift = QPointF();

    if (baselineAdjust != KoSvgText::BaselineTextTop && baselineAdjust != KoSvgText::BaselineTextBottom) {
        if (isHorizontal) {
            shift = QPointF(0, offset * -freetypePixelsToPt);
        } else {
            shift = QPointF(offset * freetypePixelsToPt, 0);
        }
    }

    shift += baselineShiftTotal;

    for (int k = i; k < j; k++) {
        result[k].baselineOffset += shift;
    }

    currentIndex = j;
}

void KoSvgTextShape::Private::handleLineBoxAlignment(const KoShape *rootShape,
                                                     QVector<CharacterResult> &result,
                                                     QVector<LineBox> lineBoxes,
                                                     int &currentIndex,
                                                     bool isHorizontal)
{
    const KoSvgTextChunkShape *chunkShape = dynamic_cast<const KoSvgTextChunkShape *>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);

    const int i = currentIndex;
    const int j = qMin(i + chunkShape->layoutInterface()->numChars(true), result.size());

    KoSvgTextProperties properties = chunkShape->textProperties();
    KoSvgText::Baseline baselineAdjust = KoSvgText::Baseline(properties.property(KoSvgTextProperties::AlignmentBaselineId).toInt());

    Q_FOREACH (KoShape *child, chunkShape->shapes()) {
        handleLineBoxAlignment(child, result, lineBoxes, currentIndex, isHorizontal);
    }
    LineBox relevantLine;
    Q_FOREACH(LineBox lineBox, lineBoxes) {
        Q_FOREACH(LineChunk chunk, lineBox.chunks) {
            if (chunk.chunkIndices.contains(i)) {
                relevantLine = lineBox;
            }
        }
    }
    QPointF shift = QPointF();
    if (baselineAdjust == KoSvgText::BaselineTextTop || baselineAdjust == KoSvgText::BaselineTextBottom) {
        double ascent = 0.0;
        double descent = 0.0;
        for (int k = i; k < j; k++) {
            // The height calculation here is to remove the shifted-part height
            // from the top (or bottom) of the line.
            calculateLineHeight(result[k], ascent, descent, isHorizontal, true);
        }

        if (baselineAdjust == KoSvgText::BaselineTextTop) {
            shift = relevantLine.baselineTop;
            shift -= isHorizontal? QPointF(0, ascent):QPointF(ascent, 0);
        } else if (baselineAdjust == KoSvgText::BaselineTextBottom) {
            shift = relevantLine.baselineBottom;
            shift -= isHorizontal? QPointF(0, descent):QPointF(descent, 0);
        }
    }

    for (int k = i; k < j; k++) {
        CharacterResult cr = result[k];
        cr.cssPosition += shift;
        cr.finalPosition = cr.cssPosition;
        result[k] = cr;
    }

    currentIndex = j;
}

/**
 * @brief KoSvgTextShape::Private::computeTextDecorations
 * Text decorations need to be computed before textPath is applied.
 * This function goes down the tree and computes textDecorations as necessary,
 * bends them to the textPath, strokes them, and then adds them to the node in
 * question.
 */
void KoSvgTextShape::Private::computeTextDecorations( // NOLINT(readability-function-cognitive-complexity)
    const KoShape *rootShape,
    const QVector<CharacterResult> &result,
    const QMap<int, int> &logicalToVisual,
    qreal minimumDecorationThickness,
    KoPathShape *textPath,
    qreal textPathoffset,
    bool side,
    int &currentIndex,
    bool isHorizontal,
    bool ltr,
    bool wrapping)
{
    const KoSvgTextChunkShape *chunkShape = dynamic_cast<const KoSvgTextChunkShape *>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);

    const int i = currentIndex;
    const int j = qMin(i + chunkShape->layoutInterface()->numChars(true), result.size());
    using namespace KoSvgText;

    KoPathShape *currentTextPath = nullptr;
    qreal currentTextPathOffset = textPathoffset;
    bool textPathSide = side;
    if (!wrapping) {
        currentTextPath = textPath ? textPath : dynamic_cast<KoPathShape *>(chunkShape->layoutInterface()->textPath());

        if (chunkShape->layoutInterface()->textPath()) {
            textPathSide = chunkShape->layoutInterface()->textOnPathInfo().side == TextPathSideRight;
            if (chunkShape->layoutInterface()->textOnPathInfo().startOffsetIsPercentage) {
                KIS_ASSERT(currentTextPath);
                currentTextPathOffset = currentTextPath->outline().length() * (0.01 * chunkShape->layoutInterface()->textOnPathInfo().startOffset);
            } else {
                currentTextPathOffset = chunkShape->layoutInterface()->textOnPathInfo().startOffset;
            }
        }
    }

    Q_FOREACH (KoShape *child, chunkShape->shapes()) {
        computeTextDecorations(child,
                               result,
                               logicalToVisual,
                               minimumDecorationThickness,
                               currentTextPath,
                               currentTextPathOffset,
                               textPathSide,
                               currentIndex,
                               isHorizontal,
                               ltr,
                               wrapping);
    }

    TextDecorations decor = chunkShape->textProperties().propertyOrDefault(KoSvgTextProperties::TextDecorationLineId).value<TextDecorations>();
    if (decor != DecorationNone && chunkShape->textProperties().hasProperty(KoSvgTextProperties::TextDecorationLineId)) {
        KoSvgTextProperties properties = chunkShape->textProperties();
        TextDecorationStyle style = TextDecorationStyle(properties.propertyOrDefault(KoSvgTextProperties::TextDecorationStyleId).toInt());
        TextDecorationUnderlinePosition underlinePosH =
            TextDecorationUnderlinePosition(chunkShape->textProperties().propertyOrDefault(KoSvgTextProperties::TextDecorationPositionHorizontalId).toInt());
        TextDecorationUnderlinePosition underlinePosV =
            TextDecorationUnderlinePosition(chunkShape->textProperties().propertyOrDefault(KoSvgTextProperties::TextDecorationPositionVerticalId).toInt());

        QPainterPathStroker stroker;

        QMap<TextDecoration, QPainterPath> decorationPaths =
                generateDecorationPaths(chunkShape, i, j,
                                        result, stroker, isHorizontal, decor,
                                        minimumDecorationThickness, style, false, currentTextPath,
                                        currentTextPathOffset, textPathSide, underlinePosH, underlinePosV
                                        );

        // And finally add the paths to the chunkshape.

        Q_FOREACH (TextDecoration type, decorationPaths.keys()) {
            QPainterPath decorationPath = decorationPaths.value(type);
            if (!decorationPath.isEmpty()) {
                stroker.setWidth(qMax(minimumDecorationThickness, chunkShape->layoutInterface()->getTextDecorationWidth(type)));
                decorationPath = stroker.createStroke(decorationPath).simplified();
                chunkShape->layoutInterface()->addTextDecoration(type, decorationPath.simplified());
            }
        }
    }
    currentIndex = j;
}

QMap<KoSvgText::TextDecoration, QPainterPath>
KoSvgTextShape::Private::generateDecorationPaths(const KoSvgTextChunkShape *chunkShape,
                                                 const int &start, const int &end,
                                                 const QVector<CharacterResult> &result,
                                                 QPainterPathStroker &stroker,
                                                 const bool isHorizontal,
                                                 const KoSvgText::TextDecorations &decor,
                                                 const qreal &minimumDecorationThickness,
                                                 const KoSvgText::TextDecorationStyle style,
                                                 const bool textDecorationSkipInset,
                                                 const KoPathShape *currentTextPath,
                                                 const qreal currentTextPathOffset,
                                                 const bool textPathSide,
                                                 const KoSvgText::TextDecorationUnderlinePosition underlinePosH,
                                                 const KoSvgText::TextDecorationUnderlinePosition underlinePosV) {
    using namespace KoSvgText;

    QMap<TextDecoration, QPainterPath> decorationPaths;

    QMap<TextDecoration, QPointF> decorationOffsets;

    decorationPaths.insert(DecorationUnderline, QPainterPath());
    decorationPaths.insert(DecorationOverline, QPainterPath());
    decorationPaths.insert(DecorationLineThrough, QPainterPath());

    Q_FOREACH (TextDecoration type, decorationPaths.keys()) {
        qreal offset = chunkShape->layoutInterface()->getTextDecorationOffset(type);
        decorationOffsets.insert(type, isHorizontal ? QPointF(0, offset) : QPointF(offset, 0));
    }

    stroker.setWidth(qMax(minimumDecorationThickness, chunkShape->layoutInterface()->getTextDecorationWidth(DecorationUnderline)));
    stroker.setCapStyle(Qt::FlatCap);
    if (style == Dotted) {
        QPen pen;
        pen.setStyle(Qt::DotLine);
        stroker.setDashPattern(pen.dashPattern());
    } else if (style == Dashed) {
        QPen pen;
        pen.setStyle(Qt::DashLine);
        stroker.setDashPattern(pen.dashPattern());
    }
    qreal top = 0;
    qreal bottom = 0;
    QPointF currentFinalPos;
    QVector<QRectF> decorationRects;
    QVector<QPointF> firstPos;
    QRectF currentRect;

    for (int k = start; k < end; k++) {
        CharacterResult charResult = result.at(k);

        if (currentTextPath) {
            characterResultOnPath(charResult,
                                  currentTextPath->outline().length(),
                                  currentTextPathOffset,
                                  isHorizontal,
                                  currentTextPath->isClosedSubpath(0));
        }

        if (charResult.hidden || !charResult.addressable) {
            continue;
        }
        if (charResult.anchored_chunk) {
            QPointF fp = isHorizontal ? QPointF(currentRect.x(), currentFinalPos.y()) : QPointF(currentFinalPos.x(), currentRect.y());
            firstPos.append(fp);
            decorationRects.append(currentRect);
            currentRect = QRectF();
        }

        currentFinalPos = charResult.finalPosition;

        const QRectF bbox = charResult.boundingBox;

        top = isHorizontal ? qMin(top, bbox.top()) : qMax(top, bbox.right());
        bottom = isHorizontal ? qMax(bottom, bbox.bottom()) : qMin(bottom, bbox.left());

        currentRect |= bbox.translated(charResult.finalPosition);
    }
    decorationRects.append(currentRect);
    QPointF fp = isHorizontal ? QPointF(currentRect.x(), currentFinalPos.y()) : QPointF(currentFinalPos.x(), currentRect.y());
    firstPos.append(fp);

    // Computing the various offsets from the 'top' & 'bottom' values.

    bool underlineOverlineFlipped = false;
    if (isHorizontal) {
        decorationOffsets[DecorationOverline] = QPointF(0, top);
        if (underlinePosH == UnderlineUnder) {
            decorationOffsets[DecorationUnderline] = QPointF(0, bottom);
        }
    } else {
        if (underlinePosV == UnderlineRight) {
            decorationOffsets[DecorationOverline] = QPointF(bottom, 0);
            decorationOffsets[DecorationUnderline] = QPointF(top, 0);
            underlineOverlineFlipped = true;
        } else {
            decorationOffsets[DecorationOverline] = QPointF(top, 0);
            decorationOffsets[DecorationUnderline] = QPointF(bottom, 0);
        }
    }
    decorationOffsets[DecorationLineThrough] = (decorationOffsets.value(DecorationUnderline) + decorationOffsets.value(DecorationOverline)) * 0.5;

    // Now to create a QPainterPath for the given style that stretches
    // over a single decoration rect,
    // transform that and add it to the general paths.
    for (int i = 0; i < decorationRects.size(); i++) {
        QRectF rect = decorationRects.at(i);
        if (textDecorationSkipInset) {
            qreal inset = stroker.width() * 0.5;
            rect.adjust(-inset, -inset, inset, inset);
        }
        QPainterPath p;
        QPointF pathWidth;
        if (style != Wavy) {
            p.moveTo(QPointF());
            // We're segmenting the path here so it'll be easier to warp
            // when text-on-path is happening.
            if (currentTextPath) {
                if (isHorizontal) {
                    const qreal total = std::floor(rect.width() / (stroker.width() * 2));
                    const qreal segment = qreal(rect.width() / total);
                    for (int i = 0; i < total; i++) {
                        p.lineTo(p.currentPosition() + QPointF(segment, 0));
                    }
                } else {
                    const qreal total = std::floor(rect.height() / (stroker.width() * 2));
                    const qreal segment = qreal(rect.height() / total);
                    for (int i = 0; i < total; i++) {
                        p.lineTo(p.currentPosition() + QPointF(0, segment));
                    }
                }
            } else {
                if (isHorizontal) {
                    p.lineTo(rect.width(), 0);
                } else {
                    p.lineTo(0, rect.height());
                }
            }
        }
        if (style == Double) {
            qreal linewidthOffset = qMax(stroker.width() * 1.5, minimumDecorationThickness * 2);
            if (isHorizontal) {
                p.addPath(p.translated(0, linewidthOffset));
                pathWidth = QPointF(0, -linewidthOffset);
            } else {
                p.addPath(p.translated(linewidthOffset, 0));
                pathWidth = QPointF(linewidthOffset, 0);
            }

        } else if (style == Wavy) {
            qreal width = isHorizontal ? rect.width() : rect.height();
            qreal height = stroker.width() * 2;

            bool down = true;
            p.moveTo(QPointF());

            for (int i = 0; i < qFloor(width / height); i++) {
                if (down) {
                    p.lineTo(p.currentPosition().x() + height, height);
                } else {
                    p.lineTo(p.currentPosition().x() + height, 0);
                }
                down = !down;
            }
            qreal offset = fmod(width, height);
            if (down) {
                p.lineTo(width, offset);
            } else {
                p.lineTo(width, height - offset);
            }
            pathWidth = QPointF(0, -stroker.width());

            // Rotate for vertical.
            if (!isHorizontal) {
                for (int i = 0; i < p.elementCount(); i++) {
                    p.setElementPositionAt(i, p.elementAt(i).y - (stroker.width() * 2), p.elementAt(i).x);
                }
                pathWidth = QPointF(stroker.width(), 0);
            }
        }

        p.translate(firstPos.at(i).x(), firstPos.at(i).y());
        if (underlineOverlineFlipped) {
            decorationOffsets[DecorationUnderline] += pathWidth;
        } else {
            decorationOffsets[DecorationOverline] += pathWidth;
        }
        decorationOffsets[DecorationLineThrough] += (pathWidth * 0.5);

        Q_FOREACH (TextDecoration type, decorationPaths.keys()) {
            if (decor.testFlag(type)) {
                QPointF offset = decorationOffsets.value(type);

                if (currentTextPath) {
                    QPainterPath path = currentTextPath->outline();
                    path = currentTextPath->transformation().map(path);
                    if (textPathSide) {
                        path = path.toReversed();
                    }

                    decorationPaths[type].addPath(
                        stretchGlyphOnPath(p.translated(offset), path, isHorizontal, currentTextPathOffset, currentTextPath->isClosedSubpath(0)));
                } else {
                    decorationPaths[type].addPath(p.translated(offset));
                }
            }
        }
    }

    return decorationPaths;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void KoSvgTextShape::Private::applyAnchoring(QVector<CharacterResult> &result, bool isHorizontal)
{
    int i = 0;
    int start = 0;

    while (start < result.size()) {
        qreal a = 0;
        qreal b = 0;
        for (i = start; i < result.size(); i++) {
            if (!result.at(i).addressable) {
                continue;
            }
            if (result.at(i).anchored_chunk && i > start) {
                break;
            }
            qreal pos = isHorizontal ? result.at(i).finalPosition.x() : result.at(i).finalPosition.y();
            qreal advance = isHorizontal ? result.at(i).advance.x() : result.at(i).advance.y();

            if (result.at(i).anchored_chunk) {
                a = qMin(pos, pos + advance);
                b = qMax(pos, pos + advance);
            } else {
                a = qMin(a, qMin(pos, pos + advance));
                b = qMax(b, qMax(pos, pos + advance));
            }
        }

        const bool rtl = result.at(start).direction == KoSvgText::DirectionRightToLeft;
        qreal shift = isHorizontal ? result.at(start).finalPosition.x() : result.at(start).finalPosition.y();

        if ((result.at(start).anchor == KoSvgText::AnchorStart && !rtl) || (result.at(start).anchor == KoSvgText::AnchorEnd && rtl)) {
            shift = shift - a;

        } else if ((result.at(start).anchor == KoSvgText::AnchorEnd && !rtl) || (result.at(start).anchor == KoSvgText::AnchorStart && rtl)) {
            shift = shift - b;

        } else {
            shift = shift - (a + b) * 0.5;
        }

        const QPointF shiftP = isHorizontal ? QPointF(shift, 0) : QPointF(0, shift);

        for (int j = start; j < i; j++) {
            result[j].finalPosition += shiftP;
        }
        start = i;
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
qreal KoSvgTextShape::Private::characterResultOnPath(CharacterResult &cr,
                                                     qreal length,
                                                     qreal offset,
                                                     bool isHorizontal,
                                                     bool isClosed)
{
    const bool rtl = (cr.direction == KoSvgText::DirectionRightToLeft);
    qreal mid = cr.finalPosition.x() + (cr.advance.x() * 0.5) + offset;
    if (!isHorizontal) {
        mid = cr.finalPosition.y() + (cr.advance.y() * 0.5) + offset;
    }
    if (isClosed) {
        if ((cr.anchor == KoSvgText::AnchorStart && !rtl) || (cr.anchor == KoSvgText::AnchorEnd && rtl)) {
            if (mid - offset < 0 || mid - offset > length) {
                cr.hidden = true;
            }
        } else if ((cr.anchor == KoSvgText::AnchorEnd && !rtl) || (cr.anchor == KoSvgText::AnchorStart && rtl)) {
            if (mid - offset < -length || mid - offset > 0) {
                cr.hidden = true;
            }
        } else {
            if (mid - offset < -(length * 0.5) || mid - offset > (length * 0.5)) {
                cr.hidden = true;
            }
        }
        if (mid < 0) {
            mid += length;
        }
        mid = fmod(mid, length);
    } else {
        if (mid < 0 || mid > length) {
            cr.hidden = true;
        }
    }
    return mid;
}

QPainterPath KoSvgTextShape::Private::stretchGlyphOnPath(const QPainterPath &glyph,
                                                         const QPainterPath &path,
                                                         bool isHorizontal,
                                                         qreal offset,
                                                         bool isClosed)
{
    QPainterPath p = glyph;
    for (int i = 0; i < glyph.elementCount(); i++) {
        qreal mid = isHorizontal ? glyph.elementAt(i).x + offset : glyph.elementAt(i).y + offset;
        qreal midUnbound = mid;
        if (isClosed) {
            if (mid < 0) {
                mid += path.length();
            }
            mid = fmod(mid, qreal(path.length()));
            midUnbound = mid;
        } else {
            mid = qBound(0.0, mid, qreal(path.length()));
        }
        const qreal percent = path.percentAtLength(mid);
        const QPointF pos = path.pointAtPercent(percent);
        qreal tAngle = path.angleAtPercent(percent);
        if (tAngle > 180) {
            tAngle = 0 - (360 - tAngle);
        }
        const QPointF vectorT(qCos(qDegreesToRadians(tAngle)), -qSin(qDegreesToRadians(tAngle)));
        QPointF finalPos = pos;
        if (isHorizontal) {
            QPointF vectorN(-vectorT.y(), vectorT.x());
            const qreal o = mid - (midUnbound);
            finalPos = pos - (o * vectorT) + (glyph.elementAt(i).y * vectorN);
        } else {
            QPointF vectorN(vectorT.y(), -vectorT.x());
            const qreal o = mid - (midUnbound);
            finalPos = pos - (o * vectorT) + (glyph.elementAt(i).x * vectorN);
        }
        p.setElementPositionAt(i, finalPos.x(), finalPos.y());
    }
    return p;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void KoSvgTextShape::Private::applyTextPath(const KoShape *rootShape,
                                            QVector<CharacterResult> &result,
                                            bool isHorizontal,
                                            QPointF &startPos)
{
    // Unlike all the other applying functions, this one only iterrates over the
    // top-level. SVG is not designed to have nested textPaths. Source:
    // https://github.com/w3c/svgwg/issues/580
    const KoSvgTextChunkShape *chunkShape = dynamic_cast<const KoSvgTextChunkShape *>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);
    bool inPath = false;
    bool afterPath = false;
    int currentIndex = 0;
    QPointF pathEnd;
    Q_FOREACH (KoShape *child, chunkShape->shapes()) {
        const KoSvgTextChunkShape *textPathChunk = dynamic_cast<const KoSvgTextChunkShape *>(child);
        KIS_SAFE_ASSERT_RECOVER_RETURN(textPathChunk);
        int endIndex = currentIndex + textPathChunk->layoutInterface()->numChars(true);

        KoPathShape *shape = dynamic_cast<KoPathShape *>(textPathChunk->layoutInterface()->textPath());
        if (shape) {
            QPainterPath path = shape->outline();
            path = shape->transformation().map(path);
            inPath = true;
            if (textPathChunk->layoutInterface()->textOnPathInfo().side == KoSvgText::TextPathSideRight) {
                path = path.toReversed();
            }
            qreal length = path.length();
            qreal offset = 0.0;
            bool isClosed = (shape->isClosedSubpath(0) && shape->subpathCount() == 1);
            if (textPathChunk->layoutInterface()->textOnPathInfo().startOffsetIsPercentage) {
                offset = length * (0.01 * textPathChunk->layoutInterface()->textOnPathInfo().startOffset);
            } else {
                offset = textPathChunk->layoutInterface()->textOnPathInfo().startOffset;
            }
            bool stretch = textPathChunk->layoutInterface()->textOnPathInfo().method == KoSvgText::TextPathStretch;

            if (child == chunkShape->shapes().first()) {
                const qreal percent = path.percentAtLength(offset);
                startPos = path.pointAtPercent(percent);
            }

            for (int i = currentIndex; i < endIndex; i++) {
                CharacterResult cr = result[i];

                if (!cr.middle) {
                    const qreal mid = characterResultOnPath(cr, length, offset, isHorizontal, isClosed);
                    if (!cr.hidden) {
                        auto *outlineGlyph = std::get_if<Glyph::Outline>(&cr.glyph);
                        // FIXME: What about other glyph formats?
                        if (stretch && outlineGlyph) {
                            const QTransform tf = cr.finalTransform();
                            QPainterPath glyph = stretchGlyphOnPath(tf.map(outlineGlyph->path), path, isHorizontal, offset, isClosed);
                            outlineGlyph->path = glyph;
                        }
                        const qreal percent = path.percentAtLength(mid);
                        const QPointF pos = path.pointAtPercent(percent);
                        qreal tAngle = path.angleAtPercent(percent);
                        if (tAngle > 180) {
                            tAngle = 0 - (360 - tAngle);
                        }
                        const QPointF vectorT(qCos(qDegreesToRadians(tAngle)), -qSin(qDegreesToRadians(tAngle)));
                        if (isHorizontal) {
                            cr.rotate -= qDegreesToRadians(tAngle);
                            QPointF vectorN(-vectorT.y(), vectorT.x());
                            const qreal o = (cr.advance.x() * 0.5);
                            cr.finalPosition = pos - (o * vectorT) + (cr.finalPosition.y() * vectorN);
                        } else {
                            cr.rotate -= qDegreesToRadians(tAngle + 90);
                            QPointF vectorN(vectorT.y(), -vectorT.x());
                            const qreal o = (cr.advance.y() * 0.5);
                            cr.finalPosition = pos - (o * vectorT) + (cr.finalPosition.x() * vectorN);
                        }
                        // FIXME: What about other glyph formats?
                        if (stretch && outlineGlyph) {
                            const QTransform tf = cr.finalTransform();
                            outlineGlyph->path = tf.inverted().map(outlineGlyph->path);
                        }
                    }
                }
                result[i] = cr;
            }
            pathEnd = path.pointAtPercent(1.0);
        } else {
            if (inPath) {
                inPath = false;
                afterPath = true;
                pathEnd -= result.at(currentIndex).finalPosition;
            }
            if (afterPath) {
                for (int i = currentIndex; i < endIndex; i++) {
                    CharacterResult cr = result[i];
                    if (cr.anchored_chunk) {
                        afterPath = false;
                    } else {
                        cr.finalPosition += pathEnd;
                        result[i] = cr;
                    }
                }
            }
        }
        currentIndex = endIndex;
    }
}
