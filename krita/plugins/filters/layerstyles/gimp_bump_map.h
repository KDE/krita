/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  gimp_bump_map contains code taken from gimp-bumpmap.c, original copyright:
 *
 *   Copyright (C) 1997 Federico Mena Quintero <federico@nuclecu.unam.mx>
 *   Copyright (C) 1997-2000 Jens Lautenbacher <jtl@gimp.org>
 *   Copyright (C) 2000 Sven Neumann <sven@gimp.org>
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __GIMP_BUMP_MAP_H
#define __GIMP_BUMP_MAP_H

#include <QtGlobal>
#include "kis_types.h"



enum BumpmapType
{
  LINEAR = 0,
  SPHERICAL,
  SINUSOIDAL
};


struct bumpmap_vals_t
{
    bumpmap_vals_t()
    : bumpmap_id(0),
      azimuth(0),
      elevation(30),
      depth(50),
      xofs(0),
      yofs(0),
      waterlevel(0),
      ambient(10),
      compensate(true),
      invert(false),
      type(0),
      tiled(false)
    {
    }

    const qint32   bumpmap_id;
    double  azimuth;
    double  elevation;
    int     depth;
    const int     xofs;
    const int     yofs;
    const int     waterlevel;
    int     ambient;
    bool compensate;
    bool invert;
    int     type;
    const bool tiled;
};

void KDE_EXPORT bumpmap (KisPixelSelectionSP device,
                         const QRect &selectionRect,
                         const bumpmap_vals_t &bmvals);

#endif /* __GIMP_BUMP_MAP_H */
