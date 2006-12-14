/* This file is part of the KDE project
   Copyright (C) 2006 Laurent Montel <montel@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef KOGRIDDATA_H
#define KOGRIDDATA_H

#include <koffice_export.h>

class KOFFICECORE_EXPORT KoGridData
{
public:
    KoGridData();
  
    /// return the grid width
    double gridX() const;
    /// return the grid height
    double gridY() const;
    /**
     * Set the size grid to a new value
     * @param x the width of a grid unit
     * @param y the height of a grid unit
     * @see snapToGrid()
     * @see gridX()
     * @see gridY()
     */
    void setGrid(double x, double y);

    /**
     * return if snap to grid is enabled.
     * @return if snap to grid is enabled.
     * @see setGrid()
     */
    bool snapToGrid() const;
    /**
     * Set the snap to grid, forcing objects being moved/scaled to the grid.
     * @param on when true, all moving and scaling will be on the grid.
     * @see setGrid()
     */
    void setSnapToGrid(bool on);
   
private:
    bool m_snapToGrid;
    double m_gridX, m_gridY;   
};


#endif

