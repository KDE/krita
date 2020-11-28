/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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

void KisMultipliersDoubleSliderSpinBox::setPrefix(const QString& prefix)
{
    d->form.sliderSpinBox->setPrefix(prefix);
}

void KisMultipliersDoubleSliderSpinBox::setSuffix(const QString& suffix)
{
    d->form.sliderSpinBox->setSuffix(suffix);
}

void KisMultipliersDoubleSliderSpinBox::setBlockUpdateSignalOnDrag(bool block)
{
    d->form.sliderSpinBox->setBlockUpdateSignalOnDrag(block);
}

void KisMultipliersDoubleSliderSpinBox::setSingleStep(qreal value)
{
    d->form.sliderSpinBox->setSingleStep(value);
}

QSize KisMultipliersDoubleSliderSpinBox::sizeHint() const
{
    QSize sliderhint = d->form.sliderSpinBox->sizeHint();
    QSize comboboxhint = d->form.comboBox->sizeHint();
    sliderhint.setWidth(sliderhint.width() + comboboxhint.width() + 10);
    sliderhint.setHeight(qMax(sliderhint.height(), comboboxhint.height()));
    return sliderhint;
}

QSize KisMultipliersDoubleSliderSpinBox::minimumSizeHint() const
{
    return sizeHint();
}

QSize KisMultipliersDoubleSliderSpinBox::minimumSize() const
{
    return QWidget::minimumSize();
}

#include "moc_kis_multipliers_double_slider_spinbox.cpp"
