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

#include <QPainter>
#include <QRect>
#include <QThreadPool>

#include <kaction.h>

#include <KoPointerEvent.h>
#include <KoViewConverter.h>

//pop up palette
#include <kis_canvas_resource_provider.h>

// Krita/image
#include <recorder/kis_action_recorder.h>
#include <recorder/kis_recorded_path_paint_action.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include <kis_painter.h>
#include <kis_paintop.h>
#include <kis_selection.h>
#include <kis_paintop_preset.h>


// Krita/ui
#include "kis_abstract_perspective_grid.h"
#include "kis_config.h"
#include <opengl/kis_opengl.h>
#include "canvas/kis_canvas2.h"
#include "kis_cursor.h"
#include <recorder/kis_node_query_path.h>
#include <kis_view2.h>
#include <kis_painting_assistants_manager.h>
#include <kis_3d_object_model.h>

#define ENABLE_RECORDING
static const int HIDE_OUTLINE_TIMEOUT = 800; // ms

KisToolFreehand::KisToolFreehand(KoCanvasBase * canvas, const QCursor & cursor, const QString & transactionText)
    : KisToolPaint(canvas, cursor)
    , m_transactionText(transactionText)
{
    m_explicitShowOutline = false;


    m_painter = 0;
    m_paintIncremental = false;
    m_executor = new QThreadPool(this);
    m_executor->setMaxThreadCount(1);
    m_smooth = true;
    m_assistant = false;
    m_smoothness = 1.0;
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

    m_outlineTimer.setSingleShot(true);
    connect(&m_outlineTimer, SIGNAL(timeout()), this, SLOT(hideOutline()));

    m_strokeTimer.setSingleShot(true);
    connect(&m_strokeTimer, SIGNAL(timeout()), this, SLOT(finishStroke()));
}

KisToolFreehand::~KisToolFreehand()
{
    delete m_painter;
}

int KisToolFreehand::flags() const
{
    return KisTool::FLAG_USES_CUSTOM_COMPOSITEOP|KisTool::FLAG_USES_CUSTOM_PRESET;
}

void KisToolFreehand::deactivate()
{
    if(mode() == PAINT_MODE)
    {
        endPaint();
        setMode(KisTool::HOVER_MODE);
    }
    KisToolPaint::deactivate();
}

inline double norm(const QPointF& p)
{
    return sqrt(p.x()*p.x() + p.y()*p.y());
}

inline double angle(const QPointF& p1, const QPointF& p2)
{
    return atan2(p1.y(), p1.x()) - atan2(p2.y(), p2.x());
}

void KisToolFreehand::mousePressEvent(KoPointerEvent *e)
{
    if(mode() == KisTool::PAINT_MODE)
        return;

    m_outlineDocPoint = e->point;

    KisConfig cfg;
    if(cfg.cursorStyle() == CURSOR_STYLE_OUTLINE) {
        updateOutlineRect();
    }

    /**
     * FIXME: we need some better way to implement modifiers
     * for a paintop level
     */
    QPointF pos = adjustPosition(e->point, e->point);
    qreal perspective = 1.0;
    foreach (const KisAbstractPerspectiveGrid* grid, static_cast<KisCanvas2*>(canvas())->view()->resourceProvider()->perspectiveGrids()) {
        if (grid->contains(pos)) {
            perspective = grid->distance(pos);
            break;
        }
    }
    bool ignoreEvent = currentPaintOpPreset()->settings()->mousePressEvent(KisPaintInformation(convertToPixelCoord(e->point),
                                                         pressureToCurve(e->pressure()), e->xTilt(), e->yTilt(),
                                                         KisVector2D::Zero(),
                                                         e->rotation(), e->tangentialPressure(), perspective, m_strokeTimeMeasure.elapsed()),e->modifiers());
    if (!ignoreEvent){
        e->accept();
        return;
    }else{
        e->ignore();
    }


    if(mode() == KisTool::HOVER_MODE &&
       e->button() == Qt::LeftButton &&
       e->modifiers() == Qt::NoModifier &&
       !specialModifierActive()) {


        if (nodePaintAbility() != PAINT)
            return;

        setMode(KisTool::PAINT_MODE);

        initPaint(e);
        m_previousPaintInformation = KisPaintInformation(convertToPixelCoord(adjustPosition(e->point, e->point)),
                                                         pressureToCurve(e->pressure()), e->xTilt(), e->yTilt(),
                                                         KisVector2D::Zero(),
                                                         e->rotation(), e->tangentialPressure(), perspective, m_strokeTimeMeasure.elapsed());
        m_previousTangent = QPointF(0.0, 0.0);
        m_haveTangent = false;
        m_strokeBegin = e->point;

        e->accept();
    }
    else {
        KisToolPaint::mousePressEvent(e);
    }
}

void KisToolFreehand::mouseMoveEvent(KoPointerEvent *e)
{
    /**
     * Update outline
     */
    if(mode() == KisTool::HOVER_MODE ||
       mode() == KisTool::PAINT_MODE) {
        m_outlineDocPoint = e->point;

        KisConfig cfg;
        if(cfg.cursorStyle() == CURSOR_STYLE_OUTLINE) {
            updateOutlineRect();
        }

#if defined(HAVE_OPENGL)
        else if (cfg.cursorStyle() == CURSOR_STYLE_3D_MODEL) {
            if (isCanvasOpenGL()) {
                m_xTilt = e->xTilt();
                m_yTilt = e->yTilt();
                // TODO : optimize? but you need to know the size of the 3d brush?
                canvas()->updateCanvas(QRect(QPoint(0, 0), QSize(currentImage()->width(), currentImage()->height())));
            }
        }
#endif
    }

    if(mode() != KisTool::PAINT_MODE) {
        KisToolPaint::mouseMoveEvent(e);
        return;
    }

    /**
     * Actual painting
     */
    QPointF adjusted = adjustPosition(e->point, m_strokeBegin);
    QPointF pos = convertToPixelCoord(adjusted);
    QPointF dragVec = pos - m_previousPaintInformation.pos();

    qreal perspective = 1.0;
    foreach (const KisAbstractPerspectiveGrid* grid, static_cast<KisCanvas2*>(canvas())->view()->resourceProvider()->perspectiveGrids()) {
        if (grid->contains(adjusted)) {
            perspective = grid->distance(adjusted);
            break;
        }
    }

    KisPaintInformation info =
        KisPaintInformation(pos, pressureToCurve(e->pressure()),
                            e->xTilt(), e->yTilt(), toKisVector2D(dragVec),
                            e->rotation(), e->tangentialPressure(), perspective,
                            m_strokeTimeMeasure.elapsed());

    if (m_smooth) {
        if (!m_haveTangent) {
            m_haveTangent = true;
            m_previousTangent = (info.pos() - m_previousPaintInformation.pos()) * m_smoothness /
                                 (3.0 * (info.currentTime() - m_previousPaintInformation.currentTime()));
        } else {
            QPointF newTangent = (info.pos() - m_olderPaintInformation.pos()) * m_smoothness /
                                  (3.0 * (info.currentTime() - m_olderPaintInformation.currentTime()));
            qreal scaleFactor = (m_previousPaintInformation.currentTime() - m_olderPaintInformation.currentTime());
            QPointF control1 = m_olderPaintInformation.pos() + m_previousTangent * scaleFactor;
            QPointF control2 = m_previousPaintInformation.pos() - newTangent * scaleFactor;
            paintBezierCurve(m_olderPaintInformation,
                            control1,
                            control2,
                            m_previousPaintInformation);
            m_previousTangent = newTangent;
        }
        m_olderPaintInformation = m_previousPaintInformation;
        m_strokeTimer.start(100);
    } else {
        paintLine(m_previousPaintInformation, info);
    }

    m_previousPaintInformation = info;
}

void KisToolFreehand::mouseReleaseEvent(KoPointerEvent* e)
{
    if(mode() == KisTool::PAINT_MODE &&
       e->button() == Qt::LeftButton) {

        if (!m_hasPaintAtLeastOnce) {
            paintAt(m_previousPaintInformation);
        } else if (m_smooth && !m_previousTangent.isNull()) {
            finishStroke();
        }
        m_strokeTimer.stop();

        endPaint();
        setMode(KisTool::HOVER_MODE);
        e->accept();
    }
    else {
        KisToolPaint::mouseReleaseEvent(e);
    }
}

void KisToolFreehand::finishStroke()
{
    if (mode() != KisTool::PAINT_MODE) {
        // shouldn't happen
        return;
    }
    QPointF newTangent = (m_previousPaintInformation.pos() - m_olderPaintInformation.pos()) * m_smoothness / 3.0;
    qreal scaleFactor = (m_previousPaintInformation.currentTime() - m_olderPaintInformation.currentTime());
    QPointF control1 = m_olderPaintInformation.pos() + m_previousTangent * scaleFactor;
    QPointF control2 = m_previousPaintInformation.pos() - newTangent;
    paintBezierCurve(m_olderPaintInformation,
                    control1,
                    control2,
                    m_previousPaintInformation);
    m_previousTangent = QPointF(0.0, 0.0);
    m_haveTangent = false;
}

void KisToolFreehand::keyPressEvent(QKeyEvent *event)
{
    if(mode() != KisTool::PAINT_MODE) {
        KisToolPaint::keyPressEvent(event);
        return;
    }

    event->accept();
}

void KisToolFreehand::keyReleaseEvent(QKeyEvent* event)
{
    if(mode() != KisTool::PAINT_MODE) {
        KisToolPaint::keyReleaseEvent(event);
        return;
    }

    event->accept();
}

void KisToolFreehand::gesture(const QPointF &offsetInDocPixels, const QPointF &initialDocPoint)
{
    currentPaintOpPreset()->settings()->changePaintOpSize(offsetInDocPixels.x(), offsetInDocPixels.y());

    m_outlineDocPoint = initialDocPoint;

    updateOutlineRect();
}

void KisToolFreehand::initPaint(KoPointerEvent *)
{
    KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2 *>(canvas());
    if(canvas2)
        canvas2->view()->disableControls();

    setCurrentNodeLocked(true);
    m_hasPaintAtLeastOnce = false;

    KisPaintDeviceSP paintDevice = currentNode()->paintDevice();
    KisPaintDeviceSP targetDevice;

    const KoCompositeOp* op = compositeOp();
    if (!op)
        op = paintDevice->colorSpace()->compositeOp(COMPOSITE_OVER);

    m_strokeTimeMeasure.start();
    m_paintIncremental = currentPaintOpPreset()->settings()->paintIncremental();

    if (!m_paintIncremental) {
        KisIndirectPaintingSupport* indirect =
            dynamic_cast<KisIndirectPaintingSupport*>(currentNode().data());

        if (indirect) {
            targetDevice = new KisPaintDevice(currentNode().data(), paintDevice->colorSpace());
            indirect->setTemporaryTarget(targetDevice);
            indirect->setTemporaryCompositeOp(op);
            indirect->setTemporaryOpacity(m_opacity);

            KisPaintLayer* paintLayer = dynamic_cast<KisPaintLayer*>(currentNode().data());

            if(paintLayer)
                indirect->setTemporaryChannelFlags(paintLayer->channelLockFlags());
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
        m_painter->setCompositeOp(compositeOp());
        m_painter->setOpacity(m_opacity);
    } else {
        m_painter->setCompositeOp(paintDevice->colorSpace()->compositeOp(COMPOSITE_ALPHA_DARKEN));
        m_painter->setOpacity(OPACITY_OPAQUE_U8);
    }

    m_previousTangent = QPointF(0, 0);
    m_haveTangent = false;


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
    KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2 *>(canvas());
    if(canvas2) {
        canvas2->view()->enableControls();
    }

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

            m_incrementalDirtyRegion.clear();
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

    if (m_assistant) {
        static_cast<KisCanvas2*>(canvas())->view()->paintingAssistantManager()->endStroke();
    }

#ifdef ENABLE_RECORDING
    if (image() && m_pathPaintAction)
        image()->actionRecorder()->addAction(*m_pathPaintAction);
    delete m_pathPaintAction;
    m_pathPaintAction = 0;
#endif

    setCurrentNodeLocked(false);
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

void KisToolFreehand::setDirty(const QVector<QRect> &rects)
{
    currentNode()->setDirty(rects);
    if (!m_paintIncremental) {
        m_incrementalDirtyRegion += rects;
    }
}


void KisToolFreehand::setDirty(const QRegion& region)
{
    qDebug() << "KisToolFreehand::setDirty(QRegion) called";
    if (region.isEmpty())
        return;

    QVector<QRect> rects = region.rects();

    currentNode()->setDirty(rects);

    if (!m_paintIncremental) {
        m_incrementalDirtyRegion += rects;
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
                QPointF pos = converter.documentToView(m_outlineDocPoint);

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
        outlineMode = KisPaintOpSettings::CursorIsNotOutline;

        if(m_explicitShowOutline ||
           mode() == KisTool::GESTURE_MODE ||
           (cfg.cursorStyle() == CURSOR_STYLE_OUTLINE &&
            (mode() == HOVER_MODE ||
             (mode() == PAINT_MODE && cfg.showOutlineWhilePainting())))) {

            outlineMode = KisPaintOpSettings::CursorIsOutline;
        }

        QPainterPath path = getOutlinePath(m_outlineDocPoint, outlineMode);
        paintToolOutline(&gc,pixelToView(path));
    }
}

QPointF KisToolFreehand::adjustPosition(const QPointF& point, const QPointF& strokeBegin)
{
    if (m_assistant) {
        QPointF ap = static_cast<KisCanvas2*>(canvas())->view()->paintingAssistantManager()->adjustPosition(point, strokeBegin);
        return (1.0 - m_magnetism) * point + m_magnetism * ap;
    }
    return point;
}

QPainterPath KisToolFreehand::getOutlinePath(const QPointF &documentPos,
                                             KisPaintOpSettings::OutlineMode outlineMode)
{
    qreal scale = 1.0;
    qreal rotation = 0;
    if (m_painter && m_painter->paintOp()){
        scale = m_painter->paintOp()->currentScale();
        rotation = m_painter->paintOp()->currentRotation();
    }

    QPointF imagePos = currentImage()->documentToPixel(documentPos);
    QPainterPath path = currentPaintOpPreset()->settings()->
        brushOutline(imagePos, outlineMode, scale, rotation);

    return path;
}

void KisToolFreehand::increaseBrushSize()
{
    int paintopSize = currentPaintOpPreset()->settings()->paintOpSize().width();
    int increment = 1;
    if(paintopSize > 100) {
        increment = 30;
    } else if (paintopSize > 10){
        increment = 10;
    }
    currentPaintOpPreset()->settings()->changePaintOpSize(increment, 0);
    showOutlineTemporary();
}

void KisToolFreehand::decreaseBrushSize()
{
    int paintopSize = currentPaintOpPreset()->settings()->paintOpSize().width();
    int decrement = -1;
    if(paintopSize > 100) {
        decrement = -30;
    } else if (paintopSize > 20){
        decrement = -10;
    }
    currentPaintOpPreset()->settings()->changePaintOpSize(decrement, 0);
    showOutlineTemporary();
}

void KisToolFreehand::updateOutlineRect()
{
    QRectF outlinePixelRect = getOutlinePath(m_outlineDocPoint, KisPaintOpSettings::CursorIsOutline).boundingRect();
    QRectF outlineDocRect = currentImage()->pixelToDocument(outlinePixelRect);

    if(!m_oldOutlineRect.isEmpty()) {
        canvas()->updateCanvas(m_oldOutlineRect);
    }

    // This adjusted call is needed as we paint with a 3 pixel wide brush and the pen is outside the bounds of the path
    // Pen uses view coordinates so we have to zoom the document value to match 2 pixel in view coordiates
    // See BUG 275829
    qreal zoomX;
    qreal zoomY;
    canvas()->viewConverter()->zoom(&zoomX, &zoomY);
    qreal xoffset = 2.0/zoomX;
    qreal yoffset = 2.0/zoomY;
    m_oldOutlineRect = outlineDocRect.adjusted(-xoffset,-yoffset,xoffset,yoffset);

    canvas()->updateCanvas(m_oldOutlineRect);
}

void KisToolFreehand::showOutlineTemporary()
{
    m_explicitShowOutline = true;
    m_outlineTimer.start(HIDE_OUTLINE_TIMEOUT);
    updateOutlineRect();
}

void KisToolFreehand::hideOutline()
{
    m_explicitShowOutline = false;
    updateOutlineRect();
}

#include "kis_tool_freehand.moc"

