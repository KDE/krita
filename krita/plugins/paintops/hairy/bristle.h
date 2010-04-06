/*
 *  Copyright (c) 2008 Lukas Tvrdy <lukast.dev@gmail.com>
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

#ifndef _BRISTLE_H_
#define _BRISTLE_H_

#include <cmath>
#include <KoColor.h>

class Bristle
{

public:
    Bristle(float x, float y, float length);
    Bristle();
    ~Bristle();

    inline float x() const{
        return m_x;
    }

    inline float y() const{
        return m_y;
    }

    inline float prevX() const{
        return m_prevX;
    }

    inline float prevY() const{
        return m_prevY;
    }

    inline float length() const{
        return m_length;
    }

    inline  const KoColor &color() const{
        return m_color;
    }

    inline int counter() const{
        return m_counter;
    }

    inline void upIncrement() {
        m_counter++;
    }

    inline float inkAmount() const{
        return m_inkAmount;
    };

    inline float distanceCenter() {
        return std::sqrt(m_x*m_x + m_y*m_y);
    }

    inline void setX(float x) {
        m_x = x;
    }
    inline void setY(float y) {
        m_y = y;
    }

    inline void setPrevX(float prevX){
        m_prevX = prevX;
    }
    
    inline void setPrevY(float prevY){
        m_prevY = prevY;
    }
    
    inline bool enabled() const{
        return m_enabled;
    }
    
    void setLength(float length);
    void setColor(const KoColor &color);

    void addInk(float value);
    void removeInk(float value);
    void setInkAmount(float inkAmount);
    void setEnabled(bool enabled);
    
    
private:
    void init(float x, float y, float length);

    // coordinates of bristle
    float m_x;
    float m_y;
    float m_prevX;
    float m_prevY;
    float m_length; // z - coordinate
    KoColor m_color;
    float m_inkAmount;

    // new dimension in bristle
    int m_counter;

    bool m_enabled;
};

#endif
