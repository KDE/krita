/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
 *  Copyright (c) 2007-2008 Boudewijn Rempt <boud@valdyas.org>
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

#include "kra/kis_kra_save_visitor.h"
#include "kra/kis_kra_tags.h"

#include <QBuffer>
#include <QByteArray>

#include <KoColorProfile.h>
#include <KoStore.h>
#include <KoColorSpace.h>

#include <filter/kis_filter_configuration.h>
#include <generator/kis_generator_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_annotation.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include <kis_selection.h>
#include <kis_shape_layer.h>
#include <kis_file_layer.h>
#include <kis_clone_layer.h>
#include <kis_mask.h>
#include <kis_filter_mask.h>
#include <kis_transform_mask.h>
#include <kis_transform_mask_params_interface.h>
#include <kis_transparency_mask.h>
#include <kis_selection_mask.h>
#include <kis_selection_component.h>
#include <kis_pixel_selection.h>
#include <metadata/kis_meta_data_store.h>
#include <metadata/kis_meta_data_io_backend.h>

#include "kis_config.h"
#include "kis_store_paintdevice_writer.h"
#include "flake/kis_shape_selection.h"

using namespace KRA;

KisKraSaveVisitor::KisKraSaveVisitor(KoStore *store, const QString & name, QMap<const KisNode*, QString> nodeFileNames)
        : KisNodeVisitor()
        , m_store(store)
        , m_external(false)
        , m_name(name)
        , m_nodeFileNames(nodeFileNames)
        , m_writer(new KisStorePaintDeviceWriter(store))
{
}

KisKraSaveVisitor::~KisKraSaveVisitor()
{
    delete m_writer;
}

void KisKraSaveVisitor::setExternalUri(const QString &uri)
{
    m_external = true;
    m_uri = uri;
}

bool KisKraSaveVisitor::visit(KisExternalLayer * layer)
{
    bool result = false;
    if (KisShapeLayer* shapeLayer = dynamic_cast<KisShapeLayer*>(layer)) {
        if (!saveMetaData(layer)) {
            m_errorMessages << i18n("Failed to save the metadata for layer %1.", layer->name());
            return false;
        }
        m_store->pushDirectory();
        m_store->enterDirectory(getLocation(layer, DOT_SHAPE_LAYER)) ;
        result = shapeLayer->saveLayer(m_store);
        m_store->popDirectory();
    }
    else if (KisFileLayer *fileLayer = dynamic_cast<KisFileLayer*>(layer)) {
        Q_UNUSED(fileLayer); // We don't save data for file layers, but we still want to save the masks.
        result = true;
    }
    return result && visitAllInverse(layer);
}

bool KisKraSaveVisitor::visit(KisPaintLayer *layer)
{
    if (!savePaintDevice(layer->paintDevice(), getLocation(layer))) {
        m_errorMessages << i18n("Failed to save the pixel data for layer %1.", layer->name());
        return false;
    }
    if (!saveAnnotations(layer)) {
        m_errorMessages << i18n("Failed to save the annotations for layer %1.", layer->name());
        return false;
    }
    if (!saveMetaData(layer)) {
        m_errorMessages << i18n("Failed to save the metadata for layer %1.", layer->name());
        return false;
    }
    return visitAllInverse(layer);
}

bool KisKraSaveVisitor::visit(KisGroupLayer *layer)
{
    if (!saveMetaData(layer)) {
        m_errorMessages << i18n("Failed to save the metadata for layer %1.", layer->name());
        return false;
    }
    return visitAllInverse(layer);
}

bool KisKraSaveVisitor::visit(KisAdjustmentLayer* layer)
{
    if (!layer->filter()) {
        m_errorMessages << i18n("Failed to save the filter layer %1: it has no filter.", layer->name());
        return false;
    }
    if (!saveSelection(layer)) {
        m_errorMessages << i18n("Failed to save the selection for filter layer %1.", layer->name());
        return false;
    }
    if (!saveFilterConfiguration(layer)) {
        m_errorMessages << i18n("Failed to save the filter configuration for filter layer %1.", layer->name());
        return false;
    }
    if (!saveMetaData(layer)) {
        m_errorMessages << i18n("Failed to save the metadata for layer %1.", layer->name());
        return false;
    }
    return visitAllInverse(layer);
}

bool KisKraSaveVisitor::visit(KisGeneratorLayer * layer)
{
    if (!saveSelection(layer)) {
        m_errorMessages << i18n("Failed to save the selection for layer %1.", layer->name());
        return false;
    }
    if (!saveAnnotations(layer)) {
        m_errorMessages << i18n("Failed to save the annotations for layer %1.", layer->name());
        return false;   // generator layers can have a profile because they have their own pixel data
    }
    if (!saveFilterConfiguration(layer)) {
        m_errorMessages << i18n("Failed to save the generator configuration for layer %1.", layer->name());
        return false;
    }
    if (!saveMetaData(layer)) {
        m_errorMessages << i18n("Failed to save the metadata for layer %1.", layer->name());
        return false;
    }
    return visitAllInverse(layer);
}

bool KisKraSaveVisitor::visit(KisCloneLayer *layer)
{
    // Clone layers do not have a profile
    if (!saveMetaData(layer)) {
        m_errorMessages << i18n("Failed to save the metadata for layer %1.", layer->name());
        return false;
    }
    return visitAllInverse(layer);
}

bool KisKraSaveVisitor::visit(KisFilterMask *mask)
{
    if (!mask->filter()) {
        m_errorMessages << i18n("Failed to save filter mask %1. It has no filter", mask->name());
        return false;
    }
    if (!saveSelection(mask)) {
        m_errorMessages << i18n("Failed to save the selection for filter mask %1.", mask->name());
        return false;
    }
    if (!saveFilterConfiguration(mask)) {
        m_errorMessages << i18n("Failed to save the filter configuration for filter mask %1.", mask->name());
        return false;
    }
    return true;
}

bool KisKraSaveVisitor::visit(KisTransformMask *mask)
{
    QDomDocument doc("transform_params");

    QDomElement root = doc.createElement("transform_params");

    QDomElement main = doc.createElement("main");
    main.setAttribute("id", mask->transformParams()->id());

    QDomElement data = doc.createElement("data");
    mask->transformParams()->toXML(&data);

    doc.appendChild(root);
    root.appendChild(main);
    root.appendChild(data);

    QString location = getLocation(mask, DOT_TRANSFORMCONFIG);
    if (m_store->open(location)) {
        QByteArray a = doc.toByteArray();
        m_store->write(a, a.size());
        m_store->close();
    }

    return true;
}

bool KisKraSaveVisitor::visit(KisTransparencyMask *mask)
{
    if (!saveSelection(mask)) {
        m_errorMessages << i18n("Failed to save the selection for transparency mask %1.", mask->name());
        return false;
    }
    return true;
}

bool KisKraSaveVisitor::visit(KisSelectionMask *mask)
{
    if (!saveSelection(mask)) {
        m_errorMessages << i18n("Failed to save the selection for local selection %1.", mask->name());
        return false;
    }
    return true;
}

QStringList KisKraSaveVisitor::errorMessages() const
{
    return m_errorMessages;
}


bool KisKraSaveVisitor::savePaintDevice(KisPaintDeviceSP device,
                                        QString location)
{
    // Layer data
    KisConfig cfg;
    m_store->setCompressionEnabled(cfg.compressKra());

    if (m_store->open(location)) {
        if (!device->write(*m_writer)) {
            device->disconnect();
            m_store->close();
            return false;
        }

        m_store->close();
    }
    if (m_store->open(location + ".defaultpixel")) {
        m_store->write((char*)device->defaultPixel(), device->colorSpace()->pixelSize());
        m_store->close();
    }
    m_store->setCompressionEnabled(true);
    return true;
}

bool KisKraSaveVisitor::saveAnnotations(KisLayer* layer)
{
    if (!layer) return false;
    if (!layer->paintDevice()) return false;
    if (!layer->paintDevice()->colorSpace()) return false;

    if (layer->paintDevice()->colorSpace()->profile()) {
        const KoColorProfile *profile = layer->paintDevice()->colorSpace()->profile();
        KisAnnotationSP annotation;
        if (profile) {
            QByteArray profileRawData = profile->rawData();
            if (!profileRawData.isEmpty()) {
                if (profile->type() == "icc") {
                    annotation = new KisAnnotation(ICC, profile->name(), profile->rawData());
                } else {
                    annotation = new KisAnnotation(PROFILE, profile->name(), profile->rawData());
                }
            }
        }

        if (annotation) {
            // save layer profile
            if (m_store->open(getLocation(layer, DOT_ICC))) {
                m_store->write(annotation->annotation());
                m_store->close();
            } else {
                return false;
            }
        }
    }
    return true;

}
bool KisKraSaveVisitor::saveSelection(KisNode* node)
{
    KisSelectionSP selection;
    if (node->inherits("KisMask")) {
        selection = static_cast<KisMask*>(node)->selection();
    } else if (node->inherits("KisAdjustmentLayer")) {
        selection = static_cast<KisAdjustmentLayer*>(node)->internalSelection();
    } else if (node->inherits("KisGeneratorLayer")) {
        selection = static_cast<KisGeneratorLayer*>(node)->internalSelection();
    } else {
        return false;
    }
    if (selection->hasPixelSelection()) {
        KisPaintDeviceSP dev = selection->pixelSelection();

        savePaintDevice(dev, getLocation(node, DOT_PIXEL_SELECTION));
    }
    if (selection->hasShapeSelection()) {
        m_store->pushDirectory();
        m_store->enterDirectory(getLocation(node, DOT_SHAPE_SELECTION));
        KisShapeSelection* shapeSelection = dynamic_cast<KisShapeSelection*>(selection->shapeSelection());
        if (!shapeSelection) {
            m_store->popDirectory();
            return false;
        }

        if (!shapeSelection->saveSelection(m_store)) {
            m_store->popDirectory();
            return false;
        }
        m_store->popDirectory();
    }
    return true;
}

bool KisKraSaveVisitor::saveFilterConfiguration(KisNode* node)
{
    KisNodeFilterInterface *filterInterface =
        dynamic_cast<KisNodeFilterInterface*>(node);

    KisSafeFilterConfigurationSP filter;

    if (filterInterface) {
        filter = filterInterface->filter();
    }

    if (filter) {
        QString location = getLocation(node, DOT_FILTERCONFIG);
        if (m_store->open(location)) {
            QString s = filter->toXML();
            m_store->write(s.toUtf8(), qstrlen(s.toUtf8()));
            m_store->close();
            return true;
        }
    }
    return false;
}

bool KisKraSaveVisitor::saveMetaData(KisNode* node)
{
    if (!node->inherits("KisLayer")) return true;

    KisMetaData::Store* metadata = (static_cast<KisLayer*>(node))->metaData();
    if (metadata->isEmpty()) return true;

    // Serialize all the types of metadata there are
    KisMetaData::IOBackend* backend = KisMetaData::IOBackendRegistry::instance()->get("xmp");
    if (!backend->supportSaving()) {
        dbgFile << "Backend " << backend->id() << " does not support saving.";
        return false;
    }

    QString location = getLocation(node, QString(".") + backend->id() +  DOT_METADATA);
    dbgFile << "going to save " << backend->id() << ", " << backend->name() << " to " << location;

    QBuffer buffer;
    backend->saveTo(metadata, &buffer);

    QByteArray data = buffer.data();
    dbgFile << "\t information size is" << data.size();

    if (data.size() > 0 && m_store->open(location)) {
        m_store->write(data,  data.size());
        m_store->close();
    }
    return true;
}

QString KisKraSaveVisitor::getLocation(KisNode* node, const QString& suffix)
{

    QString location = m_external ? QString() : m_uri;
    Q_ASSERT(m_nodeFileNames.contains(node));
    location += m_name + LAYER_PATH + m_nodeFileNames[node] + suffix;
    return location;
}
