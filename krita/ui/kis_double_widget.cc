/*
 *  kis_double_widget.cc - part of Krita
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

#include <qhbox.h>
#include <qlayout.h>
#include <qslider.h>

#include <knuminput.h>

#include "kis_double_widget.h"

KisDoubleWidget::KisDoubleWidget(QWidget* parent, const char* name)
  : super(parent, name)
{
    init(0, 1);
}

KisDoubleWidget::KisDoubleWidget(double min, double max, QWidget* parent, const char* name)
  : super(parent, name)
{
    init(min, max);
}

KisDoubleWidget::~KisDoubleWidget()
{
}

void KisDoubleWidget::init(double min, double max)
{
    m_spinBox = new KDoubleSpinBox(min, max, 0.05, 0, 2, this, "spinbox");
    connect(m_spinBox, SIGNAL(valueChanged(double)), this, SLOT(setSliderValue(double)));

    m_slider = new QSlider(static_cast<int>(min * 100 + 0.5), static_cast<int>(max * 100 + 0.5), 1, 0, Qt::Horizontal, this, "sld");
    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));
    connect(m_slider, SIGNAL(sliderPressed()), SIGNAL(sliderPressed()));
    connect(m_slider, SIGNAL(sliderReleased()), SIGNAL(sliderReleased()));

    m_layout = new QHBoxLayout(this, 0, -1, "hbox layout");

    m_layout->addWidget(m_slider);
    m_layout->addSpacing(5);
    m_layout->addWidget(m_spinBox);
    m_layout->addItem(new QSpacerItem(5,1,QSizePolicy::Expanding, QSizePolicy::Minimum));
}

double KisDoubleWidget::value() const
{
    return m_spinBox->value();
}

void KisDoubleWidget::setValue(double value)
{
    int intValue;

    if (value < 0) {
        intValue = static_cast<int>(value * 100 - 0.5);
    } else {
        intValue = static_cast<int>(value * 100 + 0.5);
    }
    m_slider->setValue(intValue);
}

void KisDoubleWidget::setRange(double min, double max)
{
    m_spinBox->setRange(min, max);
    m_slider->setRange(static_cast<int>(min * 100 + 0.5), static_cast<int>(max * 100 + 0.5));
}

void KisDoubleWidget::setTickmarks(QSlider::TickSetting tickSetting)
{
    m_slider->setTickmarks(tickSetting);
}

void KisDoubleWidget::setTickInterval(double value)
{
    m_slider->setTickInterval(static_cast<int>(value * 100 + 0.5));
}

double KisDoubleWidget::tickInterval() const
{
    return m_slider->tickInterval() / 100.0;
}

void KisDoubleWidget::setSliderValue(double value)
{
    int intValue;

    if (value < 0) {
        intValue = static_cast<int>(value * 100 - 0.5);
    } else {
        intValue = static_cast<int>(value * 100 + 0.5);
    }
    m_slider->setValue(intValue);
    emit valueChanged(value);
}

void KisDoubleWidget::sliderValueChanged(int value)
{
    m_spinBox->setValue(value / 100.0);
}

void KisDoubleWidget::setPrecision(int precision)
{
    m_spinBox->setPrecision(precision);
}

void KisDoubleWidget::setLineStep(double step)
{
    m_spinBox->setLineStep(step);
    m_slider->setLineStep(static_cast<int>(step * 100));
}

void KisDoubleWidget::setPageStep(double step)
{
    m_slider->setPageStep(static_cast<int>(step * 100));
}

void KisDoubleWidget::setTracking(bool tracking)
{
    m_slider->setTracking(tracking);
}

bool KisDoubleWidget::tracking() const
{
    return m_slider->tracking();
}

#include "kis_double_widget.moc"

