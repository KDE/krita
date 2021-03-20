/*
 *  kis_double_widget.cc - part of Krita
 *
 *  SPDX-FileCopyrightText: 1999 Carsten Pfeiffer <pfeiffer@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "widgets/kis_double_widget.h"

#include <QLayout>
#include <QSlider>
#include <QHBoxLayout>
#include <QDoubleSpinBox>

#include "kis_double_parse_spin_box.h"

KisDoubleWidget::KisDoubleWidget(QWidget* parent, const char* name)
        : QWidget(parent)
{
    setObjectName(name);
    init(0, 1);
}

KisDoubleWidget::KisDoubleWidget(double min, double max, QWidget* parent, const char* name)
        : QWidget(parent)
{
    setObjectName(name);
    init(min, max);
}

KisDoubleWidget::~KisDoubleWidget()
{
}

void KisDoubleWidget::init(double min, double max)
{
    m_spinBox = new KisDoubleParseSpinBox(this);
    m_spinBox->setMinimum(min);
    m_spinBox->setMaximum(max);
    m_spinBox->setSingleStep(0.05);
    m_spinBox->setValue(0);
    m_spinBox->setObjectName("spinbox");
    connect(m_spinBox, SIGNAL(valueChanged(double)), this, SLOT(setSliderValue(double)));

    m_slider = new QSlider(Qt::Horizontal, this);
    m_slider->setMinimum(static_cast<int>(min * 100 + 0.5));
    m_slider->setMaximum(static_cast<int>(max * 100 + 0.5));
    m_slider->setPageStep(1);
    m_slider->setValue(0);
    m_slider->setObjectName("slider");

    connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));
    connect(m_slider, SIGNAL(sliderPressed()), SIGNAL(sliderPressed()));
    connect(m_slider, SIGNAL(sliderReleased()), SIGNAL(sliderReleased()));

    m_layout = new QHBoxLayout(this);
    m_layout->setObjectName("hbox layout");
    m_layout->setMargin(0);
    m_layout->setSpacing(0);

    m_layout->addWidget(m_slider);
    m_layout->addSpacing(5);
    m_layout->addWidget(m_spinBox);
    m_layout->addItem(new QSpacerItem(5, 1, QSizePolicy::Expanding, QSizePolicy::Minimum));
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

void KisDoubleWidget::setTickPosition(QSlider::TickPosition tickPosition)
{
    m_slider->setTickPosition(tickPosition);
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
    m_spinBox->setDecimals(precision);
}

void KisDoubleWidget::setSingleStep(double step)
{
    m_spinBox->setSingleStep(step);
    m_slider->setSingleStep(static_cast<int>(step * 100));
}

void KisDoubleWidget::setPageStep(double step)
{
    m_slider->setPageStep(static_cast<int>(step * 100));
}

void KisDoubleWidget::setTracking(bool tracking)
{
    m_slider->setTracking(tracking);
}

bool KisDoubleWidget::hasTracking() const
{
    return m_slider->hasTracking();
}


