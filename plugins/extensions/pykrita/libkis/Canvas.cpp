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
#include "Canvas.h"
#include <KoCanvasBase.h>
#include <QPointer>
#include <ColorManager.h>

struct Canvas::Private {
    Private() {}
    KoCanvasBase *canvas;
};

Canvas::Canvas(KoCanvasBase *canvas, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->canvas = canvas;
}

Canvas::~Canvas()
{
    delete d;
}

Document* Canvas::document() const
{
    return 0;
}

int Canvas::zoomLevel() const
{
    return 0;
}

void Canvas::setZoomLevel(int value)
{
}


int Canvas::rotation() const
{
    return 0;
}

void Canvas::setRotation(int value)
{
}


bool Canvas::mirror() const
{
    return false;
}

void Canvas::setMirror(bool value)
{
}


ColorManager* Canvas::colorManager() const
{
    return 0;
}

void Canvas::setColorManager(ColorManager* value)
{
}






