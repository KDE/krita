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
#include "libemf/EmfParser.h"
#include "libemf/EmfOutputPainterStrategy.h"


// Vector shape
#include "DefaultEmf.h"
#include "libemf/EmfParser.h"
#include "libemf/EmfOutputPainterStrategy.h"


VectorShape::VectorShape()
    : KoFrameShape( KoXmlNS::draw, "image" )
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

void VectorShape::draw(QPainter &painter)
{
    // FIXME: Make emfOutput use QSizeF
    QSize  sizeInt( size().width(), size().height() );
    kDebug(33100) << "-------------------------------------------";
    kDebug(33100) << "size:     " << sizeInt << size();
    kDebug(33100) << "position: " << position();
    kDebug(33100) << "-------------------------------------------";

    // If the data is uninitialized, e.g. because loading failed, draw a simple cross.
    if (m_size == 0) {
        QRectF  rect(QPointF(0,0), size());
        painter.setPen(QPen(QColor(172, 196, 206)));
        painter.drawRect(rect);
        painter.drawLine(rect.topLeft(), rect.bottomRight());
        painter.drawLine(rect.bottomLeft(), rect.topRight());

        return;
    }

    // FIXME: Make it static to save time?
    Libemf::Parser  emfParser;

    Libemf::OutputPainterStrategy  emfOutput( painter, sizeInt );
    emfParser.setOutput( &emfOutput );
    
    // At this point we have some data.  Now draw it.
#if 0
    QByteArray  emfArray(&defaultEMF[0], sizeof(defaultEMF));
#else
    QByteArray  emfArray(m_bytes, m_size);
#endif
    QBuffer     emfBuffer(&emfArray);
    emfBuffer.open(QIODevice::ReadOnly);

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
    //kDebug() <<"Loading ODF frame in the vector shape. Element = " << element.tagName();
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
    //kDebug() <<"Loading ODF element: " << element.tagName();

    // Get the reference to the vector file.  If there is no href, then just return.
    const QString href = element.attribute("href");
    if (href.isEmpty())
        return false;

    // Check if the contents is a .wmf.  So far we haven't found a
    // test that picks a emf but skips a wmf.  The file name could
    // give a clue.
    if (href.endsWith(".wmf"))
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


void VectorShape::setPrintable(bool on)
{
    if (m_printable == on)
        return;

    m_printable = on;
    update();
}

