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
#include "kis_debug.h"
#include "kis_view2.h"

struct KisCanvasDecoration::Private {
    bool visible;
    KisView2* view;
    QString id;
};

KisCanvasDecoration::KisCanvasDecoration(const QString& id, KisView2 * parent, bool visible) : QObject(parent), d(new Private)
{
    d->visible = visible;
    d->view = parent;
    d->id = id;
}

KisCanvasDecoration::~KisCanvasDecoration()
{
    delete d;
}

const QString& KisCanvasDecoration::id() const
{
    return d->id;
}

void KisCanvasDecoration::setVisible(bool v)
{
    d->visible = v;
    d->view->canvas()->update();
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
    if (visible())
        drawDecoration(gc, updateArea, converter,canvas);
}

KisView2* KisCanvasDecoration::view() const
{
    return d->view;
}
#include "kis_canvas_decoration.moc"
