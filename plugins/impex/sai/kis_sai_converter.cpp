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

#include <QVector>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoCompositeOpRegistry.h>

#include <KisDocument.h>

#include <kis_transaction.h>
#include <kis_paint_device.h>

#include <kis_paint_layer.h>
#include <kis_transparency_mask.h>
#include <kis_node.h>
#include <kis_group_layer.h>


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

            if (layerData.LayerType() == sai::LayerClass::Layer) {
                KisPaintLayerSP layer = new KisPaintLayer(m_image, layerData.LayerName(), 255);
                layer->setVisible(layerData.IsVisible());
                layer->setAlphaLocked(layerData.IsPreserveOpacity());
                quint8 opacity = qRound(layerData.Opacity() * 2.55);
                layer->setOpacity(opacity);
                layer->setCompositeOpId(BlendingMode(layerData.Blending()));
                layer->setX(int(std::get<0>(layerData.Position())));
                layer->setY(int(std::get<1>(layerData.Position())));
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
