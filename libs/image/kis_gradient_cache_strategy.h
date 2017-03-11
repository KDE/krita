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
#ifndef KIS_GRADINT_CACHE_STRATEGY_H
#define KIS_GRADINT_CACHE_STRATEGY_H
#include <QtGlobal>
#include <KoColorSpace.h>
#include <KoChannelInfo.h>
#include <KoAbstractGradient.h>
#include <KoColor.h>
#include <QSharedPointer>
#include "boost/random.hpp"


class KisGradientCacheStategy
{
public: 
    KisGradientCacheStategy(qint32 steps, const KoColorSpace *colorSpace);
    virtual ~KisGradientCacheStategy();
    virtual const quint8 *cachedAt(qreal t) = 0;
    static qint32 minColorDepth(const KoColorSpace *colorSpace);

protected:
    /// gets step of change t at position 0 <= t <= 1
    double stepAt(qreal t) const;

protected:
    const KoColorSpace *m_colorSpace;
    qint32 m_max;
    KoColor m_black;
    QVector<qreal> m_step;
};

class Bit8GradientCacheStategy : public KisGradientCacheStategy
{
private:
    struct nearColors{
        nearColors(){}
        nearColors(QColor color, const KoColorSpace *m_colorSpace);
        QSharedPointer<KoColor> m_colors[7];
        QColor getColor(QSharedPointer<KoColor> color);
    };

    struct colorComponents{
        colorComponents(QColor& color);
        int m_red;
        int m_green;
        int m_blue;
        int m_redMinus;
        int m_greenMinus;
        int m_blueMinus;
        int m_alpha;
        int m_alphaMinus;

    private:
        int checkBorders(int component);
    };

public:
    Bit8GradientCacheStategy(const KoAbstractGradient *gradient, qint32 steps, const KoColorSpace *colorSpace);
    const quint8 *cachedAt(qreal t) override;

private:
    QMap<qreal, nearColors> m_nearColors;
    boost::mt11213b  m_gen;
    boost::random::uniform_smallint<> m_distribution;
};

class NotBit8GradientCacheStategy : public KisGradientCacheStategy
{
public:
    NotBit8GradientCacheStategy(const KoAbstractGradient *gradient, qint32 steps, const KoColorSpace *colorSpace);
    const quint8 *cachedAt(qreal t) override;

private:
    QVector<KoColor> m_colors;
    boost::mt11213b  m_gen;
    boost::random::uniform_smallint<> m_distribution;

};
#endif //KIS_GRADIENT_CACHE_STRATEGY_H
