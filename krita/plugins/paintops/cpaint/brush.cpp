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
#include "brush.h"


Brush::Brush ( int s )
{
    bristle = 0;
    shape = 0;
    SetSize ( s );
    for ( int i=0; i<3; i++ )
        AddInk ();
}


void Brush::SetInitPos ( double x, double y )
{
  int i;

  for ( i=0; i<numBristles; i++ ) {
    bristle[i].SetInitPos ( x, y );
  }
}


// called by constructor and callbacks
void Brush::SetSize ( int s )
{
  size = s;

  setBristlesPos();
  printf ("Num of Bristle for Brush %d is %d\n", size, numBristles );
}


void Brush::RepositionBristles ( double pre )
{
  double x, y, dx, dy, px, py, maxradius;
  int i;

  if ( pre <= 1.0 ) pre = 1.0;  // make sure pressure is at least 1
  radius = size * RADIUS_INCRE * ( 1+ pre*SPREAD );
  maxradius = 6 * RADIUS_INCRE * ( 1 + MAXPRESSURE*SPREAD );
  dx = DX * ( 1 + pre*SPREAD );
  dy = DY * ( 1 + pre*SPREAD );

  x = y = -1.0 * radius;
  i = 0;
  while ( x < radius && i < numBristles ) {
    while ( y < radius ) {
      if ( x*x + y*y < radius*radius ) {
	px = gauss::gaussian ( x, .5*DX, 1 );
	py = gauss::gaussian ( y, .5*DY, 1 );
	if ( fabs (px) > x+DX )
	  px = x;
	if ( fabs (py) > y+DY )
	  py = y;
	bristle[i++].SetPos ( px, py );
      }
      y += dy;
    }
    y = -1.0 * radius;
    x += dx;
  }

  // Create ink stealing
  int indx, i1, i2, i3, i4;
  double rn;

  if ( size > 1 ) {
    for ( i=0; i<numBristles*0.01*size; i++ ) {
      rn = ( double ) rand ();
      rn = rn / RAND_MAX * numBristles;
      indx = ( int ) rn;
      if ( indx > numBristles-1 )
	indx = numBristles/2;
      if ( indx > numBristles-2 )
	indx = indx - 2;
      if ( indx < 2 )
	indx = 2;

      if ( bristle[indx-2].GetInkAmount()-size < 0 )
	i1 = bristle[indx-2].GetInkAmount();
      else
	i1 = size;
      if ( bristle[indx-1].GetInkAmount()-size < 0 )
	i2 = bristle[indx-1].GetInkAmount();
      else
	i2 = size;
      if ( bristle[indx+1].GetInkAmount()-size < 0 )
	i3 = bristle[indx+1].GetInkAmount();
      else
	i3 = size;
      if ( bristle[indx+2].GetInkAmount()-size < 0 )
	i4 = bristle[indx+2].GetInkAmount();
      else
	i4 = size;
      bristle[indx-2].SetInkAmount ( bristle[indx-2].GetInkAmount()-i1 );
      bristle[indx-1].SetInkAmount ( bristle[indx-2].GetInkAmount()-i2 );
      bristle[indx+1].SetInkAmount ( bristle[indx+1].GetInkAmount()-i3 );
      bristle[indx+2].SetInkAmount ( bristle[indx+2].GetInkAmount()-i4 );
      bristle[indx].SetInkAmount ( bristle[indx].GetInkAmount()+i1+i2+i3+i4 );
    }
  }
}


void Brush::setBristlesPos ()
{
  double x, y, p, tx, ty, maxradius, px, py, xxyy;
  int i, j;

  numBristles = 0;
  radius = size * RADIUS_INCRE * ( 1 + 1/500.0 );
  maxradius = 6 * RADIUS_INCRE * ( 1 + MAXPRESSURE/500.0 );
  i = pow ( radius/DX * radius/DY, 2 ) * 4;
  if ( bristle )
    delete ( bristle );
  bristle = new Bristle [i];

  x = y = -1.0 * radius;
  while ( x < radius ) {
    while ( y < radius ) {
      if ( (xxyy=(x*x + y*y)) < radius*radius ) {
	px = gauss::gaussian ( x, .5*DX, 1 );
	if ( fabs (px) > x+DX )
	  px = x;
	py = gauss::gaussian ( y, .5*DY, 1 );
	if ( fabs (py) > y+DY )
	  py = y;
	bristle[numBristles].SetPos ( px, py );
	p = sqrt(px*px+py*py)/(radius) * MAXPRESSURE;
	bristle[numBristles].SetPreThres ( p );

	tx = (px / radius)*(px/radius) * 60.0;
	ty = (py / radius)*(py/radius) * 60.0;

	bristle[numBristles].SetTXThres ( tx );
	bristle[numBristles].SetTYThres ( ty );
	numBristles++;
      }
      y += DY;
    }
    y = -1.0 * radius;
    x += DX;
  }

  for ( j=0; j<numBristles; j++ ) {
    bristle[j].InitThickness ( size );
  }
}


void Brush::AddInk ()
{
  int i, indx, totalInk, ink2;
  double rn, ink;

  totalInk = numBristles * MAXINK * 0.2;
  for ( i=0; i<totalInk; i++ ) {
    rn = ( double ) rand ();
    rn = rn / RAND_MAX * numBristles;
    indx = ( int ) rn;
    if ( indx > numBristles-1 )
      indx = numBristles/2;
    if ( bristle[indx].GetInkAmount() < MAXINK ) {
      ink = bristle[indx].DistanceFromCenter() / radius;
      ink = 10.0 / ink + gauss::gaussian ( 10.0, 5.0, 0 );
      ink2  = ink;
      if ( ink2 > 200 ) ink2 = 200;
	bristle[indx].AddInk ( ink2 );
	totalInk -= ink2;
    }
  }
}


void Brush::RemoveInk ()
{
  int i, indx, totalInk;
  double rn;

  totalInk = numBristles * MAXINK * 0.2;
  for ( i=0; i<totalInk; i++ ) {
    rn = ( double ) rand ();
    rn = rn / RAND_MAX * numBristles;
    indx = ( int ) rn;
    if ( indx > numBristles-1 )
      indx = numBristles/2;
    if ( bristle[indx].GetInkAmount() > 0 ) {
      bristle[indx].MinusInk(1);
      totalInk--;
    }
  }
}
