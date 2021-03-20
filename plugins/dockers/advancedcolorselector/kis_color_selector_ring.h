/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_COLOR_SELECTOR_RING_H
#define KIS_COLOR_SELECTOR_RING_H

#include "kis_color_selector_component.h"

#include <QImage>

class KisColorSelectorRing : public KisColorSelectorComponent
{
    Q_OBJECT
public:
    explicit KisColorSelectorRing(KisColorSelector *parent);
    int innerRadius() const;
    void setColor(const KoColor &color) override;
    void setInnerRingRadiusFraction(qreal newFraction);

protected:
    void paint(QPainter*) override;
    KoColor selectColor(int x, int y) override;
    bool containsPointInComponentCoords(int x, int y) const override;

private:
    void paintCache(qreal devicePixelRatioF);
    void colorCache();
    int outerRadius() const;

    QImage m_pixelCache;
    const KoColorSpace* m_cachedColorSpace {0};
    int m_cachedSize {0};
    qreal m_lastHue {0.0};
    QList<QRgb> m_cachedColors;
    qreal m_innerRingRadiusFraction {0.85};

    qreal R {0.0};
    qreal G {0.0};
    qreal B {0.0};
    qreal Gamma {1.0};
};

#endif // KIS_COLOR_SELECTOR_RING_H
