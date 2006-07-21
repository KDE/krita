/*
 *  kis_tool_brush.cc - part of Krita
 *
 *  Copyright (c) 2003-2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Bart Coppens <kde@bartcoppens.be>
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
#include <qevent.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qwidget.h>
#include <qrect.h>

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

#include "kis_canvas_subject.h"
#include "kis_undo_adapter.h"
#include "kis_selection.h"
#include "kis_painter.h"
#include "kis_fill_painter.h"
#include "kis_tool_freehand.h"
#include "kis_cursor.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_paint_layer.h"
#include "kis_canvas.h"
#include "kis_canvas_painter.h"
#include "kis_boundary_painter.h"
#include "kis_brush.h"

KisToolFreehand::KisToolFreehand(QString transactionText)
        : super(transactionText),
        m_dragDist ( 0 ),
        m_transactionText(transactionText),
        m_mode( HOVER )
{
    m_painter = 0;
    m_currentImage = 0;
    m_tempLayer = 0;
    m_paintIncremental = true;
    m_paintOnSelection = false;
    m_paintedOutline = false;
}

KisToolFreehand::~KisToolFreehand()
{
}

void KisToolFreehand::update(KisCanvasSubject *subject)
{
    super::update(subject);
    m_currentImage = m_subject->currentImg();
}

void KisToolFreehand::buttonPress(KisButtonPressEvent *e)
{
    if (!m_subject) return;

    if (!m_subject->currentBrush()) return;

    if (!m_currentImage || !m_currentImage->activeDevice()) return;

    if (e->button() == QMouseEvent::LeftButton) {
        
        
        if (!m_currentImage->bounds().contains(e->pos().floorQPoint())) return;
        
        initPaint(e);
        paintAt(e->pos(), e->pressure(), e->xTilt(), e->yTilt());

        m_prevPos = e->pos();
        m_prevPressure = e->pressure();
        m_prevXTilt = e->xTilt();
        m_prevYTilt = e->yTilt();

        QRect r = m_painter->dirtyRect();
        if ( r.isValid() ) {
            m_dirtyRect = r;

            r = QRect(r.left()-1, r.top()-1, r.width()+2, r.height()+2); //needed to update selectionvisualization
            if (!m_paintOnSelection) {
                if (!m_paintIncremental) {
                    m_tempLayer->setDirty(r);
                }
                else {
                    m_currentImage->activeLayer()->setDirty(r);
                }
            }
            else {
                m_target->setDirty(r);
                // Just update the canvas. XXX: After 1.5, find a better way to make sure tools don't set dirty what they didn't touch.
                m_subject->canvasController()->updateCanvas( r );
            } 
        }
    }
}

void KisToolFreehand::buttonRelease(KisButtonReleaseEvent* e)
{
    if (e->button() == QMouseEvent::LeftButton && m_mode == PAINT) {
        endPaint();
    }
    KisToolPaint::buttonRelease(e);
}

void KisToolFreehand::move(KisMoveEvent *e)
{
    if (m_mode == PAINT) {

        paintLine(m_prevPos, m_prevPressure, m_prevXTilt, m_prevYTilt, e->pos(), e->pressure(), e->xTilt(), e->yTilt());
    
        m_prevPos = e->pos();
        m_prevPressure = e->pressure();
        m_prevXTilt = e->xTilt();
        m_prevYTilt = e->yTilt();

        QRect r = m_painter->dirtyRect();

        if (r.isValid()) {
            m_dirtyRect |= r;

            if (!m_paintOnSelection) {
                if (!m_paintIncremental) {
                    m_tempLayer->setDirty(r);
                }
                else {
                    m_currentImage->activeLayer()->setDirty(r);
                }
            }
            else {
                // Just update the canvas
                r = QRect(r.left()-1, r.top()-1, r.width()+2, r.height()+2); //needed to update selectionvisualization
                m_target->setDirty(r);
                m_subject->canvasController()->updateCanvas( r );
            } 
        }
    }
}

void KisToolFreehand::initPaint(KisEvent *)
{
    if (!m_currentImage || !m_currentImage->activeDevice()) return;

    m_mode = PAINT;
    m_dragDist = 0;

    // Create painter
    KisPaintDeviceSP device;
    if (m_currentImage && (device = m_currentImage->activeDevice())) {
        
        if (m_painter)
            delete m_painter;
        
        if (!m_paintIncremental) {
            if (m_currentImage->undo())
                m_currentImage->undoAdapter()->beginMacro(m_transactionText);

            //if (m_tempLayer == 0) {
                // XXX ugly! hacky! We'd like to cache the templayer, but that makes sure
                // the layer is never really removed from its parent group because of shared pointers
                m_tempLayer = new KisPaintLayer(m_currentImage, "temp", OPACITY_OPAQUE);
                m_tempLayer->setTemporary(true);
                // Yuck, what an ugly cast!
                m_target = (dynamic_cast<KisPaintLayer*>(m_tempLayer.data()))->paintDevice();
            //}

            m_tempLayer->setCompositeOp( m_compositeOp );
            m_tempLayer->setOpacity( m_opacity );

            m_tempLayer->setVisible(true);

            currentImage()->addLayer(m_tempLayer, m_currentImage->activeLayer()->parent().data(), m_currentImage->activeLayer());

        } else {
            m_target = device;
        }
        if(m_target->hasSelection()) m_target->selection()->startCachingExactRect();
        m_painter = new KisPainter( m_target );
        Q_CHECK_PTR(m_painter);
        m_source = device;
        if (currentImage()->undo()) m_painter->beginTransaction(m_transactionText);
    }

    m_painter->setPaintColor(m_subject->fgColor());
    m_painter->setBackgroundColor(m_subject->bgColor());
    m_painter->setBrush(m_subject->currentBrush());


    // if you're drawing on a temporary layer, the layer already sets this
    if (m_paintIncremental) {
        m_painter->setCompositeOp(m_compositeOp);
        m_painter->setOpacity(m_opacity);
    } else {
        m_painter->setCompositeOp(COMPOSITE_OVER);
        m_painter->setOpacity( OPACITY_OPAQUE );

    }

/*    kdDebug() << "target: " << m_target << "( " << m_target->name() << " )"
            << " source: " << m_source << "( " << m_source->name() << " )"
            << ", incremental " << m_paintIncremental
            << ", paint on selection: " << m_paintOnSelection
            << ", active device has selection: " << device->hasSelection()
            << ", target has selection: " << m_target->hasSelection()
            << endl;
*/
}

void KisToolFreehand::endPaint()
{
    m_mode = HOVER;
    if (m_currentImage) {
        
        if (m_painter) {
            // If painting in mouse release, make sure painter
            // is destructed or end()ed
            if (!m_paintIncremental) {
                if (m_currentImage->undo()) m_painter->endTransaction();
                KisPainter painter( m_source );
                painter.setCompositeOp(m_compositeOp);
                if (m_currentImage->undo()) painter.beginTransaction(m_transactionText);
                painter.bitBlt(m_dirtyRect.x(), m_dirtyRect.y(), m_compositeOp, m_target, m_opacity,
                               m_dirtyRect.x(), m_dirtyRect.y(), m_dirtyRect.width(), m_dirtyRect.height());

                if (m_currentImage->undo()) m_currentImage->undoAdapter()->addCommand(painter.endTransaction());
                m_currentImage->removeLayer(m_tempLayer);
                // The shared ptr layer vector in the group keeps the layer from being
                // being removed if it isn't removed here. It would be much faster to
                // keep the layer and clear it, but that isn't possible. Replacing the
                // old templayer with a new one at the end of paint prevents at least
                // the pause when painting again.
                //m_target->clear();
                if (m_currentImage->undo()) m_currentImage->undoAdapter()->endMacro();
            } else {
                if (m_currentImage->undo()) m_currentImage->undoAdapter()->addCommand(m_painter->endTransaction());
            }
        }
        delete m_painter;
        m_painter = 0;
        notifyModified();
        if(m_target->hasSelection()) m_target->selection()->stopCachingExactRect();
    }
}

void KisToolFreehand::paintAt(const KisPoint &pos,
               const double pressure,
               const double xTilt,
               const double yTilt)
{
    painter()->paintAt(pos, pressure, xTilt, yTilt);
}

void KisToolFreehand::paintLine(const KisPoint & pos1,
                 const double pressure1,
                 const double xtilt1,
                 const double ytilt1,
                 const KisPoint & pos2,
                 const double pressure2,
                 const double xtilt2,
                 const double ytilt2)
{
    m_dragDist = painter()->paintLine(pos1, pressure1, xtilt1, ytilt1, pos2, pressure2, xtilt2, ytilt2, m_dragDist);
}


KisImageSP KisToolFreehand::currentImage()
{
    return m_currentImage;
}


void KisToolFreehand::paintOutline(const KisPoint& point) {
    if (!m_subject) {
        return;
    }

    KisCanvasController *controller = m_subject->canvasController();

    if (currentImage() && !currentImage()->bounds().contains(point.floorQPoint())) {
        if (m_paintedOutline) {
            controller->kiscanvas()->update();
            m_paintedOutline = false;
        }
        return;
    }

    KisCanvas *canvas = controller->kiscanvas();
    canvas->repaint();

    KisBrush *brush = m_subject->currentBrush();
    // There may not be a brush present, and we shouldn't crash in that case
    if (brush) {
        KisCanvasPainter gc(canvas);
        QPen pen(Qt::SolidLine);

        KisPoint hotSpot = brush->hotSpot();

        gc.setRasterOp(Qt::NotROP);
        gc.setPen(pen);
        gc.setViewport(0, 0, static_cast<Q_INT32>(canvas->width() * m_subject->zoomFactor()),
                       static_cast<Q_INT32>(canvas->height() * m_subject->zoomFactor()));
        gc.translate((- controller->horzValue()) / m_subject->zoomFactor(),
                        (- controller->vertValue()) / m_subject->zoomFactor());

        KisPoint topLeft = point - hotSpot;

        if (m_subject->currentPaintop().id() == "pen") {
            // Pen paints on whole pixels only.
            topLeft = topLeft.roundQPoint();
        }

        gc.translate(topLeft.x(), topLeft.y());

        KisBoundaryPainter::paint(brush->boundary(), gc);
        m_paintedOutline = true;
    }
}


#include "kis_tool_freehand.moc"

