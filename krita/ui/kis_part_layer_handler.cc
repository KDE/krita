/*
 *  Copyright (c) 2006 Bart Coppens <kde@bartcoppens.be>
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

#include "kis_canvas.h"
#include <fixx11h.h> // kis_canvas.h does X11 stuff

#include <QPainter>
#include <QCursor>
//Added by qt3to4:
#include <QKeyEvent>

#include "kis_cursor.h"
#include "kis_canvas_painter.h"
#include "kis_move_event.h"
#include "kis_button_press_event.h"
#include "kis_button_release_event.h"
#include "kis_group_layer.h"
#include "kis_part_layer_handler.h"

KisPartLayerHandler::KisPartLayerHandler(KisView* view, const KoDocumentEntry& entry,
                                         KisGroupLayerSP parent, KisLayerSP above)
    : m_parent(parent), m_above(above), m_view(view), m_entry(entry) {
    m_started = false;
    view->getCanvasController()->setCanvasCursor( KisCursor::selectCursor() );
}

void KisPartLayerHandler::done() {
    emit handlerDone(); // We will get deleted by the view
}

void KisPartLayerHandler::gotMoveEvent(KisMoveEvent* event) {
    if (!m_started) {
        emit sigGotMoveEvent(event);
        return;
    }

    KisCanvasPainter painter(m_view->getCanvasController()->kiscanvas());
    //painter.setRasterOp( NotROP );

    // erase old lines
    QRect r(m_start, m_end);
    r = r.normalized();
    if (!r.isEmpty())
        painter.drawRect(r);

    m_end = event->pos().roundQPoint();
    r = QRect(m_start, m_end).normalized();

    painter.drawRect(r);
    painter.end();
}

void KisPartLayerHandler::gotButtonPressEvent(KisButtonPressEvent* event) {
    m_start = event->pos().roundQPoint();
    m_end = m_start;
    m_started = true;
}

void KisPartLayerHandler::gotButtonReleaseEvent(KisButtonReleaseEvent* event) {
    if (!m_started) {
        done();
        return;
    }

    m_end = event->pos().roundQPoint();

    QRect r(m_start, m_end);

    m_view->insertPart(r.normalized(), m_entry, m_parent, m_above);
    // We will get deleted by the view through the above
}

void KisPartLayerHandler::gotKeyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        done();
    } else {
        emit sigGotKeyPressEvent(event);
    }
}

#include "kis_part_layer_handler.moc"
