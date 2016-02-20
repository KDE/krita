
/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 * Copyright (c) 2010 Geoffry Song <goffrie@gmail.com>
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

#include <kis_ruler_assistant_tool.h>

#include <QPainter>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <QDesktopServices>
#include <QFile>
#include <QLineF>

#include <kis_debug.h>
#include <klocalizedstring.h>
#include <QMessageBox>

#include <KoIcon.h>
#include <kis_icon.h>
#include <KoFileDialog.h>
#include <KoViewConverter.h>
#include <KoPointerEvent.h>

#include <canvas/kis_canvas2.h>
#include <kis_canvas_resource_provider.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <KisViewManager.h>

#include <kis_abstract_perspective_grid.h>
#include <kis_painting_assistants_decoration.h>

#include <math.h>

KisRulerAssistantTool::KisRulerAssistantTool(KoCanvasBase * canvas)
    : KisTool(canvas, KisCursor::arrowCursor()), m_canvas(dynamic_cast<KisCanvas2*>(canvas)),
      m_assistantDrag(0), m_newAssistant(0), m_optionsWidget(0), m_handleSize(32), m_handleHalfSize(16)
{
    Q_ASSERT(m_canvas);
    setObjectName("tool_rulerassistanttool");
}

KisRulerAssistantTool::~KisRulerAssistantTool()
{
}

QPointF adjustPointF(const QPointF& _pt, const QRectF& _rc)
{
    return QPointF(qBound(_rc.left(), _pt.x(), _rc.right()), qBound(_rc.top(), _pt.y(), _rc.bottom()));
}

void KisRulerAssistantTool::activate(ToolActivation toolActivation, const QSet<KoShape*> &shapes)
{
    // Add code here to initialize your tool when it got activated
    KisTool::activate(toolActivation, shapes);

    m_handles = m_canvas->paintingAssistantsDecoration()->handles();
    m_canvas->paintingAssistantsDecoration()->setVisible(true);
    m_canvas->updateCanvas();
    m_handleDrag = 0;
    m_internalMode = MODE_CREATION;
    m_assistantHelperYOffset = 10;

}

void KisRulerAssistantTool::deactivate()
{
    // Add code here to initialize your tool when it got deactivated
    m_canvas->updateCanvas();
    KisTool::deactivate();
}

bool KisRulerAssistantTool::mouseNear(const QPointF& mousep, const QPointF& point)
{
    QRectF handlerect(point-QPointF(m_handleHalfSize,m_handleHalfSize), QSizeF(m_handleSize, m_handleSize));
    return handlerect.contains(mousep);
}

KisPaintingAssistantHandleSP KisRulerAssistantTool::nodeNearPoint(KisPaintingAssistant* grid, QPointF point)
{
    if (mouseNear(point, pixelToView(*grid->topLeft()))) {
        return grid->topLeft();
    } else if (mouseNear(point, pixelToView(*grid->topRight()))) {
        return grid->topRight();
    } else if (mouseNear(point, pixelToView(*grid->bottomLeft()))) {
        return grid->bottomLeft();
    } else if (mouseNear(point, pixelToView(*grid->bottomRight()))) {
        return grid->bottomRight();
    }
    return 0;
}

inline double norm2(const QPointF& p)
{
    return p.x() * p.x() + p.y() * p.y();
}

void KisRulerAssistantTool::beginPrimaryAction(KoPointerEvent *event)
{
    setMode(KisTool::PAINT_MODE);
    bool newAssistantAllowed = true;

    if (m_newAssistant) {
        m_internalMode = MODE_CREATION;
        *m_newAssistant->handles().back() = event->point;
        if (m_newAssistant->handles().size() == m_newAssistant->numHandles()) {
            addAssistant();
        } else {
            m_newAssistant->addHandle(new KisPaintingAssistantHandle(event->point));
        }
        m_canvas->updateCanvas();
        return;
    }
    m_handleDrag = 0;
    double minDist = 81.0;

    QPointF mousePos = m_canvas->viewConverter()->documentToView(event->point);
    Q_FOREACH (KisPaintingAssistant* assistant, m_canvas->paintingAssistantsDecoration()->assistants()) {
        Q_FOREACH (const KisPaintingAssistantHandleSP handle, m_handles) {
            double dist = norm2(mousePos - m_canvas->viewConverter()->documentToView(*handle));
            if (dist < minDist) {
                minDist = dist;
                m_handleDrag = handle;
            }
        }
        if(m_handleDrag && assistant->id() == "perspective") {
            // Look for the handle which was pressed

            if (m_handleDrag == assistant->topLeft()) {
                double dist = norm2(mousePos - m_canvas->viewConverter()->documentToView(*m_handleDrag));
                if (dist < minDist) {
                    minDist = dist;
                }
                m_dragStart = QPointF(assistant->topRight().data()->x(),assistant->topRight().data()->y());
                m_internalMode = MODE_DRAGGING_NODE;
            } else if (m_handleDrag == assistant->topRight()) {
                double dist = norm2(mousePos - m_canvas->viewConverter()->documentToView(*m_handleDrag));
                if (dist < minDist) {
                    minDist = dist;
                }
                m_internalMode = MODE_DRAGGING_NODE;
                m_dragStart = QPointF(assistant->topLeft().data()->x(),assistant->topLeft().data()->y());
            } else if (m_handleDrag == assistant->bottomLeft()) {
                double dist = norm2(mousePos - m_canvas->viewConverter()->documentToView(*m_handleDrag));
                if (dist < minDist) {
                    minDist = dist;
                }
                m_internalMode = MODE_DRAGGING_NODE;
                m_dragStart = QPointF(assistant->bottomRight().data()->x(),assistant->bottomRight().data()->y());
            } else if (m_handleDrag == assistant->bottomRight()) {
                double dist = norm2(mousePos - m_canvas->viewConverter()->documentToView(*m_handleDrag));
                if (dist < minDist) {
                    minDist = dist;
                }
                m_internalMode = MODE_DRAGGING_NODE;
                m_dragStart = QPointF(assistant->bottomLeft().data()->x(),assistant->bottomLeft().data()->y());
            } else if (m_handleDrag == assistant->leftMiddle()) {
                m_internalMode = MODE_DRAGGING_TRANSLATING_TWONODES;
                m_dragStart = QPointF((assistant->bottomLeft().data()->x()+assistant->topLeft().data()->x())*0.5,
                                      (assistant->bottomLeft().data()->y()+assistant->topLeft().data()->y())*0.5);
                m_selectedNode1 = new KisPaintingAssistantHandle(assistant->topLeft().data()->x(),assistant->topLeft().data()->y());
                m_selectedNode2 = new KisPaintingAssistantHandle(assistant->bottomLeft().data()->x(),assistant->bottomLeft().data()->y());
                m_newAssistant = KisPaintingAssistantFactoryRegistry::instance()->get("perspective")->createPaintingAssistant();
                m_newAssistant->addHandle(assistant->topLeft());
                m_newAssistant->addHandle(m_selectedNode1);
                m_newAssistant->addHandle(m_selectedNode2);
                m_newAssistant->addHandle(assistant->bottomLeft());
                m_dragEnd = event->point;
                m_handleDrag = 0;
                m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
                return;
            } else if (m_handleDrag == assistant->rightMiddle()) {
                m_dragStart = QPointF((assistant->topRight().data()->x()+assistant->bottomRight().data()->x())*0.5,
                                      (assistant->topRight().data()->y()+assistant->bottomRight().data()->y())*0.5);
                m_internalMode = MODE_DRAGGING_TRANSLATING_TWONODES;
                m_selectedNode1 = new KisPaintingAssistantHandle(assistant->topRight().data()->x(),assistant->topRight().data()->y());
                m_selectedNode2 = new KisPaintingAssistantHandle(assistant->bottomRight().data()->x(),assistant->bottomRight().data()->y());
                m_newAssistant = KisPaintingAssistantFactoryRegistry::instance()->get("perspective")->createPaintingAssistant();
                m_newAssistant->addHandle(assistant->topRight());
                m_newAssistant->addHandle(m_selectedNode1);
                m_newAssistant->addHandle(m_selectedNode2);
                m_newAssistant->addHandle(assistant->bottomRight());
                m_dragEnd = event->point;
                m_handleDrag = 0;
                m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
                return;
            } else if (m_handleDrag == assistant->topMiddle()) {
                m_dragStart = QPointF((assistant->topLeft().data()->x()+assistant->topRight().data()->x())*0.5,
                                      (assistant->topLeft().data()->y()+assistant->topRight().data()->y())*0.5);
                m_internalMode = MODE_DRAGGING_TRANSLATING_TWONODES;
                m_selectedNode1 = new KisPaintingAssistantHandle(assistant->topLeft().data()->x(),assistant->topLeft().data()->y());
                m_selectedNode2 = new KisPaintingAssistantHandle(assistant->topRight().data()->x(),assistant->topRight().data()->y());
                m_newAssistant = KisPaintingAssistantFactoryRegistry::instance()->get("perspective")->createPaintingAssistant();
                m_newAssistant->addHandle(m_selectedNode1);
                m_newAssistant->addHandle(m_selectedNode2);
                m_newAssistant->addHandle(assistant->topRight());
                m_newAssistant->addHandle(assistant->topLeft());
                m_dragEnd = event->point;
                m_handleDrag = 0;
                m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
                return;
            } else if (m_handleDrag == assistant->bottomMiddle()) {
                m_dragStart = QPointF((assistant->bottomLeft().data()->x()+assistant->bottomRight().data()->x())*0.5,
                                      (assistant->bottomLeft().data()->y()+assistant->bottomRight().data()->y())*0.5);
                m_internalMode = MODE_DRAGGING_TRANSLATING_TWONODES;
                m_selectedNode1 = new KisPaintingAssistantHandle(assistant->bottomLeft().data()->x(),assistant->bottomLeft().data()->y());
                m_selectedNode2 = new KisPaintingAssistantHandle(assistant->bottomRight().data()->x(),assistant->bottomRight().data()->y());
                m_newAssistant = KisPaintingAssistantFactoryRegistry::instance()->get("perspective")->createPaintingAssistant();
                m_newAssistant->addHandle(assistant->bottomLeft());
                m_newAssistant->addHandle(assistant->bottomRight());
                m_newAssistant->addHandle(m_selectedNode2);
                m_newAssistant->addHandle(m_selectedNode1);
                m_dragEnd = event->point;
                m_handleDrag = 0;
                m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
                return;
            }
            m_snapIsRadial = false;
        } else if (m_handleDrag && assistant->handles().size()>1 && (assistant->id() == "ruler" ||
                                    assistant->id() == "parallel ruler" ||
                                    assistant->id() == "infinite ruler" ||
                                    assistant->id() == "spline")){
            if (m_handleDrag == assistant->handles()[0]) {
                m_dragStart = *assistant->handles()[1];
            } else if (m_handleDrag == assistant->handles()[1]) {
                m_dragStart = *assistant->handles()[0];
            } else if(assistant->handles().size()==4){
                if (m_handleDrag == assistant->handles()[2]) {
                    m_dragStart = *assistant->handles()[0];
                } else if (m_handleDrag == assistant->handles()[3]) {
                    m_dragStart = *assistant->handles()[1];
                }
            }
            m_snapIsRadial = false;
        } else if (m_handleDrag && assistant->handles().size()>2 && (assistant->id() == "ellipse" ||
                                    assistant->id() == "concentric ellipse" ||
                                    assistant->id() == "fisheye-point")){
            m_snapIsRadial = false;
            if (m_handleDrag == assistant->handles()[0]) {
                m_dragStart = *assistant->handles()[1];
            } else if (m_handleDrag == assistant->handles()[1]) {
                m_dragStart = *assistant->handles()[0];
            } else if (m_handleDrag == assistant->handles()[2]) {
                m_dragStart = assistant->buttonPosition();
                m_radius = QLineF(m_dragStart, *assistant->handles()[0]);
                m_snapIsRadial = true;
            }

        } else {
            m_dragStart = assistant->buttonPosition();
            m_snapIsRadial = false;
        }
    }

    if (m_handleDrag) {
        // TODO: Shift-press should now be handled using the alternate actions
        // if (event->modifiers() & Qt::ShiftModifier) {
        //     m_handleDrag->uncache();
        //     m_handleDrag = m_handleDrag->split()[0];
        //     m_handles = m_canvas->view()->paintingAssistantsDecoration()->handles();
        // }
        m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
        return;
    }

    m_assistantDrag = 0;
    Q_FOREACH (KisPaintingAssistant* assistant, m_canvas->paintingAssistantsDecoration()->assistants()) {

        // This code contains the click event behavior. The actual display of the icons are done at the bottom
        // of the paint even. Make sure the rectangles positions are the same between the two.

        // TODO: These 6 lines are duplicated below in the paint layer. It shouldn't be done like this.
        QPointF actionsPosition = m_canvas->viewConverter()->documentToView(assistant->buttonPosition());

        QPointF iconDeletePosition(actionsPosition + QPointF(78, m_assistantHelperYOffset + 7));
        QPointF iconSnapPosition(actionsPosition + QPointF(54, m_assistantHelperYOffset + 7));
        QPointF iconMovePosition(actionsPosition + QPointF(15, m_assistantHelperYOffset));


        QRectF deleteRect(iconDeletePosition, QSizeF(16, 16));
        QRectF visibleRect(iconSnapPosition, QSizeF(16, 16));
        QRectF moveRect(iconMovePosition, QSizeF(32, 32));

        if (moveRect.contains(mousePos)) {
            m_assistantDrag = assistant;
            m_mousePosition = event->point;
            m_internalMode = MODE_EDITING;
            return;
        }
        if (deleteRect.contains(mousePos)) {
            removeAssistant(assistant);
            if(m_canvas->paintingAssistantsDecoration()->assistants().isEmpty()) {
                m_internalMode = MODE_CREATION;
            }
            else
                m_internalMode = MODE_EDITING;
            m_canvas->updateCanvas();
            return;
        }
        if (visibleRect.contains(mousePos)) {
            newAssistantAllowed = false;
            if (assistant->snapping()==true){

            snappingOff(assistant);
            outlineOff(assistant);
            }
            else{
            snappingOn(assistant);
            outlineOn(assistant);
            }
            assistant->uncache();//this updates the chache of the assistant, very important.
        }
    }
    if (newAssistantAllowed==true){//don't make a new assistant when I'm just toogling visiblity//
        QString key = m_options.comboBox->model()->index( m_options.comboBox->currentIndex(), 0 ).data(Qt::UserRole).toString();
        m_newAssistant = KisPaintingAssistantFactoryRegistry::instance()->get(key)->createPaintingAssistant();
        m_internalMode = MODE_CREATION;
        m_newAssistant->addHandle(new KisPaintingAssistantHandle(event->point));
        if (m_newAssistant->numHandles() <= 1) {
            if (key == "vanishing point"){
                m_newAssistant->addSideHandle(new KisPaintingAssistantHandle(event->point+QPointF(-70,0)));
                m_newAssistant->addSideHandle(new KisPaintingAssistantHandle(event->point+QPointF(-140,0)));
                m_newAssistant->addSideHandle(new KisPaintingAssistantHandle(event->point+QPointF(70,0)));
                m_newAssistant->addSideHandle(new KisPaintingAssistantHandle(event->point+QPointF(140,0)));
                }
            addAssistant();
        } else {
            m_newAssistant->addHandle(new KisPaintingAssistantHandle(event->point));
        }
    }

    m_canvas->updateCanvas();
}

void KisRulerAssistantTool::continuePrimaryAction(KoPointerEvent *event)
{
    if (m_handleDrag) {
        *m_handleDrag = event->point;
        //ported from the gradient tool... we need to think about this more in the future.
        if (event->modifiers() == Qt::ShiftModifier && m_snapIsRadial) {
            QLineF dragRadius = QLineF(m_dragStart, event->point);
            dragRadius.setLength(m_radius.length());
            *m_handleDrag = dragRadius.p2();
        } else if (event->modifiers() == Qt::ShiftModifier ) {
            *m_handleDrag = straightLine(event->point, m_dragStart);
        } else {
            *m_handleDrag = event->point;
        }
        m_handleDrag->uncache();

        m_handleCombine = 0;
        if (!(event->modifiers() & Qt::ShiftModifier)) {
            double minDist = 49.0;
            QPointF mousePos = m_canvas->viewConverter()->documentToView(event->point);
            Q_FOREACH (const KisPaintingAssistantHandleSP handle, m_handles) {
                if (handle == m_handleDrag) continue;
                double dist = norm2(mousePos - m_canvas->viewConverter()->documentToView(*handle));
                if (dist < minDist) {
                    minDist = dist;
                    m_handleCombine = handle;
                }
            }
        }
        m_canvas->updateCanvas();
    } else if (m_assistantDrag) {
        QPointF adjust = event->point - m_mousePosition;
        Q_FOREACH (KisPaintingAssistantHandleSP handle, m_assistantDrag->handles()) {
            *handle += adjust;

        }
        if (m_assistantDrag->id()== "vanishing point"){
            Q_FOREACH (KisPaintingAssistantHandleSP handle, m_assistantDrag->sideHandles()) {
                *handle += adjust;

            }
        }
        m_mousePosition = event->point;
        m_canvas->updateCanvas();

    } else {
        event->ignore();
    }

    bool wasHiglightedNode = m_higlightedNode != 0;
    QPointF mousep = m_canvas->viewConverter()->documentToView(event->point);
    QList <KisPaintingAssistant*> pAssistant= m_canvas->paintingAssistantsDecoration()->assistants();
    Q_FOREACH (KisPaintingAssistant*  assistant, pAssistant) {
        if(assistant->id() == "perspective") {
            if ((m_higlightedNode = nodeNearPoint(assistant, mousep))) {
                if (m_higlightedNode == m_selectedNode1 || m_higlightedNode == m_selectedNode2) {
                    m_higlightedNode = 0;
                } else {
                    m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
                    break;
                }
            }
        }

        //this following bit sets the translations for the vanishing-point handles.
        if(m_handleDrag && assistant->id() == "vanishing point" && assistant->sideHandles().size()==4) {
            //for inner handles, the outer handle gets translated.
            if (m_handleDrag == assistant->sideHandles()[0]){
            QLineF perspectiveline = QLineF(*assistant->handles()[0], *assistant->sideHandles()[0]);

            qreal length = QLineF(*assistant->sideHandles()[0], *assistant->sideHandles()[1]).length();
            if (length<2.0){length=2.0;}
            length +=perspectiveline.length();
            perspectiveline.setLength(length);
            *assistant->sideHandles()[1] = perspectiveline.p2();
            }
            else if (m_handleDrag == assistant->sideHandles()[2]){
            QLineF perspectiveline = QLineF(*assistant->handles()[0], *assistant->sideHandles()[2]);

            qreal length = QLineF(*assistant->sideHandles()[2], *assistant->sideHandles()[3]).length();
            if (length<2.0){length=2.0;}
            length +=perspectiveline.length();
            perspectiveline.setLength(length);
            *assistant->sideHandles()[3] = perspectiveline.p2();
            }
            //for outer handles, only the vanishing point is translated, but only if there's an intersection.
            else if (m_handleDrag == assistant->sideHandles()[1]|| m_handleDrag == assistant->sideHandles()[3]){
            QPointF vanishingpoint(0,0);
            QLineF perspectiveline = QLineF(*assistant->sideHandles()[0], *assistant->sideHandles()[1]);
            QLineF perspectiveline2 = QLineF(*assistant->sideHandles()[2], *assistant->sideHandles()[3]);
            if (QLineF(perspectiveline2).intersect(QLineF(perspectiveline), &vanishingpoint) != QLineF::NoIntersection){
            *assistant->handles()[0] = vanishingpoint;}
            }//and for the vanishing point itself, only the outer handles get translated.
            else if (m_handleDrag == assistant->handles()[0]){
            QLineF perspectiveline = QLineF(*assistant->handles()[0], *assistant->sideHandles()[0]);
            QLineF perspectiveline2 = QLineF(*assistant->handles()[0], *assistant->sideHandles()[2]);
            qreal length =  QLineF(*assistant->sideHandles()[0], *assistant->sideHandles()[1]).length();
            qreal length2 = QLineF(*assistant->sideHandles()[2], *assistant->sideHandles()[3]).length();
            if (length<2.0){length=2.0;}
            if (length2<2.0){length2=2.0;}
            length +=perspectiveline.length();
            length2 +=perspectiveline2.length();
            perspectiveline.setLength(length);
            perspectiveline2.setLength(length2);
            *assistant->sideHandles()[1] = perspectiveline.p2();
            *assistant->sideHandles()[3] = perspectiveline2.p2();
            }

        }
    }
    if (wasHiglightedNode && !m_higlightedNode) {
        m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
    }
}

void KisRulerAssistantTool::endPrimaryAction(KoPointerEvent *event)
{
    setMode(KisTool::HOVER_MODE);

    if (m_handleDrag) {
        if (!(event->modifiers() & Qt::ShiftModifier) && m_handleCombine) {
            m_handleCombine->mergeWith(m_handleDrag);
            m_handleCombine->uncache();
            m_handles = m_canvas->paintingAssistantsDecoration()->handles();
        }
        m_handleDrag = m_handleCombine = 0;
        m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
    } else if (m_assistantDrag) {
        m_assistantDrag = 0;
        m_canvas->updateCanvas(); // TODO update only the relevant part of the canvas
    } else if(m_internalMode == MODE_DRAGGING_TRANSLATING_TWONODES) {
        addAssistant();
        m_internalMode = MODE_CREATION;
        m_canvas->updateCanvas();
    }
    else {
        event->ignore();
    }
}

void KisRulerAssistantTool::addAssistant()
{
    m_canvas->paintingAssistantsDecoration()->addAssistant(m_newAssistant);
    m_handles = m_canvas->paintingAssistantsDecoration()->handles();
    KisAbstractPerspectiveGrid* grid = dynamic_cast<KisAbstractPerspectiveGrid*>(m_newAssistant);
    if (grid) {
        m_canvas->viewManager()->resourceProvider()->addPerspectiveGrid(grid);
    }
    m_newAssistant = 0;
}


void KisRulerAssistantTool::removeAssistant(KisPaintingAssistant* assistant)
{
    KisAbstractPerspectiveGrid* grid = dynamic_cast<KisAbstractPerspectiveGrid*>(assistant);
    if (grid) {
        m_canvas->viewManager()->resourceProvider()->removePerspectiveGrid(grid);
    }
    m_canvas->paintingAssistantsDecoration()->removeAssistant(assistant);
    m_handles = m_canvas->paintingAssistantsDecoration()->handles();
}

void KisRulerAssistantTool::snappingOn(KisPaintingAssistant* assistant)
{
    assistant->setSnapping(true);
}

void KisRulerAssistantTool::snappingOff(KisPaintingAssistant* assistant)
{
    assistant->setSnapping(false);
}

void KisRulerAssistantTool::outlineOn(KisPaintingAssistant* assistant)
{
    assistant->setOutline(true);
}

void KisRulerAssistantTool::outlineOff(KisPaintingAssistant* assistant)
{
    assistant->setOutline(false);
}


void KisRulerAssistantTool::mouseMoveEvent(KoPointerEvent *event)
{
    if (m_newAssistant && m_internalMode == MODE_CREATION) {
        *m_newAssistant->handles().back() = event->point;
        m_canvas->updateCanvas();
    } else if (m_newAssistant && m_internalMode == MODE_DRAGGING_TRANSLATING_TWONODES) {
        QPointF translate = event->point - m_dragEnd;;
        m_dragEnd = event->point;
        m_selectedNode1.data()->operator =(QPointF(m_selectedNode1.data()->x(),m_selectedNode1.data()->y()) + translate);
        m_selectedNode2.data()->operator = (QPointF(m_selectedNode2.data()->x(),m_selectedNode2.data()->y()) + translate);
        m_canvas->updateCanvas();
    }
}

QPointF KisRulerAssistantTool::straightLine(QPointF point, QPointF compare)
{
    QPointF comparison = point - compare;
    QPointF result;

    if (fabs(comparison.x()) > fabs(comparison.y())) {
        result.setX(point.x());
        result.setY(compare.y());
    } else {
        result.setX(compare.x());
        result.setY(point.y());
    }

    return result;
}

void KisRulerAssistantTool::paint(QPainter& _gc, const KoViewConverter &_converter)
{

    QPixmap iconDelete = KisIconUtils::loadIcon("dialog-cancel").pixmap(16, 16);
    QPixmap iconSnapOn = KisIconUtils::loadIcon("visible").pixmap(16, 16);
    QPixmap iconSnapOff = KisIconUtils::loadIcon("novisible").pixmap(16, 16);
    QPixmap iconMove = KisIconUtils::loadIcon("transform-move").pixmap(32, 32);
    QColor handlesColor(0, 0, 0, 125);

    if (m_newAssistant) {
        m_newAssistant->drawAssistant(_gc, QRectF(QPointF(0, 0), QSizeF(m_canvas->image()->size())), m_canvas->coordinatesConverter(), false,m_canvas, true, false);
        Q_FOREACH (const KisPaintingAssistantHandleSP handle, m_newAssistant->handles()) {
            QPainterPath path;
            path.addEllipse(QRectF(_converter.documentToView(*handle) -  QPointF(6, 6), QSizeF(12, 12)));
            KisPaintingAssistant::drawPath(_gc, path);
        }
    }

    // TODO: too  many Q_FOREACH loops going through all assistants. Condense this to one to be a little more performant

    // render handles for the asssistant
    Q_FOREACH (KisPaintingAssistant* assistant, m_canvas->paintingAssistantsDecoration()->assistants()) {
        Q_FOREACH (const KisPaintingAssistantHandleSP handle, m_handles) {
            QRectF ellipse(_converter.documentToView(*handle) -  QPointF(6, 6), QSizeF(12, 12));

            // render handles when they are being dragged and moved
            if (handle == m_handleDrag || handle == m_handleCombine) {
                _gc.save();
                _gc.setPen(Qt::transparent);
                _gc.setBrush(handlesColor);
                _gc.drawEllipse(ellipse);
                _gc.restore();
            }

            if ( assistant->id() =="vanishing point") {

                if (assistant->handles().at(0) == handle )  { // vanishing point handle
                     ellipse = QRectF(_converter.documentToView(*handle) -  QPointF(10, 10), QSizeF(20, 20));
                     // TODO: change this to be smaller, but fill in with a color
                }

                //TODO: render outside handles a little bigger than rotation anchor handles
            }

            QPainterPath path;
            path.addEllipse(ellipse);
            KisPaintingAssistant::drawPath(_gc, path);

        }
    }



    Q_FOREACH (KisPaintingAssistant* assistant, m_canvas->paintingAssistantsDecoration()->assistants()) {
        if(assistant->id()=="perspective") {
            assistant->findHandleLocation();
            QPointF topMiddle, bottomMiddle, rightMiddle, leftMiddle;
            topMiddle = (_converter.documentToView(*assistant->topLeft()) + _converter.documentToView(*assistant->topRight()))*0.5;
            bottomMiddle = (_converter.documentToView(*assistant->bottomLeft()) + _converter.documentToView(*assistant->bottomRight()))*0.5;
            rightMiddle = (_converter.documentToView(*assistant->topRight()) + _converter.documentToView(*assistant->bottomRight()))*0.5;
            leftMiddle = (_converter.documentToView(*assistant->topLeft()) + _converter.documentToView(*assistant->bottomLeft()))*0.5;
            QPainterPath path;
            path.addEllipse(QRectF(leftMiddle-QPointF(6,6),QSizeF(12,12)));
            path.addEllipse(QRectF(topMiddle-QPointF(6,6),QSizeF(12,12)));
            path.addEllipse(QRectF(rightMiddle-QPointF(6,6),QSizeF(12,12)));
            path.addEllipse(QRectF(bottomMiddle-QPointF(6,6),QSizeF(12,12)));
            KisPaintingAssistant::drawPath(_gc, path);
        }
        if(assistant->id()=="vanishing point") {
            if (assistant->sideHandles().size() == 4) {
            // Draw the line
            QPointF p0 = _converter.documentToView(*assistant->handles()[0]);
            QPointF p1 = _converter.documentToView(*assistant->sideHandles()[0]);
            QPointF p2 = _converter.documentToView(*assistant->sideHandles()[1]);
            QPointF p3 = _converter.documentToView(*assistant->sideHandles()[2]);
            QPointF p4 = _converter.documentToView(*assistant->sideHandles()[3]);

            _gc.setPen(QColor(0, 0, 0, 75));
            // Draw control lines
            QPen penStyle(QColor(120, 120, 120, 60), 2.0, Qt::DashDotDotLine);
            _gc.setPen(penStyle);
            _gc.drawLine(p0, p1);
            _gc.drawLine(p0, p3);
            _gc.drawLine(p1, p2);
            _gc.drawLine(p3, p4);
            }
        }
    }


    Q_FOREACH (const KisPaintingAssistant* assistant, m_canvas->paintingAssistantsDecoration()->assistants()) {


       // We are going to put all of the assistant actions below the bounds of the assistant
       // so they are out of the way
        // assistant->buttonPosition() gets the center X/Y position point
        QPointF actionsPosition = m_canvas->viewConverter()->documentToView(assistant->buttonPosition());

        QPointF iconDeletePosition(actionsPosition + QPointF(78, m_assistantHelperYOffset + 7));
        QPointF iconSnapPosition(actionsPosition + QPointF(54, m_assistantHelperYOffset + 7));
        QPointF iconMovePosition(actionsPosition + QPointF(15, m_assistantHelperYOffset ));



        // Background container for helpers
        QBrush backgroundColor = m_canvas->viewManager()->mainWindow()->palette().window();
        QPointF actionsBGRectangle(actionsPosition + QPointF(25, m_assistantHelperYOffset));

        _gc.setRenderHint(QPainter::Antialiasing);

        QPainterPath bgPath;
        bgPath.addRoundedRect(QRectF(actionsBGRectangle.x(), actionsBGRectangle.y(), 80, 30), 6, 6);
        QPen stroke(QColor(60, 60, 60, 80), 2);
        _gc.setPen(stroke);
        _gc.fillPath(bgPath, backgroundColor);
        _gc.drawPath(bgPath);


        QPainterPath movePath;  // render circle behind by move helper
        _gc.setPen(stroke);
        movePath.addEllipse(iconMovePosition.x()-5, iconMovePosition.y()-5, 40, 40);// background behind icon
        _gc.fillPath(movePath, backgroundColor);
        _gc.drawPath(movePath);

        // Preview/Snap Tool helper
        _gc.drawPixmap(iconDeletePosition, iconDelete);
        if (assistant->snapping()==true) {
            _gc.drawPixmap(iconSnapPosition, iconSnapOn);
        }
        else
        {
            _gc.drawPixmap(iconSnapPosition, iconSnapOff);
        }


        // Move Assistant Tool helper
        _gc.drawPixmap(iconMovePosition, iconMove);


  }
}

void KisRulerAssistantTool::removeAllAssistants()
{
    m_canvas->viewManager()->resourceProvider()->clearPerspectiveGrids();
    m_canvas->paintingAssistantsDecoration()->removeAll();
    m_handles = m_canvas->paintingAssistantsDecoration()->handles();
    m_canvas->updateCanvas();
}

void KisRulerAssistantTool::loadAssistants()
{
    KoFileDialog dialog(m_canvas->viewManager()->mainWindow(), KoFileDialog::OpenFile, "OpenAssistant");
    dialog.setCaption(i18n("Select an Assistant"));
    dialog.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
    dialog.setNameFilter("Krita Assistant (*.krassistant)");
    QString filename = dialog.filename();
    if (filename.isEmpty()) return;
    if (!QFileInfo(filename).exists()) return;

    QFile file(filename);
    file.open(QIODevice::ReadOnly);

    QByteArray data = file.readAll();
    QXmlStreamReader xml(data);
    QMap<int, KisPaintingAssistantHandleSP> handleMap;
    KisPaintingAssistant* assistant = 0;
    bool errors = false;
    while (!xml.atEnd()) {
        switch (xml.readNext()) {
        case QXmlStreamReader::StartElement:
            if (xml.name() == "handle") {
                if (assistant && !xml.attributes().value("ref").isEmpty()) {
                    KisPaintingAssistantHandleSP handle = handleMap.value(xml.attributes().value("ref").toString().toInt());
                    if (handle) {
                       assistant->addHandle(handle);
                    } else {
                        errors = true;
                    }
                } else {
                    QString strId = xml.attributes().value("id").toString(),
                            strX = xml.attributes().value("x").toString(),
                            strY = xml.attributes().value("y").toString();
                    if (!strId.isEmpty() && !strX.isEmpty() && !strY.isEmpty()) {
                        int id = strId.toInt();
                        double x = strX.toDouble(),
                                y = strY.toDouble();
                        if (!handleMap.contains(id)) {
                            handleMap.insert(id, new KisPaintingAssistantHandle(x, y));
                        } else {
                            errors = true;
                        }
                    } else {
                        errors = true;
                    }
                }
            } else if (xml.name() == "assistant") {
                const KisPaintingAssistantFactory* factory = KisPaintingAssistantFactoryRegistry::instance()->get(xml.attributes().value("type").toString());
                if (factory) {
                    if (assistant) {
                        errors = true;
                        delete assistant;
                    }
                    assistant = factory->createPaintingAssistant();
                } else {
                    errors = true;
                }
            }
            break;
        case QXmlStreamReader::EndElement:
            if (xml.name() == "assistant") {
                if (assistant) {
                    if (assistant->handles().size() == assistant->numHandles()) {
                        if (assistant->id() == "vanishing point"){
                        //ideally we'd save and load side-handles as well, but this is all I've got//
                            QPointF pos = *assistant->handles()[0];
                            assistant->addSideHandle(new KisPaintingAssistantHandle(pos+QPointF(-70,0)));
                            assistant->addSideHandle(new KisPaintingAssistantHandle(pos+QPointF(-140,0)));
                            assistant->addSideHandle(new KisPaintingAssistantHandle(pos+QPointF(70,0)));
                            assistant->addSideHandle(new KisPaintingAssistantHandle(pos+QPointF(140,0)));
                        }
                        m_canvas->paintingAssistantsDecoration()->addAssistant(assistant);
                        KisAbstractPerspectiveGrid* grid = dynamic_cast<KisAbstractPerspectiveGrid*>(assistant);
                        if (grid) {
                            m_canvas->viewManager()->resourceProvider()->addPerspectiveGrid(grid);
                        }
                    } else {
                        errors = true;
                        delete assistant;
                    }
                    assistant = 0;
                }
            }
            break;
        default:
            break;
        }
    }
    if (assistant) {
        errors = true;
        delete assistant;
    }
    if (xml.hasError()) {
        QMessageBox::warning(0, i18nc("@title:window", "Krita"), xml.errorString());
    }
    if (errors) {
        QMessageBox::warning(0, i18nc("@title:window", "Krita"), i18n("Errors were encountered. Not all assistants were successfully loaded."));
    }
    m_handles = m_canvas->paintingAssistantsDecoration()->handles();
    m_canvas->updateCanvas();

}

void KisRulerAssistantTool::saveAssistants()
{
    if (m_handles.isEmpty()) return;

    QByteArray data;
    QXmlStreamWriter xml(&data);
    xml.writeStartDocument();
    xml.writeStartElement("paintingassistant");
    xml.writeStartElement("handles");
    QMap<KisPaintingAssistantHandleSP, int> handleMap;
    Q_FOREACH (const KisPaintingAssistantHandleSP handle, m_handles) {
        int id = handleMap.size();
        handleMap.insert(handle, id);
        xml.writeStartElement("handle");
        //xml.writeAttribute("type", handle->handleType());
        xml.writeAttribute("id", QString::number(id));
        xml.writeAttribute("x", QString::number(double(handle->x()), 'f', 3));
        xml.writeAttribute("y", QString::number(double(handle->y()), 'f', 3));
        xml.writeEndElement();
    }
    xml.writeEndElement();
    xml.writeStartElement("assistants");
    Q_FOREACH (const KisPaintingAssistant* assistant, m_canvas->paintingAssistantsDecoration()->assistants()) {
        xml.writeStartElement("assistant");
        xml.writeAttribute("type", assistant->id());
        xml.writeStartElement("handles");
        Q_FOREACH (const KisPaintingAssistantHandleSP handle, assistant->handles()) {
            xml.writeStartElement("handle");
            xml.writeAttribute("ref", QString::number(handleMap.value(handle)));
            xml.writeEndElement();
        }
        xml.writeEndElement();
        xml.writeEndElement();
    }
    xml.writeEndElement();
    xml.writeEndElement();
    xml.writeEndDocument();

    KoFileDialog dialog(m_canvas->viewManager()->mainWindow(), KoFileDialog::SaveFile, "OpenAssistant");
    dialog.setCaption(i18n("Save Assistant"));
    dialog.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
    dialog.setNameFilter("Krita Assistant (*.krassistant)");
    QString filename = dialog.filename();
    if (filename.isEmpty()) return;

    QFile file(filename);
    file.open(QIODevice::WriteOnly);
    file.write(data);
}



QWidget *KisRulerAssistantTool::createOptionWidget()
{
    if (!m_optionsWidget) {
        m_optionsWidget = new QWidget;
        m_options.setupUi(m_optionsWidget);

        // See https://bugs.kde.org/show_bug.cgi?id=316896
        QWidget *specialSpacer = new QWidget(m_optionsWidget);
        specialSpacer->setObjectName("SpecialSpacer");
        specialSpacer->setFixedSize(0, 0);
        m_optionsWidget->layout()->addWidget(specialSpacer);

        m_options.loadButton->setIcon(KisIconUtils::loadIcon("document-open"));
        m_options.saveButton->setIcon(KisIconUtils::loadIcon("document-save"));
        m_options.deleteButton->setIcon(KisIconUtils::loadIcon("edit-delete"));
        Q_FOREACH (const QString& key, KisPaintingAssistantFactoryRegistry::instance()->keys()) {
            QString name = KisPaintingAssistantFactoryRegistry::instance()->get(key)->name();
            m_options.comboBox->addItem(name, key);
        }
        connect(m_options.saveButton, SIGNAL(clicked()), SLOT(saveAssistants()));
        connect(m_options.loadButton, SIGNAL(clicked()), SLOT(loadAssistants()));
        connect(m_options.deleteButton, SIGNAL(clicked()), SLOT(removeAllAssistants()));
    }
    return m_optionsWidget;
}

