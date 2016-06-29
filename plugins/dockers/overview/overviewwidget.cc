/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2014 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2.1 of the License.
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

struct OverviewThumbnailStrokeStrategy::Private
{
    KisImageWSP image;
    //QHash<KisPaintDeviceSP, KisPaintDevice::LodDataStruct*> dataObjects;

    ~Private() {
        //qDeleteAll(dataObjects);
        //dataObjects.clear();
    }

    class InitData : public KisStrokeJobData {
    public:
        InitData(KisPaintDeviceSP _device)
            : KisStrokeJobData(SEQUENTIAL),
              device(_device)
            {}

        KisPaintDeviceSP device;
    };

    class ProcessData : public KisStrokeJobData {
    public:
        ProcessData(KisImageWSP _image, const QRect &_rect)
            : KisStrokeJobData(CONCURRENT),
              image(_image), rect(_rect)
            {}

        KisImageWSP image;
        QRect rect;
    };

};

OverviewWidget::OverviewWidget(QWidget * parent)
    : QWidget(parent)
    , m_compressor(new KisSignalCompressor(500, KisSignalCompressor::POSTPONE, this))
    , m_canvas(0)
    , m_dragging(false)
    , m_imageIdleWatcher(500)
    , m_thumbnailNeedsUpdate(true)
{
    setMouseTracking(true);
    connect(m_compressor, SIGNAL(timeout()), SLOT(startUpdateCanvasProjection()));
    KisConfig cfg;
    QRgb c = cfg.readEntry("OverviewWidgetColor", 0xFF454C);
    m_outlineColor = QColor(c);
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
//        m_imageIdleConnection.reset(
//            new KisSignalAutoConnection(&m_imageIdleWatcher, SIGNAL(startedIdleMode()),
//                                        this, SLOT(generateThumbnail())));
        connect(&m_imageIdleWatcher,&KisIdleWatcher::startedIdleMode, this,&OverviewWidget::generateThumbnail);

        connect(m_canvas->image(), SIGNAL(sigImageUpdated(QRect)), m_compressor, SLOT(start()), Qt::UniqueConnection);
        connect(m_canvas->image(), SIGNAL(sigSizeChanged(QPointF, QPointF)), m_compressor, SLOT(start()), Qt::UniqueConnection);
        connect(m_canvas->canvasController()->proxyObject, SIGNAL(canvasOffsetXChanged(int)), this, SLOT(update()), Qt::UniqueConnection);
        m_compressor->start();
    }
}

QSize OverviewWidget::calculatePreviewSize()
{
    QSize imageSize(m_canvas->image()->bounds().size());
    imageSize.scale(size(), Qt::KeepAspectRatio);
    return imageSize;
}

QPointF OverviewWidget::previewOrigin()
{
    return QPointF((width() - m_pixmap.width()) / 2.0f, (height() - m_pixmap.height()) / 2.0f);
}

QPolygonF OverviewWidget::previewPolygon()
{
    if (m_canvas) {
        const KisCoordinatesConverter* converter = m_canvas->coordinatesConverter();
        QPolygonF canvasPoly = QPolygonF(QRectF(m_canvas->canvasWidget()->rect()));
        QPolygonF imagePoly = converter->widgetToImage<QPolygonF>(canvasPoly);

        QTransform imageToPreview = imageToPreviewTransform();

        return imageToPreview.map(imagePoly);
    }
    return QPolygonF();
}

QTransform OverviewWidget::imageToPreviewTransform()
{
    QTransform imageToPreview;
    imageToPreview.scale(calculatePreviewSize().width() / (float)m_canvas->image()->width(),
                         calculatePreviewSize().height() / (float)m_canvas->image()->height());
    return imageToPreview;
}

void OverviewWidget::startUpdateCanvasProjection()
{
    qDebug() << "startUpdateCanvasProjection called";
    if (!m_canvas) return;

    m_thumbnailNeedsUpdate = true;

    update();
}

void OverviewWidget::showEvent(QShowEvent *event)
{
    Q_UNUSED(event);
    m_compressor->start();
}

void OverviewWidget::resizeEvent(QResizeEvent *event)
{
    Q_UNUSED(event);
    if (m_canvas) {
        if (!m_pixmap.isNull()) {
            QSize newSize = calculatePreviewSize();
            m_pixmap = m_pixmap.scaled(newSize, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
        m_compressor->start();
    }
}

void OverviewWidget::mousePressEvent(QMouseEvent* event)
{
    if (m_canvas) {
        QPointF previewPos = event->pos() - previewOrigin();

        if (previewPolygon().containsPoint(previewPos, Qt::WindingFill)) {
            m_lastPos = previewPos;
            m_dragging = true;
        }
    }
    event->accept();
    update();
}

void OverviewWidget::mouseMoveEvent(QMouseEvent* event)
{
    if (m_dragging) {
        QPointF previewPos = event->pos() - previewOrigin();

        // position is mapped from preview image->image->canvas coordinates
        QTransform previewToImage = imageToPreviewTransform().inverted();
        const KisCoordinatesConverter* converter = m_canvas->coordinatesConverter();

        QPointF lastImagePos = previewToImage.map(m_lastPos);
        QPointF newImagePos = previewToImage.map(previewPos);

        QPointF lastWidgetPos = converter->imageToWidget<QPointF>(lastImagePos);
        QPointF newWidgetPos = converter->imageToWidget<QPointF>(newImagePos);

        QPointF diff = newWidgetPos - lastWidgetPos;
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
    float delta = event->delta();

    if (delta > 0) {
        m_canvas->viewManager()->zoomController()->zoomAction()->zoomIn();
    } else {
        m_canvas->viewManager()->zoomController()->zoomAction()->zoomOut();
    }
}

void OverviewWidget::generateThumbnail()
{
    QMutexLocker locker(&mutex);
    if (m_canvas && m_thumbnailNeedsUpdate) {
        KisImageSP image = m_canvas->image();

        if(!strokeId.isNull()){
            image->cancelStroke(strokeId);
            image->waitForDone();
        }

        OverviewThumbnailStrokeStrategy* stroke = new OverviewThumbnailStrokeStrategy(image, calculatePreviewSize());
        qDebug() << __FUNCTION__ << 1;
        strokeId = image->startStroke(stroke);
        qDebug() << __FUNCTION__ << 2;
        QList<KisStrokeJobData*> jobs = OverviewThumbnailStrokeStrategy::createJobsData(image);

        Q_FOREACH (KisStrokeJobData *jd, jobs) {
            image->addJob(strokeId,jd);
            qDebug() << __FUNCTION__ << jd;
        }
        qDebug() << __FUNCTION__ << 3;
        image->endStroke(strokeId);
        qDebug() << __FUNCTION__ << 4;
//        QSize previewSize = calculatePreviewSize();

//        if (isVisible() && previewSize.isValid()) {
//            QImage img =
//                image->projection()->
//                createThumbnail(previewSize.width(), previewSize.height(), image->bounds(), /*oversample=*/ 2,
//                                KoColorConversionTransformation::internalRenderingIntent(),
//                                KoColorConversionTransformation::internalConversionFlags()
//                               );

//            m_pixmap = QPixmap::fromImage(img);
//        }
//        update();
        m_thumbnailNeedsUpdate = false;
    }
}


void OverviewWidget::paintEvent(QPaintEvent* event)
{
    QWidget::paintEvent(event);

    if (m_canvas) {
        QPainter p(this);
        p.translate(previewOrigin());

        p.drawPixmap(0, 0, m_pixmap.width(), m_pixmap.height(), m_pixmap);

        QRect r = rect().translated(-previewOrigin().toPoint());
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

OverviewThumbnailStrokeStrategy::OverviewThumbnailStrokeStrategy(KisImageWSP image, QSize thumbnailSize)
    : KisSimpleStrokeStrategy("OverviewThumbnail"), m_thumbnailSize(thumbnailSize)
{
    enableJob(KisSimpleStrokeStrategy::JOB_INIT, true, KisStrokeJobData::BARRIER, KisStrokeJobData::EXCLUSIVE);
    enableJob(KisSimpleStrokeStrategy::JOB_DOSTROKE);
    enableJob(KisSimpleStrokeStrategy::JOB_FINISH);
    enableJob(KisSimpleStrokeStrategy::JOB_CANCEL, true, KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);

    setRequestsOtherStrokesToEnd(false);
    setClearsRedoOnStart(false);
    setCanForgetAboutMe(true);
}

QList<KisStrokeJobData *> OverviewThumbnailStrokeStrategy::createJobsData(KisImageWSP image)
{
    QVector<QRect> rects = KritaUtils::splitRectIntoPatches(m_thumbnailSize, QSize(32,32));
    QList<KisStrokeJobData*> jobsData;
    Q_FOREACH (const QRect &rc, rects) {
        jobsData << new OverviewThumbnailStrokeStrategy::Private::ProcessData(image, rc);
    }
    return jobsData;
}

OverviewThumbnailStrokeStrategy::~OverviewThumbnailStrokeStrategy()
{
    qDebug() << __FUNCTION__ << " " << this;
}


void OverviewThumbnailStrokeStrategy::initStrokeCallback()
{
    qDebug() << __FUNCTION__;
}
#include <QThread>
void OverviewThumbnailStrokeStrategy::doStrokeCallback(KisStrokeJobData *data)
{
    Private::ProcessData *d = dynamic_cast<Private::ProcessData*>(data);
    qDebug() << "+  " << __FUNCTION__ << " " << d->rect;
}

void OverviewThumbnailStrokeStrategy::finishStrokeCallback()
{
    qDebug() << __FUNCTION__;
}

void OverviewThumbnailStrokeStrategy::cancelStrokeCallback()
{
    qDebug() << __FUNCTION__;
}

