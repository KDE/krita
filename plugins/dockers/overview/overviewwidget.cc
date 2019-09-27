/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */


#include "overviewwidget.h"

#include <QMouseEvent>
#include <QPainter>
#include <QCursor>
#include <QMutex>

#include <KoCanvasController.h>
#include <KoZoomController.h>

#include <kis_canvas2.h>
#include <KisViewManager.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_signal_compressor.h>
#include <kis_config.h>
#include "kis_idle_watcher.h"
#include "krita_utils.h"
#include "kis_painter.h"
#include <KoUpdater.h>
#include "kis_transform_worker.h"
#include "kis_filter_strategy.h"
#include <KoColorSpaceRegistry.h>
#include <QApplication>

const qreal oversample = 2.;
const int thumbnailTileDim = 128;

struct OverviewThumbnailStrokeStrategy::Private {

    class ProcessData : public KisStrokeJobData
    {
    public:
        ProcessData(KisPaintDeviceSP _dev, KisPaintDeviceSP _thumbDev, const QSize& _thumbnailSize, const QRect &_rect)
            : KisStrokeJobData(CONCURRENT),
              dev(_dev), thumbDev(_thumbDev), thumbnailSize(_thumbnailSize), tileRect(_rect)
        {}

        KisPaintDeviceSP dev;
        KisPaintDeviceSP thumbDev;
        QSize thumbnailSize;
        QRect tileRect;
    };
    class FinishProcessing : public KisStrokeJobData
    {
    public:
        FinishProcessing(KisPaintDeviceSP _thumbDev, const QSize& _thumbnailSize)
            : KisStrokeJobData(SEQUENTIAL),
              thumbDev(_thumbDev), thumbnailSize(_thumbnailSize)
        {}
        KisPaintDeviceSP thumbDev;
        QSize thumbnailSize;
    };
};

OverviewWidget::OverviewWidget(QWidget * parent)
    : QWidget(parent)
    , m_canvas(0)
    , m_dragging(false)
    , m_imageIdleWatcher(250)
{
    setMouseTracking(true);
    KisConfig cfg(true);
    slotThemeChanged();
}

OverviewWidget::~OverviewWidget()
{
}

void OverviewWidget::setCanvas(KoCanvasBase * canvas)
{
    if (m_canvas) {
        m_canvas->image()->disconnect(this);
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

    if (m_canvas) {
        m_imageIdleWatcher.setTrackedImage(m_canvas->image());

        connect(&m_imageIdleWatcher, &KisIdleWatcher::startedIdleMode, this, &OverviewWidget::generateThumbnail);

        connect(m_canvas->image(), SIGNAL(sigImageUpdated(QRect)),SLOT(startUpdateCanvasProjection()));
        connect(m_canvas->image(), SIGNAL(sigSizeChanged(QPointF,QPointF)),SLOT(startUpdateCanvasProjection()));

        connect(m_canvas->canvasController()->proxyObject, SIGNAL(canvasOffsetXChanged(int)), this, SLOT(update()), Qt::UniqueConnection);
        connect(m_canvas->viewManager()->mainWindow(), SIGNAL(themeChanged()), this, SLOT(slotThemeChanged()));
        generateThumbnail();
    }
}

QSize OverviewWidget::recalculatePreviewSize()
{
    QSize imageSize(m_canvas->image()->bounds().size());

    const qreal hScale = 1.0 * this->width() / imageSize.width();
    const qreal vScale = 1.0 * this->height() / imageSize.height();

    m_previewScale = qMin(hScale, vScale);

    return imageSize * m_previewScale;
}

QPointF OverviewWidget::previewOrigin()
{
    const QSize previewSize = recalculatePreviewSize();
    return QPointF((width() - previewSize.width()) / 2.0f, (height() - previewSize.height()) / 2.0f);
}

QPolygonF OverviewWidget::previewPolygon()
{
    if (m_canvas) {
        const QRectF &canvasRect = QRectF(m_canvas->canvasWidget()->rect());
        return canvasToPreviewTransform().map(canvasRect);
    }
    return QPolygonF();
}

QTransform OverviewWidget::previewToCanvasTransform()
{
    QTransform previewToImage =
            QTransform::fromTranslate(-this->width() / 2.0, -this->height() / 2.0) *
            QTransform::fromScale(1.0 / m_previewScale, 1.0 / m_previewScale) *
            QTransform::fromTranslate(m_canvas->image()->width() / 2.0, m_canvas->image()->height() / 2.0);

    return previewToImage * m_canvas->coordinatesConverter()->imageToWidgetTransform();
}

QTransform OverviewWidget::canvasToPreviewTransform()
{
    return previewToCanvasTransform().inverted();
}

void OverviewWidget::startUpdateCanvasProjection()
{
    m_imageIdleWatcher.startCountdown();
}

void OverviewWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    m_imageIdleWatcher.startCountdown();
}

void OverviewWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if (m_canvas) {
        if (!m_oldPixmap.isNull()) {
            QSize newSize = recalculatePreviewSize();
            m_pixmap = m_oldPixmap.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        m_imageIdleWatcher.startCountdown();
    }
}

void OverviewWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_canvas) {
        QPointF previewPos = event->pos();

        if (!previewPolygon().containsPoint(previewPos, Qt::WindingFill)) {
            const QRect& canvasRect = m_canvas->canvasWidget()->rect();
            const QPointF newCanvasPos = previewToCanvasTransform().map(previewPos) -
                    QPointF(canvasRect.width() / 2.0f, canvasRect.height() / 2.0f);
            m_canvas->canvasController()->pan(newCanvasPos.toPoint());
        }
        m_lastPos = previewPos;
        m_dragging = true;
    }
    event->accept();
    update();
}

void OverviewWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging) {
        QPointF previewPos = event->pos();
        const QPointF lastCanvasPos = previewToCanvasTransform().map(m_lastPos);
        const QPointF newCanvasPos = previewToCanvasTransform().map(event->pos());

        QPointF diff = newCanvasPos - lastCanvasPos;
        m_canvas->canvasController()->pan(diff.toPoint());
        m_lastPos = previewPos;
    }
    event->accept();
}

void OverviewWidget::mouseReleaseEvent(QMouseEvent* event)
{
    m_dragging = false;
    event->accept();
    update();
}

void OverviewWidget::wheelEvent(QWheelEvent* event)
{
	if (m_canvas) {
		float delta = event->delta();

		if (delta > 0) {
			m_canvas->viewManager()->zoomController()->zoomAction()->zoomIn();
		} else {
			m_canvas->viewManager()->zoomController()->zoomAction()->zoomOut();
		}
	}
}

void OverviewWidget::generateThumbnail()
{
    if (isVisible()) {
        QMutexLocker locker(&mutex);
        if (m_canvas) {
            QSize previewSize = recalculatePreviewSize();
            if(previewSize.isValid()){
                KisImageSP image = m_canvas->image();

                if (!strokeId.isNull()) {
                    image->cancelStroke(strokeId);
                    strokeId.clear();
                }

                OverviewThumbnailStrokeStrategy* stroke = new OverviewThumbnailStrokeStrategy(image);
                connect(stroke, SIGNAL(thumbnailUpdated(QImage)), this, SLOT(updateThumbnail(QImage)));

                strokeId = image->startStroke(stroke);
                KisPaintDeviceSP dev = image->projection();
                KisPaintDeviceSP thumbDev = new KisPaintDevice(dev->colorSpace());

                //creating a special stroke that computes thumbnail image in small chunks that can be quickly interrupted
                //if user starts painting
                QList<KisStrokeJobData*> jobs = OverviewThumbnailStrokeStrategy::createJobsData(dev, image->bounds(), thumbDev, previewSize);

                Q_FOREACH (KisStrokeJobData *jd, jobs) {
                    image->addJob(strokeId, jd);
                }
                image->endStroke(strokeId);
            }
        }
    }
}

void OverviewWidget::updateThumbnail(QImage pixmap)
{
    m_pixmap = QPixmap::fromImage(pixmap);
    m_oldPixmap = m_pixmap.copy();
    update();
}

void OverviewWidget::slotThemeChanged()
{
    m_outlineColor = qApp->palette().color(QPalette::Highlight);
}


void OverviewWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    if (m_canvas) {
        QPainter p(this);

        const QSize previewSize = recalculatePreviewSize();
        const QRectF previewRect = QRectF(previewOrigin(), previewSize);
        p.drawPixmap(previewRect.toRect(), m_pixmap);

        QRect r = rect();
        QPolygonF outline;
        outline << r.topLeft() << r.topRight() << r.bottomRight() << r.bottomLeft();

        QPen pen;
        pen.setColor(m_outlineColor);
        pen.setStyle(Qt::DashLine);

        p.setPen(pen);
        p.drawPolygon(outline.intersected(previewPolygon()));

        pen.setStyle(Qt::SolidLine);
        p.setPen(pen);
        p.drawPolygon(previewPolygon());
    }
}

OverviewThumbnailStrokeStrategy::OverviewThumbnailStrokeStrategy(KisImageWSP image)
    : KisSimpleStrokeStrategy("OverviewThumbnail"), m_image(image)
{
    enableJob(KisSimpleStrokeStrategy::JOB_INIT, true, KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);
    //enableJob(KisSimpleStrokeStrategy::JOB_FINISH);
    enableJob(KisSimpleStrokeStrategy::JOB_CANCEL, true, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);

    setRequestsOtherStrokesToEnd(false);
    setClearsRedoOnStart(false);
    setCanForgetAboutMe(true);
}

QList<KisStrokeJobData *> OverviewThumbnailStrokeStrategy::createJobsData(KisPaintDeviceSP dev, const QRect& imageRect, KisPaintDeviceSP thumbDev, const QSize& thumbnailSize)
{
    QSize thumbnailOversampledSize = oversample * thumbnailSize;

    if ((thumbnailOversampledSize.width() > imageRect.width()) || (thumbnailOversampledSize.height() > imageRect.height())) {
        thumbnailOversampledSize.scale(imageRect.size(), Qt::KeepAspectRatio);
    }

    QVector<QRect> tileRects = KritaUtils::splitRectIntoPatches(QRect(QPoint(0, 0), thumbnailOversampledSize), QSize(thumbnailTileDim, thumbnailTileDim));
    QList<KisStrokeJobData*> jobsData;

    Q_FOREACH (const QRect &tileRectangle, tileRects) {
        jobsData << new OverviewThumbnailStrokeStrategy::Private::ProcessData(dev, thumbDev, thumbnailOversampledSize, tileRectangle);
    }
    jobsData << new OverviewThumbnailStrokeStrategy::Private::FinishProcessing(thumbDev, thumbnailSize);

    return jobsData;
}

OverviewThumbnailStrokeStrategy::~OverviewThumbnailStrokeStrategy()
{
}


void OverviewThumbnailStrokeStrategy::initStrokeCallback()
{
}

void OverviewThumbnailStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    Private::ProcessData *d_pd = dynamic_cast<Private::ProcessData*>(data);
    if (d_pd) {
        //we aren't going to use oversample capability of createThumbnailDevice because it recomputes exact bounds for each small patch, which is
        //slow. We'll handle scaling separately.
        KisPaintDeviceSP thumbnailTile = d_pd->dev->createThumbnailDeviceOversampled(d_pd->thumbnailSize.width(), d_pd->thumbnailSize.height(), 1, m_image->bounds(), d_pd->tileRect);
        {
            QMutexLocker locker(&m_thumbnailMergeMutex);
            KisPainter gc(d_pd->thumbDev);
            gc.bitBlt(QPoint(d_pd->tileRect.x(), d_pd->tileRect.y()), thumbnailTile, d_pd->tileRect);
        }
        return;
    }


    Private::FinishProcessing *d_fp = dynamic_cast<Private::FinishProcessing*>(data);
    if (d_fp) {
        QImage overviewImage;

        KoDummyUpdater updater;
        KisTransformWorker worker(d_fp->thumbDev, 1 / oversample, 1 / oversample, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0, 0.0,
                                  &updater, KisFilterStrategyRegistry::instance()->value("Bilinear"));
        worker.run();

        overviewImage = d_fp->thumbDev->convertToQImage(KoColorSpaceRegistry::instance()->rgb8()->profile(),
                                                        QRect(QPoint(0,0), d_fp->thumbnailSize));
        emit thumbnailUpdated(overviewImage);
        return;
    }
}

void OverviewThumbnailStrokeStrategy::finishStrokeCallback()
{
}

void OverviewThumbnailStrokeStrategy::cancelStrokeCallback()
{
}
