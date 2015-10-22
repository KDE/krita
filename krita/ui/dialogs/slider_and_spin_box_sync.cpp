/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "slider_and_spin_box_sync.h"

#include <QSpinBox>
#include "kis_slider_spin_box.h"

#include "kis_debug.h"
#include "kis_signals_blocker.h"


SliderAndSpinBoxSync::SliderAndSpinBoxSync(KisDoubleSliderSpinBox *slider,
                                           QSpinBox *spinBox,
                                           IntFunction parentValueOp)
    : m_slider(slider),
      m_spinBox(spinBox),
      m_parentValueOp(parentValueOp),
      m_blockUpdates(false)
{
    connect(m_slider, SIGNAL(valueChanged(qreal)), SLOT(sliderChanged(qreal)));
    connect(m_spinBox, SIGNAL(valueChanged(int)), SLOT(spinBoxChanged(int)));
}

SliderAndSpinBoxSync::~SliderAndSpinBoxSync()
{

}

void SliderAndSpinBoxSync::slotParentValueChanged()
{
    int parentValue = m_parentValueOp();

    m_spinBox->setRange(m_slider->minimum() * parentValue / 100,
                        m_slider->maximum() * parentValue / 100);

    sliderChanged(m_slider->value());
}

void SliderAndSpinBoxSync::sliderChanged(qreal value) {
    if (m_blockUpdates) return;
    m_blockUpdates = true;

    m_spinBox->setValue(value * m_parentValueOp() / 100);

    m_blockUpdates = false;
}

void SliderAndSpinBoxSync::spinBoxChanged(int value)
{
    if (m_blockUpdates) return;
    m_blockUpdates = true;

    m_slider->setValue(qreal(value) * 100 / m_parentValueOp());

    m_blockUpdates = false;
}
