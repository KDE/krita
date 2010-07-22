/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *  Copyright (c) 2008 Martin Renold <martinxyz@gmx.ch>
 *  Copyright (c) 2009 Ilya Portnov
 *
 *  This class is based on "lib/colorchanger.hpp" from MyPaint (mypaint.intilinux.com)
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

#include "kis_my_paint_shade_selector.h"
#include <cmath>
#include <cstdlib>
#include <QImage>
#include <QColor>
#include <QPainter>
#include <QMouseEvent>
#include <QApplication>
#include <QDesktopWidget>
#include <QtGlobal>
#include <QTimer>

#include "KoColorSpace.h"
#include "KoColor.h"

#include <KDebug>

inline int sqr(int x);
inline qreal sqr2(qreal x);
inline int signedSqr(int x);


KisMyPaintShadeSelector::KisMyPaintShadeSelector(QWidget *parent) :
        KisColorSelectorBase(parent),
        m_pixelCache(m_size, m_size, QImage::Format_ARGB32_Premultiplied),
        m_initialised(false),
        m_updateTimer(new QTimer(this))
{
    precalculateData();
    setMinimumSize(80, 80);

    m_updateTimer->setInterval(1);
    m_updateTimer->setSingleShot(true);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(update()));
}

void KisMyPaintShadeSelector::paintEvent(QPaintEvent *) {
    if (m_initialised == false || m_pixelCacheColorSpace!=colorSpace()) {
        // if this is the first paintEvent, then we have to updated the cache before painting.
        // this isn't possible in the constructor, because we don't have the colorspace.
        updateSelector();
        m_initialised = true;
    }


    QPainter painter(this);
    QRect target(0, 0, qMin(width(), height()), qMin(width(), height()));
    painter.drawImage(target, m_pixelCache, m_pixelCache.rect());
}


void KisMyPaintShadeSelector::mousePressEvent(QMouseEvent* e)
{
    KisColorSelectorBase::mousePressEvent(e);

    if(!e->isAccepted()) {
        QColor color = pickColorAt(e->x(), e->y());

        ColorRole role=Foreground;
        if(e->buttons()&Qt::RightButton)
            role=Background;

        commitColor(color, role);
    }

}

QColor KisMyPaintShadeSelector::pickColorAt(int x, int y)
{
    qreal wdgSize = qMin(width(), height());
    qreal ratio = 256.0/wdgSize;

    //set the internal color the one on pos x/y
    calculatePos(x*ratio, y*ratio);

    QColor color=m_qcolor;

    //change the color of the selector to the one calculated above
    setColor(color);


    //repaint the widget
    update();

    return color;
}

KisColorSelectorBase* KisMyPaintShadeSelector::createPopup() const
{
    KisColorSelectorBase* popup = new KisMyPaintShadeSelector(0);
    popup->resize(256,256);
    return popup;
}

void KisMyPaintShadeSelector::setColor(const QColor &c) {
    m_colorH=c.hsvHueF();
    m_colorS=c.hsvSaturationF();
    m_colorV=c.valueF();

    //repaint the cache
    m_initialised=false;

    m_updateTimer->start();
}

void KisMyPaintShadeSelector::precalculateData() {
    // Hint to the casual reader: some of the calculation here do not
    // what Martin Renold originally intended. Not everything here will make sense.
    // It does not matter in the end, as long as the result looks good.

    int width, height;
    int s_radius = m_size/2.6;

    width = m_size;
    height = m_size;

    for (int y=0; y<height; y++) {
        for (int x=0; x<width; x++) {
            float v_factor = 0.6;
            float s_factor = 0.6;
            float v_factor2 = 0.013;
            float s_factor2 = 0.013;

            int stripe_width = 15;

            float h = 0;
            float s = 0;
            float v = 0;

            int dx = x-width/2;
            int dy = y-height/2;
            int diag = sqrt(2)*m_size/2;

            int dxs, dys;
            if (dx > 0)
                dxs = dx - stripe_width;
            else
                dxs = dx + stripe_width;
            if (dy > 0)
                dys = dy - stripe_width;
            else
                dys = dy + stripe_width;

            float r = std::sqrt(sqr(dxs)+sqr(dys));

            // hue
            if (r < s_radius) {
                if (dx > 0)
                    h = 90*sqr2(r/s_radius);
                else
                    h = 360 - 90*sqr2(r/s_radius);
                s = 256*(atan2f(std::abs(dxs),dys)/M_PI) - 128;
            } else {
                h = 180 + 180*atan2f(dys,-dxs)/M_PI;
                v = 255*(r-s_radius)/(diag-s_radius) - 128;
            }

            // horizontal and vertical lines
            int min = std::abs(dx);
            if (std::abs(dy) < min) min = std::abs(dy);
            if (min < stripe_width) {
                h = 0;
                // x-axis = value, y-axis = saturation
                v =    dx*v_factor + signedSqr(dx)*v_factor2;
                s = - (dy*s_factor + signedSqr(dy)*s_factor2);
                // but not both at once
                if (std::abs(dx) > std::abs(dy)) {
                    // horizontal stripe
                    s = 0.0;
                } else {
                    // vertical stripe
                    v = 0.0;
                }
            }

            m_precalcData[x][y].h=h;
            m_precalcData[x][y].s=s;
            m_precalcData[x][y].v=v;
        }
    }
}

void KisMyPaintShadeSelector::updateSelector() {
    Q_ASSERT(colorSpace());
    m_pixelCacheColorSpace=colorSpace();

    KoColor kocolor(colorSpace());
    for(int i=0; i<m_size; i++) {
        for(int j=0; j<m_size; j++) {
            calculatePos(i, j);
            kocolor.fromQColor(m_qcolor);
            kocolor.toQColor(&m_qcolor);

            m_pixelCache.setPixel(i, j, m_qcolor.rgb());
        }
    }
}

void KisMyPaintShadeSelector::calculatePos(int x, int y)
{
    PrecalcData& pre = m_precalcData[x][y];
    qreal fh = m_colorH + pre.h/360.0;
    qreal fs = m_colorS + pre.s/255.0;
    qreal fv = m_colorV + pre.v/255.0;

    fh -= floor(fh);
    fs = qBound(qreal(0.0), fs, qreal(1.0));
    fv = qBound(qreal(0.1), fv, qreal(1.0));

    m_qcolor.setHsvF(fh, fs, fv);
}

inline int sqr(int x) {
    return x*x;
}

inline qreal sqr2(qreal x) {
    return (x*x)/2+x/2;
}

inline int signedSqr(int x) {
    int sign = x>0?1:-1;
    return x*x*sign;
}
