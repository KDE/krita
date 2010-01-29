/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2006 Thorsten Zachmann <zachmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "KoZoomTool.h"
#include "KoZoomStrategy.h"
#include "KoZoomToolWidget.h"
#include "KoPointerEvent.h"
#include "KoCanvasBase.h"
#include "KoCanvasController.h"

#include <kstandarddirs.h>
#include <kdebug.h>

KoZoomTool::KoZoomTool(KoCanvasBase *canvas)
        : KoInteractionTool(canvas),
        m_temporary(false), m_zoomInMode(true)
{
    QPixmap inPixmap, outPixmap;
    inPixmap.load(KStandardDirs::locate("data", "koffice/icons/zoom_in_cursor.png"));
    outPixmap.load(KStandardDirs::locate("data", "koffice/icons/zoom_out_cursor.png"));
    m_inCursor = QCursor(inPixmap, 4, 4);
    m_outCursor = QCursor(outPixmap, 4, 4);
}

void KoZoomTool::wheelEvent(KoPointerEvent * event)
{
    // Let KoCanvasController handle this
    event->ignore();
}

void KoZoomTool::mouseReleaseEvent(KoPointerEvent *event)
{
    KoInteractionTool::mouseReleaseEvent(event);
    if (m_temporary) {
        emit KoToolBase::done();
    }
}

void KoZoomTool::mouseMoveEvent(KoPointerEvent *event)
{
    updateCursor(event->modifiers() & Qt::ControlModifier);

    if (currentStrategy()) {
        currentStrategy()->handleMouseMove(event->point, event->modifiers());
    }
}

void KoZoomTool::keyPressEvent(QKeyEvent *event)
{
    event->ignore();

    updateCursor(event->modifiers() & Qt::ControlModifier);
}

void KoZoomTool::keyReleaseEvent(QKeyEvent *event)
{
    event->ignore();

    updateCursor(event->modifiers() & Qt::ControlModifier);

    KoInteractionTool::keyReleaseEvent(event);
}

void KoZoomTool::activate(bool temporary)
{
    m_temporary = temporary;
    updateCursor(false);
}

void KoZoomTool::mouseDoubleClickEvent(KoPointerEvent *event)
{
    mousePressEvent(event);
}

KoInteractionStrategy *KoZoomTool::createStrategy(KoPointerEvent *event)
{
    KoZoomStrategy *zs = new KoZoomStrategy(this, m_controller, event->point);
    if (event->button() == Qt::RightButton) {
        if (m_zoomInMode) {
            zs->forceZoomOut();
        } else {
            zs->forceZoomIn();
        }
    } else {
        if (m_zoomInMode) {
            zs->forceZoomIn();
        } else {
            zs->forceZoomOut();
        }
    }
    return zs;
}

QWidget* KoZoomTool::createOptionWidget()
{
    return new KoZoomToolWidget(this);
    //return 0;
}

void KoZoomTool::setZoomInMode(bool zoomIn)
{
    m_zoomInMode = zoomIn;
    updateCursor(false);
}

void KoZoomTool::updateCursor(bool swap)
{
    bool setZoomInCursor = m_zoomInMode;
    if (swap) {
        setZoomInCursor = !setZoomInCursor;
    }

    if (setZoomInCursor) {
        useCursor(m_inCursor);
    } else {
        useCursor(m_outCursor);
    }
}
