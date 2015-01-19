/*
 *  Copyright (c) 2015 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_canvas_updates_compressor.h"

bool KisCanvasUpdatesCompressor::putUpdateInfo(KisUpdateInfoSP info)
{
    const QRect newUpdateRect = info->dirtyImageRect();
    if (newUpdateRect.isEmpty()) return false;

    QMutexLocker l(&m_mutex);
    bool updateOverridden = false;

    UpdateInfoList::iterator it = m_updatesList.begin();
    while (it != m_updatesList.end()) {
        if (newUpdateRect.contains((*it)->dirtyImageRect())) {
            if (info) {
                *it = info;
                info = 0;
                ++it;
            } else {
                it = m_updatesList.erase(it);
            }

            updateOverridden = true;
        } else {
            ++it;
        }
    }

    if (!updateOverridden) {
        Q_ASSERT(info);
        m_updatesList.append(info);
    }

    return !updateOverridden;
}

KisUpdateInfoSP KisCanvasUpdatesCompressor::takeUpdateInfo()
{
    QMutexLocker l(&m_mutex);
    return !m_updatesList.isEmpty() ? m_updatesList.takeFirst() : 0;
}
