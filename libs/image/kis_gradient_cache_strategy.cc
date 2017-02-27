/*
 *  Copyright (c) 2017 Alexey Kapustin <djkah11@yandex.ru>
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
#include "kis_debug.h"
#include "kis_gradient_cache_strategy.h"

KisGradientCacheStategy::KisGradientCacheStategy(qint32 steps, const KoColorSpace *colorSpace)
                      : m_colorSpace(colorSpace)
{
    m_max = steps - 1;
    m_black = KoColor(colorSpace);
}

KisGradientCacheStategy::~KisGradientCacheStategy()
{

}

qint32 KisGradientCacheStategy::minColorDepth(const KoColorSpace *colorSpace)
{
    QList<KoChannelInfo *> colorInfo = colorSpace->channels();
    qint32 minColorDepth = colorInfo.first()->size();

    return  minColorDepth;
}

double KisGradientCacheStategy::stepAt(qreal t) const
{
    qint32 tInt = t * m_max + 0.5;
    if (m_step.size() > tInt) {
        return m_step[tInt];
    } else {
        return 0;
    }

}


Bit8GradientCacheStategy::Bit8GradientCacheStategy(const KoAbstractGradient *gradient, qint32 steps, const KoColorSpace *colorSpace)
    : KisGradientCacheStategy(steps, colorSpace), m_distribution(0, 9)
{
    KoColor tmpColor(m_colorSpace);
    KoColor nextColor(m_colorSpace);
    gradient->colorAt(tmpColor, qreal(0) / m_max);
    qreal count = 1;
    qreal lastcount = 1;
    for(qint32 i = 0; i < m_max + 1; i++) {
        qreal t =  qreal(i) / m_max;
        gradient->colorAt(tmpColor, t);
        nearColors tt(tmpColor.toQColor().alpha(), m_colorSpace);
        m_nearColors.insert(qint32(t * m_max + 0.5), tt);
        if(nextColor == tmpColor) {
            count++;
            m_step << (lastcount / m_max);
        } else {
            m_step << (count / m_max);
            lastcount = count;
            count = 1;
            nextColor = tmpColor;
       }
    }
}

const quint8 *Bit8GradientCacheStategy::cachedAt(qreal t)
{
    int randomValue = m_distribution(m_gen);
    if (randomValue>6) {
        t += stepAt(t) / 2;
    } else {
        if (randomValue < 3)
            t -= stepAt(t) / 2;
    }
    if (t > 1) {
        t = 1;
    } else if (t < 0) {
        t = 0;
    }
    qint32 tInt = t * m_max + 0.5;
    if (randomValue > 7 && m_nearColors.size() > tInt){
        return  m_nearColors.value(tInt).m_colors[0]->data();
    } else {
        if (m_nearColors.size() > tInt) {
            return  m_nearColors.value(tInt).m_colors[randomValue]->data();
        } else {
            return m_black.data();
       }
    }
}

Bit8GradientCacheStategy::nearColors::nearColors(int alpha, const KoColorSpace *m_colorSpace)
{
    for(int i = 0; i < 8 ; i++) {
        m_colors[i].reset(new KoColor(m_colorSpace));
    }
    QColor z;
    z.setAlpha(255);
    int color = 255- alpha;
    int newElement = (255 - alpha - 1) > - 1 ? (255 - alpha-1) : 0;
    z.setRgb(color,color, color);
    m_colors[0]->fromQColor(z);
    z.setRgb(newElement,color, color);
    m_colors[1]->fromQColor(z);
    z.setRgb(color,newElement, color);
    m_colors[2]->fromQColor(z);
    z.setRgb(color, color,newElement);
    m_colors[3]->fromQColor(z);
    z.setRgb(color, color,newElement);
    m_colors[4]->fromQColor(z);
    z.setRgb(color, newElement, newElement);
    m_colors[5]->fromQColor(z);
    z.setRgb(newElement,color,  newElement);
    m_colors[6]->fromQColor(z);
    z.setRgb(newElement, newElement,color);
    m_colors[7]->fromQColor(z);
}


NotBit8GradientCacheStategy::NotBit8GradientCacheStategy(const KoAbstractGradient *gradient, qint32 steps, const KoColorSpace *colorSpace)
    : KisGradientCacheStategy(steps, colorSpace), m_distribution(0,2)
{
    KoColor tmpColor(m_colorSpace);
    KoColor nextColor(m_colorSpace);
    gradient->colorAt(tmpColor, qreal(0) / m_max);
    qreal count = 1;
    qreal lastcount = 1;

    for(qint32 i = 0; i < m_max + 1; i++) {
        qreal t =  qreal(i) / m_max;
        gradient->colorAt(tmpColor, t);
        m_colors << tmpColor;
        if(nextColor == tmpColor) {
            count++;
            m_step << (lastcount / m_max);
        } else {
            m_step << (count / m_max);
            lastcount = count;
            count = 1;
            nextColor = tmpColor;
       }
    }
}

const quint8 *NotBit8GradientCacheStategy::cachedAt(qreal t)
{
    int randomValue = m_distribution(m_gen);
    if (randomValue>1) {
        t += stepAt(t);
    } else {
        if (randomValue < 1)
            t -= stepAt(t);
    }
    if (t > 1) {
        t = 1;
    } else if (t < 0) {
        t = 0;
    }
    qint32 tInt = t * m_max + 0.5;

    if (m_colors.size() > tInt) {
        return m_colors[tInt].data();
    }
    else {
        return m_black.data();
    }

}
