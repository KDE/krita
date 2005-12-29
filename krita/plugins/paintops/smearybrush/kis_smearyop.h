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

#ifndef KIS_SMEARYOP_H_
#define KIS_SMEARYOP_H_

#include "kis_paintop.h"

class KisPoint;
class KisPainter;


class KisSmearyOpFactory : public KisPaintOpFactory  {

public:
    KisSmearyOpFactory() {}
    virtual ~KisSmearyOpFactory() {}

    virtual KisPaintOp * createOp(KisPainter * painter);
    virtual KisID id() { return KisID("paintSmeary", i18n("Smeary Brush")); }
    virtual QString pixmap() { return ""; }

};

class KisSmearyOp : public KisPaintOp {

    typedef KisPaintOp super;

public:

    KisSmearyOp(KisPainter * painter);
    virtual ~KisSmearyOp();

    void paintAt(const KisPoint &pos, const KisPaintInformation& info);

};

#endif // KIS_SMEARYOP_H_
