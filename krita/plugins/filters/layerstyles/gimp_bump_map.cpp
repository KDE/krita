/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *   gimp_bump_map contains code taken from gimp-bumpmap.c, original copyright:
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

#include "gimp_bump_map.h"

#include <QRect>
#include "kis_pixel_selection.h"


typedef int gint;
typedef qint32 gint32;
typedef bool gboolean;
typedef quint8 guchar;
typedef double gdouble;

#define G_PI M_PI
#define MOD(x, y) ((x) % (y))
#define CLAMP(x, l, h) qBound(l, x, h)
#define MAX(x,y) qMax(x,y)
#define MIN(x,y) qMin(x,y)

typedef struct
{
  gint    lx, ly;       /* X and Y components of light vector */
  gint    nz2, nzlz;    /* nz^2, nz*lz */
  gint    background;   /* Shade for vertical normals */
  gdouble compensation; /* Background compensation */
  guchar  lut[256];     /* Look-up table for modes */
} bumpmap_params_t;

void bumpmap_init_params (bumpmap_params_t *params, const bumpmap_vals_t &bmvals);

void
bumpmap_row (const bumpmap_vals_t &bmvals,
             const QRect &selectionRect,
             const guchar     *src,
             guchar           *dest,
             gint              width,
             gint              bpp,
             gboolean          has_alpha,
             const guchar     *bm_row1,
             const guchar     *bm_row2,
             const guchar     *bm_row3,
             gint              bm_width,
             gint              bm_xofs,
             gboolean          tiled,
             gboolean          row_in_bumpmap,
             bumpmap_params_t *params);

////////////////////////////////////////////////////////////////////


void bumpmap (KisPixelSelectionSP device,
              const QRect &selectionRect,
              const bumpmap_vals_t &bmvals)
{
    KIS_ASSERT_RECOVER_RETURN(bmvals.xofs == 0);
    KIS_ASSERT_RECOVER_RETURN(bmvals.yofs == 0);

    bumpmap_params_t  params;
    bumpmap_init_params (&params, bmvals);

    const int rowSize = selectionRect.width() * sizeof(quint8);
    quint8 *srcRow = new quint8[rowSize];
    quint8 *dstRow = new quint8[rowSize];

    quint8 *bmRow1 = new quint8[rowSize];
    quint8 *bmRow2 = new quint8[rowSize];
    quint8 *bmRow3 = new quint8[rowSize];

    device->readBytes(bmRow1, selectionRect.left(), selectionRect.top() - 1, selectionRect.width(), 1);
    device->readBytes(bmRow2, selectionRect.left(), selectionRect.top(), selectionRect.width(), 1);
    device->readBytes(bmRow3, selectionRect.left(), selectionRect.top() + 1, selectionRect.width(), 1);

    for (int row = selectionRect.top();
         row < selectionRect.top() + selectionRect.height(); row++) {

        device->readBytes(srcRow, selectionRect.left(), row, selectionRect.width(), 1);

        bumpmap_row (bmvals, selectionRect,
                     srcRow, dstRow,
                     selectionRect.width(),
                     1, false,
                     bmRow1, bmRow2, bmRow3, selectionRect.width(), 0,
                     false /* not tiled */,
                     true /* row in bumpmap */,
                     &params);

        device->writeBytes(dstRow, selectionRect.left(), row, selectionRect.width(), 1);

        quint8 *tmp = bmRow1;
        bmRow1 = bmRow2;
        bmRow2 = bmRow3;
        bmRow3 = tmp;

        device->readBytes(bmRow3, selectionRect.left(), row + 1, selectionRect.width(), 1);
    }
}

void bumpmap_init_params (bumpmap_params_t *params, const bumpmap_vals_t &bmvals)
{
  /* Convert to radians */
  const gdouble azimuth   = G_PI * bmvals.azimuth / 180.0;
  const gdouble elevation = G_PI * bmvals.elevation / 180.0;

  gint lz, nz;
  gint i;

  /* Calculate the light vector */
  params->lx = cos (azimuth) * cos (elevation) * 255.0;
  params->ly = sin (azimuth) * cos (elevation) * 255.0;
  lz         = sin (elevation) * 255.0;

  /* Calculate constant Z component of surface normal */
  /*              (depth may be 0 if non-interactive) */
  nz           = (6 * 255) / qMax (bmvals.depth, 1);
  params->nz2  = nz * nz;
  params->nzlz = nz * lz;

  /* Optimize for vertical normals */
  params->background = lz;

  /* Calculate darkness compensation factor */
  params->compensation = sin(elevation);

  /* Create look-up table for map type */
  for (i = 0; i < 256; i++)
    {
      gdouble n;

      switch (bmvals.type)
        {
        case SPHERICAL:
          n = i / 255.0 - 1.0;
          params->lut[i] = (int) (255.0 * sqrt(1.0 - n * n) + 0.5);
          break;

        case SINUSOIDAL:
          n = i / 255.0;
          params->lut[i] = (int) (255.0 *
                                  (sin((-G_PI / 2.0) + G_PI * n) + 1.0) /
                                  2.0 + 0.5);
          break;

        case LINEAR:
        default:
          params->lut[i] = i;
        }

      if (bmvals.invert)
        params->lut[i] = 255 - params->lut[i];
    }
}

void
bumpmap_row (const bumpmap_vals_t &bmvals,
             const QRect &selectionRect,
             const guchar     *src,
             guchar           *dest,
             gint              width,
             gint              bpp,
             gboolean          has_alpha,
             const guchar     *bm_row1,
             const guchar     *bm_row2,
             const guchar     *bm_row3,
             gint              bm_width,
             gint              bm_xofs,
             gboolean          tiled,
             gboolean          row_in_bumpmap,
             bumpmap_params_t *params)
{
    KIS_ASSERT_RECOVER_RETURN(bm_xofs == 0);

    gint xofs1, xofs2;
    gint x, k;
    gint pbpp;
    gint result;
    gint tmp;

    if (has_alpha)
        pbpp = bpp - 1;
    else
        pbpp = bpp;

    for (x = 0; x < width; x++) {
        gint xofs3;
        gint shade;
        gint nx, ny;

        /* Calculate surface normal from bump map */

        xofs2 = x;
        xofs1 = xofs2 - 1;
        xofs3 = xofs2 + 1;

        nx = (bm_row1[xofs1] + bm_row2[xofs1] + bm_row3[xofs1] -
              bm_row1[xofs3] - bm_row2[xofs3] - bm_row3[xofs3]);
        ny = (bm_row3[xofs1] + bm_row3[xofs2] + bm_row3[xofs3] -
              bm_row1[xofs1] - bm_row1[xofs2] - bm_row1[xofs3]);

        /* Shade */

        if ((nx == 0) && (ny == 0)) {
            shade = params->background;
        } else {
            gint ndotl = nx * params->lx + ny * params->ly + params->nzlz;

            if (ndotl < 0) {
                shade = params->compensation * bmvals.ambient;
            } else {
                shade = ndotl / sqrt (nx * nx + ny * ny + params->nz2);

                shade = shade + MAX(0.0, (255 * params->compensation - shade)) *
                    bmvals.ambient / 255;
            }
        }

        /* Paint */

        if (bmvals.compensate) {
            for (k = pbpp; k; k--) {
                result  = (*src++ * shade) / (params->compensation * 255);
                *dest++ = MIN(255, result);
            }
        } else {
            for (k = pbpp; k; k--)
                *dest++ = *src++ * shade / 255;
        }

        if (has_alpha)
            *dest++ = *src++;
    }
}

