/* This file is part of the KDE project

   Copyright 2012 Brijesh Patel <brijesh3105@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/


#include "FormatErrorBarDialog.h"

using namespace KoChart;


FormatErrorBarDialog::FormatErrorBarDialog(QWidget *parent)
    : QDialog(parent)
{
    widget.setupUi(this);
    errorTypeChanged(0);

    connect(widget.posIndicator, SIGNAL(toggled(bool)), this, SLOT(errorIndicatorChanged()));
    connect(widget.negIndicator, SIGNAL(toggled(bool)), this, SLOT(errorIndicatorChanged()));
    connect(widget.posAndNegIndicator, SIGNAL(toggled(bool)), this, SLOT(errorIndicatorChanged()));
    connect(widget.sameValueForBoth, SIGNAL(toggled(bool)), this, SLOT(setSameErrorValueForBoth(bool)));
    connect(widget.positiveValue, SIGNAL(valueChanged(double)), this, SLOT(setSameErrorValueForBoth(double)));
    connect(widget.errorType, SIGNAL(currentIndexChanged(int)), this, SLOT(errorTypeChanged(int)));
}

FormatErrorBarDialog::~FormatErrorBarDialog()
{
}

void FormatErrorBarDialog::errorIndicatorChanged()
{
    if (widget.posIndicator->isChecked()) {
        widget.positiveValue->setEnabled(true);
        widget.negativeValue->setEnabled(false);
        widget.sameValueForBoth->setEnabled(false);
    } else if (widget.negIndicator->isChecked()) {
        widget.negativeValue->setEnabled(true);
        widget.positiveValue->setEnabled(false);
        widget.sameValueForBoth->setEnabled(false);
    } else {
        widget.positiveValue->setEnabled(true);
        widget.negativeValue->setEnabled(true);
        widget.sameValueForBoth->setEnabled(true);
        setSameErrorValueForBoth(widget.sameValueForBoth->isChecked());
    }
}

void FormatErrorBarDialog::errorTypeChanged(int currIndex)
{
    switch (currIndex) {
    case 1:
        widget.constantError->show();
        widget.percentageError->hide();
        break;
    case 2:
    case 3:
        widget.constantError->hide();
        widget.percentageError->show();
        break;
    default:
        widget.constantError->hide();
        widget.percentageError->hide();
    }
}

void FormatErrorBarDialog::setSameErrorValueForBoth(bool isChecked)
{
    if (isChecked) {
        widget.negativeValue->setEnabled(false);
        widget.negativeValue->setValue(widget.positiveValue->value());
    } else {
        widget.negativeValue->setEnabled(true);
    }
}

void FormatErrorBarDialog::setSameErrorValueForBoth(double value)
{
    if (widget.sameValueForBoth->isEnabled() && widget.sameValueForBoth->isChecked()) {
        widget.negativeValue->setValue(value);
    }
}
