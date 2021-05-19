/*
 *  SPDX-FileCopyrightText: 2016 Jouni Pentikäinen <joupent@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef _KIS_ANIMATION_CURVES_VALUE_RULER_H
#define _KIS_ANIMATION_CURVES_VALUE_RULER_H

#include <QHeaderView>
#include "kis_debug.h"
#include <QtMath>

const int UNIT_SIZE_PIXELS = 32;

class KisAnimCurvesValuesHeader : public QHeaderView
{
    Q_OBJECT

public:
    KisAnimCurvesValuesHeader(QWidget *parent);
    ~KisAnimCurvesValuesHeader() override;

    void setScale(qreal scale);
    qreal scale() const;

    void setValueOffset(qreal valueOffset);
    qreal valueOffset() const;

    qreal step() const;

    /**
     * @brief valueToWidgetOffset
     * @param value Y value in ruler units.
     * @return Pixel offset relative to widget.
     */
    qreal valueToWidget(qreal value) const;
    qreal widgetToValue(qreal position) const;

    inline qreal visibleValueMax() const {
        return widgetToValue(rect().top());
    }

    inline qreal visibleValueMin() const {
        return widgetToValue(rect().bottom());
    }

    qreal visibleValueDifference() const;

    void zoomToFitRange(qreal min, qreal max);

    inline qreal firstVisibleStep() const {
        const qreal valueStep = step();
        return qCeil(valueOffset() / valueStep) * valueStep;
    }

    inline qreal pixelsToValueOffset(const int pixels) const {
        return (qreal(pixels) * -1 / scaledUnit());
    }

    inline int valueToPixelOffset(const qreal value) const {
        return (value * scaledUnit()) / -1;
    }

    QSize sizeHint() const override {return QSize(64, 0);}
    void paintEvent(QPaintEvent *e) override;
    
Q_SIGNALS:
    void scaleChanged(qreal scale);
    
protected:
    virtual void mouseMoveEvent(QMouseEvent* mouseEvent) override;
    virtual void mousePressEvent(QMouseEvent* mouseEvent) override;

private:
    inline qreal scaledUnit() const {
        return UNIT_SIZE_PIXELS * scale();
    }

    inline qreal pixelsPerStep(const qreal step) const {
        return step * scaledUnit();
    }

    inline qreal roundUpPower10(qreal value) const {
        return qPow(10.0, (qreal)qCeil(log10(value))) / 10.0;
    }

    inline qreal roundDownPower10(qreal value) const {
        return qPow(10.0, (qreal)qFloor(log10(value))) / 10.0;
    }

    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif
