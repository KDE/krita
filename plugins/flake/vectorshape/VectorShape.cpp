/* This file is part of the KDE project
 *
 * Copyright (C) 2009 - 2011 Inge Wallin <inge@lysator.liu.se>
 * Copyright (C) 2011 Boudewijn Rempt <boud@valdyas.org>
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
#include <QFontDatabase>
#include <QPen>
#include <QPainter>
#include <QBuffer>
#include <QDataStream>
#include <QMutexLocker>
#include <QThreadPool>
#include <QSvgRenderer>

// Calligra
#include "KoUnit.h"
#include "KoStore.h"
#include "KoXmlNS.h"
#include "KoXmlReader.h"
#include "KoXmlWriter.h"
#include <KoEmbeddedDocumentSaver.h>
#include <KoShapeLoadingContext.h>
#include <KoOdfLoadingContext.h>
#include <KoShapeSavingContext.h>
#include <KoViewConverter.h>

// Wmf support
#include "WmfPainterBackend.h"

// Vector shape
#include "EmfParser.h"
#include "EmfOutputPainterStrategy.h"
#include "EmfOutputDebugStrategy.h"
#include "SvmParser.h"
#include "SvmPainterBackend.h"

// Comment out to get uncached painting, which is good for debugging
//#define VECTORSHAPE_PAINT_UNCACHED

// Comment out to get unthreaded painting, which is good for debugging
//#define VECTORSHAPE_PAINT_UNTHREADED

VectorShape::VectorShape()
    : KoFrameShape(KoXmlNS::draw, "image")
    , m_type(VectorTypeNone)
    , m_isRendering(false)
{
    setShapeId(VectorShape_SHAPEID);
    // Default size of the shape.
    KoShape::setSize(QSizeF(CM_TO_POINT(8), CM_TO_POINT(5)));
    m_cache.setMaxCost(3);
}

VectorShape::~VectorShape()
{
    // Wait for the render-thread to finish before the shape is allowed to be
    // destroyed so we can make sure to prevent crashes or unwanted
    // side-effects. Maybe as alternate we could just kill the render-thread...
    QMutexLocker locker(&m_mutex);
}

// Methods specific to the vector shape.
QByteArray  VectorShape::compressedContents() const
{
    return m_contents;
}

VectorShape::VectorType VectorShape::vectorType() const
{
    return m_type;
}

void VectorShape::setCompressedContents(const QByteArray &newContents, VectorType vectorType)
{
    QMutexLocker locker(&m_mutex);

    m_contents = newContents;
    m_type = vectorType;
    m_cache.clear();
    update();
}

RenderThread::RenderThread(const QByteArray &contents, VectorShape::VectorType type,
                           const QSizeF &size, const QSize &boundingSize, qreal zoomX, qreal zoomY)
    : QObject(), QRunnable(),
      m_contents(contents), m_type(type),
      m_size(size), m_boundingSize(boundingSize), m_zoomX(zoomX), m_zoomY(zoomY)
{
    setAutoDelete(true);
}

RenderThread::~RenderThread()
{
}

void RenderThread::run()
{
    QImage *image = new QImage(m_boundingSize, QImage::Format_ARGB32);
    image->fill(0);
    QPainter painter;
    if (!painter.begin(image)) {
        //kWarning(31000) << "Failed to create image-cache";
        delete image;
        image = 0;
    } else {
        painter.scale(m_zoomX, m_zoomY);
        draw(painter);
        painter.end();
    }
    emit finished(m_boundingSize, image);
}

void RenderThread::draw(QPainter &painter)
{
    // If the data is uninitialized, e.g. because loading failed, draw the null shape.
    if (m_contents.isEmpty()) {
        drawNull(painter);
        return;
    }

    // Actually draw the contents
    switch (m_type) {
    case VectorShape::VectorTypeWmf:
        drawWmf(painter);
        break;
    case VectorShape::VectorTypeEmf:
        drawEmf(painter);
        break;
    case VectorShape::VectorTypeSvm:
        drawSvm(painter);
        break;
    case VectorShape::VectorTypeSvg:
        drawSvg(painter);
        break;
    case VectorShape::VectorTypeNone:
    default:
        drawNull(painter);
    }
}

void RenderThread::drawNull(QPainter &painter) const
{
    QRectF  rect(QPointF(0, 0), m_size);
    painter.save();

    // Draw a simple cross in a rectangle just to indicate that there is something here.
    painter.setPen(QPen(QColor(172, 196, 206)));
    painter.drawRect(rect);
    painter.drawLine(rect.topLeft(), rect.bottomRight());
    painter.drawLine(rect.bottomLeft(), rect.topRight());

    painter.restore();
}

void RenderThread::drawWmf(QPainter &painter) const
{
    Libwmf::WmfPainterBackend  wmfPainter(&painter, m_size);
    if (!wmfPainter.load(m_contents)) {
        drawNull(painter);
        return;
    }
    painter.save();
    // Actually paint the WMF.
    wmfPainter.play();
    painter.restore();
}

void RenderThread::drawEmf(QPainter &painter) const
{
    // FIXME: Make emfOutput use QSizeF
    QSize  shapeSizeInt(m_size.width(), m_size.height());
    //kDebug(31000) << "-------------------------------------------";
    //kDebug(31000) << "size:     " << shapeSizeInt << m_size;
    //kDebug(31000) << "position: " << position();
    //kDebug(31000) << "-------------------------------------------";

    Libemf::Parser  emfParser;

#if 1  // Set to 0 to get debug output
    // Create a new painter output strategy.  Last param = true means keep aspect ratio.
    Libemf::OutputPainterStrategy  emfPaintOutput(painter, shapeSizeInt, true);
    emfParser.setOutput(&emfPaintOutput);
#else
    Libemf::OutputDebugStrategy  emfDebugOutput;
    emfParser.setOutput(&emfDebugOutput);
#endif
    emfParser.load(m_contents);
}

void RenderThread::drawSvm(QPainter &painter) const
{
    QSize  shapeSizeInt(m_size.width(), m_size.height());

    Libsvm::SvmParser  svmParser;

    // Create a new painter backend.
    Libsvm::SvmPainterBackend  svmPaintOutput(&painter, shapeSizeInt);
    svmParser.setBackend(&svmPaintOutput);
    svmParser.parse(m_contents);
}

void RenderThread::drawSvg(QPainter &painter) const
{
    QSvgRenderer renderer(m_contents);
    renderer.render(&painter, QRectF(0, 0, m_size.width(), m_size.height()));
}

void VectorShape::paint(QPainter &painter, const KoViewConverter &converter, KoShapePaintingContext &)
{
#ifdef VECTORSHAPE_PAINT_UNCACHED
    bool useCache = false;
#else
    bool useCache = true;
#endif

#ifdef VECTORSHAPE_PAINT_UNTHREADED
    bool asynchronous = false;
#else
    // Since the backends may use QPainter::drawText we need to make sure to only
    // use threads if the font-backend supports that what is in most cases.
    bool asynchronous = QFontDatabase::supportsThreadedFontRendering();
#endif

    QImage *cache = render(converter, asynchronous, useCache);
    if (cache) { // paint cached image
        Q_ASSERT(!cache->isNull());
        QVector<QRect> clipRects = painter.clipRegion().rects();
        foreach (const QRect &rc, clipRects) {
            painter.drawImage(rc.topLeft(), *cache, rc);
        }
    }
}

void VectorShape::renderFinished(QSize boundingSize, QImage *image)
{
    if (image) {
        m_cache.insert(boundingSize.height(), image);
        update();
    }
    m_isRendering = false;
}

// ----------------------------------------------------------------
//                         Loading and Saving

void VectorShape::saveOdf(KoShapeSavingContext &context) const
{
    QMutexLocker locker(&m_mutex);

    KoEmbeddedDocumentSaver &fileSaver = context.embeddedSaver();
    KoXmlWriter             &xmlWriter = context.xmlWriter();

    QString fileName = fileSaver.getFilename("VectorImages/Image");
    QByteArray mimeType;

    switch (m_type) {
    case VectorTypeWmf:
        mimeType = "image/x-wmf";
        break;
    case VectorTypeEmf:
        mimeType = "image/x-emf";
        break;
    case VectorTypeSvm:
        mimeType = "image/x-svm"; // mimetype as used inside LO/AOO
        break;
    case VectorTypeSvg:
        mimeType = "image/svg+xml";
    default:
        // FIXME: What here?
        mimeType = "application/x-what";
        break;
    }

    xmlWriter.startElement("draw:frame");
    saveOdfAttributes(context, OdfAllAttributes);
    fileSaver.embedFile(xmlWriter, "draw:image", fileName, mimeType, qUncompress(m_contents));
    xmlWriter.endElement(); // draw:frame
}

bool VectorShape::loadOdf(const KoXmlElement &element, KoShapeLoadingContext &context)
{
    //kDebug(31000) <<"Loading ODF frame in the vector shape. Element = " << element.tagName();
    loadOdfAttributes(element, context, OdfAllAttributes);
    return loadOdfFrame(element, context);
}

inline static int read32(const char *buffer, const int offset)
{
    // little endian
    int result = (int) buffer[offset];
    result |= (int) buffer[offset + 1] << 8;
    result |= (int) buffer[offset + 2] << 16;
    result |= (int) buffer[offset + 3] << 24;

    return result;
}

// Load the actual contents within the vector shape.
bool VectorShape::loadOdfFrameElement(const KoXmlElement &element,
                                      KoShapeLoadingContext &context)
{
    //kDebug(31000) <<"Loading ODF element: " << element.tagName();
    QMutexLocker locker(&m_mutex);

    // Get the reference to the vector file.  If there is no href, then just return.
    const QString href = element.attribute("href");
    if (href.isEmpty()) {
        return false;
    }

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
        //kDebug(31000) << "Too few bytes read: " << m_contents.count() << " instead of " << size;
        return false;
    }

    // Try to recognize the type.  We should do this before the
    // compression below, because that's a semi-expensive operation.
    m_type = vectorType(m_contents);

    // Return false if we didn't manage to identify the type.
    if (m_type == VectorTypeNone) {
        return false;
    }

    // Compress for biiiig memory savings.
    m_contents = qCompress(m_contents);

    return true;
}

void VectorShape::waitUntilReady(const KoViewConverter &converter, bool asynchronous) const
{
    render(converter, asynchronous, true);
}

QImage *VectorShape::render(const KoViewConverter &converter, bool asynchronous, bool useCache) const
{
    QRectF rect = converter.documentToView(boundingRect());
    int id = rect.size().toSize().height();
    QImage *cache = useCache ? m_cache[id] : 0;

    if (!cache || cache->isNull()) { // recreate the cached image
        cache = 0;
        if (!m_isRendering) {
            m_isRendering = true;
            qreal zoomX, zoomY;
            converter.zoom(&zoomX, &zoomY);
            QMutexLocker locker(&m_mutex);
            const QByteArray uncompressedContents =
                m_type != VectorShape::VectorTypeNone ? qUncompress(m_contents) : QByteArray();
            RenderThread *t = new RenderThread(uncompressedContents, m_type, size(), rect.size().toSize(), zoomX, zoomY);
            connect(t, SIGNAL(finished(QSize,QImage*)), this, SLOT(renderFinished(QSize,QImage*)));
            if (asynchronous) { // render and paint the image threaded
                QThreadPool::globalInstance()->start(t);
            } else { // non-threaded rendering and painting of the image
                t->run();
                cache = m_cache[id];
            }
        }
    }

    return cache;
}

VectorShape::VectorType VectorShape::vectorType(const QByteArray &newContents)
{
    VectorType vectorType;

    if (isWmf(newContents)) {
        vectorType = VectorShape::VectorTypeWmf;
    } else if (isEmf(newContents)) {
        vectorType = VectorShape::VectorTypeEmf;
    } else if (isSvm(newContents)) {
        vectorType = VectorShape::VectorTypeSvm;
    } else if (isSvg(newContents)) {
        vectorType = VectorShape::VectorTypeSvg;
    } else {
        vectorType = VectorShape::VectorTypeNone;
    }

    return vectorType;
}

bool VectorShape::isWmf(const QByteArray &bytes)
{
    //kDebug(31000) << "Check for WMF";

    const char *data = bytes.constData();
    const int   size = bytes.count();

    if (size < 10) {
        return false;
    }

    // This is how the 'file' command identifies a WMF.
    if (data[0] == '\327' && data[1] == '\315' && data[2] == '\306' && data[3] == '\232') {
        // FIXME: Is this a compressed wmf?  Check it up.
        //kDebug(31000) << "WMF identified: header 1";
        return true;
    }

    if (data[0] == '\002' && data[1] == '\000' && data[2] == '\011' && data[3] == '\000') {
        //kDebug(31000) << "WMF identified: header 2";
        return true;
    }

    if (data[0] == '\001' && data[1] == '\000' && data[2] == '\011' && data[3] == '\000') {
        //kDebug(31000) << "WMF identified: header 3";
        return true;
    }

    return false;
}

bool VectorShape::isEmf(const QByteArray &bytes)
{
    //kDebug(31000) << "Check for EMF";

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
            && data[40] == ' ' && data[41] == 'E' && data[42] == 'M' && data[43] == 'F') {
        //kDebug(31000) << "EMF identified";
        return true;
    }

    return false;
}

bool VectorShape::isSvm(const QByteArray &bytes)
{
    //kDebug(31000) << "Check for SVM";

    // Check the SVM signature.
    if (bytes.startsWith("VCLMTF")) {
        //kDebug(31000) << "SVM identified";
        return true;
    }

    return false;
}

bool VectorShape::isSvg(const QByteArray &bytes)
{
    //kDebug(31000) << "Check for SVG";
    return (bytes.contains("svg"));
}
