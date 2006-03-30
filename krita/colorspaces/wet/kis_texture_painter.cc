/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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
#include <math.h>

#include <kdebug.h>

#include <kis_global.h>
#include <kis_image.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>

#include "kis_wet_colorspace.h"
#include "kis_texture_painter.h"

KisTexturePainter::KisTexturePainter()
    : super()
{
    // XXX make at least one of these configurable, probably blurh
    m_height = 1;
    m_blurh = 0.7;
}

KisTexturePainter::KisTexturePainter(KisPaintDeviceSP device) : super(device)
{
    m_height = 1;
    m_blurh = 0.7;
}

void KisTexturePainter::createTexture( Q_INT32 x,  Q_INT32 y,  Q_INT32 w,  Q_INT32 h)
{
    double hscale = 128 * m_height / RAND_MAX;

    int ibh = (int) floor(256 * m_blurh + 0.5);

    // initialize with random data
    for (int y2 = 0; y2 < h; y2++) {
        KisHLineIterator i = m_device->createHLineIterator(x, y + y2, w, true);
        while (!i.isDone()) {
            WetPack* pack = reinterpret_cast<WetPack*>(i.rawData());
            WetPix* w = &(pack->adsorb);
            w->h = ( Q_UINT16)floor(128 + hscale * rand());
            ++i;
        }
    }

    int lh;

    // Blur horizontally
    for (int y2 = 0; y2 < h; y2++) {
        KisHLineIterator i = m_device->createHLineIterator(x, y + y2, w, true);

        WetPack* pack = reinterpret_cast<WetPack*>(i.rawData());
        WetPix* w = &(pack->adsorb);
        lh = w->h;
        ++i;

        while (!i.isDone()) {
            pack = reinterpret_cast<WetPack*>(i.rawData());
            w = &(pack->adsorb);
            w->h += ((lh - w->h) * ibh + 128) >> 8;
            lh = w->h;
            // XXX to make it easier for us later on, we store the height data in paint
            // as well!
            w = &(pack->paint);
            w->h = lh;
            ++i;
        }
    }

    // Vertical blurring was commented out in wetdreams, the effect seems to be achievable
    // without this.
    // I think this is because with blur in one direction, you get more the effect of
    // having 'fibers' in your paper
}
