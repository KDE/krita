/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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
#ifndef KIS_TESTING_TILE_DATA_STORE_ACCESSOR_H_
#define KIS_TESTING_TILE_DATA_STORE_ACCESSOR_H_

#include "krita_export.h"
#include "kis_tile_data_store.h"

/**
 * This class is used for testing pupposes only.
 * It allows accessing to the dile data store in tests
 */
class KRITAIMAGE_EXPORT KisTestingTileDataStoreAccessor
{
public:
    static KisTileDataStore* getStore();
    static void swapAll();
};

#endif /* KIS_TESTING_TILE_DATA_STORE_ACCESSOR_H_ */

