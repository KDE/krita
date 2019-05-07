/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2005 C. Boemann <cbo@boemann.dk>
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

#include "kis_kra_load_visitor.h"
#include "kis_kra_tags.h"
#include "flake/kis_shape_layer.h"
#include "flake/KisReferenceImagesLayer.h"
#include "KisReferenceImage.h"
#include <KisImportExportManager.h>

#include <QRect>
#include <QBuffer>
#include <QByteArray>
#include <QMessageBox>

#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoFileDialog.h>
#include <KoStore.h>
#include <KoColorSpace.h>
#include <KoShapeControllerBase.h>

// kritaimage
#include <kis_meta_data_io_backend.h>
#include <kis_meta_data_store.h>
#include <kis_types.h>
#include <kis_node_visitor.h>
#include <kis_image.h>
#include <kis_selection.h>
#include <kis_layer.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_adjustment_layer.h>
#include <filter/kis_filter_configuration.h>
#include <kis_datamanager.h>
#include <generator/kis_generator_layer.h>
#include <kis_pixel_selection.h>
#include <kis_clone_layer.h>
#include <kis_filter_mask.h>
#include <kis_transform_mask.h>
#include <kis_transform_mask_params_interface.h>
#include "kis_transform_mask_params_factory_registry.h"
#include <kis_transparency_mask.h>
#include <kis_selection_mask.h>
#include <lazybrush/kis_colorize_mask.h>
#include <lazybrush/kis_lazy_fill_tools.h>
#include "kis_shape_selection.h"
#include "kis_colorize_dom_utils.h"
#include "kis_dom_utils.h"
#include "kis_raster_keyframe_channel.h"
#include "kis_paint_device_frames_interface.h"
#include "kis_filter_registry.h"


using namespace KRA;

QString expandEncodedDirectory(const QString& _intern)
{

    QString intern = _intern;

    QString result;
    int pos;
    while ((pos = intern.indexOf('/')) != -1) {
        if (QChar(intern.at(0)).isDigit())
            result += "part";
        result += intern.left(pos + 1);   // copy numbers (or "pictures") + "/"
        intern = intern.mid(pos + 1);   // remove the dir we just processed
    }

    if (!intern.isEmpty() && QChar(intern.at(0)).isDigit())
        result += "part";
    result += intern;


    return result;
}


KisKraLoadVisitor::KisKraLoadVisitor(KisImageSP image,
                                     KoStore *store,
                                     KoShapeControllerBase *shapeController,
                                     QMap<KisNode *, QString> &layerFilenames,
                                     QMap<KisNode *, QString> &keyframeFilenames,
                                     const QString & name,
                                     int syntaxVersion)
    : KisNodeVisitor()
    , m_image(image)
    , m_store(store)
    , m_external(false)
    , m_layerFilenames(layerFilenames)
    , m_keyframeFilenames(keyframeFilenames)
    , m_name(name)
    , m_shapeController(shapeController)
{
    m_store->pushDirectory();

    if (!m_store->enterDirectory(m_name)) {
        QStringList directories = m_store->directoryList();
        dbgKrita << directories;
        if (directories.size() > 0) {
            dbgFile << "Could not locate the directory, maybe some encoding issue? Grab the first directory, that'll be the image one." << m_name << directories;
            m_name = directories.first();
        }
        else {
            dbgFile << "Could not enter directory" << m_name << ", probably an old-style file with 'part' added.";
            m_name = expandEncodedDirectory(m_name);
        }
    }
    else {
        m_store->popDirectory();
    }
    m_syntaxVersion = syntaxVersion;
}

void KisKraLoadVisitor::setExternalUri(const QString &uri)
{
    m_external = true;
    m_uri = uri;
}

bool KisKraLoadVisitor::visit(KisExternalLayer * layer)
{
    bool result = false;

    if (auto *referencesLayer = dynamic_cast<KisReferenceImagesLayer*>(layer)) {
        Q_FOREACH(KoShape *shape, referencesLayer->shapes()) {
            auto *reference = dynamic_cast<KisReferenceImage*>(shape);
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(reference, false);

            while (!reference->loadImage(m_store)) {
                if (reference->embed()) {
                    m_errorMessages << i18n("Could not load embedded reference image %1 ", reference->internalFile());
                    break;
                } else {
                    QString msg = i18nc(
                        "@info",
                        "A reference image linked to an external file could not be loaded.\n\n"
                        "Path: %1\n\n"
                        "Do you want to select another location?", reference->filename());

                    int locateManually = QMessageBox::warning(0, i18nc("@title:window", "File not found"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

                    QString url;
                    if (locateManually == QMessageBox::Yes) {
                        KoFileDialog dialog(0, KoFileDialog::OpenFile, "OpenDocument");
                        dialog.setMimeTypeFilters(KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import));
                        url = dialog.filename();
                    }

                    if (url.isEmpty()) {
                        break;
                    } else {
                        reference->setFilename(url);
                    }
                }
            }
        }
    } else if (KisShapeLayer *shapeLayer = dynamic_cast<KisShapeLayer*>(layer)) {

        if (!loadMetaData(layer)) {
            return false;
        }

        m_store->pushDirectory();
        m_store->enterDirectory(getLocation(layer, DOT_SHAPE_LAYER)) ;
        result =  shapeLayer->loadLayer(m_store);
        m_store->popDirectory();

    }

    result = visitAll(layer) && result;
    return result;
}

bool KisKraLoadVisitor::visit(KisPaintLayer *layer)
{
    loadNodeKeyframes(layer);

    if (!loadPaintDevice(layer->paintDevice(), getLocation(layer))) {
        return false;
    }
    if (!loadProfile(layer->paintDevice(), getLocation(layer, DOT_ICC))) {
        return false;
    }
    if (!loadMetaData(layer)) {
        return false;
    }

    if (m_syntaxVersion == 1) {
        // Check whether there is a file with a .mask extension in the
        // layer directory, if so, it's an old-style transparency mask
        // that should be converted.
        QString location = getLocation(layer, ".mask");

        if (m_store->open(location)) {

            KisSelectionSP selection = KisSelectionSP(new KisSelection());
            KisPixelSelectionSP pixelSelection = selection->pixelSelection();
            if (!pixelSelection->read(m_store->device())) {
                pixelSelection->disconnect();
            } else {
                KisTransparencyMask* mask = new KisTransparencyMask();
                mask->setSelection(selection);
                m_image->addNode(mask, layer, layer->firstChild());
            }
            m_store->close();
        }
    }
    bool result = visitAll(layer);
    return result;
}

bool KisKraLoadVisitor::visit(KisGroupLayer *layer)
{
    if (*layer->colorSpace() != *m_image->colorSpace()) {
        layer->resetCache(m_image->colorSpace());
    }

    if (!loadMetaData(layer)) {
        return false;
    }

    bool result = visitAll(layer);
    return result;
}

bool KisKraLoadVisitor::visit(KisAdjustmentLayer* layer)
{
    loadNodeKeyframes(layer);

    // Adjustmentlayers are tricky: there's the 1.x style and the 2.x
    // style, which has selections with selection components
    bool result = true;
    if (m_syntaxVersion == 1) {
        KisSelectionSP selection = new KisSelection();
        KisPixelSelectionSP pixelSelection = selection->pixelSelection();
        result = loadPaintDevice(pixelSelection, getLocation(layer, ".selection"));
        layer->setInternalSelection(selection);
    } else if (m_syntaxVersion == 2) {
        result = loadSelection(getLocation(layer), layer->internalSelection());

    } else {
        // We use the default, empty selection
    }

    if (!loadMetaData(layer)) {
        return false;
    }

    loadFilterConfiguration(layer->filter(), getLocation(layer, DOT_FILTERCONFIG));
    fixOldFilterConfigurations(layer->filter());

    result = visitAll(layer);
    return result;
}

bool KisKraLoadVisitor::visit(KisGeneratorLayer *layer)
{
    if (!loadMetaData(layer)) {
        return false;
    }
    bool result = true;

    loadNodeKeyframes(layer);
    result = loadSelection(getLocation(layer), layer->internalSelection());

    // HACK ALERT: we set the same filter again to ensure the layer
    // is correctly updated
    KisFilterConfigurationSP filter = layer->filter();
    result = loadFilterConfiguration(filter.data(), getLocation(layer, DOT_FILTERCONFIG));
    layer->setFilter(filter);

    result = visitAll(layer);
    return result;
}

bool KisKraLoadVisitor::visit(KisCloneLayer *layer)
{
    if (!loadMetaData(layer)) {
        return false;
    }

    // the layer might have already been lazily initialized
    // from the mask loading code
    if (layer->copyFrom()) {
        return true;
    }

    KisNodeSP srcNode = layer->copyFromInfo().findNode(m_image->rootLayer());
    KisLayerSP srcLayer = qobject_cast<KisLayer*>(srcNode.data());
    Q_ASSERT(srcLayer);

    layer->setCopyFrom(srcLayer);

    // Clone layers have no data except for their masks
    bool result = visitAll(layer);
    return result;
}

void KisKraLoadVisitor::initSelectionForMask(KisMask *mask)
{
    KisLayer *cloneLayer = dynamic_cast<KisCloneLayer*>(mask->parent().data());
    if (cloneLayer) {
        // the clone layers should be initialized out of order
        // and lazily, because their original() is still not
        // initialized
        cloneLayer->accept(*this);
    }

    KisLayer *parentLayer = qobject_cast<KisLayer*>(mask->parent().data());
    // the KisKraLoader must have already set the parent for us
    Q_ASSERT(parentLayer);
    mask->initSelection(parentLayer);
}

bool KisKraLoadVisitor::visit(KisFilterMask *mask)
{
    initSelectionForMask(mask);

    loadNodeKeyframes(mask);

    bool result = true;
    result = loadSelection(getLocation(mask), mask->selection());
    result = loadFilterConfiguration(mask->filter(), getLocation(mask, DOT_FILTERCONFIG));
    fixOldFilterConfigurations(mask->filter());
    return result;
}

bool KisKraLoadVisitor::visit(KisTransformMask *mask)
{
    QString location = getLocation(mask, DOT_TRANSFORMCONFIG);
    if (m_store->hasFile(location)) {
        QByteArray data;
        m_store->open(location);
        data = m_store->read(m_store->size());
        m_store->close();
        if (!data.isEmpty()) {
            QDomDocument doc;
            doc.setContent(data);

            QDomElement rootElement = doc.documentElement();

            QDomElement main;

            if (!KisDomUtils::findOnlyElement(rootElement, "main", &main/*, &m_errorMessages*/)) {
                return false;
            }

            QString id = main.attribute("id", "not-valid");

            if (id == "not-valid") {
                m_errorMessages << i18n("Could not load \"id\" of the transform mask");
                return false;
            }

            QDomElement data;

            if (!KisDomUtils::findOnlyElement(rootElement, "data", &data, &m_errorMessages)) {
                return false;
            }

            KisTransformMaskParamsInterfaceSP params =
                KisTransformMaskParamsFactoryRegistry::instance()->createParams(id, data);

            if (!params) {
                m_errorMessages << i18n("Could not create transform mask params");
                return false;
            }

            mask->setTransformParams(params);

            loadNodeKeyframes(mask);
            params->clearChangedFlag();

            return true;
        }
    }

    return false;
}

bool KisKraLoadVisitor::visit(KisTransparencyMask *mask)
{
    initSelectionForMask(mask);

    loadNodeKeyframes(mask);

    return loadSelection(getLocation(mask), mask->selection());
}

bool KisKraLoadVisitor::visit(KisSelectionMask *mask)
{
    initSelectionForMask(mask);
    return loadSelection(getLocation(mask), mask->selection());
}

bool KisKraLoadVisitor::visit(KisColorizeMask *mask)
{
    m_store->pushDirectory();
    QString location = getLocation(mask, DOT_COLORIZE_MASK);
    m_store->enterDirectory(location) ;

    QByteArray data;
    if (!m_store->extractFile("content.xml", data))
        return false;

    QDomDocument doc;
    if (!doc.setContent(data))
        return false;

    QVector<KisLazyFillTools::KeyStroke> strokes;
    if (!KisDomUtils::loadValue(doc.documentElement(), COLORIZE_KEYSTROKES_SECTION, &strokes, mask->colorSpace()))
        return false;

    int i = 0;
    Q_FOREACH (const KisLazyFillTools::KeyStroke &stroke, strokes) {
        const QString fileName = QString("%1_%2").arg(COLORIZE_KEYSTROKE).arg(i++);
        loadPaintDevice(stroke.dev, fileName);
    }

    mask->setKeyStrokesDirect(QList<KisLazyFillTools::KeyStroke>::fromVector(strokes));

    loadPaintDevice(mask->coloringProjection(), COLORIZE_COLORING_DEVICE);
    mask->resetCache();

    m_store->popDirectory();
    return true;
}

QStringList KisKraLoadVisitor::errorMessages() const
{
    return m_errorMessages;
}

QStringList KisKraLoadVisitor::warningMessages() const
{
    return m_warningMessages;
}

struct SimpleDevicePolicy
{
    bool read(KisPaintDeviceSP dev, QIODevice *stream) {
        return dev->read(stream);
    }

    void setDefaultPixel(KisPaintDeviceSP dev, const KoColor &defaultPixel) const {
        return dev->setDefaultPixel(defaultPixel);
    }
};

struct FramedDevicePolicy
{
    FramedDevicePolicy(int frameId)
        :  m_frameId(frameId) {}

    bool read(KisPaintDeviceSP dev, QIODevice *stream) {
        return dev->framesInterface()->readFrame(stream, m_frameId);
    }

    void setDefaultPixel(KisPaintDeviceSP dev, const KoColor &defaultPixel) const {
        return dev->framesInterface()->setFrameDefaultPixel(defaultPixel, m_frameId);
    }

    int m_frameId;
};

bool KisKraLoadVisitor::loadPaintDevice(KisPaintDeviceSP device, const QString& location)
{
    // Layer data
    KisPaintDeviceFramesInterface *frameInterface = device->framesInterface();
    QList<int> frames;

    if (frameInterface) {
        frames = device->framesInterface()->frames();
    }

    if (!frameInterface || frames.count() <= 1) {
        return loadPaintDeviceFrame(device, location, SimpleDevicePolicy());
    } else {
        KisRasterKeyframeChannel *keyframeChannel = device->keyframeChannel();

        for (int i = 0; i < frames.count(); i++) {
            int id = frames[i];
            if (keyframeChannel->frameFilename(id).isEmpty()) {
                m_warningMessages << i18n("Could not find keyframe pixel data for frame %1 in %2.", id, location);
            }
            else {
                Q_ASSERT(!keyframeChannel->frameFilename(id).isEmpty());
                QString frameFilename = getLocation(keyframeChannel->frameFilename(id));
                Q_ASSERT(!frameFilename.isEmpty());

                if (!loadPaintDeviceFrame(device, frameFilename, FramedDevicePolicy(id))) {
                    m_warningMessages << i18n("Could not load keyframe pixel data for frame %1 in %2.", id, location);
                }
            }
        }
    }

    return true;
}

template<class DevicePolicy>
bool KisKraLoadVisitor::loadPaintDeviceFrame(KisPaintDeviceSP device, const QString &location, DevicePolicy policy)
{
    {
        const int pixelSize = device->colorSpace()->pixelSize();
        KoColor color(Qt::transparent, device->colorSpace());

        if (m_store->open(location + ".defaultpixel")) {
            if (m_store->size() == pixelSize) {
                m_store->read((char*)color.data(), pixelSize);
            }

            m_store->close();
        }

        policy.setDefaultPixel(device, color);
    }

    if (m_store->open(location)) {
        if (!policy.read(device, m_store->device())) {
            m_warningMessages << i18n("Could not read pixel data: %1.", location);
            device->disconnect();
            m_store->close();
            return true;
        }
        m_store->close();
    } else {
        m_warningMessages << i18n("Could not load pixel data: %1.", location);
        return true;
    }

    return true;
}


bool KisKraLoadVisitor::loadProfile(KisPaintDeviceSP device, const QString& location)
{

    if (m_store->hasFile(location)) {
        m_store->open(location);
        QByteArray data; data.resize(m_store->size());
        dbgFile << "Data to load: " << m_store->size() << " from " << location << " with color space " << device->colorSpace()->id();
        int read = m_store->read(data.data(), m_store->size());
        dbgFile << "Profile size: " << data.size() << " " << m_store->atEnd() << " " << m_store->device()->bytesAvailable() << " " << read;
        m_store->close();
        // Create a colorspace with the embedded profile
        const KoColorProfile *profile = KoColorSpaceRegistry::instance()->createColorProfile(device->colorSpace()->colorModelId().id(), device->colorSpace()->colorDepthId().id(), data);
        if (device->setProfile(profile)) {
            return true;
        }
    }
    m_warningMessages << i18n("Could not load profile: %1.", location);
    return true;
}

bool KisKraLoadVisitor::loadFilterConfiguration(KisFilterConfigurationSP kfc, const QString& location)
{
    if (m_store->hasFile(location)) {
        QByteArray data;
        m_store->open(location);
        data = m_store->read(m_store->size());
        m_store->close();
        if (!data.isEmpty()) {
            QDomDocument doc;
            doc.setContent(data);
            QDomElement e = doc.documentElement();
            if (e.tagName() == "filterconfig") {
                kfc->fromLegacyXML(e);
            } else {
                kfc->fromXML(e);
            }
            loadDeprecatedFilter(kfc);
            return true;
        }
    }
    m_warningMessages << i18n("Could not filter configuration %1.", location);
    return true;
}

void KisKraLoadVisitor::fixOldFilterConfigurations(KisFilterConfigurationSP kfc)
{
    KisFilterSP filter = KisFilterRegistry::instance()->value(kfc->name());
    KIS_SAFE_ASSERT_RECOVER_RETURN(filter);

    if (!filter->configurationAllowedForMask(kfc)) {
        filter->fixLoadedFilterConfigurationForMasks(kfc);
    }

    KIS_SAFE_ASSERT_RECOVER_NOOP(filter->configurationAllowedForMask(kfc));
}

bool KisKraLoadVisitor::loadMetaData(KisNode* node)
{
    KisLayer* layer = qobject_cast<KisLayer*>(node);
    if (!layer) return true;

    KisMetaData::IOBackend* backend = KisMetaData::IOBackendRegistry::instance()->get("xmp");

    if (!backend || !backend->supportLoading()) {
        if (backend)
            dbgFile << "Backend " << backend->id() << " does not support loading.";
        else
            dbgFile << "Could not load the XMP backend at all";
        return true;
    }

    QString location = getLocation(node, QString(".") + backend->id() +  DOT_METADATA);
    dbgFile << "going to load " << backend->id() << ", " << backend->name() << " from " << location;

    if (m_store->hasFile(location)) {
        QByteArray data;
        m_store->open(location);
        data = m_store->read(m_store->size());
        m_store->close();
        QBuffer buffer(&data);
        if (!backend->loadFrom(layer->metaData(), &buffer)) {
            m_warningMessages << i18n("Could not load metadata for layer %1.", layer->name());
        }
    }
    return true;
}

bool KisKraLoadVisitor::loadSelection(const QString& location, KisSelectionSP dstSelection)
{
    // by default the selection is expected to be fully transparent
    {
        KisPixelSelectionSP pixelSelection = dstSelection->pixelSelection();
        KoColor transparent(Qt::transparent, pixelSelection->colorSpace());
        pixelSelection->setDefaultPixel(transparent);
    }

    // Pixel selection
    bool result = true;
    QString pixelSelectionLocation = location + DOT_PIXEL_SELECTION;
    if (m_store->hasFile(pixelSelectionLocation)) {
        KisPixelSelectionSP pixelSelection = dstSelection->pixelSelection();
        result = loadPaintDevice(pixelSelection, pixelSelectionLocation);
        if (!result) {
            m_warningMessages << i18n("Could not load raster selection %1.", location);
        }
        pixelSelection->invalidateOutlineCache();
    }

    // Shape selection
    QString shapeSelectionLocation = location + DOT_SHAPE_SELECTION;
    if (m_store->hasFile(shapeSelectionLocation + "/content.svg") ||
        m_store->hasFile(shapeSelectionLocation + "/content.xml")) {

        m_store->pushDirectory();
        m_store->enterDirectory(shapeSelectionLocation) ;

        KisShapeSelection* shapeSelection = new KisShapeSelection(m_shapeController, m_image, dstSelection);
        dstSelection->setShapeSelection(shapeSelection);
        result = shapeSelection->loadSelection(m_store);
        m_store->popDirectory();
        if (!result) {
            m_warningMessages << i18n("Could not load vector selection %1.", location);
        }
    }
    return true;
}

QString KisKraLoadVisitor::getLocation(KisNode* node, const QString& suffix)
{
    return getLocation(m_layerFilenames[node], suffix);
}

QString KisKraLoadVisitor::getLocation(const QString &filename, const QString& suffix)
{
    QString location = m_external ? QString() : m_uri;
    location += m_name + LAYER_PATH + filename + suffix;
    return location;
}

void KisKraLoadVisitor::loadNodeKeyframes(KisNode *node)
{
    if (!m_keyframeFilenames.contains(node)) return;

    node->enableAnimation();

    const QString &location = getLocation(m_keyframeFilenames[node]);

    if (!m_store->open(location)) {
        m_errorMessages << i18n("Could not load keyframes from %1.", location);
        return;
    }

    QString errorMsg;
    int errorLine;
    int errorColumn;

    QDomDocument dom;
    bool ok = dom.setContent(m_store->device(), &errorMsg, &errorLine, &errorColumn);
    m_store->close();


    if (!ok) {
        m_errorMessages << i18n("parsing error in the keyframe file %1 at line %2, column %3\nError message: %4", location, errorLine, errorColumn, i18n(errorMsg.toUtf8()));
        return;
    }

    QDomElement root = dom.firstChildElement();

    for (QDomElement child = root.firstChildElement(); !child.isNull(); child = child.nextSiblingElement()) {
        if (child.nodeName().toUpper() == "CHANNEL") {
            QString id = child.attribute("name");

            KisKeyframeChannel *channel = node->getKeyframeChannel(id, true);

            if (!channel) {
                m_warningMessages << i18n("unknown keyframe channel type: %1 in %2", id, location);
                continue;
            }

            channel->loadXML(child);
        }
    }
}

void KisKraLoadVisitor::loadDeprecatedFilter(KisFilterConfigurationSP cfg)
{
    if (cfg->getString("legacy") == "left edge detections") {
        cfg->setProperty("horizRadius", 1);
        cfg->setProperty("vertRadius", 1);
        cfg->setProperty("type", "prewitt");
        cfg->setProperty("output", "yFall");
        cfg->setProperty("lockAspect", true);
        cfg->setProperty("transparency", false);
    } else if (cfg->getString("legacy") == "right edge detections") {
        cfg->setProperty("horizRadius", 1);
        cfg->setProperty("vertRadius", 1);
        cfg->setProperty("type", "prewitt");
        cfg->setProperty("output", "yGrowth");
        cfg->setProperty("lockAspect", true);
        cfg->setProperty("transparency", false);
    } else if (cfg->getString("legacy") == "top edge detections") {
        cfg->setProperty("horizRadius", 1);
        cfg->setProperty("vertRadius", 1);
        cfg->setProperty("type", "prewitt");
        cfg->setProperty("output", "xGrowth");
        cfg->setProperty("lockAspect", true);
        cfg->setProperty("transparency", false);
    } else if (cfg->getString("legacy") == "bottom edge detections") {
        cfg->setProperty("horizRadius", 1);
        cfg->setProperty("vertRadius", 1);
        cfg->setProperty("type", "prewitt");
        cfg->setProperty("output", "xFall");
        cfg->setProperty("lockAspect", true);
        cfg->setProperty("transparency", false);
    }
}
