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
#include <KoStopGradient.h>

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


Bit8GradientCacheStategy::colorComponents::colorComponents(QColor &color)
{
//    qDebug()<<ppVar(color.red());
//    qDebug()<<ppVar(color.green());
//    qDebug()<<ppVar(color.blue());



    m_alpha = color.alpha();
    m_red = color.red();
    m_blue = color.blue();
    m_green = color.green();
    m_redMinus = checkBorders(m_red - 1);
    m_greenMinus = checkBorders(m_green - 1);
    m_blueMinus = checkBorders(m_blue - 1);
    m_alphaMinus = checkBorders(m_alpha - 1);

    //gradient is drawing via alpha chanell sometimes
    if(m_alpha != 255) {
        m_red = 255 - m_alpha + m_red;
        m_green = 255 - m_alpha + m_green;
        m_blue = 255 - m_alpha + m_blue;
        m_redMinus = checkBorders(m_red - 1);
        m_greenMinus = checkBorders(m_green - 1);
        m_blueMinus = checkBorders(m_blue- 1);
        m_alpha = 255;
        m_alphaMinus = 255;
    }
//    qDebug()<<ppVar(m_red);
//    qDebug()<<ppVar(m_green);
//    qDebug()<<ppVar(m_blue);
//    qDebug()<<ppVar(m_alpha);

}


int Bit8GradientCacheStategy::colorComponents::checkBorders(int component)
{
    int temp  = component > -1 ? component : 0;
    return  temp;
}

Bit8GradientCacheStategy::nearColors::nearColors(QColor color, const KoColorSpace *m_colorSpace)
{
    for(int i = 0; i < 7 ; i++) {
        m_colors[i].reset(new KoColor(m_colorSpace));
    }
    QColor z;
    colorComponents components(color);

    z.setRgb(components.m_red, components.m_green, components.m_blue, components.m_alpha);
    m_colors[0]->fromQColor(z);
    z.setRgb(components.m_redMinus, components.m_green,components.m_blue, components.m_alpha);
    m_colors[1]->fromQColor(z);
    z.setRgb(components.m_red, components.m_greenMinus, components.m_blue, components.m_alpha);
    m_colors[2]->fromQColor(z);
    z.setRgb(components.m_red, components.m_green, components.m_blueMinus, components.m_alphaMinus);
    m_colors[3]->fromQColor(z);
    z.setRgb(components.m_red, components.m_greenMinus, components.m_blueMinus, components.m_alphaMinus);
    m_colors[4]->fromQColor(z);
    z.setRgb(components.m_redMinus, components.m_green,  components.m_blueMinus, components.m_alphaMinus);
    m_colors[5]->fromQColor(z);
    z.setRgb(components.m_redMinus, components.m_greenMinus, components.m_blue, components.m_alphaMinus);
    m_colors[6]->fromQColor(z);
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
        nearColors tt(tmpColor.toQColor(), m_colorSpace);
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

    //making colors distribution is more random
    if (randomValue > 6) {
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
    if (randomValue > 6 && m_nearColors.size() > tInt){
        return  m_nearColors.value(tInt).m_colors[0]->data();
    } else {
        if (m_nearColors.size() > tInt) {
            return  m_nearColors.value(tInt).m_colors[randomValue]->data();
        } else {
            return m_black.data();
       }
    }
}

NotBit8GradientCacheStategy::NotBit8GradientCacheStategy(const KoAbstractGradient *gradient, qint32 steps, const KoColorSpace *colorSpace)
    : KisGradientCacheStategy(steps, colorSpace), m_distribution(0,2)
{
    KoColor tmpColor(m_colorSpace);
    KoColor nextColor(m_colorSpace);
    gradient->colorAt(tmpColor, qreal(0) / m_max);
    qreal count = 1;
    qreal lastcount = 1;
    auto temp =  dynamic_cast<const KoStopGradient*>(gradient)->stops();
    QList<QPair<qreal, QColor>> stops;

    for(auto t: temp){
        QPair<qreal, QColor> z;
        z.first = t.first;
        z.second = t.second.toQColor();
        stops.push_back(z);
    }

    qDebug() << ppVar(stops);

    for(qint32 i = 0; i < m_max + 1; i++) {
        qreal t =  qreal(i) / m_max;
        gradient->colorAt(tmpColor, t);
       // qDebug()<<ppVar(tmpColor.toQColor());

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
    if (randomValue > 1) {
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
//    qDebug()<<ppVar(m_colors[tInt].toQColor().red());
//    qDebug()<<ppVar(m_colors[tInt].toQColor().green());
//    qDebug()<<ppVar(m_colors[tInt].toQColor().blue());
//    qDebug()<<ppVar(m_colors[tInt].toQColor().alpha());

    if (m_colors.size() > tInt) {
        return m_colors[tInt].data();
    }
    else {
        return m_black.data();
    }

}

