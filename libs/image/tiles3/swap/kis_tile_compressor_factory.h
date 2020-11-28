/*
 *  SPDX-FileCopyrightText: 2010 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
            return KisAbstractTileCompressorSP(new KisLegacyTileCompressor());
            break;
        case 2:
            return KisAbstractTileCompressorSP(new KisTileCompressor2());
            break;
        default:
            qFatal("Unknown version of the tiles");
            return KisAbstractTileCompressorSP();
        };
    }

private:
    KisTileCompressorFactory();
};

#endif /* __KIS_TILE_COMPRESSOR_FACTORY_H */

