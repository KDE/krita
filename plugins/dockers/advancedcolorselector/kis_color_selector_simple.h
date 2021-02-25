/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_COLOR_SELECTOR_SIMPLE_H
#define KIS_COLOR_SELECTOR_SIMPLE_H

typedef unsigned int QRgb;

#include <QColor>
#include <QImage>

#include "KoColor.h"
#include "kis_color_selector_component.h"

namespace Acs {
    class PixelCacheRenderer;
}


class KisColorSelectorSimple : public KisColorSelectorComponent
{
Q_OBJECT
public:
    explicit KisColorSelectorSimple(KisColorSelector *parent);
    void setColor(const KoColor &color) override;

protected:
    void paint(QPainter*) override;
    KoColor selectColor(int x, int y) override;

private:
    friend class Acs::PixelCacheRenderer;
    KoColor colorAt(float x, float y);

private:
    QPointF m_lastClickPos;
    QImage m_pixelCache;
    qreal R {0.0};
    qreal G {0.0};
    qreal B {0.0};
    qreal Gamma {0.0};
};

#endif
