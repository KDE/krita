/*
 *  Copyright (c) 2014 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef PSD_ADDITIONAL_LAYER_INFO_BLOCK_H
#define PSD_ADDITIONAL_LAYER_INFO_BLOCK_H

#include <QString>
#include <QVector>
#include <QByteArray>
#include <QBitArray>
#include <QIODevice>

#include <kis_types.h>
#include <kis_paint_device.h>
#include <kis_node.h>

#include "psd.h"
#include "psd_header.h"

struct Effect {
    QString signature;
};

// cmnS: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_41831
struct CommonStateEffect {
    quint32 version;
    quint8 visible;
    quint16 unused;
};

// dsdw, isdw: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_22203
struct ShadowEffect: public Effect {
    quint32 remainingItemSize; // 41 or 50
    quint32 version;
    quint32 blurValue; // in pixels
    quint32 intensity; // in percent
    quint32 angle; // in degrees
    quint32 distance; // in pixels
    quint16 color[5]; // 2 bytes for space followed by 4 * 2 byte color component
    QString blendModeKey;
    quint8 effectEnabled;
    quint8 globalAngle; //  Use this angle in all of the layer effects
    quint8 opacity; // 0..100
    quint16 nativeColor[5];
};

// isdw: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_22203
struct InnerShadowEffect: public Effect {

};

// oglw: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_25738
struct OuterGlowEffect : public Effect {

};

// iglw: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_27692
struct InnerGlowEffect {

};

// bevl: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_31889
struct BevelEffect {

};

// sofi: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/PhotoshopFileFormats.htm#50577409_70055
struct SolidFill {

};

struct EffectsLayer {
    quint16 version; // 0
    quint16 effectsCount; //  Effects count: may be 6 (for the 6 effects in Photoshop 5 and 6) or 7 (for Photoshop 7.0)
    QVector<Effect*> effects;
};

/**
 * @brief The PsdAdditionalLayerInfoBlock class implements the Additional Layer Information block
 *
 * See: http://www.adobe.com/devnet-apps/photoshop/fileformatashtml/#50577409_71546
 */
class PsdAdditionalLayerInfoBlock
{
public:
    PsdAdditionalLayerInfoBlock();

    bool read(QIODevice* io);
    bool write(QIODevice* io, KisNodeSP node);

    bool valid();

    QString error;


    QString key;

};

#endif // PSD_ADDITIONAL_LAYER_INFO_BLOCK_H
