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
#include <kis_group_layer.h>
#include <kis_random_accessor_ng.h>

#include "kis_sai_converter.h"

class SaiLayerVisitor : public sai::VirtualFileVisitor
{
public:
    SaiLayerVisitor(KisImageSP image)
        : m_image(image)
        , FolderDepth(0)
        , parentNodeList(QMap<std::uint32_t, KisNodeSP>())
        , clippedLayers(QVector<KisNodeSP>())
    {
    }

    ~SaiLayerVisitor() override
    {
    }

    bool VisitFolderBegin(sai::VirtualFileEntry& Entry) override
    {
        qDebug() << "begin folder"
                 << Entry.GetSize() << Entry.GetTimeStamp() << Entry.GetName();
        ++FolderDepth;
        return true;
    }

    bool VisitFolderEnd(sai::VirtualFileEntry& /*Entry*/) override
    {
        qDebug() << "end folder";
        --FolderDepth;
        return true;
    }

    bool VisitFile(sai::VirtualFileEntry& Entry) override
    {
        qDebug() << Entry.GetSize() << Entry.GetTimeStamp() << Entry.GetName();
        if (FolderDepth>0) {
            sai::Layer layerData = sai::Layer(Entry);
            qDebug() << "adding layer" << layerData.LayerName();

            if (layerData.LayerType() == sai::LayerClass::Layer) {
                KisPaintLayerSP layer = new KisPaintLayer(m_image, layerData.LayerName(), 255);
                layer->setVisible(layerData.IsVisible());
                layer->setAlphaLocked(layerData.IsPreserveOpacity());
                quint8 opacity = qRound(layerData.Opacity() * 2.55);
                layer->setOpacity(opacity);
                layer->setCompositeOpId(BlendingMode(layerData.Blending()));

                ReadRasterDataIntoLayer(layer, Entry, quint32(std::get<0>(layerData.Size())), quint32(std::get<1>(layerData.Size())));

                layer->setX(int(std::get<0>(layerData.Position()))-8);
                layer->setY(int(std::get<1>(layerData.Position()))-8);
                // Bounds;

                if (layerData.IsClipping() || !clippedLayers.isEmpty()) {
                    //we should add it to a list so we can make a clipping group.
                    //All clipped layers and the first non-clipped layer go in the group.
                    layer->disableAlphaChannel(layerData.IsClipping());
                    clippedLayers.append(layer);
                } else {

                    if (layerData.ParentID() == 0) {
                        m_image->addNode(layer);
                    } else {
                        if (parentNodeList.contains(layerData.ParentID())) {
                            m_image->addNode(layer, parentNodeList.value(layerData.ParentID()));
                        }
                    }
                }
                parentNodeList.insert(layerData.Identifier(), layer);

            } /*else if (layerData.LayerType() == sai::LayerClass::Linework) {
                break;
            }*/ else if (layerData.LayerType() == sai::LayerClass::Set) {
                KisGroupLayerSP layer = new KisGroupLayer(m_image, layerData.LayerName(), 255);
                layer->setVisible(layerData.IsVisible());
                quint8 opacity = qRound(layerData.Opacity() * 2.55);
                layer->setOpacity(opacity);
                layer->setX(0);
                layer->setY(0);
                if (layerData.Blending() != sai::BlendingMode::PassThrough) {
                    layer->setCompositeOpId(BlendingMode(layerData.Blending()));
                } else {
                    layer->setPassThroughMode(true);
                }

                if (layerData.ParentID() == 0) {
                    m_image->addNode(layer);
                } else {
                    if (parentNodeList.contains(layerData.ParentID())) {
                        m_image->addNode(layer, parentNodeList.value(layerData.ParentID()));
                    }
                }
                parentNodeList.insert(layerData.Identifier(), layer);

            } else if (layerData.LayerType() == sai::LayerClass::Mask) {
                KisTransparencyMaskSP layer = new KisTransparencyMask();
                //only interesting thing here is identifying data and identifying parent layer.
                if (parentNodeList.contains(layerData.ParentID())) {
                    m_image->addNode(layer, parentNodeList.value(layerData.ParentID()));
                }
            } else {
                qDebug() << "unknown layer type";
            }
            if (!clippedLayers.isEmpty() && layerData.IsClipping() == false) {
                //XXX: Make translatable
                KisGroupLayerSP clipgroup = new KisGroupLayer(m_image, "Clipping Group", 255);
                KisNodeSP clippedLayer;
                while(!clippedLayers.isEmpty()) {
                    clippedLayer = clippedLayers.takeLast();
                    qDebug() << clippedLayer->name();
                    m_image->addNode(clippedLayer, clipgroup);
                }
                m_image->addNode(clipgroup);
                qDebug() <<clippedLayers.size();
            }
        }



        return true;
    }
private:
    KisImageSP m_image;
    std::uint32_t FolderDepth;
    QMap<std::uint32_t, KisNodeSP> parentNodeList;
    QVector<KisNodeSP> clippedLayers;

    QString BlendingMode(sai::BlendingMode mode) {
        QString s = "";
        switch (mode) {
        case sai::BlendingMode::Shade:
            s = COMPOSITE_SUBTRACT;
            break;
        case sai::BlendingMode::Binary:
            s = COMPOSITE_DISSOLVE;
            break;
        case sai::BlendingMode::Normal:
            s = COMPOSITE_OVER;
            break;
        case sai::BlendingMode::Screen:
            s = COMPOSITE_SCREEN;
            break;
        case sai::BlendingMode::Multiply:
            s = COMPOSITE_MULT;
            break;
        case sai::BlendingMode::LumiShade:
            s = COMPOSITE_LINEAR_LIGHT;
            break;
        case sai::BlendingMode::Luminosity:
            s = COMPOSITE_LUMINOSITY_SAI;
            break;
        case sai::BlendingMode::PassThrough:
            s = "passthrough";
            break;
        case sai::BlendingMode::Overlay:
            s = COMPOSITE_OVERLAY;
            break;
        }
        return s;
    }

    void ReadRasterDataIntoLayer(KisPaintLayerSP layer, sai::VirtualFileEntry &entry, quint32 width, quint32 height)
    {
        std::vector<std::uint8_t> BlockMap;
        quint32 blocksWidth = width/32;
        quint32 blocksHeight= height/32;
        BlockMap.resize(blocksWidth * blocksHeight);
        qDebug() << entry.Tell() << width << height;
        entry.Read(BlockMap.data(), blocksWidth * blocksHeight);

        KisRandomAccessorSP accessor = layer->paintDevice()->createRandomAccessorNG(0, 0);

        std::size_t acc_x = 0;
        std::size_t acc_y = 0;

        for( std::size_t y = 0; y < blocksHeight; y++ ) {
            for( std::size_t x = 0; x < blocksWidth; x++ ) {
                qDebug() << "block" << blocksWidth * y + x << "of" << blocksWidth*blocksHeight;
                if( BlockMap[blocksWidth * y + x] ) {
                    std::array<std::uint8_t, 0x800> CompressedTile;
                    alignas(sizeof(__m128i)) std::array<std::uint8_t, 0x1000> DecompressedTile;
                    std::uint8_t Channel = 0;
                    std::uint16_t Size = 0;

                    while (entry.Read<std::uint16_t>(Size)>0) {

                        entry.Read(CompressedTile.data(), Size);


                            RLEDecompress32(
                                        DecompressedTile.data(),
                                        CompressedTile.data(),
                                        Size,
                                        1024,
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
    void RLEDecompress32(void* Destination, const std::uint8_t *Source, std::size_t SourceSize, std::size_t IntCount, std::size_t Channel)
    {
        std::uint8_t *Write = reinterpret_cast<std::uint8_t*>(Destination) + Channel;
        std::size_t WriteCount = 0;

        while( WriteCount < IntCount )
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
                    *Write = *Source++;
                    Write += 4;
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
                    *Write = Value;
                    Write += 4;
                    Length--;
                }
            }
        }
    }
};

KisSaiConverter::KisSaiConverter(KisDocument *doc)
    : QObject(0),
      m_doc(doc)
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

    SaiLayerVisitor visitor(m_image);
    saiFile.IterateFileSystem(visitor);

    return ImportExportCodes::OK;
}

KisImageSP KisSaiConverter::image()
{
    return m_image;
}
