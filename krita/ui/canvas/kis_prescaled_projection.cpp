/*
 *  Copyright (c) 2007, Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008, Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2009, Dmitry Kazakov <dimula73@gmail.com>
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

#include <QImage>
#include <QPixmap>
#include <QColor>
#include <QRect>
#include <QPoint>
#include <QSize>
#include <QPainter>
#include <QTimer>

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
#include "kis_prescaled_projection.h"

#include "kis_projection_cache.h"
#include "kis_image_pyramid.h"


#define EPSILON 1e-10

inline void copyQImageBuffer(uchar* dst, const uchar* src , qint32 deltaX, qint32 width)
{
    if (deltaX >= 0) {
        memcpy(dst + 4 * deltaX, src, 4 *(width - deltaX) * sizeof(uchar));
    } else {
        memcpy(dst, src - 4 * deltaX, 4 *(width + deltaX) * sizeof(uchar));
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
            : useNearestNeighbour(false)
            , usePixmapNotQImage(false)
            , drawCheckers(false)
            , scrollCheckers(false)
            , drawMaskVisualisationOnUnscaledCanvasCache(false)
            , showMask(true)
            , cacheKisImageAsQImage(true)
            , checkSize(32)
            , documentOffset(0, 0)
            , documentOrigin(0, 0)
            , canvasSize(0, 0)
            , imageSize(0, 0)
            , viewConverter(0)
            , monitorProfile(0)
            , counter(0)
            , projectionBackend(0) {
    }

    bool useNearestNeighbour;
    bool usePixmapNotQImage;
    bool useMipmapping;
    bool drawCheckers;
    bool scrollCheckers;
    bool drawMaskVisualisationOnUnscaledCanvasCache;
    bool showMask;
    bool cacheKisImageAsQImage;

    QColor checkersColor;
    qint32 checkSize;
    QImage prescaledQImage;
    QPixmap prescaledPixmap;


    QPoint documentOffset; // in view pixels
    QPoint documentOrigin; // in view pixels too

    QSize canvasSize; // in view pixels
    QSize imageSize; // in kisimage pixels
    KisImageWSP image;
    KoViewConverter * viewConverter;
    KisNodeSP currentNode;
    QRegion rectsToSmooth;
    const KoColorProfile* monitorProfile;
    int counter;
    KisProjectionBackend* projectionBackend;
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
    if (m_d->projectionBackend)
        delete m_d->projectionBackend;

    delete m_d;
}


void KisPrescaledProjection::setImage(KisImageWSP image)
{
    Q_ASSERT(image);
    m_d->image = image;
    m_d->projectionBackend->setImage(image);

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

void KisPrescaledProjection::initBackend(bool useMipmapping, bool cacheKisImageAsQImage)
{
    if (m_d->projectionBackend) {
        delete m_d->projectionBackend;
    }

    if (useMipmapping) {
        m_d->projectionBackend = new KisImagePyramid(4);
    }
    else {
        KisProjectionCache* cache = new KisProjectionCache();
        cache->setCacheKisImageAsQImage(cacheKisImageAsQImage);
        m_d->projectionBackend = cache;
    }
    m_d->projectionBackend->setImage(m_d->image);
}

void KisPrescaledProjection::updateSettings()
{
    KisConfig cfg;


    if (m_d->useNearestNeighbour != cfg.useNearestNeighbour() ||
        m_d->useMipmapping != cfg.useMipmapping() ||
        m_d->cacheKisImageAsQImage != cfg.cacheKisImageAsQImage()) {

        m_d->useNearestNeighbour = cfg.useNearestNeighbour();
        m_d->useMipmapping = cfg.useMipmapping();
        m_d->cacheKisImageAsQImage = cfg.cacheKisImageAsQImage();

        initBackend(m_d->useMipmapping, m_d->cacheKisImageAsQImage);
    }

    m_d->usePixmapNotQImage = cfg.noXRender();

    // If any of the above are true, we don't use our own smooth scaling
    m_d->scrollCheckers = cfg.scrollCheckers();
    m_d->checkSize = cfg.checkSize();
    m_d->checkersColor = cfg.checkersColor();

    m_d->drawMaskVisualisationOnUnscaledCanvasCache = cfg.drawMaskVisualisationOnUnscaledCanvasCache();

    setMonitorProfile(KoColorSpaceRegistry::instance()->profileByName(cfg.monitorProfile()));
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

    m_d->projectionBackend->setImageSize(w, h);

    m_d->imageSize = QSize(w, h);

    QRect vRect = toAlignedRectWorkaround(viewRectFromImagePixels(QRect(0, 0, w, h)));
    vRect = vRect.intersected(QRect(QPoint(0, 0), m_d->canvasSize));
    if (!vRect.isEmpty()) {
        preScale(vRect);
    }
}

QRect KisPrescaledProjection::updateCanvasProjection(const QRect & rc)
{
    if (!m_d->image) {
        dbgRender << "Calling updateCanvasProjection without an image: " << kBacktrace() << endl;
        return QRect();
    }

    /**
     * FIXME: leave only one of these intersections: canvas or image
     */

    /**
     * We needn't this stuff ouside KisImage's area. Lets user
     * paint there, anyway we won't show him anything =)
     */
    if (!rc.intersects(m_d->image->bounds()))
        return QRect();

    m_d->projectionBackend->setDirty(rc);

    QRect viewRect = toAlignedRectWorkaround(viewRectFromImagePixels(rc));
    viewRect = viewRect.intersected(QRect(QPoint(0, 0), m_d->canvasSize));
    QRect newViewRect;

    if (!viewRect.isEmpty()) {
        newViewRect = preScale(viewRect);

        if (newViewRect != viewRect) {
            dbgRender << "canvas size:" << m_d->canvasSize;
            dbgRender << "viewRect before alignment:" << viewRect;
            dbgRender << "viewRect after alignment: " << newViewRect;
        }
    }

    return newViewRect;
}


void KisPrescaledProjection::setMonitorProfile(const KoColorProfile * profile)
{
    m_d->monitorProfile = profile;
    m_d->projectionBackend->setMonitorProfile(profile);
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

QRect KisPrescaledProjection::preScale(const QRect & rc)
{
    QRect retval;
    if (!rc.isEmpty()) {
        QPainter gc(&m_d->prescaledQImage);
        gc.setCompositionMode(QPainter::CompositionMode_Source);
        gc.fillRect(rc, QColor(0, 0, 0, 0));
        retval = drawScaledImage(rc, gc);
    }
    return retval;
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

QRect KisPrescaledProjection::drawScaledImage(const QRect & rc,  QPainter & gc)
{
    if (!m_d->image)
        return QRect();

    Q_ASSERT(m_d->viewConverter);

    // get the x and y zoom level of the canvas
    qreal zoomX, zoomY;
    m_d->viewConverter->zoom(&zoomX, &zoomY);

    // Get the KisImage resolution
    qreal resX = m_d->image->xRes();
    qreal resY = m_d->image->yRes();

    // Compute the scale factors
    qreal scaleX = zoomX / resX;
    qreal scaleY = zoomY / resY;

    QRect updateRectInImagePixels = imageRectFromViewPortPixels(toFloatRectWorkaround(rc));

    m_d->projectionBackend->alignSourceRect(updateRectInImagePixels, scaleX);

    QRectF updateRect = viewRectFromImagePixels(updateRectInImagePixels);


    drawUsingBackend(m_d->projectionBackend, gc, scaleX, scaleY,
                     updateRectInImagePixels, updateRect);

    dbgRender << "#####################################";
    dbgRender << ppVar(resX) << ppVar(resY);
    dbgRender << ppVar(zoomX) << ppVar(zoomY);
    dbgRender << ppVar(scaleX) << ppVar(scaleY);
    dbgRender << ppVar(rc);
    dbgRender << "Update rect in KisImage's pixels:\t" << updateRectInImagePixels;
    dbgRender << "Update rect in canvas' pixels:\t" << updateRect;
    dbgRender << "#####################################";

    return toAlignedRectWorkaround(updateRect);
}


#define ceiledSize(sz) QSize(ceil((sz).width()), ceil((sz).height()))
#define SCALE_LESS_THAN(scX, scY, value) \
    (scX < (value) - EPSILON && scY < (value) - EPSILON)
#define SCALE_MORE_OR_EQUAL_TO(scX, scY, value) \
    (scX > (value) - EPSILON && scY > (value) - EPSILON)
#define BORDER_SIZE(scale) (ceil(scale * 2))

void KisPrescaledProjection::drawUsingBackend(KisProjectionBackend *backend,
        QPainter &gc,
        qreal &scaleX, qreal &scaleY,
        const QRect   &imageRect,
        const QRectF  &viewportRect)
{
    if (imageRect.isEmpty()) return;

    if (SCALE_MORE_OR_EQUAL_TO(scaleX, scaleY, 1.0)) {
        QPainter::RenderHints renderHints;
        qint32 borderWidth = 0;

        if (SCALE_LESS_THAN(scaleX, scaleY, 2.0)) {
            dbgRender << "smoothBetween100And200Percent" << endl;
            renderHints = QPainter::SmoothPixmapTransform;
            borderWidth = BORDER_SIZE(qMax(scaleX, scaleY));
        }
        backend->drawFromOriginalImage(gc, imageRect, viewportRect,
                                       borderWidth,
                                       renderHints);
    } else { // <100%
        KisImagePatch patch =
            backend->getNearestPatch(scaleX, scaleY,
                                     imageRect,
                                     BORDER_SIZE(qMax(scaleX, scaleY)));

        //patch.prescaleWithBlitz(viewportRect);
        patch.drawMe(gc, viewportRect, QPainter::SmoothPixmapTransform);
    }
}

QRectF KisPrescaledProjection::viewRectFromImagePixels(const QRect& rc)
{
    Q_ASSERT(m_d->viewConverter);

    qreal xRes = m_d->image->xRes();
    qreal yRes = m_d->image->yRes();

    QRectF imageRect = toFloatRectWorkaround(rc);

    QRectF docRect;
    docRect.setCoords(imageRect.left() / xRes, imageRect.top() / yRes,
                      imageRect.right() / xRes, imageRect.bottom() / yRes);

    QRectF viewRect = m_d->viewConverter->documentToView(docRect);
    viewRect = viewRect.translated(-m_d->documentOffset);

    QRectF viewRectInter = viewRect.intersected(QRectF(0, 0, m_d->canvasSize.width(), m_d->canvasSize.height()));

    if (viewRect != viewRectInter && !viewRectInter.isEmpty()) {
        dbgRender << "*** Bad alignment of viewport rect ***";
        dbgRender << "* Calculated: " << viewRect;
        dbgRender << "* Must be:    " << viewRectInter;
        dbgRender << "*** ****************************** ***";
    }

    return viewRect;
}

QRect KisPrescaledProjection::imageRectFromViewPortPixels(const QRectF& viewportRect)
{
    QRectF intersectedRect = viewportRect;//.intersected(QRectF(0, 0, m_d->canvasSize.width(), m_d->canvasSize.height()));
    QRectF translatedRect = intersectedRect.translated(m_d->documentOffset);
    QRectF docRect = m_d->viewConverter->viewToDocument(translatedRect);

    return toAlignedRectWorkaround(m_d->image->documentToPixel(docRect)).intersected(m_d->image->bounds());
}


void KisPrescaledProjection::updateDocumentOrigin(const QPoint& documentOrigin)
{
    m_d->documentOrigin = documentOrigin;
}


QRectF toFloatRectWorkaround(const QRect& rc)
{
    QRectF temp;

    /**
     * We can't use setCoords here due to "history reasons",
     * mentioned in Qt documentation.
     * More than that we make rounding process more stable
     * with cropping the rect by EPSILON
     */
    temp.setRect(rc.left() + EPSILON, rc.top() + EPSILON,
                 rc.width() - 2 * EPSILON, rc.height() - 2 * EPSILON);

    return temp;
}

QRect toAlignedRectWorkaround(const QRectF& rc)
{
    qreal x1, y1, x2, y2;
    rc.getCoords(&x1, &y1, &x2, &y2);

    x1 = floor(x1);
    y1 = floor(y1);
    x2 = ceil(x2);
    y2 = ceil(y2);

    QRect ret;

    /**
     * We can't use setCoords here due to "history reasons",
     * mentioned in Qt documentation
     */
    ret.setRect(x1, y1, x2 - x1, y2 - y1);

    return ret;
}



#include "kis_prescaled_projection.moc"
