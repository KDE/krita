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
#include "kis_image_config.h"
#include "kis_config_notifier.h"
#include "kis_image.h"
#include "krita_utils.h"

#include "kis_coordinates_converter.h"
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
            : viewportSize(0, 0)
            , monitorProfile(0)
            , projectionBackend(0) {
    }

    QImage prescaledQImage;

    QSize updatePatchSize;
    QSize canvasSize;
    QSize viewportSize;
    KisImageWSP image;
    KisCoordinatesConverter *coordinatesConverter;
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

void KisPrescaledProjection::setCoordinatesConverter(KisCoordinatesConverter *coordinatesConverter)
{
    m_d->coordinatesConverter = coordinatesConverter;
}

void KisPrescaledProjection::initBackend()
{
    delete m_d->projectionBackend;

    // we disable building the pyramid with setting its height to 1
    // XXX: setting it higher than 1 is broken because it's not updated until you show/hide the layer
    m_d->projectionBackend = new KisImagePyramid(1);
    m_d->projectionBackend->setImage(m_d->image);
}

void KisPrescaledProjection::updateSettings()
{
    KisConfig cfg;

    if (m_d->projectionBackend == 0) {
        initBackend();
    }

    setMonitorProfile(KoColorSpaceRegistry::instance()->profileByName(cfg.monitorProfile()));

    KisImageConfig imageConfig;
    m_d->updatePatchSize.setWidth(imageConfig.updatePatchWidth());
    m_d->updatePatchSize.setHeight(imageConfig.updatePatchHeight());
}

void KisPrescaledProjection::viewportMoved(const QPointF &offset)
{
    // FIXME: \|/
    if (m_d->prescaledQImage.isNull()) return;
    if (offset.isNull()) return;

    QPoint alignedOffset = offset.toPoint();

    if(offset != alignedOffset) {
        /**
         * We can't optimize anything when offset is float :(
         * Just prescale entire image.
         */
        dbgRender << "prescaling the entire image because the offset is float";
        preScale();
        return;
    }

    QImage newImage = QImage(m_d->viewportSize, QImage::Format_ARGB32);
    newImage.fill(0);

    /**
     * TODO: viewport rects should be cropped by the borders of
     * the image, because it may be requested to read/write
     * outside QImage and copyQImage will not chatch it
     */
    QRect newViewportRect = QRect(QPoint(0,0), m_d->viewportSize);
    QRect oldViewportRect = newViewportRect.translated(alignedOffset);

    QRegion updateRegion = newViewportRect;
    QRect savedArea = newViewportRect & oldViewportRect;

    if(!savedArea.isEmpty()) {
        copyQImage(alignedOffset.x(), alignedOffset.y(), &newImage, m_d->prescaledQImage);
        updateRegion -= savedArea;
    }

    QPainter gc(&newImage);
    QVector<QRect> rects = updateRegion.rects();

    foreach(QRect rect, rects) {

        QRect imageRect =
            m_d->coordinatesConverter->viewportToImage(rect).toAlignedRect();
        QVector<QRect> patches =
            KritaUtils::splitRectIntoPatches(imageRect, m_d->updatePatchSize);

        foreach(const QRect& rc, patches) {
            QRect viewportPatch =
                m_d->coordinatesConverter->imageToViewport(rc).toAlignedRect();

            KisPPUpdateInfoSP info = getUpdateInformation(viewportPatch, QRect());
            drawUsingBackend(gc, info);
        }
    }

    m_d->prescaledQImage = newImage;
}

void KisPrescaledProjection::setImageSize(qint32 w, qint32 h)
{
    m_d->projectionBackend->setImageSize(w, h);

    QRect viewportRect = m_d->coordinatesConverter->imageToViewport(QRect(0, 0, w, h)).toAlignedRect();
    viewportRect = viewportRect.intersected(QRect(QPoint(0, 0), m_d->viewportSize));

    if (!viewportRect.isEmpty()) {
        preScale(viewportRect);
    }
}

KisUpdateInfoSP KisPrescaledProjection::updateCache(const QRect &dirtyImageRect)
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


    QRect rawViewRect =
        m_d->coordinatesConverter->imageToViewport(dirtyImageRect).toAlignedRect();

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
    QRect viewportRect(QPoint(0, 0), m_d->viewportSize);
    QRect imageRect =
        m_d->coordinatesConverter->viewportToImage(viewportRect).toAlignedRect();

    QVector<QRect> patches =
        KritaUtils::splitRectIntoPatches(imageRect, m_d->updatePatchSize);

    foreach(const QRect& rc, patches) {
        QRect viewportPatch =
            m_d->coordinatesConverter->imageToViewport(rc).toAlignedRect();
        preScale(viewportPatch);
    }
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


void KisPrescaledProjection::updateViewportSize()
{
    QRectF imageRect = m_d->coordinatesConverter->imageRectInWidgetPixels();
    QSizeF minimalSize(qMin(imageRect.width(), (qreal)m_d->canvasSize.width()),
                       qMin(imageRect.height(), (qreal)m_d->canvasSize.height()));
    QRectF minimalRect(QPointF(0,0), minimalSize);

    m_d->viewportSize = m_d->coordinatesConverter->widgetToViewport(minimalRect).toAlignedRect().size();

    if (m_d->prescaledQImage.isNull() ||
        m_d->prescaledQImage.size() != m_d->viewportSize) {

        m_d->prescaledQImage = QImage(m_d->viewportSize, QImage::Format_ARGB32);
    }
}

void KisPrescaledProjection::notifyZoomChanged()
{
    updateViewportSize();
    preScale();
}

void KisPrescaledProjection::notifyCanvasSizeChanged(const QSize &widgetSize)
{
    // FIXME: Let someone else figure out the optimization where we copy the
    // still visible part of the image after moving the offset and then
    // only draw the newly visible parts

    m_d->canvasSize = widgetSize;
    updateViewportSize();
    preScale();
}


KisPPUpdateInfoSP KisPrescaledProjection::getUpdateInformation(const QRect &viewportRect,
                                             const QRect &dirtyImageRect)
{
    KisPPUpdateInfoSP info = new KisPPUpdateInfo();

    m_d->coordinatesConverter->imageScale(&info->scaleX, &info->scaleY);

    // save it for future
    info->dirtyImageRect = dirtyImageRect;

    // first, crop the part of the view rect that is outside of the canvas
    QRect croppedViewRect = viewportRect.intersected(QRect(QPoint(0, 0), m_d->viewportSize));

    // second, align this rect to the KisImage's pixels and pixels
    // of projection backend
    info->imageRect = m_d->coordinatesConverter->viewportToImage(QRectF(croppedViewRect)).toAlignedRect();
    info->imageRect = info->imageRect & m_d->image->bounds();

    m_d->projectionBackend->alignSourceRect(info->imageRect, info->scaleX);

    // finally, compute the dirty rect of the canvas
    info->viewportRect = m_d->coordinatesConverter->imageToViewport(info->imageRect);

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
    drawUsingBackend(gc, info);
}

void KisPrescaledProjection::drawUsingBackend(QPainter &gc, KisPPUpdateInfoSP info)
{
    if (info->imageRect.isEmpty()) return;

    if (info->transfer == KisPPUpdateInfo::DIRECT) {
        m_d->projectionBackend->drawFromOriginalImage(gc, info);
    } else /* if info->transfer == KisPPUpdateInformation::PATCH */ {
        KisImagePatch patch = m_d->projectionBackend->getNearestPatch(info);
        // prescale the patch because otherwise we'd scale using QPainter, which gives
        // a crap result compared to QImage's smoothscale
        patch.preScale(info->viewportRect);
        patch.drawMe(gc, info->viewportRect, info->renderHints);
    }
}

#include "kis_prescaled_projection.moc"
