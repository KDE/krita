/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2006-2007 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2009 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
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


    setRange(0, 100);
    m_interrupted = false;
}

KoUpdater::~KoUpdater()
{
}

void KoUpdater::cancel()
{
    Q_EMIT sigCancel();
}

void KoUpdater::setProgress(int percent)
{
    const bool percentChanged = m_progressPercent != percent;

    if (percentChanged || percent == 0 || percent == 100) {
        m_progressPercent = percent;
        Q_EMIT sigProgress( percent );
    }
}

int KoUpdater::progress() const
{

    return m_progressPercent;
}

bool KoUpdater::interrupted() const
{
    return m_interrupted.loadAcquire();
}

int KoUpdater::maximum() const
{
    return max;
}

void KoUpdater::setValue( int value )
{
    value = qBound(min, value, max);

    // Go from range to percent
    const int range = max - min;

    if (range == 0) {
        m_progressPercent = max;
        Q_EMIT sigProgress(max);
    } else {
        setProgress((100 * (value - min)) / range);
    }
}

void KoUpdater::setRange( int minimum, int maximum )
{
    min = minimum;
    max = maximum;
    range = max - min;
    Q_EMIT sigHasValidRangeChanged(range != 0);
}

void KoUpdater::setFormat( const QString & format )
{
    Q_EMIT sigNestedNameChanged(format);
}

void KoUpdater::setAutoNestedName(const QString &name)
{
    Q_EMIT sigNestedNameChanged(name);
}

void KoUpdater::setInterrupted(bool value)
{
    m_interrupted.storeRelease(value);
}

KoDummyUpdaterHolder::KoDummyUpdaterHolder()
    : d(new KoUpdaterPrivate(0, "dummy"))
{
}

KoDummyUpdaterHolder::~KoDummyUpdaterHolder()
{
    d->deleteLater();
}

KoUpdater *KoDummyUpdaterHolder::updater()
{
    return d->connectedUpdater();
}
