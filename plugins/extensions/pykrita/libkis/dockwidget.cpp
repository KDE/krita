/*
 *  Copyright (c) 2015 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "dockwidget.h"
#include <canvas.h>
#include <QDebug>

DockWidget::DockWidget()
    : QDockWidget()
    , m_canvas(0)
{
}

DockWidget::~DockWidget()
{
    qDebug() << "Deleting DockWidget";
    delete m_canvas;
}

void DockWidget::setCanvas(KoCanvasBase* canvas)
{
    qDebug() << "New canvas" << canvas;
    m_canvas = new Canvas(canvas);
    canvasChanged(m_canvas);
}

void DockWidget::unsetCanvas()
{
    delete m_canvas;
    m_canvas = 0;
}
