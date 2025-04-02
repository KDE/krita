/*
 *  SPDX-FileCopyrightText: 2025 Agata Cacko
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisToolKnife.h"

#include "QApplication"
#include "QPainterPath"

#include <klocalizedstring.h>
#include <KoColor.h>
#include <KisViewManager.h>
#include "kis_canvas2.h"
#include "kis_cursor.h"
#include "kis_painter.h"
#include "kis_paintop_preset.h"

#include "kundo2magicstring.h"
#include "kundo2stack.h"
#include "commands_new/kis_transaction_based_command.h"
#include "kis_transaction.h"
#include "KoPathShape.h"

#include "kis_processing_applicator.h"
#include "kis_datamanager.h"

#include "KoColorSpaceRegistry.h"
#include <KisCursorOverrideLock.h>

#include "KisToolKnifeOptionsWidget.h"
#include "libs/image/kis_paint_device_debug_utils.h"

#include <KoSelectedShapesProxy.h>

#include "CutThroughShapeStrategy.h"
#include "RemoveGutterStrategy.h"

#include "kis_paint_layer.h"
#include "kis_algebra_2d.h"
#include "kis_resources_snapshot.h"
#include <KoSelection.h>
#include <KoShapeManager.h>


struct KisToolKnife::Private {
    KisToolKnifeOptionsWidget *optionsWidget = nullptr;
    QPointF startPoint = QPointF(0, 0);
    QPointF endPoint = QPointF(0, 0);
    //QPointF mousePoint = QPointF(0, 0);
    QRectF previousLineDirtyRect = QRectF();
    //QRectF previousMouseDirtyRect = QRectF();

};


KisToolKnife::KisToolKnife(KoCanvasBase * canvas)
    : KoInteractionTool(canvas),
      m_d(new Private)
{
    setObjectName("tool_knife");
    //useCursor(KisCursor::arrowCursor());
    useCursor(Qt::ArrowCursor);
    repaintDecorations();
}

KisToolKnife::~KisToolKnife()
{
    m_d->optionsWidget = nullptr;
}

void drawCoordsStartFrom(QPainter &painter, QTransform transform, QColor color)
{
    painter.save();
    QPen pen = QPen(QBrush(color), 5, Qt::SolidLine);
    painter.setPen(pen);
    painter.setTransform(transform * painter.transform());
    painter.drawLine(QPointF(0, 0), QPointF(100, 0));
    painter.drawLine(QPointF(0, 0), QPointF(0, 100));
    painter.drawLine(QPointF(0, 100), QPointF(5, 100));
    painter.drawLine(QPointF(100, 0), QPointF(100, 5));

    painter.drawArc(QRect(-50, -50, 100, 100), 0, -16*90);

    painter.restore();
}


void drawCoordsStart(QPainter &painter, const KoViewConverter &converter)
{
    //painter.drawLine(QPointF(0, 0), QPointF(100, 0));
    //painter.drawLine(QPointF(0, 0), QPointF(0, 100));
    //painter.drawArc(QRect(-50, -50, 100, 100), 0, -16*90);

    drawCoordsStartFrom(painter, QTransform(), Qt::red);
    drawCoordsStartFrom(painter, converter.documentToView(), Qt::blue); // < ten ma sens w painterze
    drawCoordsStartFrom(painter, converter.viewToDocument(), Qt::green);



}


void paintSelectedEdge(QPainter &painter, const KoViewConverter &converter, const QLineF &lineSegment)
{
    QLineF lineInView = converter.documentToView().map(lineSegment);
    QList<QLineF> parallels = KisAlgebra2D::getParallelLines(lineInView, 5);

    painter.save();
    qreal width = 2;
    QColor color = Qt::blue;
    color.setAlphaF(0.8);
    QColor white = Qt::white;
    white.setAlphaF(0.75);

    QPen pen = QPen(color, width);
    QPen alternative = QPen(white, width);

    alternative.setStyle(Qt::CustomDashLine);
    qreal dashLength = 6;
    alternative.setDashPattern({dashLength - 1, dashLength + 1});
    alternative.setCapStyle(Qt::RoundCap);

    pen.setCosmetic(true);
    painter.setPen(pen);

    //painter.drawLines(parallels.toVector());
    painter.drawLine(lineInView);

    alternative.setCosmetic(true);
    painter.setPen(alternative);
    //painter.drawLines(parallels.toVector());
    painter.drawLine(lineInView);


    painter.restore();

}

QPolygonF createDiamond(int size, QPointF location = QPointF())
{
    QPolygonF polygon;
    polygon << QPointF(-size, 0);
    polygon << QPointF(0, size);
    polygon << QPointF(size, 0);
    polygon << QPointF(0, -size);
    polygon.translate(location);
    return polygon;
}

void paintSelectedPoint(QPainter &painter, const KoViewConverter &converter, const QPointF &point)
{
    QPointF p = point;
    p = converter.documentToView().map(p);
    QPolygonF diamond = createDiamond(6, p);
    painter.save();
    QColor color = Qt::blue;
    color.setAlphaF(0.9);
    QColor white = Qt::white;
    white.setAlphaF(0.75);

    QPen pen = QPen(color, 2);
    pen.setCosmetic(true);
    QBrush brush = QBrush(white);
    painter.setPen(pen);
    painter.setBrush(brush);

    painter.drawPolygon(diamond);
    painter.restore();
}

void KisToolKnife::paint(QPainter &painter, const KoViewConverter &converter)
{
    Q_UNUSED(converter);

    painter.save();
    painter.restore();

    painter.save();
    painter.setBrush(Qt::darkGray);
    //painter.drawEllipse(converter.documentToView(m_d->mousePoint), 4, 4);

    painter.restore();

    KoInteractionTool::paint(painter, converter);


    bool paintSelection = false;
    if (paintSelection) {

        KIS_SAFE_ASSERT_RECOVER_RETURN(canvas());
        KIS_SAFE_ASSERT_RECOVER_RETURN(canvas()->selectedShapesProxy());

        KIS_SAFE_ASSERT_RECOVER_RETURN(canvas()->selectedShapesProxy()->selection());

        KoSelection *selection = canvas()->selectedShapesProxy()->selection();

        QList<KoShape*> shapes = selection->selectedEditableShapes();

        Q_FOREACH(KoShape* shape, shapes) {
            KisAlgebra2D::VectorPath vector = KisAlgebra2D::VectorPath(shape->outline());
            painter.save();
            painter.setTransform(painter.transform());
            for (int i = 0; i < vector.segmentsCount(); i++) {
                paintSelectedEdge(painter, converter, shape->absoluteTransformation().map(vector.segmentAtAsLine(i)));
            }
            for (int i =  0; i < vector.pointsCount(); i++) {
                paintSelectedPoint(painter, converter, shape->absoluteTransformation().map(vector.pointAt(i).endPoint));
            }
            painter.restore();

        }
    }



}

void KisToolKnife::activate(const QSet<KoShape *> &shapes)
{
    // Questions to Dmitry or Wolthera:
    // + 1. Which tool base should it be based on?
    // + 2. Which functions should I reimplement: mouse events, or beingPrimaryAction, or a strategy?
    // - 3. How to get shapes from the layer? I assume that it might happen that no shape is selected, in which case
    //      it should use all shapes on the layer as potential input
    // - 4. How to get points from that shape? I noticed I have KoPathShape* sometimes, and that has (protected) subpaths,
    //      but I can access segments I think? And outline() is useful for now for debug. But how to do it properly?
    // 5. How to get all shapes from the layer, not just selected ones? Or how to select all shapes?
    // 6. Can I make three files out of kis_algebra_2d? also what about kis_global? -> it's a mess and it's difficult to find things
    // 7. How to get a cursor? I knew how to in a different tool class, but not here...

    //canvas()->selectedShapesProxy();
    //this->set



    KoInteractionTool::activate(shapes);
    Q_FOREACH(KoShape* shape, shapes) {
        if (shape) {
            qCritical() << "We've got a shape!";
            KoPathShape* pathShape = dynamic_cast<KoPathShape *>(shape);
            qCritical() << "Is it a path shape? " << pathShape;
            if (pathShape) {
                qCritical() << "Let's see the points:";
                //qCritical() << pathShape->isClosedSubpath(0);
                //pathShape->subpaths();
                // ok, subpaths are protected...

                //temporary:
                QPainterPath pp = pathShape->outline();
                //qCritical() << pp.;
                //Q_FOREACH(QPainterPath::Element element, pp.elem)
                for (int i = 0; i < pp.elementCount(); i++) {
                    qCritical() << "Point at: " << pp.elementAt(i).x << pp.elementAt(i).y;
                }

                //Q_FOREACH(QPointF& p, pathShape.)
            }

        }
    }

    useCursor(KisCursor::arrowCursor());

}

void KisToolKnife::deactivate()
{
    KoInteractionTool::deactivate();
}

void KisToolKnife::mousePressEventBackup(KoPointerEvent *event)
{
    m_d->startPoint = event->point;//canvas()->viewConverter()->viewToDocument(event->pos());
    m_d->endPoint = event->point;//canvas()->viewConverter()->viewToDocument(event->pos());
}

void KisToolKnife::mouseMoveEvent(KoPointerEvent *event)
{
    KoInteractionTool::mouseMoveEvent(event);

    //qCritical() << "mouse move event button flags: " << event->buttons() << ", " << event->button();
    if (event->buttons().testFlag(Qt::MouseButton::LeftButton)) {

        //m_d->endPoint = canvas()->viewConverter()->viewToDocument(event->pos());
        m_d->endPoint = event->point;
        //qCritical() << "Now supposedly drawing a line between " << m_d->startPoint << " to " << m_d->endPoint;
        QRectF dirtyRect;
        KisAlgebra2D::accumulateBounds(m_d->startPoint, &dirtyRect);
        KisAlgebra2D::accumulateBounds(m_d->endPoint, &dirtyRect);
        //dirtyRect = canvas()->viewConverter()->viewToDocument(dirtyRect);
        //qCritical() << "And the dirty rect is: " << dirtyRect;

        QRectF accumulatedWithPrevious = m_d->previousLineDirtyRect;
        accumulatedWithPrevious |= dirtyRect;
        //KisAlgebra2D::accumulateBounds(dirtyRect, &accumulatedWithPrevious);
        //KisAlgebra2D::accumulateBounds(dirtyRect.topLeft(), &accumulatedWithPrevious);
        //KisAlgebra2D::accumulateBounds(dirtyRect.bottomRight(), &accumulatedWithPrevious);

        //QRectF rect = dirtyRect + m_d->previousLineDirtyRect;

        canvas()->updateCanvas(accumulatedWithPrevious);
        m_d->previousLineDirtyRect = dirtyRect;

    } else {
        //m_d->mousePoint = event->point;
        //QRectF dirtyRect;
        //KisAlgebra2D::accumulateBounds(m_d->mousePoint + QPointF(-2, -2), &dirtyRect);
        //KisAlgebra2D::accumulateBounds(m_d->mousePoint + QPointF(2, 2), &dirtyRect);

        //canvas()->updateCanvas(dirtyRect);
        //dirtyRect = canvas()->viewConverter()->viewToD(dirtyRect);

        //canvas()->updateCanvas(dirtyRect);

        //qCritical() << "Changing mouse point to " << event->point << " as opposed to " << event->pos();
    }
    repaintDecorations();
}

void KisToolKnife::mouseReleaseEvent(KoPointerEvent *event)
{
    KoInteractionTool::mouseReleaseEvent(event);

    m_d->endPoint = event->point;

    //qCritical() << "Now, AFTER release, supposedly drawing a line between " << m_d->startPoint.x() <<  ", " << m_d->startPoint.y() << " to " << m_d->endPoint.x() << "," << m_d->endPoint.y();
    QRectF dirtyRect;
    KisAlgebra2D::accumulateBounds(m_d->startPoint, &dirtyRect);
    KisAlgebra2D::accumulateBounds(m_d->endPoint, &dirtyRect);

    QRectF accumulatedWithPrevious = m_d->previousLineDirtyRect | dirtyRect;
    //KisAlgebra2D::accumulateBounds(dirtyRect.topLeft(), &accumulatedWithPrevious);
    //KisAlgebra2D::accumulateBounds(dirtyRect.bottomRight(), &accumulatedWithPrevious);

    //KisAlgebra2D::accumulateBounds(dirtyRect, &accumulatedWithPrevious);

    canvas()->updateCanvas(accumulatedWithPrevious);
    m_d->previousLineDirtyRect = dirtyRect;
    //dirtyRect = canvas()->viewConverter()->viewToDocument(dirtyRect);

    //canvas()->updateCanvas(dirtyRect);
}

KoInteractionStrategy *KisToolKnife::createStrategy(KoPointerEvent *event)
{
    qCritical() << "Creating a strategy for " << event->point << event->buttons();
    //KoSelection *selection = canvas()->selectedShapesProxy()->selection();
    QList<KoShape*> shapes = canvas()->shapeManager()->shapes();

    if (m_d->optionsWidget->getToolMode() == KisToolKnifeOptionsWidget::ToolMode::AddGutter) {
        return new CutThroughShapeStrategy(this, canvas()->selectedShapesProxy()->selection(), shapes, event->point, m_d->optionsWidget->getCurrentWidthsConfig());
    } else {
        return new RemoveGutterStrategy(this, canvas()->selectedShapesProxy()->selection(), shapes, event->point);
    }

    //return NULL;
}

QWidget * KisToolKnife::createOptionWidget()
{
    KisCanvas2 * kiscanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT(kiscanvas);
    qreal resolution = 1.0;
    if (kiscanvas->image()) {
        // we're going to assume isotropic image
        resolution = kiscanvas->image()->xRes();
    }

    m_d->optionsWidget = new KisToolKnifeOptionsWidget(kiscanvas->viewManager()->canvasResourceProvider(), 0, toolId(), resolution);
    m_d->optionsWidget->setObjectName(toolId() + "option widget");


    return m_d->optionsWidget;
}

