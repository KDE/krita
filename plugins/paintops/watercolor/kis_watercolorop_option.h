/* This file is part of the KDE project
 *
 * Copyright (C) 2017 Grigory Tantsevov <tantsevov@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef KIS_WATERCOLOROP_OPTION_H
#define KIS_WATERCOLOROP_OPTION_H

#include <kis_paintop_option.h>

const QString WATERCOLOR_TYPE = "Watercolor/type";
const QString WATERCOLOR_GRAVITY_X = "Watercolor/gravityX";
const QString WATERCOLOR_GRAVITY_Y = "Watercolor/gravityY";

class KisWatercolorOpOptionsWidget;

class KisWatercolorOpOption : public KisPaintOpOption
{
public:
    KisWatercolorOpOption();
    ~KisWatercolorOpOption();

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const;
    void readOptionSetting(const KisPropertiesConfigurationSP setting);

private:
    KisWatercolorOpOptionsWidget *m_options;
};

class WatercolorOption
{
public:
    int type;
    qreal gravityX, gravityY;

    void readOptionSetting(const KisPropertiesConfigurationSP setting) {
        type = setting->getInt(WATERCOLOR_TYPE);
        gravityX = setting->getDouble(WATERCOLOR_GRAVITY_X);
        gravityY = setting->getDouble(WATERCOLOR_GRAVITY_Y);
    }

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const {
        setting->setProperty(WATERCOLOR_TYPE, type);
        setting->setProperty(WATERCOLOR_GRAVITY_X, gravityX);
        setting->setProperty(WATERCOLOR_GRAVITY_Y, gravityY);
    }
};

#endif
