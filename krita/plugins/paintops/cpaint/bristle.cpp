/*
 *  Copyright (c) 2000 Clara Chan
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
#include "bristle.h"
#include "math.h"

void Bristle::InitThickness ( int size )
{
  thickness = gauss::gaussian ( log10(size+7)*2.6, size/3.0, 0 );
  if ( thickness < 0 )
    thickness = -thickness;
  while ( thickness > 4.0 )
    thickness -= 0.5;
}


// called by constructor of brush
void Bristle::SetPos ( double x1, double y1 )
{
  x = x1;
  y = y1;
}


void Bristle::SetInitPos ( double x1, double y1 )
{
  lastx = x;
  lasty = y;
}


void Bristle::Reposition ( double p )
{
  lastx = x;
  lasty = y;
  x = lastx + ( gauss::gaussian ( 0.0, p/500.0, 1 ) );
  y = lasty + ( gauss::gaussian ( 0.0, p/500.0, 1 ) );
}


double Bristle::DistanceFromCenter ()
{
  return sqrt ( x*x + y*y );
}
