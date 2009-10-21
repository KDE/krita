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
    quint32 width;
    quint32 height;
    qint32 offset;
    qint32 factor;
    qint32 * data;
};

KisConvolutionKernel::KisConvolutionKernel(quint32 _width, quint32 _height, qint32 _offset, qint32 _factor) : d(new Private)
{
    d->width = _width;
    d->height = _height;
    d->offset = _offset;
    d->factor = _factor;
    d->data = new qint32[d->width * d->height];
}

KisConvolutionKernel::~KisConvolutionKernel()
{
    delete[] d->data;
    delete d;
}

quint32 KisConvolutionKernel::width() const
{
    return d->width;
}

quint32 KisConvolutionKernel::height() const
{
    return d->height;
}

void KisConvolutionKernel::setSize(quint32 width, quint32 height)
{
    Q_ASSERT(d->width * d->height == width * height);
    d->width = width;
    d->height = height;
}


qint32 KisConvolutionKernel::offset() const
{
    return d->offset;
}

qint32 KisConvolutionKernel::factor() const
{
    return d->factor;
}

void KisConvolutionKernel::setFactor(qint32 factor)
{
    d->factor = factor;
}

qint32* KisConvolutionKernel::data()
{
    return d->data;
}

const qint32* KisConvolutionKernel::data() const
{
    return d->data;
}

KisConvolutionKernelSP KisConvolutionKernel::fromQImage(const QImage& img)
{
    KisConvolutionKernelSP k = new KisConvolutionKernel(img.width(), img.height(), 0, 0);
    uint count = k->width() * k->height();
    qint32* itData = k->data();
    const quint8* itImg = img.bits();
    qint32 factor = 0;
    for (uint i = 0; i < count; ++i , ++itData, itImg += 4) {
        *itData = 255 - (*itImg + *(itImg + 1) + *(itImg + 2)) / 3;
        factor += *itData;
    }
    k->d->factor = factor;
    return k;
}

KisConvolutionKernelSP KisConvolutionKernel::kernelFromMaskGenerator(KisMaskGenerator* kmg, double angle)
{
    Q_UNUSED(angle);

    qint32 width = (int)(kmg->width() + 0.5);
    qint32 height = (int)(kmg->height() + 0.5);

    KisConvolutionKernelSP k = new KisConvolutionKernel(width, height, 0, 0);
    double cosa = cos(angle);
    double sina = sin(angle);
    double xc = 0.5 * width - 0.5;
    double yc = 0.5 * height - 0.5;
    qint32 factor = 0;
    qint32* itData = k->data();
//     dbgImage << ppVar(xc) << ppVar(yc);
    for (int y_it = 0; y_it < height; ++y_it) {
        for (int x_it = 0; x_it < width; ++x_it) {
            double x_ = (x_it - xc);
            double y_ = (y_it - yc);
            double x = cosa * x_ - sina * y_;
            double y = sina * x_ + cosa * y_;
//             dbgImage << ppVar(x) << ppVar(y) << ppVar(x_) << ppVar(y_) << ppVar( kmg->interpolatedValueAt( x,y) );
            *itData = 255 - kmg->interpolatedValueAt(x, y);
            factor += *itData;
            ++itData;
        }
    }
    k->d->factor = factor;
    return k;
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
    int pos = 0;
    debug.nospace() << "[" << c.width() << "," << c.height() << "]{";
    for (unsigned int i = 0; i < c.width(); ++i) {
        debug.nospace() << " {";
        for (unsigned int j = 0; j < c.height(); ++j, ++pos) {
            debug.nospace() << c.data()[pos] << " ";
        }
        debug.nospace() << " }";
    }
    debug.nospace() << c.factor() << " " << c.offset() <<  " }";
    return debug.space();
}
