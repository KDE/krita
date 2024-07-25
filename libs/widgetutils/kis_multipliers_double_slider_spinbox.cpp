/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "kis_multipliers_double_slider_spinbox.h"
#include "kis_multipliers_double_slider_spinbox_p.h"
#include <QHBoxLayout>
#include <kis_debug.h>
#include <klocalizedstring.h>

qreal KisMultipliersDoubleSliderSpinBox::Private::currentMultiplier()
{
    return cmbMultiplier->itemData(cmbMultiplier->currentIndex()).toDouble();
}

void KisMultipliersDoubleSliderSpinBox::Private::updateRange()
{
    qreal m = currentMultiplier();
    slider->setRange(m * min, m * max, decimals);
}

KisMultipliersDoubleSliderSpinBox::KisMultipliersDoubleSliderSpinBox(QWidget* _parent)
    : QWidget(_parent)
    , d(new Private)
{
    QHBoxLayout *l = new QHBoxLayout(this);
    l->setContentsMargins(0, 0, 0, 0);

    d->slider = new KisDoubleSliderSpinBox(this);
    d->slider->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    l->addWidget(d->slider);

    d->cmbMultiplier = new QComboBox(this);
    d->cmbMultiplier->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
    l->addWidget(d->cmbMultiplier);

    addMultiplier(1.0);
    connect(d->slider, SIGNAL(valueChanged(qreal)), SIGNAL(valueChanged(qreal)));
    connect(d->cmbMultiplier, SIGNAL(activated(int)), SLOT(updateRange()));
}

KisMultipliersDoubleSliderSpinBox::~KisMultipliersDoubleSliderSpinBox()
{
    delete d;
}

void KisMultipliersDoubleSliderSpinBox::addMultiplier(double v)
{
  d->cmbMultiplier->addItem(i18n("x%1", v), v);
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
    return d->slider->value();
}

void KisMultipliersDoubleSliderSpinBox::setValue(qreal value)
{
    qreal m = d->currentMultiplier();

    if (value < m * d->min || value > m * d->max) {
        for(int i = 0; i < d->cmbMultiplier->count(); ++i) {
            qreal m = d->cmbMultiplier->itemData(i).toDouble();
            if (value >= m * d->min && value <= m * d->max) {
                d->cmbMultiplier->setCurrentIndex(i);
                d->updateRange();
                break;
            }
        }
    }

    d->slider->setValue(value);
}

void KisMultipliersDoubleSliderSpinBox::setExponentRatio(qreal dbl)
{
    d->slider->setExponentRatio(dbl);
}

void KisMultipliersDoubleSliderSpinBox::setPrefix(const QString& prefix)
{
    d->slider->setPrefix(prefix);
}

void KisMultipliersDoubleSliderSpinBox::setSuffix(const QString& suffix)
{
    d->slider->setSuffix(suffix);
}

void KisMultipliersDoubleSliderSpinBox::setBlockUpdateSignalOnDrag(bool block)
{
    d->slider->setBlockUpdateSignalOnDrag(block);
}

void KisMultipliersDoubleSliderSpinBox::setSingleStep(qreal value)
{
    d->slider->setSingleStep(value);
}

QSize KisMultipliersDoubleSliderSpinBox::sizeHint() const
{
    QSize sliderhint = d->slider->sizeHint();
    QSize comboboxhint = d->cmbMultiplier->sizeHint();
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
