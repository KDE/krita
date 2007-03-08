/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2007 Casper Boemann <cbr@boemann.dk>
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

#include "KoZoomStrategy.h"
#include "KoZoomTool.h"
#include "KoCanvasBase.h"
#include "KoCanvasController.h"

#include <kdebug.h>

KoZoomStrategy::KoZoomStrategy( KoZoomTool *tool, KoCanvasController *controller, const QPointF &clicked)
: KoShapeRubberSelectStrategy(tool, controller->canvas(), clicked, false),
m_controller(controller)
{
}

void KoZoomStrategy::finishInteraction( Qt::KeyboardModifiers modifiers ) {

    if(selectRect().width() > 5 && selectRect().height() > 5)
        m_controller->zoomTo(selectRect());
    else if( modifiers & Qt::ControlModifier)
        m_controller->zoomOut(selectRect().center());
    else
        m_controller->zoomIn(selectRect().center());
}
