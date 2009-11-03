/*
 *  Copyright (c) 2008-2009 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#ifndef KIS_SPRAYOP_OPTION_H
#define KIS_SPRAYOP_OPTION_H

#include <kis_paintop_option.h>
#include <krita_export.h>

class KisSprayOpOptionsWidget;

class KisSprayOpOption : public KisPaintOpOption
{
public:
    KisSprayOpOption();
    ~KisSprayOpOption();

    int diameter() const;
    void setDiamter(int diameter) const;
    
    int particleCount() const;

    qreal coverage() const;
    qreal amount() const;
    qreal spacing() const;
    qreal scale() const;

    bool jitterMovement() const;
    bool jitterSize() const;
    bool useDensity() const;

    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);

private:
    KisSprayOpOptionsWidget * m_options;

};

#endif
