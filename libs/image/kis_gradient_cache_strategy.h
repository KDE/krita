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
#include "boost/random.hpp"
#include <KoAbstractGradient.h>
#include <KoChannelInfo.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <QSharedPointer>
#include <QtGlobal>

class KisGradientCacheStategy {
public:
    KisGradientCacheStategy(qint32 steps, const KoColorSpace* colorSpace);
    virtual ~KisGradientCacheStategy();
    virtual const quint8* cachedAt(qreal t) = 0;
    static qint32 minColorDepth(const KoColorSpace* colorSpace);

protected:
    /// gets step of change t at position 0 <= t <= 1
    virtual double stepAt(qreal t) const = 0;

protected:
    const KoColorSpace* m_colorSpace;
    qint32 m_max;
    KoColor m_black;
    boost::mt11213b m_gen;
};

class Bit8Stategy {
public:
    Bit8Stategy(qreal m_max);
    struct offset {
        offset() {}
        offset(qreal currentOffset, qreal colorOffset)
            : m_currentOffset(currentOffset)
            , m_mainOffset(colorOffset)
        {
        }
        qreal m_currentOffset;
        qreal m_mainOffset;
    };

protected:
    /// gets a differ from the original color 0 <= t <= 1
    double offsetAt(qreal t) const;
    /// gets step of change t at position 0 <= t <= 1
    virtual double stepAt(qreal t) const = 0;
    QVector<offset> m_step;

private:
    qreal m_max2;
};

class Bit8GradientCacheStategy : public KisGradientCacheStategy, public Bit8Stategy {
public:
    Bit8GradientCacheStategy(const KoAbstractGradient* gradient, qint32 steps, const KoColorSpace* colorSpace);
    const quint8* cachedAt(qreal t) override;

protected:
    QVector<KoColor> m_colors;
    double stepAt(qreal t) const override;

private:
    boost::random::uniform_01<> m_distribution;
};

class Bit8RGBGradientCacheStategy : public Bit8GradientCacheStategy {
private:
    struct nearColors {
        nearColors() {}
        nearColors(QColor color, const KoColorSpace* m_colorSpace);
        QSharedPointer<KoColor> m_colors[7];
        QColor getColor(QSharedPointer<KoColor> color);
    };

    struct colorComponents {
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
    Bit8RGBGradientCacheStategy(const KoAbstractGradient* gradient, qint32 steps, const KoColorSpace* colorSpace);
    const quint8* cachedAt(qreal t) override;

protected:
    double stepAt(qreal t) const override;

private:
    QMap<qreal, nearColors> m_nearColors;
    boost::random::uniform_smallint<> m_distribution;
    boost::random::uniform_01<> m_distribution2;

};

class NotBit8GradientCacheStategy : public KisGradientCacheStategy {
public:
    NotBit8GradientCacheStategy(const KoAbstractGradient* gradient, qint32 steps, const KoColorSpace* colorSpace);
    const quint8* cachedAt(qreal t) override;

protected:
    double stepAt(qreal t) const override;
    QVector<qreal> m_step;

private:
    QVector<KoColor> m_colors;
    boost::random::uniform_01<> m_distribution;
};

#endif //KIS_GRADIENT_CACHE_STRATEGY_H
