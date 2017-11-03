/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
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
#include "DockWidget.h"
#include <QDebug>

#include <KoCanvasBase.h>

#include "Canvas.h"

struct DockWidget::Private {
    Private() {}

    Canvas *canvas {0};
};

DockWidget::DockWidget()
    : QDockWidget()
    , d(new Private)
{
}

DockWidget::~DockWidget()
{
    delete d;
}

Canvas* DockWidget::canvas() const
{
    return d->canvas;
}

void DockWidget::setCanvas(KoCanvasBase* canvas)
{
    delete d->canvas;
    d->canvas = new Canvas(canvas);
    canvasChanged(d->canvas);
}

void DockWidget::unsetCanvas()
{
    canvasChanged(0);
    delete d->canvas;
    d->canvas = 0;
}
