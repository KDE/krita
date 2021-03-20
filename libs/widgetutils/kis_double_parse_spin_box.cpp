/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kis_parse_spin_box_p.h>

#include "kis_double_parse_spin_box.h"

KisDoubleParseSpinBox::KisDoubleParseSpinBox(QWidget *parent) :
    QDoubleSpinBox(parent),
    d(new KisParseSpinBoxPrivate<KisDoubleParseSpinBox, QDoubleSpinBox>(this))
{}

KisDoubleParseSpinBox::~KisDoubleParseSpinBox()
{}

void KisDoubleParseSpinBox::stepBy(int steps)
{
    d->stepBy(steps);
}

void KisDoubleParseSpinBox::setValue(double value, bool overwriteExpression)
{
    d->setValue(value, overwriteExpression);
}

bool KisDoubleParseSpinBox::isLastValid() const
{
    return d->isLastValid();
}

QString KisDoubleParseSpinBox::veryCleanText() const
{
    return d->veryCleanText();
}

QValidator::State KisDoubleParseSpinBox::validate(QString &input, int &pos) const
{
    return d->validate(input, pos);
}

double KisDoubleParseSpinBox::valueFromText(const QString &text) const
{
    return d->valueFromText(text);
}

QString KisDoubleParseSpinBox::textFromValue(double value) const
{
    return d->textFromValue(value);
}
