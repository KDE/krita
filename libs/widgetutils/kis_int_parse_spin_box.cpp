/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2016 Laurent Valentin Jospin <laurent.valentin@famillejospin.ch>
 * SPDX-FileCopyrightText: 2021 Deif Lou <ginoba@gmail.com>
 *
 * SPDX-License-Identifier: GPL-2.0-or-later
 */

#include <kis_parse_spin_box_p.h>

#include "kis_int_parse_spin_box.h"

KisIntParseSpinBox::KisIntParseSpinBox(QWidget *parent) :
    QSpinBox(parent),
    d(new KisParseSpinBoxPrivate<KisIntParseSpinBox, QSpinBox>(this))
{}

KisIntParseSpinBox::~KisIntParseSpinBox()
{}

void KisIntParseSpinBox::stepBy(int steps)
{
    d->stepBy(steps);
}

void KisIntParseSpinBox::setValue(int value, bool overwriteExpression)
{
    d->setValue(value, overwriteExpression);
}

bool KisIntParseSpinBox::isLastValid() const
{
    return d->isLastValid();
}

QString KisIntParseSpinBox::veryCleanText() const
{
    return d->veryCleanText();
}

QValidator::State KisIntParseSpinBox::validate(QString &input, int &pos) const
{
    return d->validate(input, pos);
}

int KisIntParseSpinBox::valueFromText(const QString &text) const
{
    return d->valueFromText(text);
}

QString KisIntParseSpinBox::textFromValue(int value) const
{
    return d->textFromValue(value);
}
