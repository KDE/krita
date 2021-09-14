/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2010 Justin Noel <justin@ics.com>
 * SPDX-FileCopyrightText: 2010 Cyrille Berger <cberger@cberger.net>
 * SPDX-FileCopyrightText: 2015 Moritz Molch <kde@moritzmolch.de>
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include <kis_slider_spin_box_p.h>
#include "kis_slider_spin_box.h"

KisSliderSpinBox::KisSliderSpinBox(QWidget * parent)
    : KisIntParseSpinBox(parent)
    , d(new KisSliderSpinBoxPrivate<KisSliderSpinBox, KisIntParseSpinBox>(this))
{}

KisSliderSpinBox::~KisSliderSpinBox()
{}

int KisSliderSpinBox::fastSliderStep() const
{
    return d->fastSliderStep();
}

int KisSliderSpinBox::softMinimum() const
{
    return d->softMinimum();
}

int KisSliderSpinBox::softMaximum() const
{
    return d->softMaximum();
}

bool KisSliderSpinBox::isDragging() const
{
    return d->isDragging();
}

QSize KisSliderSpinBox::sizeHint() const
{
    return d->sizeHint();
}

QSize KisSliderSpinBox::minimumSizeHint() const
{
    return d->minimumSizeHint();
}

void KisSliderSpinBox::setValue(int newValue)
{
    d->setValue(newValue);
}

void KisSliderSpinBox::setRange(int newMinimum, int newMaximum, bool computeNewFastSliderStep)
{
    d->setRange(newMinimum, newMaximum, computeNewFastSliderStep);
}

void KisSliderSpinBox::setMinimum(int newMinimum, bool computeNewFastSliderStep)
{
    setRange(newMinimum, maximum(), computeNewFastSliderStep);
}

void KisSliderSpinBox::setMaximum(int newMaximum, bool computeNewFastSliderStep)
{
    setRange(minimum(), newMaximum, computeNewFastSliderStep);
}

void KisSliderSpinBox::setExponentRatio(double newExponentRatio)
{
    d->setExponentRatio(newExponentRatio);
}

void KisSliderSpinBox::setBlockUpdateSignalOnDrag(bool newBlockUpdateSignalOnDrag)
{
    d->setBlockUpdateSignalOnDrag(newBlockUpdateSignalOnDrag);
}

void KisSliderSpinBox::setFastSliderStep(int newFastSliderStep)
{
    d->setFastSliderStep(newFastSliderStep);
}

void KisSliderSpinBox::setPageStep(int value)
{
    Q_UNUSED(value);
}

void KisSliderSpinBox::setSoftRange(int newSoftMinimum, int newSoftMaximum)
{
    d->setSoftRange(newSoftMinimum, newSoftMaximum);
}

void KisSliderSpinBox::setSoftMinimum(int newSoftMinimum)
{
    setSoftRange(newSoftMinimum, d->softMaximum());
}

void KisSliderSpinBox::setSoftMaximum(int newSoftMaximum)
{
    setSoftRange(d->softMinimum(), newSoftMaximum);
}

void KisSliderSpinBox::setInternalValue(int newValue, bool newBlockUpdateSignal)
{
    d->setValue(newValue, newBlockUpdateSignal);
}

void KisSliderSpinBox::setPrivateValue(int newValue)
{
    setValue(newValue);
}

KisDoubleSliderSpinBox::KisDoubleSliderSpinBox(QWidget * parent)
    : KisDoubleParseSpinBox(parent)
    , d(new KisSliderSpinBoxPrivate<KisDoubleSliderSpinBox, KisDoubleParseSpinBox>(this))
{}

KisDoubleSliderSpinBox::~KisDoubleSliderSpinBox()
{}

double KisDoubleSliderSpinBox::fastSliderStep() const
{
    return d->fastSliderStep();
}

double KisDoubleSliderSpinBox::softMinimum() const
{
    return d->softMinimum();
}

double KisDoubleSliderSpinBox::softMaximum() const
{
    return d->softMaximum();
}

bool KisDoubleSliderSpinBox::isDragging() const
{
    return d->isDragging();
}

QSize KisDoubleSliderSpinBox::sizeHint() const
{
    return d->sizeHint();
}

QSize KisDoubleSliderSpinBox::minimumSizeHint() const
{
    return d->minimumSizeHint();
}

void KisDoubleSliderSpinBox::setValue(double newValue)
{
    d->setValue(newValue);
}

void KisDoubleSliderSpinBox::setRange(double newMinimum, double newMaximum, int newNumberOfecimals, bool computeNewFastSliderStep)
{
    d->setRange(newMinimum, newMaximum, newNumberOfecimals, computeNewFastSliderStep);
}

void KisDoubleSliderSpinBox::setMinimum(double newMinimum, bool computeNewFastSliderStep)
{
    setRange(newMinimum, maximum(), decimals(), computeNewFastSliderStep);
}

void KisDoubleSliderSpinBox::setMaximum(double newMaximum, bool computeNewFastSliderStep)
{
    setRange(minimum(), newMaximum, decimals(), computeNewFastSliderStep);
}

void KisDoubleSliderSpinBox::setExponentRatio(double newExponentRatio)
{
    Q_ASSERT(newExponentRatio > 0.0);
    d->setExponentRatio(newExponentRatio);
}

void KisDoubleSliderSpinBox::setBlockUpdateSignalOnDrag(bool newBlockUpdateSignalOnDrag)
{
    d->setBlockUpdateSignalOnDrag(newBlockUpdateSignalOnDrag);
}

void KisDoubleSliderSpinBox::setFastSliderStep(double newFastSliderStep)
{
    d->setFastSliderStep(newFastSliderStep);
}

void KisDoubleSliderSpinBox::setSoftRange(double newSoftMinimum, double newSoftMaximum)
{
    d->setSoftRange(newSoftMinimum, newSoftMaximum);
}

void KisDoubleSliderSpinBox::setSoftMinimum(double newSoftMinimum)
{
    setSoftRange(newSoftMinimum, d->softMaximum());
}

void KisDoubleSliderSpinBox::setSoftMaximum(double newSoftMaximum)
{
    setSoftRange(d->softMinimum(), newSoftMaximum);
}

void KisDoubleSliderSpinBox::setInternalValue(double newValue, bool newBlockUpdateSignal)
{
    d->setValue(newValue, newBlockUpdateSignal);
}

void KisDoubleSliderSpinBox::setPrivateValue(double newValue)
{
    setValue(newValue);
}
