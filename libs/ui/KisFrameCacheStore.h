/*
 *  Copyright (c) 2018 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KISFRAMECACHESTORE_H
#define KISFRAMECACHESTORE_H

#include "kritaui_export.h"
#include <QScopedPointer>
#include "kis_types.h"

#include "opengl/kis_texture_tile_info_pool.h"

class KisOpenGLUpdateInfoBuilder;

class KisOpenGLUpdateInfo;
typedef KisSharedPtr<KisOpenGLUpdateInfo> KisOpenGLUpdateInfoSP;

class KRITAUI_EXPORT KisFrameCacheStore
{
public:
    KisFrameCacheStore(KisTextureTileInfoPoolSP pool);
    KisFrameCacheStore(KisTextureTileInfoPoolSP pool, const QString &frameCachePath);

    ~KisFrameCacheStore();

    void saveFrame(int frameId, KisOpenGLUpdateInfoSP info);
    KisOpenGLUpdateInfoSP loadFrame(int frameId, const KisOpenGLUpdateInfoBuilder &builder);

    void forgetFrame(int frameId);
    bool hasFrame(int frameId) const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISFRAMECACHESTORE_H
