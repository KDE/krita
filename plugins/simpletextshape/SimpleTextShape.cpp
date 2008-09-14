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

#include "SimpleTextShape.h"

#include <KoPathShape.h>
#include <KoGlobal.h>
#include <KoShapeSavingContext.h>
#include <KoShapeLoadingContext.h>
#include <KoXmlNS.h>
#include <KoXmlWriter.h>
#include <KoXmlReader.h>
#include <KoUnit.h>
#include <KoPathShapeLoader.h>
#include <KoShapeBackground.h>

#include <KLocale>

#include <QtGui/QPen>
#include <QtGui/QPainter>
#include <QtGui/QFont>

#define FROM_PS_SIZE(x) (72.0 / KoGlobal::dpiY() * x)
#define TO_PS_SIZE(x) ( KoGlobal::dpiY() / 72.0 * x)

SimpleTextShape::SimpleTextShape()
    : m_text( i18n( "Simple Text" ) ), m_font( "ComicSans", FROM_PS_SIZE(20) )
    , m_path(0), m_startOffset(0.0), m_baselineOffset(0.0)
    , m_textAnchor( AnchorStart )
{
    setShapeId( SimpleTextShapeID );
    cacheGlyphOutlines();
    updateSizeAndPosition();
}

SimpleTextShape::~SimpleTextShape()
{
    if( m_path )
        m_path->removeDependee( this );
}

void SimpleTextShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    applyConversion( painter, converter );
    painter.setFont( m_font );
    if( background() )
        background()->paint( painter, outline() );
}

void SimpleTextShape::paintDecorations(QPainter &/*painter*/, const KoViewConverter &/*converter*/, const KoCanvasBase * /*canvas*/)
{
}

void SimpleTextShape::saveOdf(KoShapeSavingContext &context) const
{
    context.xmlWriter().startElement("draw:custom-shape");
    saveOdfAttributes( context, OdfAllAttributes );

    // now write the special shape data
    context.xmlWriter().addAttribute( "draw:engine", "svg:text" );

    // create the data attribute
    QString drawData = "text:" + m_text +';';
    drawData += "font-family:" + m_font.family() + ';';
    drawData += QString("font-size:%1pt;").arg( TO_PS_SIZE(m_font.pointSize()) );
    if( m_font.bold() )
        drawData += "font-weight:bold;";
    if( m_font.italic() )
        drawData += "font-style:italic;";

    qreal anchorOffset = 0.0;
    if( m_textAnchor == SimpleTextShape::AnchorMiddle )
    {
        anchorOffset += 0.5 * size().width();
        drawData += "text-anchor:middle;";
    }
    else if( m_textAnchor == SimpleTextShape::AnchorEnd )
    {
        anchorOffset += size().width();
        drawData += "text-anchor:end;";
    }

    // check if we are set on a path
    if( layout() == SimpleTextShape::OnPathShape )
    {
        /// TODO: we have to make sure that the path shape is saved before
        drawData += "textPath:" + context.drawId( m_path ) +';';
        drawData += QString( "startOffset:%1%;").arg( m_startOffset * 100.0 );
    }
    else if( layout() == SimpleTextShape::OnPath )
    {
        KoPathShape * baseline = KoPathShape::fromQPainterPath( m_baseline );
        QMatrix offsetMatrix;
        offsetMatrix.translate( 0.0, m_baselineOffset );
        drawData += "textPathData:" + baseline->toString( baseline->transformation() ) + ';';
        drawData += QString( "startOffset:%1%;").arg( m_startOffset * 100.0 );

        delete baseline;
    }

    context.xmlWriter().addAttribute( "draw:data", drawData );

    // write a enhanced geometry element for compatibility with other applications
    context.xmlWriter().startElement("draw:enhanced-geometry");

    // write the path data
    KoPathShape * path = KoPathShape::fromQPainterPath( outline() );
    context.xmlWriter().addAttribute("draw:enhanced-path", path->toString( transformation() ) );
    delete path;

    context.xmlWriter().endElement(); // draw:enhanced-geometry
    saveOdfCommonChildElements( context );
    context.xmlWriter().endElement(); // draw:custom-shape
}

bool SimpleTextShape::loadOdf( const KoXmlElement & element, KoShapeLoadingContext & context )
{
    QString drawEngine = element.attributeNS( KoXmlNS::draw, "engine", "" );
    if( drawEngine.isEmpty() || drawEngine != "svg:text" )
        return false;

    QString drawData = element.attributeNS( KoXmlNS::draw, "data" );
    if( drawData.isEmpty() )
        return false;

    QStringList properties = drawData.split( ';' );
    if( properties.count() == 0 )
        return false;

    foreach( QString property, properties )
    {
        QStringList pair = property.split( ':' );
        if( pair.count() != 2 )
            continue;
        if( pair[0] == "text" )
        {
            setText( pair[1] );
        }
        else if( pair[0] == "font-family" )
        {
            m_font.setFamily( pair[1] );
        }
        else if( pair[0] == "font-size" )
        {
            m_font.setPointSizeF( FROM_PS_SIZE( KoUnit::parseValue( pair[1], 12 ) ) );
        }
        else if( pair[0] == "font-weight" && pair[1] == "bold" )
        {
            m_font.setBold( true );
        }
        else if( pair[0] == "font-style" && pair[1] == "italic" )
        {
            m_font.setItalic( true );
        }
        else if( pair[0] == "textPathData" )
        {
            KoPathShape path;
            KoPathShapeLoader loader( &path );
            loader.parseSvg( pair[1], true );
            putOnPath( path.outline() );
        }
        else if( pair[0] == "textPath" )
        {
            KoPathShape * path = dynamic_cast<KoPathShape*>( context.shapeById( pair[1] ) ); 
            putOnPath( path );
        }
        else if( pair[0] == "startOffset" )
        {
            m_startOffset = 0.01 * pair[1].toDouble();
        }
        else if( pair[0] == "text-anchor" )
        {
            if( pair[1] == "middle" )
                m_textAnchor = AnchorMiddle;
            else if( pair[1] == "end" )
                m_textAnchor = AnchorEnd;
        }
    }

    cacheGlyphOutlines();
    updateSizeAndPosition();
    update();

    loadOdfAttributes( element, context, OdfAllAttributes );

    return true;
}

QSizeF SimpleTextShape::size() const
{
    return outline().boundingRect().size();
}

void SimpleTextShape::setSize( const QSizeF &newSize )
{
    QSizeF oldSize = size();
    if ( !oldSize.isNull() ) {
        qreal zoomX = newSize.width() / oldSize.width(); 
        qreal zoomY = newSize.height() / oldSize.height(); 
        QMatrix matrix( zoomX, 0, 0, zoomY, 0, 0 );

        update();
        applyTransformation( matrix );
        update();
    }
}

const QPainterPath SimpleTextShape::outline() const
{
    return m_outline;
}

void SimpleTextShape::createOutline()
{
    m_outline = QPainterPath();

    if( isOnPath() )
    {
        m_charOffsets.clear();
        QFontMetricsF metrics( m_font );
        int textLength = m_text.length();
        qreal charPos = m_startOffset * m_baseline.length();

        qreal anchorPoint = 0.0;
        if( m_textAnchor == AnchorMiddle )
            anchorPoint = 0.5 * metrics.width( m_text );
        else if( m_textAnchor == AnchorEnd )
            anchorPoint = metrics.width( m_text );

        charPos -= anchorPoint;

        QPointF pathPoint;

        m_charOffsets.resize( textLength + 1 );
        int charIdx = 0;
        for( ;charIdx < textLength; ++charIdx )
        {
            QString actChar( m_text[charIdx] );
            // get the percent value of the actual char position
            qreal t = m_baseline.percentAtLength( charPos );
            m_charOffsets[ charIdx ] = -1;
            if( t >= 1.0 )
                break;

            if( t >= 0.0 )
            {
                // get the path point of the given path position
                pathPoint = m_baseline.pointAtPercent( t );

                t = m_baseline.percentAtLength( charPos + 0.5 * metrics.width( actChar ) );
            }

            m_charOffsets[ charIdx ] = m_baseline.percentAtLength( charPos );
            charPos += metrics.width( actChar );
            if( t < 0.0 )
                continue;

            // get the angle at the given path position
            if( t >= 1.0 )
                break;
            qreal angle = m_baseline.angleAtPercent( t );

            QMatrix m;
            m.translate( pathPoint.x(), pathPoint.y() );
            m.rotate( 360. - angle );
            m_outline.addPath( m.map( m_charOutlines[charIdx] ) );
        }
        m_charOffsets[ charIdx ] = m_baseline.percentAtLength( charPos );
    }
    else
    {
        m_outline.addText( QPointF(), m_font, m_text );
    }
}

void SimpleTextShape::setText( const QString & text )
{
    if( m_text == text )
        return;

    update();
    m_text = text;
    cacheGlyphOutlines();
    updateSizeAndPosition();
    update();
}

QString SimpleTextShape::text() const
{
    return m_text;
}

void SimpleTextShape::setFont( const QFont & font )
{
    if( m_font == font )
        return;

    update();
    m_font = font;
    m_font.setPointSizeF( FROM_PS_SIZE( font.pointSizeF() ) );
    cacheGlyphOutlines();
    updateSizeAndPosition();
    update();
}

QFont SimpleTextShape::font() const
{
    QFont font( m_font );
    font.setPointSizeF( TO_PS_SIZE( font.pointSizeF() ) );
    return font;
}

void SimpleTextShape::setStartOffset( qreal offset )
{
    if( m_startOffset == offset )
        return;

    update();
    m_startOffset = offset;
    m_startOffset = qMin( qreal(1.0), m_startOffset );
    m_startOffset = qMax( qreal(0.0), m_startOffset );
    updateSizeAndPosition();
    update();
}

qreal SimpleTextShape::startOffset() const
{
    return m_startOffset;
}

qreal SimpleTextShape::baselineOffset() const
{
    return m_baselineOffset;
}

void SimpleTextShape::setTextAnchor( TextAnchor anchor )
{
    if( anchor == m_textAnchor )
        return;

    QFontMetricsF metrics( m_font );
    int length = metrics.width( m_text );
    qreal oldOffset = 0.0;
    if( m_textAnchor == AnchorMiddle )
        oldOffset = -0.5 * length;
    else if( m_textAnchor == AnchorEnd )
        oldOffset = -length;

    m_textAnchor = anchor;

    qreal newOffset = 0.0;
    if( m_textAnchor == AnchorMiddle )
        newOffset = -0.5 * length;
    else if( m_textAnchor == AnchorEnd )
        newOffset = -length;

    QMatrix m;
    m.translate( newOffset-oldOffset, 0.0 );

    update();
    updateSizeAndPosition();
    setTransformation( transformation() * m );
    update();
}

SimpleTextShape::TextAnchor SimpleTextShape::textAnchor() const
{
    return m_textAnchor;
}

bool SimpleTextShape::putOnPath( KoPathShape * path )
{
    if( ! path )
        return false;

    if( path->outline().isEmpty() )
        return false;

    update();

    m_path = path;
    m_path->addDependee( this );
    // use the paths outline converted to document coordinates as the baseline
    m_baseline = m_path->absoluteTransformation(0).map( m_path->outline() );

    // reset transformation
    setTransformation( QMatrix() );
    updateSizeAndPosition();
    // move to correct position
    setAbsolutePosition( m_outlineOrigin, KoFlake::TopLeftCorner );
    update();

    return true;
}

bool SimpleTextShape::putOnPath( const QPainterPath &path )
{
    if( path.isEmpty() )
        return false;

    update();
    if( m_path )
        m_path->removeDependee( this );
    m_path = 0;
    m_baseline = path;

    // reset transformation
    setTransformation( QMatrix() );
    updateSizeAndPosition();
    // move to correct position
    setAbsolutePosition( m_outlineOrigin, KoFlake::TopLeftCorner );
    update();

    return true;
}

void SimpleTextShape::removeFromPath()
{
    update();
    if( m_path )
        m_path->removeDependee( this );
    m_path = 0;
    m_baseline = QPainterPath();
    updateSizeAndPosition();
    update();
}

bool SimpleTextShape::isOnPath() const
{
    return (m_path != 0 || ! m_baseline.isEmpty() );
}

SimpleTextShape::LayoutMode SimpleTextShape::layout() const
{
    if( m_path )
        return OnPathShape;
    else if( ! m_baseline.isEmpty() )
        return OnPath;
    else
        return Straight;
}


QPainterPath SimpleTextShape::baseline() const
{
    return m_baseline;
}

const KoPathShape * SimpleTextShape::baselineShape() const
{
    return m_path;
}

QString SimpleTextShape::removeRange( unsigned int from, unsigned int nr )
{
    update();
    QString ret = m_text.mid( from, nr );
    m_text.remove( from, nr );
    cacheGlyphOutlines();
    updateSizeAndPosition();
    update();
    notifyChanged();
    return ret;
}

void SimpleTextShape::addRange( unsigned int index, const QString &str )
{
    update();
    m_text.insert( index, str );
    cacheGlyphOutlines();
    updateSizeAndPosition();
    update();
    notifyChanged();
}

void SimpleTextShape::getCharAngleAt( unsigned int charNum, qreal &angle ) const
{
    if( isOnPath() ) {
	qreal t = m_charOffsets[ qMin( int( charNum ), m_charOffsets.size() ) ];
        angle = m_baseline.angleAtPercent( t );
    } else {
        angle = 0.0;
    }
}

void SimpleTextShape::getCharPositionAt( unsigned int charNum, QPointF &pos ) const
{
    if( isOnPath() ) {
        qreal t = m_charOffsets[ qMin( int( charNum ), m_charOffsets.size() ) ];
        pos = m_baseline.pointAtPercent( t ) - m_outlineOrigin;
    } else {
        QFontMetrics metrics( m_font );
        uint l = m_text.length();
        if ( charNum >= l) {
            int w = metrics.width( m_text );
            pos = QPointF( w, size().height() );
        } else {
            int w = metrics.width( m_text.left( charNum + 1 ) );
	    int w2 = metrics.charWidth( m_text, charNum );
	    pos = QPointF( w - w2, size().height() );
	}
    }
}

void SimpleTextShape::getCharExtentsAt( unsigned int charNum, QRectF &extents ) const
{
    QFontMetrics metrics( m_font );
    int w = metrics.charWidth( m_text, qMin( int( charNum ), m_text.length() - 1 ) );
    extents = QRectF( 0, 0, w, metrics.height() );
}

void SimpleTextShape::updateSizeAndPosition( bool global )
{
    createOutline();

    QRectF bbox = m_outline.boundingRect();

    // calculate the offset we have to apply to keep our position
    QPointF offset = m_outlineOrigin - bbox.topLeft();

    // cache topleft corner of baseline path
    m_outlineOrigin = bbox.topLeft();

    if( isOnPath() )
    {
        // the outline position is in document coordinates
        // so we adjust our position
        QMatrix m;
        m.translate( -offset.x(), -offset.y() );
        global ? applyAbsoluteTransformation( m ) : applyTransformation( m );
    }
    else
    {
        // the text outlines baseline is at 0,0
        m_baselineOffset = -m_outlineOrigin.y();
    }

    setSize( bbox.size() );

    // map outline to shape coordinate system
    QMatrix normalizeMatrix;
    normalizeMatrix.translate( -m_outlineOrigin.x(), -m_outlineOrigin.y() );
    m_outline = normalizeMatrix.map( m_outline );
}

void SimpleTextShape::cacheGlyphOutlines()
{
    m_charOutlines.clear();

    int textLength = m_text.length();
    for( int charIdx = 0; charIdx < textLength; ++charIdx )
    {
        QString actChar( m_text[charIdx] );
        QPainterPath charOutline;
        charOutline.addText( QPointF(), m_font, actChar );
        m_charOutlines.append( charOutline );
    }
}

void SimpleTextShape::notifyShapeChanged( KoShape * shape, ChangeType type )
{
    if( shape == m_path )
    {
        if( type == KoShape::Deleted )
        {
            m_path = 0;
        }
        else
        {
            update();
            // use the paths outline converted to document coordinates as the baseline
            m_baseline = m_path->absoluteTransformation(0).map( m_path->outline() );
            updateSizeAndPosition( true );
            update();
        }
    }
}
