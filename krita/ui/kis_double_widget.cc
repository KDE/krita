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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#include <qhbox.h>
#include <qlayout.h>
#include <qslider.h>

#include <knuminput.h>

#include "kis_double_widget.h"

KisDoubleWidget::KisDoubleWidget(double min, double max, QWidget* parent, const char* name)
  : super(parent, name)
{
	m_spinBox = new KDoubleSpinBox(min, max, 0.05, 0, 2, this, "spinbox");
	connect(m_spinBox, SIGNAL(valueChanged(double)), this, SLOT(setSliderValue(double)));

	m_slider = new QSlider(static_cast<int>(min * 100 + 0.5), static_cast<int>(max * 100 + 0.5), 1, 0, QSlider::Horizontal, this, "sld");
	connect(m_slider, SIGNAL(valueChanged(int)), this, SLOT(sliderValueChanged(int)));

	m_layout = new QHBoxLayout(this, 0, -1, "hbox layout");

	m_layout -> addWidget(m_spinBox);
	m_layout -> addSpacing(5);
	m_layout -> addWidget(m_slider);
}

KisDoubleWidget::~KisDoubleWidget()
{
}

double KisDoubleWidget::value() const
{
	return m_spinBox -> value();
}

void KisDoubleWidget::setValue(double value)
{
	m_slider -> setValue(static_cast<int>(value * 100 + 0.5));
}

void KisDoubleWidget::setRange(double min, double max)
{
	m_spinBox -> setRange(min, max);
	m_slider -> setRange(static_cast<int>(min * 100 + 0.5), static_cast<int>(max * 100 + 0.5));
}

void KisDoubleWidget::setTickmarks(QSlider::TickSetting tickSetting)
{
	m_slider -> setTickmarks(tickSetting);
}

void KisDoubleWidget::setTickInterval(double value)
{
	m_slider -> setTickInterval(static_cast<int>(value * 100 + 0.5));
}

double KisDoubleWidget::tickInterval() const
{
	return m_slider -> tickInterval() / 100.0;
}

void KisDoubleWidget::setSliderValue(double value)
{
	m_slider -> setValue( static_cast<int>(value * 100 + 0.5));
	emit valueChanged(value);
}

void KisDoubleWidget::sliderValueChanged(int value)
{
	m_spinBox -> setValue(value / 100.0);
}

#include "kis_double_widget.moc"

