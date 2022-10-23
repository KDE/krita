/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KoSvgTextShape.h"

#include <QTextLayout>

#include <fontconfig/fontconfig.h>
#include <raqm.h>
#include FT_COLOR_H
#include <graphemebreak.h>
#include <hb-ft.h>
#include <hb-ot.h>
#include <hb.h>
#include <linebreak.h>

#include <klocalizedstring.h>

#include "KoSvgText.h"
#include "KoSvgTextProperties.h"
#include <KoDocumentResourceManager.h>
#include <KoShapeContainer_p.h>
#include <KoShapeController.h>
#include <text/KoCssTextUtils.h>
#include <text/KoFontRegistery.h>
#include <text/KoSvgTextChunkShape_p.h>
#include <text/KoSvgTextShapeMarkupConverter.h>

#include "kis_debug.h"

#include <KoClipMaskPainter.h>
#include <KoColorBackground.h>
#include <KoIcon.h>
#include <KoPathShape.h>
#include <KoProperties.h>
#include <KoShapeLoadingContext.h>
#include <KoXmlNS.h>

#include <SvgLoadingContext.h>
#include <SvgGraphicContext.h>
#include <SvgUtil.h>

#include <QApplication>
#include <QFileInfo>
#include <QLineF>
#include <QPainter>
#include <QPainterPath>
#include <QThread>
#include <QtMath>
#include <memory>
#include <vector>

#include <boost/optional.hpp>

#include <text/KoSvgTextChunkShapeLayoutInterface.h>

#include <FlakeDebug.h>

#include <QSharedData>

enum BreakType { NoBreak, SoftBreak, HardBreak };

enum LineEdgeBehaviour {
    NoChange, ///< Do nothing special.
    Collapse, ///< Collapse if first or last in line.
    HangBehaviour, ///< Hang at the start or end of line.
    ForceHang, ///< Force hanging at the end of line.
    ConditionallyHang ///< Only hang if necessary.
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
    bool addressable =
        true; // whether the character is not discarded for various reasons.
    bool middle = false; // whether the character is the second of last of a
                         // typographic character.
    bool anchored_chunk = false; // whether this is the start of a new chunk.

    QPainterPath path;
    QImage image{0};

    QVector<QPainterPath> colorLayers;
    QVector<QBrush> colorLayerColors;
    QVector<bool> replaceWithForeGroundColor;

    QRectF boundingBox;
    int visualIndex = -1;
    QPointF cssPosition = QPointF();
    QPointF advance;
    BreakType breakType = NoBreak;
    LineEdgeBehaviour lineEnd = NoChange;
    LineEdgeBehaviour lineStart = NoChange;
    bool isHanging = false;
    qreal textLengthApplied = false;

    KoSvgText::TextAnchor anchor = KoSvgText::AnchorStart;
    KoSvgText::Direction direction = KoSvgText::DirectionLeftToRight;
};

class KoSvgTextShape::Private
{
public:
    // NOTE: the cache data is shared between all the instances of
    //       the shape, though it will be reset locally if the
    //       accessing thread changes

    Private()
    {
    }

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
    void breakLines(KoSvgTextProperties properties,
                    QMap<int, int> logicalToVisual,
                    QVector<CharacterResult> &result,
                    QPointF startPos);
    void applyTextLength(const KoShape *rootShape,
                         QVector<CharacterResult> &result,
                         int &currentIndex,
                         int &resolvedDescendentNodes,
                         bool isHorizontal);
    void applyAnchoring(QVector<CharacterResult> &result, bool isHorizontal);
    qreal characterResultOnPath(CharacterResult &cr,
                                qreal length,
                                qreal offset,
                                bool isHorizontal,
                                bool isClosed);
    QPainterPath stretchGlyphOnPath(QPainterPath glyph,
                                    QPainterPath path,
                                    bool isHorizontal,
                                    qreal offset,
                                    bool isClosed);
    void applyTextPath(const KoShape *rootShape,
                       QVector<CharacterResult> &result,
                       bool isHorizontal);
    void computeFontMetrics(const KoShape *rootShape,
                            QMap<int, int> parentBaselineTable,
                            qreal parentFontSize,
                            QPointF superScript,
                            QPointF subScript,
                            QVector<CharacterResult> &result,
                            int &currentIndex,
                            qreal res,
                            bool isHorizontal);
    void computeTextDecorations(const KoShape *rootShape,
                                QVector<CharacterResult> result,
                                QMap<int, int> logicalToVisual,
                                qreal minimumDecorationThickness,
                                KoPathShape *textPath,
                                qreal textPathoffset,
                                bool side,
                                int &currentIndex,
                                bool isHorizontal,
                                bool ltr,
                                bool wrapping);
    void paintPaths(QPainter &painter,
                    QPainterPath outlineRect,
                    const KoShape *rootShape,
                    QVector<CharacterResult> &result,
                    QPainterPath &chunk,
                    int &currentIndex);
    QList<KoShape *> collectPaths(const KoShape *rootShape,
                                  QVector<CharacterResult> &result,
                                  int &currentIndex);
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

void KoSvgTextShape::paintComponent(QPainter &painter) const
{
    // if (d->textRendering == OptimizeLegibility) {
    /**
     * HACK ALERT:
     *
     * For hinting and bitmaps, we need to get the hinting metrics from
     * freetype, but those need the DPI. We can't get the DPI normally, however,
     * neither rotate and shear change the length of a line, and it may not be
     * that bad if freetype receives a scaled value for the DPI.
     */
    int xRes = qRound(
        painter.transform().map(QLineF(QPointF(), QPointF(72, 0))).length());
    int yRes = qRound(
        painter.transform().map(QLineF(QPointF(), QPointF(0, 72))).length());
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
    } else {
        painter.setRenderHint(QPainter::Antialiasing, true);
        painter.setRenderHint(QPainter::SmoothPixmapTransform, true);
    }

    QPainterPath chunk;
    int currentIndex = 0;
    if (!d->result.empty()) {
        d->paintPaths(painter,
                      this->outline(),
                      this,
                      d->result,
                      chunk,
                      currentIndex);
    }
    /* Debug
    Q_FOREACH (KoShape *child, this->shapes()) {
        const KoSvgTextChunkShape *textPathChunk = dynamic_cast<const
    KoSvgTextChunkShape*>(child); if (textPathChunk) { painter.save();
            painter.setPen(Qt::magenta);
            painter.setOpacity(0.5);
            if (textPathChunk->layoutInterface()->textPath()) {
                QPainterPath p =
    textPathChunk->layoutInterface()->textPath()->outline(); p =
    textPathChunk->layoutInterface()->textPath()->transformation().map(p);
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

void KoSvgTextShape::paintStroke(QPainter &painter) const
{
    Q_UNUSED(painter);
    // do nothing! everything is painted in paintComponent()
}

QList<KoShape *> KoSvgTextShape::textOutline() const
{
    QList<KoShape *> shapes;
    int currentIndex = 0;
    if (d->result.size() > 0) {
        shapes = d->collectPaths(this, d->result, currentIndex);
    }

    return shapes;
}

void KoSvgTextShape::setTextRenderingFromString(QString textRendering)
{
    if (textRendering == "optimizeSpeed") {
        d->textRendering = OptimizeSpeed;
    } else if (textRendering == "optimizeLegibility") {
        d->textRendering = OptimizeLegibility;
    } else if (textRendering == "geometricPrecision") {
        d->textRendering = GeometricPrecision;
    } else {
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
        this->textProperties()
            .propertyOrDefault(KoSvgTextProperties::WritingModeId)
            .toInt());
    KoSvgText::Direction direction = KoSvgText::Direction(
        this->textProperties()
            .propertyOrDefault(KoSvgTextProperties::DirectionId)
            .toInt());
    KoSvgText::AutoValue inlineSize =
        this->textProperties()
            .propertyOrDefault(KoSvgTextProperties::InlineSizeId)
            .value<KoSvgText::AutoValue>();
    QString lang = this->textProperties()
                       .property(KoSvgTextProperties::TextLanguage)
                       .toString()
                       .toUtf8();

    bool isHorizontal = writingMode == KoSvgText::HorizontalTB;

    FT_Int32 loadFlags = FT_LOAD_RENDER;

    if (d->textRendering == GeometricPrecision || d->textRendering == Auto) {
        // without load_no_hinting, the advance and offset will be rounded
        // to nearest pixel, which we don't want as we're using the vector
        // outline.

        loadFlags |= FT_LOAD_NO_HINTING;
    } else {
        // When using hinting, sometimes the bounding box does not encompass the
        // drawn glyphs properly.
        // The default hinting works best for vertical, while the 'light'
        // hinting mode works best for horizontal.
        if (isHorizontal) {
            loadFlags |= FT_LOAD_TARGET_LIGHT;
        }
    }
    // Whenever the freetype docs talk about a 26.6 floating point unit, they
    // mean a 1/64 value.
    const qreal ftFontUnit = 64.0;
    const qreal ftFontUnitFactor = 1 / ftFontUnit;
    QTransform ftTF =
        QTransform::fromScale(ftFontUnitFactor, -ftFontUnitFactor);
    qreal finalRes = qMin(d->xRes, d->yRes);
    qreal scaleToPT = float(72. / finalRes);
    qreal scaleToPixel = float(finalRes / 72.);
    QTransform dpiScale = QTransform::fromScale(scaleToPT, scaleToPT);
    ftTF *= dpiScale;
    // Some fonts have a faulty underline thickness,
    // so we limit the minimum to be a single pixel wide.
    qreal minimumDecorationThickness = scaleToPT;

    // First, get text. We use the subChunks because that handles bidi for us.
    // SVG 1.1 suggests that each time the xy position of a piece of text
    // changes, that this should be seperately shaped, but according to SVGWG
    // issues 631 and 635 noone who actually uses bidi likes this, and it also
    // complicates the algorithm, so we're not doing that. Anchored Chunks will
    // get generated later. https://github.com/w3c/svgwg/issues/631
    // https://github.com/w3c/svgwg/issues/635

    bool first = false;
    QVector<KoSvgTextChunkShapeLayoutInterface::SubChunk> textChunks =
        layoutInterface()->collectSubChunks(false, first);
    QString text;
    for (KoSvgTextChunkShapeLayoutInterface::SubChunk chunk : textChunks) {
        text.append(chunk.text);
    }
    debugFlake << "Laying out the following text: " << text;

    // 1. Setup.

    // KoSvgText::TextSpaceTrims trims =
    // this->textProperties().propertyOrDefault(KoSvgTextProperties::TextTrimId).value<KoSvgText::TextSpaceTrims>();
    KoSvgText::TextWrap wrap = KoSvgText::TextWrap(
        this->textProperties()
            .propertyOrDefault(KoSvgTextProperties::TextWrapId)
            .toInt());
    KoSvgText::TextSpaceCollapse collapse = KoSvgText::TextSpaceCollapse(
        this->textProperties()
            .propertyOrDefault(KoSvgTextProperties::TextCollapseId)
            .toInt());
    KoSvgText::LineBreak linebreakStrictness =
        KoSvgText::LineBreak(this->textProperties()
                                 .property(KoSvgTextProperties::LineBreakId)
                                 .toInt());
    QVector<bool> collapseChars =
        KoCssTextUtils::collapseSpaces(text, collapse);
    if (!lang.isEmpty()) {
        // Libunibreak currently only has support for strict, and even then only
        // for very specific cases.
        if (linebreakStrictness == KoSvgText::LineBreakStrict) {
            lang += "-strict";
        }
    }
    QVector<char> lineBreaks(text.size());
    QVector<char> graphemeBreaks(text.size());
    if (text.size() > 0) {
        // TODO: Figure out how to gracefully skip all the next steps when the text-size is 0.
        // can't currently remember if removing the associated outlines was all that is necessary.
    set_linebreaks_utf16(text.utf16(),
                         static_cast<size_t>(text.size()),
                         lang.toUtf8().data(),
                         lineBreaks.data());
    set_graphemebreaks_utf16(text.utf16(),
                             static_cast<size_t>(text.size()),
                             lang.toUtf8().data(),
                             graphemeBreaks.data());
    }

    int globalIndex = 0;
    QVector<CharacterResult> result(text.size());
    // 3. Resolve character positioning.
    // This is done earlier so it's possible to get preresolved transforms from
    // the subchunks.
    QVector<KoSvgText::CharTransformation> resolvedTransforms(text.size());
    if (resolvedTransforms.size() > 0) {
        // Ensure the first entry defaults to 0.0 for x and y, otherwise textAnchoring
        // will not work for text that has been bidi-reordered.
        resolvedTransforms[0].xPos = 0.0;
        resolvedTransforms[0].yPos = 0.0;
    }
    QMap<int, KoSvgText::TabSizeInfo> tabSizeInfo;

    // pass everything to a css-compatible text-layout algortihm.
    raqm_t_up layout(raqm_create());

    if (raqm_set_text_utf16(layout.data(), text.utf16(), text.size())) {
        if (writingMode == KoSvgText::VerticalRL
            || writingMode == KoSvgText::VerticalLR) {
            raqm_set_par_direction(layout.data(),
                                   raqm_direction_t::RAQM_DIRECTION_TTB);
        } else if (direction == KoSvgText::DirectionRightToLeft) {
            raqm_set_par_direction(layout.data(),
                                   raqm_direction_t::RAQM_DIRECTION_RTL);
        } else {
            raqm_set_par_direction(layout.data(),
                                   raqm_direction_t::RAQM_DIRECTION_LTR);
        }

        int start = 0;
        int length = 0;
        for (KoSvgTextChunkShapeLayoutInterface::SubChunk chunk : textChunks) {
            length = chunk.text.size();
            KoSvgTextProperties properties =
                chunk.format.associatedShapeWrapper().shape()->textProperties();

            // In this section we retrieve the resolved transforms and
            // direction/anchoring that we can get from the subchunks.
            KoSvgText::TextAnchor anchor = KoSvgText::TextAnchor(
                properties.propertyOrDefault(KoSvgTextProperties::TextAnchorId)
                    .toInt());
            KoSvgText::Direction direction = KoSvgText::Direction(
                properties.propertyOrDefault(KoSvgTextProperties::DirectionId)
                    .toInt());
            KoSvgText::WordBreak wordBreakStrictness = KoSvgText::WordBreak(
                properties.propertyOrDefault(KoSvgTextProperties::WordBreakId)
                    .toInt());
            KoSvgText::HangingPunctuations hang =
                properties
                    .propertyOrDefault(
                        KoSvgTextProperties::HangingPunctuationId)
                    .value<KoSvgText::HangingPunctuations>();
            KoSvgText::TabSizeInfo tabInfo =
                properties.propertyOrDefault(KoSvgTextProperties::TabSizeId)
                    .value<KoSvgText::TabSizeInfo>();
            KoSvgText::AutoValue letterSpacing =
                properties
                    .propertyOrDefault(KoSvgTextProperties::LetterSpacingId)
                    .value<KoSvgText::AutoValue>();
            KoSvgText::AutoValue wordSpacing =
                properties.propertyOrDefault(KoSvgTextProperties::WordSpacingId)
                    .value<KoSvgText::AutoValue>();

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
                if (lineBreaks[start + i] == LINEBREAK_MUSTBREAK) {
                    cr.breakType = HardBreak;
                    cr.lineEnd = Collapse;
                    cr.lineStart = Collapse;
                } else if (lineBreaks[start + i] == LINEBREAK_ALLOWBREAK
                           && wrap != KoSvgText::NoWrap) {
                    cr.breakType = SoftBreak;
                    if (KoCssTextUtils::collapseLastSpace(text.at(start + i),
                                                          collapse)) {
                        cr.lineEnd = Collapse;
                        cr.lineStart = Collapse;
                    }
                }
                if (wordBreakStrictness == KoSvgText::WordBreakBreakAll
                    || linebreakStrictness == KoSvgText::LineBreakAnywhere) {
                    if (graphemeBreaks[start + i] == GRAPHEMEBREAK_BREAK
                        && cr.breakType == NoBreak) {
                        cr.breakType = SoftBreak;
                    }
                }
                if (cr.lineStart != Collapse
                    && hang.testFlag(KoSvgText::HangFirst)) {
                    cr.lineStart =
                        KoCssTextUtils::characterCanHang(text.at(start + i),
                                                         KoSvgText::HangFirst)
                        ? HangBehaviour
                        : cr.lineEnd;
                }
                if (cr.lineEnd != Collapse) {
                    if (hang.testFlag(KoSvgText::HangLast)) {
                        cr.lineEnd = KoCssTextUtils::characterCanHang(
                                         text.at(start + i),
                                         KoSvgText::HangLast)
                            ? HangBehaviour
                            : cr.lineEnd;
                    }
                    if (hang.testFlag(KoSvgText::HangEnd)) {
                        LineEdgeBehaviour edge =
                            hang.testFlag(KoSvgText::HangForce)
                            ? ForceHang
                            : ConditionallyHang;
                        cr.lineEnd =
                            KoCssTextUtils::characterCanHang(text.at(start + i),
                                                             KoSvgText::HangEnd)
                            ? edge
                            : cr.lineEnd;
                    }
                }
                if (text.at(start + i) == QChar::Tabulation) {
                    tabSizeInfo.insert(start + i, tabInfo);
                }
                
                if (chunk.firstTextInPath && i==0) {
                    cr.anchored_chunk = true;
                }
                result[start + i] = cr;
                // TODO: figure out how to use addressability to only set
                // transforms on addressable chars.
                //! collapseChars.at(i);
                
                if (i < chunk.transformation.size()) {
                    KoSvgText::CharTransformation newTransform =
                        chunk.transformation.at(i);

                    if (chunk.textInPath) {
                        // Unset the perpendicular absolute transform for textPaths as suggested by the SVG 2 algorithm.
                        if (isHorizontal) {
                            newTransform.yPos.reset();
                        } else {
                            newTransform.xPos.reset();
                        }
                    }
                    resolvedTransforms[start + i] = newTransform;
                } else if (start > 0) {
                    resolvedTransforms[start + i].rotate =
                        resolvedTransforms[start + i - 1].rotate;
                }
                if (chunk.firstTextInPath && i==0) {
                    if (chunk.textInPath) {
                        //  Also unset the first transform on a textPath to avoid breakage with rtl text.
                        if (isHorizontal) {
                            resolvedTransforms[start + i].yPos.reset();
                            resolvedTransforms[start + i].xPos = 0.0;
                        } else {
                            resolvedTransforms[start + i].xPos.reset();
                            resolvedTransforms[start + i].yPos = 0.0;
                        }
                    }
                }
            }

            QVector<int> lengths;
            QStringList fontFeatures =
                properties.fontFeaturesForText(start, length);

            qreal fontSize =
                properties.property(KoSvgTextProperties::FontSizeId).toReal();
            const QFont::Style style = QFont::Style(
                properties.propertyOrDefault(KoSvgTextProperties::FontStyleId)
                    .toInt());
            const std::vector<FT_FaceUP> faces =
                KoFontRegistery::instance()->facesForCSSValues(
                    properties.property(KoSvgTextProperties::FontFamiliesId)
                        .toStringList(),
                    lengths,
                    chunk.text,
                    fontSize,
                    properties
                        .propertyOrDefault(KoSvgTextProperties::FontWeightId)
                        .toInt(),
                    properties
                        .propertyOrDefault(KoSvgTextProperties::FontStretchId)
                        .toInt(),
                    style != QFont::StyleNormal);
            KoSvgText::AutoValue fontSizeAdjust =
                properties
                    .propertyOrDefault(KoSvgTextProperties::FontSizeAdjustId)
                    .value<KoSvgText::AutoValue>();
            if (properties.hasProperty(KoSvgTextProperties::KraTextVersionId)) {
                fontSizeAdjust.isAuto =
                    (properties.property(KoSvgTextProperties::KraTextVersionId)
                         .toInt()
                     < 3);
            }
            KoFontRegistery::instance()->configureFaces(
                faces,
                fontSize,
                fontSizeAdjust.isAuto ? 1.0 : fontSizeAdjust.customValue,
                finalRes,
                finalRes,
                properties.fontAxisSettings());
            if (properties.hasProperty(KoSvgTextProperties::TextLanguage)) {
                raqm_set_language(
                    layout.data(),
                    properties.property(KoSvgTextProperties::TextLanguage)
                        .toString()
                        .toUtf8(),
                    start,
                    length);
            }
            for (QString feature : fontFeatures) {
                debugFlake << "adding feature" << feature;
                raqm_add_font_feature(layout.data(),
                                      feature.toUtf8(),
                                      feature.toUtf8().size());
            }

            if (!letterSpacing.isAuto) {
                raqm_set_letter_spacing_range(layout.data(),
                                              letterSpacing.customValue
                                                  * ftFontUnit * scaleToPixel,
                                              false,
                                              start,
                                              length);
            }

            if (!wordSpacing.isAuto) {
                raqm_set_word_spacing_range(layout.data(),
                                            wordSpacing.customValue * ftFontUnit
                                                * scaleToPixel,
                                            false,
                                            start,
                                            length);
            }

            for (int i = 0; i < lengths.size(); i++) {
                length = lengths.at(i);
                FT_Int32 faceLoadFlags = loadFlags;
                const FT_FaceUP &face = faces.at(i);
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
                    raqm_set_freetype_face_range(layout.data(),
                                                 face.data(),
                                                 start,
                                                 length);
                    raqm_set_freetype_load_flags_range(layout.data(),
                                                       faceLoadFlags,
                                                       start,
                                                       length);
                }
                start += length;
            }
        }
        debugFlake << "text-length:" << text.size();
    }
    // set very first character as anchored chunk.
    if (result.size() > 0) {
        result[0].anchored_chunk = true;
    }

    if (raqm_layout(layout.data())) {
        debugFlake << "layout succeeded";
    }

    // 2. Set flags and assign initial positions
    // We also retreive a glyph path here.
    size_t count;
    raqm_glyph_t *glyphs = raqm_get_glyphs(layout.data(), &count);
    if (!glyphs) {
        return;
    }

    QPointF totalAdvanceFTFontCoordinates;
    QMap<int, int> logicalToVisual;

    for (int g = 0; g < int(count); g++) {
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

        QPointF spaceAdvance;
        if (tabSizeInfo.contains(glyphs[g].cluster)) {
            FT_Load_Glyph(glyphs[g].ftface,
                          FT_Get_Char_Index(glyphs[g].ftface, ' '),
                          faceLoadFlags);
            spaceAdvance = QPointF(glyphs[g].ftface->glyph->advance.x,
                                   glyphs[g].ftface->glyph->advance.y);
        }

        int error =
            FT_Load_Glyph(glyphs[g].ftface, glyphs[g].index, faceLoadFlags);
        if (error != 0) {
            continue;
        }

        debugFlake << "glyph" << g << "cluster" << glyphs[g].cluster
                   << glyphs[g].index;

        FT_Matrix matrix;
        FT_Vector delta;
        FT_Get_Transform(glyphs[g].ftface, &matrix, &delta);
        QTransform glyphTf;
        qreal factor_16 = 1.0 / 65536.0;
        glyphTf.setMatrix(matrix.xx * factor_16,
                          matrix.xy * factor_16,
                          0,
                          matrix.yx * factor_16,
                          matrix.yy * factor_16,
                          0,
                          0,
                          0,
                          1);

        QPainterPath glyph =
            d->convertFromFreeTypeOutline(glyphs[g].ftface->glyph);

        glyph.translate(glyphs[g].x_offset, glyphs[g].y_offset);
        glyph = glyphTf.map(glyph);
        glyph = ftTF.map(glyph);

        if (!charResult.path.isEmpty()) {
            // this is for glyph clusters, unicode combining marks are always
            // added. we could have these as seperate paths, but there's no real
            // purpose, and the svg standard prefers 'ligatures' to be treated
            // as a single glyph. It simplifies things for us in any case.
            charResult.path.addPath(glyph.translated(charResult.advance));
        } else {
            charResult.path = glyph;
        }
        // TODO: Handle glyph clusters better...
        charResult.image = d->convertFromFreeTypeBitmap(glyphs[g].ftface->glyph)
                               .transformed(glyphTf,
                                            d->textRendering == OptimizeSpeed
                                                ? Qt::FastTransformation
                                                : Qt::SmoothTransformation);

        // Retreive CPAL/COLR V0 color layers, directly based off the sample
        // code in the freetype docs.
        FT_UInt layerGlyphIndex = 0;
        FT_UInt layerColorIndex = 0;
        FT_LayerIterator iterator;
        FT_Color *palette;
        int paletteIndex = 0;
        error = FT_Palette_Select(glyphs[g].ftface, paletteIndex, &palette);
        if (error) {
            palette = NULL;
        }
        iterator.p = NULL;
        bool haveLayers = FT_Get_Color_Glyph_Layer(glyphs[g].ftface,
                                                   glyphs[g].index,
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
                    layerColor =
                        QColor(color.red, color.green, color.blue, color.alpha);
                }
                FT_Load_Glyph(glyphs[g].ftface, layerGlyphIndex, faceLoadFlags);
                QPainterPath p =
                    d->convertFromFreeTypeOutline(glyphs[g].ftface->glyph);
                p.translate(glyphs[g].x_offset, glyphs[g].y_offset);
                charResult.colorLayers.append(ftTF.map(p));
                charResult.colorLayerColors.append(layerColor);
                charResult.replaceWithForeGroundColor.append(isForeGroundColor);

            } while (FT_Get_Color_Glyph_Layer(glyphs[g].ftface,
                                              glyphs[g].index,
                                              &layerGlyphIndex,
                                              &layerColorIndex,
                                              &iterator));
        }

        charResult.visualIndex = g;
        logicalToVisual.insert(glyphs[g].cluster, g);

        charResult.middle = false;
        QPointF advance(glyphs[g].x_advance, glyphs[g].y_advance);
        if (tabSizeInfo.contains(glyphs[g].cluster)) {
            KoSvgText::TabSizeInfo tabSize =
                tabSizeInfo.value(glyphs[g].cluster);
            qreal newAdvance = tabSize.value * ftFontUnit;
            if (tabSize.isNumber) {
                QPointF extraSpacing = isHorizontal
                    ? QPointF(tabSize.extraSpacing * ftFontUnit, 0)
                    : QPointF(0, tabSize.extraSpacing * ftFontUnit);
                advance = (spaceAdvance + extraSpacing) * tabSize.value;
            } else {
                advance = isHorizontal ? QPointF(newAdvance, advance.y())
                                       : QPointF(advance.x(), newAdvance);
            }
            charResult.path = QPainterPath();
            charResult.image = QImage();
        }
        charResult.advance += ftTF.map(advance);

        bool usePixmap =
            !charResult.image.isNull() && charResult.path.isEmpty();

        QRectF bbox;
        if (usePixmap) {
            QPointF topLeft((glyphs[g].ftface->glyph->bitmap_left * 64),
                            (glyphs[g].ftface->glyph->bitmap_top * 64));
            topLeft = glyphTf.map(topLeft);
            QPointF bottomRight(topLeft.x() + (charResult.image.height() * 64),
                                topLeft.y() - (charResult.image.height() * 64));
            bbox = QRectF(topLeft, bottomRight);
            if (isHorizontal) {
                bbox.setWidth(ftTF.inverted().map(charResult.advance).x());
            } else {
                bbox.setHeight(ftTF.inverted().map(charResult.advance).y());
                bbox.translate(-(bbox.width() * 0.5), 0);
            }
        } else if (isHorizontal) {
            bbox = QRectF(0,
                          glyphs[g].ftface->size->metrics.descender,
                          ftTF.inverted().map(charResult.advance).x(),
                          (glyphs[g].ftface->size->metrics.ascender
                           - glyphs[g].ftface->size->metrics.descender));
            bbox = glyphTf.mapRect(bbox);
        } else {
            hb_font_t_up font(hb_ft_font_create_referenced(glyphs[g].ftface));
            hb_position_t ascender = 0;
            hb_ot_metrics_get_position(font.data(),
                                       HB_OT_METRICS_TAG_VERTICAL_ASCENDER,
                                       &ascender);
            hb_position_t descender = 0;
            hb_ot_metrics_get_position(font.data(),
                                       HB_OT_METRICS_TAG_VERTICAL_DESCENDER,
                                       &descender);
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
        charResult.cssPosition =
            ftTF.map(totalAdvanceFTFontCoordinates) - charResult.advance;

        result[glyphs[g].cluster] = charResult;
    }

    // fix it so that characters that are in the 'middle' due to either being
    // surrogates or part of a ligature, are marked as such.
    int firstCluster = 0;
    for (int i = 0; i < result.size(); i++) {
        if (result.at(i).visualIndex != -1) {
            firstCluster = i;
        } else {
            result[firstCluster].breakType = result.at(i).breakType;
            result[firstCluster].lineStart = result.at(i).lineStart;
            result[firstCluster].lineEnd = result.at(i).lineEnd;
            result[i].middle = true;
            result[i].addressable = false;
        }
    }
    debugFlake << "Glyphs retreived";

    // Handle linebreaking.
    QPointF startPos =
        inlineSize.isAuto ? QPointF() : resolvedTransforms[0].absolutePos();
    d->breakLines(this->textProperties(), logicalToVisual, result, startPos);

    // Handle baseline alignment.
    globalIndex = 0;
    d->computeFontMetrics(this,
                          QMap<int, int>(),
                          0,
                          QPointF(),
                          QPointF(),
                          result,
                          globalIndex,
                          finalRes,
                          isHorizontal);

    // This is the best point to start applying linebreaking and text-wrapping.
    // If we're doing text-wrapping we should skip the other positioning steps
    // of the algorithm.

    if (inlineSize.isAuto) {
        debugFlake << "Starting with SVG 1.1 specific portion";
        debugFlake << "4. Adjust positions: dx, dy";
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
        debugFlake << "5. Apply ‘textLength’ attribute";
        globalIndex = 0;
        int resolved = 0;
        d->applyTextLength(this, result, globalIndex, resolved, isHorizontal);

        // 6. Adjust positions: x, y
        debugFlake << "6. Adjust positions: x, y";
        // https://github.com/w3c/svgwg/issues/617
        shift = QPointF();
        // bool setNextAnchor = false;
        for (int i = 0; i < result.size(); i++) {
            if (result.at(i).addressable) {
                KoSvgText::CharTransformation transform = resolvedTransforms[i];
                CharacterResult charResult = result[i];
                if (transform.xPos) {
                    qreal d = transform.dxPos ? *transform.dxPos : 0.0;
                    shift.setX(*transform.xPos
                               + (d - charResult.finalPosition.x()));
                }
                if (transform.yPos) {
                    qreal d = transform.dyPos ? *transform.dyPos : 0.0;
                    shift.setY(*transform.yPos
                               + (d - charResult.finalPosition.y()));
                }
                charResult.finalPosition += shift;

                /*
            if (setNextAnchor) {
                charResult.anchored_chunk = true;
            }

            if (charResult.middle && charResult.anchored_chunk) {
                charResult.anchored_chunk = false;
                if (i + 1 < result.size()) {
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
        debugFlake << "7. Apply anchoring";
        d->applyAnchoring(result, isHorizontal);

        // Computing the textDecorations needs to happen before applying the
        // textPath to the results, as we need the unapplied result vector for
        // positioning.
        debugFlake << "Now Computing text-decorations";
        globalIndex = 0;
        d->computeTextDecorations(this,
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
        d->applyTextPath(this, result, isHorizontal);
    } else {
        globalIndex = 0;
        debugFlake << "Computing text-decorationsfor inline-size";
        d->computeTextDecorations(this,
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
    d->result = result;
    globalIndex = 0;
    QTransform tf;
    for (KoSvgTextChunkShapeLayoutInterface::SubChunk chunk : textChunks) {
        KoSvgText::AssociatedShapeWrapper wrapper =
            chunk.format.associatedShapeWrapper();
        int j = chunk.text.size();
        for (int i = globalIndex; i < globalIndex + j; i++) {
            if (result.at(i).addressable && result.at(i).hidden == false) {
                tf.reset();
                tf.translate(result.at(i).finalPosition.x(),
                             result.at(i).finalPosition.y());
                tf.rotateRadians(result.at(i).rotate);
                wrapper.addCharacterRect(tf.mapRect(result.at(i).boundingBox));
            }
        }
        globalIndex += j;
    }
}

void KoSvgTextShape::Private::clearAssociatedOutlines(const KoShape *rootShape)
{
    const KoSvgTextChunkShape *chunkShape =
        dynamic_cast<const KoSvgTextChunkShape *>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);

    chunkShape->layoutInterface()->clearAssociatedOutline();

    Q_FOREACH (KoShape *child, chunkShape->shapes()) {
        clearAssociatedOutlines(child);
    }
}

QPainterPath
KoSvgTextShape::Private::convertFromFreeTypeOutline(FT_GlyphSlotRec *glyphSlot)
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
        QPointF start = QPointF(glyphSlot->outline.points[i].x,
                                glyphSlot->outline.points[i].y);
        if (!(glyphSlot->outline.tags[i] & 1)) { // start point is not on curve:
            if (!(glyphSlot->outline.tags[last_point]
                  & 1)) { // end point is not on curve:
                // qDebug() << "  start and end point are not on curve";
                start = (QPointF(glyphSlot->outline.points[last_point].x,
                                 glyphSlot->outline.points[last_point].y)
                         + start)
                    / 2.0;
            } else {
                // qDebug() << "  end point is on curve, start is not";
                start = QPointF(glyphSlot->outline.points[last_point].x,
                                glyphSlot->outline.points[last_point].y);
            }
            --i; // to use original start point as control point below
        }
        start += cp;
        // qDebug() << "  start at" << start;
        glyph.moveTo(start);
        QPointF c[4];
        c[0] = start;
        int n = 1;
        while (i < last_point) {
            ++i;
            c[n] = cp
                + QPointF(glyphSlot->outline.points[i].x,
                          glyphSlot->outline.points[i].y);
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
                c[3] = (c[3] + c[2]) / 2;
                --i;
                break;
            case 0:
                // quadratic bezier element
                if (n < 3)
                    continue;
                c[3] = (c[1] + c[2]) / 2;
                c[2] = (2 * c[1] + c[3]) / 3;
                c[1] = (2 * c[1] + c[0]) / 3;
                --i;
                break;
            case 1:
            case 3:
                if (n == 2) {
                    // qDebug() << "  lineTo" << c[1];
                    glyph.lineTo(c[1]);
                    c[0] = c[1];
                    n = 1;
                    continue;
                } else if (n == 3) {
                    c[3] = c[2];
                    c[2] = (2 * c[1] + c[3]) / 3;
                    c[1] = (2 * c[1] + c[0]) / 3;
                }
                break;
            }
            // qDebug() << "  cubicTo" << c[1] << c[2] << c[3];
            glyph.cubicTo(c[1], c[2], c[3]);
            c[0] = c[3];
            n = 1;
        }
        if (n == 1) {
            // qDebug() << "  closeSubpath";
            glyph.closeSubpath();
        } else {
            c[3] = start;
            if (n == 2) {
                c[2] = (2 * c[1] + c[3]) / 3;
                c[1] = (2 * c[1] + c[0]) / 3;
            }
            // qDebug() << "  close cubicTo" << c[1] << c[2] << c[3];
            glyph.cubicTo(c[1], c[2], c[3]);
        }
        ++i;
    }
    return glyph;
}

QImage
KoSvgTextShape::Private::convertFromFreeTypeBitmap(FT_GlyphSlotRec *glyphSlot)
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
        const uint8_t *src = glyphSlot->bitmap.buffer;
        for (uint y = 0; y < glyphSlot->bitmap.rows; y++) {
            auto *argb = reinterpret_cast<QRgb *>(img.scanLine(y));
            for (unsigned int x = 0; x < glyphSlot->bitmap.width; x++) {
                argb[x] = qRgba(src[2], src[1], src[0], src[3]);
                src += 4;
            }
        }
    }

    return img;
}

/**
 * @brief addWordToLine
 * Small function used in break lines to quickly add a 'word' to the current
 * line. Returns the last added index.
 */
void addWordToLine(QVector<CharacterResult> &result,
                   QPointF &currentPos,
                   QVector<int> &wordIndices,
                   QRectF &lineBox,
                   QVector<int> &lineIndices,
                   bool ltr,
                   bool firstLine)
{
    QPointF lineAdvance = currentPos;

    if (lineBox.isEmpty()) {
        lineIndices.clear();
    }

    for (int j : wordIndices) {
        CharacterResult cr = result.at(j);
        if (lineBox.isEmpty() && j == wordIndices.first()) {
            if (result.at(j).lineStart == Collapse) {
                result[j].addressable = false;
                result[j].hidden = true;
                continue;
            }
            cr.anchored_chunk = true;
            if (result.at(j).lineStart == HangBehaviour && firstLine) {
                if (ltr) {
                    currentPos -= cr.advance;
                } else {
                    currentPos += cr.advance;
                }
                cr.isHanging = true;
            }
        }
        cr.cssPosition = currentPos;
        currentPos += cr.advance;
        lineAdvance = currentPos;

        result[j] = cr;
        lineBox |= cr.boundingBox.translated(cr.cssPosition);
    }
    currentPos = lineAdvance;
    lineIndices += wordIndices;
    wordIndices.clear();
}

/**
 * This offsets the last line by it's ascent, and then returns the last line's
 * descent.
 */
QPointF lineHeightOffset(KoSvgText::WritingMode writingMode,
                         QVector<CharacterResult> &result,
                         QVector<int> lineIndices,
                         QRectF lineBox,
                         QPointF currentPos,
                         KoSvgText::AutoValue lineHeight)
{
    QPointF offset;
    if (lineHeight.isAuto) {
        offset = writingMode == KoSvgText::HorizontalTB
            ? QPointF(0, lineBox.height())
            : writingMode == KoSvgText::VerticalLR
            ? QPointF(lineBox.width(), 0)
            : QPointF(-lineBox.width(), 0);
    } else {
        offset = writingMode == KoSvgText::HorizontalTB
            ? QPointF(0, lineHeight.customValue)
            : writingMode == KoSvgText::VerticalLR
            ? QPointF(lineHeight.customValue, 0)
            : QPointF(-lineHeight.customValue, 0);
    }
    qreal ascentRatio = writingMode == KoSvgText::HorizontalTB
        ? abs(lineBox.top() - currentPos.y()) / lineBox.height()
        : writingMode == KoSvgText::VerticalLR
        ? abs(lineBox.left() - currentPos.x()) / lineBox.width()
        : abs(lineBox.right() - currentPos.x()) / lineBox.width();
    QPointF ascent = offset * ascentRatio;
    bool returnDescent =
        lineIndices.isEmpty() ? false : lineIndices.first() == 0;
    if (!returnDescent) {
        for (int j : lineIndices) {
            result[j].cssPosition += ascent;
            result[j].finalPosition = result.at(j).cssPosition;
        }
    } else {
        offset = offset * (1 - ascentRatio);
    }
    return offset;
}

void handleCollapseAndHang(QVector<CharacterResult> &result,
                           QVector<int> lineIndices,
                           QPointF endPos,
                           QPointF lineOffset,
                           bool inlineSize,
                           KoSvgText::WritingMode writingMode,
                           bool ltr,
                           bool atEnd)
{
    bool isHorizontal = writingMode == KoSvgText::HorizontalTB;
    if (!lineIndices.isEmpty()) {
        QVectorIterator<int> it(lineIndices);
        it.toBack();
        while (it.hasPrevious()) {
            int lastIndex = it.previous();
            if (result.at(lastIndex).lineEnd == Collapse) {
                result[lastIndex].addressable = false;
                result[lastIndex].hidden = true;
            } else if (result.at(lastIndex).lineEnd == ForceHang
                       && inlineSize) {
                QPointF pos = isHorizontal
                    ? QPointF(endPos.x(), lineOffset.y())
                    : QPointF(lineOffset.x(), endPos.y());
                if (!ltr) {
                    pos -= result.at(lastIndex).advance;
                }
                result[lastIndex].cssPosition = pos;
                result[lastIndex].finalPosition = pos;
                result[lastIndex].isHanging = true;
            } else if (result.at(lastIndex).lineEnd == HangBehaviour
                       && inlineSize && atEnd) {
                QPointF pos = isHorizontal
                    ? QPointF(endPos.x(), lineOffset.y())
                    : QPointF(lineOffset.x(), endPos.y());
                if (!ltr) {
                    pos -= result.at(lastIndex).advance;
                }
                result[lastIndex].cssPosition = pos;
                result[lastIndex].finalPosition = pos;
                result[lastIndex].isHanging = true;
            }
            if (result.at(lastIndex).lineEnd != Collapse) {
                break;
            }
        }
    }
}

void applyInlineSizeAnchoring(QVector<CharacterResult> &result,
                              QVector<int> lineIndices,
                              QPointF startPos,
                              QPointF endPos,
                              KoSvgText::TextAnchor anchor,
                              bool ltr,
                              bool isHorizontal,
                              bool firstLine,
                              bool hangTextIndent,
                              QPointF textIndent)
{
    qreal shift = isHorizontal ? startPos.x() : startPos.y();

    qreal a = 0;
    qreal b = 0;

    QPointF aStartPos = startPos;
    QPointF inlineWidth = startPos - endPos;
    QPointF aEndPos = aStartPos - inlineWidth;

    for (int i : lineIndices) {
        if (!result.at(i).addressable || result.at(i).isHanging) {
            continue;
        }
        qreal pos = isHorizontal ? result.at(i).finalPosition.x()
                                 : result.at(i).finalPosition.y();
        qreal advance =
            isHorizontal ? result.at(i).advance.x() : result.at(i).advance.y();

        if (i == lineIndices.first()) {
            a = qMin(pos, pos + advance);
            b = qMax(pos, pos + advance);
        } else {
            a = qMin(a, qMin(pos, pos + advance));
            b = qMax(b, qMax(pos, pos + advance));
        }
    }
    bool indenting = firstLine ? true : false;
    if (hangTextIndent) {
        indenting = !indenting;
    }

    if (indenting && anchor == KoSvgText::AnchorStart) {
        qreal indent = isHorizontal ? textIndent.x() : textIndent.y();
        if (ltr) {
            a -= indent;
        } else {
            b += indent;
        }
    }

    if (anchor == KoSvgText::AnchorEnd) {
        aEndPos = aStartPos;
        aStartPos = aStartPos + inlineWidth;
    }

    if ((anchor == KoSvgText::AnchorStart && ltr)
        || (anchor == KoSvgText::AnchorEnd && !ltr)) {
        shift -= a;

    } else if ((anchor == KoSvgText::AnchorEnd && ltr)
               || (anchor == KoSvgText::AnchorStart && !ltr)) {
        shift -= b;

    } else {
        aEndPos = (startPos + aEndPos) * 0.5;
        aStartPos = startPos - aEndPos;
        shift -= ((a + b) * 0.5);
    }

    QPointF shiftP = isHorizontal ? QPointF(shift, 0) : QPointF(0, shift);
    for (int j : lineIndices) {
        if (!result.at(j).isHanging) {
            result[j].cssPosition += shiftP;
            result[j].finalPosition = result.at(j).cssPosition;
        } else if (result.at(j).anchored_chunk) {
            QPointF shift = isHorizontal
                ? QPointF(aStartPos.x(), result[j].finalPosition.y())
                : QPointF(result[j].finalPosition.x(), aStartPos.y());
            shift = ltr ? shift - result.at(j).advance : shift;
            result[j].cssPosition = shift;
            result[j].finalPosition = result.at(j).cssPosition;
        } else if (result.at(j).lineEnd != NoChange) {
            QPointF shift = isHorizontal
                ? QPointF(aEndPos.x(), result[j].finalPosition.y())
                : QPointF(result[j].finalPosition.x(), aEndPos.y());
            shift = ltr ? shift : shift - result.at(j).advance;
            result[j].cssPosition = shift;
            result[j].finalPosition = result.at(j).cssPosition;
        }
    }
}

/// Finalizing the line consists of several steps, like hang/collapse, anchoring
/// into place and offsetting correctly. This can happen several times during a
/// linebreak, hence this convenience function to handle this.
void finalizeLine(QVector<CharacterResult> &result,
                  QPointF &currentPos,
                  QRectF &lineBox,
                  QVector<int> &lineIndices,
                  QPointF &lineOffset,
                  QPointF startPos,
                  QPointF endPos,
                  KoSvgText::TextAnchor anchor,
                  bool &firstLine,
                  KoSvgText::AutoValue lineHeight,
                  KoSvgText::WritingMode writingMode,
                  bool ltr,
                  bool inlineSize,
                  bool atEnd,
                  bool hangTextIndent,
                  QPointF textIndent)
{
    bool isHorizontal = writingMode == KoSvgText::HorizontalTB;

    QMap<int, int> visualToLogical;
    for (int j : lineIndices) {
        visualToLogical.insert(result.at(j).visualIndex, j);
    }
    currentPos = lineOffset;

    handleCollapseAndHang(result,
                          lineIndices,
                          endPos,
                          lineOffset,
                          inlineSize,
                          writingMode,
                          ltr,
                          atEnd);

    for (int j : visualToLogical.values()) {
        if (!result.at(j).addressable || result.at(j).isHanging) {
            continue;
        }
        result[j].cssPosition = currentPos;
        result[j].finalPosition = currentPos;
        currentPos = currentPos + result.at(j).advance;
    }

    if (inlineSize) {
        applyInlineSizeAnchoring(result,
                                 lineIndices,
                                 startPos,
                                 endPos,
                                 anchor,
                                 ltr,
                                 isHorizontal,
                                 firstLine,
                                 hangTextIndent,
                                 textIndent);
    }
    lineOffset += lineHeightOffset(writingMode,
                                   result,
                                   lineIndices,
                                   lineBox,
                                   currentPos,
                                   lineHeight);
    currentPos = lineOffset;
    if (inlineSize) {
        if (hangTextIndent) {
            currentPos += textIndent;
        }
    }
    lineBox = QRectF();
    firstLine = false;
}

void KoSvgTextShape::Private::breakLines(KoSvgTextProperties properties,
                                         QMap<int, int> logicalToVisual,
                                         QVector<CharacterResult> &result,
                                         QPointF startPos)
{
    KoSvgText::WritingMode writingMode = KoSvgText::WritingMode(
        properties.propertyOrDefault(KoSvgTextProperties::WritingModeId)
            .toInt());
    KoSvgText::Direction direction = KoSvgText::Direction(
        properties.propertyOrDefault(KoSvgTextProperties::DirectionId).toInt());
    KoSvgText::AutoValue inlineSize =
        properties.propertyOrDefault(KoSvgTextProperties::InlineSizeId)
            .value<KoSvgText::AutoValue>();
    KoSvgText::AutoValue lineHeight =
        properties.propertyOrDefault(KoSvgTextProperties::LineHeightId)
            .value<KoSvgText::AutoValue>();
    KoSvgText::OverflowWrap overflowWrap = KoSvgText::OverflowWrap(
        properties.propertyOrDefault(KoSvgTextProperties::OverflowWrapId)
            .toInt());
    KoSvgText::TextAnchor anchor = KoSvgText::TextAnchor(
        properties.propertyOrDefault(KoSvgTextProperties::TextAnchorId)
            .toInt());

    bool ltr = direction == KoSvgText::DirectionLeftToRight;
    bool isHorizontal = writingMode == KoSvgText::HorizontalTB;

    QPointF endPos; ///< Used for hanging glyphs at the end of a line.
    QPointF textIndent; ///< The textIndent.
    KoSvgText::TextIndentInfo textIndentInfo =
        properties.propertyOrDefault(KoSvgTextProperties::TextIndentId)
            .value<KoSvgText::TextIndentInfo>();
    if (!inlineSize.isAuto) {
        qreal textIdentValue = textIndentInfo.value;
        if (textIndentInfo.isPercentage) {
            textIndent *= inlineSize.customValue;
        }
        if (isHorizontal) {
            textIndent = QPointF(textIdentValue, 0);
            endPos = ltr ? QPointF(startPos.x() + inlineSize.customValue, 0)
                         : QPointF(startPos.x() - inlineSize.customValue, 0);
        } else {
            textIndent = QPointF(0, textIdentValue);
            endPos = ltr ? QPointF(0, startPos.y() + inlineSize.customValue)
                         : QPointF(0, startPos.y() - inlineSize.customValue);
        }
    }

    QVector<int> wordIndices; ///< 'word' in this case meaning characters
                              ///< inbetween softbreaks.
    QPointF wordAdvance; ///< Approximated advance of the current wordindices.
    QRectF lineBox; ///< The line box gets used to determine lineHeight;

    bool firstLine = true;
    QPointF currentPos =
        startPos; ///< Current position with advances of each character.
    if (!textIndentInfo.hanging) {
        currentPos += textIndent;
    }
    QPointF lineOffset = startPos; ///< Current line offset.

    QVector<int> lineIndices; ///< Indices of characters in line.

    // The following is because we want to do line-length calculations on the
    // 'visual order' instead of the 'logical' order. For rtl, we'll need to
    // count backwards.

    QListIterator<int> it(logicalToVisual.keys());
    while (it.hasNext()) {
        int index = it.next();
        if (index < 0) {
            continue;
        }
        CharacterResult charResult = result.at(index);
        if (!charResult.addressable) {
            continue;
        }
        bool breakLine = false; ///< Whether to break a line.
        bool wordToNextLine =
            false; ///< Whether to add the current 'word' into the next line.

        bool doNotCountAdvance = ((charResult.lineEnd == ConditionallyHang
                                   || charResult.lineEnd == HangBehaviour
                                   || charResult.lineEnd == ForceHang)
                                  && !lineBox.isEmpty());
        if (!doNotCountAdvance) {
            if (wordIndices.isEmpty()) {
                wordAdvance = charResult.advance;
            } else {
                wordAdvance += charResult.advance;
            }
        }
        wordIndices.append(index);
        bool atEnd = !it.hasNext();

        if (charResult.breakType == HardBreak) {
            breakLine = true;
        } else if (charResult.breakType == SoftBreak || atEnd
                   || overflowWrap == KoSvgText::OverflowWrapAnywhere) {
            qreal lineLength = isHorizontal
                ? (currentPos - startPos + wordAdvance).x()
                : (currentPos - startPos + wordAdvance).y();
            if (!inlineSize.isAuto) {
                // Sometimes glyphs are a fraction larger than you'd expect, but
                // not enough to really break the line, so the following is a
                // bit more stable than a simple compare.
                if (qRound((abs(lineLength) - inlineSize.customValue)) > 0) {
                    breakLine = true;
                    wordToNextLine = true;
                } else {
                    addWordToLine(result,
                                  currentPos,
                                  wordIndices,
                                  lineBox,
                                  lineIndices,
                                  ltr,
                                  firstLine);
                }
            }
        }

        if (breakLine) {
            if (wordToNextLine) {
                finalizeLine(result,
                             currentPos,
                             lineBox,
                             lineIndices,
                             lineOffset,
                             startPos,
                             endPos,
                             anchor,
                             firstLine,
                             lineHeight,
                             writingMode,
                             ltr,
                             !inlineSize.isAuto,
                             false,
                             textIndentInfo.hanging,
                             textIndent);

                if (overflowWrap == KoSvgText::OverflowWrapBreakWord) {
                    qreal wordLength =
                        isHorizontal ? wordAdvance.x() : wordAdvance.y();
                    if (!inlineSize.isAuto
                        && wordLength > inlineSize.customValue) {
                        // Word is too large, so we try to add it in
                        // max-width-friendly-chunks.
                        wordAdvance = QPointF();
                        wordLength = 0;
                        QVector<int> partialWord;
                        for (int i : wordIndices) {
                            wordAdvance += result.at(i).advance;
                            wordLength = isHorizontal ? wordAdvance.x()
                                                      : wordAdvance.y();
                            if (wordLength <= inlineSize.customValue) {
                                partialWord.append(i);
                            } else {
                                addWordToLine(result,
                                              currentPos,
                                              partialWord,
                                              lineBox,
                                              lineIndices,
                                              ltr,
                                              firstLine);

                                finalizeLine(result,
                                             currentPos,
                                             lineBox,
                                             lineIndices,
                                             lineOffset,
                                             startPos,
                                             endPos,
                                             anchor,
                                             firstLine,
                                             lineHeight,
                                             writingMode,
                                             ltr,
                                             !inlineSize.isAuto,
                                             false,
                                             textIndentInfo.hanging,
                                             textIndent);

                                wordAdvance = result.at(i).advance;
                                partialWord.append(i);
                            }
                        }
                        wordIndices = partialWord;
                    }
                }

                addWordToLine(result,
                              currentPos,
                              wordIndices,
                              lineBox,
                              lineIndices,
                              ltr,
                              firstLine);

            } else {
                addWordToLine(result,
                              currentPos,
                              wordIndices,
                              lineBox,
                              lineIndices,
                              ltr,
                              firstLine);

                finalizeLine(result,
                             currentPos,
                             lineBox,
                             lineIndices,
                             lineOffset,
                             startPos,
                             endPos,
                             anchor,
                             firstLine,
                             lineHeight,
                             writingMode,
                             ltr,
                             !inlineSize.isAuto,
                             false,
                             textIndentInfo.hanging,
                             textIndent);
            }
            firstLine = false;
        }
        if (atEnd) {
            finalizeLine(result,
                         currentPos,
                         lineBox,
                         lineIndices,
                         lineOffset,
                         startPos,
                         endPos,
                         anchor,
                         firstLine,
                         lineHeight,
                         writingMode,
                         ltr,
                         !inlineSize.isAuto,
                         true,
                         textIndentInfo.hanging,
                         textIndent);
        }
    }
    debugFlake << "Linebreaking finished";
}

void KoSvgTextShape::Private::applyTextLength(const KoShape *rootShape,
                                              QVector<CharacterResult> &result,
                                              int &currentIndex,
                                              int &resolvedDescendentNodes,
                                              bool isHorizontal)
{
    const KoSvgTextChunkShape *chunkShape =
        dynamic_cast<const KoSvgTextChunkShape *>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);

    int i = currentIndex;
    int j = i + chunkShape->layoutInterface()->numChars(true);
    int resolvedChildren = 0;

    Q_FOREACH (KoShape *child, chunkShape->shapes()) {
        applyTextLength(child,
                        result,
                        currentIndex,
                        resolvedChildren,
                        isHorizontal);
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
        bool spacingAndGlyphs = (chunkShape->layoutInterface()->lengthAdjust()
                    == KoSvgText::LengthAdjustSpacingAndGlyphs);
        if (!spacingAndGlyphs) {
            n -= 1;
        }
        qreal delta =
            chunkShape->layoutInterface()->textLength().customValue - (b - a);
            
        QPointF d = isHorizontal? QPointF(delta / n, 0)
                                : QPointF(0, delta / n);

        QPointF shift;
        bool secondTextLengthApplied = false;
        for (int k : visualToLogical.keys()) {
            CharacterResult cr = result[visualToLogical.value(k)];
            if (cr.addressable) {
                cr.finalPosition += shift;
                if (spacingAndGlyphs) {
                    QPointF scale(
                        d.x() != 0 ? (d.x() / cr.advance.x()) + 1 : 1.0,
                        d.y() != 0 ? (d.y() / cr.advance.y()) + 1 : 1.0);
                    QTransform tf = QTransform::fromScale(scale.x(), scale.y());
                    cr.path = tf.map(cr.path);
                    cr.advance = tf.map(cr.advance);
                    cr.boundingBox = tf.mapRect(cr.boundingBox);
                }
                bool last = spacingAndGlyphs? false: k==visualToLogical.keys().last();
                
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
        for (int k : visualToLogical.keys()) {
            if (k>lastVisualValue) {
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
void KoSvgTextShape::Private::computeFontMetrics(
    const KoShape *rootShape,
    QMap<int, int> parentBaselineTable,
    qreal parentFontSize,
    QPointF superScript,
    QPointF subScript,
    QVector<CharacterResult> &result,
    int &currentIndex,
    qreal res,
    bool isHorizontal)
{
    const KoSvgTextChunkShape *chunkShape =
        dynamic_cast<const KoSvgTextChunkShape *>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);

    QMap<int, int> baselineTable;
    int i = currentIndex;
    int j =
        qMin(i + chunkShape->layoutInterface()->numChars(true), result.size());

    KoSvgTextProperties properties = chunkShape->textProperties();

    qreal fontSize =
        properties.propertyOrDefault(KoSvgTextProperties::FontSizeId).toReal();
    qreal baselineShift =
        properties.property(KoSvgTextProperties::BaselineShiftValueId).toReal()
        * fontSize;
    QPointF baselineShiftTotal;
    KoSvgText::BaselineShiftMode baselineShiftMode =
        KoSvgText::BaselineShiftMode(
            properties.property(KoSvgTextProperties::BaselineShiftModeId)
                .toInt());

    if (baselineShiftMode == KoSvgText::ShiftSuper) {
        baselineShiftTotal = isHorizontal
            ? superScript
            : QPointF(-superScript.y(), superScript.x());
    } else if (baselineShiftMode == KoSvgText::ShiftSub) {
        baselineShiftTotal =
            isHorizontal ? subScript : QPointF(-subScript.y(), subScript.x());
    } else if (baselineShiftMode == KoSvgText::ShiftPercentage) {
        baselineShiftTotal = isHorizontal ? QPointF(0, baselineShift)
                                          : QPointF(-baselineShift, 0);
    }

    QVector<int> lengths;
    const QFont::Style style = QFont::Style(
        properties.propertyOrDefault(KoSvgTextProperties::FontStyleId).toInt());
    const std::vector<FT_FaceUP> faces =
        KoFontRegistery::instance()->facesForCSSValues(
            properties.property(KoSvgTextProperties::FontFamiliesId)
                .toStringList(),
            lengths,
            QString(),
            fontSize,
            properties.propertyOrDefault(KoSvgTextProperties::FontWeightId)
                .toInt(),
            properties.propertyOrDefault(KoSvgTextProperties::FontStretchId)
                .toInt(),
            style != QFont::StyleNormal);
    KoSvgText::AutoValue fontSizeAdjust =
        properties.propertyOrDefault(KoSvgTextProperties::FontSizeAdjustId)
            .value<KoSvgText::AutoValue>();
    if (properties.hasProperty(KoSvgTextProperties::KraTextVersionId)) {
        fontSizeAdjust.isAuto =
            (properties.property(KoSvgTextProperties::KraTextVersionId).toInt()
             < 3);
    }
    KoFontRegistery::instance()->configureFaces(
        faces,
        fontSize,
        fontSizeAdjust.isAuto ? 1.0 : fontSizeAdjust.customValue,
        res,
        res,
        properties.fontAxisSettings());
    hb_font_t_up font(hb_ft_font_create_referenced(faces.front().data()));
    qreal freetypePixelsToPt = (1.0 / 64.0) * float(72. / res);

    hb_direction_t dir = HB_DIRECTION_LTR;
    if (!isHorizontal) {
        dir = HB_DIRECTION_TTB;
    }
    hb_script_t script = HB_SCRIPT_UNKNOWN;
    KoSvgText::Baseline dominantBaseline = KoSvgText::Baseline(
        properties.property(KoSvgTextProperties::DominantBaselineId).toInt());

    hb_position_t baseline = 0;
    if (dominantBaseline == KoSvgText::BaselineResetSize
        && parentFontSize > 0) {
        baselineTable = parentBaselineTable;
        qreal multiplier = 1.0 / parentFontSize * fontSize;
        for (int key : baselineTable.keys()) {
            baselineTable.insert(key, baselineTable.value(key) * multiplier);
        }
        dominantBaseline = KoSvgText::BaselineAuto;
    } else if (dominantBaseline == KoSvgText::BaselineNoChange) {
        baselineTable = parentBaselineTable;
        dominantBaseline = KoSvgText::BaselineAuto;
    } else {
        if (hb_version_atleast(4, 0, 0)) {
            hb_ot_layout_get_baseline_with_fallback(
                font.data(),
                HB_OT_LAYOUT_BASELINE_TAG_ROMAN,
                dir,
                script,
                HB_TAG_NONE,
                &baseline);
            baselineTable.insert(KoSvgText::BaselineAlphabetic, baseline);
            hb_ot_layout_get_baseline_with_fallback(
                font.data(),
                HB_OT_LAYOUT_BASELINE_TAG_MATH,
                dir,
                script,
                HB_TAG_NONE,
                &baseline);
            baselineTable.insert(KoSvgText::BaselineMathematical, baseline);
            hb_ot_layout_get_baseline_with_fallback(
                font.data(),
                HB_OT_LAYOUT_BASELINE_TAG_HANGING,
                dir,
                script,
                HB_TAG_NONE,
                &baseline);
            baselineTable.insert(KoSvgText::BaselineHanging, baseline);
            hb_ot_layout_get_baseline_with_fallback(
                font.data(),
                HB_OT_LAYOUT_BASELINE_TAG_IDEO_FACE_CENTRAL,
                dir,
                script,
                HB_TAG_NONE,
                &baseline);
            baselineTable.insert(KoSvgText::BaselineCentral, baseline);
            hb_ot_layout_get_baseline_with_fallback(
                font.data(),
                HB_OT_LAYOUT_BASELINE_TAG_IDEO_EMBOX_BOTTOM_OR_LEFT,
                dir,
                script,
                HB_TAG_NONE,
                &baseline);
            baselineTable.insert(KoSvgText::BaselineIdeographic, baseline);
            if (isHorizontal) {
                hb_ot_metrics_get_position_with_fallback(
                    font.data(),
                    HB_OT_METRICS_TAG_X_HEIGHT,
                    &baseline);
                baselineTable.insert(
                    KoSvgText::BaselineMiddle,
                    (baseline
                     - baselineTable.value(KoSvgText::BaselineAlphabetic))
                        * 0.5);
                hb_ot_metrics_get_position_with_fallback(
                    font.data(),
                    HB_OT_METRICS_TAG_HORIZONTAL_ASCENDER,
                    &baseline);
                baselineTable.insert(KoSvgText::BaselineTextTop, baseline);
                hb_ot_metrics_get_position_with_fallback(
                    font.data(),
                    HB_OT_METRICS_TAG_HORIZONTAL_DESCENDER,
                    &baseline);
                baselineTable.insert(KoSvgText::BaselineTextBottom, baseline);

            } else {
                baselineTable.insert(
                    KoSvgText::BaselineMiddle,
                    baselineTable.value(KoSvgText::BaselineCentral));
                hb_ot_metrics_get_position_with_fallback(
                    font.data(),
                    HB_OT_METRICS_TAG_VERTICAL_ASCENDER,
                    &baseline);
                baselineTable.insert(KoSvgText::BaselineTextTop, baseline);
                hb_ot_metrics_get_position_with_fallback(
                    font.data(),
                    HB_OT_METRICS_TAG_VERTICAL_DESCENDER,
                    &baseline);
                baselineTable.insert(KoSvgText::BaselineTextBottom, baseline);
            }
        } else {
            hb_ot_layout_get_baseline(font.data(),
                                      HB_OT_LAYOUT_BASELINE_TAG_ROMAN,
                                      dir,
                                      script,
                                      HB_TAG_NONE,
                                      &baseline);
            baselineTable.insert(KoSvgText::BaselineAlphabetic, baseline);
            hb_ot_layout_get_baseline(font.data(),
                                      HB_OT_LAYOUT_BASELINE_TAG_MATH,
                                      dir,
                                      script,
                                      HB_TAG_NONE,
                                      &baseline);
            baselineTable.insert(KoSvgText::BaselineMathematical, baseline);
            hb_ot_layout_get_baseline(font.data(),
                                      HB_OT_LAYOUT_BASELINE_TAG_HANGING,
                                      dir,
                                      script,
                                      HB_TAG_NONE,
                                      &baseline);
            baselineTable.insert(KoSvgText::BaselineHanging, baseline);
            hb_ot_layout_get_baseline(
                font.data(),
                HB_OT_LAYOUT_BASELINE_TAG_IDEO_FACE_CENTRAL,
                dir,
                script,
                HB_TAG_NONE,
                &baseline);
            baselineTable.insert(KoSvgText::BaselineCentral, baseline);
            hb_ot_layout_get_baseline(
                font.data(),
                HB_OT_LAYOUT_BASELINE_TAG_IDEO_EMBOX_BOTTOM_OR_LEFT,
                dir,
                script,
                HB_TAG_NONE,
                &baseline);
            baselineTable.insert(KoSvgText::BaselineIdeographic, baseline);
            if (isHorizontal) {
                hb_ot_metrics_get_position(font.data(),
                                           HB_OT_METRICS_TAG_X_HEIGHT,
                                           &baseline);
                baselineTable.insert(
                    KoSvgText::BaselineMiddle,
                    (baseline
                     - baselineTable.value(KoSvgText::BaselineAlphabetic))
                        * 0.5);
                hb_ot_metrics_get_position(
                    font.data(),
                    HB_OT_METRICS_TAG_HORIZONTAL_ASCENDER,
                    &baseline);
                baselineTable.insert(KoSvgText::BaselineTextTop, baseline);
                hb_ot_metrics_get_position(
                    font.data(),
                    HB_OT_METRICS_TAG_HORIZONTAL_DESCENDER,
                    &baseline);
                baselineTable.insert(KoSvgText::BaselineTextBottom, baseline);
            } else {
                baselineTable.insert(
                    KoSvgText::BaselineMiddle,
                    baselineTable.value(KoSvgText::BaselineCentral));
                hb_ot_metrics_get_position(font.data(),
                                           HB_OT_METRICS_TAG_VERTICAL_ASCENDER,
                                           &baseline);
                baselineTable.insert(KoSvgText::BaselineTextTop, baseline);
                hb_ot_metrics_get_position(font.data(),
                                           HB_OT_METRICS_TAG_VERTICAL_DESCENDER,
                                           &baseline);
                baselineTable.insert(KoSvgText::BaselineTextBottom, baseline);
            }
        }
    }

    // Get underline and super/subscripts.
    QPointF newSuperScript;
    QPointF newSubScript;
    if (hb_version_atleast(4, 0, 0)) {
        hb_position_t baseline2 = 0;
        hb_ot_metrics_get_position_with_fallback(
            font.data(),
            HB_OT_METRICS_TAG_SUPERSCRIPT_EM_X_OFFSET,
            &baseline);
        hb_ot_metrics_get_position_with_fallback(
            font.data(),
            HB_OT_METRICS_TAG_SUPERSCRIPT_EM_Y_OFFSET,
            &baseline2);
        newSuperScript = QPointF(baseline * freetypePixelsToPt,
                                 baseline2 * -freetypePixelsToPt);
        hb_ot_metrics_get_position_with_fallback(
            font.data(),
            HB_OT_METRICS_TAG_SUBSCRIPT_EM_X_OFFSET,
            &baseline);
        hb_ot_metrics_get_position_with_fallback(
            font.data(),
            HB_OT_METRICS_TAG_SUBSCRIPT_EM_Y_OFFSET,
            &baseline2);
        newSubScript = QPointF(baseline * freetypePixelsToPt,
                               baseline2 * freetypePixelsToPt);

        qreal width = 0;
        qreal offset = 0;
        hb_ot_metrics_get_position_with_fallback(
            font.data(),
            HB_OT_METRICS_TAG_UNDERLINE_SIZE,
            &baseline);
        width = baseline;
        hb_ot_metrics_get_position_with_fallback(
            font.data(),
            HB_OT_METRICS_TAG_UNDERLINE_OFFSET,
            &baseline);
        offset = baseline;
        offset *= -freetypePixelsToPt;
        width *= -freetypePixelsToPt;

        chunkShape->layoutInterface()->setTextDecorationFontMetrics(
            KoSvgText::DecorationUnderline,
            offset,
            width);
        chunkShape->layoutInterface()->setTextDecorationFontMetrics(
            KoSvgText::DecorationOverline,
            0,
            width);

        hb_ot_metrics_get_position_with_fallback(
            font.data(),
            HB_OT_METRICS_TAG_STRIKEOUT_SIZE,
            &baseline);
        width = baseline;
        hb_ot_metrics_get_position_with_fallback(
            font.data(),
            HB_OT_METRICS_TAG_STRIKEOUT_OFFSET,
            &baseline);
        width *= -freetypePixelsToPt;
        offset *= -freetypePixelsToPt;
        chunkShape->layoutInterface()->setTextDecorationFontMetrics(
            KoSvgText::DecorationLineThrough,
            offset,
            width);
    } else {
        baseline = 0;
        hb_position_t baseline2 = 0;
        hb_ot_metrics_get_position(font.data(),
                                   HB_OT_METRICS_TAG_SUPERSCRIPT_EM_X_OFFSET,
                                   &baseline);
        hb_ot_metrics_get_position(font.data(),
                                   HB_OT_METRICS_TAG_SUPERSCRIPT_EM_Y_OFFSET,
                                   &baseline2);
        if (baseline2 == 0) {
            newSuperScript = QPointF(0, 0.6 * -fontSize);
        } else {
            newSuperScript = QPointF(baseline * freetypePixelsToPt,
                                     baseline2 * -freetypePixelsToPt);
        }
        baseline = 0;
        baseline2 = 0;
        hb_ot_metrics_get_position(font.data(),
                                   HB_OT_METRICS_TAG_SUBSCRIPT_EM_X_OFFSET,
                                   &baseline);
        hb_ot_metrics_get_position(font.data(),
                                   HB_OT_METRICS_TAG_SUBSCRIPT_EM_Y_OFFSET,
                                   &baseline2);
        // Subscript should be 'added' onto the baseline'.
        if (baseline2 == 0) {
            newSubScript = QPointF(0, 0.2 * fontSize);
        } else {
            newSubScript = QPointF(baseline * freetypePixelsToPt,
                                   baseline2 * freetypePixelsToPt);
        }

        qreal width = 0;
        qreal offset = 0;
        const int fallbackThickness = faces.front()->underline_thickness
            * (faces.front()->size->metrics.y_scale / 65535.0);
        hb_ot_metrics_get_position(font.data(),
                                   HB_OT_METRICS_TAG_UNDERLINE_SIZE,
                                   &baseline);
        width = qMax(baseline, fallbackThickness);

        hb_ot_metrics_get_position(font.data(),
                                   HB_OT_METRICS_TAG_UNDERLINE_OFFSET,
                                   &baseline);
        offset = baseline;
        offset *= -freetypePixelsToPt;
        width *= freetypePixelsToPt;

        chunkShape->layoutInterface()->setTextDecorationFontMetrics(
            KoSvgText::DecorationUnderline,
            offset,
            width);
        chunkShape->layoutInterface()->setTextDecorationFontMetrics(
            KoSvgText::DecorationOverline,
            0,
            width);

        hb_ot_metrics_get_position(font.data(),
                                   HB_OT_METRICS_TAG_STRIKEOUT_SIZE,
                                   &baseline);
        width = qMax(baseline, fallbackThickness);
        hb_ot_metrics_get_position(font.data(),
                                   HB_OT_METRICS_TAG_STRIKEOUT_OFFSET,
                                   &baseline);
        if (baseline == 0) {
            offset = baselineTable.value(KoSvgText::BaselineCentral);
        }
        width *= freetypePixelsToPt;
        offset *= -freetypePixelsToPt;

        chunkShape->layoutInterface()->setTextDecorationFontMetrics(
            KoSvgText::DecorationLineThrough,
            offset,
            width);
    }

    Q_FOREACH (KoShape *child, chunkShape->shapes()) {
        computeFontMetrics(child,
                           baselineTable,
                           fontSize,
                           newSuperScript,
                           newSubScript,
                           result,
                           currentIndex,
                           res,
                           isHorizontal);
    }

    KoSvgText::Baseline baselineAdjust = KoSvgText::Baseline(
        properties.property(KoSvgTextProperties::AlignmentBaselineId).toInt());

    if (baselineAdjust == KoSvgText::BaselineDominant) {
        baselineAdjust = dominantBaseline;
    }
    if (baselineAdjust == KoSvgText::BaselineAuto
        || baselineAdjust == KoSvgText::BaselineUseScript) {
        // UseScript got deprecated in CSS-Inline-3.
        if (isHorizontal) {
            baselineAdjust = KoSvgText::BaselineAlphabetic;
        } else {
            baselineAdjust = KoSvgText::BaselineMiddle;
        }
    }

    int offset = parentBaselineTable.value(baselineAdjust, 0)
        - baselineTable.value(baselineAdjust, 0);
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
 * bends them to the textPath, strokes them, and then adds them to the node in
 * question.
 */
void KoSvgTextShape::Private::computeTextDecorations(
    const KoShape *rootShape,
    QVector<CharacterResult> result,
    QMap<int, int> logicalToVisual,
    qreal minimumDecorationThickness,
    KoPathShape *textPath,
    qreal textPathoffset,
    bool side,
    int &currentIndex,
    bool isHorizontal,
    bool ltr,
    bool wrapping)
{
    const KoSvgTextChunkShape *chunkShape =
        dynamic_cast<const KoSvgTextChunkShape *>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);

    int i = currentIndex;
    int j =
        qMin(i + chunkShape->layoutInterface()->numChars(true), result.size());
    using namespace KoSvgText;

    KoPathShape *currentTextPath = nullptr;
    qreal currentTextPathOffset = textPathoffset;
    bool textPathSide = side;
    if (!wrapping) {
        currentTextPath = textPath
            ? textPath
            : dynamic_cast<KoPathShape *>(
                chunkShape->layoutInterface()->textPath());

        if (chunkShape->layoutInterface()->textPath()) {
            textPathSide = chunkShape->layoutInterface()->textOnPathInfo().side
                == TextPathSideRight;
            if (chunkShape->layoutInterface()
                    ->textOnPathInfo()
                    .startOffsetIsPercentage) {
                currentTextPathOffset = currentTextPath->outline().length()
                    * (0.01
                       * chunkShape->layoutInterface()
                             ->textOnPathInfo()
                             .startOffset);
            } else {
                currentTextPathOffset =
                    chunkShape->layoutInterface()->textOnPathInfo().startOffset;
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

    TextDecorations decor =
        chunkShape->textProperties()
            .propertyOrDefault(KoSvgTextProperties::TextDecorationLineId)
            .value<TextDecorations>();
    if (decor != DecorationNone && chunkShape->textProperties().hasProperty(KoSvgTextProperties::TextDecorationLineId)) {
        KoSvgTextProperties properties = chunkShape->textProperties();
        TextDecorationStyle style = TextDecorationStyle(
            properties
                .propertyOrDefault(KoSvgTextProperties::TextDecorationStyleId)
                .toInt());

        QMap<TextDecoration, QPainterPath> decorationPaths;
        QMap<TextDecoration, QPointF> decorationOffsets;

        decorationPaths.insert(DecorationUnderline, QPainterPath());
        decorationPaths.insert(DecorationOverline, QPainterPath());
        decorationPaths.insert(DecorationLineThrough, QPainterPath());

        for (TextDecoration type : decorationPaths.keys()) {
            qreal offset =
                chunkShape->layoutInterface()->getTextDecorationOffset(type);
            decorationOffsets.insert(type,
                                     isHorizontal ? QPointF(0, offset)
                                                  : QPointF(offset, 0));
        }

        QPainterPathStroker stroker;
        stroker.setWidth(
            qMax(minimumDecorationThickness,
                 chunkShape->layoutInterface()->getTextDecorationWidth(
                     DecorationUnderline)));
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

        for (int k = i; k < j; k++) {
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
                QPointF fp = isHorizontal
                    ? QPointF(currentRect.x(), currentFinalPos.y())
                    : QPointF(currentFinalPos.x(), currentRect.y());
                firstPos.append(fp);
                decorationRects.append(currentRect);
                currentRect = QRectF();
            }

            currentFinalPos = charResult.finalPosition;

            QRectF bbox = charResult.path.isEmpty()
                ? charResult.boundingBox
                : charResult.path.boundingRect();

            top =
                isHorizontal ? qMin(top, bbox.top()) : qMax(top, bbox.right());
            bottom = isHorizontal ? qMax(bottom, bbox.bottom())
                                  : qMin(bottom, bbox.left());

            currentRect |= bbox.translated(charResult.finalPosition);
        }
        decorationRects.append(currentRect);
        QPointF fp = isHorizontal
            ? QPointF(currentRect.x(), currentFinalPos.y())
            : QPointF(currentFinalPos.x(), currentRect.y());
        firstPos.append(fp);

        // Computing the various offsets from the 'top' & 'bottom' values.

        bool underlineOverlineFlipped = false;
        if (isHorizontal) {
            decorationOffsets[DecorationOverline] = QPointF(0, top);
            TextDecorationUnderlinePosition underlinePosH =
                TextDecorationUnderlinePosition(
                    properties
                        .propertyOrDefault(
                            KoSvgTextProperties::
                                TextDecorationPositionHorizontalId)
                        .toInt());
            if (underlinePosH == UnderlineUnder) {
                decorationOffsets[DecorationUnderline] = QPointF(0, bottom);
            }
        } else {
            TextDecorationUnderlinePosition underlinePosV =
                TextDecorationUnderlinePosition(
                    properties
                        .propertyOrDefault(KoSvgTextProperties::
                                               TextDecorationPositionVerticalId)
                        .toInt());
            if (underlinePosV == UnderlineRight) {
                decorationOffsets[DecorationOverline] = QPointF(bottom, 0);
                decorationOffsets[DecorationUnderline] = QPointF(top, 0);
                underlineOverlineFlipped = true;
            } else {
                decorationOffsets[DecorationOverline] = QPointF(top, 0);
                decorationOffsets[DecorationUnderline] = QPointF(bottom, 0);
            }
        }
        decorationOffsets[DecorationLineThrough] =
            (decorationOffsets.value(DecorationUnderline)
             + decorationOffsets.value(DecorationOverline))
            * 0.5;

        // Now to create a QPainterPath for the given style that stretches
        // over a single decoration rect,
        // transform that and add it to the general paths.
        for (int i = 0; i < decorationRects.size(); i++) {
            QRectF rect = decorationRects.at(i);
            QPainterPath p;
            QPointF pathWidth;
            if (style != Wavy) {
                p.moveTo(QPointF());
                // We're segmenting the path here so it'll be easier to warp
                // when text-on-path is happening.
                if (currentTextPath) {
                    if (isHorizontal) {
                        int total = floor(rect.width() / (stroker.width() * 2));
                        qreal segment = qreal(rect.width() / total);
                        for (int i = 0; i < total; i++) {
                            p.lineTo(p.currentPosition() + QPointF(segment, 0));
                        }
                    } else {
                        int total =
                            floor(rect.height() / (stroker.width() * 2));
                        qreal segment = qreal(rect.height() / total);
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
                qreal linewidthOffset = qMax(stroker.width() * 1.5, minimumDecorationThickness*2);
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
                        p.setElementPositionAt(i,
                                               p.elementAt(i).y
                                                   - (stroker.width() * 2),
                                               p.elementAt(i).x);
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

            for (TextDecoration type : decorationPaths.keys()) {
                if (decor.testFlag(type)) {
                    QPointF offset = decorationOffsets.value(type);

                    if (currentTextPath) {
                        QPainterPath path = currentTextPath->outline();
                        path = currentTextPath->transformation().map(path);
                        if (textPathSide) {
                            path = path.toReversed();
                        }

                        decorationPaths[type].addPath(stretchGlyphOnPath(
                            p.translated(offset),
                            path,
                            isHorizontal,
                            currentTextPathOffset,
                            currentTextPath->isClosedSubpath(0)));
                    } else {
                        decorationPaths[type].addPath(p.translated(offset));
                    }
                }
            }
        }

        // And finally add the paths to the chunkshape.

        chunkShape->layoutInterface()->clearTextDecorations();

        for (TextDecoration type : decorationPaths.keys()) {
            QPainterPath decorationPath = decorationPaths.value(type);
            if (!decorationPath.isEmpty()) {
                stroker.setWidth(
                    qMax(minimumDecorationThickness,
                         chunkShape->layoutInterface()->getTextDecorationWidth(
                             type)));
                decorationPath =
                    stroker.createStroke(decorationPath).simplified();
                chunkShape->layoutInterface()->addTextDecoration(
                    type,
                    decorationPath.simplified());
            }
        }
    }
    currentIndex = j;
}

void KoSvgTextShape::Private::applyAnchoring(QVector<CharacterResult> &result,
                                             bool isHorizontal)
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
            qreal pos = isHorizontal ? result.at(i).finalPosition.x()
                                     : result.at(i).finalPosition.y();
            qreal advance = isHorizontal ? result.at(i).advance.x()
                                         : result.at(i).advance.y();

            if (result.at(i).anchored_chunk) {
                a = qMin(pos, pos + advance);
                b = qMax(pos, pos + advance);
            } else {
                a = qMin(a, qMin(pos, pos + advance));
                b = qMax(b, qMax(pos, pos + advance));
            }
        }

        bool rtl =
            result.at(start).direction == KoSvgText::DirectionRightToLeft;
        qreal shift = isHorizontal ? result.at(start).finalPosition.x()
                                   : result.at(start).finalPosition.y();

        if ((result.at(start).anchor == KoSvgText::AnchorStart && !rtl)
            || (result.at(start).anchor == KoSvgText::AnchorEnd && rtl)) {
            shift = shift - a;

        } else if ((result.at(start).anchor == KoSvgText::AnchorEnd && !rtl)
                   || (result.at(start).anchor == KoSvgText::AnchorStart
                       && rtl)) {
            shift = shift - b;

        } else {
            shift = shift - (a + b) * 0.5;
        }

        QPointF shiftP = isHorizontal ? QPointF(shift, 0) : QPointF(0, shift);

        for (int j = start; j < i; j++) {
            result[j].finalPosition += shiftP;
        }
        start = i;
    }
}

qreal KoSvgTextShape::Private::characterResultOnPath(CharacterResult &cr,
                                                     qreal length,
                                                     qreal offset,
                                                     bool isHorizontal,
                                                     bool isClosed)
{
    bool rtl = (cr.direction == KoSvgText::DirectionRightToLeft);
    qreal mid = cr.finalPosition.x() + (cr.advance.x() * 0.5) + offset;
    if (!isHorizontal) {
        mid = cr.finalPosition.y() + (cr.advance.y() * 0.5) + offset;
    }
    if (isClosed) {
        if ((cr.anchor == KoSvgText::AnchorStart && !rtl)
            || (cr.anchor == KoSvgText::AnchorEnd && rtl)) {
            if (mid - offset < 0 || mid - offset > length) {
                cr.hidden = true;
            }
        } else if ((cr.anchor == KoSvgText::AnchorEnd && !rtl)
                   || (cr.anchor == KoSvgText::AnchorStart && rtl)) {
            if (mid - offset < -length || mid - offset > 0) {
                cr.hidden = true;
            }
        } else {
            if (mid - offset < -(length * 0.5)
                || mid - offset > (length * 0.5)) {
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

QPainterPath KoSvgTextShape::Private::stretchGlyphOnPath(QPainterPath glyph,
                                                         QPainterPath path,
                                                         bool isHorizontal,
                                                         qreal offset,
                                                         bool isClosed)
{
    QPainterPath p = glyph;
    for (int i = 0; i < glyph.elementCount(); i++) {
        qreal mid = isHorizontal ? glyph.elementAt(i).x + offset
                                 : glyph.elementAt(i).y + offset;
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
        qreal percent = path.percentAtLength(mid);
        QPointF pos = path.pointAtPercent(percent);
        qreal tAngle = path.angleAtPercent(percent);
        if (tAngle > 180) {
            tAngle = 0 - (360 - tAngle);
        }
        QPointF vectorT(qCos(qDegreesToRadians(tAngle)),
                        -qSin(qDegreesToRadians(tAngle)));
        QPointF finalPos = pos;
        if (isHorizontal) {
            QPointF vectorN(-vectorT.y(), vectorT.x());
            qreal o = mid - (midUnbound);
            finalPos = pos - (o * vectorT) + (glyph.elementAt(i).y * vectorN);
        } else {
            QPointF vectorN(vectorT.y(), -vectorT.x());
            qreal o = mid - (midUnbound);
            finalPos = pos - (o * vectorT) + (glyph.elementAt(i).x * vectorN);
        }
        p.setElementPositionAt(i, finalPos.x(), finalPos.y());
    }
    return p;
}

void KoSvgTextShape::Private::applyTextPath(const KoShape *rootShape,
                                            QVector<CharacterResult> &result,
                                            bool isHorizontal)
{
    // Unlike all the other applying functions, this one only iterrates over the
    // top-level. SVG is not designed to have nested textPaths. Source:
    // https://github.com/w3c/svgwg/issues/580
    const KoSvgTextChunkShape *chunkShape =
        dynamic_cast<const KoSvgTextChunkShape *>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);
    bool inPath = false;
    bool afterPath = false;
    int currentIndex = 0;
    QPointF pathEnd;
    Q_FOREACH (KoShape *child, chunkShape->shapes()) {
        const KoSvgTextChunkShape *textPathChunk =
            dynamic_cast<const KoSvgTextChunkShape *>(child);
        KIS_SAFE_ASSERT_RECOVER_RETURN(textPathChunk);
        int endIndex =
            currentIndex + textPathChunk->layoutInterface()->numChars(true);

        KoPathShape *shape = dynamic_cast<KoPathShape *>(
            textPathChunk->layoutInterface()->textPath());
        if (shape) {
            QPainterPath path = shape->outline();
            path = shape->transformation().map(path);
            inPath = true;
            if (textPathChunk->layoutInterface()->textOnPathInfo().side
                == KoSvgText::TextPathSideRight) {
                path = path.toReversed();
            }
            qreal length = path.length();
            qreal offset = 0.0;
            bool isClosed =
                (shape->isClosedSubpath(0) && shape->subpathCount() == 1);
            if (textPathChunk->layoutInterface()
                    ->textOnPathInfo()
                    .startOffsetIsPercentage) {
                offset = length
                    * (0.01
                       * textPathChunk->layoutInterface()
                             ->textOnPathInfo()
                             .startOffset);
            } else {
                offset = textPathChunk->layoutInterface()
                             ->textOnPathInfo()
                             .startOffset;
            }
            bool stretch =
                textPathChunk->layoutInterface()->textOnPathInfo().method
                == KoSvgText::TextPathStretch;

            for (int i = currentIndex; i < endIndex; i++) {
                CharacterResult cr = result[i];

                if (cr.middle == false) {
                    qreal mid = characterResultOnPath(cr,
                                                      length,
                                                      offset,
                                                      isHorizontal,
                                                      isClosed);
                    if (!cr.hidden) {
                        if (stretch && !cr.path.isEmpty()) {
                            QTransform tf =
                                QTransform::fromTranslate(cr.finalPosition.x(),
                                                          cr.finalPosition.y());
                            tf.rotateRadians(cr.rotate);
                            QPainterPath glyph =
                                stretchGlyphOnPath(tf.map(cr.path),
                                                   path,
                                                   isHorizontal,
                                                   offset,
                                                   isClosed);
                            cr.path = glyph;
                        }
                        qreal percent = path.percentAtLength(mid);
                        QPointF pos = path.pointAtPercent(percent);
                        qreal tAngle = path.angleAtPercent(percent);
                        if (tAngle > 180) {
                            tAngle = 0 - (360 - tAngle);
                        }
                        QPointF vectorT(qCos(qDegreesToRadians(tAngle)),
                                        -qSin(qDegreesToRadians(tAngle)));
                        if (isHorizontal) {
                            cr.rotate -= qDegreesToRadians(tAngle);
                            QPointF vectorN(-vectorT.y(), vectorT.x());
                            qreal o = (cr.advance.x() * 0.5);
                            cr.finalPosition = pos - (o * vectorT)
                                + (cr.finalPosition.y() * vectorN);
                        } else {
                            cr.rotate -= qDegreesToRadians(tAngle + 90);
                            QPointF vectorN(vectorT.y(), -vectorT.x());
                            qreal o = (cr.advance.y() * 0.5);
                            cr.finalPosition = pos - (o * vectorT)
                                + (cr.finalPosition.x() * vectorN);
                        }
                        if (stretch && !cr.path.isEmpty()) {
                            QTransform tf =
                                QTransform::fromTranslate(cr.finalPosition.x(),
                                                          cr.finalPosition.y());
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

void KoSvgTextShape::Private::paintPaths(QPainter &painter,
                                         QPainterPath outlineRect,
                                         const KoShape *rootShape,
                                         QVector<CharacterResult> &result,
                                         QPainterPath &chunk,
                                         int &currentIndex)
{
    const KoSvgTextChunkShape *chunkShape =
        dynamic_cast<const KoSvgTextChunkShape *>(rootShape);
    KIS_SAFE_ASSERT_RECOVER_RETURN(chunkShape);
    QMap<KoSvgText::TextDecoration, QPainterPath> textDecorations =
        chunkShape->layoutInterface()->textDecorations();
    QColor textDecorationColor =
        chunkShape->textProperties()
            .propertyOrDefault(KoSvgTextProperties::TextDecorationColorId)
            .value<QColor>();

    if (textDecorations.contains(KoSvgText::DecorationUnderline)) {
        if (chunkShape->background() && !textDecorationColor.isValid() && textDecorationColor != Qt::transparent) {

            chunkShape->background()->paint(
                painter,
                textDecorations.value(KoSvgText::DecorationUnderline));
        } else if (textDecorationColor.isValid()) {
            painter.fillPath(
                textDecorations.value(KoSvgText::DecorationUnderline),
                textDecorationColor);
        }
        if (chunkShape->stroke()) {
            QScopedPointer<KoShape> shape(
                KoPathShape::createShapeFromPainterPath(
                    textDecorations.value(KoSvgText::DecorationUnderline)));
            chunkShape->stroke()->paint(shape.data(), painter);
        }
    }
    if (textDecorations.contains(KoSvgText::DecorationOverline)) {
        if (chunkShape->background() && !textDecorationColor.isValid()) {
            chunkShape->background()->paint(
                painter,
                textDecorations.value(KoSvgText::DecorationOverline));
        } else if (textDecorationColor.isValid()) {
            painter.fillPath(
                textDecorations.value(KoSvgText::DecorationOverline),
                textDecorationColor);
        }
        if (chunkShape->stroke()) {
            QScopedPointer<KoShape> shape(
                KoPathShape::createShapeFromPainterPath(
                    textDecorations.value(KoSvgText::DecorationOverline)));
            chunkShape->stroke()->paint(shape.data(), painter);
        }
    }

    if (chunkShape->isTextNode()) {
        QTransform tf;
        int j = currentIndex + chunkShape->layoutInterface()->numChars(true);
        KoClipMaskPainter fillPainter(
            &painter,
            painter.transform().mapRect(outlineRect.boundingRect()));
        if (chunkShape->background()) {
            chunkShape->background()->paint(*fillPainter.shapePainter(),
                                            outlineRect);
            fillPainter.maskPainter()->fillPath(outlineRect, Qt::black);
            if (textRendering != OptimizeSpeed) {
                fillPainter.maskPainter()->setRenderHint(QPainter::Antialiasing,
                                                         true);
                fillPainter.maskPainter()->setRenderHint(
                    QPainter::SmoothPixmapTransform,
                    true);
            } else {
                fillPainter.maskPainter()->setRenderHint(QPainter::Antialiasing,
                                                         false);
                fillPainter.maskPainter()->setRenderHint(
                    QPainter::SmoothPixmapTransform,
                    false);
            }
        }
        QPainterPath textDecorationsRest;
        textDecorationsRest.setFillRule(Qt::WindingFill);

        for (int i = currentIndex; i < j; i++) {
            if (result.at(i).addressable && result.at(i).hidden == false) {
                tf.reset();
                tf.translate(result.at(i).finalPosition.x(),
                             result.at(i).finalPosition.y());
                tf.rotateRadians(result.at(i).rotate);
                /* Debug
                painter.save();
                painter.setBrush(Qt::transparent);
                QPen pen (QColor(0, 0, 0, 50));
                painter.setPen(pen);
                painter.drawPolygon(tf.map(result.at(i).path.boundingRect()));
                QColor penColor = result.at(i).anchored_chunk?
                                         result.at(i).isHanging? Qt::red:
                Qt::magenta: result.at(i).lineEnd==NoChange? Qt::cyan:
                Qt::yellow; pen.setColor(penColor); pen.setWidthF(72./xRes);
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
                // if (chunk.intersects(p)) {
                //     chunk |= tf.map(result.at(i).path);
                // } else {
                if (result.at(i).colorLayers.size()) {
                    for (int c = 0; c < result.at(i).colorLayers.size(); c++) {
                        QBrush color = result.at(i).colorLayerColors.at(c);
                        bool replace =
                            result.at(i).replaceWithForeGroundColor.at(c);
                        // In theory we can use the pattern or gradient as well
                        // for ColorV0 fonts, but ColorV1 fonts can have
                        // gradients, so I am hesitant.
                        KoColorBackground *b =
                            dynamic_cast<KoColorBackground *>(
                                chunkShape->background().data());
                        if (b && replace) {
                            color = b->brush();
                        }
                        painter.fillPath(tf.map(result.at(i).colorLayers.at(c)),
                                         color);
                    }
                } else {
                    chunk.addPath(p);
                }
                //}
                if (p.isEmpty() && !result.at(i).image.isNull()) {
                    if (result.at(i).image.isGrayscale()
                        || result.at(i).image.format() == QImage::Format_Mono) {
                        fillPainter.maskPainter()->save();
                        fillPainter.maskPainter()->translate(
                            result.at(i).finalPosition.x(),
                            result.at(i).finalPosition.y());
                        fillPainter.maskPainter()->rotate(
                            qRadiansToDegrees(result.at(i).rotate));
                        fillPainter.maskPainter()->setCompositionMode(
                            QPainter::CompositionMode_Plus);
                        fillPainter.maskPainter()->drawImage(
                            result.at(i).boundingBox,
                            result.at(i).image);
                        fillPainter.maskPainter()->restore();
                    } else {
                        painter.save();
                        painter.translate(result.at(i).finalPosition.x(),
                                          result.at(i).finalPosition.y());
                        painter.rotate(qRadiansToDegrees(result.at(i).rotate));
                        painter.setRenderHint(QPainter::SmoothPixmapTransform,
                                              true);
                        painter.drawImage(result.at(i).boundingBox,
                                          result.at(i).image);
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
            fillPainter.maskPainter()->fillPath(
                textDecorationsRest.simplified(),
                Qt::white);
        }
        fillPainter.renderOnGlobalPainter();
        KoShapeStrokeSP maskStroke;
        if (chunkShape->stroke()) {
            KoShapeStrokeSP stroke =
                qSharedPointerDynamicCast<KoShapeStroke>(chunkShape->stroke());

            if (stroke) {
                if (stroke->lineBrush().gradient()) {
                    KoClipMaskPainter strokePainter(
                        &painter,
                        painter.transform().mapRect(
                            outlineRect.boundingRect()));
                    strokePainter.shapePainter()->fillRect(
                        outlineRect.boundingRect(),
                        stroke->lineBrush());
                    maskStroke =
                        KoShapeStrokeSP(new KoShapeStroke(*stroke.data()));
                    maskStroke->setColor(Qt::white);
                    maskStroke->setLineBrush(Qt::white);
                    strokePainter.maskPainter()->fillPath(outlineRect,
                                                          Qt::black);
                    if (textRendering != OptimizeSpeed) {
                        strokePainter.maskPainter()->setRenderHint(
                            QPainter::Antialiasing,
                            true);
                    } else {
                        strokePainter.maskPainter()->setRenderHint(
                            QPainter::Antialiasing,
                            false);
                    }
                    {
                        QScopedPointer<KoShape> shape(
                            KoPathShape::createShapeFromPainterPath(chunk));
                        maskStroke->paint(shape.data(),
                                          *strokePainter.maskPainter());
                    }
                    if (!textDecorationsRest.isEmpty()) {
                        QScopedPointer<KoShape> shape(
                            KoPathShape::createShapeFromPainterPath(
                                textDecorationsRest));
                        maskStroke->paint(shape.data(),
                                          *strokePainter.maskPainter());
                    }
                    strokePainter.renderOnGlobalPainter();
                } else {
                    {
                        QScopedPointer<KoShape> shape(
                            KoPathShape::createShapeFromPainterPath(chunk));
                        stroke->paint(shape.data(), painter);
                    }
                    if (!textDecorationsRest.isEmpty()) {
                        QScopedPointer<KoShape> shape(
                            KoPathShape::createShapeFromPainterPath(
                                textDecorationsRest));
                        stroke->paint(shape.data(), painter);
                    }
                }
            }
        }
        chunk = QPainterPath();
        currentIndex = j;

    } else {
        Q_FOREACH (KoShape *child, chunkShape->shapes()) {
            paintPaths(painter,
                       outlineRect,
                       child,
                       result,
                       chunk,
                       currentIndex);
        }
    }
    if (textDecorations.contains(KoSvgText::DecorationLineThrough)) {
        if (chunkShape->background() && !textDecorationColor.isValid() && textDecorationColor != Qt::transparent) {
            chunkShape->background()->paint(
                painter,
                textDecorations.value(KoSvgText::DecorationLineThrough));
        } else if (textDecorationColor.isValid()) {
            painter.fillPath(
                textDecorations.value(KoSvgText::DecorationLineThrough),
                textDecorationColor);
        }
        if (chunkShape->stroke()) {
            QScopedPointer<KoShape> shape(
                KoPathShape::createShapeFromPainterPath(
                    textDecorations.value(KoSvgText::DecorationLineThrough)));
            chunkShape->stroke()->paint(shape.data(), painter);
        }
    }
}

QList<KoShape *>
KoSvgTextShape::Private::collectPaths(const KoShape *rootShape,
                                      QVector<CharacterResult> &result,
                                      int &currentIndex)
{
    const KoSvgTextChunkShape *chunkShape =
        dynamic_cast<const KoSvgTextChunkShape *>(rootShape);

    QMap<KoSvgText::TextDecoration, QPainterPath> textDecorations =
        chunkShape->layoutInterface()->textDecorations();
    QColor textDecorationColor =
        chunkShape->textProperties()
            .propertyOrDefault(KoSvgTextProperties::TextDecorationColorId)
            .value<QColor>();
    QSharedPointer<KoShapeBackground> decorationColor =
        chunkShape->background();
    if (textDecorationColor.isValid()) {
        decorationColor = QSharedPointer<KoColorBackground>(
            new KoColorBackground(textDecorationColor));
    }

    QList<KoShape *> shapes;
    if (textDecorations.contains(KoSvgText::DecorationUnderline)) {
        KoPathShape *shape = KoPathShape::createShapeFromPainterPath(
            textDecorations.value(KoSvgText::DecorationUnderline));
        shape->setBackground(decorationColor);
        shape->setStroke(chunkShape->stroke());
        shape->setZIndex(chunkShape->zIndex());
        shape->setFillRule(Qt::WindingFill);
        shapes.append(shape);
    }
    if (textDecorations.contains(KoSvgText::DecorationOverline)) {
        KoPathShape *shape = KoPathShape::createShapeFromPainterPath(
            textDecorations.value(KoSvgText::DecorationOverline));
        shape->setBackground(decorationColor);
        shape->setStroke(chunkShape->stroke());
        shape->setZIndex(chunkShape->zIndex());
        shape->setFillRule(Qt::WindingFill);
        shapes.append(shape);
    }

    if (chunkShape->isTextNode()) {
        QPainterPath chunk;

        QTransform tf;
        int j = currentIndex + chunkShape->layoutInterface()->numChars(true);
        for (int i = currentIndex; i < j; i++) {
            if (result.at(i).addressable && result.at(i).hidden == false) {
                tf.reset();
                tf.translate(result.at(i).finalPosition.x(),
                             result.at(i).finalPosition.y());
                tf.rotateRadians(result.at(i).rotate);
                QPainterPath p = tf.map(result.at(i).path);
                if (result.at(i).colorLayers.size()) {
                    for (int c = 0; c < result.at(i).colorLayers.size(); c++) {
                        QBrush color = result.at(i).colorLayerColors.at(c);
                        bool replace =
                            result.at(i).replaceWithForeGroundColor.at(c);
                        // In theory we can use the pattern or gradient as well
                        // for ColorV0 fonts, but ColorV1 fonts can have
                        // gradients, so I am hesitant.
                        KoColorBackground *b =
                            dynamic_cast<KoColorBackground *>(
                                chunkShape->background().data());
                        if (b && replace) {
                            color = b->brush();
                        }
                        KoPathShape *shape =
                            KoPathShape::createShapeFromPainterPath(
                                tf.map(result.at(i).colorLayers.at(c)));
                        shape->setBackground(QSharedPointer<KoColorBackground>(
                            new KoColorBackground(color.color())));
                        shape->setZIndex(chunkShape->zIndex());
                        shape->setFillRule(Qt::WindingFill);
                        shapes.append(shape);
                    }
                } else {
                    chunk.addPath(p);
                }
            }
        }
        KoPathShape *shape = KoPathShape::createShapeFromPainterPath(chunk);
        shape->setBackground(chunkShape->background());
        shape->setStroke(chunkShape->stroke());
        shape->setZIndex(chunkShape->zIndex());
        shape->setFillRule(Qt::WindingFill);
        shapes.append(shape);
        currentIndex = j;

    } else {
        Q_FOREACH (KoShape *child, chunkShape->shapes()) {
            shapes.append(collectPaths(child, result, currentIndex));
        }
    }
    if (textDecorations.contains(KoSvgText::DecorationLineThrough)) {
        KoPathShape *shape = KoPathShape::createShapeFromPainterPath(
            textDecorations.value(KoSvgText::DecorationLineThrough));
        shape->setBackground(decorationColor);
        shape->setStroke(chunkShape->stroke());
        shape->setZIndex(chunkShape->zIndex());
        shape->setFillRule(Qt::WindingFill);
        shapes.append(shape);
    }
    return shapes;
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

    QString svgText = params->stringProperty("svgText", i18nc("Default text for the text shape", "<text>Placeholder Text</text>"));
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
