/*
 *  Copyright (c) 2019 Wolthera van Hövell tot Westerflier
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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "sai.hpp"

//Necessary for simd
#include <emmintrin.h>
#include <immintrin.h>

#include <QVector>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOpRegistry.h>

#include <kis_types.h>
#include <KisDocument.h>

#include <kis_transaction.h>
#include <kis_paint_device.h>

#include <kis_paint_layer.h>
#include <kis_transparency_mask.h>
#include <kis_node.h>
#include <kis_layer.h>
#include <kis_group_layer.h>
#include <kis_shape_layer.h>
#include <kis_psd_layer_style.h>
#include <kis_random_accessor_ng.h>
#include <KoPattern.h>

#include "kis_sai_converter.h"


KisSaiConverter::KisSaiConverter(KisDocument *doc)
    : QObject(0)
    , m_doc(doc)
    , LastAddedLayerID(0)
    , parentNodeList(QMap<std::uint32_t, KisNodeSP>())
    , clippedLayers(QVector<KisNodeSP>())
{

}

KisImportExportErrorCode KisSaiConverter::buildImage(const QString &filename)
{
    sai::Document saiFile(QFile::encodeName(filename));
    if (!saiFile.IsOpen()) {
        dbgFile << "Could not open the file, either it does not exist, either it is not a Sai file :" << filename;
        return ImportExportCodes::FileFormatIncorrect;
    }
    std::tuple<std::uint32_t, std::uint32_t> size = saiFile.GetCanvasSize();
    m_image = new KisImage(m_doc->createUndoStore(),
                           int(std::get<0>(size)),
                           int(std::get<1>(size)),
                           KoColorSpaceRegistry::instance()->rgb8(),
                           "file");
    saiFile.IterateLayerFiles(
            [&](sai::VirtualFileEntry& LayerFile)
            {
                KisSaiConverter::processLayerFile(LayerFile);
                return true;
            }
        );
    saiFile.IterateSubLayerFiles(
            [&](sai::VirtualFileEntry& LayerFile)
            {
                KisSaiConverter::processLayerFile(LayerFile);
                return true;
            }
        );
    m_image->setDefaultProjectionColor(KoColor(Qt::white, m_image->colorSpace()));

    return ImportExportCodes::OK;
}

KisImageSP KisSaiConverter::image()
{
    return m_image;
}

void KisSaiConverter::processLayerFile(sai::VirtualFileEntry &LayerFile)
{
    sai::LayerHeader header = LayerFile.Read<sai::LayerHeader>();
    qDebug() << LayerFile.GetSize() << LayerFile.GetTimeStamp() << LayerFile.GetName();

    //sai::Layer layerData = sai::Layer(Entry);
    //qDebug() << "adding layer" << layerData.LayerName();
    // Read serialization stream
    std::uint32_t CurTag;
    std::uint32_t CurTagSize;
    char LayerName[256] = {};
    std::uint32_t ParentID = 0;
    bool maskVisible = false;
    std::uint8_t TextureName[64] = {};
    std::uint16_t TextureScale = 0;
    std::uint8_t TextureOpacity= 0;
    std::uint8_t FringeEnabled = 0; // bool
    std::uint8_t FringeOpacity = 0; // 100
    std::uint8_t FringeWidth = 0;   // 1 - 15

    while( LayerFile.Read<std::uint32_t>(CurTag) && CurTag )
    {
        LayerFile.Read<std::uint32_t>(CurTagSize);
        switch( CurTag )
        {
        case sai::Tag("name"):
        {
            LayerFile.Read(LayerName, 256);
            break;
        }
        case sai::Tag("pfid"):
        case sai::Tag("plid"):
        {
            LayerFile.Read(ParentID);
            break;
        }
        case sai::Tag("lmfl"):
        {
            std::uint32_t Options;
            LayerFile.Read(Options);
            maskVisible = (Options & 2) != 0;
            //We have no use for linked to layer transform.
            break;
        }
        case sai::Tag("texn"):
        {
            LayerFile.Read(TextureName, 64);
            break;
        }
        case sai::Tag("texp"):
        {
            LayerFile.Read(TextureScale);
            LayerFile.Read(TextureOpacity);
            break;
        }
        case sai::Tag("peff"):
        {
            LayerFile.Read(FringeEnabled);
            LayerFile.Read(FringeOpacity);
            LayerFile.Read(FringeWidth);
            break;
        }
        default:
        {
            // for any streams that we do not handle,
            // we just skip forward in the stream
            LayerFile.Seek(LayerFile.Tell() + CurTagSize);
            break;
        }
        }
    }
    quint8 opacity = qRound(int(header.Opacity) * 2.55);
    QString blendingMode = BlendingMode(static_cast<sai::BlendingModes>(header.Blending));

    KisPSDLayerStyleSP style = toQShared(new KisPSDLayerStyle());
    if (!QString((char*)TextureName).isEmpty() ){
        style->patternOverlay()->setSize(TextureScale);
        style->patternOverlay()->setOpacity(TextureOpacity);
        //TODO: Add pattern.
        //style->patternOverlay()->setPattern(new KoPattern(QString((char*)TextureName)));
        style->patternOverlay()->setEffectEnabled(true);
    }
    if (FringeEnabled) {
        style->innerShadow()->setSize(FringeWidth);
        style->innerShadow()->setOpacity(FringeOpacity);
        style->innerShadow()->setDistance(0);
        style->innerShadow()->setEffectEnabled(true);
        /*
        style->bevelAndEmboss()->setStyle(psd_bevel_inner_bevel);
        style->bevelAndEmboss()->setTechnique(psd_technique_precise);
        style->bevelAndEmboss()->setDepth(100);
        style->bevelAndEmboss()->setDirection(psd_direction_up);
        style->bevelAndEmboss()->setSize(FringeWidth);
        style->bevelAndEmboss()->setAltitude(90);
        style->bevelAndEmboss()->setHighlightOpacity(0);
        style->bevelAndEmboss()->setShadowOpacity(FringeOpacity);
        style->bevelAndEmboss()->setEffectEnabled(true);
        */
    }
    style->setEnabled(true);

    switch( static_cast<sai::LayerType>(header.Type) ) {

    case sai::LayerType::Layer:
    {
        KisPaintLayerSP layer = new KisPaintLayer(m_image, LayerName, opacity);
        ReadRasterDataIntoLayer(layer, LayerFile, quint32(header.Bounds.Width), header.Bounds.Height);
        layer->setCompositeOpId(blendingMode);
        layer->setAlphaLocked(header.PreserveOpacity);
        layer->setVisible(header.Visible);
        layer->setX(int(header.Bounds.X-8));
        layer->setY(int(header.Bounds.Y-8));
        layer->setLayerStyle(style);
        handleAddingLayer(layer, header.Clipping, header.Identifier, ParentID);
        break;
    }
    case sai::LayerType::Set: {
        KisGroupLayerSP layer = new KisGroupLayer(m_image, LayerName, opacity);

        if (static_cast<sai::BlendingModes>(header.Blending) != sai::BlendingModes::PassThrough) {
            layer->setCompositeOpId(blendingMode);
        } else {
            layer->setPassThroughMode(true);
        }

        layer->setVisible(header.Visible);
        layer->setX(int(header.Bounds.X-8));
        layer->setY(int(header.Bounds.Y-8));
        layer->setLayerStyle(style);
        handleAddingLayer(layer, header.Clipping, header.Identifier, ParentID);
        break;
    }
    case sai::LayerType::Linework: {
        KisShapeLayerSP layer = new KisShapeLayer(m_doc->shapeController(), m_image, LayerName, opacity);
        layer->setCompositeOpId(blendingMode);
        layer->setVisible(header.Visible);
        layer->setX(int(header.Bounds.X-8));
        layer->setY(int(header.Bounds.Y-8));
        layer->setLayerStyle(style);
        handleAddingLayer(layer, header.Clipping, header.Identifier, ParentID);
        break;
    }
    //case sai::LayerType::Unknown4:
    //case sai::LayerType::Unknown7:
    case sai::LayerType::Mask: {
        KisTransparencyMaskSP layer = new KisTransparencyMask();
        if (parentNodeList.contains(ParentID)) {
            KisNodeSP p = parentNodeList.value(ParentID);
            KisLayerSP parentLayer = qobject_cast<KisLayer*>(p.data());
            if (parentLayer) {
                m_image->addNode(layer, parentLayer);
                layer->initSelection(parentLayer);
                //ReadRasterDataIntoMask(layer, LayerFile, header.Bounds.Width, header.Bounds.Height);
                layer->setName(LayerName);
                layer->setOpacity(opacity);
                layer->setVisible(maskVisible);
                layer->setCompositeOpId(blendingMode);
                layer->setX(int(header.Bounds.X-8));
                layer->setY(int(header.Bounds.Y-8));
            }

        }


        //only interesting thing here is identifying data and identifying parent layer.

        break;
    }
    default:
        break;
    }

    //Setup the clipped layers.
    if (!clippedLayers.isEmpty() && header.Clipping == true) {
        //XXX: Make translatable
        KisGroupLayerSP clipgroup = new KisGroupLayer(m_image, "Clipping Group", 255);
        KisNodeSP clippedLayer = clippedLayers.takeFirst();
        m_image->addNode(clipgroup, clippedLayer->parent(), clippedLayer);
        m_image->removeNode(clippedLayer);
        m_image->addNode(clippedLayer, clipgroup);
        while(!clippedLayers.isEmpty()) {
            clippedLayer = clippedLayers.takeFirst();
            m_image->addNode(clippedLayer, clipgroup);
        }
    }
    LastAddedLayerID = header.Identifier;
}

void KisSaiConverter::ReadRasterDataIntoLayer(KisPaintLayerSP layer, sai::VirtualFileEntry &entry, quint32 width, quint32 height)
{
    std::vector<std::uint8_t> BlockMap;
    quint32 blocksWidth = width/32;
    quint32 blocksHeight= height/32;
    BlockMap.resize(blocksWidth * blocksHeight);
    //qDebug() << entry.Tell() << width << height;
    entry.Read(BlockMap.data(), blocksWidth * blocksHeight);

    KisRandomAccessorSP accessor = layer->paintDevice()->createRandomAccessorNG(0, 0);

    std::size_t acc_x = 0;
    std::size_t acc_y = 0;

    for( std::size_t y = 0; y < blocksHeight; y++ ) {
        for( std::size_t x = 0; x < blocksWidth; x++ ) {
            //qDebug() << "block" << blocksWidth * y + x << "of" << blocksWidth*blocksHeight;
            if( BlockMap[blocksWidth * y + x] ) {
                std::array<std::uint8_t, 0x800> CompressedTile;
                alignas(sizeof(__m128i)) std::array<std::uint8_t, 0x1000> DecompressedTile;
                std::uint8_t Channel = 0;
                std::uint16_t Size = 0;

                while (entry.Read<std::uint16_t>(Size)>0) {

                    entry.Read(CompressedTile.data(), Size);


                        RLEDecompressStride(
                                    DecompressedTile.data(),
                                    CompressedTile.data(),
                                    sizeof(std::uint32_t), 0x1000 / sizeof(std::uint32_t),
                                    Channel
                                    );



                    Channel++;
                    if( Channel >= 4 ) {
                        for( std::size_t i = 0; i < 4; i++ ) {
                            std::uint16_t Size = entry.Read<std::uint16_t>();
                            entry.Seek(entry.Tell() + Size);
                        }
                        break;
                    }

                };

                /*
                 * // Current 32x32 tile within final image
                std::uint32_t *ImageBlock = reinterpret_cast<std::uint32_t*>(LayerImage.data())
                                              + (x * 32)
                                              + ((y * LayerHead.Bounds.Width) * 32);
                                              */
                for( std::size_t i = 0; i < (32 * 32) / 4; i++ ) {
                    // We know which real x and real y we have here

                    __m128i QuadPixel = _mm_load_si128(reinterpret_cast<__m128i*>(DecompressedTile.data()) + i);
                    // ABGR to ARGB, if you want.
                    // Do your swizzling here
/*
                    QuadPixel = _mm_shuffle_epi8(QuadPixel,
                                                 _mm_set_epi8(
                                                     15, 12, 13, 14,
                                                     11, 8, 9, 10,
                                                     7, 4, 5, 6,
                                                     3, 0, 1, 2));
*/

                    /// Alpha is pre-multiplied, convert to straight
                    // Get Alpha into [0.0,1.0] range

                    __m128 Scale = _mm_div_ps(
                                _mm_cvtepi32_ps(
                                    _mm_shuffle_epi8(
                                        QuadPixel,
                                        _mm_set_epi8(
                                            -1, -1, -1, 15,
                                            -1, -1, -1, 11,
                                            -1, -1, -1, 7,
                                            -1, -1, -1, 3
                                            )
                                        )
                                    ), _mm_set1_ps(255.0f));


                    // Normalize each channel into straight color
                    for( std::uint8_t i = 0; i < 3; i++ )
                    {
                        __m128i CurChannel = _mm_srli_epi32(QuadPixel, i * 8);
                        CurChannel = _mm_and_si128(CurChannel, _mm_set1_epi32(0xFF));
                        __m128 ChannelFloat = _mm_cvtepi32_ps(CurChannel);

                        ChannelFloat = _mm_div_ps(ChannelFloat, _mm_set1_ps(255.0));// [0,255] to [0,1]
                        ChannelFloat = _mm_div_ps(ChannelFloat, Scale);
                        ChannelFloat = _mm_mul_ps(ChannelFloat, _mm_set1_ps(255.0));// [0,1] to [0,255]

                        CurChannel = _mm_cvtps_epi32(ChannelFloat);
                        CurChannel = _mm_and_si128(CurChannel, _mm_set1_epi32(0xff));
                        CurChannel = _mm_slli_epi32(CurChannel, i * 8);

                        QuadPixel = _mm_andnot_si128(_mm_set1_epi32(0xFF << (i * 8)), QuadPixel);
                        QuadPixel = _mm_or_si128(QuadPixel, CurChannel);
                    }

                    // Write directly to final image

                    acc_x = (x * 32) + (i*4)%32;
                    acc_y = (y * 32) + ((i*4)/32);

                    accessor->moveTo(acc_x, acc_y);
                    quint8* currentPixel[4];
                    _mm_store_si128(reinterpret_cast<__m128i*>(currentPixel),
                                                            QuadPixel
                                                            );
                    memcpy(accessor->rawData(), currentPixel, layer->colorSpace()->pixelSize()*4);

                }
            }

        }
    }
}

void KisSaiConverter::ReadRasterDataIntoMask(KisTransparencyMaskSP layer, sai::VirtualFileEntry &entry, quint32 width, quint32 height) {
    std::vector<std::uint8_t> BlockMap;
    quint32 blocksWidth = width/32;
    quint32 blocksHeight= height/32;
    BlockMap.resize(blocksWidth * blocksHeight);
    //qDebug() << entry.Tell() << width << height;
    entry.Read(BlockMap.data(), blocksWidth * blocksHeight);

    KisRandomAccessorSP accessor = layer->paintDevice()->createRandomAccessorNG(0, 0);

    for( std::size_t y = 0; y < blocksHeight; y++ ) {
        for( std::size_t x = 0; x < blocksWidth; x++ ) {
            std::array<std::uint8_t, 0x400> CompressedTile = {};
            std::array<std::uint8_t, 0x400> DecompressedTile = {};
            std::uint8_t Channel = 0;
            std::uint16_t Size = 0;
            if (entry.Read<std::uint16_t>(Size) == sizeof(std::uint16_t)) {
                entry.Read(CompressedTile.data(), Size);
                RLEDecompressStride(
                            DecompressedTile.data(),
                            CompressedTile.data(),
                            sizeof(std::uint8_t), 0x400/sizeof(std::uint8_t),
                            Channel
                            );
            }

            const std::uint8_t* ImageSource = reinterpret_cast<const std::uint8_t*>(DecompressedTile.data());
            for( std::size_t i = 0; i < (32 * 32); i++ ) {
                std::size_t acc_x = (x * 32) + (i%32);
                std::size_t acc_y = (y * 32) + (i/32);
                std::uint8_t CurPixel = ImageSource[i];
                std::uint8_t* currentPixel[1];
                accessor->moveTo(acc_x, acc_y);
                memcpy(currentPixel, &CurPixel, sizeof(CurPixel));
                memcpy(accessor->rawData(), currentPixel, layer->paintDevice()->colorSpace()->pixelSize());
            }

        }
    }
}

QString KisSaiConverter::BlendingMode(sai::BlendingModes mode)
{
    QString s = "";
    switch (mode) {
    case sai::BlendingModes::Shade:
        s = COMPOSITE_LINEAR_BURN;
        break;
    case sai::BlendingModes::Binary:
        s = COMPOSITE_DISSOLVE;
        break;
    case sai::BlendingModes::Normal:
        s = COMPOSITE_OVER;
        break;
    case sai::BlendingModes::Screen:
        s = COMPOSITE_SCREEN;
        break;
    case sai::BlendingModes::Multiply:
        s = COMPOSITE_MULT;
        break;
    case sai::BlendingModes::LumiShade:
        s = COMPOSITE_LINEAR_LIGHT;
        break;
    case sai::BlendingModes::Luminosity:
        s = COMPOSITE_LUMINOSITY_SAI;
        break;
    case sai::BlendingModes::PassThrough:
        s = "passthrough";
        break;
    case sai::BlendingModes::Overlay:
        s = COMPOSITE_OVERLAY;
        break;
    }
    return s;
}

void KisSaiConverter::RLEDecompressStride(
        std::uint8_t* Destination, const std::uint8_t *Source, std::size_t Stride,
        std::size_t StrideCount, std::size_t Channel
    )
    {
        Destination += Channel;
        std::size_t WriteCount = 0;

        while( WriteCount < StrideCount )
        {
            std::uint8_t Length = *Source++;
            if( Length == 128 ) // No-op
            {
            }
            else if( Length < 128 ) // Copy
            {
                // Copy the next Length+1 bytes
                Length++;
                WriteCount += Length;
                while( Length )
                {
                    *Destination = *Source++;
                    Destination += Stride;
                    Length--;
                }
            }
            else if( Length > 128 ) // Repeating byte
            {
                // Repeat next byte exactly "-Length + 1" times
                Length ^= 0xFF;
                Length += 2;
                WriteCount += Length;
                std::uint8_t Value = *Source++;
                while( Length )
                {
                    *Destination = Value;
                    Destination += Stride;
                    Length--;
                }
            }
        }
    }

void KisSaiConverter::handleAddingLayer(KisLayerSP layer, bool clipping, quint32 layerID, quint32 parentLayerID)
{
    if (clipping) {
        //we should add it to a list so we can make a clipping group.
        //All clipped layers and the first non-clipped layer go in the group.
        if (clippedLayers.isEmpty()) {
            if (parentNodeList.contains(LastAddedLayerID)) {
                clippedLayers.append(parentNodeList.value(LastAddedLayerID));
            }
        }
        layer->disableAlphaChannel(clipping);
        clippedLayers.append(layer);
    } else {

        if (parentLayerID == 0) {
            m_image->addNode(layer);
        } else {
            if (parentNodeList.contains(parentLayerID)) {
                m_image->addNode(layer, parentNodeList.value(parentLayerID));
            }
        }
    }
    parentNodeList.insert(layerID, layer);
}
