/* This file is part of the KDE project
 * Copyright (c) 2009 Jan Hambrecht <jaham@gmx.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "BlurEffect.h"
#include "KoFilterEffectRenderContext.h"
#include "KoFilterEffectLoadingContext.h"
#include "KoViewConverter.h"
#include "KoXmlWriter.h"
#include "KoXmlReader.h"
#include <klocalizedstring.h>
#include <QColor>
#include <QImage>

// Stack Blur Algorithm by Mario Klingemann <mario@quasimondo.com>
// fixed to handle alpha channel correctly by Zack Rusin
void fastbluralpha(QImage &img, int radius)
{
    if (radius < 1) {
        return;
    }

    QRgb *pix = (QRgb *)img.bits();
    int w   = img.width();
    int h   = img.height();
    int wm  = w - 1;
    int hm  = h - 1;
    int wh  = w * h;
    int div = radius + radius + 1;

    int *r = new int[wh];
    int *g = new int[wh];
    int *b = new int[wh];
    int *a = new int[wh];
    int rsum, gsum, bsum, asum, x, y, i, yp, yi, yw;
    QRgb p;
    int *vmin = new int[qMax(w, h)];

    int divsum = (div + 1) >> 1;
    divsum *= divsum;
    int *dv = new int[256 * divsum];
    for (i = 0; i < 256 * divsum; ++i) {
        dv[i] = (i / divsum);
    }

    yw = yi = 0;

    int **stack = new int *[div];
    for (int i = 0; i < div; ++i) {
        stack[i] = new int[4];
    }

    int stackpointer;
    int stackstart;
    int *sir;
    int rbs;
    int r1 = radius + 1;
    int routsum, goutsum, boutsum, aoutsum;
    int rinsum, ginsum, binsum, ainsum;

    for (y = 0; y < h; ++y) {
        rinsum = ginsum = binsum = ainsum
                                   = routsum = goutsum = boutsum = aoutsum
                                               = rsum = gsum = bsum = asum = 0;
        for (i = - radius; i <= radius; ++i) {
            p = pix[yi + qMin(wm, qMax(i, 0))];
            sir = stack[i + radius];
            sir[0] = qRed(p);
            sir[1] = qGreen(p);
            sir[2] = qBlue(p);
            sir[3] = qAlpha(p);

            rbs = r1 - abs(i);
            rsum += sir[0] * rbs;
            gsum += sir[1] * rbs;
            bsum += sir[2] * rbs;
            asum += sir[3] * rbs;

            if (i > 0) {
                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];
                ainsum += sir[3];
            } else {
                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];
                aoutsum += sir[3];
            }
        }
        stackpointer = radius;

        for (x = 0; x < w; ++x) {

            r[yi] = dv[rsum];
            g[yi] = dv[gsum];
            b[yi] = dv[bsum];
            a[yi] = dv[asum];

            rsum -= routsum;
            gsum -= goutsum;
            bsum -= boutsum;
            asum -= aoutsum;

            stackstart = stackpointer - radius + div;
            sir = stack[stackstart % div];

            routsum -= sir[0];
            goutsum -= sir[1];
            boutsum -= sir[2];
            aoutsum -= sir[3];

            if (y == 0) {
                vmin[x] = qMin(x + radius + 1, wm);
            }
            p = pix[yw + vmin[x]];

            sir[0] = qRed(p);
            sir[1] = qGreen(p);
            sir[2] = qBlue(p);
            sir[3] = qAlpha(p);

            rinsum += sir[0];
            ginsum += sir[1];
            binsum += sir[2];
            ainsum += sir[3];

            rsum += rinsum;
            gsum += ginsum;
            bsum += binsum;
            asum += ainsum;

            stackpointer = (stackpointer + 1) % div;
            sir = stack[(stackpointer) % div];

            routsum += sir[0];
            goutsum += sir[1];
            boutsum += sir[2];
            aoutsum += sir[3];

            rinsum -= sir[0];
            ginsum -= sir[1];
            binsum -= sir[2];
            ainsum -= sir[3];

            ++yi;
        }
        yw += w;
    }
    for (x = 0; x < w; ++x) {
        rinsum = ginsum = binsum = ainsum
                                   = routsum = goutsum = boutsum = aoutsum
                                               = rsum = gsum = bsum = asum = 0;

        yp = - radius * w;

        for (i = -radius; i <= radius; ++i) {
            yi = qMax(0, yp) + x;

            sir = stack[i + radius];

            sir[0] = r[yi];
            sir[1] = g[yi];
            sir[2] = b[yi];
            sir[3] = a[yi];

            rbs = r1 - abs(i);

            rsum += r[yi] * rbs;
            gsum += g[yi] * rbs;
            bsum += b[yi] * rbs;
            asum += a[yi] * rbs;

            if (i > 0) {
                rinsum += sir[0];
                ginsum += sir[1];
                binsum += sir[2];
                ainsum += sir[3];
            } else {
                routsum += sir[0];
                goutsum += sir[1];
                boutsum += sir[2];
                aoutsum += sir[3];
            }

            if (i < hm) {
                yp += w;
            }
        }

        yi = x;
        stackpointer = radius;

        for (y = 0; y < h; ++y) {
            pix[yi] = qRgba(dv[rsum], dv[gsum], dv[bsum], dv[asum]);

            rsum -= routsum;
            gsum -= goutsum;
            bsum -= boutsum;
            asum -= aoutsum;

            stackstart = stackpointer - radius + div;
            sir = stack[stackstart % div];

            routsum -= sir[0];
            goutsum -= sir[1];
            boutsum -= sir[2];
            aoutsum -= sir[3];

            if (x == 0) {
                vmin[y] = qMin(y + r1, hm) * w;
            }
            p = x + vmin[y];

            sir[0] = r[p];
            sir[1] = g[p];
            sir[2] = b[p];
            sir[3] = a[p];

            rinsum += sir[0];
            ginsum += sir[1];
            binsum += sir[2];
            ainsum += sir[3];

            rsum += rinsum;
            gsum += ginsum;
            bsum += binsum;
            asum += ainsum;

            stackpointer = (stackpointer + 1) % div;
            sir = stack[stackpointer];

            routsum += sir[0];
            goutsum += sir[1];
            boutsum += sir[2];
            aoutsum += sir[3];

            rinsum -= sir[0];
            ginsum -= sir[1];
            binsum -= sir[2];
            ainsum -= sir[3];

            yi += w;
        }
    }
    delete [] r;
    delete [] g;
    delete [] b;
    delete [] a;
    delete [] vmin;
    delete [] dv;

    for (int i = 0; i < div; ++i) {
        delete [] stack[i];
    }
    delete [] stack;
}

BlurEffect::BlurEffect()
    : KoFilterEffect(BlurEffectId, i18n("Gaussian blur"))
    , m_deviation(0, 0)
{
}

QPointF BlurEffect::deviation() const
{
    return m_deviation;
}

void BlurEffect::setDeviation(const QPointF &deviation)
{
    m_deviation.setX(qMax(qreal(0.0), deviation.x()));
    m_deviation.setY(qMax(qreal(0.0), deviation.y()));
}

QImage BlurEffect::processImage(const QImage &image, const KoFilterEffectRenderContext &context) const
{
    if (m_deviation.x() == 0.0 || m_deviation.y() == 0.0) {
        return image;
    }

    // TODO: take filter region into account
    // TODO: blur with different kernels in x and y
    // convert from bounding box coordinates
    QPointF dev = context.toUserSpace(m_deviation);
    // transform to view coordinates
    dev = context.viewConverter()->documentToView(dev);

    QImage result = image;
    fastbluralpha(result, dev.x());

    return result;
}

bool BlurEffect::load(const KoXmlElement &element, const KoFilterEffectLoadingContext &context)
{
    if (element.tagName() != id()) {
        return false;
    }

    QString deviationStr = element.attribute("stdDeviation");
    QStringList params = deviationStr.replace(',', ' ').simplified().split(' ');

    switch (params.count()) {
    case 1:
        m_deviation.rx() = params[0].toDouble();
        m_deviation.ry() = m_deviation.x();
        break;
    case 2:
        m_deviation.rx() = params[0].toDouble();
        m_deviation.ry() = params[1].toDouble();
        break;
    default:
        return false;
    }

    m_deviation = context.convertFilterPrimitiveUnits(m_deviation);

    return true;
}

void BlurEffect::save(KoXmlWriter &writer)
{
    writer.startElement(BlurEffectId);

    saveCommonAttributes(writer);

    if (m_deviation.x() != m_deviation.y()) {
        writer.addAttribute("stdDeviation", QString("%1, %2").arg(m_deviation.x()).arg(m_deviation.y()));
    } else {
        writer.addAttribute("stdDeviation", m_deviation.x());
    }

    writer.endElement();
}
