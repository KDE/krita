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
#include "KoSvgTextProperties.h"
#include "KoColorBackground.h"
#include "KoWritingSystemUtils.h"

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

using raqm_t_sp = KisLibraryResourcePointer<raqm_t, raqm_destroy>;

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

QString langToLibUnibreakLang(const QString lang) {
    // Libunibreak only tests against "ko", "ja" and "zh".
    QLocale locale = KoWritingSystemUtils::localeFromBcp47Locale(lang);
    if (locale.language() == QLocale::Japanese || locale.script() == QLocale::JapaneseScript) {
        return "ja";
    } else if (locale.language() == QLocale::Korean || locale.script() == QLocale::KoreanScript || locale.script() == QLocale::HangulScript) {
        return "ko";
    } else if (locale.script() == QLocale::SimplifiedChineseScript
               || locale.script() == QLocale::TraditionalChineseScript
               || locale.script() == QLocale::HanWithBopomofoScript
               || locale.script() == QLocale::HanScript
               || locale.script() == QLocale::BopomofoScript) {
        return "zh";
    }
    return lang;
}


void KoSvgTextShape::Private::updateTextWrappingAreas()
{
    KoSvgTextProperties rootProperties = textData.empty()? KoSvgTextProperties::defaultProperties(): textData.childBegin()->properties;
    currentTextWrappingAreas = getShapes(shapesInside, shapesSubtract, rootProperties);
    relayout();
}

QList<QPainterPath> KoSvgTextShape::Private::generateShapes(const QList<KoShape *> shapesInside, const QList<KoShape *> shapesSubtract, const KoSvgTextProperties &properties)
{
    return getShapes(shapesInside, shapesSubtract, properties);
}
// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void KoSvgTextShape::Private::relayout()
{
    clearAssociatedOutlines();
    this->initialTextPosition = QPointF();
    this->result.clear();
    this->cursorPos.clear();
    this->logicalToVisualCursorPos.clear();

    bool disableFontMatching = this->disableFontMatching;

    if (KisForestDetail::size(textData) == 0) {
        return;
    }
    // The following is based on the text-layout algorithm in SVG 2.
    KoSvgTextProperties rootProperties = textData.childBegin()->properties;
    rootProperties.inheritFrom(KoSvgTextProperties::defaultProperties(), true);
    KoSvgText::WritingMode writingMode = KoSvgText::WritingMode(rootProperties.propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());
    KoSvgText::Direction direction = KoSvgText::Direction(rootProperties.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
    KoSvgText::AutoValue inlineSize = rootProperties.propertyOrDefault(KoSvgTextProperties::InlineSizeId).value<KoSvgText::AutoValue>();
    const KoSvgText::TextRendering textRendering = KoSvgText::TextRendering(rootProperties.propertyOrDefault(KoSvgTextProperties::TextRenderingId).toInt());
    QString lang = rootProperties.property(KoSvgTextProperties::TextLanguage).toString();

    const bool isHorizontal = writingMode == KoSvgText::HorizontalTB;

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

    // Setup the resolution handler.
    const bool horzSnapping = textRendering == KoSvgText::RenderingOptimizeSpeed || (textRendering == KoSvgText::RenderingOptimizeLegibility && !isHorizontal);
    const bool vertSnapping = textRendering != KoSvgText::RenderingGeometricPrecision && textRendering != KoSvgText::RenderingAuto;
    const KoSvgText::ResolutionHandler resHandler(this->xRes, this->yRes, horzSnapping, vertSnapping);

    // First, get text. We use the subChunks because that handles bidi-insertion for us.

    bool _ignore = false;
    const QVector<SubChunk> textChunks =
        collectSubChunks(textData.childBegin(), KoSvgTextProperties::defaultProperties(),false, _ignore);
    QString text;
    QVector<QPair<int, int>> clusterToOriginalString;
    QMap<int, KoSvgText::TextSpaceCollapse> collapseModes;
    QString plainText;
    Q_FOREACH (const SubChunk &chunk, textChunks) {
        for (int i = 0; i < chunk.newToOldPositions.size(); i++) {
            QPair<int, int> pos = chunk.newToOldPositions.at(i);
            int a = pos.second < 0? -1: text.size()+pos.second;
            int b = pos.first < 0? -1: plainText.size()+pos.first;
            QPair<int, int> newPos = QPair<int, int> (a, b);
            clusterToOriginalString.append(newPos);
        }
        KoSvgText::TextSpaceCollapse collapse = KoSvgText::TextSpaceCollapse(chunk.inheritedProps.propertyOrDefault(KoSvgTextProperties::TextCollapseId).toInt());
        collapseModes.insert(text.size(), collapse);
        text.append(chunk.text);
        plainText.append(chunk.originalText);
    }
    QVector<bool> collapseChars = KoCssTextUtils::collapseSpaces(&text, collapseModes);
    debugFlake << "Laying out the following text: " << text;

    // 1. Setup.
    KoSvgText::TextWrap wrap = KoSvgText::TextWrap(rootProperties.propertyOrDefault(KoSvgTextProperties::TextWrapId).toInt());
    KoSvgText::LineBreak linebreakStrictness = KoSvgText::LineBreak(rootProperties.property(KoSvgTextProperties::LineBreakId).toInt());

    QVector<QPair<bool, bool>> justify;
    QVector<char> lineBreaks(text.size());
    QVector<char> wordBreaks(text.size());
    QVector<char> graphemeBreaks(text.size());
    if (text.size() > 0) {
        // TODO: Figure out how to gracefully skip all the next steps when the text-size is 0.
        // can't currently remember if removing the associated outlines was all that is necessary.
        QString unibreakLang = langToLibUnibreakLang(lang);
        if (!lang.isEmpty()) {
            // Libunibreak currently only has support for strict, and even then only
            // for very specific cases.
            if (linebreakStrictness == KoSvgText::LineBreakStrict) {
                unibreakLang += "-strict";
            }
        }
        set_linebreaks_utf16(text.utf16(), static_cast<size_t>(text.size()), unibreakLang.toUtf8().data(), lineBreaks.data());
        set_wordbreaks_utf16(text.utf16(), static_cast<size_t>(text.size()), unibreakLang.toUtf8().data(), wordBreaks.data());
        set_graphemebreaks_utf16(text.utf16(), static_cast<size_t>(text.size()), unibreakLang.toUtf8().data(), graphemeBreaks.data());
        justify = KoCssTextUtils::justificationOpportunities(text, lang);
    }


    int globalIndex = 0;
    QVector<CharacterResult> result(text.size());
    // HACK ALERT!
    // Apparently feeding a bidi algorithm a hardbreak makes it go 'ok, not doing any
    // bidi', which makes sense, Bidi is supposed to be done 'after' line breaking.
    // Without replacing hardbreaks with spaces, hardbreaks in rtl will break the bidi.
    for (int i = 0; i < text.size(); i++) {
        if (lineBreaks[i] == LINEBREAK_MUSTBREAK) {
            text[i] = QChar::Space;
        }
    }
    for (int i=0; i < clusterToOriginalString.size(); i++) {
        QPair<int, int> mapping = clusterToOriginalString.at(i);
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
    bool wrapped = !(inlineSize.isAuto && this->shapesInside.isEmpty());
    if (!resolvedTransforms.isEmpty()) {
        resolvedTransforms[0].xPos = 0;
        resolvedTransforms[0].yPos = 0;
    }
    this->resolveTransforms(textData.childBegin(), text, result, globalIndex, isHorizontal, wrapped, false, resolvedTransforms, collapseChars, KoSvgTextProperties::defaultProperties(), true);

    // pass everything to a css-compatible text-layout algorithm.
    raqm_t_sp layout(raqm_create());

    if (raqm_set_text_utf16(layout.data(), text.utf16(), static_cast<size_t>(text.size()))) {
        if (writingMode == KoSvgText::VerticalRL || writingMode == KoSvgText::VerticalLR) {
            raqm_set_par_direction(layout.data(), raqm_direction_t::RAQM_DIRECTION_TTB);
        } else if (direction == KoSvgText::DirectionRightToLeft) {
            raqm_set_par_direction(layout.data(), raqm_direction_t::RAQM_DIRECTION_RTL);
        } else {
            raqm_set_par_direction(layout.data(), raqm_direction_t::RAQM_DIRECTION_LTR);
        }

        int start = 0;
        Q_FOREACH (const SubChunk &chunk, textChunks) {
            int length = chunk.text.size();
            KoSvgTextProperties properties = chunk.inheritedProps;

            // In this section we retrieve the resolved transforms and
            // direction/anchoring that we can get from the subchunks.
            KoSvgText::TextAnchor anchor = KoSvgText::TextAnchor(properties.propertyOrDefault(KoSvgTextProperties::TextAnchorId).toInt());
            KoSvgText::Direction direction = KoSvgText::Direction(properties.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
            KoSvgText::WordBreak wordBreakStrictness = KoSvgText::WordBreak(properties.propertyOrDefault(KoSvgTextProperties::WordBreakId).toInt());
            KoSvgText::HangingPunctuations hang =
                properties.propertyOrDefault(KoSvgTextProperties::HangingPunctuationId).value<KoSvgText::HangingPunctuations>();
            KoSvgText::TabSizeInfo tabInfo = properties.propertyOrDefault(KoSvgTextProperties::TabSizeId).value<KoSvgText::TabSizeInfo>();
            KoSvgText::AutoLengthPercentage letterSpacing = properties.propertyOrDefault(KoSvgTextProperties::LetterSpacingId).value<KoSvgText::AutoLengthPercentage>();
            KoSvgText::AutoLengthPercentage wordSpacing = properties.propertyOrDefault(KoSvgTextProperties::WordSpacingId).value<KoSvgText::AutoLengthPercentage>();
            bool overflowWrap = KoSvgText::OverflowWrap(properties.propertyOrDefault(KoSvgTextProperties::OverflowWrapId).toInt()) != KoSvgText::OverflowWrapNormal;
            KoSvgText::TextSpaceCollapse collapse = KoSvgText::TextSpaceCollapse(properties.propertyOrDefault(KoSvgTextProperties::TextCollapseId).toInt());

            KoSvgText::LineBreak localLinebreakStrictness = KoSvgText::LineBreak(properties.property(KoSvgTextProperties::LineBreakId).toInt());
            QString localLang = properties.property(KoSvgTextProperties::TextLanguage).toString();

            if ((localLinebreakStrictness != linebreakStrictness && localLinebreakStrictness != KoSvgText::LineBreakAnywhere) || localLang != lang ) {
                QString unibreakLang = langToLibUnibreakLang(localLang);
                if (localLinebreakStrictness == KoSvgText::LineBreakStrict && !unibreakLang.isEmpty()) {
                    unibreakLang += "-strict";
                }
                int localLineBreakStart = qMax(0, start -1);
                int localLineBreakEnd = qMin(text.size(), start+chunk.text.size());
                QVector<char> localLineBreaks(localLineBreakEnd - localLineBreakStart);
                set_linebreaks_utf16(text.mid(localLineBreakStart, localLineBreaks.size()).utf16(),
                                     static_cast<size_t>(localLineBreaks.size()),
                                     unibreakLang.toUtf8().data(),
                                     localLineBreaks.data());
                for (int i = 0; i < localLineBreaks.size(); i++) {
                    if (i < (start - localLineBreakStart)) continue;
                    if (i+localLineBreakStart > start+chunk.text.size()) break;
                    lineBreaks[i+localLineBreakStart] = localLineBreaks[i];
                }
            }

            KoColorBackground *b = dynamic_cast<KoColorBackground *>(chunk.bg.data());
            QColor fillColor;
            if (b)
            {
                fillColor = b->color();
            }
            if (!letterSpacing.isAuto) {
                tabInfo.extraSpacing += letterSpacing.length.value;
            }
            if (!wordSpacing.isAuto) {
                tabInfo.extraSpacing += wordSpacing.length.value;
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

                    } else if (collapse == KoSvgText::BreakSpaces && wrap != KoSvgText::NoWrap && KoCssTextUtils::IsCssWordSeparator(QString(text.at(start + i)))) {
                        cr.breakType = BreakType::SoftBreak;
                    }
                }

                if ((wordBreakStrictness == KoSvgText::WordBreakBreakAll ||
                     localLinebreakStrictness == KoSvgText::LineBreakAnywhere)
                        && wrap != KoSvgText::NoWrap) {
                    if (graphemeBreaks[start + i] == GRAPHEMEBREAK_BREAK && cr.breakType == BreakType::NoBreak) {
                        cr.breakType = BreakType::SoftBreak;
                    }
                } else if (wordBreakStrictness == KoSvgText::WordBreakKeepAll) {
                    cr.breakType = (wordBreaks[start + i] == WORDBREAK_BREAK)? cr.breakType: BreakType::NoBreak;
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

            const KoSvgText::CssFontStyleData style = properties.propertyOrDefault(KoSvgTextProperties::FontStyleId).value<KoSvgText::CssFontStyleData>();
            bool synthesizeWeight = properties.propertyOrDefault(KoSvgTextProperties::FontSynthesisBoldId).toBool();
            bool synthesizeStyle = properties.propertyOrDefault(KoSvgTextProperties::FontSynthesisItalicId).toBool();

            const std::vector<FT_FaceSP> faces = KoFontRegistry::instance()->facesForCSSValues(
                lengths,
                properties.cssFontInfo(),
                chunk.text,
                static_cast<quint32>(resHandler.xRes),
                static_cast<quint32>(resHandler.yRes),
                disableFontMatching);
            const qreal fontSize = properties.cssFontInfo().size;
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
                                              static_cast<int>(letterSpacing.length.value * resHandler.freeTypePixel * resHandler.pointToPixelFactor(isHorizontal)),
                                              static_cast<size_t>(start),
                                              static_cast<size_t>(length));
            }

            if (!wordSpacing.isAuto) {
                raqm_set_word_spacing_range(layout.data(),
                                            static_cast<int>(wordSpacing.length.value * resHandler.freeTypePixel * resHandler.pointToPixelFactor(isHorizontal)),
                                            static_cast<size_t>(start),
                                            static_cast<size_t>(length));
            }

            for (int i = 0; i < lengths.size(); i++) {
                length = lengths.at(i);
                const FT_FaceSP &face = faces.at(static_cast<size_t>(i));
                const FT_Int32 faceLoadFlags = KoFontRegistry::loadFlagsForFace(face.data(), isHorizontal, 0, textRendering);
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

                QHash<QChar::Script, KoSvgText::FontMetrics> metricsList;
                for (int j=start; j<start+length; j++) {
                    const QChar::Script currentScript = QChar::script(getUcs4At(text, j));
                    if (!metricsList.contains(currentScript)) {
                        metricsList.insert(currentScript, KoFontRegistry::generateFontMetrics(face, isHorizontal, KoWritingSystemUtils::scriptTagForQCharScript(currentScript), textRendering));
                    }
                    result[j].metrics = metricsList.value(currentScript);
                    if (fontSize < 1.0) {
                        result[j].extraFontScaling = fontSize;
                    }

                    const KoSvgText::FontMetrics currentMetrics = properties.applyLineHeight(result[j].metrics);

                    if (text.at(j) == QChar::Tabulation) {
                        qreal tabSize = 0;
                        if (tabInfo.isNumber) {
                            // Try to avoid Nan situations.
                            if (result[j].metrics.spaceAdvance > 0) {
                                tabSize = (result[j].metrics.spaceAdvance + (tabInfo.extraSpacing*resHandler.freeTypePixel)) * tabInfo.value;
                            } else {
                                tabSize = ((result[j].metrics.fontSize/2) + (tabInfo.extraSpacing*resHandler.freeTypePixel)) * tabInfo.value;
                            }
                        } else {
                            tabSize = tabInfo.length.value * resHandler.freeTypePixel;
                        }
                        result[j].tabSize = tabSize;
                    }

                    result[j].fontHalfLeading = currentMetrics.lineGap / 2;
                    result[j].fontStyle = synthesizeStyle? style.style: QFont::StyleNormal;
                    result[j].fontWeight = synthesizeWeight? properties.propertyOrDefault(KoSvgTextProperties::FontWeightId).toInt(): 400;
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
    // We also retrieve a glyph path here.
    size_t count = 0;
    const raqm_glyph_t *glyphs = raqm_get_glyphs(layout.data(), &count);
    if (!glyphs) {
        return;
    }

    QPointF totalAdvanceFTFontCoordinates;
    QMap<int, int> logicalToVisual;
    this->isBidi = false;


    KIS_ASSERT(count <= INT32_MAX);

    // For detecting ligatures.
    int previousGlyph = -1;
    int previousCluster = -1;

    for (int i = 0; i < static_cast<int>(count); i++) {
        raqm_glyph_t currentGlyph = glyphs[i];
        KIS_ASSERT(currentGlyph.cluster <= INT32_MAX);
        const int cluster = static_cast<int>(currentGlyph.cluster);
        if (!result[cluster].addressable) {
            continue;
        }
        CharacterResult charResult = result[cluster];

        const FT_Int32 faceLoadFlags = KoFontRegistry::loadFlagsForFace(currentGlyph.ftface, isHorizontal, 0, textRendering);


        const char32_t codepoint = getUcs4At(text, cluster);
        debugFlake << "glyph" << i << "cluster" << cluster << currentGlyph.index << codepoint;

        charResult.cursorInfo.rtl = raqm_get_direction_at_index(layout.data(), cluster) == RAQM_DIRECTION_RTL;
        if (charResult.cursorInfo.rtl != (charResult.direction == KoSvgText::DirectionRightToLeft)) {
            this->isBidi = true;
        }

        if (!this->loadGlyph(resHandler,
                             faceLoadFlags,
                             isHorizontal,
                             codepoint,
                             textRendering,
                             currentGlyph,
                             charResult,
                             totalAdvanceFTFontCoordinates)) {
            continue;
        }

        /// Ligature caret testing. We try to test when there's a gap between clusters and glyphs.
        /// This gap happens because a single glyph covers multiple clusters.
        /// We do this because testing for caret positions is surprisingly heavy.
        if (((cluster - previousCluster) > (i-previousGlyph)) && previousCluster >= 0) {
            raqm_glyph_t previousGlyph = glyphs[i];
            result[previousCluster].cursorInfo.offsets = getLigatureCarets(resHandler, isHorizontal, previousGlyph);
        }
        // Always test last one.
        if (i+1 == count) {
            charResult.cursorInfo.offsets = getLigatureCarets(resHandler, isHorizontal, currentGlyph);
        }

        charResult.visualIndex = i;
        logicalToVisual.insert(cluster, i);

        charResult.middle = false;

        result[cluster] = charResult;
        if (previousCluster != cluster) {
            previousGlyph = i;
            previousCluster = cluster;
        }
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
        dummy.inkBoundingBox = hardbreak.inkBoundingBox;
        if (isHorizontal) {
            dummy.scaleCharacterResult(0.0, 1.0);
        } else {
            dummy.scaleCharacterResult(1.0, 0.0);
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
    this->computeFontMetrics(textData.childBegin(), KoSvgTextProperties::defaultProperties(), KoSvgText::FontMetrics(), KoSvgText::BaselineAuto, QPointF(), QPointF(), result, globalIndex, resHandler, isHorizontal, disableFontMatching);

    // Handle linebreaking.
    QPointF startPos = resolvedTransforms.value(0).absolutePos() - result.value(0).dominantBaselineOffset;
    if (!this->currentTextWrappingAreas.isEmpty()) {
        this->lineBoxes = flowTextInShapes(rootProperties, logicalToVisual, result, this->currentTextWrappingAreas, startPos, resHandler);
    } else {
        this->lineBoxes = breakLines(rootProperties, logicalToVisual, result, startPos, resHandler);
    }

    // Handle baseline alignment.
    globalIndex = 0;
    this->handleLineBoxAlignment(textData.childBegin(), result, this->lineBoxes, globalIndex, isHorizontal, KoSvgTextProperties::defaultProperties());

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
        this->applyTextLength(textData.childBegin(), result, globalIndex, resolved, isHorizontal, KoSvgTextProperties::defaultProperties(), resHandler);

        // 6. Adjust positions: x, y
        debugFlake << "6. Adjust positions: x, y";
        // https://github.com/w3c/svgwg/issues/617
        shift = QPointF();
        for (int i = 0; i < result.size(); i++) {
            if (result.at(i).addressable) {
                KoSvgText::CharTransformation transform = resolvedTransforms[i];
                CharacterResult charResult = result[i];
                if (transform.xPos) {
                    const qreal delta = charResult.baselineOffset.x() + (transform.dxPos ? *transform.dxPos : 0.0);
                    shift.setX(*transform.xPos + (delta - charResult.finalPosition.x()));
                }
                if (transform.yPos) {
                    const qreal delta = charResult.baselineOffset.y() + (transform.dyPos ? *transform.dyPos : 0.0);
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
        applyAnchoring(result, isHorizontal, resHandler);

        // Computing the textDecorations needs to happen before applying the
        // textPath to the results, as we need the unapplied result vector for
        // positioning.
        debugFlake << "Now Computing text-decorations";
        globalIndex = 0;
        this->computeTextDecorations(textData.childBegin(),
                                  result,
                                  logicalToVisual,
                                  resHandler,
                                  nullptr,
                                  0.0,
                                  false,
                                  globalIndex,
                                  isHorizontal,
                                  direction == KoSvgText::DirectionLeftToRight,
                                  false, KoSvgTextProperties::defaultProperties(), this->textPaths);

        // 8. Position on path

        debugFlake << "8. Position on path";
        this->applyTextPath(textData.childBegin(), result, isHorizontal, startPos, KoSvgTextProperties::defaultProperties(), this->textPaths);
    } else {
        globalIndex = 0;
        debugFlake << "Computing text-decorationsfor inline-size";
        this->computeTextDecorations(textData.childBegin(),
                                  result,
                                  logicalToVisual,
                                  resHandler,
                                  nullptr,
                                  0.0,
                                  false,
                                  globalIndex,
                                  isHorizontal,
                                  direction == KoSvgText::DirectionLeftToRight,
                                  true, KoSvgTextProperties::defaultProperties(), this->textPaths);
    }

    // 9. return result.
    debugFlake << "9. return result.";
    globalIndex = 0;
    QVector<CursorPos> cursorPos;
    for (auto chunk = textChunks.begin(); chunk != textChunks.end(); chunk++) {
        const int j = chunk->text.size();
        chunk->associatedLeaf->finalResultIndex = (globalIndex+j);
        for (int i = globalIndex; i < chunk->associatedLeaf->finalResultIndex; i++) {
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
                    chunk->associatedLeaf->associatedOutline.addRect(tf.mapRect(result.at(i).inkBoundingBox));
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
                textChunks.last().associatedLeaf->associatedOutline.addRect(result.at(dummyIndex).finalTransform().mapRect(result[dummyIndex].inkBoundingBox));
                textChunks.last().associatedLeaf->finalResultIndex = result.size();
            }
        }
    }
    this->initialTextPosition = startPos;
    this->plainText = plainText;
    this->result = result;
    this->cursorPos = cursorPos;
    this->logicalToVisualCursorPos = logicalToVisualCursorPositions(cursorPos, result, this->lineBoxes, direction == KoSvgText::DirectionLeftToRight);
}

void KoSvgTextShape::Private::clearAssociatedOutlines()
{
    for (auto it = textData.depthFirstTailBegin(); it != textData.depthFirstTailEnd(); it++) {
        it->associatedOutline = QPainterPath();
        it->textDecorations.clear();
        it->finalResultIndex = -1;
    }
}

/**
 * @brief KoSvgTextShape::Private::resolveTransforms
 * This resolves transforms and applies whitespace collapse.
 */
const QString bidiControls = "\u202a\u202b\u202c\u202d\u202e\u2066\u2067\u2068\u2069";
void KoSvgTextShape::Private::resolveTransforms(KisForest<KoSvgTextContentElement>::child_iterator currentTextElement,
                                                QString text, QVector<CharacterResult> &result, int &currentIndex,
                                                bool isHorizontal, bool wrapped, bool textInPath,
                                                QVector<KoSvgText::CharTransformation> &resolved, QVector<bool> collapsedChars,
                                                const KoSvgTextProperties resolvedProps, bool withControls) {
    QVector<KoSvgText::CharTransformation> local = currentTextElement->localTransformations;


    int index = currentIndex;
    int j = index + numChars(currentTextElement, withControls, resolvedProps);

    if (!currentTextElement->textPathId.isEmpty()) {
        textInPath = true;
    } else {
        int i = 0;
        for (int k = index; k < j; k++ ) {
            if (k >= text.size()) {
                continue;
            }

            bool bidi = bidiControls.contains(text.at(k));
            bool softHyphen = text.at(k) == QChar::SoftHyphen;

            // Apparently when there's bidi controls in the text, they participate in line-wrapping,
            // so we don't check for it when wrapping.
            if (collapsedChars[k] || (bidi && !wrapped) || softHyphen) {
                result[k].addressable = false;
                continue;
            }
            if (k > 0 && text.at(k).isLowSurrogate() && text.at(k-1).isHighSurrogate()) {
                // transforms apply per-undicode codepoint, not per utf16.
                result[k].addressable = false;
                continue;
            }

            if (i < local.size()) {

                KoSvgText::CharTransformation newTransform = local.at(i);
                newTransform.mergeInParentTransformation(resolved[k]);
                resolved[k] = newTransform;
                i += 1;
            } else if (k > 0) {
                if (resolved[k - 1].rotate && !resolved[k].rotate) {
                    resolved[k].rotate = resolved[k - 1].rotate;
                }
            }
        }
    }

    KoSvgTextProperties props = currentTextElement->properties;
    props.inheritFrom(resolvedProps, true);
    for (auto child = KisForestDetail::childBegin(currentTextElement); child != KisForestDetail::childEnd(currentTextElement); child++) {
        resolveTransforms(child, text, result, currentIndex, isHorizontal, false, textInPath, resolved, collapsedChars, props, withControls);

    }

    if (!currentTextElement->textPathId.isEmpty()) {
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
void KoSvgTextShape::Private::applyTextLength(KisForest<KoSvgTextContentElement>::child_iterator currentTextElement,
                                              QVector<CharacterResult> &result,
                                              int &currentIndex,
                                              int &resolvedDescendentNodes,
                                              bool isHorizontal,
                                              const KoSvgTextProperties resolvedProps, const KoSvgText::ResolutionHandler &resHandler)
{

    int i = currentIndex;
    int j = i + numChars(currentTextElement, true, resolvedProps);
    int resolvedChildren = 0;

    KoSvgTextProperties props;
    props.inheritFrom(resolvedProps, true);
    for (auto child = KisForestDetail::childBegin(currentTextElement); child != KisForestDetail::childEnd(currentTextElement); child++) {
        applyTextLength(child, result, currentIndex, resolvedChildren, isHorizontal, props, resHandler);
    }
    // Raqm handles bidi reordering for us, but this algorithm does not
    // anticipate that, so we need to keep track of which typographic item
    // belongs where.
    QMap<int, int> visualToLogical;
    if (!currentTextElement->textLength.isAuto) {
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
        bool spacingAndGlyphs = (currentTextElement->lengthAdjust == KoSvgText::LengthAdjustSpacingAndGlyphs);
        if (!spacingAndGlyphs) {
            n -= 1;
        }
        const qreal delta = currentTextElement->textLength.customValue - (b - a);

        const QPointF d = isHorizontal ? QPointF(delta / n, 0) : QPointF(0, delta / n);

        QPointF shift;
        bool secondTextLengthApplied = false;
        Q_FOREACH (int k, visualToLogical.keys()) {
            CharacterResult cr = result[visualToLogical.value(k)];
            if (cr.addressable) {
                cr.finalPosition += shift;
                cr.textLengthOffset += shift;
                if (spacingAndGlyphs) {
                    QPointF scale(d.x() != 0 ? (d.x() / cr.advance.x()) + 1 : 1.0, d.y() != 0 ? (d.y() / cr.advance.y()) + 1 : 1.0);
                    cr.scaleCharacterResult(scale.x(), scale.y());
                }
                bool last = spacingAndGlyphs ? false : k == visualToLogical.keys().last();

                if (!(cr.textLengthApplied && secondTextLengthApplied) && !last) {
                    shift = resHandler.adjust(shift+d);
                }
                secondTextLengthApplied = cr.textLengthApplied;
                cr.textLengthApplied = true;
            }
            result[visualToLogical.value(k)] = cr;
        }
        resolvedDescendentNodes += 1;

        // apply the shift to all consecutive chars as long as they don't start
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

void KoSvgTextShape::Private::computeFontMetrics(// NOLINT(readability-function-cognitive-complexity)
    KisForest<KoSvgTextContentElement>::child_iterator parent,
    const KoSvgTextProperties &parentProps,
    const KoSvgText::FontMetrics &parentBaselineTable,
    const KoSvgText::Baseline parentBaseline,
    const QPointF superScript,
    const QPointF subScript,
    QVector<CharacterResult> &result,
    int &currentIndex,
    const KoSvgText::ResolutionHandler resHandler,
    const bool isHorizontal,
    const bool disableFontMatching)
{
    const int i = currentIndex;
    const int j = qMin(i + numChars(parent, true, parentProps), result.size());

    KoSvgTextProperties properties = parent->properties;
    properties.inheritFrom(parentProps, true);

    const KoSvgText::CssLengthPercentage baselineShift = properties.property(KoSvgTextProperties::BaselineShiftValueId).value<KoSvgText::CssLengthPercentage>();
    QPointF baselineShiftTotal;
    KoSvgText::BaselineShiftMode baselineShiftMode = KoSvgText::BaselineShiftMode(properties.property(KoSvgTextProperties::BaselineShiftModeId).toInt());

    if (baselineShiftMode == KoSvgText::ShiftSuper) {
        baselineShiftTotal = isHorizontal ? superScript : QPointF(-superScript.y(), superScript.x());
    } else if (baselineShiftMode == KoSvgText::ShiftSub) {
        baselineShiftTotal = isHorizontal ? subScript : QPointF(-subScript.y(), subScript.x());
    } else if (baselineShiftMode == KoSvgText::ShiftLengthPercentage) {
        // Positive baseline-shift goes up in the inline-direction, which is up in horizontal and right in vertical.
        baselineShiftTotal = isHorizontal ? QPointF(0, -baselineShift.value) : QPointF(baselineShift.value, 0);
    }
    baselineShiftTotal = resHandler.adjust(baselineShiftTotal);

    QVector<int> lengths;
    const std::vector<FT_FaceSP> faces = KoFontRegistry::instance()->facesForCSSValues(
        lengths,
        properties.cssFontInfo(),
        QString(),
        static_cast<quint32>(resHandler.xRes),
        static_cast<quint32>(resHandler.yRes),
        disableFontMatching);


    KoSvgText::Baseline dominantBaseline = KoSvgText::Baseline(properties.propertyOrDefault(KoSvgTextProperties::DominantBaselineId).toInt());

    const KoSvgText::Baseline defaultBaseline = isHorizontal? KoSvgText::BaselineAlphabetic: KoSvgText::BaselineCentral;
    KoSvgText::FontMetrics metrics;

    // In SVG 2 and CSS-inline-3, metrics are always recalculated per box.
    metrics = KoFontRegistry::generateFontMetrics(faces.front(), isHorizontal);
    if (dominantBaseline == KoSvgText::BaselineResetSize || dominantBaseline == KoSvgText::BaselineNoChange) {
        dominantBaseline = KoSvgText::BaselineAuto;
    }

    if (dominantBaseline == KoSvgText::BaselineAuto) {
        dominantBaseline = defaultBaseline;
    }

    // Get underline and super/subscripts.
    const QTransform ftTf = resHandler.freeTypeToPointTransform();
    const QPointF newSuperScript = ftTf.map(QPointF(metrics.superScriptOffset.first, metrics.superScriptOffset.second));
    const QPointF newSubScript = ftTf.map(QPointF(metrics.subScriptOffset.first, -metrics.subScriptOffset.second));

    for (auto child = KisForestDetail::childBegin(parent); child != KisForestDetail::childEnd(parent); child++) {
        computeFontMetrics(child, properties, metrics, dominantBaseline, newSuperScript, newSubScript, result, currentIndex, resHandler, isHorizontal, disableFontMatching);
    }

    KoSvgText::Baseline baselineAdjust = KoSvgText::Baseline(properties.propertyOrDefault(KoSvgTextProperties::AlignmentBaselineId).toInt());

    if (baselineAdjust == KoSvgText::BaselineDominant) {
        baselineAdjust = parentBaseline;
    }
    if (baselineAdjust == KoSvgText::BaselineAuto || baselineAdjust == KoSvgText::BaselineUseScript) {
        // UseScript got deprecated in CSS-Inline-3.
        baselineAdjust = defaultBaseline;
    }

    bool applyGlyphAlignment = KisForestDetail::childBegin(parent) == KisForestDetail::childEnd(parent);
    const int boxOffset = parentBaselineTable.valueForBaselineValue(baselineAdjust) - metrics.valueForBaselineValue(baselineAdjust);
    const QPointF shift = resHandler.adjust(ftTf.map(isHorizontal? QPointF(0, (boxOffset)): QPointF((boxOffset), 0)));
    if (baselineShiftMode == KoSvgText::ShiftSuper || baselineShiftMode == KoSvgText::ShiftSub) {
        // We need to remove the additional alignment shift to ensure that super and sub shifts don't get too dramatic.
        baselineShiftTotal -= shift;
    }

    /*
     * The actual alignment process.
     * What the CSS-inline-3 spec wants us to do is to have inline-boxes aligned to their parent by their alignment-baseline
     * (which defaults to dominant baseline), while glyphs are aligned to their parent by the dominant baseline. This means
     * alignment baseline can nest, which makes sense since it does not inherit.
     * Glyphs' parent is the inline box they're in, so we're using the main metrics of the inline box to align them via their
     * per-glyph metrics to the inline box. This only happens if we're at a text-content element.
     * The inline box itself is then aligned to the parent inline box with the alignment baseline, using the parent
     * and inline main metrics.
     * Then, finally baseline shift is applied, if any.
     */
    for (int k = i; k < j; k++) {
        if (applyGlyphAlignment) {
            // We offset the whole glyph so that the origin is at the dominant baseline.
            // This will simplify having svg per-char transforms apply as per spec.
            const int originOffset = result[k].metrics.valueForBaselineValue(dominantBaseline);
            const QPointF newOrigin = resHandler.adjust(ftTf.map(isHorizontal? QPointF(0, originOffset): QPointF(originOffset, 0)));
            const int uniqueOffset = metrics.valueForBaselineValue(dominantBaseline);
            const QPointF uniqueShift = resHandler.adjust(ftTf.map(isHorizontal? QPointF(0, uniqueOffset): QPointF(uniqueOffset, 0)));
            result[k].translateOrigin(newOrigin);
            result[k].metrics.offsetMetricsToNewOrigin(dominantBaseline);
            result[k].dominantBaselineOffset = uniqueShift;
        }
        result[k].dominantBaselineOffset += shift;
        result[k].baselineOffset += baselineShiftTotal;
    }

    currentIndex = j;
}

void KoSvgTextShape::Private::handleLineBoxAlignment(KisForest<KoSvgTextContentElement>::child_iterator parent,
                                                     QVector<CharacterResult> &result,
                                                     const QVector<LineBox> lineBoxes,
                                                     int &currentIndex,
                                                     const bool isHorizontal,
                                                     const KoSvgTextProperties resolvedProps)
{

    const int i = currentIndex;
    const int j = qMin(i + numChars(parent, true, resolvedProps), result.size());

    KoSvgTextProperties properties = parent->properties;
    KoSvgText::BaselineShiftMode baselineShiftMode = KoSvgText::BaselineShiftMode(properties.property(KoSvgTextProperties::BaselineShiftModeId).toInt());

    properties.inheritFrom(resolvedProps);
    for (auto child = KisForestDetail::childBegin(parent); child != KisForestDetail::childEnd(parent); child++) {
        handleLineBoxAlignment(child, result, lineBoxes, currentIndex, isHorizontal, properties);
    }

    if (baselineShiftMode == KoSvgText::ShiftLineTop || baselineShiftMode == KoSvgText::ShiftLineBottom) {

        LineBox relevantLine;
        Q_FOREACH(LineBox lineBox, lineBoxes) {
            Q_FOREACH(LineChunk chunk, lineBox.chunks) {
                if (chunk.chunkIndices.contains(i)) {
                    relevantLine = lineBox;
                }
            }
        }
        QPointF shift = QPointF();
        double ascent = 0.0;
        double descent = 0.0;
        for (int k = i; k < j; k++) {
            // The height calculation here is to remove the shifted-part height
            // from the top (or bottom) of the line.
            calculateLineHeight(result[k], ascent, descent, isHorizontal, true);
        }

        if (baselineShiftMode == KoSvgText::ShiftLineTop) {
            shift = relevantLine.baselineTop;
            shift -= isHorizontal? QPointF(0, ascent):QPointF(ascent, 0);
        } else if (baselineShiftMode == KoSvgText::ShiftLineBottom) {
            shift = relevantLine.baselineBottom;
            shift -= isHorizontal? QPointF(0, descent):QPointF(descent, 0);
        }


        for (int k = i; k < j; k++) {
            CharacterResult cr = result[k];
            cr.cssPosition += shift;
            cr.finalPosition = cr.cssPosition;
            result[k] = cr;
        }
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
void KoSvgTextShape::Private::computeTextDecorations(// NOLINT(readability-function-cognitive-complexity)
    KisForest<KoSvgTextContentElement>::child_iterator currentTextElement,
    const QVector<CharacterResult> &result,
    const QMap<int, int> &logicalToVisual,
    const KoSvgText::ResolutionHandler resHandler,
    KoPathShape *textPath,
    qreal textPathoffset,
    bool side,
    int &currentIndex,
    bool isHorizontal,
    bool ltr,
    bool wrapping,
    const KoSvgTextProperties resolvedProps,
                                                     QList<KoShape *> textPaths)
{

    const int i = currentIndex;
    const int j = qMin(i + numChars(currentTextElement, true, resolvedProps), result.size());
    using namespace KoSvgText;

    KoPathShape *currentTextPath = nullptr;
    qreal currentTextPathOffset = textPathoffset;
    bool textPathSide = side;
    if (!wrapping) {
        KoShape *cTextPath = KoSvgTextShape::Private::textPathByName(currentTextElement->textPathId, textPaths);
        currentTextPath = textPath ? textPath : dynamic_cast<KoPathShape *>(cTextPath);

        if (cTextPath) {
            textPathSide = currentTextElement->textPathInfo.side == TextPathSideRight;
            if (currentTextElement->textPathInfo.startOffsetIsPercentage) {
                KIS_ASSERT(currentTextPath);
                currentTextPathOffset = currentTextPath->outline().length() * (0.01 * currentTextElement->textPathInfo.startOffset);
            } else {
                currentTextPathOffset = currentTextElement->textPathInfo.startOffset;
            }
        }
    }
    KoSvgTextProperties properties = currentTextElement->properties;
    properties.inheritFrom(resolvedProps);

    KoSvgText::TextUnderlinePosition pos = properties.propertyOrDefault(KoSvgTextProperties::TextDecorationPositionId).value<KoSvgText::TextUnderlinePosition>();
    TextDecorationUnderlinePosition newUnderlinePosH = pos.horizontalPosition;
    TextDecorationUnderlinePosition newUnderlinePosV = pos.verticalPosition;

    for (auto child = KisForestDetail::childBegin(currentTextElement); child != KisForestDetail::childEnd(currentTextElement); child++) {
        computeTextDecorations(child,
                               result,
                               logicalToVisual,
                               resHandler,
                               currentTextPath,
                               currentTextPathOffset,
                               textPathSide,
                               currentIndex,
                               isHorizontal,
                               ltr,
                               wrapping,
                               properties, textPaths
                               );
    }

    TextDecorations decor = currentTextElement->properties.propertyOrDefault(KoSvgTextProperties::TextDecorationLineId).value<TextDecorations>();
    if (decor != DecorationNone && currentTextElement->properties.hasProperty(KoSvgTextProperties::TextDecorationLineId)) {

        TextDecorationStyle style = TextDecorationStyle(
                    properties.propertyOrDefault(
                        KoSvgTextProperties::TextDecorationStyleId).toInt());

        QMap<TextDecoration, QPainterPath> decorationPaths =
                generateDecorationPaths(i, j, resHandler,
                                        result, isHorizontal, decor, style, false, currentTextPath,
                                        currentTextPathOffset, textPathSide, newUnderlinePosH, newUnderlinePosV
                                        );

        // And finally add the paths to the chunkshape.

        Q_FOREACH (TextDecoration type, decorationPaths.keys()) {
            QPainterPath decorationPath = decorationPaths.value(type);
            if (!decorationPath.isEmpty()) {
                currentTextElement->textDecorations.insert(type, decorationPath.simplified());
            }
        }
    }
    currentIndex = j;
}

QPair<QPainterPath, QPointF> generateDecorationPath (
        const QLineF length,
        const qreal strokeWidth,
        const KoSvgText::TextDecorationStyle style,
        const bool isHorizontal,
        const bool onTextPath,
        const qreal minimumDecorationThickness
        ) {
    QPainterPath p;
    QPointF pathWidth;
    if (style != KoSvgText::Wavy) {
        p.moveTo(QPointF());
        // We're segmenting the path here so it'll be easier to warp
        // when text-on-path is happening.
        if (onTextPath) {
            const int total = std::floor(length.length() / (strokeWidth * 2));
            const qreal segment = qreal(length.length() / total);
            if (isHorizontal) {
                for (int i = 0; i < total; i++) {
                    p.lineTo(p.currentPosition() + QPointF(segment, 0));
                }
            } else {
                for (int i = 0; i < total; i++) {
                    p.lineTo(p.currentPosition() + QPointF(0, segment));
                }
            }
        } else {
            if (isHorizontal) {
                p.lineTo(length.length(), 0);
            } else {
                p.lineTo(0, length.length());
            }
        }
    }
    if (style == KoSvgText::Double) {
        qreal linewidthOffset = qMax(strokeWidth * 1.5, minimumDecorationThickness * 2);
        if (isHorizontal) {
            p.addPath(p.translated(0, linewidthOffset));
            pathWidth = QPointF(0, -linewidthOffset);
        } else {
            p.addPath(p.translated(linewidthOffset, 0));
            pathWidth = QPointF(linewidthOffset, 0);
        }

    } else if (style == KoSvgText::Wavy) {
        qreal width = length.length();
        qreal height = strokeWidth * 2;

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
        pathWidth = QPointF(0, -strokeWidth);

        // Rotate for vertical.
        if (!isHorizontal) {
            for (int i = 0; i < p.elementCount(); i++) {
                p.setElementPositionAt(i, p.elementAt(i).y - (strokeWidth * 2), p.elementAt(i).x);
            }
            pathWidth = QPointF(strokeWidth, 0);
        }
    }
    return qMakePair(p, pathWidth);
}

void KoSvgTextShape::Private::finalizeDecoration (
        QPainterPath decorationPath,
        const QPointF offset,
        const QPainterPathStroker &stroker,
        const KoSvgText::TextDecoration type,
        QMap<KoSvgText::TextDecoration, QPainterPath> &decorationPaths,
        const KoPathShape *currentTextPath,
        const bool isHorizontal,
        const qreal currentTextPathOffset,
        const bool textPathSide
        ) {
    if (currentTextPath) {
        QPainterPath path = currentTextPath->outline();
        path = currentTextPath->transformation().map(path);
        if (textPathSide) {
            path = path.toReversed();
        }

        decorationPath = stretchGlyphOnPath(decorationPath.translated(offset), path, isHorizontal, currentTextPathOffset, currentTextPath->isClosedSubpath(0));
        decorationPaths[type].addPath(stroker.createStroke(decorationPath));
    } else {
        decorationPaths[type].addPath(stroker.createStroke(decorationPath.translated(offset)));
    }
    decorationPaths[type].setFillRule(Qt::WindingFill);
}

QMap<KoSvgText::TextDecoration, QPainterPath>
KoSvgTextShape::Private::generateDecorationPaths(const int &start, const int &end,
                                                 const KoSvgText::ResolutionHandler resHandler,
                                                 const QVector<CharacterResult> &result,
                                                 const bool isHorizontal,
                                                 const KoSvgText::TextDecorations &decor,
                                                 const KoSvgText::TextDecorationStyle style,
                                                 const bool textDecorationSkipInset,
                                                 const KoPathShape *currentTextPath,
                                                 const qreal currentTextPathOffset,
                                                 const bool textPathSide,
                                                 const KoSvgText::TextDecorationUnderlinePosition underlinePosH,
                                                 const KoSvgText::TextDecorationUnderlinePosition underlinePosV) {
    using namespace KoSvgText;

    const qreal freetypePixelsToPt = resHandler.freeTypePixelToPointFactor(isHorizontal);
    const qreal minimumDecorationThickness = resHandler.pixelToPointFactor(isHorizontal);

    QMap<TextDecoration, QPainterPath> decorationPaths;

    decorationPaths.insert(DecorationUnderline, QPainterPath());
    decorationPaths.insert(DecorationOverline, QPainterPath());
    decorationPaths.insert(DecorationLineThrough, QPainterPath());

    QPainterPathStroker stroker;
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

    struct DecorationBox {
        QRectF decorationRect;
        QVector<qreal> underlineOffsets;
        qint32 thickness = 0;
        int start = 0;
        int end = 0;
    };

    struct LineThrough {
        QPolygonF line;
        qint32 thickness = 0;
    };

    QVector<DecorationBox> decorationBoxes;
    QVector<LineThrough> lineThroughLines;
    DecorationBox currentBox;
    currentBox.start = start;

    // First we get all the ranges between anchored chunks and "trim" the whitespaces.
    for (int k = start; k < end; k++) {
        CharacterResult charResult = result.at(k);
        const bool atEnd = k+1 >= end;
        if ((charResult.anchored_chunk || atEnd) && currentBox.start < k) {
            // walk backwards to remove empties.
            if (k > start) {
                for (int l = atEnd? k: k-1; l > currentBox.start; l--) {
                    if (!result.at(l).inkBoundingBox.isEmpty()
                            && result.at(l).addressable
                            && !result.at(l).hidden) {
                        currentBox.end = l;
                        break;
                    }
                }
            }
            decorationBoxes.append(currentBox);
            currentBox = DecorationBox();
            currentBox.start = k;
        }
        if (currentBox.start == k && (charResult.inkBoundingBox.isEmpty()
                || !charResult.addressable
                || charResult.hidden)) {
            currentBox.start = k+1;
        }
    }

    qint32 lastFontSize = 0;
    QPointF lastBaselineOffset;
    QPointF lastLineThroughOffset;

    // Then we go over each range, and calculate their decoration box,
    // as well as calculate the linethroughs.
    for (int b = 0; b < decorationBoxes.size(); b++) {
        DecorationBox currentBox = decorationBoxes.at(b);
        LineThrough currentLineThrough = LineThrough();
        lastFontSize = result.at(currentBox.start).metrics.fontSize;
        lastBaselineOffset = result.at(currentBox.start).totalBaselineOffset();

        for (int k = currentBox.start; k <= currentBox.end; k++) {
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
            // Adjustments to "vertical align" will not affect the baseline offset, so no new stroke needs to be created.
            const bool baselineIsOffset = charResult.totalBaselineOffset() != lastBaselineOffset;
            const bool newLineThrough = charResult.metrics.fontSize != lastFontSize && !baselineIsOffset;
            const bool ignoreMetrics = !newLineThrough && baselineIsOffset && !currentLineThrough.line.isEmpty();

            if (newLineThrough) {
                lineThroughLines.append(currentLineThrough);
                currentLineThrough = LineThrough();

                lastFontSize = charResult.metrics.fontSize;
                lastBaselineOffset = charResult.totalBaselineOffset();
            }

            const qreal alphabetic = charResult.metrics.valueForBaselineValue(BaselineAlphabetic)*freetypePixelsToPt;
            const QPointF alphabeticOffset = isHorizontal? QPointF(0, -alphabetic): QPointF(alphabetic, 0);

            if (!ignoreMetrics) {
                currentBox.thickness += charResult.metrics.underlineThickness;
                currentLineThrough.thickness += charResult.metrics.lineThroughThickness;
                lastLineThroughOffset =  isHorizontal? QPointF(0, -(charResult.metrics.lineThroughOffset*freetypePixelsToPt)) + alphabeticOffset
                                                          : QPointF(charResult.metrics.valueForBaselineValue(BaselineCentral)*freetypePixelsToPt, 0);
                currentBox.underlineOffsets.append((-charResult.metrics.underlineOffset)*freetypePixelsToPt);
            }
            QPointF lastLineThroughPoint = charResult.finalPosition + charResult.advance + lastLineThroughOffset;

            if (ignoreMetrics) {
                QPointF lastP = currentLineThrough.line.last();
                if (isHorizontal) {
                    lastLineThroughPoint.setY(lastP.y());
                } else {
                    lastLineThroughPoint.setX(lastP.x());
                }
            }

            if ((isHorizontal && underlinePosH == UnderlineAuto)) {
                QRectF bbox = charResult.layoutBox().translated(-alphabeticOffset);
                bbox.setBottom(0);
                currentBox.decorationRect |= bbox.translated(charResult.finalPosition + alphabeticOffset);
            } else {
                currentBox.decorationRect |= charResult.layoutBox().translated(charResult.finalPosition);
            }
            if (currentLineThrough.line.isEmpty()) {
                currentLineThrough.line.append(charResult.finalPosition + lastLineThroughOffset);
            }
            currentLineThrough.line.append(lastLineThroughPoint);
        }
        decorationBoxes[b] = currentBox;
        lineThroughLines.append(currentLineThrough);
    }

    // Now to create a QPainterPath for the given style that stretches
    // over a single decoration rect,
    // transform that and add it to the general paths.

    const bool textOnPath = (currentTextPath)? true: false;

    for (int i = 0; i < decorationBoxes.size(); i++) {
        DecorationBox box = decorationBoxes.at(i);
        if (box.underlineOffsets.size() > 0) {
            stroker.setWidth(qMax(minimumDecorationThickness, (box.thickness/(box.underlineOffsets.size()))*freetypePixelsToPt));
            if (isHorizontal) {
                stroker.setWidth(resHandler.adjust(QPointF(0, stroker.width())).y());
            } else {
                stroker.setWidth(resHandler.adjust(QPointF(stroker.width(), 0)).x());
            }
        } else {
            stroker.setWidth(minimumDecorationThickness);
        }
        QRectF rect = box.decorationRect;
        if (textDecorationSkipInset) {
            qreal inset = stroker.width() * 0.5;
            rect.adjust(-inset, -inset, inset, inset);
        }
        QLineF length;
        length.setP1(isHorizontal? QPointF(rect.left(), 0): QPointF(0, rect.top()));
        length.setP2(isHorizontal? QPointF(rect.right(), 0): QPointF(0, rect.bottom()));

        QPair<QPainterPath, QPointF> generatedPath = generateDecorationPath(length, stroker.width(), style, isHorizontal, textOnPath, minimumDecorationThickness);
        QPainterPath p = generatedPath.first;
        QPointF pathWidth = generatedPath.second;

        QMap<TextDecoration, QPointF> decorationOffsets;
        if (isHorizontal) {
            const qreal startX = rect.left();
            decorationOffsets[DecorationOverline] = resHandler.adjustWithOffset(QPointF(startX, box.decorationRect.top()) + pathWidth, pathWidth*0.5);
            decorationOffsets[DecorationUnderline] = resHandler.adjustWithOffset(QPointF(startX, box.decorationRect.bottom()), pathWidth*0.5);
            if (underlinePosH == UnderlineAuto) {
                qreal average = 0;
                for (int j = 0; j < box.underlineOffsets.size(); j++) {
                    average += box.underlineOffsets.at(j);
                }
                average = average > 0? (average/box.underlineOffsets.size()): 0;
                decorationOffsets[DecorationUnderline] += resHandler.adjustWithOffset(QPointF(0, average), pathWidth*0.5);
            }
        } else {
            const qreal startY = rect.top();
            if (underlinePosV == UnderlineRight) {
                decorationOffsets[DecorationOverline] = resHandler.adjustWithOffset(QPointF(box.decorationRect.left(), startY), pathWidth*0.5);
                decorationOffsets[DecorationUnderline] = resHandler.adjustWithOffset(QPointF(box.decorationRect.right(), startY) +pathWidth, pathWidth*0.5);
            } else {
                decorationOffsets[DecorationOverline] = resHandler.adjustWithOffset(QPointF(box.decorationRect.right(), startY) +pathWidth, pathWidth*0.5);
                decorationOffsets[DecorationUnderline] = resHandler.adjustWithOffset(QPointF(box.decorationRect.left(), startY), pathWidth*0.5);
            }
        }

        if (decor.testFlag(DecorationUnderline)) {
            finalizeDecoration(p, decorationOffsets.value(DecorationUnderline), stroker, DecorationUnderline, decorationPaths, currentTextPath, isHorizontal, currentTextPathOffset, textPathSide);
        }
        if (decor.testFlag(DecorationOverline)) {
            finalizeDecoration(p, decorationOffsets.value(DecorationOverline), stroker, DecorationOverline, decorationPaths, currentTextPath, isHorizontal, currentTextPathOffset, textPathSide);
        }
    }
    if (decor.testFlag(DecorationLineThrough)) {
        for (int i = 0; i < lineThroughLines.size(); i++) {
            LineThrough l = lineThroughLines.at(i);
            QPolygonF poly = l.line;
            if (poly.isEmpty()) continue;
            stroker.setWidth(qMax(minimumDecorationThickness, (l.thickness/(poly.size()))*freetypePixelsToPt));
            QPointF pathWidth;
            if (isHorizontal) {
                pathWidth = resHandler.adjust(QPointF(0, stroker.width()));
                stroker.setWidth(pathWidth.y());
            } else {
                pathWidth = resHandler.adjust(QPointF(stroker.width(), 0));
                stroker.setWidth(pathWidth.x());
            }
            qreal average = 0.0;
            for (int j = 0; j < poly.size(); j++) {
                average += isHorizontal? poly.at(j).y(): poly.at(j).x();
            }
            average /= poly.size();
            QLineF line = isHorizontal? QLineF(poly.first().x(), average, poly.last().x(), average)
                                      : QLineF(average, poly.first().y(), average, poly.last().y());
            line.setP1(resHandler.adjustWithOffset(line.p1(), pathWidth*0.5));
            line.setP2(resHandler.adjustWithOffset(line.p2(), pathWidth*0.5));
            QPair<QPainterPath, QPointF> generatedPath = generateDecorationPath(line, stroker.width(), style, isHorizontal, textOnPath, minimumDecorationThickness);
            QPainterPath p = generatedPath.first;
            finalizeDecoration(p, line.p1() + (generatedPath.second * 0.5), stroker, DecorationLineThrough, decorationPaths, currentTextPath, isHorizontal, currentTextPathOffset, textPathSide);
        }
    }

    return decorationPaths;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
void KoSvgTextShape::Private::applyAnchoring(QVector<CharacterResult> &result, bool isHorizontal, const KoSvgText::ResolutionHandler resHandler)
{
    int end = 0;
    int start = 0;

    while (start < result.size()) {

        qreal shift = anchoredChunkShift(result, isHorizontal, start, end);

        const QPointF shiftP = resHandler.adjust(isHorizontal ? QPointF(shift, 0) : QPointF(0, shift));

        for (int j = start; j < end; j++) {
            result[j].finalPosition += shiftP;
            result[j].textPathAndAnchoringOffset += shiftP;
        }
        start = end;
    }
}

qreal KoSvgTextShape::Private::anchoredChunkShift(const QVector<CharacterResult> &result, const bool isHorizontal, const int start, int &end)
{
    qreal a = 0;
    qreal b = 0;
    int i = start;
    for (; i < result.size(); i++) {
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

    const CharacterResult startRes = result.at(start);

    const bool rtl = startRes.direction == KoSvgText::DirectionRightToLeft;
    qreal shift = isHorizontal ? startRes.finalPosition.x() : startRes.finalPosition.y();

    if ((startRes.anchor == KoSvgText::AnchorStart && !rtl)
            || (startRes.anchor == KoSvgText::AnchorEnd && rtl)) {
        shift = shift - a;

    } else if ((startRes.anchor == KoSvgText::AnchorEnd && !rtl)
               || (startRes.anchor == KoSvgText::AnchorStart && rtl)) {
        shift = shift - b;
    } else {
        shift = shift - (a + b) * 0.5;
    }
    end = i;
    return shift;
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
void KoSvgTextShape::Private::applyTextPath(KisForest<KoSvgTextContentElement>::child_iterator root,
                                            QVector<CharacterResult> &result,
                                            bool isHorizontal,
                                            QPointF &startPos,
                                            const KoSvgTextProperties resolvedProps, QList<KoShape *> textPaths)
{
    // Unlike all the other applying functions, this one only iterates over the
    // top-level. SVG is not designed to have nested textPaths. Source:
    // https://github.com/w3c/svgwg/issues/580
    bool inPath = false;
    bool afterPath = false;
    int currentIndex = 0;
    QPointF pathEnd;
    for (auto textShapeElement = KisForestDetail::childBegin(root); textShapeElement != KisForestDetail::childEnd(root); textShapeElement++) {
        int endIndex = currentIndex + numChars(textShapeElement, true, resolvedProps);

        KoShape *cTextPath = KoSvgTextShape::Private::textPathByName(textShapeElement->textPathId, textPaths);
        KoPathShape *shape = dynamic_cast<KoPathShape *>(cTextPath);
        if (shape) {
            QPainterPath path = shape->outline();
            path = shape->transformation().map(path);
            inPath = true;
            if (textShapeElement->textPathInfo.side == KoSvgText::TextPathSideRight) {
                path = path.toReversed();
            }
            qreal length = path.length();
            qreal offset = 0.0;
            bool isClosed = (shape->isClosedSubpath(0) && shape->subpathCount() == 1);
            if (textShapeElement->textPathInfo.startOffsetIsPercentage) {
                offset = length * (0.01 * textShapeElement->textPathInfo.startOffset);
            } else {
                offset = textShapeElement->textPathInfo.startOffset;
            }
            bool stretch = textShapeElement->textPathInfo.method == KoSvgText::TextPathStretch;

            if (textShapeElement == KisForestDetail::childBegin(root)) {
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
                        const QPointF originalPos = cr.finalPosition;
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
                        cr.textPathAndAnchoringOffset += (cr.finalPosition - originalPos);
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
                        cr.textPathAndAnchoringOffset += pathEnd;
                        result[i] = cr;
                    }
                }
            }
        }
        currentIndex = endIndex;
    }
}
QVector<SubChunk> KoSvgTextShape::Private::collectSubChunks(KisForest<KoSvgTextContentElement>::child_iterator it, KoSvgTextProperties parentProps, bool textInPath, bool &firstTextInPath)
{
    QVector<SubChunk> result;

    if (!it->textPathId.isEmpty()) {
        textInPath = true;
        firstTextInPath = true;
    }

    KoSvgTextProperties currentProps = it->properties;
    currentProps.inheritFrom(parentProps, true);


    if (childCount(it)) {
        for (auto child = KisForestDetail::childBegin(it); child != KisForestDetail::childEnd(it); child++) {
         result += collectSubChunks(child, currentProps, textInPath, firstTextInPath);
        }
    } else {
        SubChunk chunk(it);
        chunk.inheritedProps = currentProps;
        chunk.bg = chunk.inheritedProps.background();


        KoSvgText::UnicodeBidi bidi = KoSvgText::UnicodeBidi(it->properties.propertyOrDefault(KoSvgTextProperties::UnicodeBidiId).toInt());
        KoSvgText::Direction direction = KoSvgText::Direction(it->properties.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
        const QString bidiOpening = KoCssTextUtils::getBidiOpening(direction == KoSvgText::DirectionLeftToRight, bidi);
        const QString bidiClosing = KoCssTextUtils::getBidiClosing(bidi);


        if (!bidiOpening.isEmpty()) {
            chunk.text = bidiOpening;
            chunk.originalText = QString();
            chunk.newToOldPositions.clear();
            result.append(chunk);
            firstTextInPath = false;
        }

        chunk.originalText = it->text;
        chunk.text = it->getTransformedString(chunk.newToOldPositions, chunk.inheritedProps);
        result.append(chunk);

        if (!bidiClosing.isEmpty()) {
            chunk.text = bidiClosing;
            chunk.originalText = QString();
            chunk.newToOldPositions.clear();
            result.append(chunk);
        }

        firstTextInPath = false;
    }

    if (!it->textPathId.isEmpty()) {
        textInPath = false;
        firstTextInPath = false;
    }

    return result;
}
