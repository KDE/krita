/*
 *  Copyright (c) 2006 Bart Coppens <kde@bartcoppens.be>
 *
 *  This program is free software; you can redistribute it and/or modify it
 *  under the terms of the GNU General Public License as published by the Free
 *  Software Foundation; either version 2 of the License, or (at your option)
 *  any later version.
 *
 *  This program is distributed in the hope that it will be useful, but WITHOUT
 *  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 *  FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 *  more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc., 51
 *  Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_part_layer_handler.h"

//#include "kis_canvas.h"
//#include <fixx11h.h> // kis_canvas.h does X11 stuff

#include <QPainter>
#include <QCursor>
//Added by qt3to4:
#include <QKeyEvent>

#include "kis_cursor.h"
#include "KoPointerEvent.h"
#include "kis_group_layer.h"
#include "kis_view2.h"

KisPartLayerHandler::KisPartLayerHandler(KisView2* view, const KoDocumentEntry& entry,
                                         KisGroupLayerSP parent, KisLayerSP above)
    : m_parent(parent), m_above(above), m_view(view), m_entry(entry) {
    m_started = false;
    //view->canvasController()->setCanvasCursor( KisCursor::selectCursor() );
}

void KisPartLayerHandler::done() {
    emit handlerDone(); // We will get deleted by the view
}

void KisPartLayerHandler::gotMoveEvent(KoPointerEvent* event) {
    if (!m_started) {
        emit sigGotMoveEvent(event);
        return;
    }

#warning "Port or remove the part layers!"
# if 0    
    QPainter painter( m_view->canvasController()->kiscanvas()->canvasWidget() );
    painter.setRasterOp( NotROP );

    // erase old lines
    QRect r(m_start, m_end);
    r = r.normalized();
    if (!r.isEmpty())
        painter.drawRect(r);

    m_end = event->pos().roundQPoint();
    r = QRect(m_start, m_end).normalized();

    painter.drawRect(r);
    painter.end();
#endif    
}

void KisPartLayerHandler::gotButtonPressEvent(KoPointerEvent* event) 
{
    Q_UNUSED(event);
#if 0
    m_start = event->pos().roundQPoint();
    m_end = m_start;
    m_started = true;
#endif    
}

void KisPartLayerHandler::gotButtonReleaseEvent(KoPointerEvent* event)
{
    Q_UNUSED(event);
    if (!m_started) {
        done();
        return;
    }
#if 0
    m_end = event->pos().roundQPoint();

    QRect r(m_start, m_end);
    m_view->insertPart(r.normalized(), m_entry, m_parent, m_above);
    // We will get deleted by the view through the above
#endif 
}

void KisPartLayerHandler::gotKeyPressEvent(QKeyEvent* event) {
    if (event->key() == Qt::Key_Escape) {
        done();
    } else {
        emit sigGotKeyPressEvent(event);
    }
}

#include "kis_part_layer_handler.moc"
