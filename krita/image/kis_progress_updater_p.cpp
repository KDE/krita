/*
 *  Copyright (c) 2006 Thomas Zander <zander@kde.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <kis_progress_updater.h>
#include <kis_progress_updater_p.h>

// technically speaking the access to the progress integer should be guarded
// by a mutex, even if we assume that it will only ever be set from one thread.
// The reason is that while the KoAction is reading it its possible to alter the
// int and as an effect get an unpredictable value in the slider on screen.

KisUpdaterPrivate::KisUpdaterPrivate(KisProgressUpdater *parent, int weight)
    : m_progress(0),
    m_weight(weight),
    m_interrupted(false),
    m_parent(parent)
{
}

void KisUpdaterPrivate::setProgress(int progress) {
    if(m_progress >= progress)
        return;
    m_progress = progress;
    m_parent->scheduleUpdate();
}

void KisUpdaterPrivate::cancel() {
    m_parent->cancel();
}

#include <kis_progress_updater_p.moc>
