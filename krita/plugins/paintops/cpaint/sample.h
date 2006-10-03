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
#ifndef SAMPLE_H
#define SAMPLE_H


class Sample {

  private :

    int x, y;
    double p, tx, ty;

  public :

    Sample ();
    void SetPressure ( int pre ) { p = pre; }
    void SetX ( int cx ) { x = cx; }
    void SetY ( int cy ) { y = cy; }
    void SetTX ( int cx ) { tx = cx; }
    void SetTY ( int cy ) { ty = cy; }
    int GetPressure () { return static_cast<int>(p); }
    int GetX () { return x; }
    int GetY () { return y; }
    int GetTX () { return static_cast<int>(tx); }
    int GetTY () { return static_cast<int>(ty); }
};

#endif
