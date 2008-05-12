/*
 *  Copyright (c) 2008 Bart Coppens <kde@bartcoppens.be>
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
#ifndef KIS_DATAMANAGERPROXY_H_
#define KIS_DATAMANAGERPROXY_H_

#include "kis_shared.h"

class KisTile;

/**
 * Implementations of this class will return a KisTile for the given col/row, or 0 in case the data manager's
 * default tile should be returned. Is used in case a tile is not yet in the datamanager.
 * TODO: rename as KisTileSource of so?
 */
class KRITAIMAGE_EXPORT KisDataManagerProxy : public virtual KisShared {
public:
    virtual ~KisDataManagerProxy() {}

    /// Returns 0 in case this proxy knows nothing about this location; defaultTile is given so it can be used for partial tiles
    virtual KisTile* getTileDataAt(qint32 col, qint32 row, bool write, KisTile* defaultTile) = 0;
};

typedef KisSharedPtr<KisDataManagerProxy> KisDataManagerProxySP;

#endif // KIS_DATAMANAGERPROXY_H_
