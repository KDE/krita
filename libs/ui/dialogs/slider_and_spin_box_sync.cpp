/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
