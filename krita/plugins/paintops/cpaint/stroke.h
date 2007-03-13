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

#include <vector.h>
#include <math.h>
#include <stdio.h>

class Brush;
class Sample;
class KoColor;
class QImage;

class Stroke {

public:

    vector<Sample*> sampleV;

public:

    Stroke (Brush *brush);
    virtual ~Stroke();

    void draw (QPainter &);
    void setColor ( const KoColor & c );
    void storeOldPath ( double, double );

private:

    KoColor m_color;

    double lastx1, lasty1, lastx2, lasty2;
    vector<double> *oldPathx, *oldPathy;
    vector<int> *valid;
    Brush *brush;
    int numBristles;

    // test whether a bristle is touching the paper or not
    // by testing the pressure threshold and test if there is any ink
    bool testThreshold ( int, double, double, double );


};

#endif
