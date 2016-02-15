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

#ifndef __KIS_TILE_COMPRESSOR_FACTORY_H
#define __KIS_TILE_COMPRESSOR_FACTORY_H

#include "tiles3/swap/kis_legacy_tile_compressor.h"
#include "tiles3/swap/kis_tile_compressor_2.h"

class KRITAIMAGE_EXPORT KisTileCompressorFactory
{
public:
    static KisAbstractTileCompressorSP create(qint32 version) {
        switch(version) {
        case 1:
            return new KisLegacyTileCompressor();
            break;
        case 2:
            return new KisTileCompressor2();
            break;
        default:
            qFatal("Unknown version of the tiles");
            return 0;
        };
    }

private:
    KisTileCompressorFactory();
};

#endif /* __KIS_TILE_COMPRESSOR_FACTORY_H */

