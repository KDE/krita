/*
 *  Copyright (c) 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
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

#include <PaletteView.h>
#include <QVBoxLayout>

struct PaletteView::Private
{
    KisPaletteModel *model = 0;
    KisPaletteView *widget = 0;
    bool allowPaletteModification = true;
};

PaletteView::PaletteView(QWidget *parent)
    : QWidget(parent), d(new Private)
{
    d->widget = new KisPaletteView(this);
    d->model = new KisPaletteModel();
    d->widget->setPaletteModel(d->model);
    this->setLayout(new QVBoxLayout());
    this->layout()->addWidget(d->widget);

    //forward signals.
    connect(d->widget, SIGNAL(entrySelected(KisSwatch)),
                 this, SIGNAL(entrySelectedForeGround(KisSwatch)));
    connect(d->widget, SIGNAL(entrySelectedBackGround(KisSwatch)),
            this, SIGNAL(entrySelectedBackGround(KisSwatch)));
}

PaletteView::~PaletteView()
{
    delete d->model;
}

void PaletteView::setPalette(Palette *palette)
{
    d->model->setColorSet(palette->colorSet());
    d->widget->setPaletteModel(d->model);
}

bool PaletteView::addEntryWithDialog(ManagedColor *color)
{
    if (d->model->colorSet()) {
        return d->widget->addEntryWithDialog(color->color());
    }
    return false;
}

bool PaletteView::addGroupWithDialog()
{
    if (d->model->colorSet()) {
        d->widget->addGroupWithDialog();
        return false;
    }
    return false;
}

bool PaletteView::removeSelectedEntryWithDialog()
{
    if (d->model->colorSet()) {
        return d->widget->removeEntryWithDialog(d->widget->currentIndex());
    }
    return false;
}

void PaletteView::trySelectClosestColor(ManagedColor *color)
{
    d->widget->selectClosestColor(color->color());
}
