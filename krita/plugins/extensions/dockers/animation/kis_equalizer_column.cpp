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

#include "kis_equalizer_column.h"

#include <QVBoxLayout>
#include <QFontMetrics>
#include <QApplication>

#include "kis_debug.h"

#include "kis_equalizer_slider.h"
#include "kis_equalizer_button.h"

#include "kis_signals_blocker.h"

struct KisEqualizerColumn::Private
{
    KisEqualizerButton *stateButton;
    KisEqualizerSlider *mainSlider;
    int id;
};


KisEqualizerColumn::KisEqualizerColumn(QWidget *parent, int id, const QString &title)
    : QWidget(parent),
      m_d(new Private)
{
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_d->id = id;

    m_d->stateButton = new KisEqualizerButton(this);
    m_d->stateButton->setText(title);
    m_d->stateButton->setCheckable(true);
    m_d->stateButton->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    m_d->mainSlider = new KisEqualizerSlider(this);
    m_d->mainSlider->setRange(0, 100);
    m_d->mainSlider->setSingleStep(5);
    m_d->mainSlider->setPageStep(10);
    m_d->mainSlider->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    QVBoxLayout *vbox = new QVBoxLayout(this);

    vbox->setSpacing(0);
    vbox->setContentsMargins(0,0,0,0);

    vbox->addWidget(m_d->stateButton, 0);
    vbox->addWidget(m_d->mainSlider, 1);

    setLayout(vbox);

    connect(m_d->stateButton, SIGNAL(toggled(bool)),
            SLOT(slotButtonChanged(bool)));

    connect(m_d->mainSlider, SIGNAL(valueChanged(int)),
            SLOT(slotSliderChanged(int)));
}

KisEqualizerColumn::~KisEqualizerColumn()
{
}

void KisEqualizerColumn::setRightmost(bool value)
{
    m_d->stateButton->setRightmost(value);
    m_d->mainSlider->setRightmost(value);
}

void KisEqualizerColumn::slotSliderChanged(int value)
{
    KisSignalsBlocker b(m_d->stateButton);
    m_d->stateButton->setChecked(value > 0);

    emit sigColumnChanged(m_d->id, m_d->stateButton->isChecked(), m_d->mainSlider->value());

}

void KisEqualizerColumn::slotButtonChanged(bool value)
{
    Q_UNUSED(value);
    emit sigColumnChanged(m_d->id, m_d->stateButton->isChecked(), m_d->mainSlider->value());
}


int KisEqualizerColumn::value() const
{
    return m_d->mainSlider->value();
}

void KisEqualizerColumn::setValue(int value)
{
    m_d->mainSlider->setValue(value);
}

bool KisEqualizerColumn::state() const
{
    return m_d->stateButton->isChecked();
}

void KisEqualizerColumn::setState(bool value)
{
    m_d->stateButton->setChecked(value);
}


