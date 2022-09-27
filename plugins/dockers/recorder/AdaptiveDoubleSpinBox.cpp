/*
 *  SPDX-FileCopyrightText: 2022 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "AdaptiveDoubleSpinBox.h"


class AdaptiveDoubleSpinBoxPrivate
{
public:
    double smallStep = .1;
    double largeStep = 1.;
    double switchValue = 1.;
};


AdaptiveDoubleSpinBox::AdaptiveDoubleSpinBox(QWidget *parent)
    : QDoubleSpinBox(parent)
    , d(new AdaptiveDoubleSpinBoxPrivate)
{
    connect(this, SIGNAL(valueChanged(double)), this, SLOT(updateSingleStep(double)));
}

AdaptiveDoubleSpinBox::~AdaptiveDoubleSpinBox()
{
    delete d;
}

double AdaptiveDoubleSpinBox::smallStep() const
{
    return d->smallStep;
}

void AdaptiveDoubleSpinBox::setSmallStep(double value)
{
    d->smallStep = value;
    if (this->value() < d->switchValue) {
        setSingleStep(value);
    }
}

double AdaptiveDoubleSpinBox::largeStep() const
{
    return d->largeStep;
}

void AdaptiveDoubleSpinBox::setLargeStep(double value)
{
    d->largeStep = value;
    if (this->value() >= d->switchValue) {
        setSingleStep(value);
    }
}

double AdaptiveDoubleSpinBox::switchValue() const
{
    return d->switchValue;
}

void AdaptiveDoubleSpinBox::setSwitchValue(double value)
{
    d->switchValue = value;
    if (this->value() >= d->switchValue) {
        setSingleStep(d->largeStep);
    } else {
        setSingleStep(d->smallStep);
    }
}

void AdaptiveDoubleSpinBox::stepBy(int steps)
{
    const double oldValue = value();
    const double newValue = oldValue + steps * singleStep();

    if (oldValue >= d->switchValue && newValue < d->switchValue) {
        QSignalBlocker blocker(this);
        setValue(d->switchValue - d->smallStep);
        setSingleStep(d->smallStep);
    } else if (oldValue < d->switchValue && newValue >= d->switchValue) {
        QSignalBlocker blocker(this);
        setValue(d->switchValue);
        setSingleStep(d->largeStep);
    } else {
        QDoubleSpinBox::stepBy(steps);
    }
}

void AdaptiveDoubleSpinBox::updateSingleStep(double val)
{
    if (val < d->switchValue) {
        if (singleStep() != d->smallStep) {
            setSingleStep(d->smallStep);
        }
    } else if (val >= d->switchValue) {
        if (singleStep() != d->largeStep) {
            setSingleStep(d->largeStep);
        }
    }
}
