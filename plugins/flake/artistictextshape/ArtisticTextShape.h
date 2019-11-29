/* This file is part of the KDE project
 * Copyright (C) 2007-2008,2011 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2008 Rob Buis <buis@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef ARTISTICTEXTSHAPE_H
#define ARTISTICTEXTSHAPE_H

#include "ArtisticTextRange.h"
#include <KoShape.h>
#include <KoPostscriptPaintDevice.h>
#include <SvgShape.h>
#include <QFont>
#include <QPainterPath>
#include <QVector>

class QPainter;
class KoPathShape;
class ArtisticTextLoadingContext;
class SvgGraphicsContext;

#define ArtisticTextShapeID "ArtisticText"

/// Character position within text shape (range index, range character index)
typedef QPair<int, int> CharIndex;

class ArtisticTextShape : public KoShape, public SvgShape
{
public:
    enum TextAnchor { AnchorStart, AnchorMiddle, AnchorEnd };

    enum LayoutMode {
        Straight,    ///< baseline is a straight line
        OnPath,      ///< baseline is a QPainterPath
        OnPathShape  ///< baseline is the outline of a path shape
    };

    ArtisticTextShape();
    ~ArtisticTextShape() override;
    virtual KoShape *cloneShape() const override;

    /// reimplemented
    void paint(QPainter &painter, KoShapePaintingContext &paintContext) override;
    /// reimplemented
    void saveOdf(KoShapeSavingContext &context) const override;
    /// reimplemented
    bool loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context) override;
    /// reimplemented
    QSizeF size() const override;
    /// reimplemented
    void setSize(const QSizeF &size) override;
    /// reimplemented
    QPainterPath outline() const override;
    /// reimplemented from SvgShape
    bool saveSvg(SvgSavingContext &context) override;
    /// reimplemented from SvgShape
    bool loadSvg(const KoXmlElement &element, SvgLoadingContext &context) override;

    /// Sets the plain text to display
    void setPlainText(const QString &newText);

    /// Returns the plain text content
    QString plainText() const;

    /// Returns formatted text
    QList<ArtisticTextRange> text() const;

    /// Returns if text shape is empty, i.e. no text
    bool isEmpty() const;

    /// Clears the text shape
    void clear();

    /**
     * Sets the font used for drawing
     * Note that it is expected that the font has its point size set
     * in postscript points.
     */
    void setFont(const QFont &font);

    /**
     * Sets the font for the specified range of characters
     * @param charIndex the index of the first character of the range
     * @param charCount the number of characters of the range
     * @param font the new font to set
     */
    void setFont(int charIndex, int charCount, const QFont &font);

    /**
     * Returns the font at the specified character position
     * If the text shape is empty it will return the default font.
     * If the character index is smaller than zero it will return the font
     * of the first character. If the character index is greater than the
     * last character index it will return the font of the last character.
     */
    QFont fontAt(int charIndex) const;

    /// Returns the default font
    QFont defaultFont() const;

    /// Attaches this text shape to the given path shape
    bool putOnPath(KoPathShape *path);

    /// Puts the text on the given path, the path is expected to be in document coordinates
    bool putOnPath(const QPainterPath &path);

    /// Detaches this text shape from an already attached path shape
    void removeFromPath();

    /// Returns if shape is attached to a path shape
    bool isOnPath() const;

    /// Sets the offset for for text on path
    void setStartOffset(qreal offset);

    /// Returns the start offset for text on path
    qreal startOffset() const;

    /**
     * Returns the y-offset from the top-left corner to the baseline.
     * This is usable for being able to exactly position the texts baseline.
     * Note: The value makes only sense for text not attached to a path.
     */
    qreal baselineOffset() const;

    /// Sets the text anchor
    void setTextAnchor(TextAnchor anchor);

    /// Returns the actual text anchor
    TextAnchor textAnchor() const;

    /// Returns the current layout mode
    LayoutMode layout() const;

    /// Returns the baseline path
    QPainterPath baseline() const;

    /// Returns a pointer to the shape used as baseline
    KoPathShape *baselineShape() const;

    /// Removes a range of text starting from the given character
    QList<ArtisticTextRange> removeText(int charIndex, int charCount);

    /// Copies a range of text starting from the given character
    QList<ArtisticTextRange> copyText(int charIndex, int charCount);

    /// Adds a range of text at the given index
    void insertText(int charIndex, const QString &plainText);

    /// Adds range of text at the given index
    void insertText(int charIndex, const ArtisticTextRange &textRange);

    /// Adds ranges of text at the given index
    void insertText(int charIndex, const QList<ArtisticTextRange> &textRanges);

    /// Appends plain text to the last text range
    void appendText(const QString &plainText);

    /// Appends a single formatted range of text
    void appendText(const ArtisticTextRange &text);

    /// Replaces a range of text with the specified text range
    bool replaceText(int charIndex, int charCount, const ArtisticTextRange &textRange);

    /// Replaces a range of text with the specified text ranges
    bool replaceText(int charIndex, int charCount, const QList<ArtisticTextRange> &textRanges);

    /// Gets the angle of the char with the given index
    qreal charAngleAt(int charIndex) const;

    /// Gets the position of the char with the given index in shape coordinates
    QPointF charPositionAt(int charIndex) const;

    /// Gets the extents of the char with the given index
    QRectF charExtentsAt(int charIndex) const;

    /// Returns index of range and index within range of specified character
    CharIndex indexOfChar(int charIndex) const;

    /// reimplemented from KoShape
    void shapeChanged(ChangeType type, KoShape *shape) override;

private:
    void updateSizeAndPosition(bool global = false);
    void cacheGlyphOutlines();
    bool pathHasChanged() const;
    void createOutline();

    void beginTextUpdate();
    void finishTextUpdate();

    /// Calculates abstract character positions in baseline coordinates
    QVector<QPointF> calculateAbstractCharacterPositions();

    /// Returns the bounding box for an empty text shape
    QRectF nullBoundBox() const;

    /// Saves svg font
    void saveSvgFont(const QFont &font, SvgSavingContext &context);
    /// Saves svg text range
    void saveSvgTextRange(const ArtisticTextRange &range, SvgSavingContext &context, bool saveFont, qreal baselineOffset);
    /// Parse nested text ranges
    void parseTextRanges(const KoXmlElement &element, SvgLoadingContext &context, ArtisticTextLoadingContext &textContext);
    /// Creates text range
    ArtisticTextRange createTextRange(const QString &text, ArtisticTextLoadingContext &context, SvgGraphicsContext *gc);

    QList<ArtisticTextRange> m_ranges;
    KoPostscriptPaintDevice m_paintDevice;
    KoPathShape *m_path;  ///< the path shape we are attached to
    QList<QPainterPath> m_charOutlines; ///< cached character oulines
    qreal m_startOffset; ///< the offset from the attached path start point
    QPointF m_outlineOrigin; ///< the top-left corner of the non-normalized text outline
    QPainterPath m_outline; ///< the actual text outline
    QPainterPath m_baseline; ///< the baseline path the text is put on
    TextAnchor m_textAnchor; ///< the actual text anchor
    QVector<qreal> m_charOffsets; ///< char positions [0..1] on baseline path
    QVector<QPointF> m_charPositions; ///< char positions in shape coordinates
    int m_textUpdateCounter;
    QFont m_defaultFont;
};

#endif // ARTISTICTEXTSHAPE_H
