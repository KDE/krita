/*
 * Copyright (c) 2000 Clara Chan
 * Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#ifndef BRISTLE_H
#define BRISTLE_H

#include <stdio.h>
#include <math.h>

#include <KoColor.h>

class Bristle {

    friend class Brush;


public:

    Bristle () {};

    ~Bristle () {}

    void InitPos ( double );

    void SetPos ( double, double );

    void SetPreThres ( double p ) { pressureThres = p; }

    void SetTXThres ( double tx ) { txThres = tx; }

    void SetTYThres ( double ty ) { tyThres = ty; }

    void setInitialPosition ( double, double );

    int GetX () { return static_cast<int>(x); }

    int GetY () { return static_cast<int>(y); }

    double GetPreThres () { return pressureThres; }

    double GetTXThres () { return txThres; }

    double GetTYThres () { return tyThres; }

    void Reposition ( double );

    double GetThickness () { return thickness; }

    int GetInkAmount () { return inkAmount; }

    void SetInkAmount ( int i ) { inkAmount = i; }

    void depleteInk ( int i ) { inkAmount -= i; }

    void addInk ( int i ) { inkAmount += i; }

    double DistanceFromCenter ();

    void initializeThickness ( int );

private:

    double x, y, lastx, lasty;  // offset from the mouse click position
    double thickness;
    double pressureThres, txThres, tyThres;
    int inkAmount;
    KoColor inkColor;

};

#endif
