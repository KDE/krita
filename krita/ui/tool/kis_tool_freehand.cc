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
#include <KoViewConverter.h>
#include <kis_view2.h>
#include <kis_painting_assistants_manager.h>

// OpenGL 
#include <config-opengl.h>

#ifdef HAVE_OPENGL
#include <GL/gl.h>
#include <GL/glu.h>
#endif


#define ENABLE_RECORDING
#include <KoCanvasController.h>
#include <GL/glu.h>

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
}

KisToolFreehand::~KisToolFreehand()
{
}

void KisToolFreehand::mousePressEvent(KoPointerEvent *e)
{
//    dbgUI << "mousePressEvent" << m_mode;
//     if (!currentImage())
//    return;

    if (!currentNode())
        return;

    if (!currentNode()->paintDevice())
        return;

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
    if (m_mode == PAINT) {
        QPointF pos = convertToPixelCoord(e);

        KisPaintInformation info = KisPaintInformation(convertToPixelCoord(adjustPosition(e->point)),
                                   e->pressure(), e->xTilt(), e->yTilt(),
                                   KisVector2D::Zero(),
                                   e->rotation(), e->tangentialPressure());

        if (m_smooth) {
            QPointF dragVec = info.pos() - m_previousPaintInformation.pos();
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
    KisConfig cfg;
    KisPaintOpSettings::OutlineMode outlineMode;
    if (m_mode != PAINT && cfg.cursorStyle() == CURSOR_STYLE_OUTLINE) {
      outlineMode = KisPaintOpSettings::CURSOR_IS_OUTLINE;
    } else {
      outlineMode = KisPaintOpSettings::CURSOR_ISNT_OUTLINE;
    }

    if(!oldOutlineRect.isEmpty()) {
        m_canvas->updateCanvas(oldOutlineRect); // erase the old guy
    }

    mousePos = e->point;

#if defined(HAVE_OPENGL)
    if (cfg.cursorStyle() == CURSOR_STYLE_3D_MODEL){
        if (m_canvas->canvasController()->isCanvasOpenGL()){
            // TODO rethink this as I need to update whole canvas
            m_canvas->updateCanvas(     QRect( QPoint(0,0),QSize(320,240) ) );
        }
    }
#endif

    oldOutlineRect = currentPaintOpPreset()->settings()->paintOutlineRect(mousePos, currentImage(), outlineMode);
    if(!oldOutlineRect.isEmpty()) {
        m_canvas->updateCanvas(oldOutlineRect); // erase the old guy
    }
}

void KisToolFreehand::mouseReleaseEvent(KoPointerEvent* e)
{
//    dbgUI << "mouseReleaseEvent" << m_mode << " " << e->button() << " " << e->button();
    if (e->button() == Qt::LeftButton && m_mode == PAINT) {
        endPaint();
    } else {
        KisToolPaint::mouseReleaseEvent(e);
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

#if 0 //XXX: Warning, investigate what this was supposed to do!
            if (l->parentLayer() && (l->parentLayer()->parentLayer() == 0)
                    && (l->parentLayer()->childCount() == 1)
                    && l->parentLayer()->paintLayerInducesProjectionOptimization(pl)) {
                // If there's a mask, device could've been the mask. The induce function
                // should catch this, but better safe than sorry

                // XXX: What does this and why? (BSAR)
                l->parentLayer()->resetProjection(pl->paintDevice());
            }
#endif
            m_target = new KisPaintDevice(currentNode().data(),
                                          device->colorSpace());
            layer->setTemporaryTarget(m_target);
            layer->setTemporaryCompositeOp(m_compositeOp);
            layer->setTemporaryOpacity(m_opacity);
        }
    }
    if( !m_target ) {
        m_target = device;
    }
    m_painter = new KisPainter(m_target, currentSelection());
    Q_CHECK_PTR(m_painter);
    m_source = device;
    m_painter->beginTransaction(m_transactionText);

    setupPainter(m_painter);

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

            painter.beginTransaction(m_transactionText);

            QRegion r = m_incrementalDirtyRegion;
            QVector<QRect> dirtyRects = r.rects();
            QVector<QRect>::iterator it = dirtyRects.begin();
            QVector<QRect>::iterator end = dirtyRects.end();

            painter.setCompositeOp(m_compositeOp);
            painter.setOpacity(m_opacity);
            
            while (it != end) {
                
                painter.bitBlt(it->x(), it->y(), 
                               m_target,
                               it->x(), it->y(),
                               it->width(), it->height());
                ++it;
            }
            KisIndirectPaintingSupport* indirect =
                dynamic_cast<KisIndirectPaintingSupport*>(layer.data());
            if (indirect)
                indirect->setTemporaryTarget(0);
            //m_source->setDirty(painter.dirtyRegion());
	    
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

#define ZET 10
void KisToolFreehand::paint(QPainter& gc, const KoViewConverter &converter)
{
    KisConfig cfg;
#if defined(HAVE_OPENGL)
    if (m_canvas->canvasController()->isCanvasOpenGL()){
        if (cfg.cursorStyle() == CURSOR_STYLE_3D_MODEL){
            qreal sx, sy;
            converter.zoom(&sx, &sy);
            sx /= currentImage()->xRes();
            sy /= currentImage()->yRes();

            GLuint list  =  currentPaintOpPreset()->settings()->displayList();
            if (glIsList( list )){
                kDebug() << "I have list to draw!";
                QPointF pos = converter.documentToView( mousePos );
/*                glEnable(GL_LINE_SMOOTH);
                glEnable(GL_COLOR_LOGIC_OP);

                glLogicOp(GL_XOR);*/

                //glColor3f(0.501961,1.0, 0.501961);

/*        glDepthFunc(GL_LESS);
        glShadeModel(GL_SMOOTH);

        glEnable(GL_DEPTH_TEST);
        glEnable(GL_LIGHTING);
        glEnable(GL_COLOR_MATERIAL);

        glClear(GL_DEPTH_BUFFER_BIT);
        
        KisImageSP img = currentImage();
        glEnable(GL_LIGHT0);
        glEnable(GL_LIGHT1);
        glEnable(GL_LIGHT2);
        glEnable(GL_LIGHT3);
        glEnable(GL_LIGHT4);

        QPointF pos0(0,0); pos0 = converter.documentToView( pos0 );
        QPointF pos1(0,img->height()); pos1 = converter.documentToView( pos1 );
        QPointF pos2(img->width(),img->height()); pos2 = converter.documentToView( pos2 );
        QPointF pos3(img->width(),0); pos3 = converter.documentToView( pos3 );

        GLfloat position[] = { 0.0f, 0.0f, ZET };
        position[0] = pos.x();
        position[1] = pos.y();
        glLightfv(GL_LIGHT0, GL_POSITION, position);
        position[0] = pos1.x();
        position[1] = pos1.y();
        glLightfv(GL_LIGHT1, GL_POSITION, position);
        position[0] = pos2.x();
        position[1] = pos2.y();
        glLightfv(GL_LIGHT2, GL_POSITION, position);
        position[0] = pos3.x();
        position[1] = pos3.y();
        glLightfv(GL_LIGHT3, GL_POSITION, position);*/
        
            glColor3f(0.0,1.0,0.0);
            glEnable(GL_COLOR_MATERIAL);
            glDepthFunc(GL_LESS);
            glShadeModel(GL_SMOOTH);
            //glEnable(GL_DEPTH_TEST);
            glEnable(GL_LINE_SMOOTH);

                glPushMatrix();
                        glTranslatef( pos.x(), pos.y(),3 );
                        glScalef( sx,sy,1);
                        glRotatef(90, 0.8,0.7,0.6);
                            glCallList( list );
                        glScalef(1.0 / sx,1.0 / sy ,1);
                glPopMatrix();
            }else{
                kDebug() << "_No_ list to draw!";
// JUST TEST HERE 
                glDepthFunc(GL_LESS);
                glEnable(GL_DEPTH_TEST);
                glClear(GL_DEPTH_BUFFER_BIT);

                static int angle = 0;

                QPointF pos1 = converter.documentToView( mousePos );
                glPushMatrix();
                    angle++;
                    glTranslatef( pos1.x(), pos1.y(),0 );
                    glRotated(angle,0.56,0.31,0.2);
                    glScalef(100,100,100);
                
                    glBegin(GL_QUADS);
                    glColor3f(0.0,1.0,0.0);

                    //bottom
                    glVertex3f(0,0,0);
                    glVertex3f(1,0,0);
                    glVertex3f(1,1,0);
                    glVertex3f(0,1,0);
    
                    //top
                    glVertex3f(0,0,1);
                    glVertex3f(1,0,1);
                    glVertex3f(1,1,1);
                    glVertex3f(0,1,1);

                    glColor3f(1.0,0.0,0.0);
                    //left
                    glVertex3f(0,0,0);
                    glVertex3f(0,0,1);
                    glVertex3f(0,1,1);
                    glVertex3f(0,1,0);

                    // right
                    glVertex3f(1,0,0);
                    glVertex3f(1,0,1);
                    glVertex3f(1,1,1);
                    glVertex3f(1,1,0);

                    glColor3f(0.0,0.0,1.0);                    
                    // front
                    glVertex3f(0,0,0);
                    glVertex3f(0,0,1);
                    glVertex3f(1,0,1);
                    glVertex3f(1,0,0);
                
                    // back
                    glVertex3f(0,1,0);
                    glVertex3f(0,1,1);
                    glVertex3f(1,1,1);
                    glVertex3f(1,1,0);

                    glEnd();

                glPopMatrix();
        // END TEST HERE

            }// else
        }
    }
#endif

    {
        KisPaintOpSettings::OutlineMode outlineMode;
        if (m_mode != PAINT && cfg.cursorStyle() == CURSOR_STYLE_OUTLINE) {
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


#include "kis_tool_freehand.moc"

