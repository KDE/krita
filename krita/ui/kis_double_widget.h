/*
 *  kis_double_widget.h - part of Krita
 *
 *  Copyright (c) 1999 Carsten Pfeiffer <pfeiffer@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
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

#ifndef KIS_DOUBLE_WIDGET_H
#define KIS_DOUBLE_WIDGET_H

#include <qwidget.h>
#include <qslider.h>

class QHBoxLayout;
class KDoubleSpinBox;

class KisDoubleWidget : public QWidget
{
    Q_OBJECT

    typedef QWidget super;
public:
    KisDoubleWidget(QWidget* parent = 0, const char* name = 0);
    KisDoubleWidget(double min, double max, QWidget* parent = 0, const char* name = 0);
    ~KisDoubleWidget();

    double value() const;
    void setRange(double min, double max);

    void setTickmarks(QSlider::TickPosition tickMarks);
    void setTickInterval(double tickInterval);
    double tickInterval() const;

    void setPrecision(int precision);
    void setLineStep(double step);
    void setPageStep(double step);

    void setTracking(bool tracking);
    bool hasTracking() const;

signals:
    void valueChanged(double);
    void sliderPressed();
    void sliderReleased();

public slots:
    void setValue(double value);

protected slots:
    void setSliderValue(double);
    void sliderValueChanged(int);

private:
    void init(double min, double max);

protected:
    QHBoxLayout* m_layout;
    QSlider* m_slider;
    KDoubleSpinBox *m_spinBox;
};

#endif // KIS_DOUBLE_WIDGET_H

