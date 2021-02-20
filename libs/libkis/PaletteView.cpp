/*
 *  SPDX-FileCopyrightText: 2017 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
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
    d->widget = new KisPaletteView();
    d->model = new KisPaletteModel();
    d->widget->setPaletteModel(d->model);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(d->widget);

    //forward signals.
    connect(d->widget, SIGNAL(entrySelected(KisSwatch)),
                 this, SLOT(fgSelected(KisSwatch)));
    connect(d->widget, SIGNAL(entrySelectedBackGround(KisSwatch)),
            this, SLOT(bgSelected(KisSwatch)));
}

PaletteView::~PaletteView()
{
    delete d->model;
}

void PaletteView::setPalette(Palette *palette)
{
    d->model->setPalette(palette->colorSet());
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
        return d->widget->addGroupWithDialog();
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

void PaletteView::fgSelected(KisSwatch swatch)
{
    emit entrySelectedForeGround(Swatch(swatch));
}

void PaletteView::bgSelected(KisSwatch swatch)
{
    emit entrySelectedBackGround(Swatch(swatch));
}
