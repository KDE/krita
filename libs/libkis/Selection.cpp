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
#include "Selection.h"

#include <kis_selection.h>
#include <kis_pixel_selection.h>
#include <kis_paint_device.h>

#include <QByteArray>

#include <Node.h>

struct Selection::Private {
    Private() {}
    KisSelectionSP selection;
};

Selection::Selection(KisSelectionSP selection, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->selection = selection;
}


Selection::Selection(QObject *parent) 
    : QObject(parent)
    , d(new Private)
{
    d->selection = new KisSelection();
}

Selection::~Selection() 
{
    delete d;
}

int Selection::width() const
{
    if (!d->selection) return 0;
    return d->selection->selectedExactRect().width();
}

int Selection::height() const
{
    if (!d->selection) return 0;
    return d->selection->selectedExactRect().height();
}

int Selection::x() const
{
    if (!d->selection) return 0;
    return d->selection->x();
}

int Selection::y() const
{
    if (!d->selection) return 0;
    return 0;
}

void Selection::move(int x, int y)
{
    if (!d->selection) return;
}


void Selection::clear()
{
    if (!d->selection) return;
    d->selection->clear();
}

void Selection::contract(int value)
{
}

void Selection::cut(Node* node)
{
}

void Selection::paste(Node *source, Node*destination)
{
}

void Selection::deselect()
{
    if (!d->selection) return;
}

void Selection::expand(int value)
{
    if (!d->selection) return;
}

void Selection::feather(int value)
{
    if (!d->selection) return;
}

void Selection::fill(Node* node)
{
    if (!d->selection) return;
}

void Selection::grow(int value)
{
    if (!d->selection) return;
}

void Selection::invert()
{
    if (!d->selection) return;
}

void Selection::resize(int w, int h)
{
    if (!d->selection) return;
}

void Selection::rotate(int degrees)
{
    if (!d->selection) return;
}

void Selection::select(int x, int y, int w, int h, int value)
{
    if (!d->selection) return;
    d->selection->pixelSelection()->select(QRect(x, y, w, h), value);
}

void Selection::selectAll(Node *node, int value)
{
    if (!d->selection) return;
    d->selection->pixelSelection()->select(node->node()->exactBounds(), value);
}

void Selection::replace(Selection *selection)
{
    d->selection->pixelSelection()->applySelection(selection->selection(), SELECTION_REPLACE);
}

void Selection::add(Selection *selection)
{
    d->selection->pixelSelection()->applySelection(selection->selection(), SELECTION_ADD);
}

void Selection::subtract(Selection *selection)
{
    d->selection->pixelSelection()->applySelection(selection->selection(), SELECTION_SUBTRACT);
}

void Selection::intersect(Selection *selection)
{
    d->selection->pixelSelection()->applySelection(selection->selection(), SELECTION_INTERSECT);
}


QByteArray Selection::pixelData(int x, int y, int w, int h) const
{
    QByteArray ba;
    if (!d->selection) return ba;
    KisPaintDeviceSP dev = d->selection->projection();
    quint8 *data = new quint8[w * h];
    dev->readBytes(data, x, y, w, h);
    ba = QByteArray((const char*)data, (int)(w * h));
    delete[] data;
    return ba;
}

void Selection::setPixelData(QByteArray value, int x, int y, int w, int h)
{
    if (!d->selection) return;
    KisPixelSelectionSP dev = d->selection->pixelSelection();
    if (!dev) return;
    dev->writeBytes((const quint8*)value.constData(), x, y, w, h);
}

KisSelectionSP Selection::selection() const
{
    return d->selection;
}


