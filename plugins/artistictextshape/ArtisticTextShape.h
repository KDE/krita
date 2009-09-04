/* This file is part of the KDE project
 * Copyright (C) 2007-2008 Jan Hambrecht <jaham@gmx.net>
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

#include <KoShape.h>
#include <KoPostscriptPaintDevice.h>

#include <QtGui/QFont>

class QPainter;
class KoPathShape;

#define ArtisticTextShapeID "ArtisticText"

class ArtisticTextShape : public KoShape
{
public:
    enum TextAnchor { AnchorStart, AnchorMiddle, AnchorEnd };

    enum LayoutMode {
        Straight,    ///< baseline is a straight line
        OnPath,      ///< baseline is a QPainterPath
        OnPathShape  ///< baseline is the outline of a path shape
    };

    ArtisticTextShape();
    virtual ~ArtisticTextShape();

    /// reimplemented
    void paint(QPainter &painter, const KoViewConverter &converter);
    /// reimplemented to be empty (this shape is fully printing)
    void paintDecorations(QPainter &painter, const KoViewConverter &converter, const KoCanvasBase *canvas);
    /// reimplemented
    virtual void saveOdf(KoShapeSavingContext & context) const;
    /// reimplemented
    virtual bool loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context );
    /// reimplemented
    virtual QSizeF size() const;
    /// reimplemented
    virtual void setSize( const QSizeF &size );
    /// reimplemented
    virtual QPainterPath outline() const;

    /// Sets the text to display
    void setText( const QString & text );

    /// Returns the text content
    QString text() const;

    /**
     * Sets the font used for drawing
     * Note that it is expected that the font has its point size set
     * in postscript points.
     */
    void setFont( const QFont & font );

    /// Returns the font
    QFont font() const;

    /// Attaches this text shape to the given path shape
    bool putOnPath( KoPathShape * path );

    /// Puts the text on the given path, the path is expected to be in document coordinates
    bool putOnPath( const QPainterPath &path );

    /// Detaches this text shape from an already attached path shape
    void removeFromPath();

    /// Returns if shape is attached to a path shape
    bool isOnPath() const;

    /// Sets the offset for for text on path
    void setStartOffset( qreal offset );

    /// Returns the start offset for text on path
    qreal startOffset() const;

    /**
     * Returns the y-offset from the top-left corner to the baseline.
     * This is usable for being able to exactly position the texts baseline.
     * Note: The value makes only sense for text not attached to a path.
     */
    qreal baselineOffset() const;

    /// Sets the text anchor
    void setTextAnchor( TextAnchor anchor );

    /// Returns the actual text anchor
    TextAnchor textAnchor() const;

    /// Returns the current layout mode
    LayoutMode layout() const;

    /// Returns the baseline path
    QPainterPath baseline() const;

    /// Returns a pointer to the shape used as baseline
    KoPathShape * baselineShape() const;

    /// Removes a range of text from the given index
    QString removeRange( unsigned int index, unsigned int nr );
    
    /// Adds a range of text at the given index
    void addRange( unsigned int index, const QString &text );

    /// Gets the angle of the char with the given index
    void getCharAngleAt( unsigned int charNum, qreal &angle ) const;

    /// Gets the position of the char with the given index
    void getCharPositionAt( unsigned int charNum, QPointF &pos ) const;

    /// Gets the extents of the char with the given index
    void getCharExtentsAt( unsigned int charNum, QRectF &extents ) const;

    /// reimplemented from KoShape
    virtual void shapeChanged(ChangeType type, KoShape * shape);

private:
    void updateSizeAndPosition( bool global = false );
    void cacheGlyphOutlines();
    bool pathHasChanged() const;
    void createOutline();
    QRectF nullBoundBox() const;

    KoPostscriptPaintDevice m_paintDevice;
    QString m_text; ///< the text content
    QFont m_font; ///< the font to use for drawing
    KoPathShape * m_path; ///< the path shape we are attached to
    QList<QPainterPath> m_charOutlines; ///< cached character oulines
    qreal m_startOffset; ///< the offset from the attached path start point
    qreal m_baselineOffset; ///< the y-offset from the top-left corner to the baseline
    QPointF m_outlineOrigin; ///< the top-left corner of the non-normalized text outline
    QPainterPath m_outline; ///< the actual text outline
    QPainterPath m_baseline; ///< the baseline path the text is put on
    TextAnchor m_textAnchor; ///< the actual text anchor
    QVector<qreal> m_charOffsets; ///< char positions [0..1] on baseline path
};

#endif // ARTISTICTEXTSHAPE_H
