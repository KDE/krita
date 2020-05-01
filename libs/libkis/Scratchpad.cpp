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

Scratchpad::Scratchpad(View *view, const QString & defaultColor, QWidget *parent)
    : KisScratchPad(parent)
{    

    // todo hook this up later to override
    const QColor backgroundColor = QColor(Qt::white);

    KisScratchPad::setupScratchPad(view->view()->resourceProvider(), backgroundColor);
    KisScratchPad::setMinimumSize(50, 50);
}

Scratchpad::~Scratchpad()
{
}

void Scratchpad::setModeManually(bool value)
{
    KisScratchPad::setModeManually(value);
}

void Scratchpad::setMode(QString modeType)
{
    KisScratchPad::setModeType(modeType);
}

void Scratchpad::clear()
{
    KisScratchPad::fillDefault();
}
