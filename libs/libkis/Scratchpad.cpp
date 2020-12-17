/*
 *  SPDX-FileCopyrightText: 2020 Scott Petrovic <scottpetrovic@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "Scratchpad.h"
#include <KoResource.h>
#include <kis_config.h>
#include "kis_scratch_pad.h"
#include "Resource.h"
#include "View.h"
#include "Canvas.h"
#include <KoCanvasBase.h>
#include <kis_canvas2.h>

#include <QColor>
#include <QVBoxLayout>


struct Scratchpad::Private
{
    KisScratchPad *scratchpad = 0;
};


Scratchpad::Scratchpad(View *view, const QColor & defaultColor, QWidget *parent)
    : QWidget(parent), d(new Private)
{
    d->scratchpad = new KisScratchPad();
    d->scratchpad->setupScratchPad(view->view()->resourceProvider(), defaultColor);
    d->scratchpad->setMinimumSize(50, 50);

    QVBoxLayout *layout = new QVBoxLayout(this);
    layout->addWidget(d->scratchpad);
}

Scratchpad::~Scratchpad()
{
}

void Scratchpad::setModeManually(bool value)
{
    d->scratchpad->setModeManually(value);
}

void Scratchpad::setMode(QString modeType)
{
    d->scratchpad->setModeType(modeType);
}

void Scratchpad::linkCanvasZoom(bool value)
{
    d->scratchpad->linkCanvavsToZoomLevel(value);
}

void Scratchpad::loadScratchpadImage(QImage image)
{
    d->scratchpad->loadScratchpadImage(image);
}

QImage Scratchpad::copyScratchpadImageData()
{
    return d->scratchpad->copyScratchpadImageData();
}

void Scratchpad::clear()
{
    // need ability to set color
    d->scratchpad->fillDefault();
}

void Scratchpad::setFillColor(QColor color)
{
    d->scratchpad->setFillColor(color);
}
