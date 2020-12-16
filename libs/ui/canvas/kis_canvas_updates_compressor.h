/*
 *  SPDX-FileCopyrightText: 2015 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_CANVAS_UPDATES_COMPRESSOR_H
#define __KIS_CANVAS_UPDATES_COMPRESSOR_H

#include <QList>
#include <QMutex>
#include <QMutexLocker>

#include "kis_update_info.h"

typedef QList<KisUpdateInfoSP> KisUpdateInfoList;

class KisCanvasUpdatesCompressor
{
public:
    bool putUpdateInfo(KisUpdateInfoSP info);
    void takeUpdateInfo(KisUpdateInfoList &list);

private:
    QMutex m_mutex;
    KisUpdateInfoList m_updatesList;
};

#endif /* __KIS_CANVAS_UPDATES_COMPRESSOR_H */
