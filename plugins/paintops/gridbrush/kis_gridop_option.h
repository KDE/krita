/*
 * Copyright (c) 2009,2010 Lukáš Tvrdý (lukast.dev@gmail.com)
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

const QString GRID_WIDTH = "Grid/gridWidth";
const QString GRID_HEIGHT = "Grid/gridHeight";
const QString GRID_DIVISION_LEVEL = "Grid/divisionLevel";
const QString GRID_PRESSURE_DIVISION = "Grid/pressureDivision";
const QString GRID_SCALE = "Grid/scale";
const QString GRID_VERTICAL_BORDER = "Grid/verticalBorder";
const QString GRID_HORIZONTAL_BORDER = "Grid/horizontalBorder";
const QString GRID_RANDOM_BORDER = "Grid/randomBorder";


class KisGridOpOptionsWidget;

class KisGridOpOption : public KisPaintOpOption
{
public:
    KisGridOpOption();
    ~KisGridOpOption() override;

    int gridWidth() const;
    void setWidth(int width) const;

    int gridHeight() const;
    void setHeight(int height) const;

    int divisionLevel() const;
    bool pressureDivision() const;
    qreal scale() const;


    qreal vertBorder() const;
    qreal horizBorder() const;
    bool randomBorder() const;

    void writeOptionSetting(KisPropertiesConfigurationSP setting) const override;
    void readOptionSetting(const KisPropertiesConfigurationSP setting) override;

private:
    KisGridOpOptionsWidget * m_options;

};

struct KisGridOpProperties : public KisPaintopPropertiesBase
{
    int grid_width;
    int grid_height;
    int grid_division_level;
    bool grid_pressure_division;
    qreal grid_scale;
    qreal grid_vertical_border;
    qreal grid_horizontal_border;
    bool grid_random_border;


    void readOptionSettingImpl(const KisPropertiesConfiguration *setting) override {
        grid_width = qMax(1, setting->getInt(GRID_WIDTH));
        grid_height = qMax(1, setting->getInt(GRID_HEIGHT));
        grid_division_level = setting->getInt(GRID_DIVISION_LEVEL);
        grid_pressure_division = setting->getBool(GRID_PRESSURE_DIVISION);
        grid_scale = setting->getDouble(GRID_SCALE);
        grid_vertical_border = setting->getDouble(GRID_VERTICAL_BORDER);
        grid_horizontal_border = setting->getDouble(GRID_HORIZONTAL_BORDER);
        grid_random_border = setting->getBool(GRID_RANDOM_BORDER);
    }

    void writeOptionSettingImpl(KisPropertiesConfiguration *setting) const override {
        setting->setProperty(GRID_WIDTH, qMax(1, grid_width));
        setting->setProperty(GRID_HEIGHT, qMax(1, grid_height));
        setting->setProperty(GRID_DIVISION_LEVEL, grid_division_level);
        setting->setProperty(GRID_PRESSURE_DIVISION, grid_pressure_division);
        setting->setProperty(GRID_SCALE, grid_scale);
        setting->setProperty(GRID_VERTICAL_BORDER, grid_vertical_border);
        setting->setProperty(GRID_HORIZONTAL_BORDER, grid_horizontal_border);
        setting->setProperty(GRID_RANDOM_BORDER, grid_random_border);
    }
};

#endif
