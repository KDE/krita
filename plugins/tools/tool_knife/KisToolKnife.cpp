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

#include "kis_paint_layer.h"
#include "kis_algebra_2d.h"
#include "kis_resources_snapshot.h"


struct KisToolKnife::Private {
    // TO REMOVE
    KisPaintDeviceSP maskDev = nullptr;
    KisPainter maskDevPainter;
    float brushRadius = 50.; //initial default. actually read from ui.
    KisToolKnifeOptionsWidget *optionsWidget = nullptr;
    QRectF oldOutlineRect;
    QPainterPath brushOutline;

    // TO KEEP
    QPointF startPoint = QPointF(0, 0);
    QPointF endPoint = QPointF(0, 0);
    QPointF mousePoint = QPointF(0, 0);
    QRectF previousLineDirtyRect = QRectF();
    QRectF previousMouseDirtyRect = QRectF();

};


KisToolKnife::KisToolKnife(KoCanvasBase * canvas)
    : KoInteractionTool(canvas),
      m_d(new Private)
{
    //setSupportOutline(true);
    setObjectName("tool_knife");
    m_d->maskDev = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
    m_d->maskDevPainter.begin( m_d->maskDev );

    m_d->maskDevPainter.setPaintColor(KoColor(Qt::magenta, m_d->maskDev->colorSpace()));
    m_d->maskDevPainter.setBackgroundColor(KoColor(Qt::white, m_d->maskDev->colorSpace()));
    m_d->maskDevPainter.setFillStyle( KisPainter::FillStyleForegroundColor );
}

KisToolKnife::~KisToolKnife()
{
    m_d->optionsWidget = nullptr;
    m_d->maskDevPainter.end();
}

void KisToolKnife::addMaskPath( KoPointerEvent *event )
{
    KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2 *>(canvas());
    KIS_ASSERT(canvas2);
    const KisCoordinatesConverter *converter = canvas2->coordinatesConverter();

    //QPointF imagePos = currentImage()->documentToPixel(event->point);
    //QPainterPath currentBrushOutline = brushOutline().translated(KisAlgebra2D::alignForZoom(imagePos, converter->effectivePhysicalZoom()));
    //m_d->maskDevPainter.fillPainterPath(currentBrushOutline);

    //canvas()->updateCanvas(currentImage()->pixelToDocument(m_d->maskDev->exactBounds()));
}


QPainterPath KisToolKnife::brushOutline( void )
{
    const qreal diameter = m_d->brushRadius;
    QPainterPath outline;
    outline.addEllipse(QPointF(0,0), -0.5 * diameter, -0.5 * diameter );
    return outline;
}

QPainterPath KisToolKnife::getBrushOutlinePath(const QPointF &documentPos,
                                          const KoPointerEvent *event)
{
    Q_UNUSED(event);

    //QPointF imagePos = currentImage()->documentToPixel(documentPos);
    QPainterPath path = brushOutline();

    KisCanvas2 *canvas2 = dynamic_cast<KisCanvas2 *>(canvas());
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(canvas2, QPainterPath());
    const KisCoordinatesConverter *converter = canvas2->coordinatesConverter();

    //return path.translated(KisAlgebra2D::alignForZoom(imagePos, converter->effectivePhysicalZoom()));
    return path;
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

void KisToolKnife::paint(QPainter &painter, const KoViewConverter &converter)
{
    //qCritical() << "~~~~ paint!!!!";
    Q_UNUSED(converter);

    painter.save();
    //QPainterPath path = pixelToView(m_d->brushOutline);
    //paintToolOutline(&painter, path);
    painter.restore();

    painter.save();
    painter.setBrush(Qt::darkGray);
    QImage img = m_d->maskDev->convertToQImage(0);
    if( !img.size().isEmpty() ){
        //painter.drawImage(pixelToView(m_d->maskDev->exactBounds()), img);
    }
    //painter.drawLine(converter.documentToView(m_d->startPoint), converter.documentToView(m_d->endPoint));
    painter.drawEllipse(converter.documentToView(m_d->mousePoint), 4, 4);
    //QPointF topLeft = QPointF(min(m_d->startPoint.x(), m_d->endPoint.x()), min(min(m_d->startPoint.x(), m_d->endPoint.x())));
    QRectF dirtyRect;
    KisAlgebra2D::accumulateBounds(m_d->startPoint, &dirtyRect);
    KisAlgebra2D::accumulateBounds(m_d->endPoint, &dirtyRect);
    KisAlgebra2D::accumulateBounds(m_d->mousePoint + QPointF(-2, -2), &dirtyRect);
    KisAlgebra2D::accumulateBounds(m_d->mousePoint + QPointF(2, 2), &dirtyRect);


    //dirtyRect = converter.viewToDocument(dirtyRect);

    drawCoordsStart(painter, converter);



    painter.restore();

    KoInteractionTool::paint(painter, converter);
    //currentStrategy()->paint(painter, converter);

    //canvas()->updateCanvas(dirtyRect);
}

void KisToolKnife::activate(const QSet<KoShape *> &shapes)
{
    // Questions to Dmitry or Wolthera:
    // 1. Which tool base should it be based on?
    // 2. Which functions should I reimplement: mouse events, or beingPrimaryAction, or a strategy?
    // 3. How to get shapes from the layer? I assume that it might happen that no shape is selected, in which case
    //      it should use all shapes on the layer as potential input
    // 4. How to get points from that shape? I noticed I have KoPathShape* sometimes, and that has (protected) subpaths,
    //      but I can access segments I think? And outline() is useful for now for debug. But how to do it properly?

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
        m_d->mousePoint = event->point;
        QRectF dirtyRect;
        KisAlgebra2D::accumulateBounds(m_d->mousePoint + QPointF(-2, -2), &dirtyRect);
        KisAlgebra2D::accumulateBounds(m_d->mousePoint + QPointF(2, 2), &dirtyRect);

        //canvas()->updateCanvas(dirtyRect);
        //dirtyRect = canvas()->viewConverter()->viewToD(dirtyRect);

        canvas()->updateCanvas(dirtyRect);

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
    KoSelection *selection = canvas()->selectedShapesProxy()->selection();
    return new CutThroughShapeStrategy(this, selection, event->point, m_d->optionsWidget->getCurrentWidth());
    //return NULL;
}

QWidget * KisToolKnife::createOptionWidget()
{
    KisCanvas2 * kiscanvas = dynamic_cast<KisCanvas2*>(canvas());
    KIS_ASSERT(kiscanvas);

    m_d->optionsWidget = new KisToolKnifeOptionsWidget(kiscanvas->viewManager()->canvasResourceProvider(), 0);
    m_d->optionsWidget->setObjectName(toolId() + "option widget");

    return m_d->optionsWidget;
}

