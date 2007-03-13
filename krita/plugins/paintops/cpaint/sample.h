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

public:

    Sample ();

    void setPressure ( int pressure ) { m_pressure = pressure; }
    int pressure () { return static_cast<int>(m_pressure); }

    void setX( int cx ) { m_x = cx; }
    int x() { return m_x; }

    void setY( int cy ) { m_y = cy; }
    int y() { return m_y; }

    void setTiltX( int cx ) { m_tiltX = cx; }
    int tiltX() { return static_cast<int>(m_tiltX); }

    void setTiltY( int cy ) { m_tiltY = cy; }
    int tiltY() { return static_cast<int>(m_tiltY); }

private:

    int m_x;
    int m_y;
    double m_pressure;
    double m_tiltX;
    double m_tiltY;

};

#endif
