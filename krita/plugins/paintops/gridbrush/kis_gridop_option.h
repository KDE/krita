/*
 * Copyright (c) 2009 Lukáš Tvrdý (lukast.dev@gmail.com)
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

#ifndef KIS_GRIDOP_OPTION_H
#define KIS_GRIDOP_OPTION_H

#include <kis_paintop_option.h>
#include <krita_export.h>

class KisGridOpOptionsWidget;

class KisGridOpOption : public KisPaintOpOption
{
public:
    KisGridOpOption();
    ~KisGridOpOption();

    int gridWidth() const;
    void setWidth(int width) const;
   
    int gridHeight() const;
    void setHeight(int height) const;
    
    int divisionLevel() const;
    bool pressureDivision() const;
    qreal scale() const;
    
    
    qreal vertBorder() const;
    qreal horizBorder() const;
    bool jitterBorder() const;
    
    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);

private:
   KisGridOpOptionsWidget * m_options;

};

#endif
