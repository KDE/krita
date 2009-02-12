/*
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
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

#ifndef KIS_DYNAMICOP_H_
#define KIS_DYNAMICOP_H_

#include "kis_paintop.h"

class KisPainter;
class KisPaintInformation;
class KisDynamicBrush;
class KisDynamicOpSettings;

class KisDynamicOp : public KisPaintOp
{

public:

    KisDynamicOp(const KisDynamicOpSettings *settings, KisPainter * painter);

    virtual ~KisDynamicOp();

    void paintAt(const KisPaintInformation& info);

    double spacing(double & xSpacing, double & ySpacing, double pressure1, double pressure2) const {
        Q_UNUSED(pressure1);
        Q_UNUSED(pressure2);
        xSpacing = 1;
        ySpacing = 1;
        return 1;
    }


private:
    KisDynamicBrush* m_brush;
    const KisDynamicOpSettings *m_settings;
};

#endif // KIS_DYNAMICOP_H_
