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

class Bristle;

class Brush {

public:

    // Eeek....
    Bristle * m_bristles;  // a list of bristles

public:

    Brush ( int );
    ~Brush ();

    int size() {
        return m_size;
    }
    int numberOfBristles () { return m_numBristles; }
    void setSize ( int );
    void setInitialPosition ( double, double );
    void repositionBristles ( double );
    void setBristlesPos();

    void addInk ();
    void removeInk ();

private:

    int m_size;
    int m_numBristles;
    double m_radius;

};

#endif
