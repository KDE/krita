/*
 *  Copyright (c) 2005 Cyrille Berger <cberger@cberger.net>
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
#ifndef KIS_CONVOLUTION_PAINTER_H_
#define KIS_CONVOLUTION_PAINTER_H_

#include <qimage.h>

#include "ksharedptr.h"
#include "kis_types.h"
#include "kis_painter.h"

#include "krita_export.h"

enum KisConvolutionBorderOp {
    BORDER_DEFAULT_FILL = 0, // Use the default pixel to make up for the missing pixels on the border or the pixel that lies beyond
                             // the rect we are convolving.
    BORDER_WRAP = 1, // Use the pixel on the opposite side to make up for the missing pixels on the border. XXX: Not implemented yet
    BORDER_REPEAT = 2, // Use the border for the missing pixels, too.
    BORDER_AVOID = 3 // Skip convolving the border pixels at all.
};

class KisKernel;
typedef KSharedPtr<KisKernel> KisKernelSP;

class KisKernel : public KShared
{

public:

    quint32 width;
    quint32 height;
    qint32 offset;
    qint32 factor;
    qint32 * data;

    KisKernel() : width(0), height(0), offset(0), factor(0), data(0) {};

    virtual ~KisKernel() { delete [] data; };

    static KisKernelSP fromQImage(const QImage& img);

};


class KRITAIMAGE_EXPORT KisConvolutionPainter : public KisPainter
{

    typedef KisPainter super;

public:

    KisConvolutionPainter();
    KisConvolutionPainter(KisPaintDeviceSP device);

    /**
     * Convolve all channels in src using the specified kernel; there is only one kernel for all
     * channels possible. By default the the border pixels are not convolved, that is, convolving
     * starts with at (x + kernel.width/2, y + kernel.height/2) and stops at w - (kernel.width/2)
     * and h - (kernel.height/2)
     *
     * The border op decides what to do with pixels too close to the edge of the rect as defined above.
     *
     * The channels flag determines which set out of color channels, alpha channels, substance or substrate
     * channels we convolve.
     *
     * Note that we do not (currently) support different kernels for different channels _or_ channel types.
     */
    void applyMatrix(KisKernelSP kernel, qint32 x, qint32 y, qint32 w, qint32 h,
                     KisConvolutionBorderOp borderOp = BORDER_AVOID,
                     KisChannelInfo::enumChannelFlags channelFlags = KisChannelInfo::FLAG_COLOR);
private:
    /**
     * This function is called by applyMatrix when borderOp == BORDER_REPEAT
     */
    void applyMatrixRepeat(KisKernelSP kernel, qint32 x, qint32 y, qint32 w, qint32 h,
                           KisChannelInfo::enumChannelFlags channelFlags);


};
#endif //KIS_CONVOLUTION_PAINTER_H_
