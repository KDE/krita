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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#ifndef __kis_vec_h__
#define __kis_vec_h__

#include <math.h>

/*
 * 2D/3D vector class
 */

class KisVector
{
 public:
  KisVector();
  KisVector(double x, double y, double z = 0);
  KisVector(int x, int y, int z = 0);
  KisVector(long x, long y, long z = 0);

  bool isNull()	const;

  double length() const;

  double	 x() const;
  double	 y() const;
  double	 z() const;
  void   setX(double);
  void   setY(double);
  void   setZ(double);

  KisVector &normalize();
  KisVector &crossProduct(const KisVector &);
  double  dotProduct(const KisVector &) const;

  KisVector &operator+=(const KisVector &);
  KisVector &operator-=(const KisVector &);
  KisVector &operator*=(int);
  KisVector &operator*=(long);
  KisVector &operator*=(double);
  KisVector &operator/=(int);
  KisVector &operator/=(long);
  KisVector &operator/=(double);

  friend inline bool operator==(const KisVector &, const KisVector &);
  friend inline bool operator!=(const KisVector &, const KisVector &);
  friend inline KisVector operator+(const KisVector &, const KisVector &);
  friend inline KisVector operator-(const KisVector &, const KisVector &);
  friend inline KisVector operator*(const KisVector &, int);
  friend inline KisVector operator*(int, const KisVector &);
  friend inline KisVector operator*(const KisVector &, long);
  friend inline KisVector operator*(long, const KisVector &);
  friend inline KisVector operator*(const KisVector &, double);
  friend inline KisVector operator*(double, const KisVector &);
  friend inline KisVector operator-(const KisVector &);
  friend inline KisVector operator/(const KisVector &, int);
  friend inline KisVector operator/(const KisVector &, long);
  friend inline KisVector operator/(const KisVector &, double);
   
 private:
  double m_x;
  double m_y;
  double m_z;
};

inline KisVector::KisVector()
{ m_x=0; m_y=0; m_z=0; }

inline KisVector::KisVector(double x, double y, double z)
{ m_x=x; m_y=y; m_z=z; }

inline KisVector::KisVector(int x, int y, int z)
{ m_x=static_cast<double>(x); m_y=static_cast<double>(y); m_z=static_cast<double>(z); }

inline KisVector::KisVector(long x, long y, long z)
{ m_x=static_cast<double>(x); m_y=static_cast<double>(y); m_z=static_cast<double>(z); }

inline bool KisVector::isNull() const
{ return m_x == 0 && m_y == 0 && m_z == 0; }

inline double KisVector::length() const
{  return (sqrt(m_x*m_x + m_y*m_y + m_z*m_z)); }

inline double KisVector::dotProduct(const KisVector &v) const
{ return m_x*v.m_x + m_y*v.m_y + m_z*v.m_z; }

inline double KisVector::x() const
{ return m_x; }

inline double KisVector::y() const
{ return m_y; }

inline double KisVector::z() const
{ return m_z; }

inline void KisVector::setX(double x)
{ m_x=x; }

inline void KisVector::setY(double y)
{ m_y=y; }

inline void KisVector::setZ(double z)
{ m_z=z; }

inline KisVector &KisVector::operator+=(const KisVector &v)
{ m_x+=v.m_x; m_y+=v.m_y; m_z+=v.m_z; return *this; }

inline KisVector &KisVector::operator-=(const KisVector &v)
{ m_x-=v.m_x; m_y-=v.m_y; m_z-=v.m_z; return *this; }

inline KisVector &KisVector::operator*=(int c)
{ m_x*=c; m_y*=c; m_z*=c; return *this; }

inline KisVector &KisVector::operator*=(long c)
{ m_x*=c; m_y*=c; m_z*=c; return *this; }

inline KisVector &KisVector::operator*=(double c)
{ m_x*=c; m_y*=c; m_z*=c; return *this; }

inline bool operator==(const KisVector &v1, const KisVector &v2)
{ return v1.m_x == v2.m_x && v1.m_y == v2.m_y && v1.m_z == v2.m_z; }

inline bool operator!=(const KisVector &v1, const KisVector &v2)
{ return v1.m_x != v2.m_x || v1.m_y != v2.m_y || v1.m_z != v2.m_z; }

inline KisVector operator+(const KisVector &v1, const KisVector &v2)
{ return KisVector(v1.m_x+v2.m_x, v1.m_y+v2.m_y, v1.m_z+v2.m_z); }

inline KisVector operator-(const KisVector &v1, const KisVector &v2)
{ return KisVector(v1.m_x-v2.m_x, v1.m_y-v2.m_y, v1.m_z-v2.m_z); }

inline KisVector operator*(const KisVector &v, int c)
{ return KisVector((v.m_x*c), (v.m_y*c), (v.m_z*c)); }

inline KisVector operator*(int c, const KisVector &v)
{ return KisVector((v.m_x*c), (v.m_y*c), (v.m_z*c)); }

inline KisVector operator*(const KisVector &v, long c)
{ return KisVector((v.m_x*c), (v.m_y*c), (v.m_z*c)); }

inline KisVector operator*(long c, const KisVector &v)
{ return KisVector((v.m_x*c), (v.m_y*c), (v.m_z*c)); }

inline KisVector operator*(const KisVector &v, double c)
{ return KisVector(v.m_x*c, v.m_y*c, v.m_z*c); }

inline KisVector operator*(double c, const KisVector &v)
{ return KisVector(v.m_x*c, v.m_y*c, v.m_z*c); }

inline KisVector operator-(const KisVector &v)
{ return KisVector(-v.m_x, -v.m_y, -v.m_z); }

inline KisVector &KisVector::operator/=(int c)
{
  if (!c == 0)
    {
      m_x/=c;
      m_y/=c;
      m_z/=c;
    }
    return *this;
}

inline KisVector &KisVector::operator/=(long c)
{
  if (!c == 0)
    {
      m_x/=c;
      m_y/=c;
      m_z/=c;
    }
    return *this;
}

inline KisVector &KisVector::operator/=(double c)
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
