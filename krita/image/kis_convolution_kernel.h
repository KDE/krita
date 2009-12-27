/*
 *  Copyright (c) 2005,2008 Cyrille Berger <cberger@cberger.net>
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

#ifndef _KIS_CONVOLUTION_KERNEL_H_
#define _KIS_CONVOLUTION_KERNEL_H_

#include <Eigen/Core>
#include "kis_shared.h"
#include "krita_export.h"
#include "kis_types.h"

using namespace Eigen;

class KisMaskGenerator;
class QImage;

class KRITAIMAGE_EXPORT KisConvolutionKernel : public KisShared
{

public:
    KisConvolutionKernel(quint32 width, quint32 height, qreal offset, qreal factor);
    virtual ~KisConvolutionKernel();

    quint32 width() const;
    quint32 height() const;
    /**
     * Change the size of a kernel, it won't reallocate, and therefore it must keep the same kernel size ( oldwidth * oldheight = newwidth*newheight)
     */
    void setSize(quint32 width, quint32 height);
    qreal offset() const;
    qreal factor() const;
    void setFactor(qreal);
    Matrix<qreal, Dynamic, Dynamic>& data();
    const Matrix<qreal, Dynamic, Dynamic> * data() const;

    static KisConvolutionKernelSP fromQImage(const QImage& image);
    static KisConvolutionKernelSP fromMaskGenerator(KisMaskGenerator *, double angle = 0.0);
    static KisConvolutionKernelSP fromMatrix(Matrix<qreal, Dynamic, Dynamic> matrix, qreal offset, qreal factor);
private:
    struct Private;
    Private* const d;

};

class QDebug;

QDebug operator<<(QDebug debug, const KisConvolutionKernel &c);
#endif
