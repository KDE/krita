/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_color_slider.h"

#include <QPainter>

#include <KoColor.h>
#include <KoColorSpace.h>

struct KisColorSlider::Private {
    KoColor min;
    KoColor max;
    double v;
    QPixmap gradient;
    void generateGradient(int width, int height);
};


void KisColorSlider::Private::generateGradient(int width, int height)
{
    QImage img(width, height, QImage::Format_RGB32);
    KoMixColorsOp* mixop = min.colorSpace()->mixColorsOp();
    KoColor current(min.colorSpace());
    const quint8* colors[2];
    colors[0] = min.data();
    colors[1] = max.data();
    qint16 weights[2];
    for (int x = 0; x < width; x++) {
        double t = x / (double)width;
        weights[1] = t * 255;
        weights[0] = 255 - weights[1];
        mixop->mixColors( colors, weights, 2, current.data());
        QColor c = current.toQColor();
        for (int y = 0; y < height; y++) {
            img.setPixel(x, y, c.rgb());
        }
    }
    gradient = QPixmap::fromImage(img);
}

KisColorSlider::KisColorSlider( const KoColor& min, const KoColor& max ) : d(new Private)
{
    d->min = min;
    d->max = max;
    Q_ASSERT(*d->min.colorSpace() == *d->max.colorSpace());
    d->v = 0.5;
}

KisColorSlider::~KisColorSlider()
{
    delete d;
}

void KisColorSlider::setValue(double v)
{
    d->v = v;
}

void KisColorSlider::paintEvent(QPaintEvent * event)
{
    QPainter p(this);
    p.drawPixmap(0, 0, d->gradient);
    p.save();
    p.setPen(QPen(Qt::white, 1.0));
    p.translate((d->v * width()), 0.0);
    p.drawRect(QRectF(-1.5, 0 , 3.0, height()));
    p.restore();
    p.end();
}

void KisColorSlider::resizeEvent(QResizeEvent * event)
{
    d->generateGradient(width(), height());
}
