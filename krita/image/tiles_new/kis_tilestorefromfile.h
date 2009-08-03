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
#ifndef KIS_TILESTOREFROMFILE_H_
#define KIS_TILESTOREFROMFILE_H_

#include <sys/types.h>

#include <qglobal.h>
#include <QHash>
#include <QLinkedList>
#include <QVector>
#include <QMutex>

#include <ktemporaryfile.h>
#include <krita_export.h>

#include "kis_tilestore.h"

#include "kis_tile.h"

class KisSharedTileData;

/**
 * This class is a very small tilestore that knows how to deal with the case where we read
 * tiledata directly from an image file, and which on write actions will duplicate itself into
 * a tile from a memory tilestore.
 */
class KRITAIMAGE_EXPORT KisTileStoreFromFile : public KisTileStore
{
public:
    virtual ~KisTileStoreFromFile() {}

protected:
    struct StoreFromFileInfo : public KisTileStoreData {
    };
};

#endif // KIS_TILESTOREFROMFILE_H_
