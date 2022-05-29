/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSvgTextShape.h"

#include <QTextLayout>

#include <raqm.h>
#include <fontconfig/fontconfig.h>
#include FT_COLOR_H
#include <hb.h>
#include <hb-ft.h>
#include <hb-ot.h>
#include <linebreak.h>
#include <graphemebreak.h>

#include <klocalizedstring.h>

#include "KoSvgText.h"
#include "KoSvgTextProperties.h"
#include <KoShapeContainer_p.h>
#include <text/KoSvgTextChunkShape_p.h>
#include <text/KoSvgTextShapeMarkupConverter.h>
#include <text/KoFontRegistery.h>
#include <text/KoCssTextUtils.h>
#include <KoDocumentResourceManager.h>
#include <KoShapeController.h>

#include "kis_debug.h"

#include <KoXmlNS.h>
#include <KoShapeLoadingContext.h>
#include <KoIcon.h>
#include <KoProperties.h>
#include <KoColorBackground.h>
#include <KoPathShape.h>
#include <KoClipMaskPainter.h>

#include <SvgLoadingContext.h>
#include <SvgGraphicContext.h>
#include <SvgUtil.h>

#include <QApplication>
#include <QThread>
#include <vector>
#include <memory>
#include <QPainter>
#include <QPainterPath>
#include <QFileInfo>
#include <QtMath>
#include <QLineF>

#include <boost/optional.hpp>

#include <text/KoSvgTextChunkShapeLayoutInterface.h>

#include <FlakeDebug.h>

#include <QSharedData>

enum BreakType {
    NoBreak,
    SoftBreak,
    HardBreak
};

struct CharacterResult {
    QPointF finalPosition;
    qreal rotate = 0.0;
    bool hidden = false; // whether the character will be drawn.
    // we can't access characters that aren't part of a typographic character
    // so we're setting 'middle' to true and addressable to 'false'.
    // The original svg specs' notion of addressable character relies on utf16,
    // and it's suggested to have it per-typographic character.
    // https://github.com/w3c/svgwg/issues/537
    bool addressable = true; // whether the character is not discarded for various reasons.
    bool middle = false; // whether the character is the second of last of a typographic character.
    bool anchored_chunk = false; // whether this is the start of a new chunk.

    QPainterPath path;
    QImage image{0};

    QVector<QPainterPath> colorLayers;
    QVector<QBrush> colorLayerColors;
    QVector<bool> replaceWithForeGroundColor;

    QRectF boundingBox;
    int typographic_index = -1;
    QPointF cssPosition = QPointF();
    QPointF advance;
    BreakType breakType = NoBreak;
    bool collapseIfAtEndOfLine = false;
    qreal textLengthApplied = false;

    KoSvgText::TextAnchor anchor = KoSvgText::AnchorStart;
    KoSvgText::Direction direction = KoSvgText::DirectionLeftToRight;
    bool textOnPath = false;

};

class KoSvgTextShape::Private
{
public:

    // NOTE: the cache data is shared between all the instances of
    //       the shape, though it will be reset locally if the
    //       accessing thread changes

    Private()
    {}

    Private(const Private &rhs)
        : textRendering(rhs.textRendering)
        , xRes(rhs.xRes)
        , yRes(rhs.yRes)
        , result(rhs.result)
    {
    }

    TextRendering textRendering = Auto;
    int xRes = 72;
    int yRes = 72;

    QVector<CharacterResult> result;


    void clearAssociatedOutlines(const KoShape *rootShape);
    QPainterPath convertFromFreeTypeOutline(FT_GlyphSlotRec *glyphSlot);
    QImage convertFromFreeTypeBitmap(FT_GlyphSlotRec *glyphSlot);
    void breakLines(KoSvgTextProperties properties, QMap<int, int> indexToTypographic, QVector<CharacterResult> &result);
    void applyTextLength(const KoShape *rootShape,
                         QVector<CharacterResult> &result,
                         int &currentIndex,
                         int &resolvedDescendentNodes,
                         bool isHorizontal);
    void applyAnchoring(QVector<CharacterResult> &result,
                        bool isHorizontal, bool inlineSize);
    qreal characterResultOnPath(CharacterResult &cr, qreal length, qreal offset, bool isHorizontal, bool isClosed);
    QPainterPath stretchGlyphOnPath(QPainterPath glyph, QPainterPath path, bool isHorizontal, qreal offset, bool isClosed);
    void applyTextPath(const KoShape *rootShape,
                         QVector<CharacterResult> &result, bool isHorizontal);
    void computeFontMetrics(const KoShape *rootShape, QMap<int, int> parentBaselineTable, qreal parentFontSize,
                            QPointF superScript, QPointF subScript, QVector<CharacterResult> &result,
                            int &currentIndex, qreal res, bool isHorizontal);
    void computeTextDecorations(const KoShape *rootShape, QVector<CharacterResult> result,
                                KoPathShape *textPath, qreal textPathoffset, bool side, int &currentIndex, bool isHorizontal);
    void paintPaths(QPainter &painter, KoShapePaintingContext &paintContext,
                    QPainterPath outlineRect,
                    const KoShape *rootShape, QVector<CharacterResult> &result, QPainterPath &chunk, int &currentIndex);

};

KoSvgTextShape::KoSvgTextShape()
    : KoSvgTextChunkShape()
    , d(new Private)
{
    setShapeId(KoSvgTextShape_SHAPEID);
}

KoSvgTextShape::KoSvgTextShape(const KoSvgTextShape &rhs)
    : KoSvgTextChunkShape(rhs)
    , d(new Private(*rhs.d))
{
    setShapeId(KoSvgTextShape_SHAPEID);
}

KoSvgTextShape::~KoSvgTextShape()
{
}

KoShape *KoSvgTextShape::cloneShape() const
{
    return new KoSvgTextShape(*this);
}

void KoSvgTextShape::shapeChanged(ChangeType type, KoShape *shape)
{
    KoSvgTextChunkShape::shapeChanged(type, shape);

    if (type == StrokeChanged || type == BackgroundChanged || type == ContentChanged) {
        relayout();
    }
}

void KoSvgTextShape::paintComponent(QPainter &painter, KoShapePaintingContext &paintContext) const
{

    Q_UNUSED(paintContext);

    //if (d->textRendering == OptimizeLegibility) {
        /**
         * HACK ALERT:
         *
         * For hinting and bitmaps, we need to get the hinting metrics from freetype,
         * but those need the DPI. We can't get the DPI normally, however, neither rotate
         * and shear change the length of a line, and it may not be that bad if freetype
         * receives a scaled value for the DPI.
         */
        int xRes = qRound(painter.transform().map(QLineF(QPointF(), QPointF(72, 0))).length());
        int yRes = qRound(painter.transform().map(QLineF(QPointF(), QPointF(0, 72))).length());
        if (xRes != d->xRes || yRes != d->yRes) {
            d->xRes = xRes;
            d->yRes = yRes;
            relayout();
        }
    /*} else {
        if (72 != d->xRes || 72 != d->yRes) {
            d->xRes = 72;
            d->yRes = 72;
            relayout();
        }
    }*/
    painter.save();
    if (d->textRendering == OptimizeSpeed) {
        painter.setRenderHint(QPainter::Antialiasing, false);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
    }

    QPainterPath chunk;
    int currentIndex = 0;
    if (d->result.size()>0) {
        d->paintPaths(painter, paintContext, this->outline(), this, d->result, chunk, currentIndex);
    }
    /* Debug
    Q_FOREACH (KoShape *child, this->shapes()) {
        const KoSvgTextChunkShape *textPathChunk = dynamic_cast<const KoSvgTextChunkShape*>(child);
        if (textPathChunk) {
            painter.save();
            painter.setPen(Qt::magenta);
            painter.setOpacity(0.5);
            if (textPathChunk->layoutInterface()->textPath()) {
                QPainterPath p = textPathChunk->layoutInterface()->textPath()->outline();
                p = textPathChunk->layoutInterface()->textPath()->transformation().map(p);
                painter.strokePath(p, QPen(Qt::green));
                painter.drawPoint(p.pointAtPercent(0));
                painter.drawPoint(p.pointAtPercent(p.percentAtLength(p.length()*0.5)));
                painter.drawPoint(p.pointAtPercent(1.0));
            }
            painter.restore();
        }
    }
    */
    painter.restore();

}

void KoSvgTextShape::paintStroke(QPainter &painter, KoShapePaintingContext &paintContext) const
{
    Q_UNUSED(painter);
    Q_UNUSED(paintContext);

    // do nothing! everything is painted in paintComponent()
}

QPainterPath KoSvgTextShape::textOutline() const
{

    /*QPainterPath result;
    result.setFillRule(Qt::WindingFill);
    qDebug() << "starting creation textoutlne";

    for (int layoutIndex = 0; layoutIndex < (int)d->cachedLayouts.size(); layoutIndex++) {

        for (int j = 0; j < layout->lineCount(); j++) {
            QTextLine line = layout->lineAt(j);

            Q_FOREACH (const QGlyphRun &run, line.glyphRuns()) {
                const QVector<quint32> indexes = run.glyphIndexes();
                const QVector<QPointF> positions = run.positions();
                const QRawFont font = run.rawFont();

                KIS_SAFE_ASSERT_RECOVER(indexes.size() == positions.size()) { continue; }

                for (int k = 0; k < indexes.size(); k++) {
                    QPainterPath glyph = font.pathForGlyph(indexes[k]);
                    glyph.translate(positions[k] + layoutOffset);
                    result += glyph;
                }

                const qreal thickness = font.lineThickness();
                const QRectF runBounds = run.boundingRect();

                if (run.overline()) {
                    // the offset is calculated to be consistent with the way how Qt renders the text
                    const qreal y = line.y();
                    QRectF overlineBlob(runBounds.x(), y, runBounds.width(), thickness);
                    overlineBlob.translate(layoutOffset);

                    QPainterPath path;
                    path.addRect(overlineBlob);

                    // don't use direct addRect, because it doesn't care about Qt::WindingFill
                    result += path;
                }

                if (run.strikeOut()) {
                    // the offset is calculated to be consistent with the way how Qt renders the text
                    const qreal y = line.y() + 0.5 * line.height();
                    QRectF strikeThroughBlob(runBounds.x(), y, runBounds.width(), thickness);
                    strikeThroughBlob.translate(layoutOffset);

                    QPainterPath path;
                    path.addRect(strikeThroughBlob);

                    // don't use direct addRect, because it doesn't care about Qt::WindingFill
                    result += path;
                }

                if (run.underline()) {
                    const qreal y = line.y() + line.ascent() + font.underlinePosition();
                    QRectF underlineBlob(runBounds.x(), y, runBounds.width(), thickness);
                    underlineBlob.translate(layoutOffset);

                    QPainterPath path;
                    path.addRect(underlineBlob);

                    // don't use direct addRect, because it doesn't care about Qt::WindingFill
                    result += path;
                }
            }
        }
    }*/

    return QPainterPath();
}

void KoSvgTextShape::setTextRenderingFromString(QString textRendering)
{
    if (textRendering == "optimizeSpeed") {
        d->textRendering = OptimizeSpeed;
    } else if (textRendering == "optimizeLegibility") {
        d->textRendering = OptimizeLegibility;
    } else if (textRendering == "geometricPrecision") {
        d->textRendering = GeometricPrecision;
    } else{
        d->textRendering = Auto;
    }
}

QString KoSvgTextShape::textRenderingString() const
{
    if (d->textRendering == OptimizeSpeed) {
        return "optimizeSpeed";
    } else if (d->textRendering == OptimizeLegibility) {
        return "optimizeLegibility";
    } else if (d->textRendering == GeometricPrecision) {
        return "geometricPrecision";
    } else {
        return "auto";
    }
}

void KoSvgTextShape::resetTextShape()
{
    KoSvgTextChunkShape::resetTextShape();
    relayout();
}

void KoSvgTextShape::relayout() const
{
    d->clearAssociatedOutlines(this);

    // The following is based on the text-layout algorithm in SVG 2.
    KoSvgText::WritingMode writingMode = KoSvgText::WritingMode(
                this->textProperties().propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());
    KoSvgText::Direction direction = KoSvgText::Direction(
                this->textProperties().propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
    KoSvgText::AutoValue inlineSize = this->textProperties().propertyOrDefault(
                KoSvgTextProperties::InlineSizeId).value<KoSvgText::AutoValue>();

    bool isHorizontal = writingMode == KoSvgText::HorizontalTB;

    FT_Int32 loadFlags = FT_LOAD_RENDER;

    if (d->textRendering == GeometricPrecision || d->textRendering == Auto) {
        // without load_no_hinting, the advance and offset will be rounded
        // to nearest pixel, which we don't want as we're using the vector outline.

        loadFlags |= FT_LOAD_NO_HINTING;
    } else {
        // When using hinting, sometimes the bounding box does not encompass the
        // drawn glyphs properly.
        // The default hinting works best for vertical, while the 'light' hinting mode
        // works best for horizontal.
        if (isHorizontal) {
            loadFlags |= FT_LOAD_TARGET_LIGHT;
        }
    }
    // Whenever the freetype docs talk about a 26.6 floating point unit, they mean a 1/64 value.
    const qreal ftFontUnit = 64.0;
    const qreal ftFontUnitFactor = 1/ftFontUnit;
    QTransform ftTF = QTransform::fromScale(ftFontUnitFactor, -ftFontUnitFactor);
    qreal finalRes = qMin(d->xRes, d->yRes);
    qreal scaleToPT = float(72./finalRes);
    qreal scaleToPixel = float(finalRes/72.);
    QTransform dpiScale = QTransform::fromScale(scaleToPT, scaleToPT);
    ftTF *= dpiScale;

    // First, get text. We use the subChunks because that handles bidi for us.
    // SVG 1.1 suggests that each time the xy position of a piece of text changes,
    // that this should be seperately shaped, but according to SVGWG issues 631 and 635
    // noone who actually uses bidi likes this, and it also complicates the algorithm,
    // so we're not doing that. Anchored Chunks will get generated later.
    // https://github.com/w3c/svgwg/issues/631
    // https://github.com/w3c/svgwg/issues/635

    QVector<KoSvgTextChunkShapeLayoutInterface::SubChunk> textChunks = layoutInterface()->collectSubChunks();
    QString text;
    for (KoSvgTextChunkShapeLayoutInterface::SubChunk chunk : textChunks) {
        text.append(chunk.text);
    }

    // 1. Setup.

    //KoSvgText::TextSpaceTrims trims = this->textProperties().propertyOrDefault(KoSvgTextProperties::TextTrimId).value<KoSvgText::TextSpaceTrims>();
    KoSvgText::TextWrap wrap = KoSvgText::TextWrap(this->textProperties().propertyOrDefault(KoSvgTextProperties::TextWrapId).toInt());
    KoSvgText::TextSpaceCollapse collapse = KoSvgText::TextSpaceCollapse(this->textProperties().propertyOrDefault(KoSvgTextProperties::TextCollapseId).toInt());
    KoSvgText::LineBreak linebreakStrictness =  KoSvgText::LineBreak(this->textProperties().property(KoSvgTextProperties::LineBreakId).toInt());
    QVector<bool> collapseChars = KoCssTextUtils::collapseSpaces(text, collapse);
    QString lang = this->textProperties().property(KoSvgTextProperties::TextLanguage).toString().toUtf8();
    if (!lang.isEmpty()) {
        // Libunibreak currently only has support for strict, and even then only for very specific cases.
        if (linebreakStrictness == KoSvgText::LineBreakStrict) {
            lang += "-strict";
        }
    }
    char lineBreaks[text.size()];
    set_linebreaks_utf16(text.utf16(), text.size(), lang.toUtf8().data(), lineBreaks);
    char graphemeBreaks[text.size()];
    set_graphemebreaks_utf16(text.utf16(), text.size(), lang.toUtf8().data(), graphemeBreaks);

    int globalIndex = 0;
    QVector<CharacterResult> result(text.size());
    // 3. Resolve character positioning.
    // This is done earlier so it's possible to get preresolved transforms from the subchunks.
    QVector<KoSvgText::CharTransformation> resolvedTransforms(text.size());

    // pass everything to a css-compatible text-layout algortihm.
    raqm_t_up layout = toLibraryResource(raqm_create());

    if (raqm_set_text_utf16(layout.data(), text.utf16(), text.size())) {
        if (writingMode == KoSvgText::VerticalRL || writingMode == KoSvgText::VerticalLR) {
            raqm_set_par_direction(layout.data(), raqm_direction_t::RAQM_DIRECTION_TTB);
        } else if (direction == KoSvgText::DirectionRightToLeft) {
            raqm_set_par_direction(layout.data(), raqm_direction_t::RAQM_DIRECTION_RTL);
        } else {
            raqm_set_par_direction(layout.data(), raqm_direction_t::RAQM_DIRECTION_LTR);
        }

        int start = 0;
        int length = 0;
        bool textInPath = false;
        for (KoSvgTextChunkShapeLayoutInterface::SubChunk chunk : textChunks) {
            length = chunk.text.size();
            KoSvgTextProperties properties = chunk.format.associatedShapeWrapper().shape()->textProperties();

            // In this section we retrieve the resolved transforms and direction/anchoring that we can get from
            // the subchunks.
            KoSvgText::TextAnchor anchor = KoSvgText::TextAnchor(
                        properties.propertyOrDefault(KoSvgTextProperties::TextAnchorId).toInt());
            KoSvgText::Direction direction = KoSvgText::Direction(
                        properties.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
            KoSvgText::WordBreak wordBreakStrictness = KoSvgText::WordBreak(properties.propertyOrDefault(KoSvgTextProperties::WordBreakId).toInt());

            for (int i = 0; i < length; i++) {
                CharacterResult cr = result[start + i];
                cr.anchor = anchor;
                cr.direction = direction;
                if (chunk.textInPath != textInPath && i==0){
                    cr.anchored_chunk = true;
                    textInPath = chunk.textInPath;
                }
                if (lineBreaks[start+i] == LINEBREAK_MUSTBREAK) {
                    cr.breakType = HardBreak;
                    cr.collapseIfAtEndOfLine = true;
                } else if (lineBreaks[start+i] == LINEBREAK_ALLOWBREAK && wrap != KoSvgText::NoWrap) {
                    cr.breakType = SoftBreak;
                    cr.collapseIfAtEndOfLine = KoCssTextUtils::collapseLastSpace(text.at(start+i), collapse);
                }
                if (wordBreakStrictness == KoSvgText::WordBreakBreakAll || linebreakStrictness == KoSvgText::LineBreakAnywhere) {
                    if (graphemeBreaks[start+i] == GRAPHEMEBREAK_BREAK && cr.breakType == NoBreak) {
                        cr.breakType = SoftBreak;
                    }
                }
                result[start + i] = cr;
                //TODO: figure out how to use addressability to only set transforms on addressable chars.
                //!collapseChars.at(i);
                if (i < chunk.transformation.size()) {
                    KoSvgText::CharTransformation newTransform = chunk.transformation.at(i);

                    if (chunk.textInPath) {
                        if (isHorizontal) {
                            newTransform.xPos.reset();
                            newTransform.yPos.reset();
                        }
                    }
                    resolvedTransforms[start + i] = newTransform;
                } else if (start > 0) {
                    resolvedTransforms[start + i].rotate = resolvedTransforms[start + i - 1].rotate;
                }
            }

            QVector<int> lengths;
            QStringList fontFeatures = properties.fontFeaturesForText(start, length);

            qreal fontSize = properties.property(KoSvgTextProperties::FontSizeId).toReal();
            const QFont::Style style =
                QFont::Style(properties.propertyOrDefault(KoSvgTextProperties::FontStyleId).toInt());
            std::vector<FT_FaceUP> faces = KoFontRegistery::instance()->facesForCSSValues(properties.property(KoSvgTextProperties::FontFamiliesId).toStringList(),
                                                                                    lengths,
                                                                                    chunk.text,
                                                                                    fontSize,
                                                                                    properties.propertyOrDefault(KoSvgTextProperties::FontWeightId).toInt(),
                                                                                    properties.propertyOrDefault(KoSvgTextProperties::FontStretchId).toInt(),
                                                                                    style != QFont::StyleNormal);
            KoFontRegistery::instance()->configureFaces(faces, fontSize, finalRes, finalRes, properties.fontAxisSettings());
            if (properties.hasProperty(KoSvgTextProperties::TextLanguage)) {
                raqm_set_language(layout.data(),
                                  properties.property(KoSvgTextProperties::TextLanguage).toString().toUtf8(),
                                  start, length);
            }
            for (QString feature: fontFeatures) {
                qDebug() << "adding feature" << feature;
                raqm_add_font_feature(layout.data(), feature.toUtf8(), feature.toUtf8().size());
            }
            KoSvgText::AutoValue letterSpacing = properties.propertyOrDefault(KoSvgTextProperties::LetterSpacingId).value<KoSvgText::AutoValue>();

            if (!letterSpacing.isAuto) {
                raqm_set_letter_spacing_range(layout.data(), letterSpacing.customValue * ftFontUnit * scaleToPixel, false, start, length);
            }
            KoSvgText::AutoValue wordSpacing = properties.propertyOrDefault(KoSvgTextProperties::WordSpacingId).value<KoSvgText::AutoValue>();
            if (!wordSpacing.isAuto) {
                raqm_set_word_spacing_range(layout.data(), wordSpacing.customValue * ftFontUnit * scaleToPixel, false, start, length);
            }

            for (int i = 0; i < lengths.size(); i++ )  {
                length = lengths.at(i);
                FT_Int32 faceLoadFlags = loadFlags;
                FT_FaceUP &face = faces.at(i);
                if (FT_HAS_COLOR(face)) {
                    loadFlags |= FT_LOAD_COLOR;
                }
                if (!isHorizontal && FT_HAS_VERTICAL(face)) {
                    loadFlags |= FT_LOAD_VERTICAL_LAYOUT;
                }
                if (start == 0) {
                    raqm_set_freetype_face(layout.data(), face.data());
                    raqm_set_freetype_load_flags(layout.data(), faceLoadFlags);
                }
                if (length > 0) {
                    raqm_set_freetype_face_range(layout.data(), face.data(), start, length);
                    raqm_set_freetype_load_flags_range(layout.data(), faceLoadFlags, start, length);
                }
                start += length;
            }

        }
        qDebug() << "text-length:" << text.size();
    }

    if (raqm_layout(layout.data())) {
        qDebug() << "layout succeeded";
    }

    // 2. Set flags and assign initial positions
    // We also retreive a glyph path here.
    size_t count;
    raqm_glyph_t *glyphs = raqm_get_glyphs (layout.data(), &count);
    if (!glyphs) {
        return;
    }

    QPointF totalAdvanceFTFontCoordinates;
    QMap<int, int> indexToTypographic;

    for (int g=0; g < int(count); g++) {

        CharacterResult charResult = result[glyphs[g].cluster];
        charResult.addressable = !collapseChars.at(glyphs[g].cluster);
        if (!charResult.addressable) {
            continue;
        }

        FT_Int32 faceLoadFlags = loadFlags;
        if (!isHorizontal && FT_HAS_VERTICAL(glyphs[g].ftface)) {
            faceLoadFlags |= FT_LOAD_VERTICAL_LAYOUT;
        }
        if (FT_HAS_COLOR(glyphs[g].ftface)) {
            faceLoadFlags |= FT_LOAD_COLOR;
        }

        int error = FT_Load_Glyph(glyphs[g].ftface, glyphs[g].index, faceLoadFlags);
        if (error != 0) {
            continue;
        }

        //qDebug() << "glyph" << g << "cluster" << glyphs[g].cluster << glyphs[g].index;

        FT_Matrix matrix;
        FT_Vector delta;
        FT_Get_Transform(glyphs[g].ftface, &matrix, &delta);
        QTransform glyphTf;
        qreal factor_16 = 1.0 / 65536.0;
        glyphTf.setMatrix(matrix.xx*factor_16, matrix.xy*factor_16, 0, matrix.yx*factor_16, matrix.yy*factor_16, 0, 0, 0, 1);

        QPainterPath glyph = d->convertFromFreeTypeOutline(glyphs[g].ftface->glyph);

        glyph.translate(glyphs[g].x_offset, glyphs[g].y_offset);
        glyph = glyphTf.map(glyph);
        glyph = ftTF.map(glyph);


        if (!charResult.path.isEmpty()) {
            // this is for glyph clusters, unicode combining marks are always added.
            // we could have these as seperate paths, but there's no real purpose,
            // and the svg standard prefers 'ligatures' to be treated as a single glyph.
            // It simplifies things for us in any case.
            charResult.path.addPath(glyph.translated(charResult.advance));
        } else {
            charResult.path = glyph;
        }
        // TODO: Handle glyph clusters better...
        charResult.image = d->convertFromFreeTypeBitmap(glyphs[g].ftface->glyph).transformed(
                    glyphTf, d->textRendering == OptimizeSpeed? Qt::FastTransformation: Qt::SmoothTransformation);


        // Retreive CPAL/COLR V0 color layers, directly based off the sample code in the freetype docs.
        FT_UInt layerGlyphIndex = 0;
        FT_UInt layerColorIndex = 0;
        FT_LayerIterator  iterator;
        FT_Color*         palette;
        int paletteIndex = 0;
        error = FT_Palette_Select( glyphs[g].ftface, paletteIndex, &palette );
        if ( error ) {
            palette = NULL;
        }
        iterator.p = NULL;
        bool haveLayers = FT_Get_Color_Glyph_Layer( glyphs[g].ftface,
                                                  glyphs[g].index,
                                                  &layerGlyphIndex,
                                                  &layerColorIndex,
                                                  &iterator );
        if (haveLayers && palette) {
            do {
                QBrush layerColor;
                bool isForeGroundColor = false;

                if ( layerColorIndex == 0xFFFF ) {
                    layerColor = Qt::black;
                    isForeGroundColor = true;
                } else {
                    FT_Color color = palette[layerColorIndex];
                    layerColor = QColor(color.red, color.green, color.blue, color.alpha);
                }
                FT_Load_Glyph(glyphs[g].ftface, layerGlyphIndex, faceLoadFlags);
                QPainterPath p = d->convertFromFreeTypeOutline(glyphs[g].ftface->glyph);
                p.translate(glyphs[g].x_offset, glyphs[g].y_offset);
                charResult.colorLayers.append(ftTF.map(p));
                charResult.colorLayerColors.append(layerColor);
                charResult.replaceWithForeGroundColor.append(isForeGroundColor);

            } while (FT_Get_Color_Glyph_Layer( glyphs[g].ftface,
                                               glyphs[g].index,
                                               &layerGlyphIndex,
                                               &layerColorIndex,
                                               &iterator ));
        }

        charResult.typographic_index = g;
        indexToTypographic.insert(glyphs[g].cluster, g);
        if (glyphs[g].cluster == 0) {
            charResult.anchored_chunk = true;
        }

        charResult.middle = false;
        QPointF advance(glyphs[g].x_advance, glyphs[g].y_advance);
        charResult.advance += ftTF.map(advance);

        bool usePixmap = !charResult.image.isNull() && charResult.path.isEmpty();

        QRectF bbox;
        if (usePixmap) {
            QPointF topLeft((glyphs[g].ftface->glyph->bitmap_left*64),
                            (glyphs[g].ftface->glyph->bitmap_top*64));
            topLeft = glyphTf.map(topLeft);
            QPointF bottomRight (topLeft.x() + (charResult.image.height()*64),
                                 topLeft.y() - (charResult.image.height()*64));
            bbox = QRectF(topLeft, bottomRight);
            if (isHorizontal) {
                bbox.setWidth(ftTF.inverted().map(charResult.advance).x());
            } else {
                bbox.setHeight(ftTF.inverted().map(charResult.advance).y());
                bbox.translate(- (bbox.width() * 0.5), 0);
            }
        } else if (isHorizontal) {
            bbox = QRectF(0,
                          glyphs[g].ftface->size->metrics.descender,
                          ftTF.inverted().map(charResult.advance).x(),
                          (glyphs[g].ftface->size->metrics.ascender
                           - glyphs[g].ftface->size->metrics.descender));
            bbox = glyphTf.mapRect(bbox);
        } else {
            hb_font_t_up font = toLibraryResource(hb_ft_font_create_referenced(glyphs[g].ftface));
            hb_position_t ascender = 0;
            hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_VERTICAL_ASCENDER, &ascender);
            hb_position_t descender = 0;
            hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_VERTICAL_DESCENDER, &descender);
            bbox = QRectF(descender,
                          0,
                          ascender - descender,
                          ftTF.inverted().map(charResult.advance).y());
            bbox = glyphTf.mapRect(bbox);
        }
        charResult.boundingBox = ftTF.mapRect(bbox);

        if (!charResult.path.isEmpty()) {
            charResult.boundingBox |= charResult.path.boundingRect();
        }
        totalAdvanceFTFontCoordinates += advance;
        charResult.cssPosition = ftTF.map(totalAdvanceFTFontCoordinates) - charResult.advance;

        result[glyphs[g].cluster] = charResult;
    }

    // fix it so that characters that are in the 'middle' due to either being
    // surrogates or part of a ligature, are marked as such.
    int firstCluster = 0;
    for (int i = 0; i< result.size(); i++) {
        if (result.at(i).typographic_index != -1) {
            firstCluster = i;
        } else {
            result[firstCluster].breakType = result.at(i).breakType;
            result[firstCluster].collapseIfAtEndOfLine = result.at(i).collapseIfAtEndOfLine;
            result[i].middle = true;
            result[i].addressable = false;
        }
    }

    // Handle linebreaking.
    d->breakLines(this->textProperties(), indexToTypographic, result);

    // Handle baseline alignment.
    globalIndex = 0;
    d->computeFontMetrics(this, QMap<int, int>(), 0, QPointF(), QPointF(), result, globalIndex, finalRes, isHorizontal);

    // This is the best point to start applying linebreaking and text-wrapping.
    // If we're doing text-wrapping we should skip the other positioning steps of the algorithm.

    if (inlineSize.isAuto) {
        // 4. Adjust positions: dx, dy
        QPointF shift = QPointF();

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
                if (transform.startsNewChunk()) {
                    charResult.anchored_chunk = true;
                }
                result[i] = charResult;
            }
        }

        // 5. Apply ‘textLength’ attribute
        globalIndex = 0;
        int resolved = 0;
        d->applyTextLength(this, result, globalIndex, resolved, isHorizontal);

        // 6. Adjust positions: x, y

        // https://github.com/w3c/svgwg/issues/617
        shift = QPointF();
        //bool setNextAnchor = false;
        for (int i = 0; i < result.size(); i++) {
            if (result.at(i).addressable) {
                KoSvgText::CharTransformation transform = resolvedTransforms[i];
                CharacterResult charResult = result[i];
                if (transform.xPos) {
                    qreal d = transform.dxPos? *transform.dxPos : 0.0;
                    shift.setX(*transform.xPos + (d - charResult.finalPosition.x()));
                }
                if (transform.yPos) {
                    qreal d = transform.dyPos? *transform.dyPos : 0.0;
                    shift.setY(*transform.yPos + (d - charResult.finalPosition.y()));
                }
                charResult.finalPosition += shift;

                /*
            if (setNextAnchor) {
                charResult.anchored_chunk = true;
            }

            if (charResult.middle && charResult.anchored_chunk) {
                charResult.anchored_chunk = false;
                if (i+1 < result.size()) {
                    setNextAnchor = true;
                } else {
                    setNextAnchor = false;
                }
            }
            */

                result[i] = charResult;
            }
        }


        // 7. Apply anchoring
        d->applyAnchoring(result, isHorizontal, false);


        // Computing the textDecorations needs to happen before applying the textPath to the
        // results, as we need the unapplied result vector for positioning.
        globalIndex = 0;
        d->computeTextDecorations(this, result, nullptr, 0.0, false, globalIndex, isHorizontal);

        // 8. Position on path


        d->applyTextPath(this, result, isHorizontal);
    } else {
        d->applyAnchoring(result, isHorizontal, true);
        globalIndex = 0;
        d->computeTextDecorations(this, result, nullptr, 0.0, false, globalIndex, isHorizontal);
    }


    // 9. return result.
    d->result = result;
    globalIndex = 0;
    QTransform tf;
    for (KoSvgTextChunkShapeLayoutInterface::SubChunk chunk : textChunks) {
        KoSvgText::AssociatedShapeWrapper wrapper = chunk.format.associatedShapeWrapper();
        int j = chunk.text.size();
        for (int i = globalIndex; i< globalIndex + j; i++) {
            if (result.at(i).addressable && result.at(i).hidden == false) {
                tf.reset();
                tf.translate(result.at(i).finalPosition.x(), result.at(i).finalPosition.y());
                tf.rotateRadians(result.at(i).rotate);
                wrapper.addCharacterRect(tf.mapRect(result.at(i).boundingBox));
            }
        }
        globalIndex += j;
    }
}

void KoSvgTextShape::Private::clearAssociatedOutlines(const KoShape *rootShape)
{
    const KoSvgTextChunkShape *chunkShape = dynamic_cast<const KoSvgTextChunkShape*>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);

    chunkShape->layoutInterface()->clearAssociatedOutline();

    Q_FOREACH (KoShape *child, chunkShape->shapes()) {
        clearAssociatedOutlines(child);
    }
}

QPainterPath KoSvgTextShape::Private::convertFromFreeTypeOutline(FT_GlyphSlotRec *glyphSlot) {
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
        if (!(glyphSlot->outline.tags[i] & 1)) {               // start point is not on curve:
            if (!(glyphSlot->outline.tags[last_point] & 1)) {  // end point is not on curve:
                //qDebug() << "  start and end point are not on curve";
                start = (QPointF(glyphSlot->outline.points[last_point].x,
                                 glyphSlot->outline.points[last_point].y) + start) / 2.0;
            } else {
                //qDebug() << "  end point is on curve, start is not";
                start = QPointF(glyphSlot->outline.points[last_point].x,
                                glyphSlot->outline.points[last_point].y);
            }
            --i;   // to use original start point as control point below
        }
        start += cp;
        //qDebug() << "  start at" << start;
        glyph.moveTo(start);
        QPointF c[4];
        c[0] = start;
        int n = 1;
        while (i < last_point) {
            ++i;
            c[n] = cp + QPointF(glyphSlot->outline.points[i].x, glyphSlot->outline.points[i].y);
            //qDebug() << "    " << i << c[n] << "tag =" << (int)g->outline.tags[i]
            //                   << ": on curve =" << (bool)(g->outline.tags[i] & 1);
            ++n;
            switch (glyphSlot->outline.tags[i] & 3) {
            case 2:
                // cubic bezier element
                if (n < 4)
                    continue;
                c[3] = (c[3] + c[2])/2;
                --i;
                break;
            case 0:
                // quadratic bezier element
                if (n < 3)
                    continue;
                c[3] = (c[1] + c[2])/2;
                c[2] = (2*c[1] + c[3])/3;
                c[1] = (2*c[1] + c[0])/3;
                --i;
                break;
            case 1:
            case 3:
                if (n == 2) {
                    //qDebug() << "  lineTo" << c[1];
                    glyph.lineTo(c[1]);
                    c[0] = c[1];
                    n = 1;
                    continue;
                } else if (n == 3) {
                    c[3] = c[2];
                    c[2] = (2*c[1] + c[3])/3;
                    c[1] = (2*c[1] + c[0])/3;
                }
                break;
            }
            //qDebug() << "  cubicTo" << c[1] << c[2] << c[3];
            glyph.cubicTo(c[1], c[2], c[3]);
            c[0] = c[3];
            n = 1;
        }
        if (n == 1) {
            //qDebug() << "  closeSubpath";
            glyph.closeSubpath();
        } else {
            c[3] = start;
            if (n == 2) {
                c[2] = (2*c[1] + c[3])/3;
                c[1] = (2*c[1] + c[0])/3;
            }
            //qDebug() << "  close cubicTo" << c[1] << c[2] << c[3];
            glyph.cubicTo(c[1], c[2], c[3]);
        }
        ++i;
    }
    return glyph;
}

QImage KoSvgTextShape::Private::convertFromFreeTypeBitmap(FT_GlyphSlotRec *glyphSlot)
{
   QImage img;
   QSize size(glyphSlot->bitmap.width, glyphSlot->bitmap.rows);

   if (glyphSlot->bitmap.pixel_mode == FT_PIXEL_MODE_MONO) {
       img = QImage(size, QImage::Format_Mono);
       uchar *src = glyphSlot->bitmap.buffer;
       for (uint y = 0; y < glyphSlot->bitmap.rows; y++) {
           memcpy(img.scanLine(y), src, glyphSlot->bitmap.pitch);
           src += glyphSlot->bitmap.pitch;
       }
   } else if (glyphSlot->bitmap.pixel_mode == FT_PIXEL_MODE_GRAY) {
       img = QImage(size, QImage::Format_Grayscale8);
       uchar *src = glyphSlot->bitmap.buffer;
       for (uint y = 0; y < glyphSlot->bitmap.rows; y++) {
           memcpy(img.scanLine(y), src, glyphSlot->bitmap.pitch);
           src += glyphSlot->bitmap.pitch;
       }
   } else if (glyphSlot->bitmap.pixel_mode == FT_PIXEL_MODE_BGRA) {
       img = QImage(size, QImage::Format_ARGB32_Premultiplied);
       uchar *src = glyphSlot->bitmap.buffer;
       for (uint y = 0; y < glyphSlot->bitmap.rows; y++) {
           if (QSysInfo::ByteOrder == QSysInfo::BigEndian) {
               // TODO: actually test this... We need to go from BGRA to ARGB.
               for (int x = 0; x < static_cast<int>(size.width()); x++) {
                   int p = x*4;
                   img.scanLine(y)[p]   = src[p+3];
                   img.scanLine(y)[p+1] = src[p+2];
                   img.scanLine(y)[p+2] = src[p+1];
                   img.scanLine(y)[p+3] = src[p];
               }
           } else {
               memcpy(img.scanLine(y), src, glyphSlot->bitmap.pitch);
           }
           src += glyphSlot->bitmap.pitch;
       }
   }

   return img;
}

/**
 * @brief addWordToLine
 * Small function used in break lines to quickly add a 'word' to the current line. Returns the last added index.
 */
int addWordToLine(QVector<CharacterResult> &result, QPointF &currentPos, QVector<int> &wordIndices, QRectF &lineBox, qreal &a, qreal &b, QVector<int> &lineIndices, QPointF wordFirstPos, bool ltr) {
    QPointF lineAdvance = currentPos;
    if (lineBox.isEmpty()) {
        lineIndices.clear();
    }
    for (int j : wordIndices) {
        CharacterResult cr = result.at(j);
        cr.cssPosition = currentPos + cr.cssPosition - wordFirstPos;
        lineAdvance = ltr? cr.cssPosition + cr.advance: cr.cssPosition;
        cr.finalPosition = cr.cssPosition;
        if (lineBox.isEmpty() && j == wordIndices.first()) {
            cr.anchored_chunk = true;
        }
        result[j] = cr;
        lineBox |= cr.boundingBox.translated(cr.cssPosition);
    }
    currentPos = lineAdvance;
    a = 0;
    b = 0;
    int lastIndex = wordIndices.last();
    lineIndices += wordIndices;
    wordIndices.clear();
    return lastIndex;
}

/**
 * This offsets the last line by it's ascent, and then returns the last line's descent.
 */
QPointF lineHeightOffset(KoSvgText::WritingMode writingMode, QVector<CharacterResult> &result, QVector<int> lineIndices, QRectF lineBox, QPointF currentPos, KoSvgText::AutoValue lineHeight) {
    QPointF offset;
    if (lineHeight.isAuto) {
        offset = writingMode == KoSvgText::HorizontalTB? QPointF(0, lineBox.height()): writingMode == KoSvgText::VerticalLR ?
                                   QPointF(lineBox.width(), 0): QPointF(-lineBox.width(), 0);
    } else {
        offset = writingMode == KoSvgText::HorizontalTB? QPointF(0, lineHeight.customValue): writingMode == KoSvgText::VerticalLR ?
                                   QPointF(lineHeight.customValue, 0): QPointF(-lineHeight.customValue, 0);
    }
    qreal ascentRatio = writingMode == KoSvgText::HorizontalTB? abs(lineBox.top()-currentPos.y())/lineBox.height() : writingMode == KoSvgText::VerticalLR ?
                                                                    abs(lineBox.left()-currentPos.x())/lineBox.width() : abs(lineBox.right()-currentPos.x())/lineBox.width();
    QPointF ascent = offset * ascentRatio;
    bool returnDescent = lineIndices.isEmpty()? false: lineIndices.first() == 0;
    if (!returnDescent) {
        for (int j: lineIndices) {
            result[j].cssPosition += ascent;
            result[j].finalPosition = result.at(j).cssPosition;
        }
    } else {
        offset = offset * (1 - ascentRatio);
    }
    return offset;
}

void KoSvgTextShape::Private::breakLines(KoSvgTextProperties properties, QMap<int, int> indexToTypographic, QVector<CharacterResult> &result)
{
    KoSvgText::WritingMode writingMode = KoSvgText::WritingMode(
                properties.propertyOrDefault(KoSvgTextProperties::WritingModeId).toInt());
    KoSvgText::Direction direction = KoSvgText::Direction(
                properties.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
    KoSvgText::AutoValue inlineSize = properties.propertyOrDefault(
                KoSvgTextProperties::InlineSizeId).value<KoSvgText::AutoValue>();
    KoSvgText::AutoValue lineHeight = properties.propertyOrDefault(
                KoSvgTextProperties::LineHeightId).value<KoSvgText::AutoValue>();

    bool isHorizontal = writingMode == KoSvgText::HorizontalTB;
    QVector<int> wordIndices; // 'word' in this case meaning characters inbetween softbreaks.
    QPointF wordFirstPos;
    int lastIndex = 0;
    QRectF lineBox; // The line box gets used to determine lineHeight;

    QPointF currentPos;
    QPointF lineOffset;

    qreal a = 0.0; // for determining the advance of the current 'word'.
    qreal b = 0.0;

    QVector<int> lineIndices;

    // The following is because we want to do line-length calculations on the 'visual order' instead of the
    // 'logical' order. For rtl, we'll need to count backwards.
    QList<int> values = indexToTypographic.values();
    std::sort(values.begin(), values.end());
    QListIterator<int> it(values);
    bool ltr = direction == KoSvgText::DirectionLeftToRight;
    ltr? it.toFront(): it.toBack();
    while (ltr? it.hasNext(): it.hasPrevious()) {
        int index = indexToTypographic.key(ltr? it.next(): it.previous());
        if (index < 0) {
            continue;
        }
        CharacterResult charResult = result.at(index);
        if (!charResult.addressable) {
            continue;
        }
        bool breakLine = false;
        bool wordToNextLine = false;

        qreal pos = isHorizontal? charResult.cssPosition.x(): charResult.cssPosition.y();
        qreal advance = isHorizontal? charResult.advance.x(): charResult.advance.y();
        if (wordIndices.isEmpty()) {
            a = qMin(pos, pos+advance);
            b = qMax(pos, pos+advance);
            if (ltr) {
                wordFirstPos = charResult.cssPosition;
            } else {
                wordFirstPos = charResult.cssPosition+charResult.advance;
            }
        } else {
            a = qMin(a, qMin(pos, pos+advance));
            b = qMax(b, qMax(pos, pos+advance));
        }
        QPointF wordAdvance = ltr? isHorizontal? QPoint(b-a, 0): QPoint(0, b-a):
                                   isHorizontal? QPoint(a-b, 0): QPoint(0, a-b);
        wordIndices.append(index);
        bool atEnd = !(ltr? it.hasNext(): it.hasPrevious());


        if (charResult.breakType == HardBreak) {
            breakLine = true;
        } else if (charResult.breakType == SoftBreak || atEnd) {

            qreal lineLength = isHorizontal? (currentPos + wordAdvance).x():
                                             (currentPos + wordAdvance).y();
            if (!inlineSize.isAuto) {
                if (abs(lineLength) > inlineSize.customValue) {
                    breakLine = true;
                    wordToNextLine = true;
                } else {
                    lastIndex = addWordToLine(result, currentPos, wordIndices, lineBox, a, b, lineIndices, wordFirstPos, ltr);
                }
            }
        }

        if (breakLine) {
            if (wordToNextLine) {
                if (lastIndex < result.size()) {
                    CharacterResult cr = result.at(lastIndex);
                    cr.addressable = !cr.collapseIfAtEndOfLine;
                    cr.hidden = cr.collapseIfAtEndOfLine;
                    result[lastIndex] = cr;
                }

                lineOffset += lineHeightOffset(writingMode, result, lineIndices, lineBox, currentPos, lineHeight);
                currentPos = lineOffset;
                lineBox = QRectF();

                lastIndex = addWordToLine(result, currentPos, wordIndices, lineBox, a, b, lineIndices, wordFirstPos, ltr);
            } else {
                lastIndex = addWordToLine(result, currentPos, wordIndices, lineBox, a, b, lineIndices, wordFirstPos, ltr);

                lineOffset += lineHeightOffset(writingMode, result, lineIndices, lineBox, lineOffset, lineHeight);
                currentPos = lineOffset;
                lineBox = QRectF();
            }
        }
        if (atEnd) {
            lineOffset += lineHeightOffset(writingMode, result, lineIndices, lineBox, currentPos, lineHeight);
        }
    }
    qDebug() << "break lines finished";
}

void KoSvgTextShape::Private::applyTextLength(const KoShape *rootShape,
                                              QVector<CharacterResult> &result,
                                              int &currentIndex,
                                              int &resolvedDescendentNodes,
                                              bool isHorizontal)
{
    const KoSvgTextChunkShape *chunkShape = dynamic_cast<const KoSvgTextChunkShape*>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);

    int i = currentIndex;
    int j = i + chunkShape->layoutInterface()->numChars(true);
    int resolvedChildren = 0;

    Q_FOREACH (KoShape *child, chunkShape->shapes()) {
        applyTextLength(child, result, currentIndex, resolvedChildren, isHorizontal);
    }
    // Raqm handles bidi reordering for us, but this algorithm does not anticipate
    // that, so we need to keep track of which typographic item belongs where.
    QMap<int, int> typographicToIndex;
    if (!chunkShape->layoutInterface()->textLength().isAuto) {
        qreal a = 0.0;
        qreal b = 0.0;
        int n = 0;
        for (int k = i; k < j; k++) {
            if (result.at(k).addressable) {
                if (result.at(k).typographic_index > -1) {
                    typographicToIndex.insert(result.at(k).typographic_index, k);
                }
                // if character is linebreak, return;

                qreal pos = result.at(k).finalPosition.x();
                qreal advance = result.at(k).advance.x();
                if (!isHorizontal) {
                    pos = result.at(k).finalPosition.y();
                    advance = result.at(k).advance.y();
                }
                if (k==i) {
                    a = qMin(pos, pos+advance);
                    b = qMax(pos, pos+advance);
                } else {
                    a = qMin(a, qMin(pos, pos+advance));
                    b = qMax(b, qMax(pos, pos+advance));
                }
                if (!result.at(k).textLengthApplied) {
                    n +=1;
                }
            }
        }
        n += resolvedChildren;
        n -= 1;
        qreal delta = chunkShape->layoutInterface()->textLength().customValue - (b-a);
        QPointF d (delta/n, 0);

        if (!isHorizontal) {
            d = QPointF(0, delta/n);
        }
        QPointF shift;
        for (int k : typographicToIndex.keys()) {
            CharacterResult cr = result[typographicToIndex.value(k)];
            if (cr.addressable) {
                cr.finalPosition += shift;
                if (chunkShape->layoutInterface()->lengthAdjust() == KoSvgText::LengthAdjustSpacingAndGlyphs) {
                    QPointF scale(d.x()!=0? (d.x()/cr.advance.x()) + 1: 1.0,
                                  d.y()!=0? (d.y()/cr.advance.y()) + 1: 1.0);
                    QTransform tf = QTransform::fromScale(scale.x(), scale.y());
                    cr.path = tf.map(cr.path);
                    cr.advance = tf.map(cr.advance);
                    cr.boundingBox = tf.mapRect(cr.boundingBox);
                }
                if (!cr.textLengthApplied) {
                    shift += d;
                }
                cr.textLengthApplied = true;
            }
            result[typographicToIndex.value(k)] = cr;
        }
        resolvedDescendentNodes += 1;

        // apply the shift to all consequetive chars as long as they don't start a new chunk.
        for (int k = j; k < result.size(); k++) {
            if (result.at(k).anchored_chunk) {
                break;
            }
            CharacterResult cr = result[k];
            cr.finalPosition += shift;
            result[k] = cr;
        }
    }

    currentIndex = j;
}

/**
 * @brief KoSvgTextShape::Private::computeFontMetrics
 * This function handles computing the baselineOffsets
 */
void KoSvgTextShape::Private::computeFontMetrics(const KoShape *rootShape,
                                                 QMap<int, int> parentBaselineTable, qreal parentFontSize, QPointF superScript, QPointF subScript, QVector<CharacterResult> &result,
                                                 int &currentIndex, qreal res, bool isHorizontal) {
    const KoSvgTextChunkShape *chunkShape = dynamic_cast<const KoSvgTextChunkShape*>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);

    QMap<int, int> baselineTable;
    int i = currentIndex;
    int j = qMin(i + chunkShape->layoutInterface()->numChars(true), result.size());

    KoSvgTextProperties properties = chunkShape->textProperties();


    qreal fontSize = properties.propertyOrDefault(KoSvgTextProperties::FontSizeId).toReal();
    qreal baselineShift = properties.property(KoSvgTextProperties::BaselineShiftValueId).toReal() * fontSize;
    QPointF baselineShiftTotal;
    KoSvgText::BaselineShiftMode baselineShiftMode = KoSvgText::BaselineShiftMode(properties.property(KoSvgTextProperties::BaselineShiftModeId).toInt());
    //TODO: only apply if the current run is horizontal?
    if (baselineShiftMode == KoSvgText::ShiftSuper) {
         if (isHorizontal) {
            baselineShiftTotal = superScript;
         }
    } else if (baselineShiftMode == KoSvgText::ShiftSub) {
        if (isHorizontal) {

            baselineShiftTotal = subScript;
        }
    } else if (baselineShiftMode == KoSvgText::ShiftPercentage) {
        if (isHorizontal) {
            baselineShiftTotal = QPointF(0, baselineShift);
        } else {
            baselineShiftTotal = QPointF(baselineShift, 0);
        }
    }

    QVector<int> lengths;
    const QFont::Style style =
        QFont::Style(properties.propertyOrDefault(KoSvgTextProperties::FontStyleId).toInt());
    std::vector<FT_FaceUP> faces = KoFontRegistery::instance()->facesForCSSValues(properties.property(KoSvgTextProperties::FontFamiliesId).toStringList(),
                                                                            lengths,
                                                                            QString(),
                                                                            fontSize,
                                                                            properties.propertyOrDefault(KoSvgTextProperties::FontWeightId).toInt(),
                                                                            properties.propertyOrDefault(KoSvgTextProperties::FontStretchId).toInt(),
                                                                            style != QFont::StyleNormal);
    KoFontRegistery::instance()->configureFaces(faces, fontSize, res, res, properties.fontAxisSettings());
    hb_font_t_up font = toLibraryResource(hb_ft_font_create_referenced(faces.front().data()));
    qreal freetypePixelsToPt = (1.0/64.0) * float(72./res);

    hb_direction_t dir = HB_DIRECTION_LTR;
    if (!isHorizontal) {
        dir = HB_DIRECTION_TTB;
    }
    hb_script_t script = HB_SCRIPT_UNKNOWN;
    KoSvgText::Baseline dominantBaseline = KoSvgText::Baseline(properties.property(KoSvgTextProperties::DominantBaselineId).toInt());

    hb_position_t baseline = 0;
    if (dominantBaseline == KoSvgText::BaselineResetSize && parentFontSize > 0) {
        baselineTable = parentBaselineTable;
        qreal multiplier = 1.0 / parentFontSize * fontSize;
        for (int key : baselineTable.keys()) {
            baselineTable.insert(key, baselineTable.value(key) * multiplier);
        }
        dominantBaseline = KoSvgText::BaselineAuto;
    } else if(dominantBaseline == KoSvgText::BaselineNoChange) {
        baselineTable = parentBaselineTable;
        dominantBaseline = KoSvgText::BaselineAuto;
    } else {
        if (hb_version_atleast(4, 0, 0)) {
            hb_ot_layout_get_baseline_with_fallback(font.data(), HB_OT_LAYOUT_BASELINE_TAG_ROMAN,
                                                    dir, script, HB_TAG_NONE, &baseline);
            baselineTable.insert(KoSvgText::BaselineAlphabetic, baseline);
            hb_ot_layout_get_baseline_with_fallback(font.data(), HB_OT_LAYOUT_BASELINE_TAG_MATH,
                                                    dir, script, HB_TAG_NONE, &baseline);
            baselineTable.insert(KoSvgText::BaselineMathematical, baseline);
            hb_ot_layout_get_baseline_with_fallback(font.data(), HB_OT_LAYOUT_BASELINE_TAG_HANGING,
                                                    dir, script, HB_TAG_NONE, &baseline);
            baselineTable.insert(KoSvgText::BaselineHanging, baseline);
            hb_ot_layout_get_baseline_with_fallback(font.data(), HB_OT_LAYOUT_BASELINE_TAG_IDEO_FACE_CENTRAL,
                                                    dir, script, HB_TAG_NONE, &baseline);
            baselineTable.insert(KoSvgText::BaselineCentral, baseline);
            hb_ot_layout_get_baseline_with_fallback(font.data(), HB_OT_LAYOUT_BASELINE_TAG_IDEO_EMBOX_BOTTOM_OR_LEFT,
                                                    dir, script, HB_TAG_NONE, &baseline);
            baselineTable.insert(KoSvgText::BaselineIdeographic, baseline);
            if (isHorizontal) {
                hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_X_HEIGHT, &baseline);
                baselineTable.insert(KoSvgText::BaselineMiddle, (baseline - baselineTable.value(KoSvgText::BaselineAlphabetic)) * 0.5);
                hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_HORIZONTAL_ASCENDER, &baseline);
                baselineTable.insert(KoSvgText::BaselineTextTop, baseline);
                hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_HORIZONTAL_DESCENDER, &baseline);
                baselineTable.insert(KoSvgText::BaselineTextBottom, baseline);

            } else {
                baselineTable.insert(KoSvgText::BaselineMiddle, baselineTable.value(KoSvgText::BaselineCentral));
                hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_VERTICAL_ASCENDER, &baseline);
                baselineTable.insert(KoSvgText::BaselineTextTop, baseline);
                hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_VERTICAL_DESCENDER, &baseline);
                baselineTable.insert(KoSvgText::BaselineTextBottom, baseline);
            }
            hb_position_t baseline2 = 0;
            hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_SUPERSCRIPT_EM_X_OFFSET, &baseline);
            hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_SUPERSCRIPT_EM_Y_OFFSET, &baseline2);
            superScript = QPointF(baseline * freetypePixelsToPt, baseline2 * -freetypePixelsToPt);
            hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_SUBSCRIPT_EM_X_OFFSET, &baseline);
            hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_SUBSCRIPT_EM_Y_OFFSET, &baseline2);
            subScript = QPointF(baseline * freetypePixelsToPt, baseline2 * -freetypePixelsToPt);

        } else {
            hb_ot_layout_get_baseline(font.data(), HB_OT_LAYOUT_BASELINE_TAG_ROMAN,
                                      dir, script, HB_TAG_NONE, &baseline);
            baselineTable.insert(KoSvgText::BaselineAlphabetic, baseline);
            hb_ot_layout_get_baseline(font.data(), HB_OT_LAYOUT_BASELINE_TAG_MATH,
                                      dir, script, HB_TAG_NONE, &baseline);
            baselineTable.insert(KoSvgText::BaselineMathematical, baseline);
            hb_ot_layout_get_baseline(font.data(), HB_OT_LAYOUT_BASELINE_TAG_HANGING,
                                      dir, script, HB_TAG_NONE, &baseline);
            baselineTable.insert(KoSvgText::BaselineHanging, baseline);
            hb_ot_layout_get_baseline(font.data(), HB_OT_LAYOUT_BASELINE_TAG_IDEO_FACE_CENTRAL,
                                      dir, script, HB_TAG_NONE, &baseline);
            baselineTable.insert(KoSvgText::BaselineCentral, baseline);
            hb_ot_layout_get_baseline(font.data(), HB_OT_LAYOUT_BASELINE_TAG_IDEO_EMBOX_BOTTOM_OR_LEFT,
                                      dir, script, HB_TAG_NONE, &baseline);
            baselineTable.insert(KoSvgText::BaselineIdeographic, baseline);
            if (isHorizontal) {
                hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_X_HEIGHT, &baseline);
                baselineTable.insert(KoSvgText::BaselineMiddle, (baseline - baselineTable.value(KoSvgText::BaselineAlphabetic)) * 0.5);
                hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_HORIZONTAL_ASCENDER, &baseline);
                baselineTable.insert(KoSvgText::BaselineTextTop, baseline);
                hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_HORIZONTAL_DESCENDER, &baseline);
                baselineTable.insert(KoSvgText::BaselineTextBottom, baseline);
            } else {
                baselineTable.insert(KoSvgText::BaselineMiddle, baselineTable.value(KoSvgText::BaselineCentral));
                hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_VERTICAL_ASCENDER, &baseline);
                baselineTable.insert(KoSvgText::BaselineTextTop, baseline);
                hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_VERTICAL_DESCENDER, &baseline);
                baselineTable.insert(KoSvgText::BaselineTextBottom, baseline);
            }
            hb_position_t baseline2 = 0;
            hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_SUPERSCRIPT_EM_X_OFFSET, &baseline);
            hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_SUPERSCRIPT_EM_Y_OFFSET, &baseline2);
            superScript = QPointF(baseline * freetypePixelsToPt, baseline2 * -freetypePixelsToPt);
            hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_SUBSCRIPT_EM_X_OFFSET, &baseline);
            hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_SUBSCRIPT_EM_Y_OFFSET, &baseline2);
            subScript = QPointF(baseline * freetypePixelsToPt, baseline2 * -freetypePixelsToPt);
        }
    }

    if (hb_version_atleast(4, 0, 0)) {
        qreal width = 0;
        qreal offset = 0;
        hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_UNDERLINE_SIZE, &baseline);
        width = baseline;
        hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_UNDERLINE_OFFSET, &baseline);
        offset = baseline;
        offset *= -freetypePixelsToPt;
        width *= -freetypePixelsToPt;

        chunkShape->layoutInterface()->setTextDecorationFontMetrics(KoSvgText::DecorationUnderline, offset, width);
        chunkShape->layoutInterface()->setTextDecorationFontMetrics(KoSvgText::DecorationOverline, 0, width);

        hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_STRIKEOUT_SIZE, &baseline);
        width = baseline;
        hb_ot_metrics_get_position_with_fallback(font.data(), HB_OT_METRICS_TAG_STRIKEOUT_OFFSET, &baseline);
        width *= -freetypePixelsToPt;
        offset *= -freetypePixelsToPt;
        chunkShape->layoutInterface()->setTextDecorationFontMetrics(KoSvgText::DecorationLineThrough, offset, width);
    } else {
        qreal width = 0;
        qreal offset = 0;
        const int fallbackThickness = faces.front()->underline_thickness * (faces.front()->size->metrics.y_scale / 65535.0);
        hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_UNDERLINE_SIZE, &baseline);
        width = qMax(baseline, fallbackThickness);

        hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_UNDERLINE_OFFSET, &baseline);
        offset = baseline;
        offset *= -freetypePixelsToPt;
        width *= freetypePixelsToPt;

        chunkShape->layoutInterface()->setTextDecorationFontMetrics(KoSvgText::DecorationUnderline, offset, width);
        chunkShape->layoutInterface()->setTextDecorationFontMetrics(KoSvgText::DecorationOverline, 0, width);

        hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_STRIKEOUT_SIZE, &baseline);
        width = qMax(baseline, fallbackThickness);
        hb_ot_metrics_get_position(font.data(), HB_OT_METRICS_TAG_STRIKEOUT_OFFSET, &baseline);
        if (baseline == 0) {
            offset = baselineTable.value(KoSvgText::BaselineCentral);
        }
        width *= freetypePixelsToPt;
        offset *= -freetypePixelsToPt;

        chunkShape->layoutInterface()->setTextDecorationFontMetrics(KoSvgText::DecorationLineThrough, offset, width);
    }


    Q_FOREACH (KoShape *child, chunkShape->shapes()) {
        computeFontMetrics(child, baselineTable, fontSize, superScript, subScript, result, currentIndex, res, isHorizontal);
    }

    KoSvgText::Baseline baselineAdjust = KoSvgText::Baseline(properties.property(KoSvgTextProperties::AlignmentBaselineId).toInt());

    if (baselineAdjust == KoSvgText::BaselineDominant) {
        baselineAdjust = dominantBaseline;
    }
    if (baselineAdjust == KoSvgText::BaselineAuto || baselineAdjust == KoSvgText::BaselineUseScript) {
        // UseScript got deprecated in CSS-Inline-3.
        if (isHorizontal) {
            baselineAdjust = KoSvgText::BaselineAlphabetic;
        } else {
            baselineAdjust = KoSvgText::BaselineMiddle;
        }
    }

    int offset = parentBaselineTable.value(baselineAdjust, 0) - baselineTable.value(baselineAdjust, 0);
    QPointF shift;
    if (isHorizontal) {
        shift = QPointF(0, offset * -freetypePixelsToPt);
    } else {
        shift = QPointF(offset * freetypePixelsToPt, 0);
    }
    shift += baselineShiftTotal;

    for (int k = i; k < j; k++) {
        CharacterResult cr = result[k];
        cr.cssPosition += shift;
        result[k] = cr;
    }

    currentIndex = j;
}

/**
 * @brief KoSvgTextShape::Private::computeTextDecorations
 * Text decorations need to be computed before textPath is applied.
 * This function goes down the tree and computes textDecorations as necessary,
 * bends them to the textPath, strokes them, and then adds them to the node in question.
 */
void KoSvgTextShape::Private::computeTextDecorations(const KoShape *rootShape, QVector<CharacterResult> result, KoPathShape *textPath, qreal textPathoffset, bool side, int &currentIndex, bool isHorizontal) {
    const KoSvgTextChunkShape *chunkShape = dynamic_cast<const KoSvgTextChunkShape*>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);

    int i = currentIndex;
    int j = qMin(i + chunkShape->layoutInterface()->numChars(true), result.size());

    KoPathShape *currentTextPath = textPath? textPath : dynamic_cast<KoPathShape*>(chunkShape->layoutInterface()->textPath());
    qreal currentTextPathOffset = textPathoffset;
    bool textPathSide = side;
    if(chunkShape->layoutInterface()->textPath()) {
        textPathSide = chunkShape->layoutInterface()->textOnPathInfo().side == KoSvgText::TextPathSideRight;
        if (chunkShape->layoutInterface()->textOnPathInfo().startOffsetIsPercentage) {
            currentTextPathOffset = currentTextPath->outline().length() * (0.01 * chunkShape->layoutInterface()->textOnPathInfo().startOffset);
        } else {
            currentTextPathOffset = chunkShape->layoutInterface()->textOnPathInfo().startOffset;
        }
    }

    Q_FOREACH (KoShape *child, chunkShape->shapes()) {
        computeTextDecorations(child, result, currentTextPath, currentTextPathOffset, textPathSide, currentIndex, isHorizontal);
    }

    KoSvgText::TextDecorations decor = chunkShape->textProperties().propertyOrDefault(
                KoSvgTextProperties::TextDecorationLineId).value<KoSvgText::TextDecorations>();
    if (decor != KoSvgText::DecorationNone) {
        using namespace KoSvgText;
        KoSvgTextProperties properties = chunkShape->textProperties();
        TextDecorationStyle style =
                TextDecorationStyle(properties.propertyOrDefault(KoSvgTextProperties::TextDecorationStyleId).toInt());
        QPainterPath underline;
        QPointF underlineOffset;
        QPainterPath overline;
        QPointF overlineOffset;
        QPainterPath linethrough;
        QPointF lineThroughOffset;
        if (isHorizontal) {
            underlineOffset = QPointF(0, chunkShape->layoutInterface()->getTextDecorationOffset(KoSvgText::DecorationUnderline));
            lineThroughOffset = QPointF(0, chunkShape->layoutInterface()->getTextDecorationOffset(KoSvgText::DecorationLineThrough));
        } else {
            underlineOffset = QPointF(chunkShape->layoutInterface()->getTextDecorationOffset(KoSvgText::DecorationUnderline), 0);
            lineThroughOffset = QPointF(chunkShape->layoutInterface()->getTextDecorationOffset(KoSvgText::DecorationLineThrough), 0);
        }
        QPainterPathStroker stroker;
        stroker.setWidth(qMax(0.1, chunkShape->layoutInterface()->getTextDecorationWidth(KoSvgText::DecorationUnderline)));
        stroker.setCapStyle(Qt::FlatCap);
        if (style == KoSvgText::Dotted) {
            QPen pen;
            pen.setStyle(Qt::DotLine);
            stroker.setDashPattern(pen.dashPattern());
        } else if (style == KoSvgText::Dashed) {
            QPen pen;
            pen.setStyle(Qt::DashLine);
            stroker.setDashPattern(pen.dashPattern());
        }
        qreal top = 0;
        qreal bottom = 0;
        QVector<QRectF> decorationRects;
        QVector<QPointF> firstPos;
        QRectF currentRect;
        for (int k = i; k < j; k++) {
            CharacterResult charResult = result.at(k);
            if (currentTextPath) {
                characterResultOnPath(charResult, currentTextPath->outline().length(), currentTextPathOffset, isHorizontal, currentTextPath->isClosedSubpath(0));
            }
            if (charResult.hidden || !charResult.addressable) {
                continue;
            }
            if (isHorizontal) {
                if (charResult.path.isEmpty()) {
                    top = qMin(top, charResult.boundingBox.top());
                    bottom = qMax(bottom, charResult.boundingBox.bottom());
                } else {
                    top = qMin(top, charResult.path.boundingRect().top());
                    bottom = qMax(bottom, charResult.path.boundingRect().bottom());
                }
            } else {
                if (charResult.path.isEmpty()) {
                    top = qMax(top, charResult.boundingBox.right());
                    bottom = qMin(bottom, charResult.boundingBox.left());
                } else {
                    top = qMax(top, charResult.path.boundingRect().right());
                    bottom = qMin(bottom, charResult.path.boundingRect().left());
                }
            }
            if (firstPos.isEmpty()) {
                firstPos.append(charResult.finalPosition);
            }
            if (charResult.anchored_chunk) {
                decorationRects.append(currentRect);
                currentRect = QRectF();
                firstPos.append(charResult.finalPosition);
            }
            if (charResult.path.isEmpty()) {
                currentRect |= charResult.boundingBox.translated(charResult.finalPosition);
            } else {
                currentRect |= charResult.path.boundingRect().translated(charResult.finalPosition);
            }
        }
        decorationRects.append(currentRect);
        if (isHorizontal) {
            overlineOffset = QPointF(0, top);
            KoSvgText::TextDecorationUnderlinePosition underlinePosH =
                    KoSvgText::TextDecorationUnderlinePosition(properties.propertyOrDefault(
                                                                   KoSvgTextProperties::TextDecorationPositionHorizontalId).toInt());
            if (underlinePosH == KoSvgText::UnderlineUnder) {
                underlineOffset = QPointF(0, bottom);
            }
            lineThroughOffset = (underlineOffset + overlineOffset) * 0.5;
        } else {
            KoSvgText::TextDecorationUnderlinePosition underlinePosV =
                    KoSvgText::TextDecorationUnderlinePosition(properties.propertyOrDefault(
                                                                   KoSvgTextProperties::TextDecorationPositionVerticalId).toInt());
            if (underlinePosV == KoSvgText::UnderlineLeft) {
                overlineOffset = QPointF(top, 0);
                underlineOffset = QPointF(bottom, 0);
            } else {
                overlineOffset = QPointF(bottom, 0);
                underlineOffset = QPointF(top, 0);
            }
            lineThroughOffset = (underlineOffset + overlineOffset) * 0.5;

        }
        qreal wavyOffset = 0;
        for (int i=0; i<decorationRects.size(); i++) {
            QRectF rect = decorationRects.at(i);
            QPainterPath p;
            QPointF pathWidth;
            p.moveTo(QPointF());
            if (isHorizontal) {
                int total = floor(rect.width()/(stroker.width()*2));
                qreal segment = qreal(rect.width()/total);
                for (int i=0; i<total; i++) {
                    p.lineTo(p.currentPosition() + QPointF(segment, 0));
                }
            } else {
                int total = floor(rect.height()/(stroker.width()*2));
                qreal segment = qreal(rect.height()/total);
                for (int i=0; i<total; i++) {
                    p.lineTo(p.currentPosition() + QPointF(0, segment));
                }
            }
            if (style == KoSvgText::Double) {
                qreal linewidthOffset = stroker.width() * 1.5;
                if (isHorizontal) {
                    p.addPath(p.translated(0, linewidthOffset));
                    pathWidth =  QPointF(0, stroker.width() * 1.5);
                } else {
                    p.addPath(p.translated(linewidthOffset, 0));
                    pathWidth = QPointF(stroker.width() * 1.5, 0);
                }
            } else if (style == KoSvgText::Wavy) {

                qreal width = isHorizontal? rect.width(): rect.height();
                qreal height = stroker.width() * 2;

                qreal offset = fmod(wavyOffset, height);
                bool down = int(wavyOffset/height) % 2;
                p = QPainterPath();
                if (isHorizontal) {
                    if (down) {
                        p.moveTo(0, offset);
                        p.lineTo(height-offset, height);
                    } else {
                        p.moveTo(0, height-offset);
                        p.lineTo(height-offset, 0);
                    }
                    qreal restWidth = width - (height-offset);
                    down = !down;
                    for (int i = 0; i < qFloor(restWidth/height); i++) {
                        if (down) {
                            p.lineTo(p.currentPosition().x() + height, height);
                        } else {
                            p.lineTo(p.currentPosition().x() + height, 0);
                        }
                        down = !down;
                    }
                    offset = fmod(restWidth, height);
                    if (down) {
                        p.lineTo(width, offset);
                    } else {
                        p.lineTo(width, height-offset);
                    }
                    pathWidth = QPointF(0, stroker.width() * 2);
                } else {
                    if (down) {
                        p.moveTo(offset, 0);
                        p.lineTo(height, height-offset);
                    } else {
                        p.moveTo(height-offset, 0);
                        p.lineTo(0, height-offset);
                    }
                    qreal restWidth = width - (height-offset);
                    down = !down;
                    for (int i = 0; i < qFloor(restWidth/height); i++) {
                        if (down) {
                            p.lineTo(height, p.currentPosition().y() + height);
                        } else {
                            p.lineTo(0, p.currentPosition().y() + height);
                        }
                        down = !down;
                    }
                    offset = fmod(restWidth, height);
                    if (down) {
                        p.lineTo(offset, width);
                    } else {
                        p.lineTo(height-offset, width);
                    }
                    pathWidth = QPointF((stroker.width() * 2), 0);
                }

                wavyOffset += width;
            }
            p.translate(firstPos.at(i).x(), firstPos.at(i).y());

            if (decor.testFlag(KoSvgText::DecorationUnderline)) {
                if (currentTextPath) {
                    QPainterPath path = currentTextPath->outline();
                    path = currentTextPath->transformation().map(path);
                    if (textPathSide) {path = path.toReversed();}
                    underline.addPath(stretchGlyphOnPath(p.translated(underlineOffset), path, isHorizontal,
                                                         currentTextPathOffset, currentTextPath->isClosedSubpath(0)));
                } else {
                    underline.addPath(p.translated(underlineOffset));
                }
            }
            if (decor.testFlag(KoSvgText::DecorationOverline)) {
                if (currentTextPath) {
                    QPainterPath path = currentTextPath->outline();
                    path = currentTextPath->transformation().map(path);
                    if (textPathSide) {path = path.toReversed();}
                    overline.addPath(stretchGlyphOnPath(p.translated(overlineOffset-pathWidth), path, isHorizontal,
                                                        currentTextPathOffset, currentTextPath->isClosedSubpath(0)));
                } else {
                    overline.addPath(p.translated(overlineOffset-pathWidth));
                }
            }
            if (decor.testFlag(KoSvgText::DecorationLineThrough)) {
                if (currentTextPath) {
                    QPainterPath path = currentTextPath->outline();
                    path = currentTextPath->transformation().map(path);
                    if (textPathSide) {path = path.toReversed();}
                    linethrough.addPath(stretchGlyphOnPath(p.translated(lineThroughOffset - (pathWidth*0.5)), path,
                                                           isHorizontal, currentTextPathOffset, currentTextPath->isClosedSubpath(0)));
                } else {
                    linethrough.addPath(p.translated(lineThroughOffset - (pathWidth*0.5)));
                }
            }
        }
        chunkShape->layoutInterface()->clearTextDecorations();
        if (!underline.isEmpty()) {
            stroker.setWidth(qMax(0.1, chunkShape->layoutInterface()->getTextDecorationWidth(KoSvgText::DecorationUnderline)));
            underline = stroker.createStroke(underline).simplified();
            chunkShape->layoutInterface()->addTextDecoration(KoSvgText::DecorationUnderline, underline.simplified());
        }
        if (!overline.isEmpty()) {
            stroker.setWidth(qMax(0.1, chunkShape->layoutInterface()->getTextDecorationWidth(KoSvgText::DecorationOverline)));
            overline = stroker.createStroke(overline).simplified();
            chunkShape->layoutInterface()->addTextDecoration(KoSvgText::DecorationOverline, overline.simplified());
        }
        if (!linethrough.isEmpty()) {
            stroker.setWidth(qMax(0.1, chunkShape->layoutInterface()->getTextDecorationWidth(KoSvgText::DecorationLineThrough)));
            linethrough = stroker.createStroke(linethrough).simplified();
            chunkShape->layoutInterface()->addTextDecoration(KoSvgText::DecorationLineThrough, linethrough.simplified());
        }
    }
    currentIndex = j;
}

void KoSvgTextShape::Private::applyAnchoring(QVector<CharacterResult> &result, bool isHorizontal, bool inlineSize)
{
    QMap<int, int> typographicToIndex;
    int i = 0;
    int start = 0;
    while (start < result.size()) {
        int lowestTypographicalIndex = result.size();
        qreal a = 0;
        qreal b = 0;
        for (i = start; i < result.size(); i++) {
            if (!result.at(i).addressable) {
                continue;
            }
            if (result.at(i).anchored_chunk && i > start) {
                break;
            }
            if (result.at(i).typographic_index > -1) {
                typographicToIndex.insert(result.at(i).typographic_index, i);
                lowestTypographicalIndex = qMin(lowestTypographicalIndex, result.at(i).typographic_index);
            }
            qreal pos = isHorizontal? result.at(i).finalPosition.x(): result.at(i).finalPosition.y();
            qreal advance = isHorizontal? result.at(i).advance.x(): result.at(i).advance.y();
            if (result.at(i).anchored_chunk) {
                a = qMin(pos, pos+advance);
                b = qMax(pos, pos+advance);
            } else {
                a = qMin(a, qMin(pos, pos+advance));
                b = qMax(b, qMax(pos, pos+advance));
            }
        }
        qreal shift = 0;
        int typo = typographicToIndex.value(lowestTypographicalIndex);
        if (!inlineSize) {
            shift = isHorizontal? result.at(typo).finalPosition.x(): result.at(typo).finalPosition.y();
        }

        bool rtl = result.at(start).direction == KoSvgText::DirectionRightToLeft;
        if ((result.at(start).anchor == KoSvgText::AnchorStart && !rtl)
         || (result.at(start).anchor == KoSvgText::AnchorEnd    && rtl)) {

            shift -= a;

        } else if ((result.at(start).anchor == KoSvgText::AnchorEnd  && !rtl)
                || (result.at(start).anchor == KoSvgText::AnchorStart && rtl)) {

            shift -= b;

        } else {
            shift -= ((a + b) * 0.5);

        }
        QPointF shiftP(shift, 0);
        if (!isHorizontal) {
            shiftP = QPointF(0, shift);
        }

        for (int j = start; j < i; j++) {
            CharacterResult cr = result[j];
            cr.finalPosition += shiftP;
            result[j] = cr;
        }
        start = i;
    }
}

qreal KoSvgTextShape::Private::characterResultOnPath(CharacterResult &cr, qreal length, qreal offset, bool isHorizontal, bool isClosed) {
    bool rtl = (cr.direction == KoSvgText::DirectionRightToLeft);
    qreal mid = cr.finalPosition.x() + (cr.advance.x() * 0.5) + offset;
    if (!isHorizontal) {
        mid = cr.finalPosition.y() + (cr.advance.y() * 0.5) + offset;
    }
    if (isClosed) {

        if ((cr.anchor == KoSvgText::AnchorStart && !rtl)
         || (cr.anchor == KoSvgText::AnchorEnd    && rtl)) {
            if (mid - offset < 0 || mid - offset > length) {
                cr.hidden = true;
            }
        } else if ((cr.anchor == KoSvgText::AnchorEnd  && !rtl)
                || (cr.anchor == KoSvgText::AnchorStart && rtl)) {
            if (mid - offset < -length || mid - offset > 0) {
                cr.hidden = true;
            }
        } else {
            if (mid - offset < -(length*0.5) || mid - offset > (length*0.5)) {
                cr.hidden = true;
            }
        }
        if (mid < 0) { mid += length;}
        mid = fmod(mid, length);
    } else {
        if(mid < 0 || mid > length){
            cr.hidden = true;
        }
    }
    return mid;
}

QPainterPath KoSvgTextShape::Private::stretchGlyphOnPath(QPainterPath glyph, QPainterPath path, bool isHorizontal, qreal offset, bool isClosed) {
    QPainterPath p = glyph;
    for (int i = 0; i < glyph.elementCount(); i++) {
        qreal mid = isHorizontal? glyph.elementAt(i).x + offset: glyph.elementAt(i).y + offset;
        qreal midUnbound = mid;
        if (isClosed) {
            if (mid < 0) { mid += path.length();}
            mid = fmod(mid, qreal(path.length()));
            midUnbound = mid;
        } else {
            mid = qBound(0.0, mid, qreal(path.length()));
        }
        qreal percent = path.percentAtLength(mid);
        QPointF pos = path.pointAtPercent(percent);
        qreal tAngle = path.angleAtPercent(percent);
        if (tAngle>180) {

            tAngle = 0 - (360 - tAngle);
        }
        QPointF vectorT(qCos(qDegreesToRadians(tAngle)), -qSin(qDegreesToRadians(tAngle)));
        QPointF finalPos = pos;
        if (isHorizontal) {
            QPointF vectorN(-vectorT.y(), vectorT.x());
            qreal o = mid - (midUnbound);
            finalPos = pos - (o*vectorT) + (glyph.elementAt(i).y*vectorN);
        } else {
            QPointF vectorN(vectorT.y(), -vectorT.x());
            qreal o = mid - (midUnbound);
            finalPos = pos - (o*vectorT) + (glyph.elementAt(i).x*vectorN);
        }
        p.setElementPositionAt(i, finalPos.x(), finalPos.y());
    }
    return p;
}

void KoSvgTextShape::Private::applyTextPath(const KoShape *rootShape,
                                            QVector<CharacterResult> &result,
                                            bool isHorizontal)
{
    // Unlike all the other applying functions, this one only iterrates over the top-level.
    // SVG is not designed to have nested textPaths.
    // Source: https://github.com/w3c/svgwg/issues/580
    const KoSvgTextChunkShape *chunkShape = dynamic_cast<const KoSvgTextChunkShape*>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);
    bool inPath = false;
    bool afterPath = false;
    int currentIndex = 0;
    QPointF pathEnd;
    Q_FOREACH (KoShape *child, chunkShape->shapes()) {
        const KoSvgTextChunkShape *textPathChunk = dynamic_cast<const KoSvgTextChunkShape*>(child);
        KIS_SAFE_ASSERT_RECOVER_RETURN(textPathChunk);
        int endIndex = currentIndex + textPathChunk->layoutInterface()->numChars(true);

        KoPathShape *shape = dynamic_cast<KoPathShape*>(textPathChunk->layoutInterface()->textPath());
        if (shape) {
            QPainterPath path = shape->outline();
            path = shape->transformation().map(path);
            inPath = true;
            if(textPathChunk->layoutInterface()->textOnPathInfo().side == KoSvgText::TextPathSideRight) {
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

            for (int i = currentIndex; i < endIndex; i++) {
                CharacterResult cr = result[i];


                if (cr.middle == false) {
                    qreal mid = characterResultOnPath(cr, length, offset, isHorizontal, isClosed);
                    if (!cr.hidden) {
                        if (stretch && !cr.path.isEmpty()) {
                            QTransform tf = QTransform::fromTranslate(cr.finalPosition.x(), cr.finalPosition.y());
                            tf.rotateRadians(cr.rotate);
                            QPainterPath glyph = stretchGlyphOnPath(tf.map(cr.path), path, isHorizontal, offset, isClosed);
                            cr.path =glyph;
                        }
                        qreal percent = path.percentAtLength(mid);
                        QPointF pos = path.pointAtPercent(percent);
                        qreal tAngle = path.angleAtPercent(percent);
                        if (tAngle>180) {
                            
                            tAngle = 0 - (360 - tAngle);
                        }
                        QPointF vectorT(qCos(qDegreesToRadians(tAngle)), -qSin(qDegreesToRadians(tAngle)));
                        if (isHorizontal) {
                            cr.rotate -= qDegreesToRadians(tAngle);
                            QPointF vectorN(-vectorT.y(), vectorT.x());
                            qreal o = (cr.advance.x()*0.5);
                            cr.finalPosition = pos - (o*vectorT) + (cr.finalPosition.y()*vectorN);
                        } else {
                            cr.rotate -= qDegreesToRadians(tAngle+90);
                            QPointF vectorN(vectorT.y(), -vectorT.x());
                            qreal o = (cr.advance.y()*0.5);
                            cr.finalPosition = pos - (o*vectorT) + (cr.finalPosition.x()*vectorN);
                        }
                        if (stretch && !cr.path.isEmpty()) {
                            QTransform tf = QTransform::fromTranslate(cr.finalPosition.x(), cr.finalPosition.y());
                            tf.rotateRadians(cr.rotate);
                            cr.path = tf.inverted().map(cr.path);
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

void KoSvgTextShape::Private::paintPaths(QPainter &painter, KoShapePaintingContext &paintContext,
                                         QPainterPath outlineRect, const KoShape *rootShape,
                                         QVector<CharacterResult> &result, QPainterPath &chunk,
                                         int &currentIndex)
{
    const KoSvgTextChunkShape *chunkShape = dynamic_cast<const KoSvgTextChunkShape*>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);
    QMap<KoSvgText::TextDecoration, QPainterPath> textDecorations = chunkShape->layoutInterface()->textDecorations();
    QColor textDecorationColor = chunkShape->textProperties().propertyOrDefault(
                KoSvgTextProperties::TextDecorationColorId).value<QColor>();

    if (textDecorations.contains(KoSvgText::DecorationUnderline)) {
        if (chunkShape->background() && !textDecorationColor.isValid()) {
            chunkShape->background()->paint(painter, paintContext, textDecorations.value(KoSvgText::DecorationUnderline));
        } else if(textDecorationColor.isValid()) {
            painter.fillPath(textDecorations.value(KoSvgText::DecorationUnderline), textDecorationColor);
        }
        if (chunkShape->stroke()) {
            QScopedPointer<KoShape> shape(KoPathShape::createShapeFromPainterPath(textDecorations.value(KoSvgText::DecorationUnderline)));
            chunkShape->stroke()->paint(shape.data(), painter);
        }
    }
    if (textDecorations.contains(KoSvgText::DecorationOverline)) {
        if (chunkShape->background() && !textDecorationColor.isValid()) {
            chunkShape->background()->paint(painter, paintContext, textDecorations.value(KoSvgText::DecorationOverline));
        } else if(textDecorationColor.isValid()) {
            painter.fillPath(textDecorations.value(KoSvgText::DecorationOverline), textDecorationColor);
        }
        if (chunkShape->stroke()) {
            QScopedPointer<KoShape> shape(KoPathShape::createShapeFromPainterPath(textDecorations.value(KoSvgText::DecorationOverline)));
            chunkShape->stroke()->paint(shape.data(), painter);
        }
    }

    if (chunkShape->isTextNode()) {
        QTransform tf;
        int j = currentIndex + chunkShape->layoutInterface()->numChars(true);
        KoClipMaskPainter fillPainter(&painter,
                                      painter.transform().mapRect(outlineRect.boundingRect()));
        if (chunkShape->background()) {
            chunkShape->background()->paint(*fillPainter.shapePainter(), paintContext, outlineRect);
            fillPainter.maskPainter()->fillPath(outlineRect, Qt::black);
            if (textRendering != OptimizeSpeed) {
                fillPainter.maskPainter()->setRenderHint(QPainter::Antialiasing, true);
                fillPainter.maskPainter()->setRenderHint(QPainter::SmoothPixmapTransform, true);
            } else {
                fillPainter.maskPainter()->setRenderHint(QPainter::Antialiasing, false);
                fillPainter.maskPainter()->setRenderHint(QPainter::SmoothPixmapTransform, false);
            }
        }
        QPainterPath textDecorationsRest;
        textDecorationsRest.setFillRule(Qt::WindingFill);

        for (int i = currentIndex; i< j; i++) {
            if (result.at(i).addressable && result.at(i).hidden == false) {
                tf.reset();
                tf.translate(result.at(i).finalPosition.x(), result.at(i).finalPosition.y());
                tf.rotateRadians(result.at(i).rotate);
                /* Debug
                painter.save();
                painter.setBrush(Qt::transparent);
                QPen pen(result.at(i).anchored_chunk? Qt::magenta: Qt::cyan);
                pen.setWidthF(0.1);
                painter.setPen(pen);
                painter.drawPolygon(tf.map(result.at(i).boundingBox));
                painter.setPen(Qt::blue);
                if (result.at(i).breakType != NoBreak){
                    painter.drawPoint(tf.mapRect(result.at(i).boundingBox).center());
                }
                painter.setPen(Qt::red);
                painter.drawPoint(result.at(i).finalPosition);
                painter.restore();
                */
                /**
                 * There's an annoying problem here that officially speaking
                 * the chunks need to be unified into one single path before
                 * drawing, so there's no weirdness with the stroke, but
                 * QPainterPath's union function will frequently lead to
                 * reduced quality of the paths because of 'numerical
                 * instability'.
                 */

                QPainterPath p = tf.map(result.at(i).path);
                //if (chunk.intersects(p)) {
                //    chunk |= tf.map(result.at(i).path);
                //} else {
                if (result.at(i).colorLayers.size()) {
                    for (int c = 0; c < result.at(i).colorLayers.size(); c++) {
                        QBrush color = result.at(i).colorLayerColors.at(c);
                        bool replace = result.at(i).replaceWithForeGroundColor.at(c);
                        // In theory we can use the pattern or gradient as well for ColorV0 fonts, but ColorV1
                        // fonts can have gradients, so I am hesitant.
                        KoColorBackground *b = dynamic_cast<KoColorBackground*>(chunkShape->background().data());
                        if (b && replace) {
                            color = b->brush();
                        }
                        painter.fillPath(tf.map(result.at(i).colorLayers.at(c)), color);
                    }
                } else {
                    chunk.addPath(p);
                }
                //}
                if (p.isEmpty() && !result.at(i).image.isNull()) {
                    if (result.at(i).image.isGrayscale() || result.at(i).image.format() == QImage::Format_Mono) {
                        fillPainter.maskPainter()->save();
                        fillPainter.maskPainter()->translate(result.at(i).finalPosition.x(), result.at(i).finalPosition.y());
                        fillPainter.maskPainter()->rotate(qRadiansToDegrees(result.at(i).rotate));
                        fillPainter.maskPainter()->setCompositionMode(QPainter::CompositionMode_Plus);
                        fillPainter.maskPainter()->drawImage(result.at(i).boundingBox, result.at(i).image);
                        fillPainter.maskPainter()->restore();
                    } else {
                        painter.save();
                        painter.translate(result.at(i).finalPosition.x(), result.at(i).finalPosition.y());
                        painter.rotate(qRadiansToDegrees(result.at(i).rotate));
                        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
                        painter.drawImage(result.at(i).boundingBox, result.at(i).image);
                        painter.restore();
                    }
                }
            }
        }
        if (chunkShape->background()) {
            chunk.setFillRule(Qt::WindingFill);
            fillPainter.maskPainter()->fillPath(chunk, Qt::white);
        }
        if (!textDecorationsRest.isEmpty()) {
            fillPainter.maskPainter()->fillPath(textDecorationsRest.simplified(), Qt::white);
        }
        fillPainter.renderOnGlobalPainter();
        KoShapeStrokeSP maskStroke;
        if (chunkShape->stroke()) {

            KoShapeStrokeSP stroke = qSharedPointerDynamicCast<KoShapeStroke>(chunkShape->stroke());

            if (stroke) {
                if (stroke->lineBrush().gradient()) {
                    KoClipMaskPainter strokePainter(&painter,
                                                    painter.transform().mapRect(outlineRect.boundingRect()));
                    strokePainter.shapePainter()->fillRect(outlineRect.boundingRect(), stroke->lineBrush());
                    maskStroke = KoShapeStrokeSP(new KoShapeStroke(*stroke.data()));
                    maskStroke->setColor(Qt::white);
                    maskStroke->setLineBrush(Qt::white);
                    strokePainter.maskPainter()->fillPath(outlineRect, Qt::black);
                    if (textRendering != OptimizeSpeed) {
                        strokePainter.maskPainter()->setRenderHint(QPainter::Antialiasing, true);
                    } else {
                        strokePainter.maskPainter()->setRenderHint(QPainter::Antialiasing, false); 
                    }
                    {
                        QScopedPointer<KoShape> shape(KoPathShape::createShapeFromPainterPath(chunk));
                        maskStroke->paint(shape.data(), *strokePainter.maskPainter());
                    }
                    if (!textDecorationsRest.isEmpty()) {
                        QScopedPointer<KoShape> shape(KoPathShape::createShapeFromPainterPath(textDecorationsRest));
                        maskStroke->paint(shape.data(), *strokePainter.maskPainter());
                    }
                    strokePainter.renderOnGlobalPainter();
                } else {
                    {
                        QScopedPointer<KoShape> shape(KoPathShape::createShapeFromPainterPath(chunk));
                        stroke->paint(shape.data(), painter);
                    }
                    if (!textDecorationsRest.isEmpty()) {
                        QScopedPointer<KoShape> shape(KoPathShape::createShapeFromPainterPath(textDecorationsRest));
                        stroke->paint(shape.data(), painter);
                    }
                }
            }

        }
        chunk = QPainterPath();
        currentIndex = j;

    } else {
        Q_FOREACH (KoShape *child, chunkShape->shapes()) {
            paintPaths(painter, paintContext, outlineRect, child, result, chunk, currentIndex);
        }
    }
    if (textDecorations.contains(KoSvgText::DecorationLineThrough)) {
        if (chunkShape->background() && !textDecorationColor.isValid()) {
            chunkShape->background()->paint(painter, paintContext, textDecorations.value(KoSvgText::DecorationLineThrough));
        } else if(textDecorationColor.isValid()) {
            painter.fillPath(textDecorations.value(KoSvgText::DecorationLineThrough), textDecorationColor);
        }
        if (chunkShape->stroke()) {
            QScopedPointer<KoShape> shape(KoPathShape::createShapeFromPainterPath(textDecorations.value(KoSvgText::DecorationLineThrough)));
            chunkShape->stroke()->paint(shape.data(), painter);
        }
    }
}

bool KoSvgTextShape::isRootTextNode() const
{
    return true;
}

KoSvgTextShapeFactory::KoSvgTextShapeFactory()
    : KoShapeFactoryBase(KoSvgTextShape_SHAPEID, i18nc("Text label in SVG Text Tool", "Text"))
{
    setToolTip(i18n("SVG Text Shape"));
    setIconName(koIconNameCStr("x-shape-text"));
    setLoadingPriority(5);
    setXmlElementNames(KoXmlNS::svg, QStringList("text"));

    KoShapeTemplate t;
    t.name = i18n("SVG Text");
    t.iconName = koIconName("x-shape-text");
    t.toolTip = i18n("SVG Text Shape");
    addTemplate(t);
}

KoShape *KoSvgTextShapeFactory::createDefaultShape(KoDocumentResourceManager *documentResources) const
{
    debugFlake << "Create default svg text shape";

    KoSvgTextShape *shape = new KoSvgTextShape();
    shape->setShapeId(KoSvgTextShape_SHAPEID);

    KoSvgTextShapeMarkupConverter converter(shape);
    converter.convertFromSvg(i18nc("Default text for the text shape", "<text>Placeholder Text</text>"),
                             "<defs/>",
                             QRectF(0, 0, 200, 60),
                             documentResources->documentResolution());

    debugFlake << converter.errors() << converter.warnings();

    return shape;
}

KoShape *KoSvgTextShapeFactory::createShape(const KoProperties *params, KoDocumentResourceManager *documentResources) const
{
    KoSvgTextShape *shape = new KoSvgTextShape();
    shape->setShapeId(KoSvgTextShape_SHAPEID);

    QString svgText = params->stringProperty("svgText", i18nc("Default text for the text shape",
                                                              "<text>Placeholder Text</text>"));
    QString defs = params->stringProperty("defs" , "<defs/>");
    QRectF shapeRect = QRectF(0, 0, 200, 60);
    QVariant rect = params->property("shapeRect");

    if (rect.type()==QVariant::RectF) {
        shapeRect = rect.toRectF();
    }

    KoSvgTextShapeMarkupConverter converter(shape);
    converter.convertFromSvg(svgText,
                             defs,
                             shapeRect,
                             documentResources->documentResolution());

    shape->setPosition(shapeRect.topLeft());

    return shape;
}

bool KoSvgTextShapeFactory::supports(const QDomElement &/*e*/, KoShapeLoadingContext &/*context*/) const
{
    return false;
}
