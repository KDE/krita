/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2005 C. Boemann <cbo@boemann.dk>
 *  SPDX-FileCopyrightText: 2007-2008 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_kra_save_visitor.h"
#include "kis_kra_tags.h"

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
#include <KisReferenceImagesLayer.h>
#include <KisReferenceImage.h>
#include <kis_file_layer.h>
#include <kis_clone_layer.h>
#include <kis_mask.h>
#include <kis_filter_mask.h>
#include <kis_transform_mask.h>
#include <kis_transform_mask_params_interface.h>
#include <kis_transparency_mask.h>
#include <kis_selection_mask.h>
#include "lazybrush/kis_colorize_mask.h"
#include <kis_selection_component.h>
#include <kis_pixel_selection.h>
#include <kis_meta_data_store.h>
#include <kis_meta_data_io_backend.h>

#include "kis_config.h"
#include "kis_store_paintdevice_writer.h"
#include "flake/kis_shape_selection.h"

#include "kis_raster_keyframe_channel.h"
#include "kis_paint_device_frames_interface.h"

#include "lazybrush/kis_lazy_fill_tools.h"
#include <KoStoreDevice.h>
#include "kis_colorize_dom_utils.h"
#include "kis_dom_utils.h"


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
    if (auto* referencesLayer = dynamic_cast<KisReferenceImagesLayer*>(layer)) {
        result = true;
        QList <KoShape *> shapes = referencesLayer->shapes();
        std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);
        Q_FOREACH(KoShape *shape, shapes) {
            auto *reference = dynamic_cast<KisReferenceImage*>(shape);
            KIS_ASSERT_RECOVER_RETURN_VALUE(reference, false);
            bool saved = reference->saveImage(m_store);
            if (!saved) {
                m_errorMessages << i18n("Failed to save reference image %1.", reference->internalFile());
                result = false;
            }
        }
    }
    else if (KisShapeLayer *shapeLayer = dynamic_cast<KisShapeLayer*>(layer)) {
        if (!saveMetaData(layer)) {
            m_errorMessages << i18n("Failed to save the metadata for layer %1.", layer->name());
            return false;
        }
        m_store->pushDirectory();
        QString location = getLocation(layer, DOT_SHAPE_LAYER);
        result = m_store->enterDirectory(location);
        if (!result) {
            m_errorMessages << i18n("Failed to open %1.", location);
        }
        else {
            result = shapeLayer->saveLayer(m_store);
            m_store->popDirectory();
        }
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
        m_errorMessages << i18n("Failed to save filter mask %1. It has no filter.", mask->name());
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
        bool retval = m_store->write(a) == a.size();

        if (!retval) {
            warnFile << "Could not write transform mask configuration";
        }
        if (!m_store->close()) {
            warnFile << "Could not close store after writing transform mask configuration";
            retval = false;
        }
        return retval;
    }
    return false;
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

bool KisKraSaveVisitor::visit(KisColorizeMask *mask)
{
    m_store->pushDirectory();
    QString location = getLocation(mask, DOT_COLORIZE_MASK);
    bool result = m_store->enterDirectory(location);

    if (!result) {
        m_errorMessages << i18n("Failed to open %1.", location);
        return false;
    }

    if (!m_store->open("content.xml"))
        return false;

    KoStoreDevice storeDev(m_store);

    QDomDocument doc("doc");
    QDomElement root = doc.createElement("colorize");
    doc.appendChild(root);
    KisDomUtils::saveValue(&root, COLORIZE_KEYSTROKES_SECTION, QVector<KisLazyFillTools::KeyStroke>::fromList(mask->fetchKeyStrokesDirect()));

    QTextStream stream(&storeDev);
    stream << doc;

    if (!m_store->close())
        return false;

    int i = 0;
    Q_FOREACH (const KisLazyFillTools::KeyStroke &stroke, mask->fetchKeyStrokesDirect()) {
        const QString fileName = QString("%1_%2").arg(COLORIZE_KEYSTROKE).arg(i++);
        savePaintDevice(stroke.dev, fileName);
    }

    savePaintDevice(mask->coloringProjection(), COLORIZE_COLORING_DEVICE);
    saveIccProfile(mask, mask->colorSpace()->profile());

    m_store->popDirectory();

    return true;
}

QStringList KisKraSaveVisitor::errorMessages() const
{
    return m_errorMessages;
}

struct SimpleDevicePolicy
{
    bool write(KisPaintDeviceSP dev, KisPaintDeviceWriter &store) {
        return dev->write(store);
    }

    KoColor defaultPixel(KisPaintDeviceSP dev) const {
        return dev->defaultPixel();
    }
};

struct FramedDevicePolicy
{
    FramedDevicePolicy(int frameId)
        :  m_frameId(frameId) {}

    bool write(KisPaintDeviceSP dev, KisPaintDeviceWriter &store) {
        return dev->framesInterface()->writeFrame(store, m_frameId);
    }

    KoColor defaultPixel(KisPaintDeviceSP dev) const {
        return dev->framesInterface()->frameDefaultPixel(m_frameId);
    }

    int m_frameId;
};

bool KisKraSaveVisitor::savePaintDevice(KisPaintDeviceSP device,
                                        QString location)
{
    // Layer data
    KisConfig cfg(true);
    m_store->setCompressionEnabled(cfg.compressKra());

    KisPaintDeviceFramesInterface *frameInterface = device->framesInterface();
    QList<int> frames;

    if (frameInterface) {
        frames = frameInterface->frames();
    }

    if (!frameInterface || frames.count() <= 1) {
        savePaintDeviceFrame(device, location, SimpleDevicePolicy());
    } else {
        KisRasterKeyframeChannel *keyframeChannel = device->keyframeChannel();

        for (int i = 0; i < frames.count(); i++) {
            int id = frames[i];

            QString frameFilename = getLocation(keyframeChannel->frameFilename(id));
            Q_ASSERT(!frameFilename.isEmpty());

            if (!savePaintDeviceFrame(device, frameFilename, FramedDevicePolicy(id))) {
                return false;
            }
        }
    }

    m_store->setCompressionEnabled(true);
    return true;
}


template<class DevicePolicy>
bool KisKraSaveVisitor::savePaintDeviceFrame(KisPaintDeviceSP device, QString location, DevicePolicy policy)
{
    if (m_store->open(location)) {
        if (!policy.write(device, *m_writer)) {
            device->disconnect();
            m_store->close();
            return false;
        }

        m_store->close();
    }
    if (m_store->open(location + ".defaultpixel")) {
        m_store->write((char*)policy.defaultPixel(device).data(), device->colorSpace()->pixelSize());
        m_store->close();
    }

    return true;
}

bool KisKraSaveVisitor::saveAnnotations(KisLayer* layer)
{
    if (!layer) return false;
    if (!layer->paintDevice()) return false;
    if (!layer->paintDevice()->colorSpace()) return false;

    if (layer->paintDevice()->colorSpace()->profile()) {
        return saveIccProfile(layer, layer->paintDevice()->colorSpace()->profile());
    }
    return true;

}

bool KisKraSaveVisitor::saveIccProfile(KisNode *node, const KoColorProfile *profile)
{
    if (profile) {
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
            if (m_store->open(getLocation(node, DOT_ICC))) {
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

    bool retval = true;

    if (selection->hasNonEmptyPixelSelection()) {
        KisPaintDeviceSP dev = selection->pixelSelection();
        if (!savePaintDevice(dev, getLocation(node, DOT_PIXEL_SELECTION))) {
            m_errorMessages << i18n("Failed to save the pixel selection data for layer %1.", node->name());
            retval = false;
        }
    }
    if (selection->hasNonEmptyShapeSelection()) {
        m_store->pushDirectory();
        retval = m_store->enterDirectory(getLocation(node, DOT_SHAPE_SELECTION));
        if (retval) {
            KisShapeSelection* shapeSelection = dynamic_cast<KisShapeSelection*>(selection->shapeSelection());
            if (!shapeSelection) {
                retval = false;
            }

            if (retval && !shapeSelection->saveSelection(m_store)) {
                m_errorMessages << i18n("Failed to save the vector selection data for layer %1.", node->name());
                retval = false;
            }
        }
        m_store->popDirectory();
    }
    return retval;
}

bool KisKraSaveVisitor::saveFilterConfiguration(KisNode* node)
{
    KisNodeFilterInterface *filterInterface =
            dynamic_cast<KisNodeFilterInterface*>(node);

    KisFilterConfigurationSP filter;

    if (filterInterface) {
        filter = filterInterface->filter();
    }

    bool retval = false;

    if (filter) {
        QString location = getLocation(node, DOT_FILTERCONFIG);
        if (m_store->open(location)) {
            QString s = filter->toXML();
            retval = (m_store->write(s.toUtf8(), qstrlen(s.toUtf8())) == qstrlen(s.toUtf8())); m_store->close();
        }
    }
    return retval;
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
    // not that the metadata backends every return anything but true...
    bool retval = backend->saveTo(metadata, &buffer);

    if (!retval) {
        m_errorMessages << i18n("The metadata backend failed to save the metadata for %1", node->name());
    }
    else {
        QByteArray data = buffer.data();
        dbgFile << "\t information size is" << data.size();

        if (data.size() > 0 && m_store->open(location)) {
            retval = m_store->write(data,  data.size());
            m_store->close();
        }
        if (!retval) {
            m_errorMessages << i18n("Could not write for %1 metadata to the file.", node->name());
        }
    }
    return retval;
}

QString KisKraSaveVisitor::getLocation(KisNode* node, const QString& suffix)
{

    Q_ASSERT(m_nodeFileNames.contains(node));
    return getLocation(m_nodeFileNames[node], suffix);
}

QString KisKraSaveVisitor::getLocation(const QString &filename, const QString& suffix)
{
    QString location = m_external ? QString() : m_uri;
    location += m_name + LAYER_PATH + filename + suffix;
    return location;
}
