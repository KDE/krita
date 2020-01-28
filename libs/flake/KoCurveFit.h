/* This file is part of the KDE project
   Copyright (C) 2001-2003 Rob Buis <buis@kde.org>
   Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>

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

#ifndef KOCURVEFIT_H
#define KOCURVEFIT_H

#include <QList>
#include <QPointF>

#include "kritaflake_export.h"

class KoPathShape;

/*
 * Fits bezier curve to given list of points.
 *
 * An Algorithm for Automatically Fitting Digitized Curves
 * by Philip J. Schneider
 * from "Graphics Gems", Academic Press, 1990
 *
 * http://web.archive.org/web/20061118130015/http://www.acm.org/pubs/tog/GraphicsGems/gems/FitCurves.c
 * http://web.archive.org/web/20040519052901/http://www.acm.org/pubs/tog/GraphicsGems/gems/README
 *
 * @param points the list of points to fit curve to
 * @param error the max. fitting error
 * @return a path shape representing the fitted curve
 */

KRITAFLAKE_EXPORT KoPathShape * bezierFit(const QList<QPointF> &points, float error);

#endif

