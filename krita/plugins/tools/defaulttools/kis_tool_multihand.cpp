/*
 *  kis_tool_freehand.cc - part of Krita
 *
 *  Copyright (c) 2003-2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
 *  Copyright (c) 2007,2008,2010 Cyrille Berger <cberger@cberger.net>
 *  Copyright (c) 2011 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#define MAXIMUM_SMOOTHNESS 1000
#define MAXIMUM_MAGNETISM 1000


#include "kis_tool_multihand.h"
#include "kis_tool_multihand_p.h"

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
#include <kis_transaction.h>
#include <kis_slider_spin_box.h>
#include <QComboBox>
#include <QStackedWidget>
#include <QFormLayout>

#define ENABLE_RECORDING
static const int HIDE_OUTLINE_TIMEOUT = 800; // ms
static const int MAXIMUM_BRUSHES = 50;

KisToolMultihand::KisToolMultihand(KoCanvasBase * canvas, const QCursor & cursor, const QString & transactionText)
    : KisToolPaint(canvas, cursor)
    , m_transactionText(transactionText)
    , m_mirrorVertically(false)
    , m_mirrorHorizontally(true)
    , m_translateRadius(100)
    , m_dragAxis(false)
{
    m_explicitShowOutline = false;

    m_brushesCount = 6; // UI value
    m_currentTransformMode = SYMETRY;
    m_axisPoint = QPointF(0.5 * image()->width(), 0.5 * image()->height());

    m_transaction = 0;
    //m_painters;

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

    m_outlineTimer.setSingleShot(true);
    connect(&m_outlineTimer, SIGNAL(timeout()), this, SLOT(hideOutline()));

    m_airbrushTimer = new QTimer(this);
    m_rate = 100;
    m_isAirbrushing = false;
    connect(m_airbrushTimer, SIGNAL(timeout()), this, SLOT(timeoutPaint()));

}

void KisToolMultihand::timeoutPaint()
{
    Q_ASSERT(currentPaintOpPreset()->settings()->isAirbrushing());
    if (currentImage() && !m_painters.isEmpty()) {

        for (int i = 0; i < m_painters.size(); i++){
            KisPaintInformation pi1 = m_previousPaintInformation;
            pi1.setPos( m_brushTransforms.at(i).map(pi1.pos()) );
            paintAt(pi1, m_painters[i]);
            currentNode()->setDirty(m_painters[i]->takeDirtyRegion());
        }

    }
}


KisToolMultihand::~KisToolMultihand()
{
}

int KisToolMultihand::flags() const
{
    return KisTool::FLAG_USES_CUSTOM_COMPOSITEOP|KisTool::FLAG_USES_CUSTOM_PRESET;
}

void KisToolMultihand::initTransformations()
{
    // mirror mode decide own number of brushes
    if (m_currentTransformMode != MIRROR) {
        m_brushTransforms.resize(m_brushesCount);
    }

    QTransform m;

    if (m_currentTransformMode == SYMETRY) {
        qreal angle = 0.0;
        qreal angleAddition = (2 * M_PI) / m_brushesCount;

        for (int i = 0; i < m_brushesCount ; i++){
            m.reset();
            m.translate(m_axisPoint.x(), m_axisPoint.y());

            m.rotateRadians(angle);

            m.translate(-m_axisPoint.x(), -m_axisPoint.y());
            m_brushTransforms[i] = m;

            angle += angleAddition;
        }
    } else if (m_currentTransformMode == MIRROR) {
        m_brushesCount = 1;
        if (m_mirrorHorizontally && m_mirrorVertically) {
            m_brushesCount = 4;
        } else if (m_mirrorHorizontally || m_mirrorVertically){
            m_brushesCount = 2;
        } else {
            m_brushesCount = 1;
        }

        m_brushCounter->setValue(m_brushesCount);
        m_brushCounter->update();
        m_brushTransforms.resize(m_brushesCount);
        int position = 0;

        m.reset();
        m_brushTransforms[position] = m;
        position++;

        if (m_mirrorHorizontally){
            m.reset();
            m.translate(m_axisPoint.x(),m_axisPoint.y());
            m.scale(-1,1);
            m.translate(-m_axisPoint.x(), -m_axisPoint.y());
            m_brushTransforms[position] = m;
            position++;
        }

        if (m_mirrorVertically){
            m.reset();
            m.translate(m_axisPoint.x(),m_axisPoint.y());
            m.scale(1,-1);
            m.translate(-m_axisPoint.x(), -m_axisPoint.y());
            m_brushTransforms[position] = m;
            position++;
        }

        if (m_mirrorVertically && m_mirrorHorizontally){
            m.reset();
            m.translate(m_axisPoint.x(),m_axisPoint.y());
            m.scale(-1,-1);
            m.translate(-m_axisPoint.x(), -m_axisPoint.y());
            m_brushTransforms[position] = m;
        }

    } else // if (m_currentTransformMode == TRANSLATE)
    {
        int m_areaRadius = m_translateRadius;
        for (int i = 0; i < m_brushesCount ; i++){
            m.reset();
            m.translate(m_axisPoint.x(),m_axisPoint.y());

            qreal angle = drand48() * M_PI * 2;
            qreal length = drand48();
            // convert the polar coordinates to Cartesian coordinates
            qreal nx = (m_areaRadius * cos(angle)  * length);
            qreal ny = (m_areaRadius * sin(angle)  * length);

            m.translate(nx,ny);

            m.translate(-m_axisPoint.x(), -m_axisPoint.y());
            m_brushTransforms[i] = m;
        }
    }
}

void KisToolMultihand::deactivate()
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

void KisToolMultihand::mousePressEvent(KoPointerEvent *e)
{
    if (mode() == KisTool::OTHER) {
        m_dragAxis = true;
        m_axisPoint = convertToPixelCoord(e->point);
        return;
    }

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

        initTransformations();
        initPaint(e);

        m_rate = currentPaintOpPreset()->settings()->rate();
        m_isAirbrushing = currentPaintOpPreset()->settings()->isAirbrushing();


        if (m_isAirbrushing) {
            m_airbrushTimer->start(m_rate);
        }

        m_previousPaintInformation = KisPaintInformation(convertToPixelCoord(adjustPosition(e->point, e->point)),
                                                         pressureToCurve(e->pressure()), e->xTilt(), e->yTilt(),
                                                         KisVector2D::Zero(),
                                                         e->rotation(), e->tangentialPressure(), perspective, m_strokeTimeMeasure.elapsed());
        m_strokeBegin = e->point;

        e->accept();
    }
    else {
        KisToolPaint::mousePressEvent(e);
    }
}

void KisToolMultihand::mouseMoveEvent(KoPointerEvent *e)
{
    if (mode() == KisTool::OTHER){
        useCursor(KisCursor::crossCursor());
        if (m_dragAxis) {
            m_axisPoint = convertToPixelCoord(e->point);
        }
        updateCanvas();
        return;
    }

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
                updateCanvas();
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
                         info,m_painters[0]);

        // naive implementation
        for (int i = 1; i < m_brushesCount; i++){
            KisPaintInformation pi1 = m_previousPaintInformation;
            KisPaintInformation pi2 = info;

            pi1.setPos( m_brushTransforms.at(i).map(pi1.pos()) );
            pi2.setPos( m_brushTransforms.at(i).map(pi2.pos()) );

            QPointF control1tx = m_brushTransforms.at(i).map(control1);
            QPointF control2tx = m_brushTransforms.at(i).map(control2);

            paintBezierCurve(pi1,
                         control1tx,
                         control2tx,
                         pi2,m_painters[i]);
        }



        m_previousTangent = newTangent;
    } else {
        paintLine(m_previousPaintInformation, info,m_painters[0]);
        for (int i = 1; i < m_brushesCount; i++){
            KisPaintInformation pi1 = m_previousPaintInformation;
            KisPaintInformation pi2 = info;

            pi1.setPos( m_brushTransforms.at(i).map(pi1.pos()) );
            pi2.setPos( m_brushTransforms.at(i).map(pi2.pos()) );

            paintLine(pi1, pi2,m_painters[i]);
        }
    }

    m_previousPaintInformation = info;

    if (m_painters[0] && m_painters[0]->paintOp() && m_isAirbrushing) {
        m_airbrushTimer->start(m_rate);
    }
}

void KisToolMultihand::mouseReleaseEvent(KoPointerEvent* e)
{
    if(mode() == KisTool::PAINT_MODE &&
       e->button() == Qt::LeftButton) {

        if (!m_hasPaintAtLeastOnce) {
            paintAt(m_previousPaintInformation,m_painters[0]);
            for (int i = 1; i < m_brushesCount; i++) {
                KisPaintInformation pi = m_previousPaintInformation;
                pi.setPos( pi.pos()+m_translatedBrush );
                paintAt(pi,m_painters[i]);
            }
        }

        endPaint();
        setMode(KisTool::HOVER_MODE);
        e->accept();
    }
    else if (mode() == KisTool::OTHER){
        m_axisPoint = convertToPixelCoord(e->point);
        finishAxisSetup();
    } else {
        KisToolPaint::mouseReleaseEvent(e);
    }
}

void KisToolMultihand::keyPressEvent(QKeyEvent *event)
{
    if(mode() != KisTool::PAINT_MODE) {
        KisToolPaint::keyPressEvent(event);
        return;
    }

    event->accept();
}

void KisToolMultihand::keyReleaseEvent(QKeyEvent* event)
{
    if(mode() != KisTool::PAINT_MODE) {
        KisToolPaint::keyReleaseEvent(event);
        return;
    }

    event->accept();
}

void KisToolMultihand::gesture(const QPointF &offsetInDocPixels, const QPointF &initialDocPoint)
{
    currentPaintOpPreset()->settings()->changePaintOpSize(offsetInDocPixels.x(), offsetInDocPixels.y());

    m_outlineDocPoint = initialDocPoint;

    updateOutlineRect();
}

void KisToolMultihand::initPaint(KoPointerEvent *)
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
        }
        else {
            m_paintIncremental = true;
        }
    }

    if (!targetDevice) {
        targetDevice = paintDevice;
    }

    m_transaction = new KisTransaction(m_transactionText, targetDevice);
    // setup painters
    for (int i = 0; i < m_brushesCount; i++){
        KisPainter * painter = new KisPainter(targetDevice, currentSelection());
        setupPainter(painter);
        if (m_paintIncremental) {
                painter->setCompositeOp(op);
                painter->setOpacity(m_opacity);
        } else {
                painter->setCompositeOp(paintDevice->colorSpace()->compositeOp(COMPOSITE_ALPHA_DARKEN));
                painter->setOpacity(OPACITY_OPAQUE_U8);
        }
        m_painters.append(painter);
    }

    m_previousTangent = QPointF(0, 0);


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

void KisToolMultihand::endPaint()
{
    m_airbrushTimer->stop();
    KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2 *>(canvas());
    if(canvas2) {
        canvas2->view()->enableControls();
    }

    m_executor->waitForDone();

    // If painting in mouse release, make sure painter
    // is destructed or end()ed

    // XXX: For now, only layers can be painted on in non-incremental mode
    KisLayerSP layer = dynamic_cast<KisLayer*>(currentNode().data());
    if (layer && !m_paintIncremental) {
        delete m_transaction;
        m_transaction = 0;

        KisIndirectPaintingSupport *indirect =
            dynamic_cast<KisIndirectPaintingSupport*>(layer.data());
        Q_ASSERT(indirect);

        indirect->mergeToLayer(layer, m_incrementalDirtyRegion, m_transactionText);

        m_incrementalDirtyRegion = QRegion();
    } else {
        m_transaction->commit(image()->undoAdapter());
    }


    foreach (KisPainter * painter, m_painters){
        if (painter) {
            delete painter;
            painter = 0;
        }
    }
    m_painters.resize(0);

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

void KisToolMultihand::paintAt(const KisPaintInformation &pi, KisPainter * painter)
{
    m_hasPaintAtLeastOnce = true;
    FreehandPaintJob* previousJob = m_paintJobs.empty() ? 0 : m_paintJobs.last();
    queuePaintJob(new FreehandPaintAtJob(this, painter, pi, previousJob), previousJob);
    m_pathPaintAction->addPoint(pi);
}

void KisToolMultihand::paintLine(const KisPaintInformation &pi1,
                                const KisPaintInformation &pi2, KisPainter * painter)
{
    m_hasPaintAtLeastOnce = true;
    FreehandPaintJob* previousJob = m_paintJobs.empty() ? 0 : m_paintJobs.last();
    queuePaintJob(new FreehandPaintLineJob(this, painter, pi1, pi2, previousJob), previousJob);
    m_pathPaintAction->addLine(pi1, pi2);
}

void KisToolMultihand::paintBezierCurve(const KisPaintInformation &pi1,
                                       const QPointF &control1,
                                       const QPointF &control2,
                                       const KisPaintInformation &pi2, KisPainter * painter)
{
    m_hasPaintAtLeastOnce = true;
    FreehandPaintJob* previousJob = m_paintJobs.empty() ? 0 : m_paintJobs.last();
    queuePaintJob(new FreehandPaintBezierJob(this, painter, pi1, control1, control2, pi2, previousJob), previousJob);
    m_pathPaintAction->addCurve(pi1, control1, control2, pi2);
}

void KisToolMultihand::queuePaintJob(FreehandPaintJob* job, FreehandPaintJob* /*previousJob*/)
{
    m_paintJobs.append(job);
    //    dbgUI << "Queue length:" << m_executor->queueLength();
    m_executor->start(job, -m_paintJobs.size());
}

bool KisToolMultihand::wantsAutoScroll() const
{
    return false;
}

void KisToolMultihand::setDirty(const QVector<QRect> &rects)
{
    currentNode()->setDirty(rects);
    if (!m_paintIncremental) {
        foreach (const QRect &rc, rects) {
            m_incrementalDirtyRegion += rc;
        }
    }
}

void KisToolMultihand::setDirty(const QRegion& region)
{
    if (region.isEmpty())
        return;

    currentNode()->setDirty(region);

    if (!m_paintIncremental) {
        m_incrementalDirtyRegion += region;
    }
}

void KisToolMultihand::setSmooth(bool smooth)
{
    m_smooth = smooth;
}

void KisToolMultihand::setAssistant(bool assistant)
{
    m_assistant = assistant;
}

void KisToolMultihand::paint(QPainter& gc, const KoViewConverter &converter)
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

    if (mode() == KisTool::OTHER){
        QPainterPath path;
        path.moveTo(m_axisPoint.x(), 0);
        path.lineTo(m_axisPoint.x(), currentImage()->height());
        path.moveTo(0, m_axisPoint.y());
        path.lineTo(currentImage()->width(), m_axisPoint.y());
        paintToolOutline(&gc, pixelToView(path));

    }

}

QPointF KisToolMultihand::adjustPosition(const QPointF& point, const QPointF& strokeBegin)
{
    if (m_assistant) {
        QPointF ap = static_cast<KisCanvas2*>(canvas())->view()->paintingAssistantManager()->adjustPosition(point, strokeBegin);
        return (1.0 - m_magnetism) * point + m_magnetism * ap;
    }
    return point;
}

QPainterPath KisToolMultihand::getOutlinePath(const QPointF &documentPos,
                                             KisPaintOpSettings::OutlineMode outlineMode)
{
    qreal scale = 1.0;
    qreal rotation = 0;
    if (!m_painters.isEmpty() ){
        if (m_painters[0]->paintOp()){
            scale = m_painters[0]->paintOp()->currentScale();
            rotation = m_painters[0]->paintOp()->currentRotation();
        }
    }

    QPointF imagePos = currentImage()->documentToPixel(documentPos);
    QPainterPath path = currentPaintOpPreset()->settings()->
        brushOutline(imagePos, outlineMode, scale, rotation);

    return path;
}

void KisToolMultihand::increaseBrushSize()
{
    currentPaintOpPreset()->settings()->changePaintOpSize(1, 0);
    showOutlineTemporary();
}

void KisToolMultihand::decreaseBrushSize()
{
    currentPaintOpPreset()->settings()->changePaintOpSize(-1, 0);
    showOutlineTemporary();
}

void KisToolMultihand::updateOutlineRect()
{
    QRectF outlinePixelRect = getOutlinePath(m_outlineDocPoint, KisPaintOpSettings::CursorIsOutline).boundingRect();
    QRectF outlineDocRect = currentImage()->pixelToDocument(outlinePixelRect);

    if(!m_oldOutlineRect.isEmpty()) {
        canvas()->updateCanvas(m_oldOutlineRect);
    }

#ifdef __GNUC__
#warning "Remove adjusted() call -- it smells hacky"
#else
#pragma WARNING( "Remove adjusted() call -- it smells hacky" )
#endif
    m_oldOutlineRect = outlineDocRect.adjusted(-2,-2,2,2);

    canvas()->updateCanvas(m_oldOutlineRect);
}

void KisToolMultihand::showOutlineTemporary()
{
    m_explicitShowOutline = true;
    m_outlineTimer.start(HIDE_OUTLINE_TIMEOUT);
    updateOutlineRect();
}

void KisToolMultihand::hideOutline()
{
    m_explicitShowOutline = false;
    updateOutlineRect();
}

QWidget * KisToolMultihand::createOptionWidget()
{

    QWidget * optionWidget = KisToolPaint::createOptionWidget();
    optionWidget->setObjectName(toolId() + "option widget");

    m_chkSmooth = new QCheckBox(i18nc("smooth out the curves while drawing", "Smoothness:"), optionWidget);
    m_chkSmooth->setObjectName("chkSmooth");
    m_chkSmooth->setChecked(m_smooth);
    connect(m_chkSmooth, SIGNAL(toggled(bool)), this, SLOT(setSmooth(bool)));

    m_sliderSmoothness = new KisSliderSpinBox(optionWidget);
    m_sliderSmoothness->setRange(0, MAXIMUM_SMOOTHNESS);
    m_sliderSmoothness->setEnabled(m_smooth);
    connect(m_chkSmooth, SIGNAL(toggled(bool)), m_sliderSmoothness, SLOT(setEnabled(bool)));
    connect(m_sliderSmoothness, SIGNAL(valueChanged(int)), SLOT(slotSetSmoothness(int)));
    m_sliderSmoothness->setValue(m_smoothness * MAXIMUM_SMOOTHNESS);

    addOptionWidgetOption(m_sliderSmoothness, m_chkSmooth);

    // Drawing assistant configuration
    m_chkAssistant = new QCheckBox(i18n("Assistant:"), optionWidget);
    m_chkAssistant->setToolTip(i18n("You need to add Ruler Assistants before this tool will work."));
    connect(m_chkAssistant, SIGNAL(toggled(bool)), this, SLOT(setAssistant(bool)));
    m_sliderMagnetism = new KisSliderSpinBox(optionWidget);
    m_sliderMagnetism->setToolTip(i18n("Assistant Magnetism"));
    m_sliderMagnetism->setRange(0, MAXIMUM_SMOOTHNESS);
    m_sliderMagnetism->setEnabled(false);
    connect(m_chkAssistant, SIGNAL(toggled(bool)), m_sliderMagnetism, SLOT(setEnabled(bool)));
    m_sliderMagnetism->setValue(m_magnetism * MAXIMUM_MAGNETISM);
    connect(m_sliderMagnetism, SIGNAL(valueChanged(int)), SLOT(slotSetMagnetism(int)));

    addOptionWidgetOption(m_sliderMagnetism, m_chkAssistant);

    m_axisPointBtn = new QPushButton(i18n("Axis point"), optionWidget);
    m_axisPointBtn->setCheckable(true);

    connect(m_axisPointBtn, SIGNAL(clicked(bool)),this, SLOT(activateAxisPointModeSetup()));
    addOptionWidgetOption(m_axisPointBtn);

    m_transformModes = new QComboBox(optionWidget);
    m_transformModes->addItem(i18n("Symmetry"),int(SYMETRY));
    m_transformModes->addItem(i18n("Mirror"),int(MIRROR));
    m_transformModes->addItem(i18n("Translate"),int(TRANSLATE));
    connect(m_transformModes,SIGNAL(currentIndexChanged(int)),SLOT(slotSetCurrentTransformMode(int)));

    m_brushCounter = new KisSliderSpinBox(optionWidget);
    m_brushCounter->setToolTip( i18n("Brush count") );
    m_brushCounter->setRange( 0, MAXIMUM_BRUSHES );
    m_brushCounter->setValue( m_brushesCount );
    m_brushCounter->setEnabled( true );
    connect(m_brushCounter, SIGNAL(valueChanged(int)),this, SLOT(slotSetBrushCount(int)));

    addOptionWidgetOption(m_transformModes);
    addOptionWidgetOption(m_brushCounter);

    m_modeCustomOption = new QStackedWidget(optionWidget);

    QWidget * symmetryWidget = new QWidget(m_modeCustomOption);
    m_modeCustomOption->addWidget(symmetryWidget);

    QWidget * mirrorWidget = new QWidget(m_modeCustomOption);
    m_mirrorHorizontallyChCkBox = new QCheckBox(i18n("Horizontally"));
    m_mirrorHorizontallyChCkBox->setChecked(m_mirrorHorizontally);
    m_mirrorVerticallyChCkBox = new QCheckBox(i18n("Vertically"));
    m_mirrorVerticallyChCkBox->setChecked(m_mirrorVertically);
    connect(m_mirrorHorizontallyChCkBox,SIGNAL(toggled(bool)),this, SLOT(slotSetMirrorHorizontally(bool)));
    connect(m_mirrorVerticallyChCkBox,SIGNAL(toggled(bool)),this, SLOT(slotSetMirrorVertically(bool)));
    QGridLayout * mirrorLayout = new QGridLayout(mirrorWidget);
    mirrorLayout->addWidget(m_mirrorHorizontallyChCkBox,0,0);
    mirrorLayout->addWidget(m_mirrorVerticallyChCkBox,0,1);
    mirrorWidget->setLayout(mirrorLayout);
    m_modeCustomOption->addWidget(mirrorWidget);

    QWidget * translateWidget = new QWidget(m_modeCustomOption);
    m_translateRadiusSlider = new KisSliderSpinBox(translateWidget);
    m_translateRadiusSlider->setRange(0, 200);
    m_translateRadiusSlider->setValue(m_translateRadius);
    m_translateRadiusSlider->setSuffix(" px");
    connect(m_translateRadiusSlider,SIGNAL(valueChanged(int)),this,SLOT(slotSetTranslateRadius(int)));

    QFormLayout *radiusLayout = new QFormLayout(translateWidget);
    radiusLayout->addRow( i18n("Radius"), m_translateRadiusSlider);
    translateWidget->setLayout(radiusLayout);

    m_modeCustomOption->addWidget(translateWidget);

    addOptionWidgetOption(m_modeCustomOption);

    return optionWidget;
}

void KisToolMultihand::slotSetBrushCount(int count)
{
    m_brushesCount = count;
}

void KisToolMultihand::slotSetCurrentTransformMode(int qcomboboxIndex)
{
    m_currentTransformMode = enumTransforModes( m_transformModes->itemData(qcomboboxIndex).toInt() );
    if (m_modeCustomOption) {
        m_modeCustomOption->setCurrentIndex(qcomboboxIndex);
    }
}

void KisToolMultihand::slotSetSmoothness(int smoothness)
{
    m_smoothness = smoothness / (double)MAXIMUM_SMOOTHNESS;
}

void KisToolMultihand::slotSetMagnetism(int magnetism)
{
    m_magnetism = expf(magnetism / (double)MAXIMUM_MAGNETISM) / expf(1.0);
}

void KisToolMultihand::slotSetMirrorHorizontally(bool mirror)
{
    m_mirrorHorizontally = mirror;
}

void KisToolMultihand::slotSetMirrorVertically(bool mirror)
{
    m_mirrorVertically = mirror;
}

void KisToolMultihand::slotSetTranslateRadius(int radius)
{
    m_translateRadius = radius;
}

void KisToolMultihand::activateAxisPointModeSetup()
{
    if (m_axisPointBtn->isChecked()){
        setMode(KisTool::OTHER);
        useCursor(KisCursor::crossCursor());
        updateCanvas();
    } else {
        finishAxisSetup();
    }
}

void KisToolMultihand::updateCanvas()
{
    KisCanvas2 *kisCanvas = dynamic_cast<KisCanvas2*>(canvas());
    if (kisCanvas) {
        kisCanvas->updateCanvas();
    }
}

void KisToolMultihand::finishAxisSetup()
{
    setMode(KisTool::HOVER_MODE);
    m_dragAxis = false;
    resetCursorStyle();
    updateCanvas();
    m_axisPointBtn->setChecked(false);
}

#include "kis_tool_multihand.moc"

