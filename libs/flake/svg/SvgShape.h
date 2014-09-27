/* This file is part of the KDE project
 * Copyright (C) 2011 Jan Hambrecht <jaham@gmx.net>
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

#ifndef SVGSHAPE_H
#define SVGSHAPE_H

#include "flake_export.h"

class SvgSavingContext;
class SvgLoadingContext;
class KoXmlElement;

/// An interface providing svg loading and saving routines
class FLAKE_EXPORT SvgShape
{
public:
    virtual ~SvgShape();

    /// Saves data utilizing specified svg saving context
    virtual bool saveSvg(SvgSavingContext &context);

    /// Loads data from specified svg element
    virtual bool loadSvg(const KoXmlElement &element, SvgLoadingContext &context);
};

#endif // SVGSHAPE_H
