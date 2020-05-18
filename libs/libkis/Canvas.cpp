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
#include <kis_canvas2.h>
#include <KisView.h>
#include <KoCanvasController.h>
#include <kis_canvas_controller.h>
#include <kis_zoom_manager.h>
#include <QPointer>
#include <View.h>

struct Canvas::Private {
    Private() {}
    KisCanvas2 *canvas;
};

Canvas::Canvas(KoCanvasBase *canvas, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->canvas = static_cast<KisCanvas2*>(canvas);
}

Canvas::~Canvas()
{
    delete d;
}


bool Canvas::operator==(const Canvas &other) const
{
    return (d->canvas == other.d->canvas);
}

bool Canvas::operator!=(const Canvas &other) const
{
    return !(operator==(other));
}


qreal Canvas::zoomLevel() const
{
    if (!d->canvas) return 1.0;
    return d->canvas->imageView()->zoomManager()->zoom();
}

void Canvas::setZoomLevel(qreal value)
{
    if (!d->canvas) return;
    d->canvas->imageView()->zoomController()->setZoom(KoZoomMode::ZOOM_CONSTANT, value);
}

void Canvas::resetZoom()
{
    if (!d->canvas) return;
    d->canvas->imageView()->zoomManager()->zoomTo100();
}


void Canvas::resetRotation()
{
    if (!d->canvas) return;
    d->canvas->imageView()->canvasController()->resetCanvasRotation();
}

qreal Canvas::rotation() const
{
    if (!d->canvas) return 0;
    return d->canvas->imageView()->canvasController()->rotation();
}

void Canvas::setRotation(qreal angle)
{
    if (!d->canvas) return;
    d->canvas->imageView()->canvasController()->rotateCanvas(angle - rotation());
}


bool Canvas::mirror() const
{
    if (!d->canvas) return false;
    return d->canvas->imageView()->canvasIsMirrored();
}

void Canvas::setMirror(bool value)
{
    if (!d->canvas) return;
    d->canvas->imageView()->canvasController()->mirrorCanvas(value);
}

View *Canvas::view() const
{
    if (!d->canvas) return 0;
    View *view = new View(d->canvas->imageView());
    return view;
}

KisDisplayColorConverter *Canvas::displayColorConverter() const
{
    if (!d->canvas) return 0;
    return d->canvas->displayColorConverter();
}

bool Canvas::wrapAroundMode() const
{
    if (!d->canvas) return false;
    return d->canvas->imageView()->canvasController()->wrapAroundMode();
}

void Canvas::setWrapAroundMode(bool enable)
{
    if (!d->canvas) return;
    d->canvas->imageView()->canvasController()->slotToggleWrapAroundMode(enable);
}

bool Canvas::levelOfDetailMode() const
{
    if (!d->canvas) return false;
    return d->canvas->imageView()->canvasController()->levelOfDetailMode();
}

void Canvas::setLevelOfDetailMode(bool enable)
{
    if (!d->canvas) return;
    return d->canvas->imageView()->canvasController()->slotToggleLevelOfDetailMode(enable);
}
