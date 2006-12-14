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

#include "KoUnit.h"
#include "KoGridData.h"

KoGridData::KoGridData()
 :m_snapToGrid(false),
  m_gridX(MM_TO_POINT(5.0)),
  m_gridY(MM_TO_POINT(5.0)),
  m_gridColor(Qt::black)
{
}

double KoGridData::gridX() const 
{ 
   return m_gridX; 
}
   
double KoGridData::gridY() const 
{
  return m_gridY; 
}

void KoGridData::setGrid(double x, double y) 
{ 
   m_gridX = x; 
   m_gridY = y; 
}

bool KoGridData::snapToGrid() const 
{ 
   return m_snapToGrid; 
}

void KoGridData::setSnapToGrid(bool on) 
{ 
   m_snapToGrid = on; 
}

QColor KoGridData::gridColor() const
{
  return m_gridColor;
}

void KoGridData::setGridColor( const QColor & color)
{
  m_gridColor=color;
}

