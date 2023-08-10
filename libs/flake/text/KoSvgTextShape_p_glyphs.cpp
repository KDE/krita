/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSvgTextShape.h"
#include "KoSvgTextShape_p.h"

#include "KoFontLibraryResourceUtils.h"

#include <FlakeDebug.h>
#include <KoPathShape.h>

#include <kis_global.h>

#include <QPainterPath>
#include <QtMath>

#include <ft2build.h>
#include FT_FREETYPE_H
#include FT_COLOR_H
#include FT_BITMAP_H
#include FT_OUTLINE_H

#include <hb.h>
#include <hb-ft.h>

#include <raqm.h>


static QPainterPath convertFromFreeTypeOutline(FT_GlyphSlotRec *glyphSlot);
static QImage convertFromFreeTypeBitmap(FT_GlyphSlotRec *glyphSlot);

static QString glyphFormatToStr(const FT_Glyph_Format _v)
{
    const unsigned int v = _v;
    QString s;
    s += (v >> 24) & 0xFF;
    s += (v >> 16) & 0xFF;
    s += (v >> 8) & 0xFF;
    s += (v >> 0) & 0xFF;
    return s;
}

static void emboldenGlyphIfNeeded(raqm_glyph_t &currentGlyph, CharacterResult &charResult)
{
    if (charResult.fontWeight >= 600 && !(currentGlyph.ftface->style_flags & FT_STYLE_FLAG_BOLD)) {
        // This code is somewhat inspired by Firefox.
        FT_Pos strength =
            FT_MulFix(currentGlyph.ftface->units_per_EM, currentGlyph.ftface->size->metrics.y_scale) / 48;

        if (currentGlyph.ftface->glyph->format == FT_GLYPH_FORMAT_BITMAP) {
            // This is similar to what FT_GlyphSlot_Embolden does.

            // Round down to full pixel.
            strength &= ~63;
            if (strength == 0) {
                // ... but it has to be at least one pixel.
                strength = 64;
            }

            FT_GlyphSlot_Own_Bitmap(currentGlyph.ftface->glyph);

            // Embolden less vertically than horizontally. Especially if
            // strength is only 1px, don't embolden vertically at all.
            // Otherwise it makes the glyph way too heavy, especially for
            // CJK glyphs in small sizes.
            const FT_Pos strengthY = strength - 64;
            FT_Bitmap_Embolden(currentGlyph.ftface->glyph->library,
                                &currentGlyph.ftface->glyph->bitmap,
                                strength,
                                strengthY);

            if (currentGlyph.x_advance != 0) {
                currentGlyph.x_advance += strength;
            }
            if (currentGlyph.y_advance != 0) {
                currentGlyph.y_advance -= strengthY;
            }
        } else {
            FT_Outline_Embolden(&currentGlyph.ftface->glyph->outline, strength);

            if (currentGlyph.x_advance != 0) {
                currentGlyph.x_advance += strength;
            }
            if (currentGlyph.y_advance != 0) {
                currentGlyph.y_advance -= strength;
            }
        }
    }
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
bool KoSvgTextShape::Private::loadGlyph(const QTransform &ftTF,
                                        const QMap<int, KoSvgText::TabSizeInfo> &tabSizeInfo,
                                        const FT_Int32 faceLoadFlags,
                                        const bool isHorizontal,
                                        const int i,
                                        raqm_glyph_t &currentGlyph,
                                        QMap<int, int> &logicalToVisual,
                                        CharacterResult &charResult,
                                        QPointF &totalAdvanceFTFontCoordinates) const
{
    // Whenever the freetype docs talk about a 26.6 floating point unit, they
    // mean a 1/64 value.
    const qreal ftFontUnit = 64.0;
    const qreal ftFontUnitFactor = 1 / ftFontUnit;

    {
        const int cluster = static_cast<int>(currentGlyph.cluster);

        QPointF spaceAdvance;
        if (tabSizeInfo.contains(cluster)) {
            FT_Load_Glyph(currentGlyph.ftface, FT_Get_Char_Index(currentGlyph.ftface, ' '), faceLoadFlags);
            spaceAdvance = QPointF(currentGlyph.ftface->glyph->advance.x, currentGlyph.ftface->glyph->advance.y);
        }

        if (const FT_Error err = FT_Load_Glyph(currentGlyph.ftface, currentGlyph.index, faceLoadFlags)) {
            warnFlake << "Failed to load glyph, freetype error" << err;
            return false;
        }

        // Check whether we need to synthesize bold by emboldening the glyph:
        emboldenGlyphIfNeeded(currentGlyph, charResult);

        /// The matrix for Italic (oblique) synthesis of outline glyphs, or for
        /// adjusting the bounding box of bitmap glyphs.
        QTransform glyphObliqueTf;
        /// The combined offset * italic * ftTf transform for outline glyphs.
        QTransform outlineGlyphTf;

        /// The scaling factor for color bitmap glyphs, otherwise always 1.0
        qreal bitmapScale = 1.0;

        if (currentGlyph.ftface->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
            outlineGlyphTf = QTransform::fromTranslate(currentGlyph.x_offset, currentGlyph.y_offset);

            // Check whether we need to synthesize italic by shearing the glyph:
            if (charResult.fontStyle != QFont::StyleNormal
                && !(currentGlyph.ftface->style_flags & FT_STYLE_FLAG_ITALIC)) {
                // CSS Fonts Module Level 4, 2.4. Font style: the font-style property:
                // For `oblique`, "lack of an <angle> represents 14deg".
                constexpr double SLANT_14DEG = 0.24932800284318069162403993780486;
                if (isHorizontal) {
                    glyphObliqueTf.shear(SLANT_14DEG, 0);
                } else {
                    // For vertical mode, CSSWG says:
                    // - Skew around the centre
                    // - Right-side down and left-side up
                    // https://github.com/w3c/csswg-drafts/issues/2869
                    glyphObliqueTf.shear(0, -SLANT_14DEG);
                }
                outlineGlyphTf *= glyphObliqueTf;
            }
            outlineGlyphTf *= ftTF;

            QPainterPath glyph = convertFromFreeTypeOutline(currentGlyph.ftface->glyph);
            glyph = outlineGlyphTf.map(glyph);

            if (!charResult.path.isEmpty()) {
                // this is for glyph clusters, unicode combining marks are always
                // added. we could have these as seperate paths, but there's no real
                // purpose, and the svg standard prefers 'ligatures' to be treated
                // as a single glyph. It simplifies things for us in any case.
                charResult.path.addPath(glyph.translated(charResult.advance));
            } else {
                charResult.path = glyph;
            }
        } else {
            QTransform bitmapTf;

            if (currentGlyph.ftface->glyph->format == FT_GLYPH_FORMAT_BITMAP) {
                if (FT_HAS_COLOR(currentGlyph.ftface)) {
                    // This applies the transform for CBDT bitmaps (e.g. Noto
                    // Color Emoji) that was set in KoFontRegistry::configureFaces
                    FT_Matrix matrix;
                    FT_Vector delta;
                    FT_Get_Transform(currentGlyph.ftface, &matrix, &delta);
                    constexpr qreal FACTOR_16 = 1.0 / 65536.0;
                    bitmapTf.setMatrix(matrix.xx * FACTOR_16, matrix.xy * FACTOR_16, 0, matrix.yx * FACTOR_16, matrix.yy * FACTOR_16, 0, 0, 0, 1);
                    KIS_SAFE_ASSERT_RECOVER_NOOP(bitmapTf.m11() == bitmapTf.m22());
                    bitmapScale = bitmapTf.m11();
                    QPointF anchor(-currentGlyph.ftface->glyph->bitmap_left, currentGlyph.ftface->glyph->bitmap_top);
                    bitmapTf = QTransform::fromTranslate(-anchor.x(), -anchor.y()) * bitmapTf
                        * QTransform::fromTranslate(anchor.x(), anchor.y());
                }
            } else {
                debugFlake << "Unsupported glyph format" << glyphFormatToStr(currentGlyph.ftface->glyph->format)
                           << "asking freetype to render it for us";
                FT_Render_Mode mode = FT_LOAD_TARGET_MODE(faceLoadFlags);
                if (mode == FT_RENDER_MODE_NORMAL && (faceLoadFlags & FT_LOAD_MONOCHROME)) {
                    mode = FT_RENDER_MODE_MONO;
                }
                if (const FT_Error err = FT_Render_Glyph(currentGlyph.ftface->glyph, mode)) {
                    warnFlake << "Failed to render glyph, freetype error" << err;
                    return false;
                }
            }

            // TODO: Handle glyph clusters better...
            charResult.image = convertFromFreeTypeBitmap(currentGlyph.ftface->glyph);

            // Check whether we need to synthesize italic by shearing the glyph:
            if (charResult.fontStyle != QFont::StyleNormal
                && !(currentGlyph.ftface->style_flags & FT_STYLE_FLAG_ITALIC)) {
                // Since we are dealing with a bitmap glyph, we'll just use a nice
                // round floating point number.
                constexpr double SLANT_BITMAP = 0.25;
                QTransform shearTf;
                QPoint shearAt;
                if (isHorizontal) {
                    shearTf.shear(-SLANT_BITMAP, 0);
                    glyphObliqueTf.shear(SLANT_BITMAP, 0);
                    shearAt = QPoint(0, currentGlyph.ftface->glyph->bitmap_top);
                } else {
                    shearTf.shear(0, SLANT_BITMAP);
                    glyphObliqueTf.shear(0, -SLANT_BITMAP);
                    shearAt = QPoint(charResult.image.width() / 2, 0);
                }
                // We need to shear around the baseline, hence the translation.
                bitmapTf = (QTransform::fromTranslate(-shearAt.x(), -shearAt.y()) * shearTf
                    * QTransform::fromTranslate(shearAt.x(), shearAt.y())) * bitmapTf;
            }

            if (!bitmapTf.isIdentity()) {
                const QSize srcSize = charResult.image.size();
                charResult.image = std::move(charResult.image).transformed(
                    bitmapTf,
                    this->textRendering == OptimizeSpeed ? Qt::FastTransformation : Qt::SmoothTransformation);

                // This does the same as `QImage::trueMatrix` to get the image
                // offset after transforming.
                const QPoint offset = bitmapTf.mapRect(QRectF({0, 0}, srcSize)).toAlignedRect().topLeft();
                currentGlyph.ftface->glyph->bitmap_left += offset.x();
                currentGlyph.ftface->glyph->bitmap_top -= offset.y();
            }
        }

        // Retreive CPAL/COLR V0 color layers, directly based off the sample
        // code in the freetype docs.
        FT_UInt layerGlyphIndex = 0;
        FT_UInt layerColorIndex = 0;
        FT_LayerIterator iterator;
        FT_Color *palette = nullptr;
        const unsigned short paletteIndex = 0;
        if (FT_Palette_Select(currentGlyph.ftface, paletteIndex, &palette) != 0) {
            palette = nullptr;
        }
        iterator.p = nullptr;
        bool haveLayers = FT_Get_Color_Glyph_Layer(currentGlyph.ftface,
                                                   currentGlyph.index,
                                                   &layerGlyphIndex,
                                                   &layerColorIndex,
                                                   &iterator);
        if (haveLayers && palette) {
            do {
                QBrush layerColor;
                bool isForeGroundColor = false;

                if (layerColorIndex == 0xFFFF) {
                    layerColor = Qt::black;
                    isForeGroundColor = true;
                } else {
                    FT_Color color = palette[layerColorIndex];
                    layerColor = QColor(color.red, color.green, color.blue, color.alpha);
                }
                FT_Load_Glyph(currentGlyph.ftface, layerGlyphIndex, faceLoadFlags);
                if (currentGlyph.ftface->glyph->format == FT_GLYPH_FORMAT_OUTLINE) {
                    // Check whether we need to synthesize bold by emboldening the glyph:
                    emboldenGlyphIfNeeded(currentGlyph, charResult);

                    QPainterPath p = convertFromFreeTypeOutline(currentGlyph.ftface->glyph);
                    p = outlineGlyphTf.map(p);
                    charResult.colorLayers.append(p);
                    charResult.colorLayerColors.append(layerColor);
                    charResult.replaceWithForeGroundColor.append(isForeGroundColor);
                } else {
                    warnFlake << "Unsupported glyph format" << glyphFormatToStr(currentGlyph.ftface->glyph->format) << "in glyph layers";
                }
            } while (FT_Get_Color_Glyph_Layer(currentGlyph.ftface,
                                              currentGlyph.index,
                                              &layerGlyphIndex,
                                              &layerColorIndex,
                                              &iterator));
        }

        charResult.visualIndex = i;
        logicalToVisual.insert(cluster, i);

        charResult.middle = false;
        QPointF advance(currentGlyph.x_advance, currentGlyph.y_advance);
        if (tabSizeInfo.contains(cluster)) {
            KoSvgText::TabSizeInfo tabSize = tabSizeInfo.value(cluster);
            qreal newAdvance = tabSize.value * ftFontUnit;
            if (tabSize.isNumber) {
                QPointF extraSpacing = isHorizontal ? QPointF(tabSize.extraSpacing * ftFontUnit, 0) : QPointF(0, tabSize.extraSpacing * ftFontUnit);
                advance = (spaceAdvance + extraSpacing) * tabSize.value;
            } else {
                advance = isHorizontal ? QPointF(newAdvance, advance.y()) : QPointF(advance.x(), newAdvance);
            }
            charResult.path = QPainterPath();
            charResult.image = QImage();
        }
        charResult.advance += ftTF.map(advance);

        bool usePixmap = !charResult.image.isNull() && charResult.path.isEmpty();

        if (usePixmap) {
            const int width = charResult.image.width();
            const int height = charResult.image.height();
            const int left = currentGlyph.ftface->glyph->bitmap_left;
            const int top = currentGlyph.ftface->glyph->bitmap_top - height;
            QRect bboxPixel(left, top, width, height);
            if (!isHorizontal) {
                bboxPixel.moveLeft(-(bboxPixel.width() / 2));
            }
            charResult.imageDrawRect = ftTF.mapRect(QRectF(bboxPixel.topLeft() * ftFontUnit, bboxPixel.size() * ftFontUnit));
        }

        QRectF bbox;
        if (isHorizontal) {
            bbox = QRectF(0,
                          charResult.descent * bitmapScale,
                          ftTF.inverted().map(charResult.advance).x(),
                          (charResult.ascent - charResult.descent) * bitmapScale);
            bbox = glyphObliqueTf.mapRect(bbox);
        } else {
            hb_font_t_up font(hb_ft_font_create_referenced(currentGlyph.ftface));
            bbox = QRectF(charResult.descent * bitmapScale,
                          0,
                          (charResult.ascent - charResult.descent) * bitmapScale,
                          ftTF.inverted().map(charResult.advance).y());
            bbox = glyphObliqueTf.mapRect(bbox);
        }
        charResult.boundingBox = ftTF.mapRect(bbox);
        if (usePixmap) {
            charResult.boundingBox |= charResult.imageDrawRect;
        }
        charResult.halfLeading = ftTF.map(QPointF(charResult.halfLeading, charResult.halfLeading)).x();
        charResult.ascent = isHorizontal? charResult.boundingBox.top(): charResult.boundingBox.right();
        charResult.descent = isHorizontal? charResult.boundingBox.bottom(): charResult.boundingBox.left();

        if (!charResult.path.isEmpty()) {
            charResult.boundingBox |= charResult.path.boundingRect();
        }
        totalAdvanceFTFontCoordinates += advance;
        charResult.cssPosition = ftTF.map(totalAdvanceFTFontCoordinates) - charResult.advance;
    }
    return true;
}

// NOLINTNEXTLINE(readability-function-cognitive-complexity)
static QPainterPath convertFromFreeTypeOutline(FT_GlyphSlotRec *glyphSlot)
{
    QPointF cp = QPointF();
    // convert the outline to a painter path
    // This is taken from qfontengine_ft.cpp.
    QPainterPath glyph;
    glyph.setFillRule(Qt::WindingFill);
    int i = 0;
    for (int j = 0; j < glyphSlot->outline.n_contours; ++j) {
        int last_point = glyphSlot->outline.contours[j];
        // qDebug() << "contour:" << i << "to" << last_point;
        QPointF start = QPointF(glyphSlot->outline.points[i].x, glyphSlot->outline.points[i].y);
        if (!(glyphSlot->outline.tags[i] & 1)) { // start point is not on curve:
            if (!(glyphSlot->outline.tags[last_point] & 1)) { // end point is not on curve:
                // qDebug() << "  start and end point are not on curve";
                start = (QPointF(glyphSlot->outline.points[last_point].x, glyphSlot->outline.points[last_point].y) + start) / 2.0;
            } else {
                // qDebug() << "  end point is on curve, start is not";
                start = QPointF(glyphSlot->outline.points[last_point].x, glyphSlot->outline.points[last_point].y);
            }
            --i; // to use original start point as control point below
        }
        start += cp;
        // qDebug() << "  start at" << start;
        glyph.moveTo(start);
        std::array<QPointF, 4> curve;
        curve[0] = start;
        size_t n = 1;
        while (i < last_point) {
            ++i;
            curve.at(n) = cp + QPointF(glyphSlot->outline.points[i].x, glyphSlot->outline.points[i].y);
            // qDebug() << "    " << i << c[n] << "tag =" <<
            // (int)g->outline.tags[i]
            //                    << ": on curve =" << (bool)(g->outline.tags[i]
            //                    & 1);
            ++n;
            switch (glyphSlot->outline.tags[i] & 3) {
            case 2:
                // cubic bezier element
                if (n < 4)
                    continue;
                curve[3] = (curve[3] + curve[2]) / 2;
                --i;
                break;
            case 0:
                // quadratic bezier element
                if (n < 3)
                    continue;
                curve[3] = (curve[1] + curve[2]) / 2;
                curve[2] = (2 * curve[1] + curve[3]) / 3;
                curve[1] = (2 * curve[1] + curve[0]) / 3;
                --i;
                break;
            case 1:
            case 3:
                if (n == 2) {
                    // qDebug() << "  lineTo" << c[1];
                    glyph.lineTo(curve[1]);
                    curve[0] = curve[1];
                    n = 1;
                    continue;
                } else if (n == 3) {
                    curve[3] = curve[2];
                    curve[2] = (2 * curve[1] + curve[3]) / 3;
                    curve[1] = (2 * curve[1] + curve[0]) / 3;
                }
                break;
            }
            // qDebug() << "  cubicTo" << c[1] << c[2] << c[3];
            glyph.cubicTo(curve[1], curve[2], curve[3]);
            curve[0] = curve[3];
            n = 1;
        }
        if (n == 1) {
            // qDebug() << "  closeSubpath";
            glyph.closeSubpath();
        } else {
            curve[3] = start;
            if (n == 2) {
                curve[2] = (2 * curve[1] + curve[3]) / 3;
                curve[1] = (2 * curve[1] + curve[0]) / 3;
            }
            // qDebug() << "  close cubicTo" << c[1] << c[2] << c[3];
            glyph.cubicTo(curve[1], curve[2], curve[3]);
        }
        ++i;
    }
    return glyph;
}

static QImage convertFromFreeTypeBitmap(FT_GlyphSlotRec *glyphSlot)
{
    KIS_ASSERT(glyphSlot->bitmap.width <= INT32_MAX);
    KIS_ASSERT(glyphSlot->bitmap.rows <= INT32_MAX);
    QImage img;
    const int height = static_cast<int>(glyphSlot->bitmap.rows);
    const QSize size(static_cast<int>(glyphSlot->bitmap.width), height);

    if (glyphSlot->bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
        img = QImage(size, QImage::Format_Mono);
        uchar *src = glyphSlot->bitmap.buffer;
        KIS_ASSERT(glyphSlot->bitmap.pitch >= 0);
        for (int y = 0; y < height; y++) {
            memcpy(img.scanLine(y), src, static_cast<size_t>(glyphSlot->bitmap.pitch));
            src += glyphSlot->bitmap.pitch;
        }
    } else if (glyphSlot->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY) {
        img = QImage(size, QImage::Format_Grayscale8);
        uchar *src = glyphSlot->bitmap.buffer;
        KIS_ASSERT(glyphSlot->bitmap.pitch >= 0);
        for (int y = 0; y < height; y++) {
            memcpy(img.scanLine(y), src, static_cast<size_t>(glyphSlot->bitmap.pitch));
            src += glyphSlot->bitmap.pitch;
        }
    } else if (glyphSlot->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA) {
        img = QImage(size, QImage::Format_ARGB32_Premultiplied);
        const uint8_t *src = glyphSlot->bitmap.buffer;
        for (int y = 0; y < height; y++) {
            auto *argb = reinterpret_cast<QRgb *>(img.scanLine(y));
            for (unsigned int x = 0; x < glyphSlot->bitmap.width; x++) {
                argb[x] = qRgba(src[2], src[1], src[0], src[3]);
                src += 4;
            }
        }
    }

    return img;
}
