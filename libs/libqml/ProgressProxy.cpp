/* This file is part of the KDE project
 * Copyright (C) 2012 KO GmbH. Contact: Boudewijn Rempt <boud@kogmbh.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
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
        emit taskNameChanged();
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
        emit taskStarted();
    }

    if (value == d->maximum) {
        emit taskEnded();
    }

    emit valueChanged(value);
}

int ProgressProxy::maximum() const
{
    return d->maximum;
}

