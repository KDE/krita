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
#include <kis_selection_filters.h>

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

void Selection::erode()
{
    if (!d->selection) return;
    KisErodeSelectionFilter esf;
    QRect rc = esf.changeRect(d->selection->selectedExactRect());
    esf.process(d->selection->pixelSelection(), rc);
}

void Selection::dilate()
{
    if (!d->selection) return;
    KisDilateSelectionFilter dsf;
    QRect rc = dsf.changeRect(d->selection->selectedExactRect());
    dsf.process(d->selection->pixelSelection(), rc);
}

void Selection::border(int xRadius, int yRadius)
{
    if (!d->selection) return;
    KisBorderSelectionFilter sf(xRadius, yRadius);
    QRect rc = sf.changeRect(d->selection->selectedExactRect());
    sf.process(d->selection->pixelSelection(), rc);
}

void Selection::feather(int radius)
{
    if (!d->selection) return;
    KisFeatherSelectionFilter fsf(radius);
    QRect rc = fsf.changeRect(d->selection->selectedExactRect());
    fsf.process(d->selection->pixelSelection(), rc);
}

void Selection::grow(int xradius, int yradius)
{
    if (!d->selection) return;
    KisGrowSelectionFilter gsf(xradius, yradius);
    QRect rc = gsf.changeRect(d->selection->selectedExactRect());
    gsf.process(d->selection->pixelSelection(), rc);
}


void Selection::shrink(int xRadius, int yRadius, bool edgeLock)
{
    if (!d->selection) return;
    KisShrinkSelectionFilter sf(xRadius, yRadius, edgeLock);
    QRect rc = sf.changeRect(d->selection->selectedExactRect());
    sf.process(d->selection->pixelSelection(), rc);
}

void Selection::smooth()
{
    if (!d->selection) return;
    KisSmoothSelectionFilter sf;
    QRect rc = sf.changeRect(d->selection->selectedExactRect());
    sf.process(d->selection->pixelSelection(), rc);
}


void Selection::invert()
{
    if (!d->selection) return;
    KisInvertSelectionFilter sf;
    QRect rc = sf.changeRect(d->selection->selectedExactRect());
    sf.process(d->selection->pixelSelection(), rc);
}

void Selection::resize(int w, int h)
{
    if (!d->selection) return;
    d->selection->pixelSelection()->select(QRect(x(), y(), w, h));
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
    if (!d->selection) return;
    d->selection->pixelSelection()->applySelection(selection->selection()->pixelSelection(), SELECTION_REPLACE);
}

void Selection::add(Selection *selection)
{
    if (!d->selection) return;
    d->selection->pixelSelection()->applySelection(selection->selection()->pixelSelection(), SELECTION_ADD);
}

void Selection::subtract(Selection *selection)
{
    if (!d->selection) return;
    d->selection->pixelSelection()->applySelection(selection->selection()->pixelSelection(), SELECTION_SUBTRACT);
}

void Selection::intersect(Selection *selection)
{
    if (!d->selection) return;
    d->selection->pixelSelection()->applySelection(selection->selection()->pixelSelection(), SELECTION_INTERSECT);
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


