/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOSVGTEXTCHUNKSHAPELAYOUTINTERFACE_H
#define KOSVGTEXTCHUNKSHAPELAYOUTINTERFACE_H

#include <kritaflake_export.h>

#include <QTextCharFormat>

#include "KoPathShape.h"
#include "KoSvgText.h"
#include <boost/optional.hpp>
#include <utility>

class KoSvgTextChunkShape;
class KoSvgTextProperties;


/**
 * KoSvgTextChunkShapeLayoutInterface is a private interface for accessing
 * KoSvgTextChunkShape internals from KoSvgTextShape's layout code.
 */
class KRITAFLAKE_EXPORT KoSvgTextChunkShapeLayoutInterface
{
public:
    KoSvgTextChunkShapeLayoutInterface() {}
    virtual ~KoSvgTextChunkShapeLayoutInterface() {}

    /**
     * the value 'textLength' attribute of the associated dom element
     */
    virtual KoSvgText::AutoValue textLength() const = 0;

    /**
     * the value 'lengthAdjust' attribute of the associated dom element
     */
    virtual KoSvgText::LengthAdjust lengthAdjust() const = 0;

    /**
     * The number of characters contained in the dom **subtree**
     * @param withControls this will enable the bidi controls to be
     * counted as well.
     */
    virtual int numChars(bool withControls = false) const = 0;

    /**
     * Relative position of a character at position \p pos inside one of the children \p child.
     * FIXME: not used now!
     */
    virtual int relativeCharPos(KoSvgTextChunkShape *child, int pos) const = 0;

    /**
     * @return true if a shape is a leaf "text node" and false if it is
     *         an intermediate node aggregating other objects.
     */
    virtual bool isTextNode() const = 0;

    /**
     * @return the text contained in this **leaf** node. For intermediate nodes always returns false.
     */
    virtual QString nodeText() const = 0;

    /**
     * @return a text on path info struct.
     **/
    virtual KoSvgText::TextOnPathInfo textOnPathInfo() const = 0;
    /**
     * @return the textPath. returns null if this is not a textPath element.
     */
    virtual KoShape *textPath() const = 0;

    /**
     * @return a vector of local character transformations x, y, dx, dy and rotate.
     */
    virtual QVector<KoSvgText::CharTransformation> localCharTransformations() const = 0;

    /**
     * Add a rect to the aggregated outline of the shape.
     *
     * The text contained in the chunk shape can be randomly spread on the
     * canvas during layout process. The KoSvgTextChunkShape's text can be
     * split into multiple "SVG text chunks" or just messed up with
     * unicode-bidi algorithm.
     *
     * To let the shape know, where its text is placed, the layout engine
     * notifies the shape by calling `addAssociatedOutline()` for every
     * character of the shape. The shape aggregates these values to understand
     * its `outline()`.
     */
    virtual void addAssociatedOutline(const QRectF &rect) = 0;

    /**
     * Clear the aggregated outline of the shape
     *
     * @see addAssociatedOutline
     */
    virtual void clearAssociatedOutline() = 0;

    /**
     * @brief addTextDecorationFontMetrics
     * To compute the text decoration, it's best to use values gained from the
     * first available font. In the layout code this is retrieved together with
     * the baseline metrics, it is then cached so the text decoration can be
     * drawn later.
     * @param type whether it's underline, overline or linethrough.
     * @param offset the offset metric of the given decoration.
     * @param width the width metric of the given decoration.
     */
    virtual void setTextDecorationFontMetrics(KoSvgText::TextDecoration type, qreal offset, qreal width) = 0;

    /**
     * @brief getTextDecorationWidth
     * @return get the offset for the given decoration.
     */
    virtual qreal getTextDecorationOffset(KoSvgText::TextDecoration type) = 0;
    /**
     * @brief getTextDecorationWidth
     * @return get the line width for the given decoration.
     */
    virtual qreal getTextDecorationWidth(KoSvgText::TextDecoration type) = 0;
    /**
     * @brief addTextDecoration
     * @param type whether it's an underline, over, etc.
     * @param path the full text-decoration path.
     *
     * textDecorations are applied on textChunks, so a KoSvgTextChunk with
     * two child chunks will not apply the text-decoration per child-chunk,
     * but rather on the parent chunk. The text decoration can thus
     * only reliably be calculated during the text-layout process.
     */
    virtual void addTextDecoration(KoSvgText::TextDecoration type, QPainterPath path) = 0;
    /**
     * Clear all text-decorations
     *
     * @see addTextDecoration
     */
    virtual void clearTextDecorations() = 0;

    /**
     * @brief textDecorations
     * get the cached textDecorations so they can be drawn.
     */
    virtual QMap<KoSvgText::TextDecoration, QPainterPath> textDecorations() = 0;

    virtual void insertText(int index, QString text) = 0;

    virtual void removeText(int &index, int length) = 0;

    virtual void setTextProperties(KoSvgTextProperties properties) = 0;

    /**
     * A QTextLayout-compatible representation of a single leaf of
     * KoSvgTextChunkShape subtree
     */
    struct SubChunk
    {
        SubChunk()
        {
        }

        SubChunk(QString _text, QString originalText, KoSvgText::KoSvgCharChunkFormat _format, const QVector<QPair<int, int>> &positions, bool textInPath = false, bool firstTextInPath = false)
            : text(std::move(_text))
            , originalText(std::move(originalText))
            , format(std::move(_format))
            , newToOldPositions(positions)
            , textInPath(textInPath)
            , firstTextInPath(firstTextInPath)
        {
        }

        SubChunk(QString _text,
                 QString originalText,
                 KoSvgText::KoSvgCharChunkFormat _format,
                 const QVector<KoSvgText::CharTransformation> &t,
                 const QVector<QPair<int, int>> &positions,
                 bool textInPath = false,
                 bool firstTextInPath = false)
            : text(std::move(_text))
            , originalText(std::move(originalText))
            , format(std::move(_format))
            , transformation(t)
            , newToOldPositions(positions)
            , textInPath(textInPath)
            , firstTextInPath(firstTextInPath)
        {
        }

        QString text;
        QString originalText;
        KoSvgText::KoSvgCharChunkFormat format;
        QVector<KoSvgText::CharTransformation> transformation;
        QVector<QPair<int, int>> newToOldPositions; ///< For transformed strings, we need to know which
                                                    ///< original index matches which new index;
        bool textInPath = false;
        bool firstTextInPath = false; ///< We need to mark the first text in path as an anchored chunk.

    };

    /**
     * Return a linearized representation of a subtree of text "subchunks".
     */
    virtual QVector<SubChunk> collectSubChunks(bool textInPath, bool &firstTextInPath) const = 0;
};

#endif // KOSVGTEXTCHUNKSHAPELAYOUTINTERFACE_H
