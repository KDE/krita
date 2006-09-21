/*
 * kis_texture_filter.cc -- Part of Krita
 *
 * Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include <kis_view.h>
#include <kis_image.h>
#include "kis_texture_painter.h"
#include "kis_texture_filter.h"

void WetPaintDevAction::act(KisPaintDeviceSP device, qint32 w, qint32 h) const {
    KoColorSpace * cs = device->colorSpace();

    if (cs->id() != KoID("WET","")) {
        return;
    }

    // XXX if params of the painter get configurable, make them here configurable as well?
    KisTexturePainter painter(device);
    painter.createTexture(0, 0, w, h);
    painter.end();
}

