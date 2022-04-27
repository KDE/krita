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
    explicit KisHsvColorSlider(QWidget *parent = 0, KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance());
    explicit KisHsvColorSlider(Qt::Orientation orientation, QWidget *parent = 0, KoColorDisplayRendererInterface *displayRenderer = KoDumbColorDisplayRenderer::instance());
    ~KisHsvColorSlider() override;

    void setColors(const KoColor& minColor, const KoColor& maxColor);
    KoColor currentColor() const;

    void setCircularHue(bool);

protected:
    void drawContents(QPainter*) override;
    void drawArrow(QPainter *painter, const QPoint &pos) override;

    struct Private;
    Private* const d;

private:
    // Determine the commonly used hsv values for calulations.
    void baseRange(qreal &minH, qreal &minS, qreal &minV, qreal &dH, qreal &dS, qreal &dV) const;
};

#endif
