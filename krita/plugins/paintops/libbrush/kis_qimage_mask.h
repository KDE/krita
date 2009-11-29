/*
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_QIMAGE_MASK_H_
#define KIS_QIMAGE_MASK_H_

#include "kis_gbr_brush.h"

#include <QImage>
#include <QVector>

#include <kis_shared.h>

#include <KoColorSpace.h>

#include "kis_global.h"
#include "kis_types.h"

/**
 * KisQImagemask is intended to create alpha values from a QImage for use
 * in brush creation. It is not a generic alpha mask that can be used with
 * KisPaintDevices: use a KisSelection for that.
 */
class KisQImagemask : public KisShared
{

public:
    /**
       Create an alpha mask based on the specified QImage. If the image is
       not a grayscale, the mask value is calculated from the effective grey
       level and alpha value.
    */
    KisQImagemask(const QImage& image);

    /**
       As above except quicker as the image does not need to be scanned
       to see if it has any color pixels.
    */
    KisQImagemask(const QImage& image, bool hasColor);

    /**
       Create a transparent mask.
    */
    KisQImagemask(qint32 width, qint32 height);

    virtual ~KisQImagemask();

    /**
       @return the number of alpha values in a scanline.
    */
    qint32 height() const;

    /**
       @return the number of lines in the mask.
     */
    qint32 width() const;

    /**
     * Return the alpha mask bytes
     */
    inline quint8 * data() {
        return m_data.data();
    }

    /**
       @return the alpha value at the specified position.

       Returns quint8 OPACITY_TRANSPARENT if the value is
       outside the bounds of the mask.

       XXX: this is, of course, not the best way of masking.
       Better would be to let KisQImagemask fill a chunk of memory
       with the alpha values at the right position, something like
       void applyMask(quint8 *pixeldata, qint32 pixelWidth,
       qint32 alphaPos). That would be fastest, or we could
       provide an iterator over the mask, that would be nice, too.
    */
    inline quint8 alphaAt(qint32 x, qint32 y) const {
        if (y >= 0 && y < m_height && x >= 0 && x < m_width) {
            return m_data[(y * m_width) + x];
        } else {
            return OPACITY_TRANSPARENT;
        }
    }

    inline void setAlphaAt(qint32 x, qint32 y, quint8 alpha) {
        if (y >= 0 && y < m_height && x >= 0 && x < m_width) {
            m_data[(y * m_width) + x] = alpha;
        }
    }


    // Create a new mask by interpolating between mask1 and mask2 as t
    // goes from 0 to 1.
    static KisQImagemaskSP interpolate(KisQImagemaskSP mask1, KisQImagemaskSP mask2, double t);

private:
    void computeAlpha(const QImage& image);
    void copyAlpha(const QImage& image);

    QVector<quint8> m_data;
    qint32 m_width;
    qint32 m_height;
};

#endif // KIS_ALPHA_MASK_

