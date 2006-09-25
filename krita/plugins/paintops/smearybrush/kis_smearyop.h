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
//Added by qt3to4:
#include <Q3PtrList>

class KisPoint;
class KisPainter;


class KisSmearyOpFactory : public KisPaintOpFactory  {

public:
    KisSmearyOpFactory() {}
    virtual ~KisSmearyOpFactory() {}

    virtual KisPaintOp * createOp(const KisPaintOpSettings *settings, KisPainter * painter);
    virtual KoID id() { return KoID("paintSmeary", i18n("Smeary Brush")); }
    virtual bool userVisible(KoColorSpace * ) { return false; }
    virtual QString pixmap() { return ""; }

};

/**
 * The smeary brush implements bidirectional paint transfer. It takes the
 * color at the footprint of the brush (unless it's the image color or
 * transparent and mixes it with the paint color, creating a new paint
 * color.
 *
 * A brush contains a number of tufts. Depending on pressure, the tufts
 * will be more or less concentrated around the paint position. Tufts
 * mix with the color under each tuft and load the tuft with the mixture
 * for the next paint operation. The mixture is also dependent upon pressure.
 *
 * The paint load will run out after a certain number of paintAt's, depending
 * on pressure.
 */
class KisSmearyOp : public KisPaintOp {

    typedef KisPaintOp super;

public:

    KisSmearyOp(KisPainter * painter);
    virtual ~KisSmearyOp();

    void paintAt(const KisPoint &pos, const KisPaintInformation& info);


private:
    class SmearyTuft;
    
    Q3PtrList<SmearyTuft> m_rightTufts;
    Q3PtrList<SmearyTuft> m_leftTufts;

};

#endif // KIS_SMEARYOP_H_
