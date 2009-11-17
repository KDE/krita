/*
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
#ifndef KIS_CHALKOP_OPTION_H
#define KIS_CHALKOP_OPTION_H

#include <kis_paintop_option.h>
#include <krita_export.h>

class KisChalkOpOptionsWidget;

class KisChalkOpOption : public KisPaintOpOption
{
public:
    KisChalkOpOption();
    ~KisChalkOpOption();

    void setRadius(int radius) const;
    int radius() const;

    bool inkDepletion() const; 
    bool saturation() const;
    bool opacity() const;
    
    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);


private:

    KisChalkOpOptionsWidget * m_options;

};

#endif
