/*
 *  Copyright (c) 2014 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_sync_lod_cache_stroke_strategy.h"

#include <kis_image.h>
#include <kundo2magicstring.h>



struct KisSyncLodCacheStrokeStrategy::Private
{
    KisImageWSP image;

    void visitNode(KisNodeSP node);
};

KisSyncLodCacheStrokeStrategy::KisSyncLodCacheStrokeStrategy(KisImageWSP image)
    : KisSimpleStrokeStrategy("SyncLodCacheStroke", kundo2_i18n("Update LOD")),
      m_d(new Private)
{
    m_d->image = image;
    enableJob(JOB_INIT, true);

    setRequestsOtherStrokesToEnd(false);
    setClearsRedoOnStart(false);
}

KisSyncLodCacheStrokeStrategy::~KisSyncLodCacheStrokeStrategy()
{
}

void KisSyncLodCacheStrokeStrategy::initStrokeCallback()
{
    KIS_ASSERT_RECOVER_RETURN(m_d->image);
    m_d->visitNode(m_d->image->root());
}

void KisSyncLodCacheStrokeStrategy::Private::visitNode(KisNodeSP node)
{
    node->syncLodCache();

    node = node->firstChild();
    while (node) {
        visitNode(node);
        node = node->nextSibling();
    }
}
