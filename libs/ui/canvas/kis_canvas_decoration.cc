/*
 * Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#include "kis_canvas_decoration.h"
#include "kis_canvas2.h"
#include "kis_debug.h"

struct KisCanvasDecoration::Private {
    bool visible;
    QPointer<KisView> view;
    QString id;
    int priority = 0;
};

KisCanvasDecoration::KisCanvasDecoration(const QString& id, QPointer<KisView>parent)
    : QObject(parent)
    , d(new Private)
{
    d->visible = false;
    d->view = parent;
    d->id = id;
}

KisCanvasDecoration::~KisCanvasDecoration()
{
    delete d;
}

void KisCanvasDecoration::setView(QPointer<KisView>imageView)
{
    d->view = imageView;
}


const QString& KisCanvasDecoration::id() const
{
    return d->id;
}

void KisCanvasDecoration::setVisible(bool v)
{
    d->visible = v;
    if (d->view &&
            d->view->canvasBase()) {

        d->view->canvasBase()->updateCanvas();
    }
}

bool KisCanvasDecoration::visible() const
{
    return d->visible;
}

void KisCanvasDecoration::toggleVisibility()
{
    setVisible(!visible());
}

void KisCanvasDecoration::paint(QPainter& gc, const QRectF& updateArea, const KisCoordinatesConverter *converter, KisCanvas2 *canvas = 0)
{
    if (!canvas) {
        dbgFile<<"canvas does not exist:"<<canvas;
    }
    if (visible()) {
        drawDecoration(gc, updateArea, converter,canvas);
    }
}

int KisCanvasDecoration::priority() const
{
    return d->priority;
}

void KisCanvasDecoration::setPriority(int value)
{
    d->priority = value;
}

bool KisCanvasDecoration::comparePriority(KisCanvasDecorationSP decoration1, KisCanvasDecorationSP decoration2)
{
    return decoration1->priority() < decoration2->priority();
}


QPointer<KisView>KisCanvasDecoration::imageView()
{
    return d->view;
}


QPointer<KisView>KisCanvasDecoration::view() const
{
    return d->view;
}
