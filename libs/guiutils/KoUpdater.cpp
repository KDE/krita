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

#include <QApplication>
#include <QDebug>

#include "KoProgressUpdater.h"
#include "KoUpdaterPrivate.h"

KoUpdater::KoUpdater(KoUpdaterPrivate *p)
{
    qDebug() << "created KoUpdater in thread " << thread();

    d = p;
    Q_ASSERT(p);
    Q_ASSERT(!d.isNull());

    connect( this, SIGNAL( sigCancel() ), d, SLOT( cancel() ), Qt::QueuedConnection );
    connect( this, SIGNAL( sigProgress( int ) ), d, SLOT( setProgress( int ) ), Qt::QueuedConnection );
    connect( d, SIGNAL( sigInterrupted() ), this, SLOT( interrupt() ), Qt::QueuedConnection );

    setRange(0, 100);
    m_interrupted = false;
}

void KoUpdater::cancel()
{
    qDebug() << "KoUpdater::cancel in " << thread();
    emit sigCancel();
}

void KoUpdater::setProgress(int percent)
{
    qDebug() << "KoUpdater::setProgress " << percent << " in " << thread();
    emit sigProgress( percent );
    qApp->processEvents();
}

int KoUpdater::progress() const
{
    qDebug() << "KoUpdater::progress in " << thread();

    if ( d.isNull() ) {
        qDebug() << "\t Private is null!";
        return 100;
    }

    return d->progress();
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
    qDebug() << "KoUpdater::setValue " << value << " in " << thread();

    if ( value < min ) value = min;
    if ( value > max ) value = max;
    // Go from range to percent
    setProgress( (100 * value ) / range + 1 );
}

void KoUpdater::setRange( int minimum, int maximum )
{
    min = minimum - 1;
    max = maximum;
    range = max - min;
}

void KoUpdater::setFormat( const QString & format )
{
    Q_UNUSED(format);
    // XXX: Do nothing
}

void KoUpdater::interrupt()
{
    qDebug() << "KoUpdater::interrupt() in " << thread();
    m_interrupted = true;
}

#include "KoUpdater.moc"
