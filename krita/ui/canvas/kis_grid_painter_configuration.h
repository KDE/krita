/*
 * This file is part of Krita
 *
 *  Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_GRID_PAINTER_CONFIGURATION_H
#define KIS_GRID_PAINTER_CONFIGURATION_H

class QPen;

class KisGridPainterConfiguration
{
public:
    /**
     * @return the pen use for drawing the main line of the grid
     */
    static QPen mainPen();
    /**
     * @return the pen use for drawing the subdivision line of the grid
     */
    static QPen subdivisionPen();
};

#endif
