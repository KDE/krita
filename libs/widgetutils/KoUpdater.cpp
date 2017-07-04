/* This file is part of the KDE project
 *
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009 Boudewijn Rempt <boud@valdyas.org>
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

#include "KoUpdater.h"

#include "KoUpdaterPrivate_p.h"

KoUpdater::KoUpdater(KoUpdaterPrivate *_d)
    : m_progressPercent(0)
{
    d = _d;
    Q_ASSERT(!d.isNull());

    connect(this, SIGNAL(sigCancel()), d, SLOT(cancel()));
    connect(this, SIGNAL(sigProgress(int)), d, SLOT(setProgress(int)));
    connect(this, SIGNAL(sigNestedNameChanged(QString)), d, SLOT(setAutoNestedName(QString)));
    connect(this, SIGNAL(sigHasValidRangeChanged(bool)), d, SLOT(setHasValidRange(bool)));
    connect(d, SIGNAL(sigInterrupted(bool)), this, SLOT(setInterrupted(bool)));


    setRange(0, 100);
    m_interrupted = false;
}

KoUpdater::~KoUpdater()
{
}

void KoUpdater::cancel()
{
    emit sigCancel();
}

void KoUpdater::setProgress(int percent)
{
    m_progressPercent = percent;

    emit sigProgress( percent );
}

int KoUpdater::progress() const
{

    return m_progressPercent;
}

bool KoUpdater::interrupted() const
{
    return m_interrupted;
}

int KoUpdater::maximum() const
{
    return 100;
}

void KoUpdater::setValue( int value )
{
    value = qBound(min, value, max);

    // Go from range to percent
    const int range = max - min;

    if (range == 0) {
        m_progressPercent = max;
        emit sigProgress(max);
    } else {
        setProgress((100 * (value - min)) / (max - min));
    }
}

void KoUpdater::setRange( int minimum, int maximum )
{
    min = minimum;
    max = maximum;
    range = max - min;
    emit sigHasValidRangeChanged(range != 0);
}

void KoUpdater::setFormat( const QString & format )
{
    emit sigNestedNameChanged(format);
}

void KoUpdater::setAutoNestedName(const QString &name)
{
    emit sigNestedNameChanged(name);
}

void KoUpdater::setInterrupted(bool value)
{
    m_interrupted = true;
}

KoDummyUpdater::KoDummyUpdater()
    : KoUpdater(new KoUpdaterPrivate(0, 0, "dummy"))
{
}
