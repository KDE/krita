/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *  SPDX-FileCopyrightText: 2022 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KO_SVG_TEXT_SHAPE_P_H
#define KO_SVG_TEXT_SHAPE_P_H

#include "KoSvgTextShape.h"

#include "KoSvgText.h"
#include "KoSvgTextContentElement.h"

#include <kis_assert.h>
#include <KisForest.h>

#include <QFont>
#include <QImage>
#include <QLineF>
#include <QPainterPath>
#include <QPointF>
#include <QRectF>
#include <QVector>
#include <QtMath>

#include <variant>

#include <ft2build.h>
#include FT_FREETYPE_H

constexpr qreal SHAPE_PRECISION = 1e-6; ///< Value that indcates the precision for testing coordinates for text-in-shape layout.

class KoPathShape;
struct raqm_glyph_t;

enum class BreakType {
    NoBreak,
    SoftBreak,
    HardBreak
};

enum class LineEdgeBehaviour {
    NoChange, ///< Do nothing special.
    Collapse, ///< Collapse if first or last in line.
    ForceHang, ///< Force hanging at the start or end of a line, never measured for justfication.
    ConditionallyHang ///< Only hang if no space otherwise, only measured for justification if not hanging.
};

struct CursorInfo {
    QLineF caret; ///< Caret for this characterResult
    QVector<int> graphemeIndices; ///< The text-string indices of graphemes starting here, starting grapheme is not present.
    QVector<QPointF> offsets; ///< The advance offsets for each grapheme index.
    bool rtl = false; ///< Whether the current glyph is right-to-left, as opposed to the markup.
    bool isWordBoundary = false;
    QColor color; ///< Which color the current position has.
};

struct CursorPos {
    int cluster; ///< Which character result this position belongs in.
    int index; ///< Which grapheme this position belongs with.
    int offset; ///< Which offset this position belongs with.
    bool synthetic = false; ///< Whether this position was inserted to have a visual indicator.
};

namespace Glyph {

struct Outline {
    QPainterPath path;
};

struct Bitmap {
    QVector<QImage> images;
    QVector<QRectF> drawRects;
};

struct ColorLayers {
    QVector<QPainterPath> paths;
    QVector<QBrush> colors;
    QVector<bool> replaceWithForeGroundColor;
};

using Variant = std::variant<std::monostate, Outline, Bitmap, ColorLayers>;

} // namespace Glyph

struct CharacterResult {
    QPointF finalPosition; ///< the final position, taking into account both CSS and SVG positioning considerations.
    qreal rotate = 0.0;
    bool hidden = false; // whether the character will be drawn.
    // The original svg specs' notion of addressable character relies on utf16,
    // but there's an issue to switch to unicode codepoints proper (that is, utf 32), though it was never applied.
    // https://github.com/w3c/svgwg/issues/537
    bool addressable = true; ///< whether the character is not discarded for various reasons,
                             ///< this is necessary for determining to which index a given x/y transform is applied,
                             ///< and in this code we also use it to determine if a character is considered for line-breaking.
    bool middle = false; ///< whether the character is the second of last of a
                         ///< typographic character.
    bool anchored_chunk = false; ///< whether this is the start of a new chunk.

    Glyph::Variant glyph;

    QRectF inkBoundingBox; ///< The bounds of the drawn glyph. Different from the bounds the charresult takes up in the layout, @see layoutBox();
    bool isHorizontal = true; ///< Whether the current glyph lays out horizontal or vertical. Currently same as paragraph, but in future may change.
    int visualIndex = -1;
    int plaintTextIndex = -1;
    QPointF cssPosition = QPointF(); ///< the position in accordance with the CSS specs, as opossed to the SVG spec.
    QPointF dominantBaselineOffset = QPointF(); // Shift caused by aligning glyphs to dominant baseline.
    QPointF baselineOffset = QPointF(); ///< The computed baseline offset, will be applied
                                        ///< when calculating the line-offset during line breaking.
    QPointF advance;
    BreakType breakType = BreakType::NoBreak;
    LineEdgeBehaviour lineEnd = LineEdgeBehaviour::NoChange;
    LineEdgeBehaviour lineStart = LineEdgeBehaviour::NoChange;
    bool justifyBefore = false;///< Justification Opportunity precedes this character.
    bool justifyAfter = false; ///< Justification Opportunity follows this character.
    bool isHanging = false;
    bool textLengthApplied = false;
    bool overflowWrap = false;

    qreal extraFontScaling = 1.0; ///< Freetype doesn't allow us to scale below 1pt, so we need to do an extra transformation in these cases.
    qreal fontHalfLeading; ///< Leading for both sides, can be either negative or positive.
    KoSvgText::FontMetrics metrics; ///< Fontmetrics for current font, in Freetype scanline coordinates.
    qreal scaledHalfLeading{}; ///< Leading for both sides, can be either negative or positive, in pt
    qreal scaledAscent{}; ///< Ascender, in pt
    qreal scaledDescent{}; ///< Descender, in pt

    std::optional<qreal> tabSize; ///< If present, this is a tab and it should align to multiples of this tabSize value.

    void calculateAndApplyTabsize(QPointF currentPos, bool isHorizontal, const KoSvgText::ResolutionHandler &resHandler) {
        if (!tabSize) return;
        if (*tabSize == qInf() || qIsNaN(*tabSize)) return;

        if (*tabSize > 0) {
            qreal remainder = *tabSize - (isHorizontal? fmod(currentPos.x(), *tabSize): fmod(currentPos.y(), *tabSize));
            advance = resHandler.adjust(isHorizontal? QPointF(remainder, advance.y()): QPointF(advance.x(), remainder));
        }
    }

    /**
     * @brief layoutBox
     * @return a dynamically calculated layoutBox, this is different from the Ink bounding box.
     */
    QRectF layoutBox() const {
        return isHorizontal? QRectF(0, advance.y()+scaledAscent, advance.x(), scaledDescent-scaledAscent)
                         : QRectF(advance.x()+scaledDescent, 0, scaledAscent-scaledDescent, advance.y());
    }
    /**
     * @brief lineHeightBox
     * @return The box representing the line height of this char.
     */
    QRectF lineHeightBox () const {
        QRectF lBox = layoutBox();
        return isHorizontal? lBox.adjusted(0, -scaledHalfLeading, 0, scaledHalfLeading)
                         : lBox.adjusted(-scaledHalfLeading, 0, scaledHalfLeading, 0);
    }

    /**
     * @brief translateOrigin
     * For dominant baseline, we want to move the glyph origin.
     * This encompassed the glyph, the ascent and descent, and the metrics.
     */
    void translateOrigin(QPointF newOrigin) {
        if (newOrigin == QPointF()) return;
        if (Glyph::Outline *outlineGlyph = std::get_if<Glyph::Outline>(&glyph)) {
            outlineGlyph->path.translate(-newOrigin);
        } else if (Glyph::Bitmap *bitmapGlyph = std::get_if<Glyph::Bitmap>(&glyph)) {
            for (int i = 0; i< bitmapGlyph->drawRects.size(); i++) {
                bitmapGlyph->drawRects[i].translate(-newOrigin);
            }
        } else if  (Glyph::ColorLayers *colorGlyph = std::get_if<Glyph::ColorLayers>(&glyph)) {
            for (int i = 0; i< colorGlyph->paths.size(); i++) {
                colorGlyph->paths[i].translate(-newOrigin);
            }
        }
        cursorInfo.caret.translate(-newOrigin);
        inkBoundingBox.translate(-newOrigin);

        if (isHorizontal) {
            scaledDescent -= newOrigin.y();
            scaledAscent -= newOrigin.y();
        } else {
            scaledDescent -= newOrigin.x();
            scaledAscent -= newOrigin.x();
        }
    }

    /**
     * @brief scaleCharacterResult
     * convenience function to scale the whole character result.
     * @param xScale -- the factor by which the width should be scaled.
     * @param yScale -- the factor by which the height should be scaled.
     */
    void scaleCharacterResult(qreal xScale, qreal yScale) {
        QTransform scale = QTransform::fromScale(xScale, yScale);
        if (scale.isIdentity()) return;
        const bool scaleToZero = !(xScale > 0 && yScale > 0);

        if (Glyph::Outline *outlineGlyph = std::get_if<Glyph::Outline>(&glyph)) {
            if (!outlineGlyph->path.isEmpty()) {
                if (scaleToZero) {
                    outlineGlyph->path = QPainterPath();
                } else {
                    outlineGlyph->path = scale.map(outlineGlyph->path);
                }
            }
        } else if (Glyph::Bitmap *bitmapGlyph = std::get_if<Glyph::Bitmap>(&glyph)) {
            if (scaleToZero) {
                bitmapGlyph->drawRects.clear();
                bitmapGlyph->images.clear();
            } else {
                for (int i = 0; i< bitmapGlyph->drawRects.size(); i++) {
                    bitmapGlyph->drawRects[i] = scale.mapRect(bitmapGlyph->drawRects[i]);
                }
            }
        } else if  (Glyph::ColorLayers *colorGlyph = std::get_if<Glyph::ColorLayers>(&glyph)) {
            for (int i = 0; i< colorGlyph->paths.size(); i++) {
                if (scaleToZero) {
                    colorGlyph->paths[i] = QPainterPath();
                } else {
                    colorGlyph->paths[i] = scale.map(colorGlyph->paths[i]);
                }
            }
        }
        advance = scale.map(advance);
        cursorInfo.caret = scale.map(cursorInfo.caret);
        for (int i = 0; i < cursorInfo.offsets.size(); i++) {
            cursorInfo.offsets[i] = scale.map(cursorInfo.offsets.at(i));
        }
        inkBoundingBox = scale.mapRect(inkBoundingBox);

        if (isHorizontal) {
            scaledDescent *= yScale;
            scaledAscent *= yScale;
            scaledHalfLeading *= yScale;
            metrics.scaleBaselines(yScale);
            if (tabSize) {
                tabSize = scale.map(QPointF(*tabSize, *tabSize)).x();
            }
        } else {
            scaledDescent *= xScale;
            scaledAscent *= xScale;
            scaledHalfLeading *= xScale;
            metrics.scaleBaselines(xScale);
            if (tabSize) {
                tabSize = scale.map(QPointF(*tabSize, *tabSize)).y();
            }
        }
    }

    QPointF totalBaselineOffset() const {
        return baselineOffset+dominantBaselineOffset;
    }

    QFont::Style fontStyle = QFont::StyleNormal;
    int fontWeight = 400;

    CursorInfo cursorInfo;

    KoSvgText::TextAnchor anchor = KoSvgText::AnchorStart;
    KoSvgText::Direction direction = KoSvgText::DirectionLeftToRight;

    QTransform finalTransform() const {
        QTransform tf =
            QTransform::fromTranslate(finalPosition.x(), finalPosition.y());
        tf.rotateRadians(rotate);
        return tf;
    }
};

struct LineChunk {
    QLineF length; ///< Used to measure how long the current line is allowed to be.
    QVector<int> chunkIndices; ///< charResult indices that belong to this chunk.
    QRectF boundingBox;
    QPointF conditionalHangEnd = QPointF();
};

/**
 * @brief The LineBox struct
 *
 * The line box struct is to simplify keeping track of lines inside the wrapping
 * functions. It somewhat corresponds to CSS line boxes, with the caveat that formally,
 * a line split in two in CSS/SVG would be two line boxes, while we instead have two
 * line chunks in a single line box. This is necessary to ensure we can calculate the
 * same line height for boxes split by a shape.
 *
 * CSS-Inline-3 defines Line Boxes here: https://www.w3.org/TR/css-inline-3/#line-box
 * CSS-Text-3 briefly talks about them here: https://www.w3.org/TR/css-text-3/#bidi-linebox
 * SVG-2 chapter text talks about them here: https://svgwg.org/svg2-draft/text.html#TextLayoutAutoNotes
 *
 * What is important to us is that all the above specifications, when they talk about Bidi-reordering,
 * agree that the order is dependant on the paragraph/block level direction, and is not affected by
 * the inline content changing direction. Which is good, because that'd make things super hard.
 */
struct LineBox {

    LineBox() {
    }

    LineBox(QPointF start, QPointF end, const KoSvgText::ResolutionHandler &resHandler) {
        LineChunk chunk;
        chunk.length =  QLineF(resHandler.adjustCeil(start), resHandler.adjustFloor(end));
        chunks.append(chunk);
        currentChunk = 0;
    }

    LineBox(QVector<QLineF> lineWidths, bool ltr, QPointF indent, const KoSvgText::ResolutionHandler &resHandler) {
        textIndent = indent;
        if (ltr) {
            Q_FOREACH(QLineF line, lineWidths) {
                LineChunk chunk;
                chunk.length = QLineF(resHandler.adjustCeil(line.p1()), resHandler.adjustFloor(line.p2()));
                chunks.append(chunk);
                currentChunk = 0;
            }
        } else {
            Q_FOREACH(QLineF line, lineWidths) {
                LineChunk chunk;
                chunk.length = QLineF(resHandler.adjustFloor(line.p2()), resHandler.adjustCeil(line.p1()));
                chunks.insert(0, chunk);
                currentChunk = 0;
            }
        }
    }

    QVector<LineChunk> chunks;
    int currentChunk = -1;

    qreal expectedLineTop = 0; ///< Because fonts can affect lineheight mid-line, and this affects wrapping, this estimates the line-height.
    qreal actualLineTop = 0;
    qreal actualLineBottom = 0;

    QPointF baselineTop = QPointF(); ///< Used to identify the top of the line for baseline-alignment.
    QPointF baselineBottom = QPointF(); ///< Used to identify the bottom of the line for baseline-alignment.

    QPointF textIndent = QPointF();
    bool firstLine = false;
    bool lastLine = false;
    bool lineFinalized = false;
    bool justifyLine = false;

    LineChunk chunk() {
        return chunks.value(currentChunk);
    }

    void setCurrentChunk(LineChunk chunk) {
        currentChunk = qMax(currentChunk, 0);
        if (currentChunk < chunks.size()) {
            chunks[currentChunk] = chunk;
        } else {
            chunks.append(chunk);
        }
    }

    void clearAndAdjust(bool isHorizontal, QPointF current, QPointF indent) {
        actualLineBottom = 0;
        actualLineTop = 0;
        LineChunk chunk;
        textIndent = indent;
        QLineF length = chunks.at(currentChunk).length;
        if (isHorizontal) {
            length.setP1(QPointF(length.p1().x(), current.y()));
            length.setP2(QPointF(length.p2().x(), current.y()));
        } else {
            length.setP1(QPointF(current.x(), length.p1().y()));
            length.setP2(QPointF(current.x(), length.p2().y()));
        }
        chunks.clear();
        currentChunk = 0;
        chunk.length = length;
        chunks.append(chunk);
        firstLine = false;
    }

    void setCurrentChunkForPos(QPointF pos, bool isHorizontal) {
        for (int i=0; i<chunks.size(); i++) {
            LineChunk chunk = chunks.at(i);
            if (isHorizontal) {
                qreal min = qMin(chunk.length.p1().x(), chunk.length.p2().x()) - SHAPE_PRECISION;
                qreal max = qMax(chunk.length.p1().x(), chunk.length.p2().x()) + SHAPE_PRECISION;
                if ((pos.x() < max) &&
                        (pos.x() >= min)) {
                        currentChunk = i;
                        break;
                }
            } else {
                qreal min = qMin(chunk.length.p1().y(), chunk.length.p2().y()) - SHAPE_PRECISION;
                qreal max = qMax(chunk.length.p1().y(), chunk.length.p2().y()) + SHAPE_PRECISION;
                if ((pos.y() < max) &&
                        (pos.y() >= min)) {
                    currentChunk = i;
                    break;
                }
            }
        }
    }

    bool isEmpty() {
        if (chunks.isEmpty()) return true;
        for (int i =0; i < chunks.size(); i++) {
            if (!chunks.at(i).chunkIndices.isEmpty()) return false;
        }
        return true;
    }

};

/**
 * A representation of a single leaf of the KisForest<KoTextContentElement>
 **/
struct SubChunk {

    SubChunk(KisForest<KoSvgTextContentElement>::child_iterator leaf):
        associatedLeaf(leaf){

    }

    QString text;
    QString originalText;
    KisForest<KoSvgTextContentElement>::child_iterator associatedLeaf;
    QVector<QPair<int, int>> newToOldPositions; ///< For transformed strings, we need to know which
    bool textInPath = false;
    bool firstTextInPath = false; ///< We need to mark the first text in path as an anchored chunk.
                                  ///< original index matches which new index;
    KoSvgTextProperties inheritedProps;
    QSharedPointer<KoShapeBackground> bg;
};

class KRITAFLAKE_EXPORT KoSvgTextShape::Private
{
public:
    // NOTE: the cache data is shared between all the instances of
    //       the shape, though it will be reset locally if the
    //       accessing thread changes

    Private() = default;

    Private(const Private &rhs)
        : textData(rhs.textData) {
        Q_FOREACH (KoShape *shape, rhs.shapesInside) {
            KoShape *clonedShape = shape->cloneShape();
            KIS_ASSERT_RECOVER(clonedShape) { continue; }

            shapesInside.append(clonedShape);
        }
        Q_FOREACH (KoShape *shape, rhs.shapesSubtract) {
            KoShape *clonedShape = shape->cloneShape();
            KIS_ASSERT_RECOVER(clonedShape) { continue; }

            shapesSubtract.append(clonedShape);
        }
        yRes = rhs.yRes;
        xRes = rhs.xRes;
        result = rhs.result;
        lineBoxes = rhs.lineBoxes;

        cursorPos = rhs.cursorPos;
        logicalToVisualCursorPos = rhs.logicalToVisualCursorPos;
        plainText = rhs.plainText;
        isBidi = rhs.isBidi;
        initialTextPosition = rhs.initialTextPosition;

        isLoading = rhs.isLoading;
        disableFontMatching = rhs.disableFontMatching;
    }

    ~Private() {
        qDeleteAll(shapesInside);
        qDeleteAll(shapesSubtract);
    }

    int xRes = 72;
    int yRes = 72;
    QList<KoShape*> shapesInside;
    QList<KoShape*> shapesSubtract;

    KisForest<KoSvgTextContentElement> textData;
    bool isLoading = false; ///< Turned on when loading in text data, blocks updates to shape listeners.

    bool disableFontMatching = false; ///< Turn off font matching, which should speed up relayout slightly.

    QVector<CharacterResult> result;
    QVector<LineBox> lineBoxes;

    QVector<CursorPos> cursorPos;
    QMap<int, int> logicalToVisualCursorPos;

    QString plainText;
    bool isBidi = false;
    QPointF initialTextPosition = QPointF();

    void relayout();

    static bool loadGlyph(const KoSvgText::ResolutionHandler &resHandler,
                   const FT_Int32 faceLoadFlags,
                   const bool isHorizontal,
                   const char32_t firstCodepoint,
                   const KoSvgText::TextRendering rendering,
                   raqm_glyph_t &currentGlyph,
                   CharacterResult &charResult,
                   QPointF &totalAdvanceFTFontCoordinates);

    static std::pair<QTransform, qreal> loadGlyphOnly(const QTransform &ftTF,
                                               FT_Int32 faceLoadFlags,
                                               bool isHorizontal,
                                               raqm_glyph_t &currentGlyph,
                                               CharacterResult &charResult, const KoSvgText::TextRendering rendering);

    void clearAssociatedOutlines();
    void resolveTransforms(KisForest<KoSvgTextContentElement>::child_iterator currentTextElement,
                           QString text, QVector<CharacterResult> &result, int &currentIndex,
                           bool isHorizontal, bool wrapped, bool textInPath, QVector<KoSvgText::CharTransformation> &resolved,
                           QVector<bool> collapsedChars, const KoSvgTextProperties resolvedProps);

    void applyTextLength(KisForest<KoSvgTextContentElement>::child_iterator currentTextElement, QVector<CharacterResult> &result, int &currentIndex, int &resolvedDescendentNodes, bool isHorizontal,
                         const KoSvgTextProperties resolvedProps, const KoSvgText::ResolutionHandler &resHandler);
    static void applyAnchoring(QVector<CharacterResult> &result, bool isHorizontal, const KoSvgText::ResolutionHandler resHandler);
    static qreal
    characterResultOnPath(CharacterResult &cr, qreal length, qreal offset, bool isHorizontal, bool isClosed);
    static QPainterPath stretchGlyphOnPath(const QPainterPath &glyph,
                                           const QPainterPath &path,
                                           bool isHorizontal,
                                           qreal offset,
                                           bool isClosed);
    static void applyTextPath(KisForest<KoSvgTextContentElement>::child_iterator parent, QVector<CharacterResult> &result, bool isHorizontal, QPointF &startPos, const KoSvgTextProperties resolvedProps);
    static void computeFontMetrics(KisForest<KoSvgTextContentElement>::child_iterator parent, const KoSvgTextProperties &parentProps,
                            const KoSvgText::FontMetrics &parentBaselineTable, const KoSvgText::Baseline parentBaseline,
                            const QPointF superScript,
                            const QPointF subScript,
                            QVector<CharacterResult> &result,
                            int &currentIndex,
                            const KoSvgText::ResolutionHandler resHandler,
                            const bool isHorizontal,
                            const bool disableFontMatching);
    static void handleLineBoxAlignment(KisForest<KoSvgTextContentElement>::child_iterator parent,
                            QVector<CharacterResult> &result, const QVector<LineBox> lineBoxes,
                            int &currentIndex,
                            const bool isHorizontal, const KoSvgTextProperties resolvedProps);
    void computeTextDecorations(KisForest<KoSvgTextContentElement>::child_iterator currentTextElement,
                                const QVector<CharacterResult>& result,
                                const QMap<int, int>& logicalToVisual,
                                const KoSvgText::ResolutionHandler resHandler,
                                KoPathShape *textPath,
                                qreal textPathoffset,
                                bool side,
                                int &currentIndex,
                                bool isHorizontal,
                                bool ltr,
                                bool wrapping,
                                const KoSvgTextProperties resolvedProps);
    QMap<KoSvgText::TextDecoration, QPainterPath> generateDecorationPaths(const int &start, const int &end,
                                                                          const KoSvgText::ResolutionHandler resHandler,
                                                                          const QVector<CharacterResult> &result,
                                                                          const bool isHorizontal,
                                                                          const KoSvgText::TextDecorations &decor,
                                                                          const KoSvgText::TextDecorationStyle style = KoSvgText::TextDecorationStyle::Solid,
                                                                          const bool textDecorationSkipInset = false,
                                                                          const KoPathShape *currentTextPath = nullptr,
                                                                          const qreal currentTextPathOffset = 0.0,
                                                                          const bool textPathSide = false,
                                                                          const KoSvgText::TextDecorationUnderlinePosition underlinePosH = KoSvgText::TextDecorationUnderlinePosition::UnderlineAuto,
                                                                          const KoSvgText::TextDecorationUnderlinePosition underlinePosV = KoSvgText::TextDecorationUnderlinePosition::UnderlineAuto);
    static void finalizeDecoration (
            QPainterPath decorationPath,
            const QPointF offset,
            const QPainterPathStroker &stroker,
            const KoSvgText::TextDecoration type,
            QMap<KoSvgText::TextDecoration, QPainterPath> &decorationPaths,
            const KoPathShape *currentTextPath,
            const bool isHorizontal,
            const qreal currentTextPathOffset,
            const bool textPathSide
            );

    void paintTextDecoration(QPainter &painter,
                             const QPainterPath &outlineRect,
                             const KoShape *rootShape,
                             const KoSvgText::TextDecoration type,
                             const KoSvgText::TextRendering rendering);
    void paintPaths(QPainter &painter,
                    const QPainterPath &outlineRect,
                    const KoShape *rootShape,
                    const QVector<CharacterResult> &result,
                    const KoSvgText::TextRendering rendering,
                    QPainterPath &chunk,
                    int &currentIndex);
    KoShape* collectPaths(const KoShape *rootShape, QVector<CharacterResult> &result, int &currentIndex);
    void paintDebug(QPainter &painter,
                    const QVector<CharacterResult> &result,
                    int &currentIndex);

    /// Get the number of characters for the whole subtree of this node.
    static int numChars(KisForest<KoSvgTextContentElement>::child_iterator parent, bool withControls = false, KoSvgTextProperties resolvedProps = KoSvgTextProperties::defaultProperties()) {
        KoSvgTextProperties props = parent->properties;
        props.inheritFrom(resolvedProps, true);
        int count = parent->numChars(withControls, props);
        for (auto it = KisForestDetail::childBegin(parent); it != KisForestDetail::childEnd(parent); it++) {
            count += numChars(it, withControls, props);
        }
        return count;
    }

    /// Get the child count of the current node. A node without children is a text node.
    static int childCount(KisForest<KoSvgTextContentElement>::child_iterator it) {
        return std::distance(KisForestDetail::childBegin(it), KisForestDetail::childEnd(it));
    }
    /**
     * Return a linearized representation of a subtree of text "subchunks".
     */
    static QVector<SubChunk> collectSubChunks(KisForest<KoSvgTextContentElement>::child_iterator it, KoSvgTextProperties parent, bool textInPath, bool &firstTextInPath);

    /**
     * @brief findTextContentElementForIndex
     * Finds the given leaf of the current tree-wide string index.
     * @param tree -- tree to search in.
     * @param currentIndex -- currentIndex, will always be set to the start index of the found element.
     * @param sought -- index sought.
     * @param skipZeroWidth -- whether to explicitely skip zero-width chunks. Remove text may set this to true, while inserting text into empty chunks requires this to be false.
     * @return iterator -- found iterator. Will default to tree end if nothing is found.
     */
    static KisForest<KoSvgTextContentElement>::depth_first_tail_iterator findTextContentElementForIndex(KisForest<KoSvgTextContentElement> &tree,
                                                                                                        int &currentIndex,
                                                                                                        int sought,
                                                                                                        bool skipZeroWidth = false)
    {
        auto it = tree.depthFirstTailBegin();
        for (; it != tree.depthFirstTailEnd(); it++) {
            if (childCount(siblingCurrent(it)) > 0) {
                continue;
            }
            int length = it->numChars(false);
            if (length == 0 && skipZeroWidth) {
                continue;
            }

            if (sought == currentIndex || (sought > currentIndex && sought < currentIndex + length)) {
                break;
            } else {
                currentIndex += length;
            }
        }
        return it;
    }
    /**
     * @brief splitContentElement
     * split the contentElement in tree at index into two nodes.
     * @param tree -- tree to work on.
     * @param index -- index
     * @return whether it was successful.
     */
    static bool splitContentElement(KisForest<KoSvgTextContentElement> &tree, int index) {
        int currentIndex = 0;

        // If there's only a single root element, don't bother searching.
        auto contentElement = depth(tree) == 1? tree.depthFirstTailBegin(): findTextContentElementForIndex(tree, currentIndex, index, true);
        if (contentElement == tree.depthFirstTailEnd()) return false;

        bool suitableStartIndex = siblingCurrent(contentElement) == tree.childBegin()? index >= currentIndex: index > currentIndex;
        bool suitableEndIndex = siblingCurrent(contentElement) == tree.childBegin()? true: index < currentIndex + contentElement->numChars(false);

        if (suitableStartIndex && suitableEndIndex) {
            KoSvgTextContentElement duplicate = KoSvgTextContentElement();
            duplicate.text = contentElement->text;
            int start = index - currentIndex;
            int length = contentElement->numChars(false) - start;
            int zero = 0;
            duplicate.removeText(start, length);

            // TODO: handle localtransforms better; annoyingly, this requires whitespace handling

            if (siblingCurrent(contentElement) != tree.childBegin()
                    && !contentElement->textPath
                    && contentElement->textLength.isAuto
                    && contentElement->localTransformations.isEmpty()) {
                contentElement->removeText(zero, start);
                duplicate.properties = contentElement->properties;
                tree.insert(siblingCurrent(contentElement), duplicate);
            } else {
                KoSvgTextContentElement duplicate2 = KoSvgTextContentElement();
                duplicate2.text = contentElement->text;
                duplicate2.removeText(zero, start);
                contentElement->text.clear();
                tree.insert(childBegin(contentElement), duplicate);
                tree.insert(childEnd(contentElement), duplicate2);
            }
            return true;
        }
        return false;
    }

    /**
     * @brief cleanUp
     * This cleans up the tree by...
     * - Removing empty text elements that are not the root or text paths.
     * - Merging sibling elements with similar properties.
     * - Merging single children with parent if possible.
     * @param tree -- tree to clean up
     */
    static void cleanUp(KisForest<KoSvgTextContentElement> &tree) {
        for (auto it = tree.depthFirstTailBegin(); it != tree.depthFirstTailEnd(); it++) {
            const int children = childCount(siblingCurrent(it));
            if (children == 0) {

                // Remove empty leafs that are not the root or text paths.
                const int length = it->numChars(false);
                if (length == 0 && siblingCurrent(it) != tree.childBegin() && !it->textPath) {
                    tree.erase(siblingCurrent(it));
                } else {
                    // check if siblings are similar.
                    auto siblingPrev = siblingCurrent(it);
                    KoSvgText::UnicodeBidi bidi = KoSvgText::UnicodeBidi(it->properties.property(KoSvgTextProperties::UnicodeBidiId,
                                                                                                 QVariant(KoSvgText::BidiNormal)).toInt());
                    siblingPrev--;
                    if (!isEnd(siblingPrev)
                            && siblingPrev != siblingCurrent(it)
                            && (siblingPrev->localTransformations.isEmpty() && it->localTransformations.isEmpty())
                            && (!siblingPrev->textPath && !it->textPath)
                            && (siblingPrev->textLength.isAuto && it->textLength.isAuto)
                            && (siblingPrev->properties == it->properties)
                            && (bidi != KoSvgText::BidiIsolate && bidi != KoSvgText::BidiIsolateOverride)) {
                        // TODO: handle localtransforms better; annoyingly, this requires whitespace handling
                        siblingPrev->text += it->text;
                        tree.erase(siblingCurrent(it));
                    }
                }
            } else if (children == 1) {
                // merge single children into parents if possible.
                auto child = childBegin(siblingCurrent(it));
                if ((child->localTransformations.isEmpty() && it->localTransformations.isEmpty())
                        && (!child->textPath && !it->textPath)
                        && (child->textLength.isAuto && it->textLength.isAuto)
                        && (!child->properties.hasNonInheritableProperties() || !it->properties.hasNonInheritableProperties())) {
                    if (it->properties.hasNonInheritableProperties()) {
                        KoSvgTextProperties props = it->properties;
                        props.setAllButNonInheritableProperties(child->properties);
                        child->properties = props;
                    } else {
                        child->properties.inheritFrom(it->properties);
                    }
                    tree.move(child, siblingCurrent(it));
                    tree.erase(siblingCurrent(it));
                }
            }
        }
    }

    static KisForest<KoSvgTextContentElement>::child_iterator iteratorForTreeIndex(const QVector<int> treeIndex, KisForest<KoSvgTextContentElement>::child_iterator parent) {
        if (treeIndex.isEmpty()) return parent;
        QVector<int> idx = treeIndex;
        int count = idx.takeFirst();
        for (auto child = KisForestDetail::childBegin(parent); child != KisForestDetail::childEnd(parent); child++) {
            if (count == 0) {
                if ((KisForestDetail::childBegin(child) != KisForestDetail::childEnd(child))) {
                    return iteratorForTreeIndex(idx, child);
                } else {
                    return child;
                }
            }
            count -= 1;
        }
        return KisForestDetail::childEnd(parent);
    }

    static bool startIndexOfIterator(KisForest<KoSvgTextContentElement>::child_iterator parent, KisForest<KoSvgTextContentElement>::child_iterator target, int &currentIndex) {
        for (auto child = KisForestDetail::childBegin(parent); child != KisForestDetail::childEnd(parent); child++) {
            if (child == target) {
                return true;
            } else if ((KisForestDetail::childBegin(child) != KisForestDetail::childEnd(child))) {
                if (startIndexOfIterator(child, target, currentIndex)) {
                    return true;
                }
            } else {
                currentIndex += numChars(child);
            }
        }
        return false;
    }
};

#endif // KO_SVG_TEXT_SHAPE_P_H
