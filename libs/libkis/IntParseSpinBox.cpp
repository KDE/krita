/*
 * SPDX-FileCopyrightText: 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "IntParseSpinBox.h"

struct IntParseSpinBox::Private {
    Private() {}

    KisIntParseSpinBox *widget;
};

IntParseSpinBox::IntParseSpinBox()
    : QObject()
    , d(new Private)
{
    d->widget = new KisIntParseSpinBox();

    // Forward signals from KisIntParseSpinBox to IntParseSpinBox
    connect(d->widget, SIGNAL(errorWhileParsing(QString)), this, SIGNAL(errorWhileParsing(QString)));
    connect(d->widget, SIGNAL(noMoreParsingError()), this, SIGNAL(noMoreParsingError()));
}

IntParseSpinBox::~IntParseSpinBox()
{
    delete d;
}

QSpinBox* IntParseSpinBox::widget() const
{
    return d->widget;
}

void IntParseSpinBox::stepBy(int steps)
{
    d->widget->stepBy(steps);
}

void IntParseSpinBox::setValue(int value, bool overwriteExpression)
{
    d->widget->setValue(value, overwriteExpression);
}

bool IntParseSpinBox::isLastValid() const
{
    return d->widget->isLastValid();
}

QString IntParseSpinBox::veryCleanText() const
{
    return d->widget->veryCleanText();
}
