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
#include "KoUpdaterPrivate.h"

#include <kdebug.h>

KoUpdaterPrivate::~KoUpdaterPrivate()
{
    interrupt();
}

void KoUpdaterPrivate::cancel()
{
    kDebug(30004) << "KoUpdaterPrivate::cancel in " << thread();
    m_parent->cancel();
}

void KoUpdaterPrivate::interrupt()
{
    kDebug(30004) << "KoUpdaterPrivate::interrupt in " << thread();
    m_interrupted = true;
    emit sigInterrupted();
}

void KoUpdaterPrivate::setProgress(int percent)
{
    kDebug(30004) << "KoUpdaterPrivate::setProgress" << percent << " in " << thread();
    if(m_progress >= percent) {
        return;
    }
    m_progress = percent;
    emit sigUpdated();
}

#include "KoUpdaterPrivate.moc"
