/* 
 * Copyright (C)  Hans Bakker <hansmbakker@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef MOUSE3DEVENT_H
#define MOUSE3DEVENT_H

enum
{
    MOUSE3D_EVENT_MOTION,
    MOUSE3D_EVENT_BUTTON	/* includes both press and release */
};

/**
 * XXX
 */ 
class Ko3DMouseEvent
{
public:
    Ko3DMouseEvent();
    virtual ~Ko3DMouseEvent();

    int getType();

    int getX();
    int getY();
    int getZ();
    int getRX();
    int getRY();
    int getRZ();

    int getBNum();
    int getPress();

    void setType(int type);
    void setMovement(int x, int y, int z, int rx, int ry, int rz);
    void setButton(int bnum, int press);

protected:
private:
    int m_type;
    int m_x, m_y, m_z, m_rx, m_ry, m_rz;
    int m_bnum, m_press;
};

#endif // MOUSE3DEVENT_H
