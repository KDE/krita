/* This file is part of the KDE project
   Copyright (C) 2001 Lukas Tinkl <lukas@kde.org>

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

   This file borrows from the QSize class;
   Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
*/

#ifndef koSize_h
#define koSize_h

#include <QSize>
#include <q3tl.h>

/**
 * A size whose coordinates are floating-point values ( "double"s ).
 * The API isn't documented, it's a perfect mirror of QSize.
 */
class KoSize
{
public:
  KoSize();
  KoSize( double w, double h );

  bool   isNull()  const;
  bool   isEmpty() const;
  bool   isValid() const;

  double width()   const;
  double height()  const;
  void   setWidth( double w );
  void   setHeight( double h );

  KoSize expandedTo( const KoSize & ) const;
  KoSize boundedTo( const KoSize & ) const;

  double &rwidth();
  double &rheight();

  KoSize &operator+=( const KoSize & );
  KoSize &operator-=( const KoSize & );
  KoSize &operator*=( int c );
  KoSize &operator*=( double c );
  KoSize &operator/=( int c );
  KoSize &operator/=( double c );

  friend inline bool operator==( const KoSize &, const KoSize & );
  friend inline bool operator!=( const KoSize &, const KoSize & );
  friend inline const KoSize operator+( const KoSize &, const KoSize & );
  friend inline const KoSize operator-( const KoSize &, const KoSize & );
  friend inline const KoSize operator*( const KoSize &, int );
  friend inline const KoSize operator*( int, const KoSize & );
  friend inline const KoSize operator*( const KoSize &, double );
  friend inline const KoSize operator*( double, const KoSize & );
  friend inline const KoSize operator/( const KoSize &, int );
  friend inline const KoSize operator/( const KoSize &, double );

  inline QSize toQSize() const;
  static KoSize fromQSize( const QSize &size )
  {
    return KoSize(size.width(), size.height());
  }

  void transpose()
  {
    qSwap(wd, ht);
  }

private:
  static void warningDivByZero()
  {
#if defined(QT_CHECK_MATH)
    qWarning( "KoSize: Division by zero error" );
#endif
  }

  double wd;
  double ht;
};


/*****************************************************************************
  KoSize inline functions
 *****************************************************************************/

inline KoSize::KoSize()
{ wd = ht = -1.0; }

inline KoSize::KoSize( double w, double h )
{ wd=w; ht=h; }

inline bool KoSize::isNull() const
{ return wd==0.0 && ht==0.0; }

inline bool KoSize::isEmpty() const
{ return wd<=0.0 || ht<=0.0; }

inline bool KoSize::isValid() const
{ return wd>=0.0 && ht>=0.0; }

inline double KoSize::width() const
{ return wd; }

inline double KoSize::height() const
{ return ht; }

inline void KoSize::setWidth( double w )
{ wd=w; }

inline void KoSize::setHeight( double h )
{ ht=h; }

inline double &KoSize::rwidth()
{ return wd; }

inline double &KoSize::rheight()
{ return ht; }

inline KoSize &KoSize::operator+=( const KoSize &s )
{ wd+=s.wd; ht+=s.ht; return *this; }

inline KoSize &KoSize::operator-=( const KoSize &s )
{ wd-=s.wd; ht-=s.ht; return *this; }

inline KoSize &KoSize::operator*=( int c )
{ wd*=c; ht*=c; return *this; }

inline KoSize &KoSize::operator*=( double c )
{ wd=wd*c; ht=ht*c; return *this; }

inline bool operator==( const KoSize &s1, const KoSize &s2 )
{ return s1.wd == s2.wd && s1.ht == s2.ht; }

inline bool operator!=( const KoSize &s1, const KoSize &s2 )
{ return s1.wd != s2.wd || s1.ht != s2.ht; }

inline const KoSize operator+( const KoSize & s1, const KoSize & s2 )
{ return KoSize(s1.wd+s2.wd, s1.ht+s2.ht); }

inline const KoSize operator-( const KoSize &s1, const KoSize &s2 )
{ return KoSize(s1.wd-s2.wd, s1.ht-s2.ht); }

inline const KoSize operator*( const KoSize &s, int c )
{ return KoSize(s.wd*c, s.ht*c); }

inline const KoSize operator*( int c, const KoSize &s )
{  return KoSize(s.wd*c, s.ht*c); }

inline const KoSize operator*( const KoSize &s, double c )
{ return KoSize(s.wd*c, s.ht*c); }

inline const KoSize operator*( double c, const KoSize &s )
{ return KoSize(s.wd*c, s.ht*c); }

inline KoSize &KoSize::operator/=( int c )
{
#if defined(QT_CHECK_MATH)
  if ( c == 0 )
    warningDivByZero();
#endif
  wd/=c; ht/=c;
  return *this;
}

inline KoSize &KoSize::operator/=( double c )
{
#if defined(QT_CHECK_MATH)
  if ( c == 0.0 )
    warningDivByZero();
#endif
  wd=wd/c; ht=ht/c;
  return *this;
}

inline const KoSize operator/( const KoSize &s, int c )
{
#if defined(QT_CHECK_MATH)
  if ( c == 0 )
    KoSize::warningDivByZero();
#endif
  return KoSize(s.wd/c, s.ht/c);
}

inline const KoSize operator/( const KoSize &s, double c )
{
#if defined(QT_CHECK_MATH)
  if ( c == 0.0 )
    KoSize::warningDivByZero();
#endif
  return KoSize(s.wd/c, s.ht/c);
}

inline KoSize KoSize::expandedTo( const KoSize & otherSize ) const
{
  return KoSize( qMax(wd,otherSize.wd), qMax(ht,otherSize.ht) );
}

inline KoSize KoSize::boundedTo( const KoSize & otherSize ) const
{
  return KoSize( qMin(wd,otherSize.wd), qMin(ht,otherSize.ht) );
}

inline QSize KoSize::toQSize() const
{
  return QSize(qRound(wd), qRound(ht));
}

/******************************
  kDebug support
*******************************/
#include <kdebug.h>

inline kdbgstream operator<<( kdbgstream str, const KoSize & sz )  { str << "[" << sz.width() << "x" << sz.height() << "]"; return str; }
inline kndbgstream operator<<( kndbgstream str, const KoSize & )  { return str; }

#endif
