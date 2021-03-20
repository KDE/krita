/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOSVGTEXTCHUNKSHAPELAYOUTINTERFACE_H
#define KOSVGTEXTCHUNKSHAPELAYOUTINTERFACE_H

#include <kritaflake_export.h>

#include <QTextCharFormat>

#include "KoSvgText.h"
#include <boost/optional.hpp>

class KoSvgTextChunkShape;


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
     * FIXME: not used now!
     */
    virtual int numChars() const = 0;

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
     * @return a vector of local character transformations x, y, dx, dy and rotate.
     */
    virtual QVector<KoSvgText::CharTransformation> localCharTransformations() const = 0;

    /**
     * Add a rect to the aggregated outline of the shape.
     *
     * The text contained in the chunk shape can be randomly spread on the
     * canvas during layouting process. The KoSvgTextChunkShape's text can be
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
     * A QTextLayout-compatible representation of a single leaf of
     * KoSvgTextChunkShape subtree
     */
    struct SubChunk
    {
        SubChunk()
        {
        }

        SubChunk(const QString &_text, const KoSvgText::KoSvgCharChunkFormat &_format)
            : text(_text), format(_format)
        {
        }

        SubChunk(const QString &_text, const KoSvgText::KoSvgCharChunkFormat &_format,
                  const KoSvgText::CharTransformation &t)
            : text(_text), format(_format), transformation(t)
        {
        }

        QString text;
        KoSvgText::KoSvgCharChunkFormat format;
        KoSvgText::CharTransformation transformation;
    };

    /**
     * Return a linearized representation of a subtree of text "subchunks".
     *
     * TRICK ALERT: every `SubChunk` represents a single `KoSvgTextChunkShape`.
     * The name is changed because KoSvgTextChunkShape is not the same thing as
     * "svg text chunk". Later in the layouting code, the "subchunks" will be
     * split up into even smaller parts and then joined into proper "svg text
     * chunks".
     */
    virtual QVector<SubChunk> collectSubChunks() const = 0;
};

#endif // KOSVGTEXTCHUNKSHAPELAYOUTINTERFACE_H
