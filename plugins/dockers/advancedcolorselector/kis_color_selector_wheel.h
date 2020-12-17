/*
 *  SPDX-FileCopyrightText: 2010 Adam Celarek <kdedev at xibo dot at>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KIS_COLOR_SELECTOR_WHEEL_H
#define KIS_COLOR_SELECTOR_WHEEL_H

typedef unsigned int QRgb;

#include <QColor>
#include <QImage>
#include <QSize>

#include "KoColor.h"
#include "kis_color_selector_component.h"

namespace Acs {
    class PixelCacheRenderer;
}

class KisColorSelectorWheel : public KisColorSelectorComponent
{
    Q_OBJECT
public:
    explicit KisColorSelectorWheel(KisColorSelector *parent);
    void setColor(const KoColor &color) override;

protected:
    KoColor selectColor(int x, int y) override;
    void paint(QPainter*) override;

private:
    friend class Acs::PixelCacheRenderer;
    KoColor colorAt(float x, float y, bool forceValid = false);

private:
    bool allowsColorSelectionAtPoint(const QPoint &pt) const override;
    QPointF m_lastClickPos;
    QImage m_pixelCache;
    QPoint m_pixelCacheOffset;
    qreal R;
    qreal G;
    qreal B;
    qreal Gamma;

    QSize m_renderAreaSize;
    qreal m_renderAreaOffsetX;
    qreal m_renderAreaOffsetY;
    QTransform m_toRenderArea;
};

#endif // KIS_COLOR_SELECTOR_WHEEL_H
