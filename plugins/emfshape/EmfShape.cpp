/* This file is part of the KDE project
 *
 * Copyright (C) 2009 Inge Wallin <inge@lysator.liu.se>
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


// Own
#include "EmfShape.h"

// Posix
#include <math.h>

// Qt
#include <QPen>
#include <QPainter>
#include <QByteArray>
#include <QBuffer>
#include <QDataStream>
#include <QPixmap>


// KDE
#include <KDebug>

// KOffice
#include "KoUnit.h"
#include "KoXmlNS.h"
// FIXME
#include "libemf/EmfParser.h"
#include "libemf/EmfOutputPainterStrategy.h"

// EMF shape
#include "DefaultEmf.h"


EmfShape::EmfShape()
    : KoFrameShape( KoXmlNS::draw, "object" ) // FIXME: Use something else(?)
    , m_bytes(0)
    , m_size(0)
    , m_printable(true)
{
    setShapeId(EmfShape_SHAPEID);

   // Default size of the shape.
    KoShape::setSize( QSizeF( CM_TO_POINT( 8 ), CM_TO_POINT( 5 ) ) );
}

EmfShape::~EmfShape()
{
}

void  EmfShape::setEmfBytes( char *bytes, int size )
{
    m_bytes = bytes;
    m_size  = size;
}

char *EmfShape::emfBytes()
{
    return m_bytes;
}

int   EmfShape::emfSize()
{
    return m_size;
}


void EmfShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    //if (m_printable) {
        applyConversion(painter, converter);
        draw(painter);
        //}
}

void EmfShape::paintDecorations(QPainter &painter, const KoViewConverter &converter, const KoCanvasBase *canvas)
{
    Q_UNUSED(canvas);

    if (!m_printable) {
        applyConversion(painter, converter);
        painter.setRenderHint(QPainter::Antialiasing);

        draw(painter);
    }
}

void EmfShape::draw(QPainter &painter)
{
    Libemf::Parser  emfParser;

    // FIXME: Make emfOutput use QSizeF
    QSize  sizeInt( size().width(), size().height() );
    kDebug() << "-------------------------------------------";
    kDebug() << "size: " << sizeInt;
    kDebug() << "-------------------------------------------";
    Libemf::OutputPainterStrategy  emfOutput( painter, sizeInt );
    emfParser.setOutput( &emfOutput );
    
    // FIXME: Use the actual bytes.
    QByteArray  emfArray( &defaultEMF[0], sizeof(defaultEMF) );
    QBuffer     emfBuffer( &emfArray );
    emfBuffer.open( QIODevice::ReadOnly );

    QDataStream  emfStream;
    emfStream.setDevice( &emfBuffer );
    emfStream.setByteOrder( QDataStream::LittleEndian );

    emfParser.loadFromStream( emfStream );

    return;

    // Old code, not in use any more.
    QPixmap pixmap = QPixmap::fromImage( *(emfOutput.image()) );
    painter.drawPixmap( 0, 0, 
                        pixmap.scaled( int(size().width()), int(size().height()),
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation ) );
    return;

    // Draw a cross
    QRectF  rect(QPointF(0,0), size());
    painter.setPen(QPen(QColor(172, 196, 206)));
    painter.drawRect(rect);
    painter.drawLine(rect.topLeft(), rect.bottomRight());
    painter.drawLine(rect.bottomLeft(), rect.topRight());
}

void EmfShape::saveOdf(KoShapeSavingContext & context) const
{
    Q_UNUSED(context);

    // FIXME: NYI
}

bool EmfShape::loadOdf( const KoXmlElement & element, KoShapeLoadingContext &context )
{
    kDebug() <<"Loading ODF frame in the EMF shape";
    loadOdfAttributes(element, context, OdfAllAttributes);
    return loadOdfFrame(element, context);
}

// Load the actual contents within the EMF shape.
bool EmfShape::loadOdfFrameElement( const KoXmlElement & element,
                                    KoShapeLoadingContext &/*context*/ )
{
    kDebug() <<"Loading ODF frame contents in the EMF shape";

    // FIXME
    return false;
}


void EmfShape::setPrintable(bool on)
{
    if (m_printable == on)
        return;

    m_printable = on;
    update();
}

