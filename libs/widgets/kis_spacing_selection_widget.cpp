/*
 *  SPDX-FileCopyrightText: 2014 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_spacing_selection_widget.h"

#include <QCheckBox>
#include <QHBoxLayout>

#include "klocalizedstring.h"

#include "kis_signals_blocker.h"
#include "kis_slider_spin_box.h"



struct KisSpacingSelectionWidget::Private
{
    Private(KisSpacingSelectionWidget *_q)
        : q(_q)
    {
    }

    KisSpacingSelectionWidget *q;

    KisDoubleSliderSpinBox *slider {0};
    QCheckBox *autoButton {0};

    qreal oldSliderValue {0.1};

    void slotSpacingChanged(qreal value);
    void slotAutoSpacing(bool value);
};


KisSpacingSelectionWidget::KisSpacingSelectionWidget(QWidget *parent)
    : QWidget(parent),
      m_d(new Private(this))
{
    m_d->slider = new KisDoubleSliderSpinBox(this);
    m_d->slider->setRange(0.02, 10.0, 2);
    m_d->slider->setExponentRatio(3);
    m_d->slider->setSingleStep(0.01);
    m_d->slider->setValue(0.1);
    m_d->slider->setSizePolicy(QSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Fixed));


    m_d->autoButton = new QCheckBox(this);
    m_d->autoButton->setText(i18nc("@action:button", "Auto"));
    m_d->autoButton->setToolTip(i18nc("@info:tooltip", "In auto mode the spacing of the brush will be calculated automatically depending on its size"));
    m_d->autoButton->setCheckable(true);
    m_d->autoButton->setSizePolicy(QSizePolicy(QSizePolicy::Minimum, QSizePolicy::Fixed));

    QHBoxLayout *layout = new QHBoxLayout(this);
    layout->addWidget(m_d->autoButton);
    layout->addWidget(m_d->slider);
    layout->setMargin(0);

    connect(m_d->slider, SIGNAL(valueChanged(qreal)), SLOT(slotSpacingChanged(qreal)));
    connect(m_d->autoButton, SIGNAL(toggled(bool)), SLOT(slotAutoSpacing(bool)));
}

KisSpacingSelectionWidget::~KisSpacingSelectionWidget()
{
}

qreal KisSpacingSelectionWidget::spacing() const
{
    return autoSpacingActive() ? 0.1 : m_d->slider->value();
}

bool KisSpacingSelectionWidget::autoSpacingActive() const
{
    return m_d->autoButton->isChecked();
}

qreal KisSpacingSelectionWidget::autoSpacingCoeff() const
{
    return autoSpacingActive() ? m_d->slider->value() : 1.0;
}

void KisSpacingSelectionWidget::setSpacing(bool isAuto, qreal spacing)
{
    KisSignalsBlocker b1(m_d->autoButton);
    KisSignalsBlocker b2(m_d->slider);

    m_d->autoButton->setChecked(isAuto);
    m_d->slider->setValue(spacing);
}

void KisSpacingSelectionWidget::Private::slotSpacingChanged(qreal value)
{
    Q_UNUSED(value);
    emit q->sigSpacingChanged();
}

void KisSpacingSelectionWidget::Private::slotAutoSpacing(bool value)
{
    qreal newSliderValue = 0.0;

    if (value) {
        newSliderValue = 1.0;
        oldSliderValue = slider->value();
    } else {
        newSliderValue = oldSliderValue;
    }

    {
        KisSignalsBlocker b(slider);
        slider->setValue(newSliderValue);
    }

    emit q->sigSpacingChanged();
}

#include "moc_kis_spacing_selection_widget.moc"
