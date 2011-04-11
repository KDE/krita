/* This file is part of the KDE project
 *
 * Copyright (C) 2009 - 2011 Inge Wallin <inge@lysator.liu.se>
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
#include <KoShapeSavingContext.h>
#include <KoViewConverter.h>

#include "kowmfpaint.h"

// Vector shape
#include "libemf/EmfParser.h"
#include "libemf/EmfOutputPainterStrategy.h"
#include "libemf/EmfOutputDebugStrategy.h"


VectorShape::VectorShape()
    : KoFrameShape( KoXmlNS::draw, "image" )
    , m_type(VectorTypeNone)
    , m_currentZoomLevel(-1)
{
    setShapeId(VectorShape_SHAPEID);
   // Default size of the shape.
    KoShape::setSize( QSizeF( CM_TO_POINT( 8 ), CM_TO_POINT( 5 ) ) );
}

VectorShape::~VectorShape()
{
}

VectorShape::VectorType VectorShape::vectorType() const
{
    return m_type;
}


void VectorShape::paint(QPainter &painter, const KoViewConverter &converter)
{
    if (m_cache.isNull() || !qFuzzyCompare(m_currentZoomLevel, converter.zoom())) {
        // create the cache image
        QRectF rc = converter.documentToView(boundingRect());
        m_cache = QImage(rc.size().toSize(), QImage::Format_ARGB32);
        QPainter gc(&m_cache);
        applyConversion(gc, converter);
        draw(gc);
        m_currentZoomLevel = converter.zoom();
    }
    painter.drawImage(0, 0, m_cache);
}

void VectorShape::draw(QPainter &painter) const
{
    // If the data is uninitialized, e.g. because loading failed, draw the null shape
    if (m_contents.count() == 0) {
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
    // Debug
    //drawNull(painter);

    KoWmfPaint  wmfPainter;

    if (!wmfPainter.load(m_contents)) {
        drawNull(painter);
        return;
    }

    painter.save();

    // Position the bitmap to the right place and resize it to fit.
    QRect   wmfBoundingRect = wmfPainter.boundingRect(); // Not QRectF because a wmf contains only ints.
    QSizeF  shapeSize       = size();

#if DEBUG_VECTORSHAPE
    kDebug(31000) << "-------------------------------- Starting WMF --------------------------------";
    kDebug(31000) << "wmfBoundingRect: " << wmfBoundingRect;
    kDebug(31000) << "shapeSize: "       << shapeSize;
#endif

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
    QSize  shapeSizeInt( size().width(), size().height() );
    //kDebug(31000) << "-------------------------------------------";
    //kDebug(31000) << "size:     " << shapeSizeInt << size();
    //kDebug(31000) << "position: " << position();
    //kDebug(31000) << "-------------------------------------------";

    // Create a QBuffer to read from...
    QBuffer     emfBuffer((QByteArray  *)&m_contents, 0);
    emfBuffer.open(QIODevice::ReadOnly);

    // ...but what we really want is a stream.
    QDataStream  emfStream;
    emfStream.setDevice(&emfBuffer);
    emfStream.setByteOrder(QDataStream::LittleEndian);

    // FIXME: Make it static to save time?
    Libemf::Parser  emfParser;
#if 1
    // Create a new painter output strategy.  Last param = true means keep aspect ratio.
    Libemf::OutputPainterStrategy  emfPaintOutput( painter, shapeSizeInt, true );
    emfParser.setOutput( &emfPaintOutput );
#else
    Libemf::OutputDebugStrategy  emfDebugOutput;
    emfParser.setOutput( &emfDebugOutput );
#endif
    emfParser.loadFromStream(emfStream);

    return;
}

void VectorShape::saveOdf(KoShapeSavingContext & context) const
{
    Q_UNUSED(context);
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

    if (!result) {
        return false;
    }

    int size = store->size();
    if (size < 88) {
        store->close();
        return false;
    }

    m_contents = store->read(size);
    store->close();
    if (m_contents.count() < size) {
        kDebug(31000) << "Too few bytes read: " << m_contents.count() << " instead of " << size;
        return false;
    }

    if (isWmf(m_contents))
        m_type = VectorTypeWmf;
    else if (isEmf(m_contents))
        m_type = VectorTypeEmf;
    else
        m_type = VectorTypeNone;


    // Return true if we managed to identify the type.
    return m_type != VectorTypeNone;
}


bool VectorShape::isWmf(const QByteArray &bytes) const
{
    kDebug(31000) << "Check for WMF";

    const char *data = bytes.constData();
    const int   size = bytes.count();

    if (size < 10)
        return false;

    // This is how the 'file' command identifies a WMF.
    if (data[0] == '\327' && data[1] == '\315' && data[2] == '\306' && data[3] == '\232')
    {
        // FIXME: Is this a compressed wmf?  Check it up.
        kDebug(31000) << "WMF identified: header 1";
        return true;
    }

    if (data[0] == '\002' && data[1] == '\000' && data[2] == '\011' && data[3] == '\000')
    {
        kDebug(31000) << "WMF identified: header 2";
        return true;
    }

    if (data[0] == '\001' && data[1] == '\000' && data[2] == '\011' && data[3] == '\000')
    {
        kDebug(31000) << "WMF identified: header 3";
        return true;
    }

    return false;
}

bool VectorShape::isEmf(const QByteArray &bytes) const
{
    kDebug(31000) << "Check for EMF";

    const char *data = bytes.constData();
    const int   size = bytes.count();

    // This is how the 'file' command identifies an EMF.
    // 1. Check type
    qint32 mark = read32(data, 0);
    if (mark != 0x00000001) {
        //kDebug(31000) << "Not an EMF: mark = " << mark << " instead of 0x00000001";
        return false;
    }

    // 2. An EMF has the string " EMF" at the start + offset 40.
    if (size > 44
        && data[40] == ' ' && data[41] == 'E' && data[42] == 'M' && data[43] == 'F')
    {
        kDebug(31000) << "EMF identified";
        return true;
    }

    return false;
}
