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
    const KoColorSpace* m_cachedColorSpace;
    int m_cachedSize;
    qreal m_lastHue;
    QList<QRgb> m_cachedColors;
    qreal m_innerRingRadiusFraction;

    qreal R;
    qreal G;
    qreal B;
    qreal Gamma;
};

#endif // KIS_COLOR_SELECTOR_RING_H
