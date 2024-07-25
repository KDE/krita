/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *  SPDX-FileCopyrightText: 2009, 2010 Lukáš Tvrdý (lukast.dev@gmail.com)
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_GRID_OP_OPTION_DATA_H
#define KIS_GRID_OP_OPTION_DATA_H


#include "kis_types.h"
#include <boost/operators.hpp>
#include <kritapaintop_export.h>

class KisPropertiesConfiguration;


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


struct KisGridOpOptionData : boost::equality_comparable<KisGridOpOptionData>
{	
	
    inline friend bool operator==(const KisGridOpOptionData &lhs, const KisGridOpOptionData &rhs) {
        return lhs.diameter == rhs.diameter
			&& lhs.grid_width == rhs.grid_width
			&& lhs.grid_height == rhs.grid_height
			
			&& lhs.horizontal_offset == rhs.horizontal_offset
			&& lhs.vertical_offset == rhs.vertical_offset
			&& lhs.grid_division_level == rhs.grid_division_level
			
			&& lhs.grid_pressure_division == rhs.grid_pressure_division
			&& lhs.grid_scale == rhs.grid_scale
			&& lhs.grid_vertical_border == rhs.grid_vertical_border
			
			&& lhs.grid_horizontal_border == rhs.grid_horizontal_border
			&& lhs.grid_random_border == rhs.grid_random_border
			;
    }

	// sane defaults (for Coverity)
    int diameter {25};
    int grid_width {25};
    int grid_height {25};
    
    qreal horizontal_offset {0.0};
    qreal vertical_offset {0.0};
    int grid_division_level {2};
    
    bool grid_pressure_division {false};
    qreal grid_scale {1.0};
    qreal grid_vertical_border {0.0};
    
    qreal grid_horizontal_border {0.0};
    bool grid_random_border {false};
    
	// functions
    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;
    
};

#endif // KIS_GRID_OP_OPTION_DATA_H
