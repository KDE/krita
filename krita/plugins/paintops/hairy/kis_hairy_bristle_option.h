/*
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_HAIRY_BRISTLE_OPTION_H
#define KIS_HAIRY_BRISTLE_OPTION_H

#include <kis_paintop_option.h>
#include <krita_export.h>

const QString HAIRY_BRISTLE_USE_MOUSEPRESSURE = "HairyBristle/useMousePressure";
const QString HAIRY_BRISTLE_SCALE = "HairyBristle/scale";
const QString HAIRY_BRISTLE_SHEAR = "HairyBristle/shear";
const QString HAIRY_BRISTLE_RANDOM = "HairyBristle/random";
const QString HAIRY_BRISTLE_DENSITY = "HairyBristle/density";
const QString HAIRY_BRISTLE_THRESHOLD = "HairyBristle/threshold";
const QString HAIRY_BRISTLE_ANTI_ALIASING = "HairyBristle/antialias";
const QString HAIRY_BRISTLE_USE_COMPOSITING = "HairyBristle/useCompositing";
const QString HAIRY_BRISTLE_CONNECTED = "HairyBristle/isConnected";

class KisBristleOptionsWidget;

class KisHairyBristleOption : public KisPaintOpOption
{
public:
    KisHairyBristleOption();
    ~KisHairyBristleOption();

    void setScaleFactor(qreal scale) const;
    
    bool useMousePressure() const;

    double scaleFactor() const;
    double shearFactor() const;
    double randomFactor() const;

    void writeOptionSetting(KisPropertiesConfiguration* config) const;
    void readOptionSetting(const KisPropertiesConfiguration* config);
private:
    KisBristleOptionsWidget * m_options;
};

#endif // KIS_HAIRY_BRISTLE_OPTION_H

