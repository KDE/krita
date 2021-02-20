/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoProgressBar.h"

KoProgressBar::KoProgressBar(QWidget *parent)
 : QProgressBar(parent)
{
}

KoProgressBar::~KoProgressBar()
{
}

int KoProgressBar::maximum() const
{
    return QProgressBar::maximum();
}

void KoProgressBar::setValue(int value)
{
    QProgressBar::setValue(value);

    // we also show the bar if it is in undetermined state
    if (minimum() == maximum() ||
        (value >= minimum() && value < maximum())) {

        setVisible( true );

    } else {

        emit done();
        setVisible( false );
    }
}

void KoProgressBar::setRange(int minimum, int maximum)
{
    QProgressBar::setRange(minimum, maximum);
}

void KoProgressBar::setFormat(const QString &format)
{
    QProgressBar::setFormat(format);
}
