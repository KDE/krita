/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
 *
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

#include <QPoint>

#include <KDebug>

#include "KoColor.h"
#include "KoColorSpace.h"

#include "kis_image.h"
#include "kis_iterators_pixel.h"
#include "kis_paint_device.h"
#include "kis_paint_layer.h"
#include "kis_painter.h"
#include "kis_painterly_overlay.h"
#include "kis_random_accessor.h"

#include "kis_complex_color.h"

const PropertyCell hardDefault = {
    0.7,
    0.7,
    0.7,
    0.7,
    0.7,
    0.7,
    0.7,
    0.7
};

const double DEFAULT_SCALE = 1.0;

KisComplexColor::KisComplexColor(KoColorSpace *colorSpace)
    : KisPaintDevice(colorSpace)
{
    m_scaling = DEFAULT_SCALE;
    m_defaultColor = KoColor(colorSpace);
    m_defaultProperty = 0;

    KoColor kc(Qt::black, colorSpace);
    fromKoColor(kc);
}

KisComplexColor::KisComplexColor(KoColorSpace *colorSpace, const KoColor &kc)
    : KisPaintDevice(colorSpace)
{
    m_scaling = DEFAULT_SCALE;
    m_defaultColor = KoColor(colorSpace);
    m_defaultProperty = 0;

    fromKoColor(kc);
}

KisComplexColor::~KisComplexColor()
{
    if (m_defaultProperty)
        delete [] m_defaultProperty;
}

void KisComplexColor::fromKoColor(const KoColor &kc)
{
    if (painterlyOverlay())
        removePainterlyOverlay();
    createPainterlyOverlay();
    clear();

    m_center = QPoint(0,0);
    m_offset = QPoint(0,0);
    m_size = QSize(1,1);

    translate();

    setDefaultColor(kc);
    setDefaultProperty(hardDefault);
}

KoColor KisComplexColor::simpleColor()
{
    const int channels = colorSpace()->colorChannelCount();
    int total = 0;
    QVector<float> sum(channels+1);
    QVector<float> cur(channels+1);

    KoColor kc(colorSpace());

    for (int i = 0; i < channels; i++)
        sum[i] = 0;
    sum[channels] = 1;

    for (int y = top(); y < bottom(); y++) {
        for (int x = left(); x < right(); x++) {
            if (colorSpace()->alpha(rawData(x,y))) {
                colorSpace()->normalisedChannelsValue(rawData(x,y), cur);
                for (int i = 0; i < channels; i++)
                    sum[i] += cur[i];
                total++;
            }
        }
    }

    if (total) {
        for (int i = 0; i < channels; i++)
            sum[i] /= (float)total;

        colorSpace()->fromNormalisedChannelsValue(kc.data(), sum);
    }
    return kc;
}

KoColor KisComplexColor::defaultColor()
{
    if (!m_defaultColor.colorSpace() || m_defaultColor.colorSpace()->id() != colorSpace()->id())
        m_defaultColor.convertTo(colorSpace());
    return m_defaultColor;
}

quint8 * KisComplexColor::defaultProperty()
{
    return m_defaultProperty;
}

void KisComplexColor::setDefaultColor(const KoColor &kc)
{
    m_defaultColor = kc;
    m_defaultColor.convertTo(colorSpace());
}

void KisComplexColor::setDefaultProperty(const PropertyCell &pc)
{
    if (m_defaultProperty)
        delete [] m_defaultProperty;

    m_defaultProperty = new quint8[painterlyOverlay()->colorSpace()->pixelSize()];

    memcpy(m_defaultProperty, reinterpret_cast<const quint8 *>(&pc), sizeof(PropertyCell));
}

QSize KisComplexColor::size()
{
    return m_size;
}

void KisComplexColor::setSize(const QSize &rc)
{

    QPoint more;

    int oldleft = absLeft();
    int oldtop = absTop();

    m_size = rc;

    int newleft = absLeft();
    int newtop = absTop();

    more.setX( ( newleft - oldleft ) );
    more.setY( ( newtop - oldtop ) );

//     m_center -= more;
    m_offset += more;

    translate();
}

int KisComplexColor::absLeft()
{
    int l = - ( m_size.width() - (m_size.width() % 2) ) / 2;
    return l;
}

int KisComplexColor::absTop()
{
    int t = - ( m_size.height() - (m_size.height() % 2) ) / 2;
    return t;
}

int KisComplexColor::absRight()
{
    int r = absLeft() + m_size.width();
    return r;
}

int KisComplexColor::absBottom()
{
    int b = absTop() + m_size.height();
    return b;
}

int KisComplexColor::left()
{
    int l = absLeft() - m_center.x();
    return l;
}

int KisComplexColor::top()
{
    int t = absTop() - m_center.y();
    return t;
}

int KisComplexColor::right()
{
    int r = absRight() - m_center.x();
    return r;
}

int KisComplexColor::bottom()
{
    int b = absBottom() - m_center.y();
    return b;
}

quint8 * KisComplexColor::rawData(int x, int y)
{
    absolute(&x, &y);

    KisRandomAccessorPixel ac = createRandomAccessor(x,y);

    if (!colorSpace()->alpha(ac.rawData()))
        memcpy(ac.rawData(), defaultColor().data(), colorSpace()->pixelSize());

    return ac.rawData();
}

quint8 * KisComplexColor::rawData(const QPoint &p)
{
    return rawData(p.x(),p.y());
}

PropertyCell * KisComplexColor::property(int x, int y)
{
    absolute(&x, &y);

    KisRandomAccessorPixel ac = painterlyOverlay()->createRandomAccessor(x,y);

    PropertyCell *cell = reinterpret_cast<PropertyCell *>(ac.rawData());
    if (!cell->volume)
        memcpy(ac.rawData(), m_defaultProperty, sizeof(PropertyCell));

    return cell;
}

PropertyCell * KisComplexColor::property(const QPoint &p)
{
    return property(p.x(),p.y());
}

float KisComplexColor::scaling()
{
    return m_scaling;
}

QPoint KisComplexColor::center()
{
    return m_center;
}

void KisComplexColor::center(int *x, int *y)
{
    *x = m_center.x();
    *y = m_center.y();
}

void KisComplexColor::setColor(int x, int y, const KoColor &kc)
{
    absolute(&x, &y);

    KoColor tmp = kc;
    tmp.convertTo(colorSpace());

    KisRandomAccessorPixel ac = createRandomAccessor(x,y);
    memcpy(ac.rawData(), tmp.data(), colorSpace()->pixelSize());

    colorSpace()->setAlpha(ac.rawData(), 255, 1);
}

void KisComplexColor::setColor(const QPoint &p, const KoColor &kc)
{
    setColor(p.x(),p.y(),kc);
}

void KisComplexColor::setProperty(int x, int y, const PropertyCell &pc)
{
    absolute(&x, &y);

    KisRandomAccessorPixel ac = painterlyOverlay()->createRandomAccessor(x,y);
    memcpy(ac.rawData(), reinterpret_cast<const quint8 *>(&pc), painterlyOverlay()->colorSpace()->pixelSize());
}

void KisComplexColor::setProperty(const QPoint &p, const PropertyCell &pc)
{
    setProperty(p.x(),p.y(),pc);
}

float KisComplexColor::setScaling(float s)
{
    float oldScaling = m_scaling;
    m_scaling = s;

    return oldScaling;
}

QPoint KisComplexColor::setCenter(int x, int y)
{
    QPoint oldCenter = m_center;

    absolute(&x, &y);
    m_center = QPoint(x,y);

    return oldCenter;
}

QPoint KisComplexColor::setCenter(QPoint p)
{
    QPoint oldCenter = m_center;

    absolute(&p);
    m_center = p;

    return oldCenter;
}

KisPaintDeviceSP KisComplexColor::dab(int w, int h)
{/*
   if (w > m_size.width() || h > m_size.height())
   return 0;

   int x = m_center.x() - w/2;
   int y = m_center.y() - h/2;

   KisPaintDeviceSP dst = new KisPaintDevice(colorSpace());

   KisRectIteratorPixel it_dst = dst->createRectIterator(0,0,w,h);
   KisRectIteratorPixel it_src = createRectIterator(x,y,w,h);

   while(!it_src.isDone()) {
   memcpy(it_dst.rawData(),it_src.rawData(),colorSpace()->pixelSize());
   ++it_src;
   ++it_dst;
   }

   return dst;
 */}

void KisComplexColor::scale(int *x, int *y)
{
    *x = (int)(*x * m_scaling);
    *y = (int)(*y * m_scaling);
}

void KisComplexColor::scale(QPoint *p)
{
    *p *= m_scaling;
}

void KisComplexColor::absolute(int *x, int *y)
{
    scale(x,y);
    *x += m_center.x();
    *y += m_center.y();
    Q_ASSERT(*x >= absLeft() && *y >= absTop());
    Q_ASSERT(*x < absRight() && *y < absBottom());
}

void KisComplexColor::absolute(QPoint *p)
{
    int x = p->x();
    int y = p->y();
    absolute(&x,&y);
    p->setX(x);
    p->setY(y);
}

void KisComplexColor::translate()
{
    move(m_offset);
    painterlyOverlay()->move(m_offset);
}

#include "kis_complex_color.moc"
