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

#include "kis_selection.h"
#include "kis_painter.h"
#include "kis_fill_painter.h"
#include "kis_tool_freehand.h"
#include "kis_cursor.h"
#include "kis_doc.h"
#include "kis_view.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_move_event.h"
#include "kis_layer.h"

KisToolFreehand::KisToolFreehand(QString transactionText)
        : super(transactionText),
        m_dragDist ( 0 ),
        m_transactionText(transactionText),
        m_mode( HOVER )
{
    m_painter = 0;
    m_currentImage = 0;

    m_useTempLayer = false;
}

KisToolFreehand::~KisToolFreehand()
{
}

void KisToolFreehand::update(KisCanvasSubject *subject)
{
    super::update(subject);
    m_currentImage = m_subject -> currentImg();
}

void KisToolFreehand::buttonPress(KisButtonPressEvent *e)
{
    if (!m_subject) return;

    if (!m_subject -> currentBrush()) return;

    if (!m_currentImage || !m_currentImage -> activeDevice()) return;

    if (e -> button() == QMouseEvent::LeftButton) {

        initPaint(e);

        paintAt(e -> pos(), e -> pressure(), e -> xTilt(), e -> yTilt());

        m_prevPos = e -> pos();
        m_prevPressure = e -> pressure();
        m_prevXTilt = e -> xTilt();
        m_prevYTilt = e -> yTilt();

         QRect r = m_painter -> dirtyRect();
         if ( r.isValid() ) {
             m_dirtyRect = r;
             m_currentImage -> notify(r);
         }
    }
}

void KisToolFreehand::buttonRelease(KisButtonReleaseEvent* e)
{
    if (e -> button() == QMouseEvent::LeftButton && m_mode == PAINT) {
        endPaint();
    }
}

void KisToolFreehand::move(KisMoveEvent *e)
{
    if (m_mode == PAINT) {
        paintLine(m_prevPos, m_prevPressure, m_prevXTilt, m_prevYTilt, e -> pos(), e -> pressure(), e -> xTilt(), e -> yTilt());

        m_prevPos = e -> pos();
        m_prevPressure = e -> pressure();
        m_prevXTilt = e -> xTilt();
        m_prevYTilt = e -> yTilt();

        QRect r = m_painter -> dirtyRect();
        m_dirtyRect |= r;
        m_currentImage -> notify(r);
    }
}

void KisToolFreehand::initPaint(KisEvent *)
{
    if (!m_currentImage || !m_currentImage -> activeDevice()) return;

    m_mode = PAINT;
    m_dragDist = 0;

    // Create painter
    KisPaintDeviceImplSP device;
    if (m_currentImage && (device = m_currentImage -> activeDevice())) {
        if (m_painter)
            delete m_painter;
        if (m_useTempLayer) {
            if (m_currentImage -> undoAdapter())
                m_currentImage -> undoAdapter() -> beginMacro(m_transactionText);

            // XXX ugly! hacky!
            m_target = m_currentImage->layerAdd("temp", OPACITY_OPAQUE);

            m_target -> setCompositeOp(m_compositeOp);

            if (device -> hasSelection()) {
                m_target -> addSelection(device -> selection());
            }

            dynamic_cast<KisLayer*>(m_target.data()) -> setVisible(true);

            // XXX doesn't look very good I'm afraid
            currentImage() -> add(dynamic_cast<KisLayer*>(m_target.data()),
                currentImage() -> index(dynamic_cast<KisLayer*>(device.data())) + 1);
            m_target = currentImage() -> activate(dynamic_cast<KisLayer*>(m_target.data()));
            currentImage() -> notify();
        } else {
            m_target = device;
        }
        m_painter = new KisPainter( m_target );
        Q_CHECK_PTR(m_painter);
        m_source = device;
        m_painter -> beginTransaction(m_transactionText);
    }

    m_painter -> setPaintColor(m_subject -> fgColor());
    m_painter -> setBackgroundColor(m_subject -> bgColor());
    m_painter -> setBrush(m_subject -> currentBrush());

    m_painter -> setOpacity(m_opacity);

    // if you're drawing on a temporary layer, the layer already sets this
    if (m_useTempLayer) {
        m_painter -> setCompositeOp(COMPOSITE_OVER);
    } else {
        m_painter -> setCompositeOp(m_compositeOp);
    }

#if 0
    // Setting cursors has no effect until the tool is selected again; this
    // should be fixed.
    setCursor(KisCursor::brushCursor());
#endif
}

void KisToolFreehand::endPaint()
{
    m_mode = HOVER;
    if (m_currentImage) {
        KisUndoAdapter *adapter = m_currentImage -> undoAdapter();
        if (adapter && m_painter) {
            // If painting in mouse release, make sure painter
            // is destructed or end()ed
            if (m_useTempLayer) {
                m_painter -> endTransaction();
                KisPainter painter( m_source );
                painter.setCompositeOp(m_compositeOp);
                painter.beginTransaction(m_transactionText);
                 painter.bitBlt(m_dirtyRect.x(), m_dirtyRect.y(), m_compositeOp, m_target, OPACITY_OPAQUE,
                           m_dirtyRect.x(), m_dirtyRect.y(), m_dirtyRect.width(), m_dirtyRect.height());

                adapter -> addCommand(painter.endTransaction());
                m_currentImage->layerRemove(dynamic_cast<KisLayer*>(m_target.data()));
                currentImage() -> activate(dynamic_cast<KisLayer*>(m_source.data()));
                adapter -> endMacro();
            } else {
                adapter -> addCommand(m_painter->endTransaction());
            }
        }
        delete m_painter;
        m_painter = 0;
        notifyModified();
    }
}

void KisToolFreehand::paintAt(const KisPoint &pos,
               const double pressure,
               const double xTilt,
               const double yTilt)
{
    painter() -> paintAt(pos, pressure, xTilt, yTilt);
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
    m_dragDist = painter() -> paintLine(pos1, pressure1, xtilt1, ytilt1, pos2, pressure2, xtilt2, ytilt2, m_dragDist);
}


KisImageSP KisToolFreehand::currentImage()
{
    return m_currentImage;
}

void KisToolFreehand::setUseTempLayer(bool u) {
    m_useTempLayer = u;
}

#include "kis_tool_freehand.moc"

