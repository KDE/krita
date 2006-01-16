/*
 *  kis_tool_transform.cc -- part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Casper Boemann <cbr@boemann.dk>
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


#include <qpainter.h>
#include <qpen.h>
#include <qpushbutton.h>
#include <qobject.h>
#include <qcombobox.h>
#include <qapplication.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>
#include <knuminput.h>

#include <kis_global.h>
#include <kis_painter.h>
#include <kis_canvas_controller.h>
#include <kis_canvas_subject.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <kis_undo_adapter.h>
#include <kis_selected_transaction.h>
#include <kis_button_press_event.h>
#include <kis_button_release_event.h>
#include <kis_move_event.h>
#include <kis_selection.h>
#include <kis_filter_strategy.h>
#include <kis_cmb_idlist.h>
#include <kis_id.h>
#include <kis_tool_controller.h>
#include <kis_transform_visitor.h>

#include "kis_tool_transform.h"
#include "wdg_tool_transform.h"
#include "kis_canvas.h"
#include "kis_canvas_painter.h"

namespace {
    class TransformCmd : public KisSelectedTransaction {
        typedef KisSelectedTransaction super;

    public:
        TransformCmd(KisPaintDeviceImplSP device);
        virtual ~TransformCmd();

    public:
        virtual void execute();
        virtual void unexecute();
    };

    TransformCmd::TransformCmd(KisPaintDeviceImplSP device) : super(i18n("Transform"), device)
    {
    }

    TransformCmd::~TransformCmd()
    {
    }

    void TransformCmd::execute()
    {
        super::execute();
    }

    void TransformCmd::unexecute()
    {
        super::unexecute();
    }
}

KisToolTransform::KisToolTransform()
    : super("Transform")
{
    setName("tool_transform");
    setCursor(KisCursor::selectCursor());
    m_subject = 0;
    m_selecting = false;
    m_startPos = QPoint(0, 0);
    m_endPos = QPoint(0, 0);
    m_optWidget = 0;
    m_transaction = 0;
    m_sizeCursors[0] = KisCursor::sizeVerCursor();
    m_sizeCursors[1] = KisCursor::sizeBDiagCursor();
    m_sizeCursors[2] = KisCursor::sizeHorCursor();
    m_sizeCursors[3] = KisCursor::sizeFDiagCursor();
    m_sizeCursors[4] = KisCursor::sizeVerCursor();
    m_sizeCursors[5] = KisCursor::sizeBDiagCursor();
    m_sizeCursors[6] = KisCursor::sizeHorCursor();
    m_sizeCursors[7] = KisCursor::sizeFDiagCursor();
}

KisToolTransform::~KisToolTransform()
{
    delete m_transaction;
}

void KisToolTransform::clear()
{
    KisImageSP img = m_subject -> currentImg();

    if (!img)
        return;

    paintOutline();

    if (m_transaction && img -> undoAdapter()) {
        img -> undoAdapter() -> addCommand(m_transaction);
    }
    else
        delete m_transaction;
    m_transaction=0;
}

void KisToolTransform::activate()
{
    if(m_subject)
    {
        KisToolControllerInterface *controller = m_subject -> toolController();

        if (controller)
            controller->setCurrentTool(this);
            
        Q_INT32 x,y,w,h;
        KisImageSP img = m_subject -> currentImg();
        KisPaintDeviceImplSP dev = img -> activeDevice();
        if(dev->hasSelection())
        {
            KisSelectionSP sel = dev->selection();
            QRect r = sel->selectedExactRect();
            r.rect(&x, &y, &w, &h);
        }
        else
            dev->exactBounds(x,y,w,h);
        
        m_startPos = QPoint(x, y);
        m_endPos = QPoint(x+w-1, y+h-1);
        m_org_cenX = (m_startPos.x() + m_endPos.x()) / 2.0;
        m_org_cenY = (m_startPos.y() + m_endPos.y()) / 2.0;

        m_a = 0.0;
        m_scaleX = 1.0;
        m_scaleY = 1.0;
        m_translateX = m_org_cenX;
        m_translateY = m_org_cenY;
        
        paintOutline();
    }
}

void KisToolTransform::paint(KisCanvasPainter& gc)
{
    paintOutline(gc, QRect());
}

void KisToolTransform::paint(KisCanvasPainter& gc, const QRect& rc)
{
    paintOutline(gc, rc);
}


void KisToolTransform::buttonPress(KisButtonPressEvent *e)
{
    if (m_subject) {
        KisImageSP img = m_subject -> currentImg();

        if (img && img -> activeDevice() && e -> button() == LeftButton) {
            switch(m_function)
            {
                case ROTATE:
                    m_clickoffset = e -> pos().floorQPoint() 
                        - QPoint(static_cast<int>(m_translateX),static_cast<int>(m_translateY));
                    m_clickangle = -m_a - atan2(m_clickoffset.x(),m_clickoffset.y());
                    m_clickoffset = QPoint(0, 0);
                    break;
                case MOVE:
                    m_clickoffset = e -> pos().floorQPoint() 
                        - QPoint(static_cast<int>(m_translateX),static_cast<int>(m_translateY));
                    break;
                case TOPSCALE:
                    m_clickoffset = e -> pos().floorQPoint() 
                            - QPoint((m_topleft + m_topright)/2);
                    break;
                case TOPRIGHTSCALE:
                    m_clickoffset = e -> pos().floorQPoint() - m_topright;
                    break;
                case RIGHTSCALE:
                    m_clickoffset = e -> pos().floorQPoint() 
                            - QPoint((m_topright + m_bottomright)/2);
                    break;
                case BOTTOMRIGHTSCALE:
                    m_clickoffset = e -> pos().floorQPoint() - m_bottomright;
                    break;
                case BOTTOMSCALE:
                    m_clickoffset = e -> pos().floorQPoint() 
                            - QPoint((m_bottomleft + m_bottomright)/2);
                    break;
                case BOTTOMLEFTSCALE:
                    m_clickoffset = e -> pos().floorQPoint() - m_bottomleft;
                    break;
                case LEFTSCALE:
                    m_clickoffset = e -> pos().floorQPoint() 
                            - QPoint((m_topleft + m_bottomleft)/2);
                    break;
                case TOPLEFTSCALE:
                    m_clickoffset = e -> pos().floorQPoint() - m_topleft;
                    break;
            }
            m_selecting = true;
        }
    }
}

int KisToolTransform::det(QPoint v,QPoint w)
{
    return v.x()*w.y()-v.y()*w.x();
}
int KisToolTransform::distsq(QPoint v,QPoint w)
{
    v -= w;
    return v.x()*v.x() + v.y()*v.y();
}

void KisToolTransform::setFunctionalCursor()
{
    int rotOctant = 8 + int(8.5 + m_a* 4 / M_PI);
    
    int s;
    if(m_scaleX*m_scaleY<0)
        s = -1;
    else
        s=1;
        
    switch(m_function)
    {
        case MOVE:
            setCursor(KisCursor::moveCursor());
            break;
        case ROTATE:
            setCursor(KisCursor::rotateCursor());
            break;
        case TOPSCALE:
            setCursor(m_sizeCursors[(0*s +rotOctant)%8]);
            break;
        case TOPRIGHTSCALE:
            setCursor(m_sizeCursors[(1*s +rotOctant)%8]);
            break;
        case RIGHTSCALE:
            setCursor(m_sizeCursors[(2*s +rotOctant)%8]);
            break;
        case BOTTOMRIGHTSCALE:
            setCursor(m_sizeCursors[(3*s +rotOctant)%8]);
            break;
        case BOTTOMSCALE:
            setCursor(m_sizeCursors[(4*s +rotOctant)%8]);
            break;
        case BOTTOMLEFTSCALE:
            setCursor(m_sizeCursors[(5*s +rotOctant)%8]);
            break;
        case LEFTSCALE:
            setCursor(m_sizeCursors[(6*s +rotOctant)%8]);
            break;
        case TOPLEFTSCALE:
            setCursor(m_sizeCursors[(7*s +rotOctant)%8]);
            break;
    }
}

void KisToolTransform::move(KisMoveEvent *e)
{
    if (m_subject) {
        KisCanvasController *controller = m_subject -> canvasController();

        Q_ASSERT(controller);
        QPoint topleft = m_topleft;
        QPoint topright = m_topright;
        QPoint bottomleft = m_bottomleft;
        QPoint bottomright = m_bottomright;
        
        QPoint mousePos = e -> pos().floorQPoint();
        
        if (m_subject && m_selecting) {
            paintOutline();
            
            mousePos -= m_clickoffset;
            
            // transform mousePos coords, so it seems like it isn't rotated and centered at 0,0
            double newX = invrotX(mousePos.x() - m_translateX, mousePos.y() - m_translateY);
            double newY = invrotY(mousePos.x() - m_translateX, mousePos.y() - m_translateY);
            double dx=0, dy=0;
            double oldScaleX = m_scaleX;
            double oldScaleY = m_scaleY;
            
            if(m_function == MOVE)
            {
                m_translateX += newX;
                m_translateY += newY;
            }

            if(m_function == ROTATE)
            {
                m_a = -atan2(mousePos.x() - m_translateX, mousePos.y() - m_translateY)
                    - m_clickangle;
            }
            
            if(m_function == TOPSCALE)
            {
                dy = (newY - m_scaleY * (m_startPos.y() - m_org_cenY)) / 2;
                m_scaleY = (newY - dy) / (m_startPos.y() - m_org_cenY);
                
                // enforce same acpect if shift button is pressed
                if(e->state() & Qt::ShiftButton)
                {
                    if(m_scaleX>0) // handle the mirrored cases
                        m_scaleX = fabs(m_scaleY);
                    else
                        m_scaleX = -fabs(m_scaleY);
                }
            }
            
            if(m_function == TOPRIGHTSCALE)
            {
                dx = (newX - m_scaleX * (m_endPos.x() - m_org_cenX)) / 2;
                m_scaleX = (newX - dx) / (m_endPos.x() - m_org_cenX);
                
                dy = (newY - m_scaleY * (m_startPos.y() - m_org_cenY)) / 2;
                m_scaleY = (newY - dy) / (m_startPos.y() - m_org_cenY);
                
                // enforce same acpect if shift button is pressed
                if(e->state() & Qt::ShiftButton)
                {
                    if(m_scaleX < m_scaleY)
                    {
                        if(m_scaleX>0) // handle the mirrored cases
                            m_scaleX = fabs(m_scaleY);
                        else
                            m_scaleX = -fabs(m_scaleY);
                        dx = (m_scaleX - oldScaleX) * (m_endPos.x() - m_org_cenX);
                    }
                    else
                    {
                        if(m_scaleY>0) // handle the mirrored cases
                            m_scaleY = fabs(m_scaleX);
                        else
                            m_scaleY = -fabs(m_scaleX);
                        dy = (m_scaleY - oldScaleY) * (m_startPos.y() - m_org_cenY);
                    }
                }
            }
            
            if(m_function == RIGHTSCALE)
            {
                dx = (newX - m_scaleX * (m_endPos.x() - m_org_cenX)) / 2;
                m_scaleX = (newX - dx) / (m_endPos.x() - m_org_cenX);
                
                // enforce same acpect if shift button is pressed
                if(e->state() & Qt::ShiftButton)
                {
                    if(m_scaleY>0) // handle the mirrored cases
                        m_scaleY = fabs(m_scaleX);
                    else
                        m_scaleY = -fabs(m_scaleX);
                }
            }
            
            if(m_function == BOTTOMRIGHTSCALE)
            {
                dx = (newX - m_scaleX * (m_endPos.x() - m_org_cenX)) / 2;
                m_scaleX = (newX - dx) / (m_endPos.x() - m_org_cenX);
                
                dy = (newY - m_scaleY * (m_endPos.y() - m_org_cenY)) / 2;
                m_scaleY = (newY - dy) / (m_endPos.y() - m_org_cenY);
                
                // enforce same acpect if shift button is pressed
                if(e->state() & Qt::ShiftButton)
                {
                    if(m_scaleX < m_scaleY)
                    {
                        if(m_scaleX>0) // handle the mirrored cases
                            m_scaleX = fabs(m_scaleY);
                        else
                            m_scaleX = -fabs(m_scaleY);
                        dx = (m_scaleX - oldScaleX) * (m_endPos.x() - m_org_cenX);
                    }
                    else
                    {
                        if(m_scaleY>0) // handle the mirrored cases
                            m_scaleY = fabs(m_scaleX);
                        else
                            m_scaleY = -fabs(m_scaleX);
                        dy = (m_scaleY - oldScaleY) * (m_endPos.y() - m_org_cenY);
                    }
                }
            }
            
            if(m_function == BOTTOMSCALE)
            {
                dy = (newY - m_scaleY * (m_endPos.y() - m_org_cenY)) / 2;
                m_scaleY = (newY - dy) / (m_endPos.y() - m_org_cenY);
                
                // enforce same acpect if shift button is pressed
                if(e->state() & Qt::ShiftButton)
                {
                    if(m_scaleX>0) // handle the mirrored cases
                        m_scaleX = fabs(m_scaleY);
                    else
                        m_scaleX = -fabs(m_scaleY);
                }
            }
            
            if(m_function == BOTTOMLEFTSCALE)
            {
                dx = (newX - m_scaleX * (m_startPos.x() - m_org_cenX)) / 2;
                m_scaleX = (newX - dx) / (m_startPos.x() - m_org_cenX);
                
                dy = (newY - m_scaleY * (m_endPos.y() - m_org_cenY)) / 2;
                m_scaleY = (newY - dy) / (m_endPos.y() - m_org_cenY);
                
                // enforce same acpect if shift button is pressed
                if(e->state() & Qt::ShiftButton)
                {
                    if(m_scaleX < m_scaleY)
                    {
                        if(m_scaleX>0) // handle the mirrored cases
                            m_scaleX = fabs(m_scaleY);
                        else
                            m_scaleX = -fabs(m_scaleY);
                        dx = (m_scaleX - oldScaleX) * (m_startPos.x() - m_org_cenX);
                    }
                    else
                    {
                        if(m_scaleY>0) // handle the mirrored cases
                            m_scaleY = fabs(m_scaleX);
                        else
                            m_scaleY = -fabs(m_scaleX);
                        dy = (m_scaleY - oldScaleY) * (m_endPos.y() - m_org_cenY);
                    }
                }
            }
            
            if(m_function == LEFTSCALE)
            {
                dx = (newX - m_scaleX * (m_startPos.x() - m_org_cenX)) / 2;
                m_scaleX = (newX - dx) / (m_startPos.x() - m_org_cenX);
                
                // enforce same acpect if shift button is pressed
                if(e->state() & Qt::ShiftButton)
                {
                    if(m_scaleY>0) // handle the mirrored cases
                        m_scaleY = fabs(m_scaleX);
                    else
                        m_scaleY = -fabs(m_scaleX);
                }
            }
            
            if(m_function == TOPLEFTSCALE)
            {
                dx = (newX - m_scaleX * (m_startPos.x() - m_org_cenX)) / 2;
                m_scaleX = (newX - dx) / (m_startPos.x() - m_org_cenX);
                
                dy = (newY - m_scaleY * (m_startPos.y() - m_org_cenY)) / 2;
                m_scaleY = (newY - dy) / (m_startPos.y() - m_org_cenY);
                
                // enforce same acpect if shift button is pressed
                if(e->state() & Qt::ShiftButton)
                {
                    if(m_scaleX < m_scaleY)
                    {
                        if(m_scaleX>0) // handle the mirrored cases
                            m_scaleX = fabs(m_scaleY);
                        else
                            m_scaleX = -fabs(m_scaleY);
                        dx = (m_scaleX - oldScaleX) * (m_startPos.x() - m_org_cenX);
                    }
                    else
                    {
                        if(m_scaleY>0) // handle the mirrored cases
                            m_scaleY = fabs(m_scaleX);
                        else
                            m_scaleY = -fabs(m_scaleX);
                        dy = (m_scaleY - oldScaleY) * (m_startPos.y() - m_org_cenY);
                    }
                }
            }
            
            m_translateX += rotX(dx, dy);
            m_translateY += rotY(dx, dy);
            
            paintOutline();
        }
        else
        {
            if(det(mousePos - topleft, topright - topleft)>0)
                m_function = ROTATE;
            else if(det(mousePos - topright, bottomright - topright)>0)
                m_function = ROTATE;
            else if(det(mousePos - bottomright, bottomleft - bottomright)>0)
                m_function = ROTATE;
            else if(det(mousePos - bottomleft, topleft - bottomleft)>0)
                m_function = ROTATE;
            else
                m_function = MOVE;
            
            if(distsq(mousePos, (m_topleft + m_topright)/2)<25)
                m_function = TOPSCALE;
            if(distsq(mousePos, m_topright)<25)
                m_function = TOPRIGHTSCALE;
            if(distsq(mousePos, (m_topright + m_bottomright)/2)<25)
                m_function = RIGHTSCALE;
            if(distsq(mousePos, m_bottomright)<25)
                m_function = BOTTOMRIGHTSCALE;
            if(distsq(mousePos, (m_bottomleft + m_bottomright)/2)<25)
                m_function = BOTTOMSCALE;
            if(distsq(mousePos, m_bottomleft)<25)
                m_function = BOTTOMLEFTSCALE;
            if(distsq(mousePos, (m_topleft + m_bottomleft)/2)<25)
                m_function = LEFTSCALE;
            if(distsq(mousePos, m_topleft)<25)
                m_function = TOPLEFTSCALE;
            
            setFunctionalCursor();
        }
    }
}

void KisToolTransform::buttonRelease(KisButtonReleaseEvent */*e*/)
{
    KisImageSP img = m_subject -> currentImg();

    if (!img)
        return;

    if (m_subject && m_selecting) {
        m_selecting = false;
    }
    QApplication::setOverrideCursor(KisCursor::waitCursor());
    transform();
    QApplication::restoreOverrideCursor();
}

void KisToolTransform::paintOutline()
{
    if (m_subject) {
        KisCanvasController *controller = m_subject -> canvasController();
        KisCanvas *canvas = controller -> kiscanvas();
        KisCanvasPainter gc(canvas);
        QRect rc;

        paintOutline(gc, rc);
    }
}

void KisToolTransform::recalcOutline()
{
    double x,y;
    
    m_sina = sin(m_a);
    m_cosa = cos(m_a);
    
    x = (m_startPos.x() - m_org_cenX) * m_scaleX;
    y = (m_startPos.y() - m_org_cenY) * m_scaleY;
    m_topleft = QPoint(int(rotX(x,y) + m_translateX), int(rotY(x,y) + m_translateY));
    
    x = (m_endPos.x() - m_org_cenX) * m_scaleX;
    y = (m_startPos.y() - m_org_cenY) * m_scaleY;
    m_topright = QPoint(int(rotX(x,y) + m_translateX), int(rotY(x,y) + m_translateY));
    
    x = (m_startPos.x() - m_org_cenX) * m_scaleX;
    y = (m_endPos.y() - m_org_cenY) * m_scaleY;
    m_bottomleft = QPoint(int(rotX(x,y) + m_translateX), int(rotY(x,y) + m_translateY));
    
    x = (m_endPos.x() - m_org_cenX) * m_scaleX;
    y = (m_endPos.y() - m_org_cenY) * m_scaleY;
    m_bottomright = QPoint(int(rotX(x,y) + m_translateX), int(rotY(x,y) + m_translateY));
}

void KisToolTransform::paintOutline(KisCanvasPainter& gc, const QRect&)
{
    if (m_subject) {
        KisCanvasController *controller = m_subject -> canvasController();
        RasterOp op = gc.rasterOp();
        QPen old = gc.pen();
        QPen pen(Qt::SolidLine);
        pen.setWidth(1);
        Q_ASSERT(controller);

        recalcOutline();
        QPoint topleft = controller -> windowToView(m_topleft);
        QPoint topright = controller -> windowToView(m_topright);
        QPoint bottomleft = controller -> windowToView(m_bottomleft);
        QPoint bottomright = controller -> windowToView(m_bottomright);

        gc.setRasterOp(Qt::NotROP);
        gc.setPen(pen);
        gc.drawRect(topleft.x()-4, topleft.y()-4, 8, 8);
        gc.drawLine(topleft.x(), topleft.y(), (topleft.x()+topright.x())/2, (topleft.y()+topright.y())/2);
        gc.drawRect((topleft.x()+topright.x())/2-4, (topleft.y()+topright.y())/2-4, 8, 8);
        gc.drawLine((topleft.x()+topright.x())/2, (topleft.y()+topright.y())/2, topright.x(), topright.y());
        gc.drawRect(topright.x()-4, topright.y()-4, 8, 8);
        gc.drawLine(topright.x(), topright.y(), (topright.x()+bottomright.x())/2, (topright.y()+bottomright.y())/2);
        gc.drawRect((topright.x()+bottomright.x())/2-4, (topright.y()+bottomright.y())/2-4, 8, 8);
        gc.drawLine((topright.x()+bottomright.x())/2, (topright.y()+bottomright.y())/2,bottomright.x(), bottomright.y());
        gc.drawRect(bottomright.x()-4, bottomright.y()-4, 8, 8);
        gc.drawLine(bottomright.x(), bottomright.y(), (bottomleft.x()+bottomright.x())/2, (bottomleft.y()+bottomright.y())/2);
        gc.drawRect((bottomleft.x()+bottomright.x())/2-4, (bottomleft.y()+bottomright.y())/2-4, 8, 8);
        gc.drawLine((bottomleft.x()+bottomright.x())/2, (bottomleft.y()+bottomright.y())/2, bottomleft.x(), bottomleft.y());
        gc.drawRect(bottomleft.x()-4, bottomleft.y()-4, 8, 8);
        gc.drawLine(bottomleft.x(), bottomleft.y(), (topleft.x()+bottomleft.x())/2, (topleft.y()+bottomleft.y())/2);
        gc.drawRect((topleft.x()+bottomleft.x())/2-4, (topleft.y()+bottomleft.y())/2-4, 8, 8);
        gc.drawLine((topleft.x()+bottomleft.x())/2, (topleft.y()+bottomleft.y())/2, topleft.x(), topleft.y());
        gc.setRasterOp(op);
        gc.setPen(old);
    }
}

void KisToolTransform::transform() {
    KisImageSP img = m_subject -> currentImg();

    if (!img)
        return;
/*
    double tx = m_translateX - m_org_cenX * m_scaleX;
    double ty = m_translateY - m_org_cenY * m_scaleY;
    */
    
    double tx = m_translateX - rotX(m_org_cenX * m_scaleX, m_org_cenY * m_scaleY);
    double ty = m_translateY - rotY(m_org_cenX * m_scaleX, m_org_cenY * m_scaleY);
    KisProgressDisplayInterface *progress = m_subject->progressDisplay();
printf("%f %f\n",tx,ty);
    if(m_transaction)
    {
        m_transaction->unexecuteNoUpdate();
        delete m_transaction;
    }    
    m_transaction = new TransformCmd(img->activeDevice().data());
    Q_CHECK_PTR(m_transaction);
    
    KisTransformVisitor t(m_scaleX, m_scaleY, 0, 0, m_a, int(tx), int(ty), progress, m_filter);
    KisPainter gc(img->activeDevice());
    t.visit(gc, img->activeDevice());
    
    if(t.isCanceled())
    {
        m_transaction->unexecute();
        delete m_transaction;
        m_transaction = 0;
        activate();
        return;
    }
    QRect rc = img->activeDevice()->extent();
    rc = rc.normalize();
    
    img -> notify(rc);
}

void KisToolTransform::slotSetFilter(const KisID &filterID)
{
    m_filter = KisFilterStrategyRegistry::instance() -> get(filterID);
}

QWidget* KisToolTransform::createOptionWidget(QWidget* parent)
{

    m_optWidget = new WdgToolTransform(parent);
    Q_CHECK_PTR(m_optWidget);
    
    m_optWidget -> cmbFilter -> clear();
    m_optWidget -> cmbFilter -> setIDList(KisFilterStrategyRegistry::instance() -> listKeys());

    m_optWidget -> cmbFilter -> setCurrentText("Mitchell");
    connect(m_optWidget -> cmbFilter, SIGNAL(activated(const KisID &)),
        this, SLOT(slotSetFilter(const KisID &)));

    KisID filterID = m_optWidget -> cmbFilter -> currentItem();
    m_filter = KisFilterStrategyRegistry::instance() -> get(filterID);
    
/*    connect(m_optWidget -> bnCrop, SIGNAL(clicked()), this, SLOT(crop()));

    connect(m_optWidget -> intStartX, SIGNAL(valueChanged(int)), this, SLOT(setStartX(int)));
    connect(m_optWidget -> intStartY, SIGNAL(valueChanged(int)), this, SLOT(setStartY(int)));
    connect(m_optWidget -> intEndX, SIGNAL(valueChanged(int)), this, SLOT(setEndX(int)));
    connect(m_optWidget -> intEndY, SIGNAL(valueChanged(int)), this, SLOT(setEndY(int)));
*/
    return m_optWidget;
}

QWidget* KisToolTransform::optionWidget()
{
    return m_optWidget;
}

void KisToolTransform::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection -> action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("&Transform"), 
                        "transform", 
                        0, 
                        this,
                        SLOT(activate()), 
                        collection, 
                        name());
        Q_CHECK_PTR(m_action);
        m_action -> setToolTip(i18n("Transform a layer or a selection"));
        m_action -> setExclusiveGroup("tools");
        m_ownAction = true;
    }
}

#include "kis_tool_transform.moc"
