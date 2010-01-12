/* This file is part of the KDE project
 *
 * Copyright (C) 2009 - 2010 Inge Wallin <inge@lysator.liu.se>
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
#include "VectorShape.h"

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
#include <WmfPainter.h>

// Vector shape
#include "libemf/EmfParser.h"
#include "libemf/EmfOutputPainterStrategy.h"


VectorShape::VectorShape()
    : KoFrameShape( KoXmlNS::draw, "image" )
    , m_type(VectorTypeNone)
    , m_bytes(0)
    , m_size(0)
    , m_ownsBytes(false)
    , m_printable(true)
{
    setShapeId(VectorShape_SHAPEID);

   // Default size of the shape.
    KoShape::setSize( QSizeF( CM_TO_POINT( 8 ), CM_TO_POINT( 5 ) ) );
}

VectorShape::~VectorShape()
{
}

void  VectorShape::setVectorBytes( char *bytes, int size, bool takeOwnership )
{
    if (m_bytes != 0 && m_ownsBytes)
        delete []m_bytes;

    m_bytes = bytes;
    m_size = size;
    m_ownsBytes = takeOwnership;
}

char *VectorShape::vectorBytes()
{
    return m_bytes;
}

int VectorShape::vectorSize()
{
    return m_size;
}


void VectorShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    applyConversion(painter, converter);
    draw(painter);
}

void VectorShape::paintDecorations(QPainter &painter, const KoViewConverter &converter,
                                   const KoCanvasBase *canvas)
{
    Q_UNUSED(canvas);

    if (1 || !m_printable) {
        applyConversion(painter, converter);
        painter.setRenderHint(QPainter::Antialiasing);

        draw(painter);
    }
}

void VectorShape::draw(QPainter &painter) const
{
    // If the data is uninitialized, e.g. because loading failed, draw the null shape
    if (m_size == 0) {
        drawNull(painter);
        return;
    }

    // Actually draw the contents
    switch (m_type) {
    case VectorTypeNone:
        drawNull(painter);
        break;
    case VectorTypeWmf:
        drawWmf(painter);
        break;
    case VectorTypeEmf:
        drawEmf(painter);
        break;
    default:
        drawNull(painter);
    }
}

void VectorShape::drawNull(QPainter &painter) const
{
    QRectF  rect(QPointF(0,0), size());
    painter.save();

    // Draw a simple cross in a rectangle just to indicate that there is something here.
    painter.setPen(QPen(QColor(172, 196, 206)));
    painter.drawRect(rect);
    painter.drawLine(rect.topLeft(), rect.bottomRight());
    painter.drawLine(rect.bottomLeft(), rect.topRight());

    painter.restore();
}

void VectorShape::drawWmf(QPainter &painter) const
{
    //drawNull(painter);

    WmfPainter  wmfPainter;
    QByteArray  emfArray(m_bytes, m_size);

    // FIXME: Switch name from emfArray
    if (!wmfPainter.load(emfArray)) {
        drawNull(painter);
        return;
    }

    painter.save();

    // Position the bitmap to the right place and resize it to fit.
    QRect   wmfBoundingRect = wmfPainter.boundingRect(); // FIXME: Should this be made QRectF?
    QSizeF  shapeSize       = size();

    //kDebug(31000) << "wmfBoundingRect: " << wmfBoundingRect;
    //kDebug(31000) << "shapeSize: "       << shapeSize;

    // Create a transformation that makes the Wmf fit perfectly into the shape size.
    painter.scale(shapeSize.width() / wmfBoundingRect.width(),
                  shapeSize.height() / wmfBoundingRect.height());
    painter.translate(-wmfBoundingRect.left(), -wmfBoundingRect.top());

    // Actually paint the WMF.
    wmfPainter.play(painter, true);
    painter.restore();
}

void VectorShape::drawEmf(QPainter &painter) const
{
    // FIXME: Make emfOutput use QSizeF
    QSize  sizeInt( size().width(), size().height() );
    //kDebug(31000) << "-------------------------------------------";
    //kDebug(31000) << "size:     " << sizeInt << size();
    //kDebug(31000) << "position: " << position();
    //kDebug(31000) << "-------------------------------------------";

    // FIXME: Make it static to save time?
    Libemf::Parser  emfParser;

    Libemf::OutputPainterStrategy  emfOutput( painter, sizeInt );
    emfParser.setOutput( &emfOutput );
    
    // Create a QBuffer to read from...
    QByteArray  emfArray(m_bytes, m_size);
    QBuffer     emfBuffer(&emfArray);
    emfBuffer.open(QIODevice::ReadOnly);

    // ...but what we really want is a stream.
    QDataStream  emfStream;
    emfStream.setDevice(&emfBuffer);
    emfStream.setByteOrder(QDataStream::LittleEndian);

    // This does the actual painting.
    emfParser.loadFromStream(emfStream);

    return;
}

void VectorShape::saveOdf(KoShapeSavingContext & context) const
{
    Q_UNUSED(context);

    // FIXME: NYI
}

bool VectorShape::loadOdf(const KoXmlElement & element, KoShapeLoadingContext &context)
{
    //kDebug(31000) <<"Loading ODF frame in the vector shape. Element = " << element.tagName();
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

// Load the actual contents within the vector shape.
bool VectorShape::loadOdfFrameElement(const KoXmlElement & element,
                                      KoShapeLoadingContext &context)
{
    //kDebug(31000) <<"Loading ODF element: " << element.tagName();

    // Get the reference to the vector file.  If there is no href, then just return.
    const QString href = element.attribute("href");
    if (href.isEmpty())
        return false;

    // Try to open the embedded file.
    KoStore *store  = context.odfLoadingContext().store();
    bool     result = store->open(href);

    if (!result)
        return false;

    // Store the size and make a sanity check.
    // The size of the minimum EMF header record is 88.
    m_size = store->size();
    if (m_size < 88) {
        m_size = 0;
        return false;
    }

    if (m_bytes && m_ownsBytes) {
        delete []m_bytes;
        m_bytes = 0;
    }
    m_bytes = new char[m_size];
    m_ownsBytes = true;

    qint64 bytesRead = store->read(m_bytes, m_size);
    store->close();
    if (bytesRead < m_size) {
        kDebug(31000) << "Too few bytes read: " << bytesRead << " instead of " << m_size;
        return false;
    }

    if (isWmf())
        m_type = VectorTypeWmf;
    else if (isEmf())
        m_type = VectorTypeEmf;
    else
        m_type = VectorTypeNone;
    
    // Return true if we managed to identify the type.
    return m_type != VectorTypeNone;
}


bool VectorShape::isWmf() const
{
    kDebug(31000) << "Check for WMF";

    if (m_size < 10)
        return false;

    // This is how the 'file' command identifies a WMF.
    if (m_bytes[0] == '\327' && m_bytes[1] == '\315' && m_bytes[2] == '\306' && m_bytes[3] == '\232')
    {
        // FIXME: Is this a compressed wmf?  Check it up.
        kDebug(31000) << "WMF identified: header 1";
        return true;
    }

    if (m_bytes[0] == '\002' && m_bytes[1] == '\000' && m_bytes[2] == '\011' && m_bytes[3] == '\000')
    {
        kDebug(31000) << "WMF identified: header 2";
        return true;
    }

    if (m_bytes[0] == '\001' && m_bytes[1] == '\000' && m_bytes[2] == '\011' && m_bytes[3] == '\000')
    {
        kDebug(31000) << "WMF identified: header 3";
        return true;
    }

    return false;
}

bool VectorShape::isEmf() const
{
    kDebug(31000) << "Check for EMF";

    // This is how the 'file' command identifies an EMF.
    // 1. Check type
    qint32 mark = read32(m_bytes, 0);
    if (mark != 0x00000001) {
        //kDebug(31000) << "Not an EMF: mark = " << mark << " instead of 0x00000001";
        return false;
    }

    // 2. An EMF has the string " EMF" at the start + offset 40.
    if (m_size > 44 && m_bytes[40] == ' '
        && m_bytes[41] == 'E' && m_bytes[42] == 'M' && m_bytes[43] == 'F')
    {
        kDebug(31000) << "EMF identified";
        return true;
    }

    return false;
}


void VectorShape::setPrintable(bool on)
{
    if (m_printable == on)
        return;

    m_printable = on;
    update();
}

