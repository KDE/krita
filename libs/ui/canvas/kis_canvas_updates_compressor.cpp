/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_canvas_updates_compressor.h"

bool KisCanvasUpdatesCompressor::putUpdateInfo(KisUpdateInfoSP info)
{
    const int levelOfDetail = info->levelOfDetail();
    const QRect newUpdateRect = info->dirtyImageRect();
    if (newUpdateRect.isEmpty()) return false;

    QMutexLocker l(&m_mutex);

    if (info->canBeCompressed()) {
        KisUpdateInfoList::iterator it = m_updatesList.begin();
        while (it != m_updatesList.end()) {
            if ((*it)->canBeCompressed() &&
                levelOfDetail == (*it)->levelOfDetail() &&
                newUpdateRect.contains((*it)->dirtyImageRect())) {

                /**
                 * We should always remove the overridden update and put 'info' to the end
                 * of the queue. Otherwise, the updates will become reordered and the canvas
                 * may have tiles artifacts with "outdated" data
                 */
                it = m_updatesList.erase(it);
            } else {
                ++it;
            }
        }
    }

    m_updatesList.append(info);

    return m_updatesList.size() <= 1;
}

void KisCanvasUpdatesCompressor::takeUpdateInfo(KisUpdateInfoList &list)
{
    KIS_SAFE_ASSERT_RECOVER(list.isEmpty()) { list.clear(); }

    QMutexLocker l(&m_mutex);
    m_updatesList.swap(list);
}
