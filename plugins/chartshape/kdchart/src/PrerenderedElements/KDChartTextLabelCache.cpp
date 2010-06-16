/* -*- Mode: C++ -*-
   KDChart - a multi-platform charting engine
   */

/****************************************************************************
 ** Copyright (C) 2005-2007 Klarälvdalens Datakonsult AB.  All rights reserved.
 **
 ** This file is part of the KD Chart library.
 **
 ** This file may be used under the terms of the GNU General Public
 ** License versions 2.0 or 3.0 as published by the Free Software
 ** Foundation and appearing in the files LICENSE.GPL2 and LICENSE.GPL3
 ** included in the packaging of this file.  Alternatively you may (at
 ** your option) use any later version of the GNU General Public
 ** License if such license has been publicly approved by
 ** Klarälvdalens Datakonsult AB (or its successors, if any).
 ** 
 ** This file is provided "AS IS" with NO WARRANTY OF ANY KIND,
 ** INCLUDING THE WARRANTIES OF DESIGN, MERCHANTABILITY AND FITNESS FOR
 ** A PARTICULAR PURPOSE. Klarälvdalens Datakonsult AB reserves all rights
 ** not expressly granted herein.
 ** 
 ** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
 ** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 **
 **********************************************************************/

#include "KDChartTextLabelCache.h"

#include <cmath>

#include <QtDebug>
#include <QImage>
#include <QPixmap>
#include <QPainter>
#include <QApplication>

#ifndef NDEBUG
int HitCount = 0;
int MissCount = 0;
#define INC_HIT_COUNT { ++HitCount; }
#define INC_MISS_COUNT  { ++MissCount; }
#define DUMP_CACHE_STATS \
    if ( HitCount != 0 && MissCount != 0 ) { \
        int total = HitCount + MissCount; \
        double hitQuote = ( 1.0 * HitCount ) / total; \
        qDebug() << "PrerenderedLabel dtor: hits/misses/total:" \
        << HitCount << "/" << MissCount << "/" << total \
                 << "(" << 100 * hitQuote << "% hits)"; \
    }
#else
#define INC_HIT_COUNT
#define INC_MISS_COUNT
#define DUMP_CACHE_STATS
#endif

PrerenderedElement::PrerenderedElement()
    : m_referencePoint( KDChartEnums::PositionNorthWest )
{
}

void PrerenderedElement::setPosition( const QPointF& position )
{   // this does not invalidate the element
    m_position = position;
}

const QPointF& PrerenderedElement::position() const
{
    return m_position;
}

void PrerenderedElement::setReferencePoint( KDChartEnums::PositionValue point )
{   // this does not invalidate the element
    m_referencePoint = point;
}

KDChartEnums::PositionValue PrerenderedElement::referencePoint() const
{
    return m_referencePoint;
}

PrerenderedLabel::PrerenderedLabel()
    : PrerenderedElement()
    , m_dirty( true )
    , m_font( qApp->font() )
    , m_brush( Qt::black )
    , m_pen( Qt::black ) // do not use anything invisible
    , m_angle( 0.0 )
{
}

PrerenderedLabel::~PrerenderedLabel()
{
    DUMP_CACHE_STATS;
}

/**
  * Invalidates the preredendered data, forces re-rendering.
  */
void PrerenderedLabel::invalidate() const
{
    m_dirty = true;
}

/**
  * Sets the label's font to \a font.
  */
void PrerenderedLabel::setFont( const QFont& font )
{
    m_font = font;
    invalidate();
}

/**
  * @return the label's font.
  */
const QFont& PrerenderedLabel::font() const
{
    return m_font;
}

/**
  * Sets the label's text to \a text
  */
void PrerenderedLabel::setText( const QString& text )
{
    m_text = text;
    invalidate();
}

/**
  * @return the label's text
  */
const QString& PrerenderedLabel::text() const
{
    return m_text;
}

/**
  * Sets the label's brush to \a brush
  */
void PrerenderedLabel::setBrush( const QBrush& brush )
{
    m_brush = brush;
    invalidate();
}

/**
  * @return the label's brush
  */
const QBrush& PrerenderedLabel::brush() const
{
    return m_brush;
}

/**
  * Sets the angle of the label to \a angle degrees
  */
void PrerenderedLabel::setAngle( double angle )
{
    m_angle = angle;
    invalidate();
}

/**
  * @return the label's angle in degrees
  */
double PrerenderedLabel::angle() const
{
    return m_angle;
}

const QPixmap& PrerenderedLabel::pixmap() const
{
    if ( m_dirty ) {
        INC_MISS_COUNT;
        paint();
    } else {
        INC_HIT_COUNT;
    }
    return m_pixmap;
}

void PrerenderedLabel::paint() const
{
    // FIXME find a better value using font metrics of text (this
    // requires finding the diameter of the circle formed by rotating
    // the bounding rect around the center):
    const int Width = 1000;
    const int Height = Width;

    QRectF boundingRect;
    const QColor FullTransparent( 255, 255, 255, 0 );
#ifdef Q_WS_X11
    QImage pixmap( Width, Height, QImage::Format_ARGB32_Premultiplied );
    qWarning() << "PrerenderedLabel::paint: using QImage for prerendered labels "
               << "to work around XRender/Qt4 bug.";
#else
    QPixmap pixmap( Width, Height );
#endif
    // pixmap.fill( FullTransparent );
    {
        static const QPointF Center ( 0.0, 0.0 );
        QPointF textBottomRight;
        QPainter painter( &pixmap );
        painter.setRenderHint(QPainter::TextAntialiasing, true );
        painter.setRenderHint(QPainter::Antialiasing, true );

        // QImage (X11 workaround) does not have fill():
        painter.setPen( FullTransparent );
        painter.setBrush( FullTransparent );
        painter.drawRect( 0, 0, Width, Height );

        QMatrix matrix;
        matrix.translate( 0.5 * Width,  0.5 * Height );
        matrix.rotate( m_angle );
#if QT_VERSION > 0x040199
        painter.setWorldMatrix( matrix );
#else
        painter.setMatrix( matrix );
#endif

        painter.setPen( m_pen );
        painter.setBrush( m_brush );
        painter.setFont( m_font );
        QRectF container( -0.5 * Width, -0.5 * Height, Width, 0.5 * Height );
        painter.drawText( container, Qt::AlignHCenter | Qt::AlignBottom,
                          m_text, &boundingRect );
        m_referenceBottomLeft = QPointF( boundingRect.bottomLeft().x(), 0.0 );
        textBottomRight = QPointF( boundingRect.bottomRight().x(), 0.0 );
        m_textAscendVector = boundingRect.topRight() - textBottomRight;
        m_textBaseLineVector = textBottomRight - m_referenceBottomLeft;

        // FIXME translate topright by char height
        boundingRect = matrix.mapRect( boundingRect );
        m_referenceBottomLeft = matrix.map( m_referenceBottomLeft )
                                - boundingRect.topLeft();
        textBottomRight = matrix.map( textBottomRight )
                          - boundingRect.topLeft();
        m_textAscendVector = matrix.map( m_textAscendVector )
                             - matrix.map( Center );
        m_textBaseLineVector = matrix.map( m_textBaseLineVector )
                            - matrix.map( Center );
    }

    m_dirty = false; // now all the calculation vectors are valid

    QPixmap temp( static_cast<int>( boundingRect.width() ),
                  static_cast<int>( boundingRect.height() ) );
    {
        temp.fill( FullTransparent );
        QPainter painter( &temp );
#ifdef Q_WS_X11
        painter.drawImage( QPointF( 0.0, 0.0 ), pixmap, boundingRect );
#else
        painter.drawPixmap( QPointF( 0.0, 0.0 ), pixmap, boundingRect );
#endif
// #define PRERENDEREDLABEL_DEBUG
#ifdef PRERENDEREDLABEL_DEBUG
        painter.setPen( QPen( Qt::red, 2 ) );
        painter.setBrush( Qt::red );
        // paint markers for the reference points
        QList<KDChartEnums::PositionValue> positions;
        positions << KDChartEnums::PositionCenter
                  << KDChartEnums::PositionNorthWest
                  << KDChartEnums::PositionNorth
                  << KDChartEnums::PositionNorthEast
                  << KDChartEnums::PositionEast
                  << KDChartEnums::PositionSouthEast
                  << KDChartEnums::PositionSouth
                  << KDChartEnums::PositionSouthWest
                  << KDChartEnums::PositionWest;
        Q_FOREACH( KDChartEnums::PositionValue position, positions ) {
            static const double Radius = 0.5;
            static const double Diameter = 2 * Radius;

            QPointF point ( referencePointLocation( position ) );
            painter.drawEllipse( QRectF( point - QPointF( Radius, Radius ),
                                         QSizeF( Diameter, Diameter ) ) );
        }
#endif
    }

    m_pixmap = temp;
}

QPointF PrerenderedLabel::referencePointLocation() const
{
    return referencePointLocation( referencePoint() );
}

QPointF PrerenderedLabel::referencePointLocation( KDChartEnums::PositionValue position ) const
{
    if ( m_dirty ) {
        INC_MISS_COUNT;
        paint();
    } else {
        INC_HIT_COUNT;
    }

    switch( position ) {
    case KDChartEnums::PositionCenter:
        return m_referenceBottomLeft + 0.5 * m_textBaseLineVector + 0.5 * m_textAscendVector;
    case KDChartEnums::PositionNorthWest:
        return m_referenceBottomLeft + m_textAscendVector;
    case KDChartEnums::PositionNorth:
        return m_referenceBottomLeft + 0.5 * m_textBaseLineVector + m_textAscendVector;
    case KDChartEnums::PositionNorthEast:
        return m_referenceBottomLeft + m_textBaseLineVector + m_textAscendVector;
    case KDChartEnums::PositionEast:
        return m_referenceBottomLeft + 0.5 * m_textAscendVector;
    case KDChartEnums::PositionSouthEast:
        return m_referenceBottomLeft + m_textBaseLineVector;
    case KDChartEnums::PositionSouth:
        return m_referenceBottomLeft + 0.5 * m_textBaseLineVector;
    case KDChartEnums::PositionSouthWest:
        return m_referenceBottomLeft;
    case KDChartEnums::PositionWest:
        return m_referenceBottomLeft + m_textBaseLineVector + 0.5 * m_textAscendVector;

    case KDChartEnums::PositionUnknown: // intentional fall-through
    case KDChartEnums::PositionFloating: // intentional fall-through
    default:
        return QPointF();
    }
}
