/*
 *  kis_tool_crop.cc -- part of Krita
 *
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2005 Michael Thaler <michael.thaler@physik.tu-muenchen.de>
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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


#include <qcheckbox.h>
#include <qcombobox.h>
#include <qobject.h>
#include <qpainter.h>
#include <qpen.h>
#include <qpushbutton.h>
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
#include <kis_layer.h>
#include <kis_crop_visitor.h>

#include "kis_tool_crop.h"

#include "kis_canvas.h"
#include "kis_canvas_painter.h"



KisToolCrop::KisToolCrop()
    : super(i18n("Crop"))
{
    setObjectName("tool_crop");
    m_cropCursor = KisCursor::load("tool_crop_cursor.png", 6, 6);
    setCursor(m_cropCursor);
    m_subject = 0;
    m_selecting = false;
    m_rectCrop = QRect(0, 0, 0, 0);
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

    // No current crop rectangle, try to use the selection of the device to make a rectangle
    if (m_subject && m_subject->currentImg() && m_subject->currentImg()->activeDevice()) {
        KisPaintDeviceSP device = m_subject->currentImg()->activeDevice();
        if (!device->hasSelection())
            return;

        m_rectCrop = device->selection()->exactBounds();
        validateSelection();
        crop();
    }
}

void KisToolCrop::deactivate()
{
    if (m_subject) {
        KisCanvasController *controller = m_subject->canvasController();
        KisImageSP img = m_subject->currentImg();

        Q_ASSERT(controller);

        controller->kiscanvas()->update();

        m_rectCrop = QRect(0,0,0,0);

        updateWidgetValues();
        m_selecting = false;
    }
}

void KisToolCrop::paint(KisCanvasPainter& gc)
{
    paintOutlineWithHandles(gc, QRect());
}

void KisToolCrop::paint(KisCanvasPainter& gc, const QRect& rc)
{
    paintOutlineWithHandles(gc, rc);
}

void KisToolCrop::clearRect()
{
    if (m_subject) {
        KisCanvasController *controller = m_subject->canvasController();
        KisImageSP img = m_subject->currentImg();

        Q_ASSERT(controller);

        controller->kiscanvas()->update();

        m_rectCrop = QRect(0,0,0,0);

        updateWidgetValues();
        m_selecting = false;
    }
}

void KisToolCrop::buttonPress(KisButtonPressEvent *e)
{
    if (m_subject) {
        KisImageSP img = m_subject->currentImg();

        if (img && img->activeDevice() && e->button() == Qt::LeftButton) {

            if (img->bounds().contains(e->pos().floorQPoint())) {

                m_selecting = true;

                if( !m_haveCropSelection ) //if the selection is not set
                {
                    QPoint p = e->pos().floorQPoint();
                    m_rectCrop = QRect( p.x(), p.y(), 0, 0);
                    paintOutlineWithHandles();
                }
                else
                {
                    KisCanvasController *controller = m_subject->canvasController();
                    m_mouseOnHandleType = mouseOnHandle(controller ->windowToView(e->pos().floorQPoint()));
                    m_dragStart = e->pos().floorQPoint();
                }

                updateWidgetValues();
            }
        }
    }
}

void KisToolCrop::move(KisMoveEvent *e)
{
    if ( m_subject && m_subject->currentImg())
    {
        if( m_selecting ) //if the user selects
        {
            if( !m_haveCropSelection ) //if the cropSelection is not yet set
            {
                paintOutlineWithHandles();

                m_rectCrop.setBottomRight( e->pos().floorQPoint());

                KisImageSP image = m_subject->currentImg();

                m_rectCrop.setRight( qMin(m_rectCrop.right(), image->width()));
                m_rectCrop.setBottom( qMin(m_rectCrop.bottom(), image->width()));

                paintOutlineWithHandles();
            }
            else //if the crop selection is set
            {
                m_dragStop = e->pos().floorQPoint();
                if (m_mouseOnHandleType != None && m_dragStart != m_dragStop ) {


                    qint32 imageWidth = m_subject->currentImg()->width();
                    qint32 imageHeight = m_subject->currentImg()->height();

                    paintOutlineWithHandles();

                    QPoint pos = e->pos().floorQPoint();
                    if( m_mouseOnHandleType == Inside )
                    {
                        m_rectCrop.translate( ( m_dragStop.x() - m_dragStart.x() ),  ( m_dragStop.y() - m_dragStart.y() ) );
                        if( m_rectCrop.left() < 0 )
                        {
                            m_rectCrop.moveLeft( 0 );
                        }
                        if( m_rectCrop.right() > imageWidth )
                        {
                            m_rectCrop.moveRight( imageWidth  );
                        }
                        if( m_rectCrop.top() < 0 )
                        {
                            m_rectCrop.moveTop( 0 );
                        }
                        if( m_rectCrop.bottom() > imageHeight )
                        {
                            m_rectCrop.moveBottom( imageHeight  );
                        }
                    } else if(m_optWidget->boolRatio->isChecked())
                    {
                        QPoint drag = m_dragStop - m_dragStart;
                        if( ! m_optWidget->boolWidth->isChecked() && !m_optWidget->boolHeight->isChecked() )
                        {
                            switch (m_mouseOnHandleType)
                            {
                            case (UpperLeft):
                            {
                                qint32 dep = (drag.x() + drag.y()) / 2;
                                m_rectCrop.setTop( m_rectCrop.top() + dep  );
                                m_rectCrop.setLeft( (int) ( m_rectCrop.right() - m_optWidget->doubleRatio->value() * m_rectCrop.height() ) );
                            }
                                break;
                            case (LowerRight):
                            {
                                qint32 dep = (drag.x() + drag.y()) / 2;
                                m_rectCrop.setBottom( m_rectCrop.bottom() + dep );
                                m_rectCrop.setWidth( (int) ( m_optWidget->doubleRatio->value() * m_rectCrop.height() ) );
                                break;
                            }
                            case (UpperRight):
                            {
                                qint32 dep = (drag.x() - drag.y()) / 2;
                                m_rectCrop.setTop( m_rectCrop.top() - dep  );
                                m_rectCrop.setWidth( (int) ( m_optWidget->doubleRatio->value() * m_rectCrop.height() ) );
                                break;
                            }
                            case (LowerLeft):
                            {
                                qint32 dep = (drag.x() - drag.y()) / 2;
                                m_rectCrop.setBottom( m_rectCrop.bottom() - dep );
                                m_rectCrop.setLeft( (int) ( m_rectCrop.right() - m_optWidget->doubleRatio->value() * m_rectCrop.height() ) );
                                break;
                            }
                            case (Upper):
                                m_rectCrop.setTop( pos.y() + m_dy );
                                m_rectCrop.setWidth( (int) (m_rectCrop.height() * m_optWidget->doubleRatio->value()) );
                                break;
                            case (Lower):
                                m_rectCrop.setBottom( pos.y() + m_dy );
                                m_rectCrop.setWidth( (int) (m_rectCrop.height() * m_optWidget->doubleRatio->value()) );
                                break;
                            case (Left):
                                m_rectCrop.setLeft( pos.x() + m_dx );
                                m_rectCrop.setHeight( (int) (m_rectCrop.width() / m_optWidget->doubleRatio->value()) );
                                break;
                            case (Right):
                                m_rectCrop.setRight( pos.x() + m_dx );
                                m_rectCrop.setHeight( (int) (m_rectCrop.width() / m_optWidget->doubleRatio->value()) );
                                break;
                            case (Inside): // never happen
                                break;
                            }
                        }
                    } else {
                        if( m_optWidget->boolWidth->isChecked() )
                        {
                            m_rectCrop.setWidth( m_optWidget->intWidth->value() + 1 );
                        } else {
                            switch (m_mouseOnHandleType)
                            {
                                case (LowerLeft):
                                case (Left):
                                case (UpperLeft):
                                    m_rectCrop.setLeft( pos.x() + m_dx );
                                    break;
                                case (Right):
                                case (UpperRight):
                                case (LowerRight):
                                    m_rectCrop.setRight( pos.x() + m_dx );
                                    break;
                                default:
                                    break;
                            }
                        }
                        if( m_optWidget->boolHeight->isChecked() )
                        {
                            m_rectCrop.setHeight( m_optWidget->intHeight->value() + 1 );
                        } else {
                            switch (m_mouseOnHandleType)
                            {
                                case (UpperLeft):
                                case (Upper):
                                case (UpperRight):
                                    m_rectCrop.setTop( pos.y() + m_dy );
                                    break;
                                case (LowerRight):
                                case (LowerLeft):
                                case (Lower):
                                    m_rectCrop.setBottom( pos.y() + m_dy );
                                    break;
                                default:
                                    break;
                            }
                        }
                    }
                    m_rectCrop = m_rectCrop.intersect( QRect(0,0, imageWidth + 1, imageHeight + 1 ) );
                    m_dragStart = e->pos().floorQPoint();
                    paintOutlineWithHandles();
                }
            }
            updateWidgetValues();
        }
        else //if we are not selecting
        {
            if ( m_haveCropSelection )  //if the crop selection is set
            {
                KisCanvasController *controller = m_subject->canvasController();
                qint32 type = mouseOnHandle(controller->windowToView(e->pos().floorQPoint()));
                //set resize cursor if we are on one of the handles
                setMoveResizeCursor(type);
            }
        }
    }
}

void KisToolCrop::updateWidgetValues(bool updateratio)
{
    QRect r = realRectCrop();
    setOptionWidgetX(r.x());
    setOptionWidgetY(r.y());
    setOptionWidgetWidth(r.width() );
    setOptionWidgetHeight(r.height() );
    if(updateratio && !m_optWidget->boolRatio->isChecked() )
        setOptionWidgetRatio((double)r.width() / (double)r.height() );
}

void KisToolCrop::buttonRelease(KisButtonReleaseEvent *e)
{
    if (m_subject && m_subject->currentImg() && m_selecting && e->button() == Qt::LeftButton) {

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

void KisToolCrop::validateSelection(bool updateratio)
{
    if (m_subject) {
        KisImageSP image = m_subject->currentImg();

        if (image) {
            qint32 imageWidth = image->width();
            qint32 imageHeight = image->height();
            m_rectCrop.setLeft(qMax(0, m_rectCrop.left()));
            m_rectCrop.setTop(qMax(0, m_rectCrop.top()));
            m_rectCrop.setRight(qMin(imageWidth, m_rectCrop.right()));
            m_rectCrop.setBottom(qMin(imageHeight, m_rectCrop.bottom()));

            updateWidgetValues(updateratio);
        }
    }
}

void KisToolCrop::paintOutlineWithHandles()
{
    if (m_subject) {
        KisCanvasController *controller = m_subject->canvasController();
        KisCanvas *canvas = controller->kiscanvas();
        KisCanvasPainter gc(canvas);
        QRect rc;

        paintOutlineWithHandles(gc, rc);
    }
}

void KisToolCrop::paintOutlineWithHandles(KisCanvasPainter& gc, const QRect&)
{
    if (m_subject && (m_selecting || m_haveCropSelection)) {
        KisCanvasController *controller = m_subject->canvasController();
        //RasterOp op = gc.rasterOp();
        QPen old = gc.pen();
        QPen pen(Qt::SolidLine);
        pen.setWidth(1);
        QPoint start;
        QPoint end;

        Q_ASSERT(controller);
        start = controller->windowToView(m_rectCrop.topLeft());
        end = controller->windowToView(m_rectCrop.bottomRight());

        //gc.setRasterOp(Qt::NotROP);
        gc.setPen(pen);
        //draw handles
        m_handlesRegion = handles(QRect(start, end));

        qint32 startx;
        qint32 starty;
        qint32 endx;
        qint32 endy;
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
        gc.drawLine(startx,endy + m_handleSize / 2 + 1, startx, controller->kiscanvas()->height());
        gc.drawLine(endx,0,endx,starty - m_handleSize / 2);
        gc.drawLine(endx + m_handleSize / 2 + 1,starty, controller->kiscanvas()->width(), starty);
        Q3MemArray <QRect> rects = m_handlesRegion.rects ();
        for (Q3MemArray <QRect>::ConstIterator it = rects.begin (); it != rects.end (); ++it)
        {
            gc.fillRect (*it, Qt::black);
        }


        //gc.setRasterOp(op);
        gc.setPen(old);
    }
}

void KisToolCrop::crop() {
    // XXX: Should cropping be part of KisImage/KisPaintDevice's API?

    m_haveCropSelection = false;
    setCursor(m_cropCursor);

    KisImageSP img = m_subject->currentImg();

    if (!img)
        return;

    QRect rc =  realRectCrop().normalized();


    // The visitor adds the undo steps to the macro
    if (m_optWidget->cmbType->currentIndex() == 0) {
        // The layer(s) under the current layer will take care of adding
        // undo information to the Crop macro.
        if (img->undo())
            img->undoAdapter()->beginMacro(i18n("Crop"));

        KisCropVisitor v(rc, false);
        KisLayerSP layer = img->activeLayer();
        layer->accept(v);

        if (img->undo())
            img->undoAdapter()->endMacro();

    }
    else {
        // Resize creates the undo macro itself
        img->resize(rc, true);
    }

    m_rectCrop = QRect(0,0,0,0);

    updateWidgetValues();
}

void KisToolCrop::setCropX(int x)
{
    if (!m_haveCropSelection) {
        m_haveCropSelection = true;
    }
    else {
        paintOutlineWithHandles(); // remove outlines
    }

    m_rectCrop.setX(x);

    validateSelection();
    paintOutlineWithHandles();
}

void KisToolCrop::setCropY(int y)
{
    if (!m_haveCropSelection) {
        m_haveCropSelection = true;
    }
    else {
        paintOutlineWithHandles(); // remove outlines
    }

    m_rectCrop.setY(y);

    validateSelection();
    paintOutlineWithHandles();

}

void KisToolCrop::setCropWidth(int w)
{
    if (!m_haveCropSelection) {
        m_haveCropSelection = true;
    }
    else {
        paintOutlineWithHandles(); // remove outlines
    }

    m_rectCrop.setWidth(w + 1);

    if( m_optWidget->boolRatio->isChecked() )
    {
        m_rectCrop.setHeight( (int) ( w / m_optWidget->doubleRatio->value() ) );
    } else {
        setOptionWidgetRatio((double)m_rectCrop.width() / (double)m_rectCrop.height() );
    }

    validateSelection();
    paintOutlineWithHandles();

}

void KisToolCrop::setCropHeight(int h)
{
    if (!m_haveCropSelection) {
        m_haveCropSelection = true;
    }
    else {
        paintOutlineWithHandles(); // remove outlines
    }

    m_rectCrop.setHeight(h + 1);

    if( m_optWidget->boolRatio->isChecked() )
    {
        m_rectCrop.setWidth( (int) ( h * m_optWidget->doubleRatio->value() ) );
    } else {
        setOptionWidgetRatio((double)m_rectCrop.width() / (double)m_rectCrop.height() );
    }

    validateSelection();
    paintOutlineWithHandles();

}

void KisToolCrop::setRatio(double )
{
    if( ! (m_optWidget->boolWidth->isChecked() && m_optWidget->boolHeight->isChecked() ))
    {
        if (!m_haveCropSelection) {
            m_haveCropSelection = true;
        }
        else {
            paintOutlineWithHandles(); // remove outlines
        }
        if( m_optWidget->boolWidth->isChecked() )
        {
            m_rectCrop.setHeight( (int) ( m_rectCrop.width() / m_optWidget->doubleRatio->value()) );
            setOptionWidgetHeight( m_rectCrop.height() );
        } else if(m_optWidget->boolHeight->isChecked()) {
            m_rectCrop.setWidth( (int) (m_rectCrop.height() * m_optWidget->doubleRatio->value()) );
            setOptionWidgetWidth( m_rectCrop.width() );
        } else {
            int newwidth = (int) (m_optWidget->doubleRatio->value() * m_rectCrop.height());
            newwidth = (newwidth + m_rectCrop.width()) / 2;
            m_rectCrop.setWidth( newwidth + 1);
            setOptionWidgetWidth( newwidth );
            m_rectCrop.setHeight( (int) (newwidth / m_optWidget->doubleRatio->value()) + 1 );
            setOptionWidgetHeight( m_rectCrop.height() - 1 );
        }
        validateSelection(false);
        paintOutlineWithHandles();
    }
}

void KisToolCrop::setOptionWidgetX(qint32 x)
{
    // Disable signals otherwise we get the valueChanged signal, which we don't want
    // to go through the logic for setting values that way.
    m_optWidget->intX->blockSignals(true);
    m_optWidget->intX->setValue(x);
    m_optWidget->intX->blockSignals(false);
}

void KisToolCrop::setOptionWidgetY(qint32 y)
{
    m_optWidget->intY->blockSignals(true);
    m_optWidget->intY->setValue(y);
    m_optWidget->intY->blockSignals(false);
}

void KisToolCrop::setOptionWidgetWidth(qint32 x)
{
    m_optWidget->intWidth->blockSignals(true);
    m_optWidget->intWidth->setValue(x);
    m_optWidget->intWidth->blockSignals(false);
}

void KisToolCrop::setOptionWidgetHeight(qint32 y)
{
    m_optWidget->intHeight->blockSignals(true);
    m_optWidget->intHeight->setValue(y);
    m_optWidget->intHeight->blockSignals(false);
}

void KisToolCrop::setOptionWidgetRatio(double ratio)
{
    m_optWidget->doubleRatio->blockSignals(true);
    m_optWidget->doubleRatio->setValue(ratio);
    m_optWidget->doubleRatio->blockSignals(false);
}


QWidget* KisToolCrop::createOptionWidget(QWidget* parent)
{
    m_optWidget = new WdgToolCrop(parent);
    Q_CHECK_PTR(m_optWidget);

    connect(m_optWidget->bnCrop, SIGNAL(clicked()), this, SLOT(crop()));

    connect(m_optWidget->intX, SIGNAL(valueChanged(int)), this, SLOT(setCropX(int)));
    connect(m_optWidget->intY, SIGNAL(valueChanged(int)), this, SLOT(setCropY(int)));
    connect(m_optWidget->intWidth, SIGNAL(valueChanged(int)), this, SLOT(setCropWidth(int)));
    connect(m_optWidget->intHeight, SIGNAL(valueChanged(int)), this, SLOT(setCropHeight(int)));
    connect(m_optWidget->doubleRatio, SIGNAL(valueChanged(double)), this, SLOT(setRatio( double )));

    return m_optWidget;
}

QWidget* KisToolCrop::optionWidget()
{
    return m_optWidget;
}

void KisToolCrop::setup(KActionCollection *collection)
{
    m_action = collection->action(objectName());

    if (m_action == 0) {
        m_action = new KAction(KIcon("crop"),
                               i18n("&Crop"),
                               collection,
                               objectName());
        Q_CHECK_PTR(m_action);
        connect(m_action, SIGNAL(triggered()), this, SLOT(activate()));
        m_action->setToolTip(i18n("Crop an area"));
        m_action->setActionGroup(actionGroup());

        m_ownAction = true;
    }
}

QRect toQRect(double x, double y, int w, int h)
{
    return QRect(int(x), int(y), w, h);
}

QRegion KisToolCrop::handles(QRect rect)
{
    QRegion handlesRegion;

    //add handle at the lower right corner
    handlesRegion += toQRect( QABS( rect.width() ) - m_handleSize / 2.0, QABS( rect.height() ) - m_handleSize / 2.0, m_handleSize, m_handleSize );
    //add handle at the upper right corner
    handlesRegion += toQRect( QABS( rect.width() ) - m_handleSize / 2.0 , 0 - m_handleSize / 2.0, m_handleSize, m_handleSize );
    //add rectangle at the lower left corner
    handlesRegion += toQRect( 0 - m_handleSize / 2.0 , QABS( rect.height() ) - m_handleSize / 2.0, m_handleSize, m_handleSize );
    //add rectangle at the upper left corner
    handlesRegion += toQRect( 0 - m_handleSize / 2.0, 0 - m_handleSize / 2.0, m_handleSize, m_handleSize );
    //add handle at the lower edge of the rectangle
    handlesRegion += toQRect( ( QABS( rect.width() ) - m_handleSize ) / 2.0 , QABS( rect.height() ) - m_handleSize / 2.0, m_handleSize, m_handleSize );
    //add handle at the right edge of the rectangle
    handlesRegion += toQRect( QABS( rect.width() ) - m_handleSize / 2.0 , ( QABS( rect.height() ) - m_handleSize ) / 2.0, m_handleSize, m_handleSize );
    //add handle at the upper edge of the rectangle
    handlesRegion += toQRect( ( QABS( rect.width() ) - m_handleSize ) / 2.0 , 0 - m_handleSize / 2.0, m_handleSize, m_handleSize );
    //add handle at the left edge of the rectangle
    handlesRegion += toQRect( 0 - m_handleSize / 2.0, ( QABS( rect.height() ) - m_handleSize ) / 2.0, m_handleSize, m_handleSize );

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

qint32 KisToolCrop::mouseOnHandle(QPoint currentViewPoint)
{
    KisCanvasController *controller = m_subject->canvasController();
    Q_ASSERT(controller);
    QPoint start = controller->windowToView(m_rectCrop.topLeft());
    QPoint end = controller->windowToView(m_rectCrop.bottomRight());

    qint32 startx;
    qint32 starty;
    qint32 endx;
    qint32 endy;
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

    if ( toQRect ( startx - m_handleSize / 2.0, starty - m_handleSize / 2.0, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
    {
        if( !m_selecting )
        {
            m_dx= startx-currentViewPoint.x();
            m_dy = starty - currentViewPoint.y();
        }
        return UpperLeft;
    }
    else if ( toQRect ( startx - m_handleSize / 2.0, endy -  m_handleSize / 2.0, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
    {
        if( !m_selecting )
        {
            m_dx = startx-currentViewPoint.x();
            m_dy = endy-currentViewPoint.y();
        }
        return LowerLeft;
    }
    else if ( toQRect ( endx - m_handleSize / 2.0, starty - m_handleSize / 2.0, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
    {
        if( !m_selecting )
        {
            m_dx = endx - currentViewPoint.x();
            m_dy = starty - currentViewPoint.y() ;
        }
        return UpperRight;
    }
    else if ( toQRect ( endx - m_handleSize / 2.0, endy - m_handleSize / 2.0, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
    {
        if( !m_selecting )
        {
            m_dx = endx - currentViewPoint.x();
            m_dy= endy - currentViewPoint.y();
        }
        return LowerRight;
    }
    else if ( toQRect ( startx + ( endx - startx - m_handleSize ) / 2.0, starty - m_handleSize / 2.0, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
    {
        if( !m_selecting )
        {
            m_dy = starty - currentViewPoint.y() ;
        }
        return Upper;
    }
    else if ( toQRect ( startx + ( endx - startx - m_handleSize ) / 2.0, endy - m_handleSize / 2, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
    {
        if( !m_selecting )
        {
            m_dy = endy - currentViewPoint.y();
        }
        return Lower;
    }
    else if ( toQRect ( startx - m_handleSize / 2.0, starty + ( endy - starty - m_handleSize ) / 2.0, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
    {
        if( !m_selecting )
        {
            m_dx = startx - currentViewPoint.x() ;
        }
        return Qt::DockLeft;
    }
    else if ( toQRect ( endx - m_handleSize / 2.0 , starty + ( endy - starty - m_handleSize ) / 2.0, m_handleSize, m_handleSize ).contains( currentViewPoint ) )
    {
        if( !m_selecting )
        {
            m_dx = endx-currentViewPoint.x();
        }
        return Qt::DockRight;
    }
    else if ( toQRect ( startx , starty, endx - startx , endy - starty ).contains( currentViewPoint ) )
    {
        return Inside;
    }
    else return None;
}

void KisToolCrop::setMoveResizeCursor (qint32 handle)
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
    m_subject->canvasController()->setCanvasCursor(KisCursor::arrowCursor());
    return;
}


#include "kis_tool_crop.moc"
