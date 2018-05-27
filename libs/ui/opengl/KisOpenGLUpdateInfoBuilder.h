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
#ifndef KISOPENGLUPDATEINFOBUILDER_H
#define KISOPENGLUPDATEINFOBUILDER_H

#include "kritaui_export.h"
#include <QScopedPointer>
#include <QSharedPointer>

class KisProofingConfiguration;
typedef QSharedPointer<KisProofingConfiguration> KisProofingConfigurationSP;

class KisTextureTileInfoPool;
typedef QSharedPointer<KisTextureTileInfoPool> KisTextureTileInfoPoolSP;

template<class T>
class KisSharedPtr;

class KisImage;
typedef KisSharedPtr<KisImage> KisImageSP;

class KisPaintDevice;
typedef KisSharedPtr<KisPaintDevice> KisPaintDeviceSP;

class KisOpenGLUpdateInfo;
typedef KisSharedPtr<KisOpenGLUpdateInfo> KisOpenGLUpdateInfoSP;

class KoColorSpace;
struct ConversionOptions;


class KRITAUI_EXPORT KisOpenGLUpdateInfoBuilder
{
public:
    KisOpenGLUpdateInfoBuilder();
    ~KisOpenGLUpdateInfoBuilder();

    KisOpenGLUpdateInfoSP buildUpdateInfo(const QRect& rect, KisImageSP srcImage, bool convertColorSpace);
    KisOpenGLUpdateInfoSP buildUpdateInfo(const QRect& rect, KisPaintDeviceSP projection, const QRect &bounds, int levelOfDetail, bool convertColorSpace);

    QRect calculatePhysicalTileRect(int col, int row, const QRect &imageBounds, int levelOfDetail) const;
    QRect calculateEffectiveTileRect(int col, int row, const QRect &imageBounds) const;
    int xToCol(int x) const;
    int yToRow(int y) const;

    const KoColorSpace* destinationColorSpace() const;

    void setConversionOptions(const ConversionOptions &options);
    void setChannelFlags(const QBitArray &channelFrags, bool onlyOneChannelSelected, int selectedChannelIndex);

    void setTextureBorder(int value);
    void setEffectiveTextureSize(const QSize &size);

    void setTextureInfoPool(KisTextureTileInfoPoolSP pool);
    KisTextureTileInfoPoolSP textureInfoPool() const;

    void setProofingConfig(KisProofingConfigurationSP config);
    KisProofingConfigurationSP proofingConfig() const;

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif // KISOPENGLUPDATEINFOBUILDER_H
