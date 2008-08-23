/* This file is part of the KDE project
   Copyright (C) 2003 Rob Buis <buis@kde.org>
   Copyright (C) 2005-2006 Laurent Montel <montel@kde.org>
   Copyright (C) 2005 Thomas Zander <zander@kde.org>
   Copyright (C) 2007 David Faure <faure@kde.org>

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

#ifndef __KOSVGPATHPARSER_H__
#define __KOSVGPATHPARSER_H__

class QString;

#include "flake_export.h"
/**
 * Parser for svg path data, passed by argument in the parseSvg() method
 *
 * The parser delivers encountered commands and parameters by calling
 * methods that correspond to those commands. Clients have to derive
 * from this class and implement the abstract command methods.
 *
 * There are two operating modes. By default the parser just delivers unaltered
 * svg path data commands and parameters. In the second mode, it will convert all
 * relative coordinates to absolute ones, and convert all curves to cubic beziers.
 */
class FLAKE_EXPORT KoSvgPathParser
{
public:
    KoSvgPathParser() {}
    virtual ~KoSvgPathParser(){}
    void parseSvg( const QString &svgInputData, bool process = false );

protected:
    virtual void svgMoveTo( qreal x1, qreal y1, bool abs = true ) = 0;
    virtual void svgLineTo( qreal x1, qreal y1, bool abs = true ) = 0;
    virtual void svgLineToHorizontal( qreal x, bool abs = true );
    virtual void svgLineToVertical( qreal y, bool abs = true );
    virtual void svgCurveToCubic( qreal x1, qreal y1, qreal x2, qreal y2, qreal x, qreal y, bool abs = true ) = 0;
    virtual void svgCurveToCubicSmooth( qreal x, qreal y, qreal x2, qreal y2, bool abs = true );
    virtual void svgCurveToQuadratic( qreal x, qreal y, qreal x1, qreal y1, bool abs = true );
    virtual void svgCurveToQuadraticSmooth( qreal x, qreal y, bool abs = true );
    virtual void svgArcTo( qreal x, qreal y, qreal r1, qreal r2, qreal angle, bool largeArcFlag, bool sweepFlag, bool abs = true );
    virtual void svgClosePath() = 0;

private:
    const char *getCoord( const char *, qreal & );
    void calculateArc( bool relative, qreal &curx, qreal &cury, qreal angle, qreal x, qreal y, qreal r1, qreal r2, bool largeArcFlag, bool sweepFlag );
};

#endif
