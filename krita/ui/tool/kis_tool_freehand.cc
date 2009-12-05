/*
 *  kis_tool_freehand.cc - part of Krita
 *
 *  Copyright (c) 2003-2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2007-2008 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include "kis_tool_freehand.h"
#include "kis_tool_freehand_p.h"

#include <QEvent>
#include <QLabel>
#include <QLayout>
#include <QMutex>
#include <QMutexLocker>
#include "QPainter"
#include <QRect>
#include <QThreadPool>
#include <QWidget>

#include <kis_debug.h>
#include "kis_config.h"
#include <klocale.h>

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>
#include <KoCanvasController.h>
#include <KoViewConverter.h>

// Krita/image
#include <recorder/kis_action_recorder.h>
#include <kis_fill_painter.h>
#include <kis_group_layer.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <recorder/kis_recorded_polyline_paint_action.h>
#include <recorder/kis_recorded_bezier_curve_paint_action.h>
#include <kis_selection.h>
#include <kis_paintop_preset.h>

#include <kis_transaction.h>

// Krita/ui
#include "canvas/kis_canvas2.h"
#include "kis_cursor.h"
#include "kis_painting_assistant.h"
#include <recorder/kis_node_query_path.h>
#include <kis_view2.h>
#include <kis_painting_assistants_manager.h>
#include <kis_3d_object_model.h>
#include "kis_color_picker_utils.h"

// OpenGL
#include <config-opengl.h>

#ifdef HAVE_OPENGL
#include <GL/gl.h>
#include <GL/glu.h>
#endif


#define ENABLE_RECORDING

KisToolFreehand::KisToolFreehand(KoCanvasBase * canvas, const QCursor & cursor, const QString & transactionText)
    : KisToolPaint(canvas, cursor)
    , m_dragDist(0)
    , m_transactionText(transactionText)
    , m_mode(HOVER)
{
    m_painter = 0;
    m_tempLayer = 0;
    m_paintIncremental = false;
    m_paintOnSelection = false;
    m_paintedOutline = false;
    m_executor = new QThreadPool(this);
    m_executor->setMaxThreadCount(1);
    m_smooth = true;
    m_assistant = false;
    m_smoothness = 0.5;
    m_magnetism = 1.0;

    setSupportOutline(true);

#if defined(HAVE_OPENGL)
    m_xTilt = 0.0;
    m_yTilt = 0.0;
    m_prevxTilt = 0.0;
    m_prevyTilt = 0.0;
#endif
}

KisToolFreehand::~KisToolFreehand()
{
    delete m_painter;
}

void KisToolFreehand::mousePressEvent(KoPointerEvent *e)
{
    //    dbgUI << "mousePressEvent" << m_mode;
    //     if (!currentImage())
    //    return;
    if (m_mode == PAN) {
        initPan(e);
        return;
    }

    if (!currentNode())
        return;

    if (!currentNode()->paintDevice())
        return;

    // control-click gets the color at the current point. For now, only with a ratio of 1
    if (e->modifiers() == (Qt::ControlModifier | Qt::ShiftModifier)) {
        if (e->button() == Qt::LeftButton)
            m_canvas->resourceProvider()->setResource(KoCanvasResource::ForegroundColor,
                                                      KisToolUtils::pick(currentNode()->paintDevice(),
                                                                         convertToIntPixelCoord(e)));
        else
            m_canvas->resourceProvider()->setResource(KoCanvasResource::BackgroundColor,
                                                      KisToolUtils::pick(currentNode()->paintDevice(),
                                                                         convertToIntPixelCoord(e)));

    }
    else if (e->modifiers() == Qt::ControlModifier ) {

        if (e->button() == Qt::LeftButton)
            m_canvas->resourceProvider()->setResource(KoCanvasResource::ForegroundColor,
                                                      KisToolUtils::pick(currentImage()->mergedImage(),
                                                                         convertToIntPixelCoord(e)));
        else
            m_canvas->resourceProvider()->setResource(KoCanvasResource::BackgroundColor,
                                                      KisToolUtils::pick(currentImage()->mergedImage(),
                                                                         convertToIntPixelCoord(e)));
    }
    else if (e->modifiers() == Qt::ShiftModifier){
        m_mode = EDIT_BRUSH;
        m_prevMousePos = e->point;
        m_originalPos = e->globalPos();
        return;
    }
    else { // No modifiers

        if (currentPaintOpPreset() && currentPaintOpPreset()->settings()) {
            m_paintIncremental = currentPaintOpPreset()->settings()->paintIncremental();
            currentPaintOpPreset()->settings()->mousePressEvent(e);
            if (e->isAccepted()) {
                return;
            }
        }

        if (e->button() == Qt::LeftButton) {
            initPaint(e);
            m_previousPaintInformation = KisPaintInformation(convertToPixelCoord(adjustPosition(e->point)),
                                                             e->pressure(), e->xTilt(), e->yTilt(),
                                                             KisVector2D::Zero(),
                                                             e->rotation(), e->tangentialPressure());
            paintAt(m_previousPaintInformation);
            if (!m_smooth) {
#ifdef ENABLE_RECORDING
                m_polyLinePaintAction->addPoint(m_previousPaintInformation);
#endif
            }
        }
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
    //    dbgUI << "mouseMoveEvent " << m_mode << " " << e->button() << " " << e->buttons();
    switch (m_mode) {
    case PAINT:
        {
            QPointF pos = convertToPixelCoord(adjustPosition(e->point));
            QPointF dragVec = pos - m_previousPaintInformation.pos();

            KisPaintInformation info = KisPaintInformation(pos,
                                                           e->pressure(), e->xTilt(), e->yTilt(),
                                                           toKisVector2D(dragVec),
                                                           e->rotation(), e->tangentialPressure());

            if (m_smooth) {
                QPointF newTangent;
                if (m_previousDrag.y() == 0.0 && m_previousDrag.x() == 0.0) {
                    newTangent = dragVec;
                } else {
                    double angleTangent = angle(dragVec, m_previousDrag);
                    double cosTangent = cos(angleTangent);
                    double sinTangent = sin(angleTangent);
                    newTangent = QPointF(
                            cosTangent * dragVec.x() - sinTangent * dragVec.y(),
                            sinTangent * dragVec.x() + cosTangent * dragVec.y());
                }

                if (norm(newTangent) != 0) {
                    newTangent /= norm(newTangent) ;
                }

                double cosPreviousNewTangent = cos(angle(newTangent, m_previousTangent));
                newTangent += m_previousTangent;
                newTangent *= 0.5 * cosPreviousNewTangent;

                if (norm(dragVec) != 0) {
                    newTangent += (1.0 - cosPreviousNewTangent) * dragVec / norm(dragVec);
                }

                newTangent *= m_smoothness / norm(newTangent) ;
                double normVec = 0.5 * norm(dragVec);
                QPointF control1 = m_previousPaintInformation.pos() + m_previousTangent * normVec;
                QPointF control2 = info.pos() - newTangent * normVec;

                paintBezierCurve(m_previousPaintInformation,
                                 control1,
                                 control2,
                                 info);
#ifdef ENABLE_RECORDING
                m_bezierCurvePaintAction->addPoint(m_previousPaintInformation, control1, control2, info);
#endif
                m_previousTangent = newTangent;
                m_previousDrag = dragVec;
            } else {
                paintLine(m_previousPaintInformation, info);
#ifdef ENABLE_RECORDING
                m_polyLinePaintAction->addPoint(info);
#endif
            }

            m_previousPaintInformation = info;
        }
        break;
    case EDIT_BRUSH:
        {
            useCursor(KisCursor::blankCursor());
            qreal dx = m_prevMousePos.x() - e->point.x();
            qreal dy = m_prevMousePos.y() - e->point.y();
            currentPaintOpPreset()->settings()->changePaintOpSize( -dx, -dy );
            QCursor::setPos(m_originalPos);
        }
        break;
    case PAN:
        {
            pan(e);
        }
    default:
        ;
    };

    KisConfig cfg;
    KisPaintOpSettings::OutlineMode outlineMode;
    if (m_mode != PAINT && (cfg.cursorStyle() == CURSOR_STYLE_OUTLINE || m_mode == EDIT_BRUSH)) {
        outlineMode = KisPaintOpSettings::CURSOR_IS_OUTLINE;
    } else {
        outlineMode = KisPaintOpSettings::CURSOR_ISNT_OUTLINE;
    }

    if (!oldOutlineRect.isEmpty()) {
        m_canvas->updateCanvas(oldOutlineRect); // erase the old guy
    }

    mousePos = e->point;

#if defined(HAVE_OPENGL)
    if (cfg.cursorStyle() == CURSOR_STYLE_3D_MODEL) {
        if (m_canvas->canvasController()->isCanvasOpenGL()) {
            m_xTilt = e->xTilt();
            m_yTilt = e->yTilt();
            // TODO : optimize? but you need to know the size of the 3d brush?
            m_canvas->updateCanvas(QRect(QPoint(0, 0), QSize(currentImage()->width(), currentImage()->height())));
        }
    }
#endif

    oldOutlineRect = currentPaintOpPreset()->settings()->paintOutlineRect(mousePos, currentImage(), outlineMode);
    if (!oldOutlineRect.isEmpty()) {
        m_canvas->updateCanvas(oldOutlineRect); // erase the old guy
    }

}

void KisToolFreehand::mouseReleaseEvent(KoPointerEvent* e)
{
    //    dbgUI << "mouseReleaseEvent" << m_mode << " " << e->button() << " " << e->button();
    switch(m_mode) {
    case PAINT:
        endPaint();
        break;
    case PAN:
        endPan();
        break;
    default:
        ;
    };
    KisToolPaint::mouseReleaseEvent(e);
    m_mode = HOVER;
    resetCursorStyle();
}

void KisToolFreehand::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Space) {
        m_mode = PAN;
        useCursor(Qt::OpenHandCursor);

        event->accept();
    }
    else {
        event->ignore();
    }
}

void KisToolFreehand::initPaint(KoPointerEvent *)
{
    //    dbgUI << "initPaint";
    if (!currentNode() || !currentNode()->paintDevice())
        return;

    if (m_compositeOp == 0) {
        KisPaintDeviceSP device = currentNode()->paintDevice();
        if (device) {
            m_compositeOp = device->colorSpace()->compositeOp(COMPOSITE_OVER);
        }
    }

    m_mode = PAINT;
    m_dragDist = 0;

    // Create painter
    KisPaintDeviceSP device = currentNode()->paintDevice();

    if (m_painter)
        delete m_painter;

    if (!m_paintIncremental) {
        KisIndirectPaintingSupport* layer;
        if ((layer = dynamic_cast<KisIndirectPaintingSupport*>(currentNode().data()))) {
            // Hack for the painting of single-layered layers using indirect painting,
            // because the group layer would not have a correctly synched cache (
            // because of an optimization that would happen, having this layer as
            // projection).
            KisLayerSP l = layer->layer();
            KisPaintLayerSP pl = dynamic_cast<KisPaintLayer*>(l.data());
            m_target = new KisPaintDevice(currentNode().data(), device->colorSpace());
            layer->setTemporaryTarget(m_target);
            layer->setTemporaryCompositeOp(m_compositeOp);
            layer->setTemporaryOpacity(m_opacity);
        }
    }
    if (!m_target) {
        m_target = device;
    }
    m_painter = new KisPainter(m_target, currentSelection());

    m_source = device;
    m_painter->beginTransaction(m_transactionText);

    setupPainter(m_painter);

    if (m_paintIncremental) {
        if (KisPaintLayer* l = dynamic_cast<KisPaintLayer*>(currentNode().data())) {
            m_painter->setChannelFlags(l->channelFlags());
            if (l->alphaLocked()) {
                m_painter->setLockAlpha(l->alphaLocked());
            }
        }
    }

    // if you're drawing on a temporary layer, the layer already sets this
    if (m_paintIncremental) {
        m_painter->setCompositeOp(m_compositeOp);
        m_painter->setOpacity(m_opacity);
    } else {
        m_painter->setCompositeOp(device->colorSpace()->compositeOp(COMPOSITE_ALPHA_DARKEN));
        m_painter->setOpacity(OPACITY_OPAQUE);
    }

    m_previousTangent = QPointF(0, 0);
    m_previousDrag = QPointF(0, 0);
    /*    dbgUI <<"target:" << m_target <<"(" << m_target->name() <<" )"
          << " source: " << m_source << "( " << m_source->name() << " )"
          << ", incremental " << m_paintIncremental
          << ", paint on selection: " << m_paintOnSelection
          << ", active device has selection: " << device->hasSelection()
          << endl;
    */
#ifdef ENABLE_RECORDING // Temporary, to figure out what is going without being
    // distracted by the recording
    if (m_smooth) {
        m_bezierCurvePaintAction = new KisRecordedBezierCurvePaintAction(i18n("Freehand tool"),
                                                                         KisNodeQueryPath::absolutePath(currentNode()),
                                                                         currentPaintOpPreset(),
                                                                         m_painter->paintColor(),
                                                                         m_painter->backgroundColor(),
                                                                         m_painter->opacity(),
                                                                         m_paintIncremental,
                                                                         m_compositeOp);
    } else {
        m_polyLinePaintAction = new KisRecordedPolyLinePaintAction(i18n("Freehand tool"),
                                                                   KisNodeQueryPath::absolutePath(currentNode()),
                                                                   currentPaintOpPreset(),
                                                                   m_painter->paintColor(),
                                                                   m_painter->backgroundColor(),
                                                                   m_painter->opacity(),
                                                                   m_paintIncremental,
                                                                   m_compositeOp);
    }
#endif
}

void KisToolFreehand::endPaint()
{
    //    dbgUI << "endPaint";
    m_mode = HOVER;
    if (m_painter) {

        m_executor->waitForDone();
        // If painting in mouse release, make sure painter
        // is destructed or end()ed

        // XXX: For now, only layers can be painted on in non-incremental mode
        KisLayerSP layer = dynamic_cast<KisLayer*>(currentNode().data());

        if (layer && !m_paintIncremental) {
            KisTransaction *incrementalTransaction =
                    dynamic_cast<KisTransaction*>(m_painter->endTransaction());

            KisPainter painter(m_source, currentSelection());
            painter.setCompositeOp(m_compositeOp);
            painter.setOpacity(m_opacity);

            if (KisPaintLayer* l = dynamic_cast<KisPaintLayer*>(currentNode().data())) {
                painter.setChannelFlags(l->channelFlags());
                if (l->alphaLocked()) {
                    painter.setLockAlpha(l->alphaLocked());
                }
            }

            painter.beginTransaction(m_transactionText);

            QRegion r = m_incrementalDirtyRegion;
            foreach(const QRect& rc, r.rects()) {
                painter.bitBlt(rc.topLeft(), m_target, rc);
            }
            KisIndirectPaintingSupport* indirect = dynamic_cast<KisIndirectPaintingSupport*>(layer.data());
            if (indirect) {
                indirect->setTemporaryTarget(0);
            }
            m_source->setDirty(painter.dirtyRegion());
            delete incrementalTransaction;

            m_canvas->addCommand(painter.endTransaction());
        } else {
            m_canvas->addCommand(m_painter->endTransaction());
        }
    }
    delete m_painter;
    m_painter = 0;
    m_target = 0 ;
    notifyModified();

    if (!m_paintJobs.empty()) {
        foreach(FreehandPaintJob* job , m_paintJobs) {
            delete job;
        }
        m_paintJobs.clear();
    }
#ifdef ENABLE_RECORDING
    if (m_smooth) {
        if (image())
            image()->actionRecorder()->addAction(*m_bezierCurvePaintAction);
        delete m_bezierCurvePaintAction;
        m_bezierCurvePaintAction = 0;
    } else {
        if (image())
            image()->actionRecorder()->addAction(*m_polyLinePaintAction);
        delete m_polyLinePaintAction;
        m_polyLinePaintAction = 0;
    }
#endif
}

void KisToolFreehand::paintAt(const KisPaintInformation &pi)
{
    FreehandPaintJob* previousJob = m_paintJobs.empty() ? 0 : m_paintJobs.last();
    queuePaintJob(new FreehandPaintAtJob(this, m_painter, pi, previousJob), previousJob);
}

void KisToolFreehand::paintLine(const KisPaintInformation &pi1,
                                const KisPaintInformation &pi2)
{
    FreehandPaintJob* previousJob = m_paintJobs.empty() ? 0 : m_paintJobs.last();
    queuePaintJob(new FreehandPaintLineJob(this, m_painter, pi1, pi2, previousJob), previousJob);
}

void KisToolFreehand::paintBezierCurve(const KisPaintInformation &pi1,
                                       const QPointF &control1,
                                       const QPointF &control2,
                                       const KisPaintInformation &pi2)
{
    FreehandPaintJob* previousJob = m_paintJobs.empty() ? 0 : m_paintJobs.last();
    queuePaintJob(new FreehandPaintBezierJob(this, m_painter, pi1, control1, control2, pi2, previousJob), previousJob);
}

void KisToolFreehand::queuePaintJob(FreehandPaintJob* job, FreehandPaintJob* /*previousJob*/)
{
    m_paintJobs.append(job);
    //    dbgUI << "Queue length:" << m_executor->queueLength();
    m_executor->start(job, -m_paintJobs.size());
}

bool KisToolFreehand::wantsAutoScroll()
{
    if (m_mode == PAN ) {
        return false;
    }
    return true;
}

void KisToolFreehand::setDirty(const QRegion& region)
{
    if (region.numRects() < 1)
        return;

    if (!m_paintOnSelection) {
        currentNode()->setDirty(region);
    } else {
        QRect r = region.boundingRect();
        r = QRect(r.left() - 1, r.top() - 1, r.width() + 2, r.height() + 2); //needed to update selectionvisualization
        m_target->setDirty(r);
    }
    if (!m_paintIncremental) {
        m_incrementalDirtyRegion += region;
    }
}

void KisToolFreehand::setSmooth(bool smooth)
{
    m_smooth = smooth;
}

void KisToolFreehand::setAssistant(bool assistant)
{
    m_assistant = assistant;
}

#define ZET 7
void KisToolFreehand::paint(QPainter& gc, const KoViewConverter &converter)
{
    KisConfig cfg;
#if defined(HAVE_OPENGL)
    if (m_canvas->canvasController()->isCanvasOpenGL()) {
        if (cfg.cursorStyle() == CURSOR_STYLE_3D_MODEL) {
            qreal sx, sy;
            converter.zoom(&sx, &sy);
            sx /= currentImage()->xRes();
            sy /= currentImage()->yRes();

            // check if the paintop has been changed
            // TODO: maybe find a better way -- signal from paintop to ui/freehand that paintop has been changed
            if (m_brushModelName.compare(currentPaintOpPreset()->settings()->modelName()) != 0) {
                glDeleteLists(m_displayList, 1);
                m_displayList = 0;
            }

            if (glIsList(m_displayList)) {
                QPointF pos = converter.documentToView(mousePos);

                glColor3f(0.0, 1.0, 0.0);
                glShadeModel(GL_SMOOTH);

                glEnable(GL_DEPTH_TEST);
                glClear(GL_DEPTH_BUFFER_BIT);


                glEnable(GL_LINE_SMOOTH);
                glEnable(GL_COLOR_MATERIAL);

                glPushMatrix();
                glTranslatef(pos.x(), pos.y(), 0.0);
                glScalef(sx, sy, 1);
                glRotated(90.0, 1.0, 0.0, 0.0);
                glRotated(-(m_xTilt*0.5 + m_prevxTilt*0.5) , 0.0, 0.0, 1.0);
                glRotated(-(m_yTilt*0.5 + m_prevyTilt*0.5) , 1.0, 0.0, 0.0);

                glCallList(m_displayList);
                glScalef(1.0 / sx, 1.0 / sy , 1);
                glPopMatrix();

                glDisable(GL_DEPTH_TEST);
                glDisable(GL_LINE_SMOOTH);
                glDisable(GL_COLOR_MATERIAL);

                m_prevxTilt = m_xTilt;
                m_prevyTilt = m_yTilt;

            } else {
                dbgUI << "Warning: I don't have list to draw!";
                dbgUI << "Default model will be used";
                Kis3DObjectModel * model;
                m_brushModelName = currentPaintOpPreset()->settings()->modelName();

                // here is the default 3d model filename for brushes
                if (m_brushModelName.isEmpty()) {
                    model = new Kis3DObjectModel("3d-deform-brush.obj" , "3d-deform-brush.mtl");
                } else {
                    model = new Kis3DObjectModel(m_brushModelName + ".obj" , m_brushModelName + ".mtl");
                }
                m_displayList = model->displayList();
                if (!glIsList(m_displayList)){
                    dbgUI << "Default model has not been found!";
                }
                delete model;
            }
        }
    }
#endif

    {
        KisPaintOpSettings::OutlineMode outlineMode;
        if (m_mode != PAINT && (cfg.cursorStyle() == CURSOR_STYLE_OUTLINE || m_mode == EDIT_BRUSH)) {
            outlineMode = KisPaintOpSettings::CURSOR_IS_OUTLINE;
        } else {
            outlineMode = KisPaintOpSettings::CURSOR_ISNT_OUTLINE;
        }
        currentPaintOpPreset()->settings()->paintOutline(mousePos, currentImage(), gc, converter, outlineMode);

    }
}

QPointF KisToolFreehand::adjustPosition(const QPointF& point)
{
    if (m_assistant) {
        QPointF ap = static_cast<KisCanvas2*>(canvas())->view()->paintingAssistantManager()->adjustPosition(point);
        return (1.0 - m_magnetism) * point + m_magnetism * ap;
    }
    return point;
}

void KisToolFreehand::initPan(KoPointerEvent *event)
{
    m_mode = PAN;
    m_lastPosition = documentToViewport(event->point);
    event->accept();
    useCursor(QCursor(Qt::ClosedHandCursor), true);
}

void KisToolFreehand::pan(KoPointerEvent *event)
{
    Q_ASSERT(canvas());
    Q_ASSERT(canvas()->canvasController());


    if (event->buttons() == 0)
        return;
    event->accept();

    QPointF actualPosition = documentToViewport(event->point);
    QPointF distance(m_lastPosition - actualPosition);
    canvas()->canvasController()->pan(distance.toPoint());

    m_lastPosition = actualPosition;
}

void KisToolFreehand::endPan()
{
    m_mode = HOVER;
    resetCursorStyle();
}

void KisToolFreehand::customMoveEvent(KoPointerEvent * event)
{
    if (m_mode == PAN) {
        canvas()->canvasController()->pan(QPoint(-event->x(), -event->y()));
        event->accept();
    }
    event->ignore();
}


QPointF KisToolFreehand::documentToViewport(const QPointF &p)
{
    QPointF viewportPoint = canvas()->viewConverter()->documentToView(p);
    viewportPoint += canvas()->documentOrigin();
    viewportPoint += QPoint(canvas()->canvasController()->canvasOffsetX(),
                            canvas()->canvasController()->canvasOffsetY());
    return viewportPoint;
}


#include "kis_tool_freehand.moc"

