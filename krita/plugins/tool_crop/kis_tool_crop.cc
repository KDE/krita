/*
 *  kis_tool_crop.cc -- part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
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
#include <qrect.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>
#include <knuminput.h>
#include <kdebug.h>

#include <kis_global.h>
#include <kis_painter.h>
#include <kis_canvas_controller.h>
#include <kis_canvas_subject.h>
#include <kis_cursor.h>
#include <kis_image.h>
#include <kis_undo_adapter.h>
#include <kis_button_press_event.h>
#include <kis_button_release_event.h>
#include <kis_move_event.h>
#include <kis_selected_transaction.h>
#include <kis_selection.h>

#include "kis_tool_crop.h"
#include "wdg_tool_crop.h"


KisToolCrop::KisToolCrop()
{
    setName("tool_crop");
    setCursor(KisCursor::selectCursor());
    m_subject = 0;
    m_selecting = false;
    m_startPos = QPoint(0, 0);
    m_endPos = QPoint(0, 0);
        m_handleSize = 13;
        m_haveCropSelection = false;
    m_optWidget = 0;
}

KisToolCrop::~KisToolCrop()
{
}

void KisToolCrop::update(KisCanvasSubject *subject)
{
    m_subject = subject;
    super::update(m_subject);
}

void KisToolCrop::activate()
{
    super::activate();

//    if ( (m_startPos - m_endPos) != QPoint(0,0) )
//        return;
    // No current crop rectangle, try to use the selection of the device to make a rectangle
    if (m_subject && m_subject -> currentImg() && m_subject -> currentImg() -> activeDevice()) {
        KisPaintDeviceImplSP device = m_subject -> currentImg() -> activeDevice();
        if (!device -> hasSelection())
            return;

        QRect extent = device -> selection() -> exactBounds();
        m_startPos = extent.topLeft();
        m_endPos = extent.bottomRight();
        validateSelection();
        //paintOutlineWithHandles();
        crop();
    }
}

void KisToolCrop::paint(QPainter& gc)
{
    paintOutlineWithHandles(gc, QRect());
}

void KisToolCrop::paint(QPainter& gc, const QRect& rc)
{
    paintOutlineWithHandles(gc, rc);
}

void KisToolCrop::clearRect()
{
    if (m_subject) {
        KisCanvasControllerInterface *controller = m_subject -> canvasController();
        KisImageSP img = m_subject -> currentImg();

        Q_ASSERT(controller);

        controller -> canvas() -> update();
        
        m_startPos = QPoint(0, 0);
        m_endPos = QPoint(0, 0);

        setOptionWidgetStartX(m_startPos.x());
        setOptionWidgetStartY(m_startPos.y());
        setOptionWidgetEndX(m_endPos.x());
        setOptionWidgetEndY(m_endPos.y());

        m_selecting = false;
    }
}

void KisToolCrop::buttonPress(KisButtonPressEvent *e)
{
    if (m_subject) {
        KisImageSP img = m_subject -> currentImg();

        if (img && img -> activeDevice() && e -> button() == LeftButton) {

            if (img -> bounds().contains(e -> pos().floorQPoint())) {

                m_selecting = true;

                if( !m_haveCropSelection ) //if the selection is not set
                {
                    m_startPos = e -> pos().floorQPoint();
                    m_endPos = e -> pos().floorQPoint();

                    paintOutlineWithHandles();
                }
                else
                {
                    KisCanvasControllerInterface *controller = m_subject -> canvasController();
                    m_mouseOnHandleType = mouseOnHandle(controller ->windowToView(e -> pos().floorQPoint()));
                    m_dragStart = e -> pos().floorQPoint();
                }

                setOptionWidgetStartX(m_startPos.x());
                setOptionWidgetStartY(m_startPos.y());
                setOptionWidgetEndX(m_endPos.x());
                setOptionWidgetEndY(m_endPos.y());
            }
                }
        }
}

void KisToolCrop::move(KisMoveEvent *e)
{
        if ( m_subject && m_subject -> currentImg())
        {
                if( m_selecting ) //if the user selects
                {
                        if( !m_haveCropSelection ) //if the cropSelection is not yet set
                        {
                paintOutlineWithHandles();

                m_endPos = e -> pos().floorQPoint();

                KisImageSP image = m_subject -> currentImg();

                m_endPos.setX(CLAMP(m_endPos.x(), 0, image -> width()));
                m_endPos.setY(CLAMP(m_endPos.y(), 0, image -> height()));

                                paintOutlineWithHandles();
                        }
                        else //if the crop selection is set
                        {
                m_dragStop = e -> pos().floorQPoint();
                if (m_mouseOnHandleType != None && m_dragStart != m_dragStop ) {

                    QPoint pos = e -> pos().floorQPoint();
                    Q_INT32 width = QABS( m_startPos.x() - m_endPos.x() ); //with of the selected rectangle
                    Q_INT32 height = QABS( m_startPos.y() - m_endPos.y() ); //height of the selected rectangle

                    Q_INT32 imageWidth = m_subject -> currentImg() -> width();
                    Q_INT32 imageHeight = m_subject -> currentImg() -> height();

                    paintOutlineWithHandles();

                    switch (m_mouseOnHandleType)
                    {
                        case (UpperLeft):
                            m_startPos.setX( pos.x() + m_dx );
                            m_startPos.setY( pos.y() + m_dy );
                            break;
                        case (LowerRight):
                            m_endPos.setX( pos.x() + m_dx );
                            m_endPos.setY( pos.y() + m_dy );
                            break;
                        case (UpperRight):
                            m_endPos.setX( pos.x() + m_dx );
                            m_startPos.setY( pos.y() + m_dy );
                            break;
                        case (LowerLeft):
                            m_startPos.setX( pos.x() + m_dx );
                            m_endPos.setY( pos.y() + m_dy );
                            break;
                        case (Upper):
                            m_startPos.setY( pos.y() + m_dy );
                            break;
                        case (Lower):
                            m_endPos.setY( pos.y() + m_dy );
                            break;
                        case (Left):
                            m_startPos.setX( pos.x() + m_dx );
                            break;
                        case (Right):
                            m_endPos.setX( pos.x() + m_dx );
                            break;
                        case (Inside):
                            m_startPos.setX( m_startPos.x() - ( m_dragStart.x() - m_dragStop.x() ) );
                            m_endPos.setX( m_endPos.x() - ( m_dragStart.x() - m_dragStop.x() ) );
                            m_startPos.setY( m_startPos.y() - ( m_dragStart.y() - m_dragStop.y() ) );
                            m_endPos.setY( m_endPos.y() - ( m_dragStart.y() - m_dragStop.y() ) );
                            break;
                    }


                    m_startPos.setX(CLAMP(m_startPos.x(), 0, imageWidth));
                    m_startPos.setY(CLAMP(m_startPos.y(), 0, imageHeight));
                    m_endPos.setX(CLAMP(m_endPos.x(), 0, imageWidth));
                    m_endPos.setY(CLAMP(m_endPos.y(), 0, imageHeight));
                    m_dragStart = e -> pos().floorQPoint();
                    paintOutlineWithHandles();
                }
                        }

            setOptionWidgetStartX(QMIN(m_startPos.x(), m_endPos.x()));
            setOptionWidgetStartY(QMIN(m_startPos.y(), m_endPos.y()));
            setOptionWidgetEndX(QMAX(m_startPos.x(), m_endPos.x()));
            setOptionWidgetEndY(QMAX(m_startPos.y(), m_endPos.y()));
                }
                else //if we are not selecting
                {
                        if ( m_haveCropSelection )  //if the crop selection is set
                        {
                KisCanvasControllerInterface *controller = m_subject -> canvasController();
                                Q_INT32 type = mouseOnHandle(controller -> windowToView(e -> pos().floorQPoint()));
                                //set resize cursor if we are on one of the handles
                                setMoveResizeCursor(type);
                        } 
                }
        }
}

void KisToolCrop::buttonRelease(KisButtonReleaseEvent *e)
{
    if (m_subject && m_subject -> currentImg() && m_selecting && e -> button() == LeftButton) {

        m_selecting = false;
        m_haveCropSelection = true;

        paintOutlineWithHandles();
        validateSelection();
        paintOutlineWithHandles();
    }
}

void KisToolCrop::doubleClick(KisDoubleClickEvent *)
{
    if (m_haveCropSelection) crop();
}

void KisToolCrop::validateSelection(void)
{
    if (m_subject) {
        KisImageSP image = m_subject -> currentImg();

        if (image) {
            Q_INT32 imageWidth = image -> width();
            Q_INT32 imageHeight = image -> height();

            m_startPos.setX(CLAMP(m_startPos.x(), 0, imageWidth));
            m_startPos.setY(CLAMP(m_startPos.y(), 0, imageHeight));
            m_endPos.setX(CLAMP(m_endPos.x(), 0, imageWidth));
            m_endPos.setY(CLAMP(m_endPos.y(), 0, imageHeight));

            if (m_startPos.x() > m_endPos.x()) {
                Q_INT32 startX = m_startPos.x();
                m_startPos.setX(m_endPos.x());
                m_endPos.setX(startX);
            }

            if (m_startPos.y() > m_endPos.y()) {
                Q_INT32 startY = m_startPos.y();
                m_startPos.setY(m_endPos.y());
                m_endPos.setY(startY);
            }

            setOptionWidgetStartX(m_startPos.x());
            setOptionWidgetStartY(m_startPos.y());
            setOptionWidgetEndX(m_endPos.x());
            setOptionWidgetEndY(m_endPos.y());
        }
    }
}

void KisToolCrop::paintOutlineWithHandles()
{
    if (m_subject) {
        KisCanvasControllerInterface *controller = m_subject -> canvasController();
        QWidget *canvas = controller -> canvas();
        QPainter gc(canvas);
        QRect rc;

        paintOutlineWithHandles(gc, rc);
    }
}

void KisToolCrop::paintOutlineWithHandles(QPainter& gc, const QRect&)
{
    if (m_subject && (m_selecting || m_haveCropSelection)) {
        KisCanvasControllerInterface *controller = m_subject -> canvasController();
        RasterOp op = gc.rasterOp();
        QPen old = gc.pen();
        QPen pen(Qt::SolidLine);
        pen.setWidth(1);
        QPoint start;
        QPoint end;

        Q_ASSERT(controller);
        start = controller -> windowToView(m_startPos);
        end = controller -> windowToView(m_endPos);

        gc.setRasterOp(Qt::NotROP);
                gc.setPen(pen);
                //draw handles
                m_handlesRegion = handles(QRect(start, end));

                Q_INT32 startx;
                Q_INT32 starty;
                Q_INT32 endx;
                Q_INT32 endy;
                if(start.x()<=end.x())
                {
                        startx=start.x();
                        endx=end.x();
                }
                else
                {
                        startx=end.x();
                        endx=start.x();
                }
                if(start.y()<=end.y())
                {
                        starty=start.y();
                        endy=end.y();
                }
                else
                {
                        starty=end.y();
                        endy=start.y();
                }
        //draw upper line of selection
                gc.drawLine(startx + m_handleSize / 2 + 1, starty, startx + (endx - startx - m_handleSize) / 2 + 1, starty);
        gc.drawLine(startx + (endx - startx + m_handleSize) / 2 + 1, starty, endx - m_handleSize / 2, starty);
        //draw lower line of selection
        gc.drawLine(startx + m_handleSize / 2 + 1, endy, startx + (endx - startx - m_handleSize) / 2 + 1, endy);
        gc.drawLine(startx + (endx - startx + m_handleSize) / 2 + 1, endy, endx - m_handleSize / 2 , endy);
        //draw right line of selection
        gc.drawLine(startx, starty + m_handleSize / 2 + 1, startx, starty + (endy - starty - m_handleSize) / 2 + 1);
        gc.drawLine(startx, starty + (endy - starty + m_handleSize) / 2 + 1, startx, endy - m_handleSize / 2);
        //draw left line of selection
        gc.drawLine(endx, starty + m_handleSize / 2 + 1, endx, starty + (endy - starty - m_handleSize) / 2 + 1);
        gc.drawLine(endx, starty + (endy - starty + m_handleSize) / 2 + 1, endx, endy - m_handleSize / 2);

        //draw guides
        gc.drawLine(0,endy,startx - m_handleSize / 2,endy);
                gc.drawLine(startx,endy + m_handleSize / 2 + 1, startx, controller -> canvas() -> height());
                gc.drawLine(endx,0,endx,starty - m_handleSize / 2);
                gc.drawLine(endx + m_handleSize / 2 + 1,starty, controller -> canvas() -> width(), starty);
                QMemArray <QRect> rects = m_handlesRegion.rects (); 
                for (QMemArray <QRect>::ConstIterator it = rects.begin (); it != rects.end (); it++)
                {
                        gc.fillRect (*it, Qt::black);
                }


                gc.setRasterOp(op);
        gc.setPen(old);
    }
}

void KisToolCrop::crop() {
    // XXX: Should cropping be part of KisImage/KisPaintDeviceImpl's API?

        m_haveCropSelection = false;

    KisImageSP img = m_subject -> currentImg();

    if (img -> undoAdapter())
        img -> undoAdapter() -> beginMacro(i18n("Crop"));
    
    if (!img)
        return;

    if (m_endPos.y() < 0)
        m_endPos.setY(0);
    
    if (m_endPos.y() > img -> height())
        m_endPos.setY(img -> height());
    
    if (m_endPos.x() < 0)
        m_endPos.setX(0);
    
    if (m_endPos.x() > img -> width())
                m_endPos.setX(img -> width());
    
    QRect rc(m_startPos, m_endPos);
    rc = rc.normalize();

    // We don't want the border of the 'rectangle' to be included in our selection
    rc.setSize(rc.size() - QSize(1,1));
    
    
    if (m_optWidget -> cmbType -> currentItem() == 0) {
        KisLayerSP layer = img -> activeLayer();
        cropLayer(layer, rc);
        KNamedCommand * cmd = layer -> moveCommand(layer -> getX() - rc.x(), layer -> getY() - rc.y());
        if (m_subject -> undoAdapter()) m_subject -> undoAdapter() -> addCommand(cmd);
        img -> notify();
    }
    else {
        vKisLayerSP layers = img -> layers();
        vKisLayerSP_it it;
        for ( it = layers.begin(); it != layers.end(); ++it ) {
            KisLayerSP layer = (*it);
            cropLayer(layer, rc);
            KNamedCommand * cmd = layer -> moveCommand(layer -> getX() - rc.x(), layer -> getY() - rc.y());
            if (m_subject -> undoAdapter()) m_subject -> undoAdapter() -> addCommand(cmd);
        }
        img -> resize(rc);
        img -> notify(QRect(0, 0, rc.width(), rc.height()));
    }

    if (img -> undoAdapter())
        img -> undoAdapter() -> endMacro();

    m_startPos = QPoint(0, 0);
    m_endPos = QPoint(0, 0);

    setOptionWidgetStartX(0);
    setOptionWidgetStartY(0);
    setOptionWidgetEndX(0);
    setOptionWidgetEndY(0);
}

void KisToolCrop::setStartX(int x)
{ 
    if (x <= m_endPos.x() || m_startPos.x() == m_endPos.x()) {

        if (!m_haveCropSelection) {
            m_haveCropSelection = true;
        }
        else {
            paintOutlineWithHandles();
        }

        m_startPos.setX(x);

        if (m_endPos.x() < x) {
            m_endPos.setX(x);
        }

        validateSelection();
        paintOutlineWithHandles();
    } else {
        setOptionWidgetStartX(m_startPos.x());
    }
}

void KisToolCrop::setStartY(int y)
{ 
    if (y <= m_endPos.y() || m_startPos.y() == m_endPos.y()) {

        if (!m_haveCropSelection) {
            m_haveCropSelection = true;
        }
        else {
            paintOutlineWithHandles();
        }

        m_startPos.setY(y);

        if (m_endPos.y() < y) {
            m_endPos.setY(y);
        }

        validateSelection();
        paintOutlineWithHandles();
    } else {
        setOptionWidgetStartY(m_startPos.y());
    }
}

void KisToolCrop::setEndX(int x)
{
    if (x >= m_startPos.x() || m_startPos.x() == m_endPos.x()) {

        if (!m_haveCropSelection) {
            m_haveCropSelection = true;
        }
        else {
            paintOutlineWithHandles();
        }

        m_endPos.setX(x);

        if (m_startPos.x() > x) {
            m_startPos.setX(x);
        }

        validateSelection();
        paintOutlineWithHandles();
    } else {
        setOptionWidgetEndX(m_endPos.x());
    }
}

void KisToolCrop::setEndY(int y)
{ 
    if (y >= m_startPos.y() || m_startPos.y() == m_endPos.y()) {

        if (!m_haveCropSelection) {
            m_haveCropSelection = true;
        }
        else {
            paintOutlineWithHandles();
        }

        m_endPos.setY(y);

        if (m_startPos.y() > y) {
            m_startPos.setY(y);
        }

        validateSelection();
        paintOutlineWithHandles();
    } else {
        setOptionWidgetEndY(m_endPos.y());
    }
}

void KisToolCrop::setOptionWidgetStartX(Q_INT32 x)
{
    // Disable signals otherwise we get the valueChanged signal, which we don't want
    // to go through the logic for setting values that way.
    m_optWidget -> intStartX -> blockSignals(true);
    m_optWidget -> intStartX -> setValue(x);
    m_optWidget -> intStartX -> blockSignals(false);
}

void KisToolCrop::setOptionWidgetStartY(Q_INT32 y)
{
    m_optWidget -> intStartY -> blockSignals(true);
    m_optWidget -> intStartY -> setValue(y);
    m_optWidget -> intStartY -> blockSignals(false);
}

void KisToolCrop::setOptionWidgetEndX(Q_INT32 x)
{
    m_optWidget -> intEndX -> blockSignals(true);
    m_optWidget -> intEndX -> setValue(x);
    m_optWidget -> intEndX -> blockSignals(false);
}

void KisToolCrop::setOptionWidgetEndY(Q_INT32 y)
{
    m_optWidget -> intEndY -> blockSignals(true);
    m_optWidget -> intEndY -> setValue(y);
    m_optWidget -> intEndY -> blockSignals(false);
}

void KisToolCrop::cropLayer(KisLayerSP layer, QRect rc) 
{
    KisSelectedTransaction * t = new KisSelectedTransaction(i18n("Crop"), layer.data());
    Q_CHECK_PTR(t);
    
    layer -> crop(rc);

    m_subject -> undoAdapter() -> addCommand(t);
    
}

QWidget* KisToolCrop::createOptionWidget(QWidget* parent)
{
    m_optWidget = new WdgToolCrop(parent);
    Q_CHECK_PTR(m_optWidget);
                      
    connect(m_optWidget -> bnCrop, SIGNAL(clicked()), this, SLOT(crop()));

    connect(m_optWidget -> intStartX, SIGNAL(valueChanged(int)), this, SLOT(setStartX(int)));
    connect(m_optWidget -> intStartY, SIGNAL(valueChanged(int)), this, SLOT(setStartY(int)));
    connect(m_optWidget -> intEndX, SIGNAL(valueChanged(int)), this, SLOT(setEndX(int)));
    connect(m_optWidget -> intEndY, SIGNAL(valueChanged(int)), this, SLOT(setEndY(int)));

    return m_optWidget;
}

QWidget* KisToolCrop::optionWidget()
{
    return m_optWidget;
}

void KisToolCrop::setup(KActionCollection *collection)
{
    m_action = static_cast<KRadioAction *>(collection -> action(name()));

    if (m_action == 0) {
        m_action = new KRadioAction(i18n("&Crop"),
                        "crop", 
                        0, 
                        this,
                        SLOT(activate()), 
                        collection, 
                        name());
        Q_CHECK_PTR(m_action);

        m_action -> setToolTip(i18n("Crop an area"));
        m_action -> setExclusiveGroup("tools");

        m_ownAction = true;
    }
}

QRegion KisToolCrop::handles(QRect rect)
{
        QRegion handlesRegion;

        //add handle at the lower right corner
        handlesRegion += QRect( QABS( rect.width() ) - m_handleSize / 2.0, QABS( rect.height() ) - m_handleSize / 2.0, m_handleSize, m_handleSize );
        //add handle at the upper right corner
        handlesRegion += QRect( QABS( rect.width() ) - m_handleSize / 2.0 , 0 - m_handleSize / 2.0, m_handleSize, m_handleSize );
        //add rectangle at the lower left corner
        handlesRegion += QRect( 0 - m_handleSize / 2.0 , QABS( rect.height() ) - m_handleSize / 2.0, m_handleSize, m_handleSize );
        //add rectangle at the upper left corner
        handlesRegion += QRect( 0 - m_handleSize / 2.0, 0 - m_handleSize / 2.0, m_handleSize, m_handleSize );
    //add handle at the lower edge of the rectangle
    handlesRegion += QRect( ( QABS( rect.width() ) - m_handleSize ) / 2.0 , QABS( rect.height() ) - m_handleSize / 2.0, m_handleSize, m_handleSize );
    //add handle at the right edge of the rectangle
    handlesRegion += QRect( QABS( rect.width() ) - m_handleSize / 2.0 , ( QABS( rect.height() ) - m_handleSize ) / 2.0, m_handleSize, m_handleSize );
    //add handle at the upper edge of the rectangle
    handlesRegion += QRect( ( QABS( rect.width() ) - m_handleSize ) / 2.0 , 0 - m_handleSize / 2.0, m_handleSize, m_handleSize );
    //add handle at the left edge of the rectangle
    handlesRegion += QRect( 0 - m_handleSize / 2.0, ( QABS( rect.height() ) - m_handleSize ) / 2.0, m_handleSize, m_handleSize );

        //move the handles to the correct position
        if( rect.width() >= 0 && rect.height() >= 0)
        {
                handlesRegion.translate ( rect.x(), rect.y() );
        }
        else if( rect.width() < 0 && rect.height() >= 0)
        {
                handlesRegion.translate ( rect.x() - QABS( rect.width() ), rect.y() );
        }
        else if( rect.width() >= 0 && rect.height() < 0)
        {
                handlesRegion.translate ( rect.x(), rect.y() - QABS( rect.height() ) );
        }
        else if( rect.width() < 0 && rect.height() < 0)
        {
                handlesRegion.translate ( rect.x() - QABS( rect.width() ), rect.y() - QABS( rect.height() ) );
        }
        return handlesRegion;
}

Q_INT32 KisToolCrop::mouseOnHandle(QPoint currentViewPoint)
{
        KisCanvasControllerInterface *controller = m_subject -> canvasController();
        Q_ASSERT(controller);
        QPoint start = controller -> windowToView(m_startPos);
        QPoint end = controller -> windowToView(m_endPos);

        Q_INT32 startx;
                Q_INT32 starty;
                Q_INT32 endx;
                Q_INT32 endy;
                if(start.x()<=end.x())
                {
                        startx=start.x();
                        endx=end.x();
                }
                else
                {
                        startx=end.x();
                        endx=start.x();
                }
                if(start.y()<=end.y())
                {
                        starty=start.y();
                        endy=end.y();
                }
                else
                {
                        starty=end.y();
                        endy=start.y();
                }

        if ( QRect ( startx - m_handleSize / 2.0, starty - m_handleSize / 2.0, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
        {
                if( !m_selecting )
                {
                        m_dx= startx-currentViewPoint.x();
                        m_dy = starty - currentViewPoint.y();
                }
                return UpperLeft;
        }
        else if ( QRect ( startx - m_handleSize / 2.0, endy -  m_handleSize / 2.0, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
        {
                if( !m_selecting )
                {
                        m_dx = startx-currentViewPoint.x();
                        m_dy = endy-currentViewPoint.y();
                }
                return LowerLeft;
        }
        else if ( QRect ( endx - m_handleSize / 2.0, starty - m_handleSize / 2.0, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
        {
                if( !m_selecting )
                {
                        m_dx = endx - currentViewPoint.x();
                        m_dy = starty - currentViewPoint.y() ;
                }
                return UpperRight;
        }
        else if ( QRect ( endx - m_handleSize / 2.0, endy - m_handleSize / 2.0, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
        {
                if( !m_selecting )
                {
                        m_dx = endx - currentViewPoint.x();
                        m_dy= endy - currentViewPoint.y();
                }
                return LowerRight;
        }
    else if ( QRect ( startx + ( endx - startx - m_handleSize ) / 2.0, starty - m_handleSize / 2.0, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
        {
                if( !m_selecting )
                {
                        m_dy = starty - currentViewPoint.y() ;
                }
                return Upper;
        }
    else if ( QRect ( startx + ( endx - startx - m_handleSize ) / 2.0, endy - m_handleSize / 2, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
        {
                if( !m_selecting )
                {
                        m_dy = endy - currentViewPoint.y();
                }
                return Lower;
        }
    else if ( QRect ( startx - m_handleSize / 2.0, starty + ( endy - starty - m_handleSize ) / 2.0, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
        {
                if( !m_selecting )
                {
                        m_dx = startx - currentViewPoint.x() ;
                }
                return Left;
        }
    else if ( QRect ( endx - m_handleSize / 2.0 , starty + ( endy - starty - m_handleSize ) / 2.0, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
        {
                if( !m_selecting )
                {
                        m_dx = endx-currentViewPoint.x();
                }
                return Right;
        }
    else if ( QRect ( m_startPos.x() , m_startPos.y(), m_endPos.x() - m_startPos.x() , m_endPos.y() - m_startPos.y() ).contains( currentViewPoint ) )
        {
                return Inside;
        }
    else return None;
}

void KisToolCrop::setMoveResizeCursor (Q_INT32 handle)
{
        switch (handle)
        {
        case (UpperLeft):
    case (LowerRight):
                m_subject->canvasController()->setCanvasCursor(KisCursor::sizeFDiagCursor());
                return;
        case (LowerLeft):
        case (UpperRight):
                m_subject->canvasController()->setCanvasCursor(KisCursor::sizeBDiagCursor());
                return;
    case (Upper):
    case (Lower):
        m_subject->canvasController()->setCanvasCursor(KisCursor::sizeVerCursor());
                return;
    case (Left):
    case (Right):
        m_subject->canvasController()->setCanvasCursor(KisCursor::sizeHorCursor());
                return;
    case (Inside):
        m_subject->canvasController()->setCanvasCursor(KisCursor::sizeAllCursor());
        return;
    }    
        m_subject->canvasController()->setCanvasCursor(KisCursor::selectCursor());
        return;
}


#include "kis_tool_crop.moc"
