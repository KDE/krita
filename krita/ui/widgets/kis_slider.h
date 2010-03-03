/*
 *  Copyright (c) 2010 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_SLIDER_H
#define KIS_SLIDER_H

#include <QWidget>
#include <QValidator>

#include "krita_export.h"

class QStyleOptionSlider;

class KisSliderPrivate;

/**
 * A tablet-friendly slider implementation
 *
 * Features:
 *
 * - show state as a fill
 * - move the state to the position the user clicks inside the slider
 * - change state by tilt
 * - allow manual input of numbers
 * - no drop-down
 */
class KRITAUI_EXPORT KisSlider : public QWidget
{

    Q_OBJECT

    Q_PROPERTY(double minimum READ minimum WRITE setMinimum)
    Q_PROPERTY(double maximum READ maximum WRITE setMaximum)
    Q_PROPERTY(double singleStep READ singleStep WRITE setSingleStep)
    Q_PROPERTY(double pageStep READ pageStep WRITE setPageStep)
    Q_PROPERTY(double sliderPosition READ sliderPosition WRITE setSliderPosition NOTIFY sliderMoved)
    Q_PROPERTY(bool tracking READ hasTracking WRITE setTracking)
    Q_PROPERTY(bool sliderDown READ isSliderDown WRITE setSliderDown DESIGNABLE false)
    Q_PROPERTY(double value READ value WRITE setValue NOTIFY valueChanged USER true)
    Q_PROPERTY(int decimals READ decimals WRITE setDecimals)

public:

    explicit KisSlider(QWidget* parent = 0);
    ~KisSlider();

    double minimum();
    void setMinimum(double value);

    double maximum();
    void setMaximum(double value);

    void setRange(double minimum, double maximum);

    double singleStep();
    void setSingleStep(double value);

    void setPageStep(double);
    double pageStep() const;

    void setTracking(bool enable);
    bool hasTracking() const;

    void setSliderDown(bool);
    bool isSliderDown() const;

    void setSliderPosition(int);
    int sliderPosition() const;

    int decimals() const;
    void setDecimals(int prec);

    enum SliderAction {
           SliderNoAction,
           SliderSingleStepAdd,
           SliderSingleStepSub,
           SliderPageStepAdd,
           SliderPageStepSub,
           SliderToMinimum,
           SliderToMaximum,
           SliderMove
       };

    double value();

    void triggerAction(SliderAction action);

    QSize sizeHint() const;
    QSize minimumSizeHint() const;


    bool event(QEvent *event);

protected:

    void keyReleaseEvent(QKeyEvent* event);
    void resizeEvent(QResizeEvent *event);
    void paintEvent(QPaintEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void tabletEvent(QTabletEvent* event);
    void timerEvent(QTimerEvent *event);
    void wheelEvent(QWheelEvent *e);
    void changeEvent(QEvent *event);

    virtual QValidator::State validate(QString &input, int &pos) const;
    virtual double valueFromText(const QString &text) const;
    virtual QString textFromValue(double val) const;
    virtual void fixup(QString &str) const;

public Q_SLOTS:

    void setValue(double value);

Q_SIGNALS:

    void valueChanged(double);
    void valueChanged(const QString&);

    void sliderPressed();
    void sliderMoved(int position);
    void sliderReleased();

    void rangeChanged(int min, int max);

    void actionTriggered(int action);

private:

    Q_DISABLE_COPY(KisSlider)
    Q_DECLARE_PRIVATE(KisSlider)
};

#endif // KIS_SLIDER_H
