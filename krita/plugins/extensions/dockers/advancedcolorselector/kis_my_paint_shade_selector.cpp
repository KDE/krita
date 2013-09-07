/*
 *  Copyright (c) 2010 Adam Celarek <kdedev at xibo dot at>
 *  Copyright (c) 2008 Martin Renold <martinxyz@gmx.ch>
 *  Copyright (c) 2009 Ilya Portnov <nomail>
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

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kcomponentdata.h>
#include <kglobal.h>

#include "KoColorSpace.h"
#include "KoColorSpaceRegistry.h"
#include "KoColor.h"
#include "KoCanvasResourceManager.h"

inline int sqr(int x);
inline qreal sqr2(qreal x);
inline int signedSqr(int x);


KisMyPaintShadeSelector::KisMyPaintShadeSelector(QWidget *parent) :
        KisColorSelectorBase(parent),
        m_pixelCache(0, 0, QImage::Format_ARGB32_Premultiplied),
        m_updateTimer(new QTimer(this))
{
    setAcceptDrops(true);

    setMinimumSize(80, 80);
    setColor(QColor(255,0,0));

    m_updateTimer->setInterval(1);
    m_updateTimer->setSingleShot(true);
    connect(m_updateTimer, SIGNAL(timeout()), this, SLOT(update()));
}

void KisMyPaintShadeSelector::paintEvent(QPaintEvent *) {
    // Hint to the casual reader: some of the calculation here do not
    // what Martin Renold originally intended. Not everything here will make sense.
    // It does not matter in the end, as long as the result looks good.

    // This selector was ported from MyPaint in 2010

    m_pixelCache = QImage(width(), height(), QImage::Format_ARGB32_Premultiplied);
    QImage circleBorder(width(), height(), QImage::Format_ARGB32_Premultiplied);
    circleBorder.fill(qRgba(0,0,0,0));

    int size = qMin(width(), height());
    int s_radius = size/2.6;

    KoColor kocolor(colorSpace());
    QColor qcolor;

    for (int x=0; x<width(); x++) {
        for (int y=0; y<height(); y++) {

            float v_factor = 0.6f;
            float s_factor = 0.6f;
            float v_factor2 = 0.013f;
            float s_factor2 = 0.013f;

            int stripe_width = 15*size/255.;

            float h = 0;
            float s = 0;
            float v = 0;

            int dx = x-width()/2;
            int dy = y-height()/2;
            int diag = sqrt(2.0)*size/2;

            int dxs, dys;
            if (dx > 0)
                dxs = dx - stripe_width;
            else
                dxs = dx + stripe_width;
            if (dy > 0)
                dys = dy - stripe_width;
            else
                dys = dy + stripe_width;

            qreal r = std::sqrt(qreal(sqr(dxs)+sqr(dys)));

            if (qMin(abs(dx), abs(dy)) < stripe_width) {
                // horizontal and vertical lines
                dx = (dx/qreal(width()))*255;
                dy = (dy/qreal(height()))*255;
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
            else if (r < s_radius+1) {
                // hue
                if (dx > 0)
                    h = 90*sqr2(r/s_radius);
                else
                    h = 360 - 90*sqr2(r/s_radius);
                s = 256*(atan2f(std::abs(dxs),dys)/M_PI) - 128;

                if (r > s_radius) {
                    // antialiasing boarder
                    qreal aaFactor = r-floor(r); // part after the decimal point
                    aaFactor = 1-aaFactor;

                    qreal fh = m_colorH + h/360.0;
                    qreal fs = m_colorS + s/255.0;
                    qreal fv = m_colorV + v/255.0;

                    fh -= floor(fh);
                    fs = qBound(qreal(0.0), fs, qreal(1.0));
                    fv = qBound(qreal(0.1), fv, qreal(1.0));

                    qcolor.setHsvF(fh, fs, fv);
                    kocolor.fromQColor(qcolor);
                    kocolor.toQColor(&qcolor);

                    int aaR = qcolor.red()*aaFactor;
                    int aaG = qcolor.green()*aaFactor;
                    int aaB = qcolor.blue()*aaFactor;

                    circleBorder.setPixel(x, y, qRgba(aaR, aaG, aaB, 255*aaFactor));

                    h = 180 + 180*atan2f(dys,-dxs)/M_PI;
                    v = 255*(r-s_radius)/(diag-s_radius) - 128;
                }
            }
            else {
                // background (hue+darkness gradient)
                h = 180 + 180*atan2f(dys,-dxs)/M_PI;
                v = 255*(r-s_radius)/(diag-s_radius) - 128;
            }

            qreal fh = m_colorH + h/360.0;
            qreal fs = m_colorS + s/255.0;
            qreal fv = m_colorV + v/255.0;

            fh -= floor(fh);
            fs = qBound(qreal(0.0), fs, qreal(1.0));
            fv = qBound(qreal(0.1), fv, qreal(1.0));

            qcolor.setHsvF(fh, fs, fv);
            kocolor.fromQColor(qcolor);
            kocolor.toQColor(&qcolor);
            m_pixelCache.setPixel(x, y, qcolor.rgb());
        }
    }

    QPainter pixelCachePainter(&m_pixelCache);
    pixelCachePainter.drawImage(0,0, circleBorder);

    QPainter painter(this);
    painter.drawImage(0, 0, m_pixelCache);
}


void KisMyPaintShadeSelector::mousePressEvent(QMouseEvent* e)
{
    e->setAccepted(false);
    KisColorSelectorBase::mousePressEvent(e);
}

void KisMyPaintShadeSelector::mouseMoveEvent(QMouseEvent *e)
{
    if(rect().contains(e->pos()))
        updateColorPreview(m_pixelCache.pixel(e->x(), e->y()));
    KisColorSelectorBase::mouseMoveEvent(e);
}

void KisMyPaintShadeSelector::mouseReleaseEvent(QMouseEvent *e)
{
    e->setAccepted(false);
    KisColorSelectorBase::mouseReleaseEvent(e);

    if(!e->isAccepted()) {
        QColor color = QColor(m_pixelCache.pixel(e->x(), e->y()));
        color = findGeneratingColor(KoColor(color, KoColorSpaceRegistry::instance()->rgb8()));

        ColorRole role=Foreground;
        if(e->button()&Qt::RightButton)
            role=Background;

        KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");
        bool onRightClick = cfg.readEntry("shadeSelectorUpdateOnRightClick", false);
        bool onLeftClick = cfg.readEntry("shadeSelectorUpdateOnLeftClick", false);

        if((e->button()&Qt::LeftButton && onLeftClick)
            || (e->button()&Qt::RightButton && onRightClick)) {
            setColor(color);
        }

        commitColor(KoColor(color, colorSpace()), role);
    }
}

KisColorSelectorBase* KisMyPaintShadeSelector::createPopup() const
{
    KisColorSelectorBase* popup = new KisMyPaintShadeSelector(0);
    popup->setColor(m_lastColor);
    return popup;
}

void KisMyPaintShadeSelector::setColor(const QColor &c) {
    m_colorH=c.hsvHueF();
    m_colorS=c.hsvSaturationF();
    m_colorV=c.valueF();
    m_lastColor=c;

    m_updateTimer->start();
}

void KisMyPaintShadeSelector::canvasResourceChanged(int key, const QVariant &v)
{
    if(m_colorUpdateAllowed==false)
        return;

    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");

    bool onForeground = cfg.readEntry("shadeSelectorUpdateOnForeground", false);
    bool onBackground = cfg.readEntry("shadeSelectorUpdateOnBackground", true);

    if ((key == KoCanvasResourceManager::ForegroundColor && onForeground)
        || (key == KoCanvasResourceManager::BackgroundColor && onBackground)) {
        setColor(findGeneratingColor(v.value<KoColor>()));
    }
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
