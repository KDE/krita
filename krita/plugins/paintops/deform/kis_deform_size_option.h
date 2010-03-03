/*
 *  Copyright (c) 2009,2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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
#ifndef KIS_DEFORM_SIZE_OPTION_H
#define KIS_DEFORM_SIZE_OPTION_H

#include <kis_paintop_option.h>
#include <krita_export.h>

class KisDeformSizeOptionsWidget;

const QString DEFORM_SHAPE = "Soft/shape";
const QString DEFORM_DIAMETER = "Soft/diameter";
const QString DEFORM_ASPECT = "Soft/aspect";
const QString DEFORM_SCALE = "Soft/scale";
const QString DEFORM_ROTATION = "Soft/rotation";
const QString DEFORM_SPACING = "Soft/spacing";
const QString DEFORM_DENSITY = "Soft/density";
const QString DEFORM_JITTER_MOVEMENT = "Soft/jitterMovement";
const QString DEFORM_JITTER_MOVEMENT_ENABLED = "Soft/jitterMovementEnabled";

class KisDeformSizeOption : public KisPaintOpOption
{
public:
    KisDeformSizeOption();
    ~KisDeformSizeOption();

    int diameter() const;
    void setDiameter(int diameter);
    
    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);

private:
    KisDeformSizeOptionsWidget * m_options;
};

#endif
