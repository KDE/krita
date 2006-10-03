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

#ifndef STROKE_H
#define STROKE_H

#include "sample.h"
#include <vector.h>
#include <math.h>
#include <stdio.h>
#include <kis_types.h>
#include <kis_color.h>

#include "brush.h"
#define MAXBRUSHSIZE 100


class Stroke {

  private :

    KisColor m_color;

    double lastx1, lasty1, lastx2, lasty2;
    vector<double> *oldPathx, *oldPathy;
    vector<int> *valid;
    Brush *brush;
    int numBristles;
    int newStroke;

    int testThreshold ( int, double, double, double );

  protected :
    
    int sealFlag;
    
  public :

    vector<Sample*> sampleV;

    Stroke (Brush *brush);
    virtual ~Stroke() {};

    void Draw (KisPaintDeviceSP);
    void Redraw ();
    void FreeSamples ();
    void StoreColor ( const KisColor & c );
    void StoreOldPath ( double, double );
    void ResetBrush ();

};

#endif
