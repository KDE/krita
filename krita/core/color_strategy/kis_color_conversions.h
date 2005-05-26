/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _KIS_CONVERSIONS_H_
#define _KIS_CONVERSIONS_H_

#include <qglobal.h>
#include <kdebug.h>
#include "koffice_export.h"

/**
 * A number of often-used conversions between color models
 */
KRITA_EXPORT void rgb_to_hsv(int R, int G, int B, int *H, int *S, int *V);

KRITA_EXPORT void hsv_to_rgb(int H, int S, int V, int *R, int *G, int *B);

KRITA_EXPORT void rgb_to_hls(Q_UINT8 r, Q_UINT8 g, Q_UINT8 b, float * h, float * l, float * s);

KRITA_EXPORT float hue_value(float n1, float n2, float hue);

KRITA_EXPORT void hls_to_rgb(float h, float l, float s, Q_UINT8 * r, Q_UINT8 * g, Q_UINT8 * b);

KRITA_EXPORT void rgb_to_hls(Q_UINT8 r, Q_UINT8 g, Q_UINT8 b, int * h, int * l, int * s);
KRITA_EXPORT void hls_to_rgb(int h, int l, int s, Q_UINT8 * r, Q_UINT8 * g, Q_UINT8 * b);

#endif // _KIS_CONVERSIONS_H_

