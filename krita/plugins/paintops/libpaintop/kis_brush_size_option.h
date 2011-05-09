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
#ifndef KIS_SIZE_OPTION_H_
#define KIS_SIZE_OPTION_H_

#include <cmath>
#include <QtGlobal>

#include <kis_paintop_option.h>
#include <krita_export.h>

class KisBrushSizeOptionsWidget;

const QString BRUSH_SHAPE = "Brush/shape";
const QString BRUSH_DIAMETER = "Brush/diameter";
const QString BRUSH_ASPECT = "Brush/aspect";
const QString BRUSH_SCALE = "Brush/scale";
const QString BRUSH_ROTATION = "Brush/rotation";
const QString BRUSH_SPACING = "Brush/spacing";
const QString BRUSH_DENSITY = "Brush/density";
const QString BRUSH_JITTER_MOVEMENT = "Brush/jitterMovement";
const QString BRUSH_JITTER_MOVEMENT_ENABLED = "Brush/jitterMovementEnabled";

class PAINTOP_EXPORT KisBrushSizeOption : public KisPaintOpOption
{
public:
    KisBrushSizeOption();
    ~KisBrushSizeOption();

    int diameter() const;
    void setDiameter(int diameter);

    void setSpacing(qreal spacing);
    qreal spacing() const;

    qreal brushAspect() const;

    void writeOptionSetting(KisPropertiesConfiguration* setting) const;
    void readOptionSetting(const KisPropertiesConfiguration* setting);

private:
    KisBrushSizeOptionsWidget * m_options;
};

class PAINTOP_EXPORT KisBrushSizeProperties{

public:
    quint16 shape;
    quint16 diameter;
    qreal aspect;
    qreal scale;
    qreal rotation;
    qreal spacing;
    qreal density;

    qreal jitterMovementAmount;
    bool jitterEnabled;

public:
    void readOptionSetting(const KisPropertiesConfiguration * settings){
        //TODO: shape
        shape = 0;
        diameter = quint16(settings->getDouble(BRUSH_DIAMETER));
        aspect = settings->getDouble(BRUSH_ASPECT);
        rotation = settings->getDouble(BRUSH_ROTATION) * (M_PI/180.0);
        scale = settings->getDouble(BRUSH_SCALE);
        density = settings->getDouble(BRUSH_DENSITY) * 0.01;
        spacing = settings->getDouble(BRUSH_SPACING);
        if ((jitterEnabled = settings->getBool(BRUSH_JITTER_MOVEMENT_ENABLED))){
            jitterMovementAmount = settings->getDouble(BRUSH_JITTER_MOVEMENT);
        }else{
            jitterMovementAmount = 0.0;
        }
    }
};


#endif
