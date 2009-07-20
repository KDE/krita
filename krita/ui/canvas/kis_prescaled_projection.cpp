/*
 *  Copyright (c) 2007, Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008, Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_prescaled_projection.h"


#include <QImage>
#include <QPixmap>
#include <QColor>
#include <QRect>
#include <QPoint>
#include <QSize>
#include <QPainter>
#include <QTimer>

#include <qimageblitz.h>

#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KoViewConverter.h>

#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_mask.h"
#include "kis_node.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_selection.h"
#include "kis_selection_mask.h"
#include "kis_types.h"
#include "kis_projection_cache.h"

#define EPSILON 1e-6


inline void copyQImageBuffer(uchar* dst, const uchar* src , qint32 deltaX, qint32 width)
{
    if (deltaX >= 0) {
        memcpy(dst + 4 * deltaX, src, 4 * (width - deltaX) * sizeof(uchar));
    } else {
        memcpy(dst, src - 4 * deltaX, 4 * (width + deltaX) * sizeof(uchar));
    }
}

void copyQImage(qint32 deltaX, qint32 deltaY, QImage* dstImg, const QImage& srcImg)
{
    qint32 height = dstImg->height();
    qint32 width = dstImg->width();
    Q_ASSERT(dstImg->width() == srcImg.width() && dstImg->height() == srcImg.height());
    if (deltaY >= 0) {
        for (int y = 0; y < height - deltaY; y ++) {
            const uchar* src = srcImg.scanLine(y);
            uchar* dst = dstImg->scanLine(y + deltaY);
            copyQImageBuffer(dst, src, deltaX, width);
        }
    } else {
        for (int y = 0; y < height + deltaY; y ++) {
            const uchar* src = srcImg.scanLine(y - deltaY);
            uchar* dst = dstImg->scanLine(y);
            copyQImageBuffer(dst, src, deltaX, width);
        }
    }
}

struct KisPrescaledProjection::Private {
    Private()
        : useNearestNeighbour( false )
        , usePixmapNotQImage(false)
        , drawCheckers(false)
        , scrollCheckers(false)
        , drawMaskVisualisationOnUnscaledCanvasCache(false)
        , showMask(true)
        , checkSize(32)
        , documentOffset(0, 0)
        , documentOrigin(0, 0)
        , canvasSize(0, 0)
        , imageSize(0, 0)
        , viewConverter(0)
        , monitorProfile( 0 )
        , counter( 0 )
    {
    }

    bool useNearestNeighbour;
    bool usePixmapNotQImage;
    bool drawCheckers;
    bool scrollCheckers;
    bool drawMaskVisualisationOnUnscaledCanvasCache;
    bool showMask;

    QColor checkersColor;
    qint32 checkSize;
    QImage prescaledQImage;
    QPixmap prescaledPixmap;

    
    QPoint documentOffset; // in view pixels
    QPoint documentOrigin; // in view pixels too 

    QSize canvasSize; // in view pixels
    QSize imageSize; // in kisimage pixels
    KisImageSP image;
    KoViewConverter * viewConverter;
    KisNodeSP currentNode;
    QRegion rectsToSmooth;
    KisProjectionCache projectionCache;
    const KoColorProfile* monitorProfile;
    int counter;
};

KisPrescaledProjection::KisPrescaledProjection()
    : QObject(0)
    , m_d(new Private())
{
    updateSettings();
    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), this, SLOT(updateSettings()));
}

KisPrescaledProjection::~KisPrescaledProjection()
{
    delete m_d;
}


void KisPrescaledProjection::setImage(KisImageSP image)
{
    Q_ASSERT(image);
    m_d->image = image;
    m_d->projectionCache.setImage( image );
    setImageSize(image->width(), image->height());
}


bool KisPrescaledProjection::drawCheckers() const
{
    return m_d->drawCheckers;
}


void KisPrescaledProjection::setDrawCheckers(bool drawCheckers)
{
    m_d->drawCheckers = drawCheckers;
}

QPixmap KisPrescaledProjection::prescaledPixmap() const
{
    return m_d->prescaledPixmap;
}

QImage KisPrescaledProjection::prescaledQImage() const
{
    return m_d->prescaledQImage;
}

void KisPrescaledProjection::setViewConverter(KoViewConverter * viewConverter)
{
    m_d->viewConverter = viewConverter;
}

void KisPrescaledProjection::updateSettings()
{
    KisConfig cfg;

    m_d->useNearestNeighbour = cfg.useNearestNeighbour();
    m_d->usePixmapNotQImage = cfg.noXRender();
    m_d->projectionCache.setCacheKisImageAsQImage( cfg.cacheKisImageAsQImage() );

    // If any of the above are true, we don't use our own smooth scaling
    m_d->scrollCheckers = cfg.scrollCheckers();
    m_d->checkSize = cfg.checkSize();
    m_d->checkersColor = cfg.checkersColor();

    m_d->drawMaskVisualisationOnUnscaledCanvasCache = cfg.drawMaskVisualisationOnUnscaledCanvasCache();

    setMonitorProfile( KoColorSpaceRegistry::instance()->profileByName(cfg.monitorProfile()) );

}


void KisPrescaledProjection::documentOffsetMoved(const QPoint &documentOffset)
{
    dbgRender << "documentOffsetMoved " << m_d->documentOffset << ", to " << documentOffset;
    QPoint oldDocumentOffset = m_d->documentOffset;
    m_d->documentOffset = documentOffset;

    // We've called documentOffsetMoved before even updating the projection
    if (m_d->prescaledQImage.isNull()) {

        return;
    }

    qint32 width = m_d->prescaledQImage.width();
    qint32 height = m_d->prescaledQImage.height();

    QRegion exposedRegion = QRect(0, 0, width, height);

    qint32 oldCanvasXOffset = oldDocumentOffset.x();
    qint32 oldCanvasYOffset = oldDocumentOffset.y();

    dbgRender << "w: " << width << ", h" << height << ", oldCanvasXOffset " << oldCanvasXOffset << ", oldCanvasYOffset " << oldCanvasYOffset
              << ", new offset: " << documentOffset;

    QImage img = QImage(width, height, QImage::Format_ARGB32);
    QPainter gc(&img);

    if (oldCanvasXOffset != m_d->documentOffset.x() || oldCanvasYOffset != m_d->documentOffset.y()) {

        qint32 deltaX =  oldCanvasXOffset - m_d->documentOffset.x();
        qint32 deltaY = oldCanvasYOffset - m_d->documentOffset.y();

        dbgRender << "deltaX: " << deltaX << ", deltaY: " << deltaY;
        if (qAbs(deltaX) < width && qAbs(deltaY) < height) {
            dbgRender << "Copy old data";
            copyQImage(deltaX, deltaY, &img, m_d->prescaledQImage);

            dbgRender << "exposedRegion: " << exposedRegion;
            dbgRender << "Preexistant data: " << QRegion(QRect(deltaX, deltaY, width , height));
            exposedRegion -= QRegion(QRect(deltaX, deltaY, width , height));
            dbgRender << "exposedRegion: " << exposedRegion;
        }
        dbgRender << "exposedRegion: " << exposedRegion;
        if (!exposedRegion.isEmpty()) {

            QVector<QRect> rects = exposedRegion.rects();

            for (int i = 0; i < rects.count(); i++) {
                QRect r = rects[i];
                // Set the areas to empty. Who knows, there may be not
                // enough image to draw in them.
                gc.fillRect(r, QColor(0, 0, 0, 0));
                dbgRender << "render on rect" << r;
                drawScaledImage(r , gc);
            }
        }
        m_d->prescaledQImage = img;
    } else {
        // Don't do "anything" if this function is called while the offset hasn't changed
        // because that usually happen when the scrollbars appears
        dbgRender << "Document Offset Moved but without moving !";
    }
}

void KisPrescaledProjection::setImageSize(qint32 w, qint32 h)
{

    m_d->projectionCache.setImageSize( w, h );
    m_d->imageSize = QSize(w, h);

    QRect vRect = viewRectFromImagePixels(QRect( 0, 0, w, h ));
    if (!vRect.isEmpty()) {
        preScale(vRect);
    }
}



void KisPrescaledProjection::updateCanvasProjection(const QRect & rc)
{
    if (!m_d->image) {
        dbgRender << "Calling updateCanvasProjection without an image: " << kBacktrace() << endl;
        return;
    }

    m_d->projectionCache.updateUnscaledCache( rc );

    QRect vRect = viewRectFromImagePixels(rc);

    dbgRender << "vRect = " << vRect;
    if (!vRect.isEmpty()) {
        preScale(vRect);
    }

}


void KisPrescaledProjection::setMonitorProfile(const KoColorProfile * profile)
{
    m_d->monitorProfile = profile;
    m_d->projectionCache.setMonitorProfile( profile );
}

void KisPrescaledProjection::setCurrentNode(const KisNodeSP node)
{
    m_d->currentNode = node;
}

void KisPrescaledProjection::showCurrentMask(bool showMask)
{
    m_d->showMask = showMask;
}


void KisPrescaledProjection::preScale()
{
    Q_ASSERT(m_d->canvasSize.isValid());
    preScale(QRect(QPoint(0, 0), m_d->canvasSize));
}

void KisPrescaledProjection::preScale(const QRect & rc)
{

    if (!rc.isEmpty()) {
        QPainter gc(&m_d->prescaledQImage);
        gc.setCompositionMode(QPainter::CompositionMode_Source);
        gc.fillRect(rc, QColor(0, 0, 0, 0));
        drawScaledImage(rc, gc);
    }

}

void KisPrescaledProjection::resizePrescaledImage(const QSize & newSize)
{

    QSize oldSize;

    if (m_d->prescaledQImage.isNull()) {
        oldSize = QSize(0, 0);
    } else {
        oldSize = m_d->prescaledQImage.size();
    }

    dbgRender << "KisPrescaledProjection::resizePrescaledImage from " << oldSize << " to " << newSize << endl;

    QImage img = QImage(newSize, QImage::Format_ARGB32);
//     m_d->prescaledQImage.fill( QColor( 255, 0, 0, 128 ).rgba() );

// Let someone else figure out the optimization where we copy the
// still visible part of the image after moving the offset and then
// only draw the newly visible parts
#if 0
    QPainter gc(&img);
    gc.setCompositionMode(QPainter::CompositionMode_Source);

    if (newSize.width() > oldSize.width() || newSize.height() > oldSize.height()) {

        gc.drawImage(0, 0,
                     m_d->prescaledQImage,
                     0, 0, m_d->prescaledQImage.width(), m_d->prescaledQImage.height());

        QRegion r(QRect(0, 0, newSize.width(), newSize.height()));
        r -= QRegion(QRect(0, 0, m_d->prescaledQImage.width(), m_d->prescaledQImage.height()));
        foreach(QRect rc, r.rects()) {
            drawScaledImage(rc, gc);
        }
    } else {
        gc.drawImage(0, 0, m_d->prescaledQImage,
                     0, 0, m_d->prescaledQImage.width(), m_d->prescaledQImage.height());
    }
#endif

#if 1
    m_d->prescaledQImage = img;
    m_d->canvasSize = newSize;
    preScale();
#endif

}

void KisPrescaledProjection::drawScaledImage(const QRect & rc,  QPainter & gc)
{
    if (!m_d->image)
        return;

    Q_ASSERT(m_d->viewConverter);

    // get the x and y zoom level
    qreal zoomX, zoomY;
    m_d->viewConverter->zoom(&zoomX, &zoomY);

    // Get the KisImage resolution
    qreal xRes = m_d->image->xRes();
    qreal yRes = m_d->image->yRes();

    // Compute the scale factors
    qreal scaleX = zoomX / xRes;
    qreal scaleY = zoomY / yRes;

    // Size of the image in KisImage pixels
    QSize imagePixelSize(m_d->image->width(), m_d->image->height());

    // compute how large a fully scaled image is in viewpixels
    QSize dstSize = QSize(int(imagePixelSize.width() * scaleX), int(imagePixelSize.height() * scaleY));

    // Don't go outside the image (will crash the sampleImage method below)
    QRect drawRect = rc.translated(m_d->documentOffset).intersected(QRect(QPoint(0, 0), dstSize));

    // Go from the widget coordinates to points
    QRectF imageRect = m_d->viewConverter->viewToDocument(rc.translated(m_d->documentOffset));

    // Go from points to view pixels
    imageRect.setCoords(imageRect.left() * xRes, imageRect.top() * yRes,
                        imageRect.right() * xRes, imageRect.bottom() * yRes);

    // Don't go outside the image
    QRect alignedImageRect = imageRect.intersected(m_d->image->bounds()).toAlignedRect();
    alignedImageRect.setBottom(alignedImageRect.bottom() + 1);  // <= it's a work around in a Qt bug (bouuuh) (reported to them see http://trolltech.com/developer/task-tracker/index_html?method=entry&id=187008 ) the returned .toAlignedRect() is one pixel too small
    alignedImageRect.setRight(alignedImageRect.right() + 1);

    // the size of the rect after scaling
    QSize scaledSize = QSize((int)(alignedImageRect.width() * scaleX), (int)(alignedImageRect.height() * scaleY));

    // Compute the corresponding area on the canvas
    QRectF rcFromAligned;
    rcFromAligned.setCoords(alignedImageRect.left(), alignedImageRect.top(),
                            alignedImageRect.right(), alignedImageRect.bottom());
    rcFromAligned.setCoords(alignedImageRect.left() / xRes, alignedImageRect.top() / yRes,
                            (alignedImageRect.right() /*+ 1.0*/) / xRes, (alignedImageRect.bottom() /*+ 1.0*/) / yRes);
    m_d->viewConverter->documentToViewY(rcFromAligned.height());
    rcFromAligned = m_d->viewConverter->documentToView(rcFromAligned).translated(-m_d->documentOffset) ;

    //m_d->projectionCache.updateUnscaledCache(alignedImageRect);

    QPointF rcTopLeftUnscaled(rcFromAligned.topLeft().x() / scaleX, rcFromAligned.topLeft().y() / scaleY);

#if 1
    dbgRender << "#####################################";
    dbgRender << " xRed = " << xRes << " yRes = " << yRes;
    dbgRender << " m_d->viewConverter->documentToView( imageRect ) = " << m_d->viewConverter->documentToView(imageRect);
    dbgRender << " rc.translated( m_d->documentOffset ) = " << rc.translated(m_d->documentOffset);
    dbgRender << " m_d->viewConverter->documentToView( m_d->viewConverter->viewToDocument( rc ) ) = " << m_d->viewConverter->documentToView(m_d->viewConverter->viewToDocument(rc));
    dbgRender << " imageRect = " << imageRect << " " << imageRect.bottomRight() << " " << imageRect.right() << " , " << imageRect.bottom();
    dbgRender << " imageRect.toRect() = " << imageRect.toRect();
    dbgRender << " alignedImageRect = " << alignedImageRect << " " << alignedImageRect.bottomRight();
    dbgRender << " scaledSize = " << scaledSize;
    dbgRender << " imageRect.intersected( m_d->image->bounds() ) = " << imageRect.intersected(m_d->image->bounds());
    dbgRender << " m_d->image->bounds() = " << m_d->image->bounds();
    dbgRender << " rcFromAligned = " << rcFromAligned << " " << rcFromAligned.bottomRight();
    dbgRender << " drawRect = " << drawRect;
    dbgRender << " rc = " << rc;
    dbgRender << " dstSize = " << dstSize;
    dbgRender << " rcTopLeftUnscaled = " << rcTopLeftUnscaled;
    dbgRender << "#####################################";
#endif

    // And now for deciding what to do and when -- the complicated bit
    if (scaleX > 1.0 - EPSILON && scaleY > 1.0 - EPSILON) {

        gc.save();
        gc.scale(scaleX, scaleY);
        gc.setCompositionMode(QPainter::CompositionMode_Source);

        if ( scaleX < 2.0 - EPSILON && scaleY < 2.0 - EPSILON ) {

            dbgRender << "smoothBetween100And200Percent" << endl;

            gc.setRenderHint(QPainter::SmoothPixmapTransform, true);

        }

        m_d->projectionCache.drawImage( gc, rcTopLeftUnscaled, alignedImageRect);
        gc.restore();

    } else { // << 200%



        QSize s( ceil( rcFromAligned.size().width() ), ceil( rcFromAligned.size().height() ) );

        if ( s.width() > 0 && s.height() > 0 ) {


            double scaleXbis = rcFromAligned.width() / s.width();
            double scaleYbis = rcFromAligned.height() / s.height();

            QPointF pt( rcFromAligned.topLeft().x() / scaleXbis,
                        rcFromAligned.topLeft().y() / scaleYbis );


            if (m_d->useNearestNeighbour) {

                QImage tmpImage = m_d->image->convertToQImage(alignedImageRect, s, m_d->monitorProfile);
                tmpImage.save( QString( "tmp_%1.png" ).arg( m_d->counter ) );
                m_d->counter++;
                gc.drawImage(pt, tmpImage);

            }
            else {

                // either from the cached image or directly from the
                // KisImage projection
                QImage croppedImage = m_d->projectionCache.image(alignedImageRect);

                gc.save();
                gc.setRenderHint(QPainter::SmoothPixmapTransform, true);
                gc.scale(scaleXbis, scaleYbis);
                // just to make it fit in without seams, it seems.
                gc.drawImage(pt, Blitz::smoothScale(croppedImage, s, Qt::IgnoreAspectRatio));

                gc.restore();
            }
        }
    }
}

QRect KisPrescaledProjection::viewRectFromImagePixels(const QRect & rc)
{
    double xRes = m_d->image->xRes();
    double yRes = m_d->image->yRes();

    QRectF docRect;
    docRect.setCoords((rc.left() - 2) / xRes, (rc.top() - 2) / yRes, (rc.right() + 2) / xRes, (rc.bottom() + 2) / yRes);

    Q_ASSERT(m_d->viewConverter);

    QRect viewRect = m_d->viewConverter->documentToView(docRect).toAlignedRect();
    viewRect = viewRect.translated(-m_d->documentOffset);
    viewRect = viewRect.intersected(QRect(0, 0, m_d->canvasSize.width(), m_d->canvasSize.height()));
    return viewRect;
}

QRect KisPrescaledProjection::imageRectFromViewPortPixels(const QRect & viewportRect)
{
    QRect intersectedRect = viewportRect.intersected(QRect(0, 0, m_d->canvasSize.width(), m_d->canvasSize.height()));
    QRect translatedRect = intersectedRect.translated(-m_d->documentOffset);
    QRectF docRect = m_d->viewConverter->viewToDocument(translatedRect);

    return m_d->image->documentToIntPixel(docRect).intersected(m_d->image->bounds());
}


void KisPrescaledProjection::updateDocumentOrigin(const QPoint& documentOrigin)
{
    m_d->documentOrigin = documentOrigin;
}


#include "kis_prescaled_projection.moc"
