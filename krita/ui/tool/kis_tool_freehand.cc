/*
 *  kis_tool_freehand.cc - part of Krita
 *
 *  Copyright (c) 2003-2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2007,2008,2010 Cyrille Berger <cberger@cberger.net>
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

#include <klocale.h>
#include <kaction.h>

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoResourceManager.h>
#include <KoCanvasController.h>
#include <KoViewConverter.h>

//pop up palette
#include <ko_favorite_resource_manager.h>
#include <kis_canvas_resource_provider.h>

// Krita/image
#include <recorder/kis_action_recorder.h>
#include <kis_fill_painter.h>
#include <kis_group_layer.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <recorder/kis_recorded_path_paint_action.h>
#include <kis_selection.h>
#include <kis_paintop_preset.h>
#include <kis_debug.h>
#include <kis_transaction.h>

// Krita/ui
#include "kis_config.h"
#include <opengl/kis_opengl.h>
#include "canvas/kis_canvas2.h"
#include "kis_cursor.h"
#include "kis_painting_assistant.h"
#include <recorder/kis_node_query_path.h>
#include <kis_view2.h>
#include <kis_painting_assistants_manager.h>
#include <kis_3d_object_model.h>
#include "kis_color_picker_utils.h"

#define ENABLE_RECORDING
static const int HIDE_OUTLINE_TIMEOUT = 800; // ms

KisToolFreehand::KisToolFreehand(KoCanvasBase * canvas, const QCursor & cursor, const QString & transactionText)
    : KisToolPaint(canvas, cursor)
    , m_dragDist(0)
    , m_transactionText(transactionText)
    , m_mode(HOVER)
{
    m_painter = 0;
    m_paintIncremental = false;
    m_executor = new QThreadPool(this);
    m_executor->setMaxThreadCount(1);
    m_smooth = true;
    m_assistant = false;
    m_smoothness = 0.5;
    m_magnetism = 1.0;
    m_pathPaintAction = 0;

    setSupportOutline(true);

#if defined(HAVE_OPENGL)
    m_xTilt = 0.0;
    m_yTilt = 0.0;
    m_prevxTilt = 0.0;
    m_prevyTilt = 0.0;
#endif

    m_increaseBrushSize = new KAction(i18n("Increase Brush Size"), this);
    m_increaseBrushSize->setShortcut(Qt::Key_Period);
    connect(m_increaseBrushSize, SIGNAL(activated()), SLOT(increaseBrushSize()));
    addAction("increase_brush_size", m_increaseBrushSize);

    m_decreaseBrushSize = new KAction(i18n("Decrease Brush Size"), this);
    m_decreaseBrushSize->setShortcut(Qt::Key_Comma);
    connect(m_decreaseBrushSize, SIGNAL(activated()), SLOT(decreaseBrushSize()));
    addAction("decrease_brush_size", m_decreaseBrushSize);

    KisCanvas2* canvas2 = static_cast<KisCanvas2*>(canvas);
    connect(this, SIGNAL(sigFavoritePaletteCalled(const QPoint&)), canvas2, SIGNAL(favoritePaletteCalled(const QPoint&)));
    connect(this, SIGNAL(sigPainting()), canvas2->view()->resourceProvider(), SLOT(slotPainting()));

    m_showOutline = false;
    m_timer.setSingleShot(true);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT( hideOutline() ));

}

KisToolFreehand::~KisToolFreehand()
{
    delete m_painter;
}

void KisToolFreehand::deactivate()
{
    endPaint();
    KisToolPaint::deactivate();
}

void KisToolFreehand::mousePressEvent(KoPointerEvent *e)
{
    if (currentPaintOpPreset() && currentPaintOpPreset()->settings()) {
        m_paintIncremental = currentPaintOpPreset()->settings()->paintIncremental();
        currentPaintOpPreset()->settings()->mousePressEvent(e);
        if (e->isAccepted()) {
            return;
        }
    }

    KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2 *>(canvas());
    if (canvas2->handlePopupPaletteIsVisible(e)) return;

    m_strokeTimeMeasure.start();

    //    dbgUI << "mousePressEvent" << m_mode;
    //     if (!currentImage())
    //    return;
    if (m_mode == PAN) {
        initPan(e);
        e->accept();
        return;
    }

    if (!currentNode())
        return;

    if (!currentNode()->paintDevice())
        return;

    // control-click gets the color at the current point. For now, only with a ratio of 1
    // get the current color of the project, as per Deevad's suggestion.
    // this shouldn't be changed back to the current node :-)
    // CTRL+ALT picks color from current layer
    // CTRL picks color from projection
    if (e->modifiers() & Qt::ControlModifier) {
        
        m_toForegroundColor = (e->button() == Qt::LeftButton);
        pickColor(convertToIntPixelCoord(e),e->modifiers() & Qt::AltModifier);
        m_mode = COLOR_PICKING;
        useCursor(KisCursor::pickerCursor());
        e->accept();
        
   } else if (e->modifiers() == Qt::ShiftModifier) {
        m_mode = EDIT_BRUSH;
        m_prevMousePos = e->point;
        m_originalPos = e->globalPos();
        m_originalMousePos = e->point;
        e->accept();
        return;
    } else { // No modifiers

        if (e->button() == Qt::LeftButton) {
            initPaint(e);
            m_previousPaintInformation = KisPaintInformation(convertToPixelCoord(adjustPosition(e->point)),
                                                             pressureToCurve(e->pressure()), e->xTilt(), e->yTilt(),
                                                             KisVector2D::Zero(),
                                                             e->rotation(), e->tangentialPressure(), m_strokeTimeMeasure.elapsed());
        } else if (m_mode == PAINT && (e->button() == Qt::RightButton || e->button() == Qt::MidButton)) {
            // end painting, if calling the menu or the pop up palette. otherwise there is weird behaviour
            endPaint();
        }
    }
    e->accept();
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
    case PAINT: {
            QPointF pos = convertToPixelCoord(adjustPosition(e->point));
            QPointF dragVec = pos - m_previousPaintInformation.pos();

            KisPaintInformation info = KisPaintInformation(pos,
                                                           pressureToCurve(e->pressure()), e->xTilt(), e->yTilt(),
                                                           toKisVector2D(dragVec),
                                                           e->rotation(), e->tangentialPressure(), m_strokeTimeMeasure.elapsed());

            if (m_smooth) {
                QPointF newTangent = dragVec;

                if (norm(newTangent) != 0) {
                    newTangent /= norm(newTangent) ;
                }
                double normVec = 0.5 * m_smoothness * norm(dragVec);
                QPointF control1 = m_previousPaintInformation.pos() + m_previousTangent * normVec;
                QPointF control2 = info.pos() - newTangent * normVec;
                paintBezierCurve(m_previousPaintInformation,
                                 control1,
                                 control2,
                                 info);
                m_previousTangent = newTangent;
                m_previousDrag = dragVec;
            } else {
                paintLine(m_previousPaintInformation, info);
            }

            m_previousPaintInformation = info;
        }
        break;
    case EDIT_BRUSH: {
            useCursor(KisCursor::blankCursor());
            qreal dx = m_prevMousePos.x() - e->point.x();
            qreal dy = m_prevMousePos.y() - e->point.y();
            currentPaintOpPreset()->settings()->changePaintOpSize(-dx, -dy);
            m_prevMousePos = e->point;
        }
        break;
    case PAN: {
            pan(e);
        }
        break;
    case COLOR_PICKING:{
            useCursor(KisCursor::pickerCursor());
            if (e->modifiers() & Qt::ControlModifier) {
                pickColor(convertToIntPixelCoord(e), e->modifiers() & Qt::AltModifier);
            }else {
                // DO Nothing
            }
            
        }
        break;
    default:
        ;
    }

    KisConfig cfg;
    KisPaintOpSettings::OutlineMode outlineMode;
    if ((m_mode != PAINT  || cfg.showOutlineWhilePainting()) &&
        (cfg.cursorStyle() == CURSOR_STYLE_OUTLINE || m_mode == EDIT_BRUSH || m_showOutline)) {
        outlineMode = KisPaintOpSettings::CursorIsOutline;
    } else {
        outlineMode = KisPaintOpSettings::CursorIsNotOutline;
    }

    if (!m_oldOutlineRect.isEmpty()) {
        canvas()->updateCanvas(m_oldOutlineRect.adjusted(-2,-2,2,2)); // erase the old guy
    }

    m_mousePos = e->point;

#if defined(HAVE_OPENGL)
    if (cfg.cursorStyle() == CURSOR_STYLE_3D_MODEL) {
        if (isCanvasOpenGL()) {
            m_xTilt = e->xTilt();
            m_yTilt = e->yTilt();
            // TODO : optimize? but you need to know the size of the 3d brush?
            canvas()->updateCanvas(QRect(QPoint(0, 0), QSize(currentImage()->width(), currentImage()->height())));
        }
    }
#endif

    
    QPainterPath path = currentPaintOpPreset()->settings()->brushOutline(currentImage()->documentToPixel(outlinePos()), outlineMode);
    m_oldOutlineRect = currentImage()->pixelToDocument(path.boundingRect());
    if (!m_oldOutlineRect.isEmpty()) {
        canvas()->updateCanvas(m_oldOutlineRect.adjusted(-2,-2,2,2)); 
    }

}

void KisToolFreehand::mouseReleaseEvent(KoPointerEvent* e)
{
    KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2 *>(canvas());
    if (canvas2->handlePopupPaletteIsVisible(e)) {
        return;
    }
    else if (e->button() == Qt::LeftButton) {
        //TODO: There is a bug here. If pop up palette is visible and a new colour is selected,
        //the new colour will be added when the user clicks on the canvas to hide the palette
        //This is used to handle recently used colour (KoFavoriteResourceManager)
        emit sigPainting();
    }
    
    switch (m_mode) {
    case PAINT:
        if (!m_hasPaintAtLeastOnce)
        {
            paintAt(m_previousPaintInformation);
        }
        endPaint();
        break;
    case PAN:
        return;
    case EDIT_BRUSH:
        QCursor::setPos(m_originalPos);
        break;
    default:
        ;
    };
    
    if (m_mode != COLOR_PICKING){ 
        KisToolPaint::mouseReleaseEvent(e);
    }
    
    m_mode = HOVER;
    resetCursorStyle();
}

void KisToolFreehand::keyPressEvent(QKeyEvent *event)
{
    if (m_mode != HOVER)
    {
        event->accept(); // Make sure nothing disturb the painting
    } else if (event->key() == Qt::Key_Space) {
        m_mode = PAN;
        useCursor(Qt::OpenHandCursor);

        event->accept();
    }/* else if (event->key() == Qt::Key_Control){ // we need to reset the cursor back when the user does not hold any key so commented so far
        useCursor(KisCursor::pickerCursor());
    }*/else {
        event->ignore();
    }
}

void KisToolFreehand::keyReleaseEvent(QKeyEvent* event)
{
    if (m_mode == PAN && event->key() == Qt::Key_Space) {
        endPan();
        event->accept();
    }
    KoToolBase::keyReleaseEvent(event);
}

void KisToolFreehand::initPaint(KoPointerEvent *)
{
    //    dbgUI << "initPaint";
    if (!currentNode() || !currentNode()->paintDevice())
        return;

    m_mode = PAINT;
    m_hasPaintAtLeastOnce = false;
    m_dragDist = 0;

    KisPaintDeviceSP paintDevice = currentNode()->paintDevice();
    KisPaintDeviceSP targetDevice;

    if (!m_compositeOp)
        m_compositeOp = paintDevice->colorSpace()->compositeOp(COMPOSITE_OVER);


    if (!m_paintIncremental) {
        KisIndirectPaintingSupport* indirect =
            dynamic_cast<KisIndirectPaintingSupport*>(currentNode().data());

        if (indirect) {
            targetDevice = new KisPaintDevice(currentNode().data(), paintDevice->colorSpace());
            indirect->setTemporaryTarget(targetDevice);
            indirect->setTemporaryCompositeOp(m_compositeOp);
            indirect->setTemporaryOpacity(m_opacity);
        }
        else {
            m_paintIncremental = true;
        }
    }

    if (!targetDevice)
        targetDevice = paintDevice;


    if (m_painter)
        delete m_painter;

    m_painter = new KisPainter(targetDevice, currentSelection());
    m_painter->beginTransaction(m_transactionText);

    setupPainter(m_painter);

    if (m_paintIncremental) {
        m_painter->setCompositeOp(m_compositeOp);
        m_painter->setOpacity(m_opacity);
    } else {
        m_painter->setCompositeOp(paintDevice->colorSpace()->compositeOp(COMPOSITE_ALPHA_DARKEN));
        m_painter->setOpacity(OPACITY_OPAQUE_U8);
    }

    m_previousTangent = QPointF(0, 0);
    m_previousDrag = QPointF(0, 0);


#ifdef ENABLE_RECORDING // Temporary, to figure out what is going without being
    // distracted by the recording
    m_pathPaintAction = new KisRecordedPathPaintAction(
            KisNodeQueryPath::absolutePath(currentNode()),
            currentPaintOpPreset()
            );
    m_pathPaintAction->setPaintIncremental(m_paintIncremental);
    setupPaintAction(m_pathPaintAction);
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
            m_painter->deleteTransaction();

            KisIndirectPaintingSupport *indirect =
                dynamic_cast<KisIndirectPaintingSupport*>(layer.data());
            Q_ASSERT(indirect);

            indirect->mergeToLayer(layer, m_incrementalDirtyRegion, m_transactionText);

            m_incrementalDirtyRegion = QRegion();
        } else {
            m_painter->endTransaction(image()->undoAdapter());
        }
    }
    delete m_painter;
    m_painter = 0;
    notifyModified();

    if (!m_paintJobs.empty()) {
        foreach(FreehandPaintJob* job , m_paintJobs) {
            delete job;
        }
        m_paintJobs.clear();
    }
#ifdef ENABLE_RECORDING
    if (image() && m_pathPaintAction)
        image()->actionRecorder()->addAction(*m_pathPaintAction);
    delete m_pathPaintAction;
    m_pathPaintAction = 0;
#endif
}

void KisToolFreehand::paintAt(const KisPaintInformation &pi)
{
    m_hasPaintAtLeastOnce = true;
    FreehandPaintJob* previousJob = m_paintJobs.empty() ? 0 : m_paintJobs.last();
    queuePaintJob(new FreehandPaintAtJob(this, m_painter, pi, previousJob), previousJob);
    m_pathPaintAction->addPoint(pi);
}

void KisToolFreehand::paintLine(const KisPaintInformation &pi1,
                                const KisPaintInformation &pi2)
{
    m_hasPaintAtLeastOnce = true;
    FreehandPaintJob* previousJob = m_paintJobs.empty() ? 0 : m_paintJobs.last();
    queuePaintJob(new FreehandPaintLineJob(this, m_painter, pi1, pi2, previousJob), previousJob);
    m_pathPaintAction->addLine(pi1, pi2);
}

void KisToolFreehand::paintBezierCurve(const KisPaintInformation &pi1,
                                       const QPointF &control1,
                                       const QPointF &control2,
                                       const KisPaintInformation &pi2)
{
    m_hasPaintAtLeastOnce = true;
    FreehandPaintJob* previousJob = m_paintJobs.empty() ? 0 : m_paintJobs.last();
    queuePaintJob(new FreehandPaintBezierJob(this, m_painter, pi1, control1, control2, pi2, previousJob), previousJob);
    m_pathPaintAction->addCurve(pi1, control1, control2, pi2);
}

void KisToolFreehand::queuePaintJob(FreehandPaintJob* job, FreehandPaintJob* /*previousJob*/)
{
    m_paintJobs.append(job);
    //    dbgUI << "Queue length:" << m_executor->queueLength();
    m_executor->start(job, -m_paintJobs.size());
}

bool KisToolFreehand::wantsAutoScroll() const
{
    return false;
}

void KisToolFreehand::setDirty(const QRegion& region)
{
    if (region.numRects() < 1)
        return;

    currentNode()->setDirty(region);

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
    if (isCanvasOpenGL()) {
        if (cfg.cursorStyle() == CURSOR_STYLE_3D_MODEL) {
            beginOpenGL();

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
                QPointF pos = converter.documentToView(m_mousePos);

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
                if (!glIsList(m_displayList)) {
                    dbgUI << "Default model has not been found!";
                }
                delete model;
            }
            endOpenGL();
        }
    }
#endif

    {
        KisPaintOpSettings::OutlineMode outlineMode;
        if (m_mode == PAINT) {
            m_showOutline = false;
        }
        
        if ((m_mode != PAINT || cfg.showOutlineWhilePainting()) &&
            (cfg.cursorStyle() == CURSOR_STYLE_OUTLINE || m_mode == EDIT_BRUSH || m_showOutline)) {
            outlineMode = KisPaintOpSettings::CursorIsOutline;
        } else {
            outlineMode = KisPaintOpSettings::CursorIsNotOutline;
        }
        
        qreal zoomX, zoomY;
        converter.zoom(&zoomX, &zoomY);
        
        
        QPainterPath path = currentPaintOpPreset()->settings()->brushOutline(currentImage()->documentToPixel(outlinePos()),outlineMode);
        m_oldOutlineRect = currentImage()->pixelToDocument(path.boundingRect());
        
/*        QTransform m;
        m.reset();
        // document to view
        m.scale(zoomX, zoomY);
        // translate according outlinePos in document coordinates
        // pixel to document
        m.scale(1.0/currentImage()->xRes(),1.0/currentImage()->yRes());
        
        m.map(path);*/
        
        paintToolOutline(&gc,pixelToView(path));
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
    m_lastPosition = convertDocumentToWidget(event->point);
    event->accept();
    useCursor(QCursor(Qt::ClosedHandCursor));
}

void KisToolFreehand::pan(KoPointerEvent *event)
{
    Q_ASSERT(canvas());
    Q_ASSERT(canvas()->canvasController());


    if (event->buttons() == 0)
        return;
    event->accept();

    QPointF actualPosition = convertDocumentToWidget(event->point);
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

void KisToolFreehand::increaseBrushSize()
{
    currentPaintOpPreset()->settings()->changePaintOpSize(1, 0);
    m_oldOutlineRect.adjust(-1, -1, 1, 1);
    showOutlineTemporary();
}

void KisToolFreehand::decreaseBrushSize()
{
    currentPaintOpPreset()->settings()->changePaintOpSize(-1, 0);
    showOutlineTemporary();
}

QPointF KisToolFreehand::outlinePos() const
{
    if (m_mode == EDIT_BRUSH) {
        return m_originalMousePos;
    } else {
        return m_mousePos;
    }
}

void KisToolFreehand::showOutlineTemporary()
{
    m_showOutline = true;
    m_timer.start(HIDE_OUTLINE_TIMEOUT);
    //canvas()->updateCanvas(m_oldOutlineRect);
    canvas()->updateCanvas(QRect(QPoint(0, 0), QSize(currentImage()->width(), currentImage()->height())));
}

void KisToolFreehand::hideOutline()
{
    m_showOutline = false;
    //canvas()->updateCanvas(m_oldOutlineRect);
    canvas()->updateCanvas(QRect(QPoint(0, 0), QSize(currentImage()->width(), currentImage()->height())));
}


void KisToolFreehand::pickColor(const QPoint &pos, bool fromCurrentNode)
{
    int key = m_toForegroundColor ? KoCanvasResource::ForegroundColor : KoCanvasResource::BackgroundColor;
    if (fromCurrentNode){
            canvas()->resourceManager()->setResource(key,KisToolUtils::pick( currentNode()->paintDevice(),pos));
    } else /* projection */{
            canvas()->resourceManager()->setResource(key,KisToolUtils::pick( image()->projection(), pos));
    }
}


#include "kis_tool_freehand.moc"

