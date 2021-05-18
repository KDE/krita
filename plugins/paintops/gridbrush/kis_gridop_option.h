/*
 * SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_GRIDOP_OPTION_H
#define KIS_GRIDOP_OPTION_H

#include <kis_paintop_option.h>

const QString DIAMETER = "Grid/diameter";
const QString GRID_WIDTH = "Grid/gridWidth";
const QString GRID_HEIGHT = "Grid/gridHeight";
const QString HORIZONTAL_OFFSET = "Grid/horizontalOffset";
const QString VERTICAL_OFFSET = "Grid/verticalOffset";
const QString GRID_DIVISION_LEVEL = "Grid/divisionLevel";
const QString GRID_PRESSURE_DIVISION = "Grid/pressureDivision";
const QString GRID_SCALE = "Grid/scale";
const QString GRID_VERTICAL_BORDER = "Grid/verticalBorder";
const QString GRID_HORIZONTAL_BORDER = "Grid/horizontalBorder";
const QString GRID_RANDOM_BORDER = "Grid/randomBorder";
const QString GRID_SHAPE = "GridShape/shape";


class KisGridOpOptionsWidget;

class KisGridOpOption : public KisPaintOpOption
{
public:
    KisGridOpOption();
    ~KisGridOpOption() override;

    int diameter() const;
    void setDiameter(int diameter) const;

    int gridWidth() const;
    void setWidth(int width) const;

    int gridHeight() const;
    void setHeight(int height) const;

    qreal horizontalOffset() const;
    void setHorizontalOffset(qreal horizontalOffset) const;

    qreal verticalOffset() const;
    void setVerticalOffset(qreal verticalOffset) const;


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
    int diameter;
    int grid_width;
    int grid_height;
    qreal horizontal_offset;
    qreal vertical_offset;
    int grid_division_level;
    bool grid_pressure_division;
    qreal grid_scale;
    qreal grid_vertical_border;
    qreal grid_horizontal_border;
    bool grid_random_border;
    int grid_shape;

    void readOptionSettingImpl(const KisPropertiesConfiguration *setting) override {

        grid_width = qMax(1, setting->getInt(GRID_WIDTH));
        grid_height = qMax(1, setting->getInt(GRID_HEIGHT));
        diameter = setting->getInt(DIAMETER);
        // If loading an old brush without a diameter set, set to grid_width as was the old logic
        if (!diameter) {
            diameter = grid_width;
        }
        else {
            diameter = qMax(1, diameter);
        }
        horizontal_offset = setting->getDouble(HORIZONTAL_OFFSET);
        vertical_offset = setting->getDouble(VERTICAL_OFFSET);
        grid_division_level = setting->getInt(GRID_DIVISION_LEVEL);
        grid_pressure_division = setting->getBool(GRID_PRESSURE_DIVISION);
        grid_scale = setting->getDouble(GRID_SCALE);
        grid_vertical_border = setting->getDouble(GRID_VERTICAL_BORDER);
        grid_horizontal_border = setting->getDouble(GRID_HORIZONTAL_BORDER);
        grid_random_border = setting->getBool(GRID_RANDOM_BORDER);
        grid_shape = setting->getInt(GRID_SHAPE);
    }

    void writeOptionSettingImpl(KisPropertiesConfiguration *setting) const override {
        setting->setProperty(DIAMETER, qMax(1,diameter));
        setting->setProperty(GRID_WIDTH, qMax(1, grid_width));
        setting->setProperty(GRID_HEIGHT, qMax(1, grid_height));
        setting->setProperty(HORIZONTAL_OFFSET, horizontal_offset);
        setting->setProperty(VERTICAL_OFFSET, vertical_offset);
        setting->setProperty(GRID_DIVISION_LEVEL, grid_division_level);
        setting->setProperty(GRID_PRESSURE_DIVISION, grid_pressure_division);
        setting->setProperty(GRID_SCALE, grid_scale);
        setting->setProperty(GRID_VERTICAL_BORDER, grid_vertical_border);
        setting->setProperty(GRID_HORIZONTAL_BORDER, grid_horizontal_border);
        setting->setProperty(GRID_RANDOM_BORDER, grid_random_border);
        setting->setProperty(GRID_SHAPE, grid_shape);
    }
};

#endif
