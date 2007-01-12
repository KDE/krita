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
#include <QEvent>
#include <QLabel>
#include <QLayout>
#include <QWidget>
#include <QRect>
#include "QPainter"

#include <kdebug.h>
#include <kaction.h>
#include <kcommand.h>
#include <klocale.h>

#include <KoPointerEvent.h>
#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>

#include "kis_brush.h"
#include "kis_fill_painter.h"
#include "kis_group_layer.h"
#include "kis_layer.h"
#include "kis_paint_layer.h"
#include "kis_painter.h"
#include "kis_selection.h"
#include "kis_undo_adapter.h"

#include "kis_boundary_painter.h"
#include "kis_canvas2.h"
#include "kis_cursor.h"
#include "kis_tool_freehand.h"

KisToolFreehand::KisToolFreehand(KoCanvasBase * canvas, const QCursor & cursor, const QString & transactionText)
        : KisToolPaint(canvas, cursor)
        , m_dragDist ( 0 )
        , m_transactionText(transactionText)
        , m_mode( HOVER )
{
    m_painter = 0;
    m_tempLayer = 0;
    m_paintIncremental = true;
    m_paintOnSelection = false;
    m_paintedOutline = false;
}

KisToolFreehand::~KisToolFreehand()
{
}


void KisToolFreehand::mousePressEvent(KoPointerEvent *e)
{
    if (!m_currentImage) return;

    if (!m_currentBrush) return;

    if (!m_currentImage->activeDevice()) return;

    if (e->button() == Qt::LeftButton) {

       initPaint(e);

        m_prevPos = convertToPixelCoord(e);
        paintAt(m_prevPos, e->pressure(), e->xTilt(), e->yTilt());

        m_prevPressure = e->pressure();
        m_prevXTilt = e->xTilt();
        m_prevYTilt = e->yTilt();

        QRegion r = m_painter->dirtyRegion();
        kDebug() << "KisToolFreehand::mousePressEvent " << r.boundingRect() << endl;

        if (!m_paintOnSelection) {
            if (!m_paintIncremental) {
                m_currentImage->activeLayer()->setDirty(r);
            }
        }
        else {
            m_target->setDirty( r);
        }
    }
}

void KisToolFreehand::mouseMoveEvent(KoPointerEvent *e)
{
    if (m_mode == PAINT) {
        QPointF pos = convertToPixelCoord(e);

        paintLine(m_prevPos, m_prevPressure, m_prevXTilt, m_prevYTilt, pos, e->pressure(), e->xTilt(), e->yTilt());

        m_prevPos = pos;
        m_prevPressure = e->pressure();
        m_prevXTilt = e->xTilt();
        m_prevYTilt = e->yTilt();

        QRegion region = m_painter->dirtyRegion();
        kDebug() << "KisToolFreehand::mouseNoveEvent: " << region.boundingRect() << endl;

        if (!m_paintOnSelection) {
            m_currentImage->activeLayer()->setDirty(region);
        }
        else {
            // Just update the canvas
            // XXX: How to do this hack with regions?
            // r = QRect(r.left()-1, r.top()-1, r.width()+2, r.height()+2); //needed to update selectionvisualization
            m_target->setDirty(region);
        }
    }
}

void KisToolFreehand::mouseReleaseEvent(KoPointerEvent* e)
{
    if (e->button() == Qt::LeftButton && m_mode == PAINT) {
        endPaint();
    }
}



void KisToolFreehand::initPaint(KoPointerEvent *)
{
    if (!m_currentImage || !m_currentImage->activeDevice()) return;

    if (m_compositeOp == 0 ) {
        KisPaintDeviceSP device = m_currentImage->activeDevice();
        if (device) {
            m_compositeOp = device->colorSpace()->compositeOp( COMPOSITE_OVER );
        }
    }

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

            KisLayerSupportsIndirectPainting* layer;
            if ((layer = dynamic_cast<KisLayerSupportsIndirectPainting*>(
                    m_currentImage->activeLayer().data()))) {
                // Hack for the painting of single-layered layers using indirect painting,
                // because the group layer would not have a correctly synched cache (
                // because of an optimization that would happen, having this layer as
                // projection).
                KisLayerSP l = layer->layer();
                KisPaintLayerSP pl = dynamic_cast<KisPaintLayer*>(l.data());
                if (l->parent() && (l->parent()->parent() == 0)
                    && (l->parent()->childCount() == 1)
                    && l->parent()->paintLayerInducesProjectionOptimization(pl)) {
                    // If there's a mask, device could've been the mask. The induce function
                    // should catch this, but better safe than sorry
                    l->parent()->resetProjection(pl->paintDevice());
                }

                m_target = new KisPaintDevice(m_currentImage->activeLayer().data(),
                                              device->colorSpace());
                layer->setTemporaryTarget(m_target);
                layer->setTemporaryCompositeOp(m_compositeOp);
                layer->setTemporaryOpacity(m_opacity);

                if (device->hasSelection())
                    m_target->setSelection(device->selection());
            }
        } else {
            m_target = device;
        }
        m_painter = new KisPainter( m_target );
        Q_CHECK_PTR(m_painter);
        m_source = device;
        if (m_currentImage->undo()) m_painter->beginTransaction(m_transactionText);
    }

    m_painter->setPaintColor(m_currentFgColor);
    m_painter->setBackgroundColor(m_currentBgColor);
    m_painter->setBrush(m_currentBrush);


    // if you're drawing on a temporary layer, the layer already sets this
    if (m_paintIncremental) {
        m_painter->setCompositeOp(m_compositeOp);
        m_painter->setOpacity(m_opacity);
    } else {
        m_painter->setCompositeOp(device->colorSpace()->compositeOp(COMPOSITE_ALPHA_DARKEN));
        m_painter->setOpacity( OPACITY_OPAQUE );

    }

/*    kDebug(41007) << "target: " << m_target << "( " << m_target->name() << " )"
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
                if (m_currentImage->undo())
                    m_painter->endTransaction();
                KisPainter painter( m_source );
                painter.setCompositeOp(m_compositeOp);

                if (m_currentImage->undo())
                    painter.beginTransaction(m_transactionText);

                QRegion r = painter.dirtyRegion();
                kDebug() << "endPaint: " << r.boundingRect() << endl;
                QVector<QRect> dirtyRects = r.rects();
                QVector<QRect>::iterator it = dirtyRects.begin();
                QVector<QRect>::iterator end = dirtyRects.end();

                while (it != end) {

                    painter.bitBlt(it->x(), it->y(), m_compositeOp, m_target,
                                   m_opacity,
                                   it->x(), it->y(),
                                   it->width(), it->height());
                    ++it;
                }
                KisLayerSupportsIndirectPainting* layer =
                    dynamic_cast<KisLayerSupportsIndirectPainting*>(m_source->parentLayer());
                layer->setTemporaryTarget(0);
                m_source->parentLayer()->setDirty(painter.dirtyRegion());

                if (m_currentImage->undo()) {
                    m_currentImage->undoAdapter()->addCommand(painter.endTransaction());
                    m_currentImage->undoAdapter()->endMacro();
                }
            } else {
                if (m_currentImage->undo())
                    m_currentImage->undoAdapter()->addCommand(m_painter->endTransaction());
            }
        }
        delete m_painter;
        m_painter = 0;
        notifyModified();
    }
}

void KisToolFreehand::paintAt(const QPointF &pos,
               const double pressure,
               const double xTilt,
               const double yTilt)
{
    m_painter->paintAt(pos, pressure, xTilt, yTilt);
}

void KisToolFreehand::paintLine(const QPointF & pos1,
                 const double pressure1,
                 const double xtilt1,
                 const double ytilt1,
                 const QPointF & pos2,
                 const double pressure2,
                 const double xtilt2,
                 const double ytilt2)
{
    m_dragDist = m_painter->paintLine(pos1, pressure1, xtilt1, ytilt1, pos2, pressure2, xtilt2, ytilt2, m_dragDist);
}


void KisToolFreehand::paintOutline(const QPointF& point) {
    Q_UNUSED( point );
#if 0
    // XXX TOOL_PORT
    // I don't get this 1.6 code: we update the whole canvas, and then
    // repaint all of it again? It's incompatible with qt4 anyway --
    // rework later.

    if (!m_canvas) {
        return;
    }

    if (m_currentImage && !m_currentImage->bounds().contains(point.floorQPoint())) {
        if (m_paintedOutline) {
            m_canvas->updateCanvas(); // Huh? The _whole_ canvas needs to be
                                // repainted for this outline cursor?
                                // It was this way in 1.6, btw.
            m_paintedOutline = false;
        }
        return;
    }

    KisCanvas *canvas = controller->kiscanvas();
    canvas->repaint();

    KisBrush *brush = m_subject->currentBrush();
    // There may not be a brush present, and we shouldn't crash in that case
    if (brush) {
        QPainter gc( canvas->canvasWidget() );
        QPen pen(Qt::SolidLine);

        QPointF hotSpot = brush->hotSpot();

        //gc.setRasterOp(Qt::NotROP);
        gc.setPen(pen);
        gc.setViewport(0, 0, static_cast<qint32>(canvas->width() * m_subject->zoomFactor()),
                       static_cast<qint32>(canvas->height() * m_subject->zoomFactor()));
        gc.translate((- controller->horzValue()) / m_subject->zoomFactor(),
                        (- controller->vertValue()) / m_subject->zoomFactor());

        QPointF topLeft = point - hotSpot;

        if (m_subject->currentPaintop().id() == "pen") {
            // Pen paints on whole pixels only.
            topLeft = topLeft.roundQPoint();
        }

        gc.translate(topLeft.x(), topLeft.y());
        gc.end();
        KisBoundaryPainter::paint(brush->boundary(), gc);

        m_paintedOutline = true;
    }
#endif
}


#include "kis_tool_freehand.moc"

