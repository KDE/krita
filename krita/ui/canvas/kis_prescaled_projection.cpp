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
#include "kis_prescaled_projection.h"

#include <math.h>

#include <QImage>
#include <QColor>
#include <QRect>
#include <QPoint>
#include <QSize>
#include <QPainter>

#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KoViewConverter.h>

#include "kis_config.h"
#include "kis_config_notifier.h"
#include "kis_image.h"

#include "kis_projection_backend.h"
#include "kis_image_pyramid.h"


#define EPSILON 1e-10

#define ceiledSize(sz) QSize(ceil((sz).width()), ceil((sz).height()))
#define SCALE_LESS_THAN(scX, scY, value) \
    (scX < (value) - EPSILON && scY < (value) - EPSILON)
#define SCALE_MORE_OR_EQUAL_TO(scX, scY, value) \
    (scX > (value) - EPSILON && scY > (value) - EPSILON)
#define BORDER_SIZE(scale) (ceil(scale * 2))

inline void copyQImageBuffer(uchar* dst, const uchar* src , qint32 deltaX, qint32 width)
{
    if (deltaX >= 0) {
        memcpy(dst + 4 * deltaX, src, 4 *(width - deltaX) * sizeof(uchar));
    } else {
        memcpy(dst, src - 4 * deltaX, 4 *(width + deltaX) * sizeof(uchar));
    }
}

void copyQImage(qint32 deltaX, qint32 deltaY, QImage* dstImage, const QImage& srcImage)
{
    qint32 height = dstImage->height();
    qint32 width = dstImage->width();
    Q_ASSERT(dstImage->width() == srcImage.width() && dstImage->height() == srcImage.height());
    if (deltaY >= 0) {
        for (int y = 0; y < height - deltaY; y ++) {
            const uchar* src = srcImage.scanLine(y);
            uchar* dst = dstImage->scanLine(y + deltaY);
            copyQImageBuffer(dst, src, deltaX, width);
        }
    } else {
        for (int y = 0; y < height + deltaY; y ++) {
            const uchar* src = srcImage.scanLine(y - deltaY);
            uchar* dst = dstImage->scanLine(y);
            copyQImageBuffer(dst, src, deltaX, width);
        }
    }
}

struct KisPrescaledProjection::Private {
    Private()
            : useNearestNeighbour(false)
            , cacheKisImageAsQImage(true)
            , documentOffset(0, 0)
            , documentOrigin(0, 0)
            , canvasSize(0, 0)
            , imageSize(0, 0)
            , viewConverter(0)
            , monitorProfile(0)
            , projectionBackend(0) {
    }

    /**
     * TODO: useNearestNeighbour is not used at the moment
     * thought this option worth implementing (ok, yeah.. reimplementing)
     */
    bool useNearestNeighbour;
    bool cacheKisImageAsQImage;

    QImage prescaledQImage;

    QPoint documentOffset; // in view pixels
    QPoint documentOrigin; // in view pixels too

    QSize canvasSize; // in view pixels
    QSize imageSize; // in kisimage pixels
    KisImageWSP image;
    KoViewConverter * viewConverter;
    const KoColorProfile* monitorProfile;
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

QImage KisPrescaledProjection::prescaledQImage() const
{
    return m_d->prescaledQImage;
}

void KisPrescaledProjection::setViewConverter(KoViewConverter * viewConverter)
{
    m_d->viewConverter = viewConverter;
}

void KisPrescaledProjection::initBackend(bool cacheKisImageAsQImage)
{
    Q_UNUSED(cacheKisImageAsQImage);
    if (m_d->projectionBackend) {
        delete m_d->projectionBackend;
    }

    // we disable building the pyramid with setting its height to 1
    m_d->projectionBackend = new KisImagePyramid(1);
    m_d->projectionBackend->setImage(m_d->image);
}

void KisPrescaledProjection::updateSettings()
{
    KisConfig cfg;

    if (m_d->useNearestNeighbour != cfg.useNearestNeighbour() ||
        m_d->cacheKisImageAsQImage != cfg.cacheKisImageAsQImage() ||
        m_d->projectionBackend == 0 ) {

        m_d->useNearestNeighbour = cfg.useNearestNeighbour();
        m_d->cacheKisImageAsQImage = cfg.cacheKisImageAsQImage();

        initBackend(m_d->cacheKisImageAsQImage);
    }

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

    QImage image = QImage(width, height, QImage::Format_ARGB32);
    QPainter gc(&image);

    if (oldCanvasXOffset != m_d->documentOffset.x() || oldCanvasYOffset != m_d->documentOffset.y()) {

        qint32 deltaX =  oldCanvasXOffset - m_d->documentOffset.x();
        qint32 deltaY = oldCanvasYOffset - m_d->documentOffset.y();

        dbgRender << "deltaX: " << deltaX << ", deltaY: " << deltaY;
        if (qAbs(deltaX) < width && qAbs(deltaY) < height) {
            dbgRender << "Copy old data";
            copyQImage(deltaX, deltaY, &image, m_d->prescaledQImage);

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
                /**
                 * FIXME: It can happen that we will need access to KisImage here
                 */
                KisPPUpdateInfoSP info = getUpdateInformation(r, QRect());
                drawUsingBackend(gc, info);
            }
        }
        m_d->prescaledQImage = image;
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

    QRect vRect = viewRectFromImagePixels(QRect(0, 0, w, h)).toAlignedRect();
    vRect = vRect.intersected(QRect(QPoint(0, 0), m_d->canvasSize));
    if (!vRect.isEmpty()) {
        preScale(vRect);
    }
}

KisUpdateInfoSP
KisPrescaledProjection::updateCache(const QRect &dirtyImageRect)
{
    if (!m_d->image) {
        dbgRender << "Calling updateCache without an image: " << kBacktrace() << endl;
        // return invalid info
        return new KisPPUpdateInfo();
    }

    /**
     * We needn't this stuff ouside KisImage's area. Lets user
     * paint there, anyway we won't show him anything =)
     */
    if (!dirtyImageRect.intersects(m_d->image->bounds()))
        return new KisPPUpdateInfo();

    QRect rawViewRect = viewRectFromImagePixels(dirtyImageRect).toAlignedRect();
    KisPPUpdateInfoSP info = getUpdateInformation(rawViewRect, dirtyImageRect);

    m_d->projectionBackend->updateCache(info);

    return info;
}

void KisPrescaledProjection::recalculateCache(KisUpdateInfoSP info)
{
    KisPPUpdateInfoSP ppInfo = dynamic_cast<KisPPUpdateInfo*>(info.data());
    if(!ppInfo) return;

    m_d->projectionBackend->recalculateCache(ppInfo);

    if(!info->dirtyViewportRect().isEmpty())
        updateScaledImage(ppInfo);
}

void KisPrescaledProjection::preScale()
{
    Q_ASSERT(m_d->canvasSize.isValid());
    preScale(QRect(QPoint(0, 0), m_d->canvasSize));
}

QRect KisPrescaledProjection::preScale(const QRect & rc)
{
    if (!rc.isEmpty() && m_d->image) {
        /**
         * FIXME: It can happen that we will need access to KisImage here
         */
        KisPPUpdateInfoSP info = getUpdateInformation(rc, QRect());

        QPainter gc(&m_d->prescaledQImage);
        gc.setCompositionMode(QPainter::CompositionMode_Source);
        gc.fillRect(rc, QColor(0, 0, 0, 0));
        drawUsingBackend(gc, info);
        //FIXME: leave one of those rects, probably, first.
        return rc | info->viewportRect.toAlignedRect();
    }
    return QRect();
}

void KisPrescaledProjection::setMonitorProfile(const KoColorProfile * profile)
{
    m_d->monitorProfile = profile;
    m_d->projectionBackend->setMonitorProfile(profile);
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

    QImage image = QImage(newSize, QImage::Format_ARGB32);
//     m_d->prescaledQImage.fill( QColor( 255, 0, 0, 128 ).rgba() );

// FIXME: Let someone else figure out the optimization where we copy the
// still visible part of the image after moving the offset and then
// only draw the newly visible parts

    m_d->prescaledQImage = image;
    m_d->canvasSize = newSize;
    preScale();
}

KisPPUpdateInfoSP
KisPrescaledProjection::getUpdateInformation(const QRect &viewportRect,
                                             const QRect &dirtyImageRect)
{
    Q_ASSERT(m_d->viewConverter);

    KisPPUpdateInfoSP info = new KisPPUpdateInfo();

    // get the x and y zoom level of the canvas
    qreal zoomX, zoomY;
    m_d->viewConverter->zoom(&zoomX, &zoomY);

    // Get the KisImage resolution
    qreal resX = m_d->image->xRes();
    qreal resY = m_d->image->yRes();

    // Compute the scale factors
    info->scaleX = zoomX / resX;
    info->scaleY = zoomY / resY;

    // save it for future
    info->dirtyImageRect = dirtyImageRect;

    // first, crop the part of the view rect that is outside of the canvas
    QRect croppedViewRect = viewportRect.intersected(QRect(QPoint(0, 0), m_d->canvasSize));

    // second, align this rect to the KisImage's pixels and pixels
    // of projection backend
    info->imageRect = imageRectFromViewPortPixels(QRectF(croppedViewRect));
    m_d->projectionBackend->alignSourceRect(info->imageRect, info->scaleX);

    // finally, compute the dirty rect of the canvas
    info->viewportRect = viewRectFromImagePixels(info->imageRect);

    info->borderWidth = 0;
    if (SCALE_MORE_OR_EQUAL_TO(info->scaleX, info->scaleY, 1.0)) {
        if (SCALE_LESS_THAN(info->scaleX, info->scaleY, 2.0)) {
            dbgRender << "smoothBetween100And200Percent" << endl;
            info->renderHints = QPainter::SmoothPixmapTransform;
            info->borderWidth = BORDER_SIZE(qMax(info->scaleX, info->scaleY));
        }
        info->transfer = KisPPUpdateInfo::DIRECT;
    } else { // <100%
        info->renderHints = QPainter::SmoothPixmapTransform;
        info->borderWidth = BORDER_SIZE(qMax(info->scaleX, info->scaleY));
        info->transfer = KisPPUpdateInfo::PATCH;
    }

    dbgRender << "#####################################";
    dbgRender << ppVar(resX) << ppVar(resY);
    dbgRender << ppVar(zoomX) << ppVar(zoomY);
    dbgRender << ppVar(info->scaleX) << ppVar(info->scaleY);
    dbgRender << ppVar(info->borderWidth) << ppVar(info->renderHints);
    dbgRender << ppVar(info->transfer);
    dbgRender << ppVar(dirtyImageRect);
    dbgRender << "Not aligned rect of the canvas (raw):\t" << croppedViewRect;
    dbgRender << "Update rect in KisImage's pixels:\t" << info->imageRect;
    dbgRender << "Update rect in canvas' pixels:\t" << info->viewportRect;
    dbgRender << "#####################################";

    return info;
}

void KisPrescaledProjection::updateScaledImage(KisPPUpdateInfoSP info)
{
    QPainter gc(&m_d->prescaledQImage);
    gc.setCompositionMode(QPainter::CompositionMode_Source);
//    gc.fillRect(viewRectFromImagePixels(info->dirtyImageRect).toAlignedRect(), QColor(255, 0, 0, 255));
    drawUsingBackend(gc, info);
}

void KisPrescaledProjection::drawUsingBackend(QPainter &gc, KisPPUpdateInfoSP info)
{
    if (info->imageRect.isEmpty()) return;

    if (info->transfer == KisPPUpdateInfo::DIRECT) {
        m_d->projectionBackend->drawFromOriginalImage(gc, info);
    } else /* if info->transfer == KisPPUpdateInformation::PATCH */ {
        KisImagePatch patch = m_d->projectionBackend->getNearestPatch(info);

        //patch.prescaleWithBlitz(viewportRect);
        patch.drawMe(gc, info->viewportRect, info->renderHints);
    }
}

QRectF KisPrescaledProjection::viewRectFromImagePixels(const QRect& rc)
{
    Q_ASSERT(m_d->viewConverter);

    qreal xRes = m_d->image->xRes();
    qreal yRes = m_d->image->yRes();

    QRectF imageRect(rc);

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

    return m_d->image->documentToPixel(docRect).toAlignedRect().intersected(m_d->image->bounds());
}


void KisPrescaledProjection::updateDocumentOrigin(const QPoint& documentOrigin)
{
    m_d->documentOrigin = documentOrigin;
}

#include "kis_prescaled_projection.moc"
