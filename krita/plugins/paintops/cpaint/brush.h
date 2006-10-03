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
#ifndef BRUSH_H
#define BRUSH_H

#include <stdio.h>
#include <stdlib.h>
#include "gauss.h"
#include "bristle.h"

#define RADIUS_INCRE 1.5
#define DX .8
#define DY .8
#define MAXPRESSURE 1023
#define MAXTX 100
#define MAXTY 100
#define MAXINK 200  //max units of ink on a bristle
#define SPREAD 1/350

class Brush {


private :

    int size;
    int numBristles;
    int shape;
    double radius;

    void setBristlesPos ();

  public :

    Bristle *bristle;  // a list of bristles

    Brush ( int );
    ~Brush () { delete bristle; };

    int GetNumBristles () { return numBristles; }
    void SetSize ( int );
    void SetInitPos ( double, double );
    void RepositionBristles ( double );

    void AddInk ();
    void RemoveInk ();

};

#endif
