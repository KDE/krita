/* This file is part of the KDE project
 * Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_multipliers_double_slider_spinbox.h"
#include "kis_multipliers_double_slider_spinbox_p.h"

#include "ui_wdgmultipliersdoublesliderspinbox.h"

#include "kis_debug.h"

qreal KisMultipliersDoubleSliderSpinBox::Private::currentMultiplier()
{
    return form.comboBox->itemData(form.comboBox->currentIndex()).toDouble();
}

void KisMultipliersDoubleSliderSpinBox::Private::updateRange()
{
    qreal m = currentMultiplier();
    form.sliderSpinBox->setRange(m * min, m * max, decimals);
}

KisMultipliersDoubleSliderSpinBox::KisMultipliersDoubleSliderSpinBox(QWidget* _parent)
    : QWidget(_parent)
    , d(new Private)
{
    d->form.setupUi(this);
    addMultiplier(1.0);
    connect(d->form.sliderSpinBox, SIGNAL(valueChanged(qreal)), SIGNAL(valueChanged(qreal)));
    connect(d->form.comboBox, SIGNAL(activated(int)), SLOT(updateRange()));
}

KisMultipliersDoubleSliderSpinBox::~KisMultipliersDoubleSliderSpinBox()
{
    delete d;
}

void KisMultipliersDoubleSliderSpinBox::addMultiplier(double v)
{
  d->form.comboBox->addItem(i18n("x%1", v), v);
}

void KisMultipliersDoubleSliderSpinBox::setRange(qreal minimum, qreal maximum, int decimals)
{
    d->min = minimum;
    d->max = maximum;
    d->decimals = decimals;
    d->updateRange();
}

void KisMultipliersDoubleSliderSpinBox::setPrefix(QString prefixText)
{
   d->form.sliderSpinBox->setPrefix(prefixText);
}

qreal KisMultipliersDoubleSliderSpinBox::value()
{
    return d->form.sliderSpinBox->value();
}

void KisMultipliersDoubleSliderSpinBox::setValue(qreal value)
{
    qreal m = d->currentMultiplier();

    if (value < m * d->min || value > m * d->max) {
        for(int i = 0; i < d->form.comboBox->count(); ++i) {
            qreal m = d->form.comboBox->itemData(i).toDouble();
            if (value >= m * d->min && value <= m * d->max) {
                d->form.comboBox->setCurrentIndex(i);
                d->updateRange();
                break;
            }
        }
    }

    d->form.sliderSpinBox->setValue(value);
}

void KisMultipliersDoubleSliderSpinBox::setExponentRatio(qreal dbl)
{
    d->form.sliderSpinBox->setExponentRatio(dbl);
}

#include "moc_kis_multipliers_double_slider_spinbox.cpp"
