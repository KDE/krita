/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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

#ifndef KIS_CONVOLVEOP_H_
#define KIS_CONVOLVEOP_H_

#include "kis_paintop.h"

class KisPoint;
class KisPainter;


class KisConvolveOpFactory : public KisPaintOpFactory  {

public:
    KisConvolveOpFactory() {}
    virtual ~KisConvolveOpFactory() {}

    virtual KisPaintOp * createOp(const KisPaintOpSettings *settings, KisPainter * painter);
    virtual KisID id() { return KisID("convolve", i18n("Convolve")); }
};


class KisConvolveOp : public KisPaintOp {

    typedef KisPaintOp super;

public:

    KisConvolveOp(KisPainter * painter);
    virtual ~KisConvolveOp();

    void paintAt(const KisPoint &pos, const KisPaintInformation& info);

};

#endif // KIS_CONVOLVEOP_H_
