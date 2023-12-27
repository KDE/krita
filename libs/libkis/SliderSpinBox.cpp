/*
 * SPDX-FileCopyrightText: 2010 Justin Noel <justin@ics.com>
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "SliderSpinBox.h"
#include "kis_debug.h"

struct SliderSpinBox::Private {
    Private() {}

    KisSliderSpinBox *widget;
};

SliderSpinBox::SliderSpinBox()
    : IntParseSpinBox()
    , d(new Private)
{
    d->widget = new KisSliderSpinBox();

    // Forward KisSliderSpinBox::draggingFinished to SliderSpinBox::draggingFinished
    connect(d->widget, SIGNAL(draggingFinished()), this, SIGNAL(draggingFinished()));
}

SliderSpinBox::~SliderSpinBox()
{
    delete d;
}

QWidget* SliderSpinBox::widget() const
{
    return d->widget;
}
int SliderSpinBox::fastSliderStep() const
{
    return d->widget->fastSliderStep();
}

int SliderSpinBox::softMinimum() const
{
    return d->widget->softMinimum();
}

int SliderSpinBox::softMaximum() const
{
    return d->widget->softMaximum();
}

bool SliderSpinBox::isDragging() const
{
    return d->widget->isDragging();
}

void SliderSpinBox::setValue(int newValue)
{
    d->widget->setValue(newValue);
}

void SliderSpinBox::setRange(int newMinimum, int newMaximum, bool computeNewFastSliderStep)
{
    d->widget->setRange(newMinimum, newMaximum, computeNewFastSliderStep);
}

void SliderSpinBox::setMinimum(int newMinimum, bool computeNewFastSliderStep)
{
    d->widget->setMinimum(newMinimum, computeNewFastSliderStep);
}

void SliderSpinBox::setMaximum(int newMaximum, bool computeNewFastSliderStep)
{
    d->widget->setMaximum(newMaximum, computeNewFastSliderStep);
}

void SliderSpinBox::setExponentRatio(double newExponentRatio)
{
    if (newExponentRatio <= 0.0) {
        dbgScript << "Script using SliderSpinbox.setExponentRatio passed value <= 0.0 (" << newExponentRatio << "), ignoring.";
        return;
    }
    d->widget->setExponentRatio(newExponentRatio);
}

void SliderSpinBox::setBlockUpdateSignalOnDrag(bool newBlockUpdateSignalOnDrag)
{
    d->widget->setBlockUpdateSignalOnDrag(newBlockUpdateSignalOnDrag);
}

void SliderSpinBox::setFastSliderStep(int newFastSliderStep)
{
    d->widget->setFastSliderStep(newFastSliderStep);
}

void SliderSpinBox::setSoftRange(int newSoftMinimum, int newSoftMaximum)
{
    d->widget->setSoftRange(newSoftMinimum, newSoftMaximum);
}

void SliderSpinBox::setSoftMinimum(int newSoftMinimum)
{
    d->widget->setSoftMinimum(newSoftMinimum);
}

void SliderSpinBox::setSoftMaximum(int newSoftMaximum)
{
    d->widget->setSoftMaximum(newSoftMaximum);
}

struct DoubleSliderSpinBox::Private {
    Private() {}

    KisDoubleSliderSpinBox *widget;
};

DoubleSliderSpinBox::DoubleSliderSpinBox()
    : DoubleParseSpinBox()
    , d(new Private)
{
    d->widget = new KisDoubleSliderSpinBox();

    // Forward KisDoubleSliderSpinBox::draggingFinished to DoubleSliderSpinBox::draggingFinished
    connect(d->widget, SIGNAL(draggingFinished()), this, SIGNAL(draggingFinished()));
}

DoubleSliderSpinBox::~DoubleSliderSpinBox()
{
    delete d;
}

QWidget* DoubleSliderSpinBox::widget() const
{
    return d->widget;
}

double DoubleSliderSpinBox::fastSliderStep() const
{
    return d->widget->fastSliderStep();
}

double DoubleSliderSpinBox::softMinimum() const
{
    return d->widget->softMinimum();
}

double DoubleSliderSpinBox::softMaximum() const
{
    return d->widget->softMaximum();
}

bool DoubleSliderSpinBox::isDragging() const
{
    return d->widget->isDragging();
}

void DoubleSliderSpinBox::setValue(double newValue)
{
    d->widget->setValue(newValue);
}

void DoubleSliderSpinBox::setRange(double newMinimum, double newMaximum, int newNumberOfDecimals, bool computeNewFastSliderStep)
{
    d->widget->setRange(newMinimum, newMaximum, newNumberOfDecimals, computeNewFastSliderStep);
}

void DoubleSliderSpinBox::setMinimum(double newMinimum, bool computeNewFastSliderStep)
{
    d->widget->setMinimum(newMinimum, computeNewFastSliderStep);
}

void DoubleSliderSpinBox::setMaximum(double newMaximum, bool computeNewFastSliderStep)
{
    d->widget->setMaximum(newMaximum, computeNewFastSliderStep);
}

void DoubleSliderSpinBox::setExponentRatio(double newExponentRatio)
{
    if (newExponentRatio <= 0.0) {
        dbgScript << "Script using DoubleSliderSpinbox.setExponentRatio passed value <= 0.0 (" << newExponentRatio << "), ignoring.";
        return;
    }
    d->widget->setExponentRatio(newExponentRatio);
}

void DoubleSliderSpinBox::setBlockUpdateSignalOnDrag(bool newBlockUpdateSignalOnDrag)
{
    d->widget->setBlockUpdateSignalOnDrag(newBlockUpdateSignalOnDrag);
}

void DoubleSliderSpinBox::setFastSliderStep(double newFastSliderStep)
{
    d->widget->setFastSliderStep(newFastSliderStep);
}

void DoubleSliderSpinBox::setSoftRange(double newSoftMinimum, double newSoftMaximum)
{
    d->widget->setSoftRange(newSoftMinimum, newSoftMaximum);
}

void DoubleSliderSpinBox::setSoftMinimum(double newSoftMinimum)
{
    setSoftRange(newSoftMinimum, d->widget->softMaximum());
}

void DoubleSliderSpinBox::setSoftMaximum(double newSoftMaximum)
{
    setSoftRange(d->widget->softMinimum(), newSoftMaximum);
}
