/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#ifndef KIS_WSBRUSHOP_H_
#define KIS_WSBRUSHOP_H_

#include "kis_paintop.h"
#include "kis_types.h"

class KisPoint;
class KisPainter;


class KisWSBrushOpFactory : public KisPaintOpFactory  {

public:
    KisWSBrushOpFactory() {}
    virtual ~KisWSBrushOpFactory() {}

    virtual KisPaintOp * createOp(const KisPaintOpSettings *settings, KisPainter * painter);
    virtual KisID id() { return KisID("wsbrush", i18n("Wet & Sticky Paintbrush")); }
    virtual QString pixmap() { return "wetpaintbrush.png"; }
};

class KisWSBrushOp : public KisPaintOp {

    typedef KisPaintOp super;

public:

    KisWSBrushOp(KisPainter * painter);
    virtual ~KisWSBrushOp();

    void paintAt(const KisPoint &pos,
             const double pressure,
             const double /*xTilt*/,
             const double /*yTilt*/);

};

#endif // KIS_WSBRUSHOP_H_
