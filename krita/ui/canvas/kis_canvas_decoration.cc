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

#include "kis_view2.h"

struct KisCanvasDecoration::Private {
    bool visible;
    KisView2* view;
    QString id;
    QString name;
};

KisCanvasDecoration::KisCanvasDecoration(const QString& id, const QString& name, KisView2 * parent) : QObject(parent), d(new Private)
{
    d->visible = false;
    d->view = parent;
    d->id = id;
    d->name = name;
}

KisCanvasDecoration::~KisCanvasDecoration()
{
    delete d;
}

const QString& KisCanvasDecoration::id() const
{
    return d->id;
}

const QString& KisCanvasDecoration::name() const
{
    return d->name;
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
    d->visible = !d->visible;
}

void KisCanvasDecoration::paint(QPainter& gc, const QPoint & documentOffset, const QRect& area, const KoViewConverter &converter)
{
    if (visible()) {
        //dbgKrita << "Decoration " << name() << " is visible";
        drawDecoration(gc, documentOffset, area, converter);
    }
}

KisView2* KisCanvasDecoration::view() const
{
    return d->view;
}
#include "kis_canvas_decoration.moc"
