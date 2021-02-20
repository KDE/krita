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

#include "gimp_bump_map.h"

#include <QRect>
#include "kis_pixel_selection.h"
#include "kis_image.h"


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
             guchar           *dest,
             gint              width,
             const guchar     *bm_row1,
             const guchar     *bm_row2,
             const guchar     *bm_row3,
             bumpmap_params_t *params);

////////////////////////////////////////////////////////////////////

void convertRow(quint8 *data, int width, const quint8 *lut)
{
    for (int i = 0; i < width; i++) {
        *data = lut[*data];
        data++;
    }
}

void bumpmap (KisPixelSelectionSP device,
              const QRect &selectionRect,
              const bumpmap_vals_t &bmvals)
{
    KIS_ASSERT_RECOVER_RETURN(bmvals.xofs == 0);
    KIS_ASSERT_RECOVER_RETURN(bmvals.yofs == 0);

    bumpmap_params_t  params;
    bumpmap_init_params (&params, bmvals);

    const QRect dataRect = kisGrowRect(selectionRect, 1);

    const int dataRowSize = dataRect.width() * sizeof(quint8);
    const int selectionRowSize = selectionRect.width() * sizeof(quint8);
    QScopedArrayPointer<quint8> dstRow(new quint8[selectionRowSize]);

    QScopedArrayPointer<quint8> bmRow1(new quint8[dataRowSize]);
    QScopedArrayPointer<quint8> bmRow2(new quint8[dataRowSize]);
    QScopedArrayPointer<quint8> bmRow3(new quint8[dataRowSize]);

    device->readBytes(bmRow1.data(), dataRect.left(), dataRect.top(), dataRect.width(), 1);
    device->readBytes(bmRow2.data(), dataRect.left(), dataRect.top() + 1, dataRect.width(), 1);
    device->readBytes(bmRow3.data(), dataRect.left(), dataRect.top() + 2, dataRect.width(), 1);

    convertRow(bmRow1.data(), dataRect.width(), params.lut);
    convertRow(bmRow2.data(), dataRect.width(), params.lut);
    convertRow(bmRow3.data(), dataRect.width(), params.lut);

    for (int row = selectionRect.top();
         row < selectionRect.top() + selectionRect.height(); row++) {

        bumpmap_row (bmvals, dstRow.data(), selectionRect.width(),
                     bmRow1.data() + 1, bmRow2.data() + 1, bmRow3.data() + 1,
                     &params);

        device->writeBytes(dstRow.data(), selectionRect.left(), row, selectionRect.width(), 1);

        bmRow1.swap(bmRow2);
        bmRow2.swap(bmRow3);

        device->readBytes(bmRow3.data(), dataRect.left(), row + 1, dataRect.width(), 1);
        convertRow(bmRow3.data(), dataRect.width(), params.lut);
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
             guchar           *dest,
             gint              width,
             const guchar     *bm_row1,
             const guchar     *bm_row2,
             const guchar     *bm_row3,
             bumpmap_params_t *params)
{
    gint xofs1, xofs2;
    gint x;

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
            int result  = shade / params->compensation;
            *dest++ = MIN(255, result);
        } else {
            *dest++ = shade;
        }
    }
}

