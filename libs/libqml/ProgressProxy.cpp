/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2012 KO GmbH. Contact : Boudewijn Rempt <boud@kogmbh.com>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */
#include "ProgressProxy.h"

class ProgressProxy::Private
{
public:
    int minimum;
    int maximum;
    QString taskName;
};

ProgressProxy::ProgressProxy(QObject *parent)
    : QObject(parent), d(new Private)
{
}

ProgressProxy::~ProgressProxy()
{

}

QString ProgressProxy::taskName() const
{
    return d->taskName;
}

void ProgressProxy::setFormat(const QString& format)
{
    if ( format != d->taskName ) {
        d->taskName = format;
        Q_EMIT taskNameChanged();
    }
}

void ProgressProxy::setRange(int minimum, int maximum)
{
    d->minimum = minimum;
    d->maximum = maximum;
}

void ProgressProxy::setValue(int value)
{
    if (value == d->minimum) {
        Q_EMIT taskStarted();
    }

    if (value == d->maximum) {
        Q_EMIT taskEnded();
    }

    Q_EMIT valueChanged(value);
}

int ProgressProxy::maximum() const
{
    return d->maximum;
}

