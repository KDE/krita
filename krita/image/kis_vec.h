/*
 *  kis_vec.h - part of KImageShop
 *
 *  Copyright (c) 1999 Matthias Elter <me@kde.org>
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

#ifndef __kis_vec_h__
#define __kis_vec_h__

#include <math.h>
#include <cfloat>
#include <QPoint>
#include "kis_point.h"
#include <krita_export.h>

/*
 * vector classes
 */
const double epsilon = DBL_EPSILON;

class KRITAIMAGE_EXPORT KisVector2D
{
public:
    KisVector2D();
    KisVector2D(double x, double y);
    KisVector2D(const QPoint& p);
    KisVector2D(const KisPoint& p);

    bool isNull()    const;

    double length() const;

    double     x() const;
    double     y() const;
    void   setX(double);
    void   setY(double);

    KisVector2D &normalize();
    double  dotProduct(const KisVector2D &) const;

    KisVector2D &operator+=(const KisVector2D &);
    KisVector2D &operator-=(const KisVector2D &);
    KisVector2D &operator*=(int);
    KisVector2D &operator*=(long);
    KisVector2D &operator*=(double);
    KisVector2D &operator/=(int);
    KisVector2D &operator/=(long);
    KisVector2D &operator/=(double);

    friend inline bool operator==(const KisVector2D &, const KisVector2D &);
    friend inline bool operator!=(const KisVector2D &, const KisVector2D &);
    friend inline KisVector2D operator+(const KisVector2D &, const KisVector2D &);
    friend inline KisVector2D operator-(const KisVector2D &, const KisVector2D &);
    friend inline KisVector2D operator*(const KisVector2D &, int);
    friend inline KisVector2D operator*(int, const KisVector2D &);
    friend inline KisVector2D operator*(const KisVector2D &, long);
    friend inline KisVector2D operator*(long, const KisVector2D &);
    friend inline KisVector2D operator*(const KisVector2D &, double);
    friend inline KisVector2D operator*(double, const KisVector2D &);
    friend inline KisVector2D operator-(const KisVector2D &);
    friend inline KisVector2D operator/(const KisVector2D &, int);
    friend inline KisVector2D operator/(const KisVector2D &, long);
    friend inline KisVector2D operator/(const KisVector2D &, double);

    KisPoint toKisPoint() const;

private:
    double m_x;
    double m_y;
};

inline KisVector2D::KisVector2D()
{ m_x=0; m_y=0; }

inline KisVector2D::KisVector2D(double x, double y)
{ m_x=x; m_y=y; }

inline KisVector2D::KisVector2D(const QPoint& p)
{
    m_x=p.x(); m_y=p.y();
}

inline KisVector2D::KisVector2D(const KisPoint& p)
{
    m_x=p.x(); m_y=p.y();
}

inline bool KisVector2D::isNull() const
{ return fabs(m_x) < epsilon && fabs(m_y) < epsilon; }

inline double KisVector2D::length() const
{  return (sqrt(m_x*m_x + m_y*m_y)); }

inline double KisVector2D::dotProduct(const KisVector2D &v) const
{ return m_x*v.m_x + m_y*v.m_y; }

inline double KisVector2D::x() const
{ return m_x; }

inline double KisVector2D::y() const
{ return m_y; }

inline void KisVector2D::setX(double x)
{ m_x=x; }

inline void KisVector2D::setY(double y)
{ m_y=y; }

inline KisVector2D &KisVector2D::operator+=(const KisVector2D &v)
{ m_x+=v.m_x; m_y+=v.m_y; return *this; }

inline KisVector2D &KisVector2D::operator-=(const KisVector2D &v)
{ m_x-=v.m_x; m_y-=v.m_y; return *this; }

inline KisVector2D &KisVector2D::operator*=(int c)
{ m_x*=c; m_y*=c; return *this; }

inline KisVector2D &KisVector2D::operator*=(long c)
{ m_x*=c; m_y*=c; return *this; }

inline KisVector2D &KisVector2D::operator*=(double c)
{ m_x*=c; m_y*=c; return *this; }

inline bool operator==(const KisVector2D &v1, const KisVector2D &v2)
{ return fabs(v1.m_x - v2.m_x) < epsilon && fabs(v1.m_y - v2.m_y) < epsilon; }

inline bool operator!=(const KisVector2D &v1, const KisVector2D &v2)
{ return !(v1 == v2); }

inline KisVector2D operator+(const KisVector2D &v1, const KisVector2D &v2)
{ return KisVector2D(v1.m_x+v2.m_x, v1.m_y+v2.m_y); }

inline KisVector2D operator-(const KisVector2D &v1, const KisVector2D &v2)
{ return KisVector2D(v1.m_x-v2.m_x, v1.m_y-v2.m_y); }

inline KisVector2D operator*(const KisVector2D &v, int c)
{ return KisVector2D((v.m_x*c), (v.m_y*c)); }

inline KisVector2D operator*(int c, const KisVector2D &v)
{ return KisVector2D((v.m_x*c), (v.m_y*c)); }

inline KisVector2D operator*(const KisVector2D &v, long c)
{ return KisVector2D((v.m_x*c), (v.m_y*c)); }

inline KisVector2D operator*(long c, const KisVector2D &v)
{ return KisVector2D((v.m_x*c), (v.m_y*c)); }

inline KisVector2D operator*(const KisVector2D &v, double c)
{ return KisVector2D(v.m_x*c, v.m_y*c); }

inline KisVector2D operator*(double c, const KisVector2D &v)
{ return KisVector2D(v.m_x*c, v.m_y*c); }

inline KisVector2D operator-(const KisVector2D &v)
{ return KisVector2D(-v.m_x, -v.m_y); }

inline KisVector2D operator/(const KisVector2D &v, int c)
{
    if (c != 0) {
        return KisVector2D(v.x() / c, v.y() / c);
    } else {
        return v;
    }
}

inline KisVector2D operator/(const KisVector2D &v, long c)
{
    if (c != 0) {
        return KisVector2D(v.x() / c, v.y() / c);
    } else {
        return v;
    }
}

inline KisVector2D operator/(const KisVector2D &v, double c)
{
    if (c > DBL_EPSILON || c < -DBL_EPSILON) {
        return KisVector2D(v.x() / c, v.y() / c);
    } else {
        return v;
    }
}

inline KisVector2D &KisVector2D::operator/=(int c)
{
    if (!c == 0)
    {
        m_x/=c;
        m_y/=c;
    }
    return *this;
}

inline KisVector2D &KisVector2D::operator/=(long c)
{
    if (!c == 0)
    {
        m_x/=c;
        m_y/=c;
    }
    return *this;
}

inline KisVector2D &KisVector2D::operator/=(double c)
{
    if (!c == 0)
    {
        m_x/=c;
        m_y/=c;
    }
    return *this;
}

inline KisPoint KisVector2D::toKisPoint() const
{
    return KisPoint(m_x, m_y);
}

class KisVector3D
{
public:
    KisVector3D();
    KisVector3D(double x, double y, double z = 0);
    KisVector3D(int x, int y, int z = 0);
    KisVector3D(long x, long y, long z = 0);

    bool isNull()    const;

    double length() const;

    double     x() const;
    double     y() const;
    double     z() const;
    void   setX(double);
    void   setY(double);
    void   setZ(double);

    KisVector3D &normalize();
    KisVector3D &crossProduct(const KisVector3D &);
    double  dotProduct(const KisVector3D &) const;

    KisVector3D &operator+=(const KisVector3D &);
    KisVector3D &operator-=(const KisVector3D &);
    KisVector3D &operator*=(int);
    KisVector3D &operator*=(long);
    KisVector3D &operator*=(double);
    KisVector3D &operator/=(int);
    KisVector3D &operator/=(long);
    KisVector3D &operator/=(double);

    friend inline bool operator==(const KisVector3D &, const KisVector3D &);
    friend inline bool operator!=(const KisVector3D &, const KisVector3D &);
    friend inline KisVector3D operator+(const KisVector3D &, const KisVector3D &);
    friend inline KisVector3D operator-(const KisVector3D &, const KisVector3D &);
    friend inline KisVector3D operator*(const KisVector3D &, int);
    friend inline KisVector3D operator*(int, const KisVector3D &);
    friend inline KisVector3D operator*(const KisVector3D &, long);
    friend inline KisVector3D operator*(long, const KisVector3D &);
    friend inline KisVector3D operator*(const KisVector3D &, double);
    friend inline KisVector3D operator*(double, const KisVector3D &);
    friend inline KisVector3D operator-(const KisVector3D &);
    friend inline KisVector3D operator/(const KisVector3D &, int);
    friend inline KisVector3D operator/(const KisVector3D &, long);
    friend inline KisVector3D operator/(const KisVector3D &, double);

private:
    double m_x;
    double m_y;
    double m_z;
};

inline KisVector3D::KisVector3D()
{ m_x=0; m_y=0; m_z=0; }

inline KisVector3D::KisVector3D(double x, double y, double z)
{ m_x=x; m_y=y; m_z=z; }

inline KisVector3D::KisVector3D(int x, int y, int z)
{ m_x=static_cast<double>(x); m_y=static_cast<double>(y); m_z=static_cast<double>(z); }

inline KisVector3D::KisVector3D(long x, long y, long z)
{ m_x=static_cast<double>(x); m_y=static_cast<double>(y); m_z=static_cast<double>(z); }

inline bool KisVector3D::isNull() const
{ return fabs(m_x) < epsilon && fabs(m_y) < epsilon && fabs(m_z) < epsilon; }

inline double KisVector3D::length() const
{  return (sqrt(m_x*m_x + m_y*m_y + m_z*m_z)); }

inline double KisVector3D::dotProduct(const KisVector3D &v) const
{ return m_x*v.m_x + m_y*v.m_y + m_z*v.m_z; }

inline double KisVector3D::x() const
{ return m_x; }

inline double KisVector3D::y() const
{ return m_y; }

inline double KisVector3D::z() const
{ return m_z; }

inline void KisVector3D::setX(double x)
{ m_x=x; }

inline void KisVector3D::setY(double y)
{ m_y=y; }

inline void KisVector3D::setZ(double z)
{ m_z=z; }

inline KisVector3D &KisVector3D::operator+=(const KisVector3D &v)
{ m_x+=v.m_x; m_y+=v.m_y; m_z+=v.m_z; return *this; }

inline KisVector3D &KisVector3D::operator-=(const KisVector3D &v)
{ m_x-=v.m_x; m_y-=v.m_y; m_z-=v.m_z; return *this; }

inline KisVector3D &KisVector3D::operator*=(int c)
{ m_x*=c; m_y*=c; m_z*=c; return *this; }

inline KisVector3D &KisVector3D::operator*=(long c)
{ m_x*=c; m_y*=c; m_z*=c; return *this; }

inline KisVector3D &KisVector3D::operator*=(double c)
{ m_x*=c; m_y*=c; m_z*=c; return *this; }

inline bool operator==(const KisVector3D &v1, const KisVector3D &v2)
{ return fabs(v1.m_x - v2.m_x) < epsilon && fabs(v1.m_y - v2.m_y) < epsilon && fabs(v1.m_z - v2.m_z) < epsilon; }

inline bool operator!=(const KisVector3D &v1, const KisVector3D &v2)
{ return !(v1 == v2); }

inline KisVector3D operator+(const KisVector3D &v1, const KisVector3D &v2)
{ return KisVector3D(v1.m_x+v2.m_x, v1.m_y+v2.m_y, v1.m_z+v2.m_z); }

inline KisVector3D operator-(const KisVector3D &v1, const KisVector3D &v2)
{ return KisVector3D(v1.m_x-v2.m_x, v1.m_y-v2.m_y, v1.m_z-v2.m_z); }

inline KisVector3D operator*(const KisVector3D &v, int c)
{ return KisVector3D((v.m_x*c), (v.m_y*c), (v.m_z*c)); }

inline KisVector3D operator*(int c, const KisVector3D &v)
{ return KisVector3D((v.m_x*c), (v.m_y*c), (v.m_z*c)); }

inline KisVector3D operator*(const KisVector3D &v, long c)
{ return KisVector3D((v.m_x*c), (v.m_y*c), (v.m_z*c)); }

inline KisVector3D operator*(long c, const KisVector3D &v)
{ return KisVector3D((v.m_x*c), (v.m_y*c), (v.m_z*c)); }

inline KisVector3D operator*(const KisVector3D &v, double c)
{ return KisVector3D(v.m_x*c, v.m_y*c, v.m_z*c); }

inline KisVector3D operator*(double c, const KisVector3D &v)
{ return KisVector3D(v.m_x*c, v.m_y*c, v.m_z*c); }

inline KisVector3D operator-(const KisVector3D &v)
{ return KisVector3D(-v.m_x, -v.m_y, -v.m_z); }

inline KisVector3D &KisVector3D::operator/=(int c)
{
    if (!c == 0)
    {
        m_x/=c;
        m_y/=c;
        m_z/=c;
    }
    return *this;
}

inline KisVector3D &KisVector3D::operator/=(long c)
{
    if (!c == 0)
    {
        m_x/=c;
        m_y/=c;
        m_z/=c;
    }
    return *this;
}

inline KisVector3D &KisVector3D::operator/=(double c)
{
    if (!c == 0)
    {
        m_x/=c;
        m_y/=c;
        m_z/=c;
    }
    return *this;
}

#endif
