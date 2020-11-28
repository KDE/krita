/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef _KOPATHSHAPELOADER_H_
#define _KOPATHSHAPELOADER_H_

#include "kritaflake_export.h"

class KoPathShape;
class KoPathShapeLoaderPrivate;
class QString;

/**
 * Parser for svg path data, passed by argument in the parseSvg() method
 * A helper class for parsing path data when loading from svg/odf
 */
class KRITAFLAKE_EXPORT KoPathShapeLoader
{
public:
    explicit KoPathShapeLoader(KoPathShape *path);
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
