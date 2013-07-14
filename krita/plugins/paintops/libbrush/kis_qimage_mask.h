/*
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2010 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#include <KoColorSpace.h>

#include "kis_global.h"
#include "kis_types.h"
#include <kis_shared.h>

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
    KisQImagemask(qint32 width, qint32 height, bool initialize = true);

    virtual ~KisQImagemask();


    
    /**
     * @return the number of lines in the mask.
     */
    qint32 height() const {
        return m_height;
    }

    /**
     * @return the number of alpha values in a scanline.
     */
    qint32 width() const{
        return m_width;
    }

    /**
     * Return the alpha mask bytes
     */
    inline quint8 * scanline(int y) {
        return m_data.scanLine(y);
    }

    /**
       @return the alpha value at the specified position.

       Returns quint8 OPACITY_TRANSPARENT if the value is
       outside the bounds of the mask.
       
       Now it is much faster, we cache the pointer to data
       in storage (QImage) instead of calling scanlines which
       was slow

       XXX: this is, of course, not the best way of masking.
       Better would be to let KisQImagemask fill a chunk of memory
       with the alpha values at the right position, something like
       void applyMask(quint8 *pixeldata, qint32 pixelWidth,
       qint32 alphaPos). That would be fastest, or we could
       provide an iterator over the mask, that would be nice, too.
     */
    inline quint8 alphaAt(qint32 x, qint32 y) const {
        if (y >= 0 && y < m_height && x >= 0 && x < m_width) {
            return m_dataPtr[(y * m_bytesPerLine + x)];
        } else {
            return OPACITY_TRANSPARENT_U8;
        }
    }

    inline void setAlphaAt(qint32 x, qint32 y, quint8 alpha) {
        if (y >= 0 && y < m_height && x >= 0 && x < m_width) {
            m_dataPtr[(y * m_bytesPerLine + x)] = alpha;
        }
    }

    /**
     * Apply rotation to the mask
     */
    void rotation(double angle);


private:
    /// init the internal storage (QImage)
    void init(int width, int height);
    void computeAlphaMaskFromGrayScale(const QImage& image);
    void computeAlphaMaskFromRGBA(const QImage& image);

private:    
    int m_width;
    int m_height;
    QImage m_data;
    
    int m_bytesPerLine;
    uchar * m_dataPtr;

};

#endif // KIS_ALPHA_MASK_

