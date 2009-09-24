/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
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
#ifndef KIS_TOP_DOWN_UPDATE_STRATEGY_H
#define KIS_TOP_DOWN_UPDATE_STRATEGY_H

#include "kis_projection_update_strategy.h"
#include "kis_types.h"

class QRect;

/**
 * The KisTopDownUpdateStrategy keeps the image projection up to date
 * by eagerly making sure that from the leaf nodes back to the root
 * all layer projections are always up-to-date
 */
class KisTopDownUpdateStrategy : public KisProjectionUpdateStrategy
{

public:

    KisTopDownUpdateStrategy(KisNodeWSP node);

    ~KisTopDownUpdateStrategy();

    void setDirty(const QRect & rc);

    void setImage(KisImageWSP image);

    void lock();

    void unlock();

protected:

    KisPaintDeviceSP updateGroupLayerProjection(const QRect & rc, KisPaintDeviceSP projection);
    void setFilthyNode(const KisNodeWSP node);

private:

    class Private;
    Private * const m_d;

};

#endif
