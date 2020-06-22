/*
 *  Copyright (c) 2020 Scott Petrovic <scottpetrovic@gmail.com>
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

    setLayout(new QVBoxLayout());
    layout()->addWidget(d->scratchpad);
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
