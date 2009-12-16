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

#include "flake_export.h"

class KoPathShape;
class KoPathShapeLoaderPrivate;
class QString;

/**
 * Parser for svg path data, passed by argument in the parseSvg() method
 * A helper class for parsing path data when loading from svg/odf
 */
class FLAKE_EXPORT KoPathShapeLoader
{
public:
    KoPathShapeLoader(KoPathShape *path);
    ~KoPathShapeLoader();

    /**
     * There are two operating modes. By default the parser just delivers unaltered
     * svg path data commands and parameters. In the second mode, it will convert all
     * relative coordinates to absolute ones, and convert all curves to cubic beziers.
     */
    void parseSvg(const QString &svgInputData, bool process = false);

private:
    KoPathShapeLoaderPrivate* const d;
};

#endif // _KOPATHSHAPELOADER_H_
