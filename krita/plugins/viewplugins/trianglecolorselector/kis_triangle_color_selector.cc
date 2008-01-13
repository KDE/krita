/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
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

#include "kis_triangle_color_selector.h"

#include <math.h>

#include <kis_debug.h>

#include <QPainter>
#include <QPixmap>
#include <KoColorConversions.h>

struct KisTriangleColorSelector::Private {
    QPixmap wheelPixmap;
};

KisTriangleColorSelector::KisTriangleColorSelector(QWidget* parent) : QWidget(parent), d(new Private)
{
}

KisTriangleColorSelector::~KisTriangleColorSelector()
{
    delete d;
}

void KisTriangleColorSelector::paintEvent( QPaintEvent * event )
{
    Q_UNUSED(event);
    QPainter p(this);
    int sizeWheel = qMin(width(), height());
    p.drawPixmap( (width()-sizeWheel) / 2, (height()-sizeWheel) / 2, d->wheelPixmap);
    p.end();
}

void KisTriangleColorSelector::resizeEvent( QResizeEvent * event )
{
    QWidget::resizeEvent( event );
    generateWheel();
}

inline double pow2(double v)
{
    return v*v;
}

void KisTriangleColorSelector::generateWheel()
{
    int size = qMin(width(), height());
    QImage img(size,size, QImage::Format_ARGB32_Premultiplied);
    double center = 0.5 * size;
    double normExt = pow2(center);
    double normInt = pow2(center * 0.7);
    for(int y = 0; y < size; y++)
    {
        double yc = y - center;
        double y2 = pow2( yc );
        for(int x = 0; x < size; x++)
        {
            double xc = x - center;
            double norm = pow2( xc ) + y2;
            if( norm <= normExt and norm >= normInt )
            {
                double angle = atan2(yc, xc);
                int h = (int)((180 * angle / M_PI) + 180);
                int r,g,b;
                hsv_to_rgb(h, 255, 255, &r, &g, &b);
                img.setPixel(x,y, qRgb(r, g, b));
            } else {
                img.setPixel(x,y, qRgba(0,0,0,0));
            }
        }
    }
    d->wheelPixmap = QPixmap::fromImage(img);
}
