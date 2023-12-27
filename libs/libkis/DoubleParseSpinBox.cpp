/*
 * SPDX-FileCopyrightText: 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "DoubleParseSpinBox.h"

struct DoubleParseSpinBox::Private {
    Private() {}

    KisDoubleParseSpinBox *widget;
};

DoubleParseSpinBox::DoubleParseSpinBox()
    : QObject()
    , d(new Private)
{
    d->widget = new KisDoubleParseSpinBox();

    // Forward signals from KisDoubleParseSpinBox to DoubleParseSpinBox
    connect(d->widget, SIGNAL(errorWhileParsing(QString)), this, SIGNAL(errorWhileParsing(QString)));
    connect(d->widget, SIGNAL(noMoreParsingError()), this, SIGNAL(noMoreParsingError()));
}

DoubleParseSpinBox::~DoubleParseSpinBox()
{
    delete d;
}

QDoubleSpinBox* DoubleParseSpinBox::widget() const
{
    return d->widget;
}

void DoubleParseSpinBox::stepBy(int steps)
{
    d->widget->stepBy(steps);
}

void DoubleParseSpinBox::setValue(double value, bool overwriteExpression)
{
    d->widget->setValue(value, overwriteExpression);
}

bool DoubleParseSpinBox::isLastValid() const
{
    return d->widget->isLastValid();
}

QString DoubleParseSpinBox::veryCleanText() const
{
    return d->widget->veryCleanText();
}
