/*
    Copyright (C) 2011 Silvio Heinrich <plassy@web.de>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/


#ifndef H_KIS_COLOR_H
#define H_KIS_COLOR_H

#include <QtGlobal>
#include <Eigen/Core>
#include <QColor>

class KisColor
{
public:
    enum Type { HSY, HSV, HSL, HSI };
    typedef Eigen::Vector4f VecHSXA;
    typedef Eigen::Vector3f VecRGB;
    
    struct Core
    {
        virtual ~Core() { }
        virtual void setRGB(float r, float g, float b, float a) = 0;
        virtual void setHSX(float h, float s, float x, float a) = 0;
        virtual void updateRGB() = 0;
        virtual void updateHSX() = 0;
        
        VecRGB  rgb;
        VecHSXA hsx;
        Type    type;
    };
    
public:
     KisColor(Type type=HSY);
     KisColor(float hue, float a=1.0f, Type type=HSY);
     KisColor(float r, float g, float b, float a=1.0f, Type type=HSY);
     KisColor(const QColor& color, Type type=HSY);
     KisColor(Qt::GlobalColor color, Type type=HSY);
     KisColor(const KisColor& color);
     KisColor(const KisColor& color, Type type);
	~KisColor();
    
    inline Type getType()     const { return core()->type;   }
    inline bool hasUndefHue() const { return getS() == 0.0f; }
    
    inline float getR() const { return core()->rgb(0); }
    inline float getG() const { return core()->rgb(1); }
    inline float getB() const { return core()->rgb(2); }
    inline float getH() const { return core()->hsx(0); }
    inline float getS() const { return core()->hsx(1); }
    inline float getX(float gamma=1.0f) const { return pow(core()->hsx(2), 1/gamma); }
    inline float getA() const { return core()->hsx(3); }
    
    inline void setR(float v) { setRGB(v, core()->rgb(1), core()->rgb(2), core()->hsx(3)); }
    inline void setG(float v) { setRGB(core()->rgb(0), v, core()->rgb(2), core()->hsx(3)); }
    inline void setB(float v) { setRGB(core()->rgb(0), core()->rgb(1), v, core()->hsx(3)); }
    inline void setH(float v) { setHSX(v, core()->hsx(1), core()->hsx(2), core()->hsx(3)); }
    inline void setS(float v) { setHSX(core()->hsx(0), v, core()->hsx(2), core()->hsx(3)); }
    inline void setX(float v, float gamma=1.0f) {
        setHSX(core()->hsx(0), core()->hsx(1), v, core()->hsx(3), gamma);
    }
    inline void setA(float v) { core()->hsx(3) = qBound(0.0f, v, 1.0f);                    }
    
    inline QColor         getQColor() const { return QColor(getR()*255, getG()*255, getB()*255, getA()*255); }
    
    inline void setRGB(float r, float g, float b, float a=1.0f) { core()->setRGB(r, g, b, a); }
    inline void setHSX(float h, float s, float x, float a=1.0f, float gamma=1.0f) {
        core()->setHSX(h, s, pow(x, gamma), a);
    }

    KisColor& operator = (const KisColor& color);
    
    friend KisColor operator - (const KisColor& a, const KisColor& b) {
        KisColor result;
        result.core()->hsx = a.core()->hsx - b.core()->hsx;
        result.core()->updateRGB();
        
        if(a.hasUndefHue() || b.hasUndefHue())
            result.setH(0.0f);
        
        return result;
    }
    
    friend KisColor operator + (const KisColor& a, const KisColor& b) {
        KisColor result;
        result.core()->hsx = a.core()->hsx + b.core()->hsx;
        result.core()->updateRGB();
        return result;
    }
    
    friend KisColor operator * (const KisColor& a, float b) {
        KisColor result;
        result.core()->hsx = a.core()->hsx * b;
        result.core()->updateRGB();
        return result;
    }
    
    friend KisColor operator * (float a, const KisColor& b) {
        return b * a;
    }
    
private:
    void initRGB(Type type, float r, float g, float b, float a);
    void initHSX(Type type, float h, float s, float x, float a);
    inline Core*       core()       { return reinterpret_cast<Core*>      (m_coreData + m_offset); }
    inline const Core* core() const { return reinterpret_cast<const Core*>(m_coreData + m_offset); }
    
private:
    quint8 m_coreData[sizeof(Core) + 15];
	quint8 m_offset;
};

#endif // H_KIS_COLOR_H
