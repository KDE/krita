/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

#ifndef _KOPATHSHAPELOADER_H_
#define _KOPATHSHAPELOADER_H_

#include "KoSvgPathParser.h"

class KoPathShape;

/// A helper class for parsing path data when loading from svg/odf
class FLAKE_EXPORT KoPathShapeLoader : public KoSvgPathParser
{
public:
    KoPathShapeLoader( KoPathShape * path );
    ~KoPathShapeLoader();

    /// reimplemented
    virtual void svgMoveTo( double x1, double y1, bool abs = true );
    /// reimplemented
    virtual void svgLineTo( double x1, double y1, bool abs = true );
        /// reimplemented
    virtual void svgLineToHorizontal( double x, bool abs = true );
        /// reimplemented
    virtual void svgLineToVertical( double y, bool abs = true );
        /// reimplemented
    virtual void svgCurveToCubic( double x1, double y1, double x2, double y2, double x, double y, bool abs = true );
    /// reimplemented
    virtual void svgCurveToCubicSmooth( double x, double y, double x2, double y2, bool abs = true );
    /// reimplemented
    virtual void svgCurveToQuadratic( double x, double y, double x1, double y1, bool abs = true );
    /// reimplemented
    virtual void svgCurveToQuadraticSmooth( double x, double y, bool abs = true );
    /// reimplemented
    virtual void svgArcTo( double x, double y, double r1, double r2, double angle, bool largeArcFlag, bool sweepFlag, bool abs = true );
    /// reimplemented
    virtual void svgClosePath();

private:
    class Private;
    Private * const d;
};

#endif // _KOPATHSHAPELOADER_H_
