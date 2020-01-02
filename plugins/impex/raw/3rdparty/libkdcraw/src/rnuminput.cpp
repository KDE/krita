/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="https://www.digikam.org">https://www.digikam.org</a>
 *
 * @date   2008-08-16
 * @brief  Integer and double num input widget
 *         re-implemented with a reset button to switch to
 *         a default value
 *
 * @author Copyright (C) 2008-2015 by Gilles Caulier
 *         <a href="mailto:caulier dot gilles at gmail dot com">caulier dot gilles at gmail dot com</a>
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General
 * Public License as published by the Free Software Foundation;
 * either version 2, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * ============================================================ */

#include "rnuminput.h"

// C++ includes

#include <cmath>

// Qt includes

#include <QToolButton>
#include <QApplication>
#include <QStyle>
#include <QHBoxLayout>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

// Local includes

#include "rsliderspinbox.h"

#include <kis_icon_utils.h>

namespace KDcrawIface
{

class Q_DECL_HIDDEN RIntNumInput::Private
{

public:

    Private()
    {
        defaultValue = 0;
        resetButton  = 0;
        input        = 0;
    }

    int             defaultValue;

    QToolButton*    resetButton;

    RSliderSpinBox* input;
};

RIntNumInput::RIntNumInput(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QHBoxLayout* const hlay  = new QHBoxLayout(this);
    d->input                 = new RSliderSpinBox(this);
    d->resetButton           = new QToolButton(this);
    d->resetButton->setAutoRaise(true);
    d->resetButton->setFocusPolicy(Qt::NoFocus);
    d->resetButton->setIcon(KisIconUtils::loadIcon("document-revert").pixmap(16, 16));
    d->resetButton->setToolTip(i18nc("@info:tooltip", "Reset to default value"));

    hlay->addWidget(d->input);
    hlay->addWidget(d->resetButton);
    hlay->setContentsMargins(QMargins());
    hlay->setStretchFactor(d->input, 10);
    hlay->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // -------------------------------------------------------------

    connect(d->resetButton, &QToolButton::clicked,
            this, &RIntNumInput::slotReset);

    connect(d->input, &RSliderSpinBox::valueChanged,
            this, &RIntNumInput::slotValueChanged);
}

RIntNumInput::~RIntNumInput()
{
    delete d;
}

void RIntNumInput::setRange(int min, int max, int step)
{
    d->input->setRange(min, max);
    d->input->setSingleStep(step);
}

int RIntNumInput::value() const
{
    return d->input->value();
}

void RIntNumInput::setValue(int v)
{
    d->input->setValue(v);
}

int RIntNumInput::defaultValue() const
{
    return d->defaultValue;
}

void RIntNumInput::setDefaultValue(int v)
{
    d->defaultValue = v;
    d->input->setValue(d->defaultValue);
    slotValueChanged(v);
}

void RIntNumInput::setSuffix(const QString& suffix)
{
    d->input->setSuffix(suffix);
}

void RIntNumInput::slotReset()
{
    d->input->setValue(d->defaultValue);
    d->resetButton->setEnabled(false);
    emit reset();
}

void RIntNumInput::slotValueChanged(int v)
{
    d->resetButton->setEnabled(v != d->defaultValue);
    emit valueChanged(v);
}

// ----------------------------------------------------

class Q_DECL_HIDDEN RDoubleNumInput::Private
{

public:

    Private()
    {
        defaultValue = 0.0;
        resetButton  = 0;
        input        = 0;
    }

    double                defaultValue;

    QToolButton*          resetButton;

    RDoubleSliderSpinBox* input;
};

RDoubleNumInput::RDoubleNumInput(QWidget* const parent)
    : QWidget(parent),
      d(new Private)
{
    QHBoxLayout* const hlay  = new QHBoxLayout(this);
    d->input                 = new RDoubleSliderSpinBox(this);
    d->resetButton           = new QToolButton(this);
    d->resetButton->setAutoRaise(true);
    d->resetButton->setFocusPolicy(Qt::NoFocus);
    d->resetButton->setIcon(KisIconUtils::loadIcon("document-revert").pixmap(16, 16));
    d->resetButton->setToolTip(i18nc("@info:tooltip", "Reset to default value"));

    hlay->addWidget(d->input);
    hlay->addWidget(d->resetButton);
    hlay->setContentsMargins(QMargins());
    hlay->setStretchFactor(d->input, 10);
    hlay->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // -------------------------------------------------------------

    connect(d->resetButton, &QToolButton::clicked,
            this, &RDoubleNumInput::slotReset);

    connect(d->input, &RDoubleSliderSpinBox::valueChanged,
            this, &RDoubleNumInput::slotValueChanged);
}

RDoubleNumInput::~RDoubleNumInput()
{
    delete d;
}

void RDoubleNumInput::setDecimals(int p)
{
    d->input->setRange(d->input->minimum(), d->input->maximum(), p);
}

void RDoubleNumInput::setRange(double min, double max, double step)
{
    d->input->setRange(min, max, (int) -floor(log10(step)));
    d->input->setFastSliderStep(5 * step);
    d->input->setSingleStep(step);
}

double RDoubleNumInput::value() const
{
    return d->input->value();
}

void RDoubleNumInput::setValue(double v)
{
    d->input->setValue(v);
}

double RDoubleNumInput::defaultValue() const
{
    return d->defaultValue;
}

void RDoubleNumInput::setDefaultValue(double v)
{
    d->defaultValue = v;
    d->input->setValue(d->defaultValue);
    slotValueChanged(v);
}

void RDoubleNumInput::setSuffix(const QString& suffix)
{
    d->input->setSuffix(suffix);
}

void RDoubleNumInput::slotReset()
{
    d->input->setValue(d->defaultValue);
    d->resetButton->setEnabled(false);
    emit reset();
}

void RDoubleNumInput::slotValueChanged(double v)
{
    d->resetButton->setEnabled(v != d->defaultValue);
    emit valueChanged(v);
}

}  // namespace KDcrawIface
