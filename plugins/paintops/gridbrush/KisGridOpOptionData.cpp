/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisGridOpOptionData.h"

#include "kis_properties_configuration.h"


bool KisGridOpOptionData::read(const KisPropertiesConfiguration *setting)
{
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
    return true;
}

void KisGridOpOptionData::write(KisPropertiesConfiguration *setting) const
{
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
}
