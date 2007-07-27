/*
 *  kis_tool_brush.cc - part of Krita
 *
 *  Copyright (c) 2003-2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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
#include <QEvent>
#include <QLabel>
#include <QLayout>
#include <QMutex>
#include <QMutexLocker>
#include "QPainter"
#include <QRect>
#include <QThread>
#include <QWidget>

#include <kdebug.h>
#include <klocale.h>

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>

// Krita/image
#include <kis_action_recorder.h>
#include <kis_brush.h>
#include <kis_fill_painter.h>
#include <kis_group_layer.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_recorded_paint_actions.h>
#include <kis_selection.h>

// Krita/ui
#include "kis_boundary_painter.h"
#include "kis_canvas2.h"
#include "kis_cursor.h"
#include "kis_tool_freehand.h"


class FreehandPaintJob {
public:
    FreehandPaintJob(KisToolFreehand* freeHand,
                     KisPainter* painter,
                     const KisPaintInformation & pi1,
                     const KisPaintInformation & pi2,
                     const FreehandPaintJob* previousPaintJob);
public:
    double dragDist() const { return m_dragDist; }
public:
    virtual void run() =0;
protected:
    KisToolFreehand* m_toolFreeHand;
    KisPainter* m_painter;
    double m_dragDist;
    KisPaintInformation m_pi1;
    KisPaintInformation m_pi2;
    const FreehandPaintJob* m_previousPaintJob;
};

FreehandPaintJob::FreehandPaintJob(KisToolFreehand* toolFreeHand, KisPainter* painter,
                                   const KisPaintInformation & pi1,
                                   const KisPaintInformation & pi2,
                                   const FreehandPaintJob* previousPaintJob) :
    m_toolFreeHand(toolFreeHand),
    m_painter(painter),
    m_pi1(pi1),
    m_pi2(pi2),
    m_previousPaintJob(previousPaintJob)
{
}

class FreehandPaintLineJob : public FreehandPaintJob {
public:
    FreehandPaintLineJob(KisToolFreehand* freeHand,
                         KisPainter* painter,
                         const KisPaintInformation & pi1,
                         const KisPaintInformation & pi2,
                         const FreehandPaintJob* previousPaintJob);
    virtual void run();
};


FreehandPaintLineJob::FreehandPaintLineJob(KisToolFreehand* toolFreeHand, KisPainter* painter,
                                           const KisPaintInformation & pi1,
                                           const KisPaintInformation & pi2,
                                           const FreehandPaintJob* previousPaintJob) : FreehandPaintJob(toolFreeHand, painter, pi1, pi2, previousPaintJob)
{
}

void FreehandPaintLineJob::run()
{
    m_dragDist = (m_previousPaintJob) ? m_dragDist = m_previousPaintJob->dragDist() : 0.0;
    m_dragDist = m_painter->paintLine(m_pi1, m_pi2, m_dragDist);
    m_toolFreeHand->setDirty( m_painter->dirtyRegion() );
}

class FreehandPaintBezierJob : public FreehandPaintJob {
public:
    FreehandPaintBezierJob(KisToolFreehand* freeHand,
                           KisPainter* painter,
                           const KisPaintInformation & pi1,
                           const QPointF& control1,
                           const QPointF& control2,
                           const KisPaintInformation & pi2,
                           const FreehandPaintJob* previousPaintJob);
    virtual void run();
private:
    QPointF m_control1;
    QPointF m_control2;
};


FreehandPaintBezierJob::FreehandPaintBezierJob(KisToolFreehand* toolFreeHand, KisPainter* painter,
                                               const KisPaintInformation & pi1,
                                               const QPointF& control1,
                                               const QPointF& control2,
                                               const KisPaintInformation & pi2,
                                               const FreehandPaintJob* previousPaintJob) : FreehandPaintJob(toolFreeHand, painter, pi1, pi2, previousPaintJob), m_control1(control1),m_control2(control2)
{
}

void FreehandPaintBezierJob::run()
{
    m_dragDist = (m_previousPaintJob) ? m_dragDist = m_previousPaintJob->dragDist() : 0.0;
    m_dragDist = m_painter->paintBezierCurve(m_pi1, m_control1, m_control2, m_pi2, m_dragDist);
    m_toolFreeHand->setDirty( m_painter->dirtyRegion() );
}

class FreehandPaintJobExecutor : public QThread {
    public:
        FreehandPaintJobExecutor() : m_finish(false) {
        }
        virtual void run()
        {
            QMutexLocker lockRunning(&m_mutex_running);
            kDebug(41007) << "run" << endl;
            while(not m_finish or not empty() )
            {
                FreehandPaintJob* nextJob = 0;
                {
                    QMutexLocker lock(&m_mutex_queue);
                    if(m_queue.size() > 0)
                    {
                        nextJob = m_queue.dequeue();
                    }
                }
                kDebug(41007) << "nextJob = " << nextJob << endl;
                if(nextJob)
                {
                    nextJob->run();
                } else {
                    msleep(1);
                }
            }
            kDebug(41007) << "finish running" << endl;
        }
        void postJob(FreehandPaintJob* job)
        {
            QMutexLocker lock(&m_mutex_queue);
            kDebug(41007) << "push job = " << job << endl;
            m_queue.enqueue(job);
        }
        void finish() {
            m_finish = true;
            QMutexLocker lockRunning(&m_mutex_running);
        }
        bool empty() {
            QMutexLocker lock(&m_mutex_queue);
            return m_queue.size() == 0;
        }
        int queueLength() {
            QMutexLocker lock(&m_mutex_queue);
            return m_queue.size();
        }
        void start() {
            m_finish = false;
            QThread::start();
        }
    private:
        QQueue<FreehandPaintJob* > m_queue;
        QMutex m_mutex_queue;
        QMutex m_mutex_running;
        bool m_finish;
};


KisToolFreehand::KisToolFreehand(KoCanvasBase * canvas, const QCursor & cursor, const QString & transactionText)
    : KisToolPaint(canvas, cursor)
    , m_dragDist ( 0 )
    , m_transactionText(transactionText)
    , m_mode( HOVER )
{
    m_painter = 0;
    m_tempLayer = 0;
    m_paintIncremental = true;
    m_paintOnSelection = false;
    m_paintedOutline = false;
    m_executor = new FreehandPaintJobExecutor();
    m_smooth = false;
    m_smoothness = 0.5;
}

KisToolFreehand::~KisToolFreehand()
{
}


void KisToolFreehand::mousePressEvent(KoPointerEvent *e)
{
    if (!currentImage()) return;

    if (!currentBrush()) return;

    if (!currentLayer()->paintDevice()) return;

    if(currentPaintOpSettings())
    {
        currentPaintOpSettings()->mousePressEvent(e);
        if(e->isAccepted()) return;
    }

    if (e->button() == Qt::LeftButton)
    {
        initPaint(e);
        m_previousPaintInformation = KisPaintInformation(convertToPixelCoord(e),
                                                         e->pressure(), e->xTilt(), e->yTilt());
        paintAt(m_previousPaintInformation);
    }
}

inline double norm(const QPointF& p)
{
    return sqrt(p.x()*p.x() + p.y()*p.y());
}

inline double angle(const QPointF& p1, const QPointF& p2)
{
    return atan2(p1.y(), p1.x()) - atan2(p2.y(), p2.x());
}

void KisToolFreehand::mouseMoveEvent(KoPointerEvent *e)
{
    if (m_mode == PAINT) {
        QPointF pos = convertToPixelCoord(e);

        KisPaintInformation info = KisPaintInformation(convertToPixelCoord(e),
                                                       e->pressure(), e->xTilt(), e->yTilt());
        if(m_smooth)
        {
            QPointF dragVec = info.pos - m_previousPaintInformation.pos;
            QPointF newTangent;
            if( m_previousDrag.y() == 0.0 and m_previousDrag.x() == 0.0 )
            {
                newTangent = dragVec;
            } else {
                double angleTangent = angle( dragVec, m_previousDrag);
                double cosTangent = cos(angleTangent);
                double sinTangent = sin(angleTangent);
                newTangent = QPointF(
                    cosTangent * dragVec.x() - sinTangent * dragVec.y(),
                    sinTangent * dragVec.x() + cosTangent * dragVec.y() );
            }

            if ( norm( newTangent ) != 0 ) {
                newTangent /= norm( newTangent ) ;
            }

            double cosPreviousNewTangent = cos(angle(newTangent,m_previousTangent ));
            newTangent += m_previousTangent;
            newTangent *= 0.5 * cosPreviousNewTangent;

            if ( norm( dragVec ) != 0 ) {
                newTangent += (1.0 - cosPreviousNewTangent ) * dragVec / norm(dragVec);
            }

            newTangent *= m_smoothness / norm( newTangent ) ;
            double normVec = 0.5 * norm(dragVec);
            paintBezierCurve(m_previousPaintInformation,
                             m_previousPaintInformation.pos + m_previousTangent * normVec,
                             info.pos - newTangent * normVec,
                             info);
            m_previousTangent = newTangent;
            m_previousDrag = dragVec;
        } else {
            paintLine(m_previousPaintInformation, info);
            m_polyLinePaintAction->addPoint(info);
        }

        m_previousPaintInformation = info;
    }
}

void KisToolFreehand::mouseReleaseEvent(KoPointerEvent* e)
{
    if (e->button() == Qt::LeftButton && m_mode == PAINT) {
        endPaint();
    }
}



void KisToolFreehand::initPaint(KoPointerEvent *)
{
    if (!currentLayer() || !currentLayer()->paintDevice()) return;

    if (m_compositeOp == 0 ) {
        KisPaintDeviceSP device = currentLayer()->paintDevice();
        if (device) {
            m_compositeOp = device->colorSpace()->compositeOp( COMPOSITE_OVER );
        }
    }

    m_mode = PAINT;
    m_dragDist = 0;

    // Create painter
    KisPaintDeviceSP device = currentLayer()->paintDevice();

    if (m_painter)
        delete m_painter;

    if (!m_paintIncremental) {

        KisIndirectPaintingSupport* layer;
        if ((layer = dynamic_cast<KisIndirectPaintingSupport*>(
                 currentLayer().data())))
        {
            // Hack for the painting of single-layered layers using indirect painting,
            // because the group layer would not have a correctly synched cache (
            // because of an optimization that would happen, having this layer as
            // projection).
            KisLayerSP l = layer->layer();
            KisPaintLayerSP pl = dynamic_cast<KisPaintLayer*>(l.data());
            if (l->parentLayer() && (l->parentLayer()->parentLayer() == 0)
                && (l->parentLayer()->childCount() == 1)
                && l->parentLayer()->paintLayerInducesProjectionOptimization(pl))
            {
                // If there's a mask, device could've been the mask. The induce function
                // should catch this, but better safe than sorry

                // XXX: What does this and why? (BSAR)
                l->parentLayer()->resetProjection(pl->paintDevice());
            }

            m_target = new KisPaintDevice(currentLayer().data(),
                                          device->colorSpace());
            layer->setTemporaryTarget(m_target);
            layer->setTemporaryCompositeOp(m_compositeOp);
            layer->setTemporaryOpacity(m_opacity);

            if (device->hasSelection())
                m_target->setSelection(device->selection());
        }
    } else {
        m_target = device;
    }
    m_painter = new KisPainter( m_target );
    Q_CHECK_PTR(m_painter);
    m_source = device;
    m_painter->beginTransaction(m_transactionText);

    m_painter->setPaintColor(currentFgColor());
    m_painter->setBackgroundColor(currentBgColor());
    m_painter->setBrush(currentBrush());


    // if you're drawing on a temporary layer, the layer already sets this
    if (m_paintIncremental) {
        m_painter->setCompositeOp(m_compositeOp);
        m_painter->setOpacity(m_opacity);
    } else {
        m_painter->setCompositeOp(device->colorSpace()->compositeOp(COMPOSITE_ALPHA_DARKEN));
        m_painter->setOpacity( OPACITY_OPAQUE );

    }
    m_previousTangent = QPointF(0,0);
    m_previousDrag = QPointF(0,0);
/*    kDebug(41007) << "target: " << m_target << "( " << m_target->name() << " )"
      << " source: " << m_source << "( " << m_source->name() << " )"
      << ", incremental " << m_paintIncremental
      << ", paint on selection: " << m_paintOnSelection
      << ", active device has selection: " << device->hasSelection()
//       << ", target has selection: " << m_target->hasSelection()
      << endl;
*/
    if(m_smooth)
    {
    } else {
        m_polyLinePaintAction = new KisRecordedPolyLinePaintAction(i18n("Freehand tool"), currentLayer(), currentBrush(), currentPaintOp() );
    }
    m_executor->start();
}

void KisToolFreehand::endPaint()
{
    m_mode = HOVER;
    m_executor->finish();
    if (currentImage()) {

        if (m_painter) {
            // If painting in mouse release, make sure painter
            // is destructed or end()ed
            if (!m_paintIncremental) {
                m_painter->endTransaction();

                KisPainter painter( m_source );
                painter.setCompositeOp(m_compositeOp);

                painter.beginTransaction(m_transactionText);

                QRegion r = m_incrementalDirtyRegion;
                QVector<QRect> dirtyRects = r.rects();
                QVector<QRect>::iterator it = dirtyRects.begin();
                QVector<QRect>::iterator end = dirtyRects.end();

                while (it != end) {

                    painter.bitBlt(it->x(), it->y(), m_compositeOp, m_target,
                                   m_opacity,
                                   it->x(), it->y(),
                                   it->width(), it->height());
                    ++it;
                }
                KisIndirectPaintingSupport* layer =
                    dynamic_cast<KisIndirectPaintingSupport*>(currentLayer().data());
                layer->setTemporaryTarget(0);
                m_source->setDirty(painter.dirtyRegion());

                m_canvas->addCommand(painter.endTransaction());
            } else {
                m_canvas->addCommand(m_painter->endTransaction());
            }
        }
        delete m_painter;
        m_painter = 0;
        notifyModified();
    }
    if(not m_paintJobs.empty())
    {
        foreach(FreehandPaintJob* job , m_paintJobs)
        {
            delete job;
        }
        m_paintJobs.clear();
    }
    if(m_smooth)
    {
    } else {
        currentLayer()->image()->actionRecorder()->addAction(m_polyLinePaintAction);
        m_polyLinePaintAction = 0;
    }
}

void KisToolFreehand::paintAt(const KisPaintInformation &pi)
{
    m_painter->paintAt(KisPaintInformation(pi));
    setDirty( m_painter->dirtyRegion() );
}

void KisToolFreehand::paintLine(const KisPaintInformation &pi1,
                                const KisPaintInformation &pi2)
{
    FreehandPaintJob* previousJob = m_paintJobs.empty() ? 0 : m_paintJobs.last();
    queuePaintJob( new FreehandPaintLineJob(this, m_painter, pi1, pi2, previousJob), previousJob );
}

void KisToolFreehand::paintBezierCurve(const KisPaintInformation &pi1,
                                       const QPointF &control1,
                                       const QPointF &control2,
                                       const KisPaintInformation &pi2)
{
    FreehandPaintJob* previousJob = m_paintJobs.empty() ? 0 : m_paintJobs.last();
    queuePaintJob( new FreehandPaintBezierJob(this, m_painter, pi1, control1, control2, pi2, previousJob), previousJob );
}

void KisToolFreehand::queuePaintJob(FreehandPaintJob* job, FreehandPaintJob* /*previousJob*/)
{
    m_paintJobs.append(job);
    kDebug(41007) << "Queue length: " << m_executor->queueLength() << endl;
    m_executor->postJob(job);
}

void KisToolFreehand::setDirty(const QRegion& region)
{
    if (!m_paintOnSelection) {
        currentLayer()->setDirty(region);
    }
    else {
        // Just update the canvas
        // XXX: How to do this hack with regions?
        // r = QRect(r.left()-1, r.top()-1, r.width()+2, r.height()+2); //needed to update selectionvisualization
        m_target->setDirty(region);
    }
    if (!m_paintIncremental) {
        m_incrementalDirtyRegion += region;
    }
}

void KisToolFreehand::setSmooth(bool smooth)
{
    m_smooth = smooth;
}


void KisToolFreehand::paintOutline(const QPointF& point) {
    Q_UNUSED( point );
#if 0
    // XXX TOOL_PORT
    // I don't get this 1.6 code: we update the whole canvas, and then
    // repaint all of it again? It's incompatible with qt4 anyway --
    // rework later.

    if (!m_canvas) {
        return;
    }

    if (currentImage() && !currentImage()->bounds().contains(point.floorQPoint())) {
        if (m_paintedOutline) {
            m_canvas->updateCanvas(); // Huh? The _whole_ canvas needs to be
            // repainted for this outline cursor?
            // It was this way in 1.6, btw.
            m_paintedOutline = false;
        }
        return;
    }

    KisCanvas *canvas = controller->kiscanvas();
    canvas->repaint();

    KisBrush *brush = m_subject->currentBrush();
    // There may not be a brush present, and we shouldn't crash in that case
    if (brush) {
        QPainter gc( canvas->canvasWidget() );
        QPen pen(Qt::SolidLine);

        QPointF hotSpot = brush->hotSpot();

        //gc.setRasterOp(Qt::NotROP);
        gc.setPen(pen);
        gc.setViewport(0, 0, static_cast<qint32>(canvas->width() * m_subject->zoomFactor()),
                       static_cast<qint32>(canvas->height() * m_subject->zoomFactor()));
        gc.translate((- controller->horzValue()) / m_subject->zoomFactor(),
                     (- controller->vertValue()) / m_subject->zoomFactor());

        QPointF topLeft = point - hotSpot;

        if (m_subject->currentPaintop().id() == "pen") {
            // Pen paints on whole pixels only.
            topLeft = topLeft.roundQPoint();
        }

        gc.translate(topLeft.x(), topLeft.y());
        gc.end();
        KisBoundaryPainter::paint(brush->boundary(), gc);

        m_paintedOutline = true;
    }
#endif
}


#include "kis_tool_freehand.moc"

