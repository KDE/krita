/*
 *  SPDX-FileCopyrightText: 2005, 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_CONVOLUTION_KERNEL_H_
#define _KIS_CONVOLUTION_KERNEL_H_

#include <cstddef>
#include <Eigen/Core>
#include "kis_shared.h"
#include "kritaimage_export.h"
#include "kis_types.h"

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
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic>& data();
    const Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> * data() const;

    static KisConvolutionKernelSP fromQImage(const QImage& image);
    static KisConvolutionKernelSP fromMaskGenerator(KisMaskGenerator *, qreal angle = 0.0);
    static KisConvolutionKernelSP fromMatrix(Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> matrix, qreal offset, qreal factor);
private:
    struct Private;
    Private* const d;

};

class QDebug;

QDebug operator<<(QDebug debug, const KisConvolutionKernel &c);
#endif
