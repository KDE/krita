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

#include "kis_convolution_kernel.h"

#include <math.h>

#include <QImage>
#include <kis_mask_generator.h>

struct KisConvolutionKernel::Private {
    qreal offset;
    qreal factor;
    Matrix<qreal, Dynamic, Dynamic> data;
};

KisConvolutionKernel::KisConvolutionKernel(quint32 _width, quint32 _height, qreal _offset, qreal _factor) : d(new Private)
{
    d->offset = _offset;
    d->factor = _factor;
    setSize(_width, _height);
}

KisConvolutionKernel::~KisConvolutionKernel()
{
    delete d;
}

quint32 KisConvolutionKernel::width() const
{
    return d->data.cols();
}

quint32 KisConvolutionKernel::height() const
{
    return d->data.rows();
}

void KisConvolutionKernel::setSize(quint32 width, quint32 height)
{
    d->data.resize(height, width);
}


qreal KisConvolutionKernel::offset() const
{
    return d->offset;
}

qreal KisConvolutionKernel::factor() const
{
    return d->factor;
}

void KisConvolutionKernel::setFactor(qreal factor)
{
    d->factor = factor;
}

Matrix<qreal, Dynamic, Dynamic>& KisConvolutionKernel::data()
{
    return d->data;
}

const Matrix<qreal, Dynamic, Dynamic>* KisConvolutionKernel::data() const
{
    return &(d->data);
}

KisConvolutionKernelSP KisConvolutionKernel::fromQImage(const QImage& image)
{
    KisConvolutionKernelSP kernel = new KisConvolutionKernel(image.width(), image.height(), 0, 0);

    Matrix<qreal, Dynamic, Dynamic>& data = kernel->data();

    const quint8* itImage = image.bits();
    qreal factor = 0;

    for (int r = 0; r < image.height(); r++) {
        for (int c = 0; c < image.width(); c++, itImage += 4)
        {
            uint value = 255 - (*itImage + *(itImage + 1) + *(itImage + 2)) / 3;
            data(r, c) = value;
            factor += value;
        }
    }

    kernel->setFactor(factor);
    return kernel;
}

KisConvolutionKernelSP KisConvolutionKernel::fromMaskGenerator(KisMaskGenerator* kmg, double angle)
{
    Q_UNUSED(angle);

    qint32 width = (int)(kmg->width() + 0.5);
    qint32 height = (int)(kmg->height() + 0.5);

    KisConvolutionKernelSP kernel = new KisConvolutionKernel(width, height, 0, 0);

    double cosa = cos(angle);
    double sina = sin(angle);
    double xc = 0.5 * width - 0.5;
    double yc = 0.5 * height - 0.5;

    Matrix<qreal, Dynamic, Dynamic>& data = kernel->data();
    qreal factor = 0;

//     dbgImage << ppVar(xc) << ppVar(yc);
    for (int r = 0; r < height; ++r) {
        for (int c = 0; c < width; ++c) {
            double x_ = (c - xc);
            double y_ = (r - yc);
            double x = cosa * x_ - sina * y_;
            double y = sina * x_ + cosa * y_;
//             dbgImage << ppVar(x) << ppVar(y) << ppVar(x_) << ppVar(y_) << ppVar( kmg->interpolatedValueAt( x,y) );
            uint value = 255 - kmg->interpolatedValueAt(x, y);
            data(r, c) = value;
            factor += value;
        }
    }
    kernel->setFactor(factor);
    return kernel;
}

KisConvolutionKernelSP KisConvolutionKernel::fromMatrix(Matrix<qreal, Dynamic, Dynamic> matrix, qreal offset, qreal factor)
{
    KisConvolutionKernelSP kernel = new KisConvolutionKernel(matrix.cols(), matrix.rows(), offset, factor);        
    kernel->data() = matrix;

    return kernel;
}




#if 0
double xr = (x /*- m_xcenter*/);
double yr = (y /*- m_ycenter*/);
double n = norme(xr * m_xcoef, yr * m_ycoef);
if (n > 1)
{
    return 255;
} else
{
    double normeFade = norme(xr * m_xfadecoef, yr * m_yfadecoef);
    if (normeFade > 1) {
        double xle, yle;
        // xle stands for x-coordinate limit exterior
        // yle stands for y-coordinate limit exterior
        // we are computing the coordinate on the external ellipse in order to compute
        // the fade value
        if (xr == 0) {
            xle = 0;
            yle = yr > 0 ? 1 / m_ycoef : -1 / m_ycoef;
        } else {
            double c = yr / (double)xr;
            xle = sqrt(1 / norme(m_xcoef, c * m_ycoef));
            xle = xr > 0 ? xle : -xle;
            yle = xle * c;
        }
        // On the internal limit of the fade area, normeFade is equal to 1
        double normeFadeLimitE = norme(xle * m_xfadecoef, yle * m_yfadecoef);
        return (uchar)(255 *(normeFade - 1) / (normeFadeLimitE - 1));
    } else {
        return 0;
    }
}
#endif

#include "kis_debug.h"

QDebug operator<<(QDebug debug, const KisConvolutionKernel &c)
{
    debug.nospace() << "[" << c.width() << "," << c.height() << "]{";
    for (unsigned int i = 0; i < c.width(); ++i) {
        debug.nospace() << " {";
        for (unsigned int j = 0; j < c.height(); ++j) {
            debug.nospace() << (*(c.data()))(j, i) << " ";
        }
        debug.nospace() << " }";
    }
    debug.nospace() << c.factor() << " " << c.offset() <<  " }";
    return debug.space();
}
