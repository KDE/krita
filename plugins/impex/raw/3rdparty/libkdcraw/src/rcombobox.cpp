/** ===========================================================
 * @file
 *
 * This file is a part of digiKam project
 * <a href="https://www.digikam.org">https://www.digikam.org</a>
 *
 * @date   2008-08-16
 * @brief  a combo box widget re-implemented with a
 *         reset button to switch to a default item
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

#include "rcombobox.h"

// Qt includes

#include <QApplication>
#include <QStyle>
#include <QToolButton>
#include <QHBoxLayout>
#include <QIcon>

// KDE includes

#include <klocalizedstring.h>

#include <kis_icon_utils.h>

namespace KDcrawIface
{

class Q_DECL_HIDDEN RComboBox::Private
{

public:

    Private()
    {
        defaultIndex = 0;
        resetButton  = 0;
        combo        = 0;
    }

    int          defaultIndex;

    QToolButton* resetButton;

    QComboBox*   combo;
};

RComboBox::RComboBox(QWidget* const parent)
         : QWidget(parent), d(new Private)
{
    QHBoxLayout* const hlay = new QHBoxLayout(this);
    d->combo                = new QComboBox(this);
    d->resetButton          = new QToolButton(this);
    d->resetButton->setAutoRaise(true);
    d->resetButton->setFocusPolicy(Qt::NoFocus);
    d->resetButton->setIcon(KisIconUtils::loadIcon("document-revert").pixmap(16, 16));
    d->resetButton->setToolTip(i18nc("@info:tooltip", "Reset to default value"));

    hlay->addWidget(d->combo);
    hlay->addWidget(d->resetButton);
    hlay->setStretchFactor(d->combo, 10);
    hlay->setMargin(0);
    hlay->setSpacing(QApplication::style()->pixelMetric(QStyle::PM_DefaultLayoutSpacing));

    // -------------------------------------------------------------

    connect(d->resetButton, &QToolButton::clicked,
            this, &RComboBox::slotReset);

    connect(d->combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::activated),
            this, &RComboBox::slotItemActivated);

    connect(d->combo, static_cast<void (QComboBox::*)(int)>(&QComboBox::currentIndexChanged),
            this, &RComboBox::slotCurrentIndexChanged);
}

RComboBox::~RComboBox()
{
    delete d;
}

QComboBox* RComboBox::combo() const
{
    return d->combo;
}

void RComboBox::addItem(const QString& t, int index)
{
    d->combo->addItem(t, index);
}

void RComboBox::insertItem(int index, const QString& t)
{
    d->combo->insertItem(index, t);
}

int RComboBox::currentIndex() const
{
    return d->combo->currentIndex();
}

void RComboBox::setCurrentIndex(int v)
{
    d->combo->setCurrentIndex(v);
}

int RComboBox::defaultIndex() const
{
    return d->defaultIndex;
}

void RComboBox::setDefaultIndex(int v)
{
    d->defaultIndex = v;
    d->combo->setCurrentIndex(d->defaultIndex);
    slotItemActivated(v);
}

void RComboBox::slotReset()
{
    d->combo->setCurrentIndex(d->defaultIndex);
    d->resetButton->setEnabled(false);
    slotItemActivated(d->defaultIndex);
    emit reset();
}

void RComboBox::slotItemActivated(int v)
{
    d->resetButton->setEnabled(v != d->defaultIndex);
    emit activated(v);
}

void RComboBox::slotCurrentIndexChanged(int v)
{
    d->resetButton->setEnabled(v != d->defaultIndex);
    emit currentIndexChanged(v);
}

}  // namespace KDcrawIface
