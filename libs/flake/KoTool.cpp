/* This file is part of the KDE project
 * Copyright (C) 2006 Thomas Zander <zander@kde.org>
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

#include <QWidget>
#include <QLabel>

#include <klocale.h>

#include "KoTool.h"
#include "KoCanvasBase.h"
#include "KoViewConverter.h"
#include "KoPointerEvent.h"

KoTool::KoTool(KoCanvasBase *canvas )
    : m_canvas(canvas)
    , m_previousCursor(Qt::ArrowCursor)
    , m_optionWidget( 0 )
{
}

void KoTool::activate(bool temporary) {
    Q_UNUSED(temporary);
}

void KoTool::deactivate() {
}

bool KoTool::wantsAutoScroll() {
    return true;
}

void KoTool::mouseDoubleClickEvent( KoPointerEvent *event ) {
    event->ignore();
}

void KoTool::keyPressEvent(QKeyEvent *e) {
    e->ignore();
}

void KoTool::keyReleaseEvent(QKeyEvent *e) {
    e->ignore();
}

void KoTool::wheelEvent( KoPointerEvent * e ) {
    e->ignore();
}


void KoTool::useCursor(QCursor cursor, bool force) {
    if(!force && cursor.shape() == m_previousCursor.shape())
        return;
    m_previousCursor = cursor;
    emit sigCursorChanged(m_previousCursor);
}

QWidget * KoTool::optionWidget(QWidget * parent) {
    if ( !m_optionWidget ) {
        // XXX: Re-add the ui tool name  to show in this label. It probably
        //      was removed post moving KisTool to KoTool and I'm too fed up
        //      to re-add it right now. (boud)
        // TZ: On the other hand, just returning any widget here will loose the
        //     information whether we actually have options or not. I'd much rather
        //     return 0 if there are no options and let the GUI show a better default.
        //     Especially since I expect to have an interface soon where we can extract
        //     the options from the option-panel instance and persist them between restarts.
        //m_optionWidget = new QLabel(i18n("No options for %1.", m_visibleName), parent);
        QLabel * l = new QLabel(i18n("No options for the current tool"), parent);
        //m_optionWidget->setWindowTitle(m_visibleName);
        l->setAlignment(Qt::AlignCenter);
        m_optionWidget = l;

    }
    return m_optionWidget;
}

#include "KoTool.moc"
