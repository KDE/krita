/* This file is part of the KDE project
   Copyright (C) 2001 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef koPoint_h
#define koPoint_h

#include <QMatrix>
#include <math.h>

/**
 * A point whose coordinates are floating-point values ( "double"s ).
 * The API isn't documented, it's a perfect mirror of QPoint.
 */
class KoPoint {

public:
    KoPoint() { m_x = 0; m_y = 0; }
    KoPoint(const double &x, const double &y) : m_x(x), m_y(y) {}
    explicit KoPoint(const QPoint & p) : m_x(p.x()), m_y(p.y()) {}
    ~KoPoint() {}

    bool operator==(const KoPoint &rhs) const { return QABS(m_x-rhs.x()) < 1E-10 && QABS(m_y-rhs.y()) < 1E-10; }
    bool operator!=(const KoPoint &rhs) const { return QABS(m_x-rhs.x()) > 1E-10 || QABS(m_y-rhs.y()) > 1E-10; }

    bool isNull() const { return m_x == 0 && m_y == 0; }

    double x() const { return m_x; }
    double y() const { return m_y; }
    void setX(const double &x) { m_x = x; }
    void setY(const double &y) { m_y = y; }

    double &rx() { return m_x; }
    double &ry() { return m_y; }

    KoPoint &operator=(const KoPoint &rhs) { m_x = rhs.x(); m_y = rhs.y(); return *this; }
    KoPoint &operator+=( const KoPoint &rhs ) { m_x += rhs.x(); m_y += rhs.y(); return *this; }
    KoPoint &operator-=( const KoPoint &rhs ) { m_x -= rhs.x(); m_y -= rhs.y(); return *this; }
    KoPoint &operator*=( const double &c ) { m_x *= c; m_y *= c; return *this; }

    friend inline KoPoint operator+( const KoPoint &, const KoPoint & );
    friend inline KoPoint operator-( const KoPoint &, const KoPoint & );
    friend inline KoPoint operator*( const KoPoint &, const double & );
    friend inline KoPoint operator*( const double &, const KoPoint & );
    friend inline double  operator*( const KoPoint &a, const KoPoint &b );

    // Not in QPoint:
    void setCoords(const double &x, const double &y) { m_x = x; m_y = y; }
    KoPoint transform (const QMatrix &m) const
    {
      double x, y;
      m.map(m_x, m_y, &x, &y);
      return KoPoint(x, y);
    };

    bool isNear(const KoPoint &p, double range) const
    {
      return (p.x() >= m_x - range && p.x() <= m_x + range && p.y() >= m_y - range && p.y() <= m_y + range);
    }

    static double getAngle( const KoPoint& p1, const KoPoint& p2 ) {
	double a = atan2( p2.x() - p1.x(), p2.y() - p1.y() ) + M_PI;
	return ( ( - ( a * 360 ) / ( 2 * M_PI ) - 90 ) - 180 );
    }

    double manhattanLength() const
    {
      return QABS( m_x ) + QABS( m_y );
    }

    /// Convert to a QPoint - with precision loss!
    QPoint toQPoint() const
    {
      return QPoint( qRound( m_x ), qRound( m_y ) );
    }

private:
    double m_x, m_y;
};

inline KoPoint operator+( const KoPoint &p1, const KoPoint &p2 )
{ return KoPoint( p1.m_x+p2.m_x, p1.m_y+p2.m_y ); }

inline KoPoint operator-( const KoPoint &p1, const KoPoint &p2 )
{ return KoPoint( p1.m_x-p2.m_x, p1.m_y-p2.m_y ); }

inline KoPoint operator*( const KoPoint &p, const double &c )
{ return KoPoint( p.m_x*c, p.m_y*c ); }

inline KoPoint operator*( const double &c, const KoPoint &p )
{ return KoPoint( p.m_x*c, p.m_y*c ); }

inline double operator*( const KoPoint &a, const KoPoint &b )
{ return a.m_x * b.m_x + a.m_y * b.m_y; }

/******************************
  kDebug support
*******************************/

#include <kdebug.h>

/** Show a floating point value with great precision (use within kDebug) */
#define DEBUGDOUBLE(d) QString::number( (d), 'g', 20 )

inline kdbgstream operator<<( kdbgstream str, const KoPoint & r )  {
    // should this use DEBUGDOUBLE?
    str << "(" << r.x() << ", " << r.y() << ")";
    return str;
}

inline kndbgstream operator<<( kndbgstream str, const KoPoint & )  { return str; }

#endif
