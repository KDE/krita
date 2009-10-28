/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#ifndef KIS_SUMIPAINTOP_H_
#define KIS_SUMIPAINTOP_H_

#include <klocale.h>
#include <kis_paintop.h>
#include <kis_paintop_factory.h>
#include <kis_types.h>

//#define BENCHMARK

#include "brush.h"

#include "kis_sumi_paintop_settings.h"

class QPointF;
class KisPainter;

class KisSumiPaintOp : public KisPaintOp
{

public:
    KisSumiPaintOp(const KisSumiPaintOpSettings *settings, KisPainter * painter, KisImageWSP image);
    virtual ~KisSumiPaintOp();

    void paintAt(const KisPaintInformation& info);
    double paintLine(const KisPaintInformation &pi1, const KisPaintInformation &pi2, double savedDist);

    double spacing(double & xSpacing, double & ySpacing, double pressure1, double pressure2) const {
        Q_UNUSED(xSpacing);
        Q_UNUSED(ySpacing);
        Q_UNUSED(pressure1);
        Q_UNUSED(pressure2);
        // XXX: this is wrong, but that doesn't matter, since paintLine doesn't use spacing.
        return 0.5;
    }


private:

    const KisSumiPaintOpSettings* m_settings;
    QPointF m_previousPoint;
    KisImageWSP m_image;
    bool newStrokeFlag;

    KisPaintDeviceSP m_dab;
    KisPaintDeviceSP m_dev;

    Brush m_brush;
    
#ifdef BENCHMARK
    int m_total;
    int m_count;
#endif
   
};

#endif // KIS_SUMIPAINTOP_H_
