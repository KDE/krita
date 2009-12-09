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
#include "KoStore.h"
#include "KoXmlNS.h"
#include "KoXmlReader.h"
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include "KoShapeSavingContext.h"
#include "libemf/EmfParser.h"
#include "libemf/EmfOutputPainterStrategy.h"


// EMF shape
#include "DefaultEmf.h"
#include "libemf/EmfParser.h"
#include "libemf/EmfOutputPainterStrategy.h"


EmfShape::EmfShape()
    : KoFrameShape( KoXmlNS::draw, "image" )
    , m_bytes(0)
    , m_size(0)
    , m_ownsBytes(false)
    , m_printable(true)
{
    setShapeId(EmfShape_SHAPEID);

   // Default size of the shape.
    KoShape::setSize( QSizeF( CM_TO_POINT( 8 ), CM_TO_POINT( 5 ) ) );
}

EmfShape::~EmfShape()
{
}

void  EmfShape::setEmfBytes( char *bytes, int size, bool takeOwnership )
{
    if (m_bytes != 0 && m_ownsBytes)
        delete []m_bytes;

    m_bytes = bytes;
    m_size = size;
    m_ownsBytes = takeOwnership;
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
    applyConversion(painter, converter);
    draw(painter);
}

void EmfShape::paintDecorations(QPainter &painter, const KoViewConverter &converter, const KoCanvasBase *canvas)
{
    Q_UNUSED(canvas);

    if (1 || !m_printable) {
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
    //kDebug() << "-------------------------------------------";
    //kDebug() << "size: " << sizeInt;
    //kDebug() << "-------------------------------------------";
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
    //kDebug() <<"Loading ODF frame in the EMF shape. Element = " << element.tagName();
    loadOdfAttributes(element, context, OdfAllAttributes);
    return loadOdfFrame(element, context);
}


inline static int read32(const char *buffer, const int offset)
{
    // little endian
    int result = (int) buffer[offset];
    result |= (int) buffer[offset+1] << 8;
    result |= (int) buffer[offset+2] << 16;
    result |= (int) buffer[offset+3] << 24;

    return result;
}

// Load the actual contents within the EMF shape.
bool EmfShape::loadOdfFrameElement( const KoXmlElement & element,
                                    KoShapeLoadingContext &context )
{
    //kDebug() <<"Loading ODF element: " << element.tagName();

    // Get the reference to the EMF file.  If there is no href, then just return.
    const QString href = element.attribute("href");
    if (href.isEmpty())
        return false;

    KoStore *store  = context.odfLoadingContext().store();
    bool     result = store->open(href);

    if (!result)
        return false;

    // Store the size and make a sanity check if this could be an EMF.
    // The size of the minimum EMF header record is 88.
    m_size = store->size();
    if (m_size < 88)
        return false;

    if (m_bytes && m_ownsBytes)
        delete []m_bytes;
    m_bytes = new char[m_size];

    qint64 bytesRead = store->read(m_bytes, m_size);
    store->close();
    if (bytesRead < m_size) {
        kDebug() << "Too few bytes read: " << bytesRead << " instead of " << m_size;
        return false;
    }

    // Check if the contents is actually an EMF.
    // 1. Check type
    qint32 mark = read32(m_bytes, 0);
    if (mark != 0x00000001) {
        //kDebug() << "Not an EMF: mark = " << mark << " instead of 0x00000001";
        return false;
    }

    // FIXME: We should probably do more checks here, but I don't know what right now.

    // Ok, we have an EMF.  Yay!
    return true;
}


void EmfShape::setPrintable(bool on)
{
    if (m_printable == on)
        return;

    m_printable = on;
    update();
}

