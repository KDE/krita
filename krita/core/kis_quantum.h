/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_Q_UINT8_H_
#define KIS_Q_UINT8_H_

#include "kis_global.h"
/**
 * An editable wrapper around the byte values for a single colour channel.
 */
class KisQuantum {

public:
    //XXX: Have removed the KisQuantumOperation mechanism since only Linear was used, which is 
    // simple assignment, and it appears to be the source of hard to track bugs. It's also
    // needlessly decreasing peformance. AP
        
    inline KisQuantum(Q_UINT8* q) :    m_quantum(q) {};
    
public:
    inline operator Q_UINT8() const { return *m_quantum; };

    
    inline Q_UINT8 operator=(Q_UINT8 q)
        {
            return *m_quantum = q;
        };

    inline Q_UINT8 operator-=(Q_UINT8 q)
        {
            return (*this = *this - q);
        }

    inline Q_UINT8 operator+=(Q_UINT8 q)
        {
            return (*this = *this + q);
        }

    /** 
     * This operator allow to acces to a neighbour quantum, it's mean to be used
     * inside a pixel
     */
    KisQuantum operator[](int index) const {
        return KisQuantum( m_quantum + index );
    }
private:
    Q_UINT8* m_quantum;
};


#endif
