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
#ifndef KIS_TEXTURE_PAINTER_H_
#define KIS_TEXTURE_PAINTER_H_

#include "kis_types.h"
#include "kis_painter.h"

class KisTexturePainter : public KisPainter
{

    typedef KisPainter super;

public:

    KisTexturePainter();
    KisTexturePainter(KisPaintDeviceSP device);

    void createTexture(Q_INT32 x, Q_INT32 y, Q_INT32 w, Q_INT32 h);

private:
    double m_blurh, m_height;

};
#endif //KIS_TEXTURE_PAINTER_H_
