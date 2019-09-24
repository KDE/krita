/*
 *  Copyright (c) 2005 Michael Thaler
 *  Copyright (c) 2011 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_selection_filters.h"

#include <klocalizedstring.h>

#include <KoColorSpace.h>
#include "kis_convolution_painter.h"
#include "kis_convolution_kernel.h"
#include "kis_pixel_selection.h"

#define MAX(a, b) ((a) > (b) ? (a) : (b))
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define RINT(x) floor ((x) + 0.5)

KisSelectionFilter::~KisSelectionFilter()
{
}

KUndo2MagicString KisSelectionFilter::name()
{
    return KUndo2MagicString();
}

QRect KisSelectionFilter::changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(defaultBounds);
    return rect;
}

void KisSelectionFilter::computeBorder(qint32* circ, qint32 xradius, qint32 yradius)
{
    qint32 i;
    qint32 diameter = xradius * 2 + 1;
    double tmp;

    for (i = 0; i < diameter; i++) {
        if (i > xradius)
            tmp = (i - xradius) - 0.5;
        else if (i < xradius)
            tmp = (xradius - i) - 0.5;
        else
            tmp = 0.0;

        circ[i] = (qint32) RINT(yradius / (double) xradius * sqrt(xradius * xradius - tmp * tmp));
    }
}

void KisSelectionFilter::rotatePointers(quint8** p, quint32 n)
{
    quint32 i;
    quint8  *p0 = p[0];
    for (i = 0; i < n - 1; i++) {
        p[i] = p[i + 1];
    }
    p[i] = p0;
}

void KisSelectionFilter::computeTransition(quint8* transition, quint8** buf, qint32 width)
{
    qint32 x = 0;

    if (width == 1) {
        if (buf[1][x] > 127 && (buf[0][x] < 128 || buf[2][x] < 128))
            transition[x] = 255;
        else
            transition[x] = 0;
        return;
    }
    if (buf[1][x] > 127) {
        if (buf[0][x] < 128 || buf[0][x + 1] < 128 ||
            buf[1][x + 1] < 128 ||
            buf[2][x] < 128 || buf[2][x + 1] < 128)
            transition[x] = 255;
        else
            transition[x] = 0;
    } else
        transition[x] = 0;
    for (qint32 x = 1; x < width - 1; x++) {
        if (buf[1][x] >= 128) {
            if (buf[0][x - 1] < 128 || buf[0][x] < 128 || buf[0][x + 1] < 128 ||
                buf[1][x - 1] < 128           ||          buf[1][x + 1] < 128 ||
                buf[2][x - 1] < 128 || buf[2][x] < 128 || buf[2][x + 1] < 128)
                transition[x] = 255;
            else
                transition[x] = 0;
        } else
            transition[x] = 0;
    }
    if (buf[1][x] >= 128) {
        if (buf[0][x - 1] < 128 || buf[0][x] < 128 ||
            buf[1][x - 1] < 128 ||
            buf[2][x - 1] < 128 || buf[2][x] < 128)
            transition[x] = 255;
        else
            transition[x] = 0;
    } else
        transition[x] = 0;
}


KUndo2MagicString KisErodeSelectionFilter::name()
{
    return kundo2_i18n("Erode Selection");
}

QRect KisErodeSelectionFilter::changeRect(const QRect& rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(defaultBounds);

    const qint32 radius = 1;
    return rect.adjusted(-radius, -radius, radius, radius);
}

void KisErodeSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect& rect)
{
    // Erode (radius 1 pixel) a mask (1bpp)
    quint8* buf[3];

    qint32 width = rect.width();
    qint32 height = rect.height();

    quint8* out = new quint8[width];
    for (qint32 i = 0; i < 3; i++)
        buf[i] = new quint8[width + 2];


    // load top of image
    pixelSelection->readBytes(buf[0] + 1, rect.x(), rect.y(), width, 1);

    buf[0][0]         = buf[0][1];
    buf[0][width + 1] = buf[0][width];

    memcpy(buf[1], buf[0], width + 2);

    for (qint32 y = 0; y < height; y++) {
        if (y + 1 < height) {
            pixelSelection->readBytes(buf[2] + 1, rect.x(), rect.y() + y + 1, width, 1);

            buf[2][0]         = buf[2][1];
            buf[2][width + 1] = buf[2][width];
        } else {
            memcpy(buf[2], buf[1], width + 2);
        }

        for (qint32 x = 0 ; x < width; x++) {
            qint32 min = 255;

            if (buf[0][x+1] < min) min = buf[0][x+1];
            if (buf[1][x]   < min) min = buf[1][x];
            if (buf[1][x+1] < min) min = buf[1][x+1];
            if (buf[1][x+2] < min) min = buf[1][x+2];
            if (buf[2][x+1] < min) min = buf[2][x+1];

            out[x] = min;
        }

        pixelSelection->writeBytes(out, rect.x(), rect.y() + y, width, 1);
        rotatePointers(buf, 3);
    }

    for (qint32 i = 0; i < 3; i++)
        delete[] buf[i];
    delete[] out;
}


KUndo2MagicString KisDilateSelectionFilter::name()
{
    return kundo2_i18n("Dilate Selection");
}

QRect KisDilateSelectionFilter::changeRect(const QRect& rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(defaultBounds);

    const qint32 radius = 1;
    return rect.adjusted(-radius, -radius, radius, radius);
}

void KisDilateSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect& rect)
 {
    // dilate (radius 1 pixel) a mask (1bpp)
    quint8* buf[3];

    qint32 width = rect.width();
    qint32 height = rect.height();

    quint8* out = new quint8[width];
    for (qint32 i = 0; i < 3; i++)
        buf[i] = new quint8[width + 2];


    // load top of image
    pixelSelection->readBytes(buf[0] + 1, rect.x(), rect.y(), width, 1);

    buf[0][0]         = buf[0][1];
    buf[0][width + 1] = buf[0][width];

    memcpy(buf[1], buf[0], width + 2);

    for (qint32 y = 0; y < height; y++) {
        if (y + 1 < height) {
            pixelSelection->readBytes(buf[2] + 1, rect.x(), rect.y() + y + 1, width, 1);

            buf[2][0]         = buf[2][1];
            buf[2][width + 1] = buf[2][width];
        } else {
            memcpy(buf[2], buf[1], width + 2);
        }

        for (qint32 x = 0 ; x < width; x++) {
            qint32 max = 0;

            if (buf[0][x+1] > max) max = buf[0][x+1];
            if (buf[1][x]   > max) max = buf[1][x];
            if (buf[1][x+1] > max) max = buf[1][x+1];
            if (buf[1][x+2] > max) max = buf[1][x+2];
            if (buf[2][x+1] > max) max = buf[2][x+1];

            out[x] = max;
        }

        pixelSelection->writeBytes(out, rect.x(), rect.y() + y, width, 1);
        rotatePointers(buf, 3);
    }

    for (qint32 i = 0; i < 3; i++)
        delete[] buf[i];
    delete[] out;
}


KisBorderSelectionFilter::KisBorderSelectionFilter(qint32 xRadius, qint32 yRadius)
  : m_xRadius(xRadius),
    m_yRadius(yRadius)
{
}

KUndo2MagicString KisBorderSelectionFilter::name()
{
    return kundo2_i18n("Border Selection");
}

QRect KisBorderSelectionFilter::changeRect(const QRect& rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(defaultBounds);

    return rect.adjusted(-m_xRadius, -m_yRadius, m_xRadius, m_yRadius);
}

void KisBorderSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect& rect)
{
    if (m_xRadius <= 0 || m_yRadius <= 0) return;

    quint8  *buf[3];
    quint8 **density;
    quint8 **transition;

    if (m_xRadius == 1 && m_yRadius == 1) {
        // optimize this case specifically
        quint8* source[3];

        for (qint32 i = 0; i < 3; i++)
            source[i] = new quint8[rect.width()];

        quint8* transition = new quint8[rect.width()];

        pixelSelection->readBytes(source[0], rect.x(), rect.y(), rect.width(), 1);
        memcpy(source[1], source[0], rect.width());
        if (rect.height() > 1)
            pixelSelection->readBytes(source[2], rect.x(), rect.y() + 1, rect.width(), 1);
        else
            memcpy(source[2], source[1], rect.width());

        computeTransition(transition, source, rect.width());
        pixelSelection->writeBytes(transition, rect.x(), rect.y(), rect.width(), 1);

        for (qint32 y = 1; y < rect.height(); y++) {
            rotatePointers(source, 3);
            if (y + 1 < rect.height())
                pixelSelection->readBytes(source[2], rect.x(), rect.y() + y + 1, rect.width(), 1);
            else
                memcpy(source[2], source[1], rect.width());
            computeTransition(transition, source, rect.width());
            pixelSelection->writeBytes(transition, rect.x(), rect.y() + y, rect.width(), 1);
        }

        for (qint32 i = 0; i < 3; i++)
            delete[] source[i];
        delete[] transition;
        return;
    }

    qint32* max = new qint32[rect.width() + 2 * m_xRadius];
    for (qint32 i = 0; i < (rect.width() + 2 * m_xRadius); i++)
        max[i] = m_yRadius + 2;
    max += m_xRadius;

    for (qint32 i = 0; i < 3; i++)
        buf[i] = new quint8[rect.width()];

    transition = new quint8*[m_yRadius + 1];
    for (qint32 i = 0; i < m_yRadius + 1; i++) {
        transition[i] = new quint8[rect.width() + 2 * m_xRadius];
        memset(transition[i], 0, rect.width() + 2 * m_xRadius);
        transition[i] += m_xRadius;
    }
    quint8* out = new quint8[rect.width()];
    density = new quint8*[2 * m_xRadius + 1];
    density += m_xRadius;

    for (qint32 x = 0; x < (m_xRadius + 1); x++) { // allocate density[][]
        density[ x]  = new quint8[2 * m_yRadius + 1];
        density[ x] += m_yRadius;
        density[-x]  = density[x];
    }
    for (qint32 x = 0; x < (m_xRadius + 1); x++) { // compute density[][]
        double tmpx, tmpy, dist;
        quint8 a;

        tmpx = x > 0.0 ? x - 0.5 : 0.0;

        for (qint32 y = 0; y < (m_yRadius + 1); y++) {
            tmpy = y > 0.0 ? y - 0.5 : 0.0;

            dist = ((tmpy * tmpy) / (m_yRadius * m_yRadius) +
                    (tmpx * tmpx) / (m_xRadius * m_xRadius));
            if (dist < 1.0)
                a = (quint8)(255 * (1.0 - sqrt(dist)));
            else
                a = 0;
            density[ x][ y] = a;
            density[ x][-y] = a;
            density[-x][ y] = a;
            density[-x][-y] = a;
        }
    }
    pixelSelection->readBytes(buf[0], rect.x(), rect.y(), rect.width(), 1);
    memcpy(buf[1], buf[0], rect.width());
    if (rect.height() > 1)
        pixelSelection->readBytes(buf[2], rect.x(), rect.y() + 1, rect.width(), 1);
    else
        memcpy(buf[2], buf[1], rect.width());
    computeTransition(transition[1], buf, rect.width());

    for (qint32 y = 1; y < m_yRadius && y + 1 < rect.height(); y++) { // set up top of image
        rotatePointers(buf, 3);
        pixelSelection->readBytes(buf[2], rect.x(), rect.y() + y + 1, rect.width(), 1);
        computeTransition(transition[y + 1], buf, rect.width());
    }
    for (qint32 x = 0; x < rect.width(); x++) { // set up max[] for top of image
        max[x] = -(m_yRadius + 7);
        for (qint32 j = 1; j < m_yRadius + 1; j++)
            if (transition[j][x]) {
                max[x] = j;
                break;
            }
    }
    for (qint32 y = 0; y < rect.height(); y++) { // main calculation loop
        rotatePointers(buf, 3);
        rotatePointers(transition, m_yRadius + 1);
        if (y < rect.height() - (m_yRadius + 1)) {
            pixelSelection->readBytes(buf[2], rect.x(), rect.y() + y + m_yRadius + 1, rect.width(), 1);
            computeTransition(transition[m_yRadius], buf, rect.width());
        } else
            memcpy(transition[m_yRadius], transition[m_yRadius - 1], rect.width());

        for (qint32 x = 0; x < rect.width(); x++) { // update max array
            if (max[x] < 1) {
                if (max[x] <= -m_yRadius) {
                    if (transition[m_yRadius][x])
                        max[x] = m_yRadius;
                    else
                        max[x]--;
                } else if (transition[-max[x]][x])
                    max[x] = -max[x];
                else if (transition[-max[x] + 1][x])
                    max[x] = -max[x] + 1;
                else
                    max[x]--;
            } else
                max[x]--;
            if (max[x] < -m_yRadius - 1)
                max[x] = -m_yRadius - 1;
        }
        quint8 last_max =  max[0][density[-1]];
        qint32 last_index = 1;
        for (qint32 x = 0 ; x < rect.width(); x++) { // render scan line
            last_index--;
            if (last_index >= 0) {
                last_max = 0;
                for (qint32 i = m_xRadius; i >= 0; i--)
                    if (max[x + i] <= m_yRadius && max[x + i] >= -m_yRadius && density[i][max[x+i]] > last_max) {
                        last_max = density[i][max[x + i]];
                        last_index = i;
                    }
                out[x] = last_max;
            } else {
                last_max = 0;
                for (qint32 i = m_xRadius; i >= -m_xRadius; i--)
                    if (max[x + i] <= m_yRadius && max[x + i] >= -m_yRadius && density[i][max[x + i]] > last_max) {
                        last_max = density[i][max[x + i]];
                        last_index = i;
                    }
                out[x] = last_max;
            }
            if (last_max == 0) {
                qint32 i;
                for (i = x + 1; i < rect.width(); i++) {
                    if (max[i] >= -m_yRadius)
                        break;
                }
                if (i - x > m_xRadius) {
                    for (; x < i - m_xRadius; x++)
                        out[x] = 0;
                    x--;
                }
                last_index = m_xRadius;
            }
        }
        pixelSelection->writeBytes(out, rect.x(), rect.y() + y, rect.width(), 1);
    }
    delete [] out;

    for (qint32 i = 0; i < 3; i++)
        delete[] buf[i];

    max -= m_xRadius;
    delete[] max;

    for (qint32 i = 0; i < m_yRadius + 1; i++) {
        transition[i] -= m_xRadius;
        delete transition[i];
    }
    delete[] transition;

    for (qint32 i = 0; i < m_xRadius + 1 ; i++) {
        density[i] -= m_yRadius;
        delete density[i];
    }
    density -= m_xRadius;
    delete[] density;
}


KisFeatherSelectionFilter::KisFeatherSelectionFilter(qint32 radius)
    : m_radius(radius)
{
}

KUndo2MagicString KisFeatherSelectionFilter::name()
{
    return kundo2_i18n("Feather Selection");
}

QRect KisFeatherSelectionFilter::changeRect(const QRect& rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(defaultBounds);

    return rect.adjusted(-m_radius, -m_radius,
                         m_radius, m_radius);
}

void KisFeatherSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect& rect)
{
    // compute horizontal kernel
    const uint kernelSize = m_radius * 2 + 1;
    Eigen::Matrix<qreal, Eigen::Dynamic, Eigen::Dynamic> gaussianMatrix(1, kernelSize);

    const qreal multiplicand = 1.0 / (2.0 * M_PI * m_radius * m_radius);
    const qreal exponentMultiplicand = 1.0 / (2.0 * m_radius * m_radius);

    for (uint x = 0; x < kernelSize; x++) {
        uint xDistance = qAbs((int)m_radius - (int)x);
        gaussianMatrix(0, x) = multiplicand * exp( -(qreal)((xDistance * xDistance) + (m_radius * m_radius)) * exponentMultiplicand );
    }

    KisConvolutionKernelSP kernelHoriz = KisConvolutionKernel::fromMatrix(gaussianMatrix, 0, gaussianMatrix.sum());
    KisConvolutionKernelSP kernelVertical = KisConvolutionKernel::fromMatrix(gaussianMatrix.transpose(), 0, gaussianMatrix.sum());

    KisPaintDeviceSP interm = new KisPaintDevice(pixelSelection->colorSpace());
    interm->prepareClone(pixelSelection);

    KisConvolutionPainter horizPainter(interm);
    horizPainter.setChannelFlags(interm->colorSpace()->channelFlags(false, true));
    horizPainter.applyMatrix(kernelHoriz, pixelSelection, rect.topLeft(), rect.topLeft(), rect.size(), BORDER_REPEAT);
    horizPainter.end();

    KisConvolutionPainter verticalPainter(pixelSelection);
    verticalPainter.setChannelFlags(pixelSelection->colorSpace()->channelFlags(false, true));
    verticalPainter.applyMatrix(kernelVertical, interm, rect.topLeft(), rect.topLeft(), rect.size(), BORDER_REPEAT);
    verticalPainter.end();
}


KisGrowSelectionFilter::KisGrowSelectionFilter(qint32 xRadius, qint32 yRadius)
    : m_xRadius(xRadius),
        m_yRadius(yRadius)
{
}

KUndo2MagicString KisGrowSelectionFilter::name()
{
    return kundo2_i18n("Grow Selection");
}

QRect KisGrowSelectionFilter::changeRect(const QRect& rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(defaultBounds);

    return rect.adjusted(-m_xRadius, -m_yRadius, m_xRadius, m_yRadius);
}

void KisGrowSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect& rect)
{
    if (m_xRadius <= 0 || m_yRadius <= 0) return;

    /**
        * Much code resembles Shrink filter, so please fix bugs
        * in both filters
        */

    quint8  **buf;  // caches the region's pixel data
    quint8  **max;  // caches the largest values for each column

    max = new quint8* [rect.width() + 2 * m_xRadius];
    buf = new quint8* [m_yRadius + 1];
    for (qint32 i = 0; i < m_yRadius + 1; i++) {
        buf[i] = new quint8[rect.width()];
    }
    quint8* buffer = new quint8[(rect.width() + 2 * m_xRadius) *(m_yRadius + 1)];
    for (qint32 i = 0; i < rect.width() + 2 * m_xRadius; i++) {
        if (i < m_xRadius)
            max[i] = buffer;
        else if (i < rect.width() + m_xRadius)
            max[i] = &buffer[(m_yRadius + 1) * (i - m_xRadius)];
        else
            max[i] = &buffer[(m_yRadius + 1) * (rect.width() + m_xRadius - 1)];

        for (qint32 j = 0; j < m_xRadius + 1; j++)
            max[i][j] = 0;
    }
    /* offset the max pointer by m_xRadius so the range of the array
        is [-m_xRadius] to [region->w + m_xRadius] */
    max += m_xRadius;

    quint8* out = new quint8[ rect.width()];  // holds the new scan line we are computing

    qint32* circ = new qint32[ 2 * m_xRadius + 1 ]; // holds the y coords of the filter's mask
    computeBorder(circ, m_xRadius, m_yRadius);

    /* offset the circ pointer by m_xRadius so the range of the array
        is [-m_xRadius] to [m_xRadius] */
    circ += m_xRadius;

    memset(buf[0], 0, rect.width());
    for (qint32 i = 0; i < m_yRadius && i < rect.height(); i++) { // load top of image
        pixelSelection->readBytes(buf[i + 1], rect.x(), rect.y() + i, rect.width(), 1);
    }

    for (qint32 x = 0; x < rect.width() ; x++) { // set up max for top of image
        max[x][0] = 0;         // buf[0][x] is always 0
        max[x][1] = buf[1][x]; // MAX (buf[1][x], max[x][0]) always = buf[1][x]
        for (qint32 j = 2; j < m_yRadius + 1; j++) {
            max[x][j] = MAX(buf[j][x], max[x][j-1]);
        }
    }

    for (qint32 y = 0; y < rect.height(); y++) {
        rotatePointers(buf, m_yRadius + 1);
        if (y < rect.height() - (m_yRadius))
            pixelSelection->readBytes(buf[m_yRadius], rect.x(), rect.y() + y + m_yRadius, rect.width(), 1);
        else
            memset(buf[m_yRadius], 0, rect.width());
        for (qint32 x = 0; x < rect.width(); x++) { /* update max array */
            for (qint32 i = m_yRadius; i > 0; i--) {
                max[x][i] = MAX(MAX(max[x][i - 1], buf[i - 1][x]), buf[i][x]);
            }
            max[x][0] = buf[0][x];
        }
        qint32 last_max = max[0][circ[-1]];
        qint32 last_index = 1;
        for (qint32 x = 0; x < rect.width(); x++) { /* render scan line */
            last_index--;
            if (last_index >= 0) {
                if (last_max == 255)
                    out[x] = 255;
                else {
                    last_max = 0;
                    for (qint32 i = m_xRadius; i >= 0; i--)
                        if (last_max < max[x + i][circ[i]]) {
                            last_max = max[x + i][circ[i]];
                            last_index = i;
                        }
                    out[x] = last_max;
                }
            } else {
                last_index = m_xRadius;
                last_max = max[x + m_xRadius][circ[m_xRadius]];
                for (qint32 i = m_xRadius - 1; i >= -m_xRadius; i--)
                    if (last_max < max[x + i][circ[i]]) {
                        last_max = max[x + i][circ[i]];
                        last_index = i;
                    }
                out[x] = last_max;
            }
        }
        pixelSelection->writeBytes(out, rect.x(), rect.y() + y, rect.width(), 1);
    }
    /* undo the offsets to the pointers so we can free the malloced memory */
    circ -= m_xRadius;
    max -= m_xRadius;

    delete[] circ;
    delete[] buffer;
    delete[] max;
    for (qint32 i = 0; i < m_yRadius + 1; i++)
        delete[] buf[i];
    delete[] buf;
    delete[] out;
}


KisShrinkSelectionFilter::KisShrinkSelectionFilter(qint32 xRadius, qint32 yRadius, bool edgeLock)
    : m_xRadius(xRadius),
      m_yRadius(yRadius),
      m_edgeLock(edgeLock)
{
}

KUndo2MagicString KisShrinkSelectionFilter::name()
{
    return kundo2_i18n("Shrink Selection");
}

QRect KisShrinkSelectionFilter::changeRect(const QRect& rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(defaultBounds);
    return rect.adjusted(-m_xRadius, -m_yRadius, m_xRadius, m_yRadius);
}

void KisShrinkSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect& rect)
{
    if (m_xRadius <= 0 || m_yRadius <= 0) return;

    /*
        pretty much the same as fatten_region only different
        blame all bugs in this function on jaycox@gimp.org
    */
    /* If edge_lock is true  we assume that pixels outside the region
        we are passed are identical to the edge pixels.
        If edge_lock is false, we assume that pixels outside the region are 0
    */
    quint8  **buf;  // caches the region's pixels
    quint8  **max;  // caches the smallest values for each column
    qint32    last_max, last_index;

    max = new quint8* [rect.width() + 2 * m_xRadius];
    buf = new quint8* [m_yRadius + 1];
    for (qint32 i = 0; i < m_yRadius + 1; i++) {
        buf[i] = new quint8[rect.width()];
    }

    qint32 buffer_size = (rect.width() + 2 * m_xRadius + 1) * (m_yRadius + 1);
    quint8* buffer = new quint8[buffer_size];

    if (m_edgeLock)
        memset(buffer, 255, buffer_size);
    else
        memset(buffer, 0, buffer_size);

    for (qint32 i = 0; i < rect.width() + 2 * m_xRadius; i++) {
        if (i < m_xRadius)
            if (m_edgeLock)
                max[i] = buffer;
            else
                max[i] = &buffer[(m_yRadius + 1) * (rect.width() + m_xRadius)];
        else if (i < rect.width() + m_xRadius)
            max[i] = &buffer[(m_yRadius + 1) * (i - m_xRadius)];
        else if (m_edgeLock)
            max[i] = &buffer[(m_yRadius + 1) * (rect.width() + m_xRadius - 1)];
        else
            max[i] = &buffer[(m_yRadius + 1) * (rect.width() + m_xRadius)];
    }
    if (!m_edgeLock)
        for (qint32 j = 0 ; j < m_xRadius + 1; j++) max[0][j] = 0;

    // offset the max pointer by m_xRadius so the range of the array is [-m_xRadius] to [region->w + m_xRadius]
    max += m_xRadius;

    quint8* out = new quint8[rect.width()]; // holds the new scan line we are computing

    qint32* circ = new qint32[2 * m_xRadius + 1]; // holds the y coords of the filter's mask

    computeBorder(circ, m_xRadius, m_yRadius);

    // offset the circ pointer by m_xRadius so the range of the array is [-m_xRadius] to [m_xRadius]
    circ += m_xRadius;

    for (qint32 i = 0; i < m_yRadius && i < rect.height(); i++) // load top of image
        pixelSelection->readBytes(buf[i + 1], rect.x(), rect.y() + i, rect.width(), 1);

    if (m_edgeLock)
        memcpy(buf[0], buf[1], rect.width());
    else
        memset(buf[0], 0, rect.width());


    for (qint32 x = 0; x < rect.width(); x++) { // set up max for top of image
        max[x][0] = buf[0][x];
        for (qint32 j = 1; j < m_yRadius + 1; j++)
            max[x][j] = MIN(buf[j][x], max[x][j-1]);
    }

    for (qint32 y = 0; y < rect.height(); y++) {
        rotatePointers(buf, m_yRadius + 1);
        if (y < rect.height() - m_yRadius)
            pixelSelection->readBytes(buf[m_yRadius], rect.x(), rect.y() + y + m_yRadius, rect.width(), 1);
        else if (m_edgeLock)
            memcpy(buf[m_yRadius], buf[m_yRadius - 1], rect.width());
        else
            memset(buf[m_yRadius], 0, rect.width());

        for (qint32 x = 0 ; x < rect.width(); x++) { // update max array
            for (qint32 i = m_yRadius; i > 0; i--) {
                max[x][i] = MIN(MIN(max[x][i - 1], buf[i - 1][x]), buf[i][x]);
            }
            max[x][0] = buf[0][x];
        }
        last_max =  max[0][circ[-1]];
        last_index = 0;

        for (qint32 x = 0 ; x < rect.width(); x++) { // render scan line
            last_index--;
            if (last_index >= 0) {
                if (last_max == 0)
                    out[x] = 0;
                else {
                    last_max = 255;
                    for (qint32 i = m_xRadius; i >= 0; i--)
                        if (last_max > max[x + i][circ[i]]) {
                            last_max = max[x + i][circ[i]];
                            last_index = i;
                        }
                    out[x] = last_max;
                }
            } else {
                last_index = m_xRadius;
                last_max = max[x + m_xRadius][circ[m_xRadius]];
                for (qint32 i = m_xRadius - 1; i >= -m_xRadius; i--)
                    if (last_max > max[x + i][circ[i]]) {
                        last_max = max[x + i][circ[i]];
                        last_index = i;
                    }
                out[x] = last_max;
            }
        }
        pixelSelection->writeBytes(out, rect.x(), rect.y() + y, rect.width(), 1);
    }

    // undo the offsets to the pointers so we can free the malloced memory
    circ -= m_xRadius;
    max -= m_xRadius;

    delete[] circ;
    delete[] buffer;
    delete[] max;
    for (qint32 i = 0; i < m_yRadius + 1; i++)
        delete[] buf[i];
    delete[] buf;
    delete[] out;
}


KUndo2MagicString KisSmoothSelectionFilter::name()
{
    return kundo2_i18n("Smooth Selection");
}

QRect KisSmoothSelectionFilter::changeRect(const QRect& rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(defaultBounds);

    const qint32 radius = 1;
    return rect.adjusted(-radius, -radius, radius, radius);
}

void KisSmoothSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect& rect)
{
    // Simple convolution filter to smooth a mask (1bpp)
    quint8      *buf[3];

    qint32 width = rect.width();
    qint32 height = rect.height();


    quint8* out = new quint8[width];
    for (qint32 i = 0; i < 3; i++)
        buf[i] = new quint8[width + 2];


    // load top of image
    pixelSelection->readBytes(buf[0] + 1, rect.x(), rect.y(), width, 1);

    buf[0][0]         = buf[0][1];
    buf[0][width + 1] = buf[0][width];

    memcpy(buf[1], buf[0], width + 2);

    for (qint32 y = 0; y < height; y++) {
        if (y + 1 < height) {
            pixelSelection->readBytes(buf[2] + 1, rect.x(), rect.y() + y + 1, width, 1);

            buf[2][0]         = buf[2][1];
            buf[2][width + 1] = buf[2][width];
        } else {
            memcpy(buf[2], buf[1], width + 2);
        }

        for (qint32 x = 0 ; x < width; x++) {
            qint32 value = (buf[0][x] + buf[0][x+1] + buf[0][x+2] +
                            buf[1][x] + buf[2][x+1] + buf[1][x+2] +
                            buf[2][x] + buf[1][x+1] + buf[2][x+2]);

            out[x] = value / 9;
        }

        pixelSelection->writeBytes(out, rect.x(), rect.y() + y, width, 1);
        rotatePointers(buf, 3);
    }

    for (qint32 i = 0; i < 3; i++)
        delete[] buf[i];
    delete[] out;
}


KUndo2MagicString KisInvertSelectionFilter::name()
{
    return kundo2_i18n("Invert Selection");
}

QRect KisInvertSelectionFilter::changeRect(const QRect &rect, KisDefaultBoundsBaseSP defaultBounds)
{
    Q_UNUSED(rect);
    return defaultBounds->bounds();
}

void KisInvertSelectionFilter::process(KisPixelSelectionSP pixelSelection, const QRect& rect)
{
    Q_UNUSED(rect);
    pixelSelection->invert();
}
