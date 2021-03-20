/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  gimp_bump_map contains code taken from gimp-bumpmap.c, original copyright:
 *
 *  SPDX-FileCopyrightText: 1997 Federico Mena Quintero <federico@nuclecu.unam.mx>
 *  SPDX-FileCopyrightText: 1997-2000 Jens Lautenbacher <jtl@gimp.org>
 *  SPDX-FileCopyrightText: 2000 Sven Neumann <sven@gimp.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

void KRITAIMAGE_EXPORT bumpmap (KisPixelSelectionSP device,
                                const QRect &selectionRect,
                                const bumpmap_vals_t &bmvals);

#endif /* __GIMP_BUMP_MAP_H */
