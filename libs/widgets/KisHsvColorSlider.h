/*
 *  SPDX-FileCopyrightText: 2022 Sam Linnfer <littlelightlittlefire@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */
#ifndef _KIS_HSV_COLOR_SLIDER_H_
#define _KIS_HSV_COLOR_SLIDER_H_

#include <kselector.h>
#include "kritawidgets_export.h"
#include "KoColorDisplayRendererInterface.h"

class KoColor;

// Same as the KoColorSlider, except mixing is done with HSV values.
class KRITAWIDGETS_EXPORT KisHsvColorSlider : public KSelector
{
    Q_OBJECT

public:
    enum class MIX_MODE {
        COLOR_SPACE, HSV, HSL, HSI, HSY,
    };

    explicit KisHsvColorSlider(QWidget *parent = 0, KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance());
    explicit KisHsvColorSlider(Qt::Orientation orientation, QWidget *parent = 0, KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance());
    ~KisHsvColorSlider() override;

    void setColors(const KoColor minColor, const KoColor maxColor);
    void setColors(const QColor minColor, const QColor maxColor);
    void setColors(qreal minH, qreal minS, qreal minV, qreal maxH, qreal maxS, qreal maxV);

    void setMixMode(MIX_MODE mode);

    void setCircularHue(bool);

protected:
    void drawContents(QPainter*) override;
    void drawArrow(QPainter *painter, const QPoint &pos) override;

    QPoint calcArrowPos(int value);

    struct Private;
    Private* const d;
};

#endif
