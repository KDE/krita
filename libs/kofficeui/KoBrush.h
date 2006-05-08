// -*- Mode: c++; c-basic-offset: 4; indent-tabs-mode: nil; tab-width: 4; -*-
/* This file is part of the KDE project
   Copyright (C) 2005-2006 Thorsten Zachmann <zachmann@kde.org>

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

#ifndef KOBRUSH_H
#define KOBRUSH_H

#include <qbrush.h>

#include "koffice_export.h"

class KoGenStyle;
class KoGenStyles;
class KoOasisContext;
class QDomElement;


enum BCType {
    BCT_PLAIN = 0,
    BCT_GHORZ = 1,
    BCT_GVERT = 2,
    BCT_GDIAGONAL1 = 3,
    BCT_GDIAGONAL2 = 4,
    BCT_GCIRCLE = 5,
    BCT_GRECT = 6,
    BCT_GPIPECROSS = 7,
    BCT_GPYRAMID = 8
};

enum FillType
{
    FT_BRUSH = 0,
    FT_GRADIENT = 1
};

class KOFFICEUI_EXPORT KoBrush
{
public:
    KoBrush();
    KoBrush(  const QBrush &brush, const QColor &gColor1, const QColor &gColor2,
              BCType gType, FillType fillType, bool unbalanced,
              int xfactor, int yfactor );

    KoBrush &operator=( const KoBrush &brush );

    void setBrush( const QBrush &brush )
        {  m_brush = brush; }
    void setGColor1( const QColor &gColor1 )
        {  m_gColor1 = gColor1; }
    void setGColor2( const QColor &gColor2 )
        {  m_gColor2 = gColor2; }
    void setGType( BCType gType )
        {  m_gType = gType; }
    void setFillType( FillType fillType )
        {  m_fillType = fillType; }
    void setGUnbalanced( bool unbalanced )
        {  m_unbalanced = unbalanced; }
    void setGXFactor( int xfactor )
        {  m_xfactor = xfactor; }
    void setGYFactor( int yfactor )
        {  m_yfactor = yfactor; }

    QBrush getBrush() const
        { return m_brush; }
    QColor getGColor1() const
        { return m_gColor1; }
    QColor getGColor2() const
        { return m_gColor2; }
    BCType getGType() const
        { return m_gType; }
    FillType getFillType() const
        { return m_fillType; }
    bool getGUnbalanced() const
        { return m_unbalanced; }
    int getGXFactor() const
        { return m_xfactor; }
    int getGYFactor() const
        { return m_yfactor; }

private:
    QBrush m_brush;
    QColor m_gColor1;
    QColor m_gColor2;
    BCType m_gType;
    FillType m_fillType;
    bool m_unbalanced;
    int m_xfactor;
    int m_yfactor;
};

#endif /* KOBRUSH_H */

