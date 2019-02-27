/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2007
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "kis_kra_loader.h"

#include <QApplication>
#include <QStringList>

#include <QMessageBox>

#include <QUrl>
#include <QBuffer>

#include <KoStore.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorSpaceEngine.h>
#include <KoColorProfile.h>
#include <KoDocumentInfo.h>
#include <KoFileDialog.h>
#include <KisImportExportManager.h>
#include <KoXmlReader.h>
#include <KoStoreDevice.h>
#include <KoResourceServerProvider.h>

#include <filter/kis_filter.h>
#include <filter/kis_filter_registry.h>
#include <generator/kis_generator.h>
#include <generator/kis_generator_layer.h>
#include <generator/kis_generator_registry.h>
#include <kis_adjustment_layer.h>
#include <kis_annotation.h>
#include <kis_base_node.h>
#include <kis_clone_layer.h>
#include <kis_debug.h>
#include <kis_assert.h>
#include <kis_external_layer_iface.h>
#include <kis_filter_mask.h>
#include <kis_transform_mask.h>
#include "lazybrush/kis_colorize_mask.h"
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_name_server.h>
#include <kis_paint_layer.h>
#include <kis_selection.h>
#include <kis_selection_mask.h>
#include <kis_shape_layer.h>
#include <kis_transparency_mask.h>
#include <kis_layer_composition.h>
#include <kis_file_layer.h>
#include <kis_psd_layer_style.h>
#include <kis_psd_layer_style_resource.h>
#include "KisResourceServerProvider.h"
#include "kis_keyframe_channel.h"
#include <kis_filter_configuration.h>
#include "KisReferenceImagesLayer.h"
#include "KisReferenceImage.h"
#include <KoColorSet.h>

#include "KisDocument.h"
#include "kis_config.h"
#include "kis_kra_tags.h"
#include "kis_kra_utils.h"
#include "kis_kra_load_visitor.h"
#include "kis_dom_utils.h"
#include "kis_image_animation_interface.h"
#include "kis_time_range.h"
#include "kis_grid_config.h"
#include "kis_guides_config.h"
#include "kis_image_config.h"
#include "KisProofingConfiguration.h"
#include "kis_layer_properties_icons.h"
#include "kis_node_view_color_scheme.h"
#include "KisMirrorAxisConfig.h"

/*

  Color model id comparison through the ages:

2.4        2.5          2.6         ideal

ALPHA      ALPHA        ALPHA       ALPHAU8

CMYK       CMYK         CMYK        CMYKAU8
           CMYKAF32     CMYKAF32
CMYKA16    CMYKAU16     CMYKAU16

GRAYA      GRAYA        GRAYA       GRAYAU8
GrayF32    GRAYAF32     GRAYAF32
GRAYA16    GRAYAU16     GRAYAU16

LABA       LABA         LABA        LABAU16
           LABAF32      LABAF32
           LABAU8       LABAU8

RGBA       RGBA         RGBA        RGBAU8
RGBA16     RGBA16       RGBA16      RGBAU16
RgbAF32    RGBAF32      RGBAF32
RgbAF16    RgbAF16      RGBAF16

XYZA16     XYZA16       XYZA16      XYZAU16
           XYZA8        XYZA8       XYZAU8
XyzAF16    XyzAF16      XYZAF16
XyzAF32    XYZAF32      XYZAF32

YCbCrA     YCBCRA8      YCBCRA8     YCBCRAU8
YCbCrAU16  YCBCRAU16    YCBCRAU16
           YCBCRF32     YCBCRF32
 */

using namespace KRA;

struct KisKraLoader::Private
{
public:

    KisDocument* document;
    QString imageName; // used to be stored in the image, is now in the documentInfo block
    QString imageComment; // used to be stored in the image, is now in the documentInfo block
    QMap<KisNode*, QString> layerFilenames; // temp storage during loading
    int syntaxVersion; // version of the fileformat we are loading
    vKisNodeSP selectedNodes; // the nodes that were active when saving the document.
    QMap<QString, QString> assistantsFilenames;
    QList<KisPaintingAssistantSP> assistants;
    QMap<KisNode*, QString> keyframeFilenames;
    QVector<QString> paletteFilenames;
    QStringList errorMessages;
    QStringList warningMessages;
};

void convertColorSpaceNames(QString &colorspacename, QString &profileProductName) {
    if (colorspacename  == "Grayscale + Alpha") {
        colorspacename  = "GRAYA";
        profileProductName.clear();
    }
    else if (colorspacename == "RgbAF32") {
        colorspacename = "RGBAF32";
        profileProductName.clear();
    }
    else if (colorspacename == "RgbAF16") {
        colorspacename = "RGBAF16";
        profileProductName.clear();
    }
    else if (colorspacename == "CMYKA16") {
        colorspacename = "CMYKAU16";
    }
    else if (colorspacename == "GrayF32") {
        colorspacename =  "GRAYAF32";
        profileProductName.clear();
    }
    else if (colorspacename == "GRAYA16") {
        colorspacename  = "GRAYAU16";
    }
    else if (colorspacename == "XyzAF16") {
        colorspacename  = "XYZAF16";
        profileProductName.clear();
    }
    else if (colorspacename == "XyzAF32") {
        colorspacename  = "XYZAF32";
        profileProductName.clear();
    }
    else if (colorspacename == "YCbCrA") {
        colorspacename  = "YCBCRA8";
    }
    else if (colorspacename == "YCbCrAU16") {
        colorspacename  = "YCBCRAU16";
    }
}

KisKraLoader::KisKraLoader(KisDocument * document, int syntaxVersion)
        : m_d(new Private())
{
    m_d->document = document;
    m_d->syntaxVersion = syntaxVersion;
}


KisKraLoader::~KisKraLoader()
{
    delete m_d;
}


KisImageSP KisKraLoader::loadXML(const KoXmlElement& element)
{
    QString attr;
    KisImageSP image = 0;
    qint32 width;
    qint32 height;
    QString profileProductName;
    double xres;
    double yres;
    QString colorspacename;
    const KoColorSpace * cs;


    if ((attr = element.attribute(MIME)) == NATIVE_MIMETYPE) {

        if ((m_d->imageName = element.attribute(NAME)).isNull()) {
            m_d->errorMessages << i18n("Image does not have a name.");
            return KisImageSP(0);
        }


        if ((attr = element.attribute(WIDTH)).isNull()) {
            m_d->errorMessages << i18n("Image does not specify a width.");
            return KisImageSP(0);
        }
        width = KisDomUtils::toInt(attr);

        if ((attr = element.attribute(HEIGHT)).isNull()) {
            m_d->errorMessages << i18n("Image does not specify a height.");
            return KisImageSP(0);
        }

        height = KisDomUtils::toInt(attr);

        m_d->imageComment = element.attribute(DESCRIPTION);

        xres = 100.0 / 72.0;
        if (!(attr = element.attribute(X_RESOLUTION)).isNull()) {
            qreal value = KisDomUtils::toDouble(attr);

            if (value > 1.0) {
                xres = value / 72.0;
            }
        }

        yres = 100.0 / 72.0;
        if (!(attr = element.attribute(Y_RESOLUTION)).isNull()) {
            qreal value = KisDomUtils::toDouble(attr);
            if (value > 1.0) {
                yres = value / 72.0;
            }
        }

        if ((colorspacename = element.attribute(COLORSPACE_NAME)).isNull()) {
            // An old file: take a reasonable default.
            // Krita didn't support anything else in those
            // days anyway.
            colorspacename = "RGBA";
        }

        profileProductName = element.attribute(PROFILE);
        // A hack for an old colorspacename
        convertColorSpaceNames(colorspacename, profileProductName);

        QString colorspaceModel = KoColorSpaceRegistry::instance()->colorSpaceColorModelId(colorspacename).id();
        QString colorspaceDepth = KoColorSpaceRegistry::instance()->colorSpaceColorDepthId(colorspacename).id();

        if (profileProductName.isNull()) {
            // no mention of profile so get default profile";
            cs = KoColorSpaceRegistry::instance()->colorSpace(colorspaceModel, colorspaceDepth, "");
        } else {
            cs = KoColorSpaceRegistry::instance()->colorSpace(colorspaceModel, colorspaceDepth, profileProductName);
        }

        if (cs == 0) {
            // try once more without the profile
            cs = KoColorSpaceRegistry::instance()->colorSpace(colorspaceModel, colorspaceDepth, "");
            if (cs == 0) {
                m_d->errorMessages << i18n("Image specifies an unsupported color model: %1.", colorspacename);
                return KisImageSP(0);
            }
        }
        KisProofingConfigurationSP proofingConfig = KisImageConfig(true).defaultProofingconfiguration();
        if (!(attr = element.attribute(PROOFINGPROFILENAME)).isNull()) {
            proofingConfig->proofingProfile = attr;
        }
        if (!(attr = element.attribute(PROOFINGMODEL)).isNull()) {
            proofingConfig->proofingModel = attr;
        }
        if (!(attr = element.attribute(PROOFINGDEPTH)).isNull()) {
            proofingConfig->proofingDepth = attr;
        }
        if (!(attr = element.attribute(PROOFINGINTENT)).isNull()) {
            proofingConfig->intent = (KoColorConversionTransformation::Intent) KisDomUtils::toInt(attr);
        }

        if (!(attr = element.attribute(PROOFINGADAPTATIONSTATE)).isNull()) {
            proofingConfig->adaptationState = KisDomUtils::toDouble(attr);
        }

        if (m_d->document) {
            image = new KisImage(m_d->document->createUndoStore(), width, height, cs, m_d->imageName);
        }
        else {
            image = new KisImage(0, width, height, cs, m_d->imageName);
        }
        image->setResolution(xres, yres);
        loadNodes(element, image, const_cast<KisGroupLayer*>(image->rootLayer().data()));


        KoXmlNode child;
        for (child = element.lastChild(); !child.isNull(); child = child.previousSibling()) {
            KoXmlElement e = child.toElement();

            if(e.tagName() == CANVASPROJECTIONCOLOR) {
                if (e.hasAttribute(COLORBYTEDATA)) {
                    QByteArray colorData = QByteArray::fromBase64(e.attribute(COLORBYTEDATA).toLatin1());
                    KoColor color((const quint8*)colorData.data(), image->colorSpace());
                    image->setDefaultProjectionColor(color);
                }
            }


            if(e.tagName() == GLOBALASSISTANTSCOLOR) {
                if (e.hasAttribute(SIMPLECOLORDATA)) {
                    QString colorData = e.attribute(SIMPLECOLORDATA);
                    m_d->document->setAssistantsGlobalColor(KisDomUtils::qStringToQColor(colorData));
                }
            }


            if(e.tagName()== PROOFINGWARNINGCOLOR) {
                QDomDocument dom;
                KoXml::asQDomElement(dom, e);
                QDomElement eq = dom.firstChildElement();
                proofingConfig->warningColor = KoColor::fromXML(eq.firstChildElement(), Integer8BitsColorDepthID.id());
            }

            if (e.tagName().toLower() == "animation") {
                loadAnimationMetadata(e, image);
            }
        }

        image->setProofingConfiguration(proofingConfig);

        for (child = element.lastChild(); !child.isNull(); child = child.previousSibling()) {
            KoXmlElement e = child.toElement();
            if(e.tagName() == "compositions") {
                loadCompositions(e, image);
            }
        }
    }
    KoXmlNode child;

    for (child = element.lastChild(); !child.isNull(); child = child.previousSibling()) {
        KoXmlElement e = child.toElement();
        if (e.tagName() == "grid") {
            loadGrid(e);
        } else if (e.tagName() == "guides") {
            loadGuides(e);
        } else if (e.tagName() == MIRROR_AXIS) {
            loadMirrorAxis(e);
        } else if (e.tagName() == "assistants") {
            loadAssistantsList(e);
        } else if (e.tagName() == "audio") {
            loadAudio(e, image);
        }
    }

    // reading palettes from XML
    for (child = element.lastChild(); !child.isNull(); child = child.previousSibling()) {
        QDomElement e = child.toElement();
        if (e.tagName() == PALETTES) {
            for (QDomElement paletteElement = e.lastChildElement();
                 !paletteElement.isNull();
                 paletteElement = paletteElement.previousSiblingElement()) {
                QString paletteName = paletteElement.attribute("filename");
                m_d->paletteFilenames.append(paletteName);
            }
            break;
        }
    }


    return image;
}

void KisKraLoader::loadBinaryData(KoStore * store, KisImageSP image, const QString & uri, bool external)
{
    // icc profile: if present, this overrides the profile product name loaded in loadXML.
    QString location = external ? QString() : uri;
    location += m_d->imageName + ICC_PATH;
    if (store->hasFile(location)) {
        if (store->open(location)) {
            QByteArray data; data.resize(store->size());
            bool res = (store->read(data.data(), store->size()) > -1);
            store->close();
            if (res) {
                const KoColorProfile *profile = KoColorSpaceRegistry::instance()->createColorProfile(image->colorSpace()->colorModelId().id(), image->colorSpace()->colorDepthId().id(), data);
                if (profile && profile->valid()) {
                    res = image->assignImageProfile(profile);
                }
                if (!res) {
                    const QString defaultProfileId = KoColorSpaceRegistry::instance()->defaultProfileForColorSpace(image->colorSpace()->id());
                    profile = KoColorSpaceRegistry::instance()->profileByName(defaultProfileId);
                    Q_ASSERT(profile && profile->valid());
                    image->assignImageProfile(profile);
                }
            }
        }
    }
    //load the embed proofing profile, it only needs to be loaded into Krita, not assigned.
    location = external ? QString() : uri;
    location += m_d->imageName + ICC_PROOFING_PATH;
    if (store->hasFile(location)) {
        if (store->open(location)) {
            QByteArray proofingData;
            proofingData.resize(store->size());
            bool proofingProfileRes = (store->read(proofingData.data(), store->size())>-1);
            store->close();

            KisProofingConfigurationSP proofingConfig = image->proofingConfiguration();
            if (!proofingConfig) {
                proofingConfig = KisImageConfig(true).defaultProofingconfiguration();
            }

            if (proofingProfileRes) {
                const KoColorProfile *proofingProfile = KoColorSpaceRegistry::instance()->createColorProfile(proofingConfig->proofingModel, proofingConfig->proofingDepth, proofingData);
                if (proofingProfile->valid()){
                    KoColorSpaceRegistry::instance()->addProfile(proofingProfile);
                }
            }
        }
    }


    // Load the layers data: if there is a profile associated with a layer it will be set now.
    KisKraLoadVisitor visitor(image, store, m_d->document->shapeController(), m_d->layerFilenames, m_d->keyframeFilenames, m_d->imageName, m_d->syntaxVersion);

    if (external) {
        visitor.setExternalUri(uri);
    }

    image->rootLayer()->accept(visitor);
    if (!visitor.errorMessages().isEmpty()) {
        m_d->errorMessages.append(visitor.errorMessages());
    }
    if (!visitor.warningMessages().isEmpty()) {
        m_d->warningMessages.append(visitor.warningMessages());
    }

    // annotations
    // exif
    location = external ? QString() : uri;
    location += m_d->imageName + EXIF_PATH;
    if (store->hasFile(location)) {
        QByteArray data;
        store->open(location);
        data = store->read(store->size());
        store->close();
        image->addAnnotation(KisAnnotationSP(new KisAnnotation("exif", "", data)));
    }


    // layer styles
    location = external ? QString() : uri;
    location += m_d->imageName + LAYER_STYLES_PATH;
    if (store->hasFile(location)) {
        KisPSDLayerStyleCollectionResource *collection =
            new KisPSDLayerStyleCollectionResource("Embedded Styles.asl");

        collection->setName(i18nc("Auto-generated layer style collection name for embedded styles (collection)", "<%1> (embedded)", m_d->imageName));

        KIS_ASSERT_RECOVER_NOOP(!collection->valid());

        store->open(location);
        {
            KoStoreDevice device(store);
            device.open(QIODevice::ReadOnly);

            /**
             * ASL loading code cannot work with non-sequential IO devices,
             * so convert the device beforehand!
             */
            QByteArray buf = device.readAll();
            QBuffer raDevice(&buf);
            raDevice.open(QIODevice::ReadOnly);
            collection->loadFromDevice(&raDevice);
        }
        store->close();

        if (collection->valid()) {
            KoResourceServer<KisPSDLayerStyleCollectionResource> *server = KisResourceServerProvider::instance()->layerStyleCollectionServer();
            server->addResource(collection, false);

            collection->assignAllLayerStyles(image->root());
        } else {
            warnKrita << "WARNING: Couldn't load layer styles library from .kra!";
            delete collection;
        }
    }

    if (m_d->document && m_d->document->documentInfo()->aboutInfo("title").isNull())
        m_d->document->documentInfo()->setAboutInfo("title", m_d->imageName);
    if (m_d->document && m_d->document->documentInfo()->aboutInfo("comment").isNull())
        m_d->document->documentInfo()->setAboutInfo("comment", m_d->imageComment);

    loadAssistants(store, uri, external);
}

void KisKraLoader::loadPalettes(KoStore *store, KisDocument *doc)
{
    QList<KoColorSet*> list;
    Q_FOREACH (const QString &filename, m_d->paletteFilenames) {
        KoColorSet *newPalette = new KoColorSet(filename);
        store->open(m_d->imageName + PALETTE_PATH + filename);
        QByteArray data = store->read(store->size());
        newPalette->fromByteArray(data);
        newPalette->setIsGlobal(false);
        newPalette->setIsEditable(true);
        store->close();
        list.append(newPalette);
    }
    doc->setPaletteList(list);
}

vKisNodeSP KisKraLoader::selectedNodes() const
{
    return m_d->selectedNodes;
}

QList<KisPaintingAssistantSP> KisKraLoader::assistants() const
{
    return m_d->assistants;
}

QStringList KisKraLoader::errorMessages() const
{
    return m_d->errorMessages;
}

QStringList KisKraLoader::warningMessages() const
{
    return m_d->warningMessages;
}


void KisKraLoader::loadAssistants(KoStore *store, const QString &uri, bool external)
{
    QString file_path;
    QString location;
    QMap<int ,KisPaintingAssistantHandleSP> handleMap;
    KisPaintingAssistant* assistant = 0;
    const QColor globalColor = m_d->document->assistantsGlobalColor();

    QMap<QString,QString>::const_iterator loadedAssistant = m_d->assistantsFilenames.constBegin();
    while (loadedAssistant != m_d->assistantsFilenames.constEnd()){
        const KisPaintingAssistantFactory* factory = KisPaintingAssistantFactoryRegistry::instance()->get(loadedAssistant.value());
        if (factory) {
            assistant = factory->createPaintingAssistant();
            location = external ? QString() : uri;
            location += m_d->imageName + ASSISTANTS_PATH;
            file_path = location + loadedAssistant.key();
            assistant->loadXml(store, handleMap, file_path);
            assistant->setAssistantGlobalColorCache(globalColor);

            //If an assistant has too few handles than it should according to it's own setup, just don't load it//
            if (assistant->handles().size()==assistant->numHandles()){
                m_d->assistants.append(toQShared(assistant));
            }
        }
        loadedAssistant++;
    }
}

void KisKraLoader::loadAnimationMetadata(const KoXmlElement &element, KisImageSP image)
{
    QDomDocument qDom;
    KoXml::asQDomElement(qDom, element);
    QDomElement qElement = qDom.firstChildElement();

    float framerate;
    KisTimeRange range;
    int currentTime;

    KisImageAnimationInterface *animation = image->animationInterface();

    if (KisDomUtils::loadValue(qElement, "framerate", &framerate)) {
        animation->setFramerate(framerate);
    }

    if (KisDomUtils::loadValue(qElement, "range", &range)) {
        animation->setFullClipRange(range);
    }

    if (KisDomUtils::loadValue(qElement, "currentTime", &currentTime)) {
        animation->switchCurrentTimeAsync(currentTime);
    }
}

KisNodeSP KisKraLoader::loadNodes(const KoXmlElement& element, KisImageSP image, KisNodeSP parent)
{

    KoXmlNode node = element.firstChild();
    KoXmlNode child;

    if (!node.isNull()) {

        if (node.isElement()) {

            if (node.nodeName().toUpper() == LAYERS.toUpper() || node.nodeName().toUpper() == MASKS.toUpper()) {
                for (child = node.lastChild(); !child.isNull(); child = child.previousSibling()) {
                    KisNodeSP node = loadNode(child.toElement(), image);
                    if (node) {
                        image->nextLayerName(); // Make sure the nameserver is current with the number of nodes.
                        image->addNode(node, parent);
                        if (node->inherits("KisLayer") && KoXml::childNodesCount(child) > 0) {
                            loadNodes(child.toElement(), image, node);
                        }
                    }
                }
            }
        }
    }

    return parent;
}

KisNodeSP KisKraLoader::loadNode(const KoXmlElement& element, KisImageSP image)
{
    // Nota bene: If you add new properties to layers, you should
    // ALWAYS define a default value in case the property is not
    // present in the layer definition: this helps a LOT with backward
    // compatibility.
    QString name = element.attribute(NAME, "No Name");

    QUuid id = QUuid(element.attribute(UUID, QUuid().toString()));

    qint32 x = element.attribute(X, "0").toInt();
    qint32 y = element.attribute(Y, "0").toInt();

    qint32 opacity = element.attribute(OPACITY, QString::number(OPACITY_OPAQUE_U8)).toInt();
    if (opacity < OPACITY_TRANSPARENT_U8) opacity = OPACITY_TRANSPARENT_U8;
    if (opacity > OPACITY_OPAQUE_U8) opacity = OPACITY_OPAQUE_U8;

    const KoColorSpace* colorSpace = 0;
    if ((element.attribute(COLORSPACE_NAME)).isNull()) {
        dbgFile << "No attribute color space for layer: " << name;
        colorSpace = image->colorSpace();
    }
    else {
        QString colorspacename = element.attribute(COLORSPACE_NAME);
        QString profileProductName;

        convertColorSpaceNames(colorspacename, profileProductName);

        QString colorspaceModel = KoColorSpaceRegistry::instance()->colorSpaceColorModelId(colorspacename).id();
        QString colorspaceDepth = KoColorSpaceRegistry::instance()->colorSpaceColorDepthId(colorspacename).id();
        dbgFile << "Searching color space: " << colorspacename << colorspaceModel << colorspaceDepth << " for layer: " << name;
        // use default profile - it will be replaced later in completeLoading

        colorSpace = KoColorSpaceRegistry::instance()->colorSpace(colorspaceModel, colorspaceDepth, "");
        dbgFile << "found colorspace" << colorSpace;
        if (!colorSpace) {
            m_d->warningMessages << i18n("Layer %1 specifies an unsupported color model: %2.", name, colorspacename);
            return 0;
        }
    }

    const bool visible = element.attribute(VISIBLE, "1") == "0" ? false : true;
    const bool locked = element.attribute(LOCKED, "0") == "0" ? false : true;
    const bool collapsed = element.attribute(COLLAPSED, "0") == "0" ? false : true;
    int colorLabelIndex = element.attribute(COLOR_LABEL, "0").toInt();
    QVector<QColor> labels = KisNodeViewColorScheme::instance()->allColorLabels();
    if (colorLabelIndex >= labels.size()) {
        colorLabelIndex = labels.size() - 1;
    }

    // Now find out the layer type and do specific handling
    QString nodeType;

    if (m_d->syntaxVersion == 1) {
        nodeType = element.attribute("layertype");
        if (nodeType.isEmpty()) {
            nodeType = PAINT_LAYER;
        }
    }
    else {
        nodeType = element.attribute(NODE_TYPE);
    }

    if (nodeType.isEmpty()) {
        m_d->warningMessages << i18n("Layer %1 has an unsupported type.", name);
        return 0;
    }


    KisNodeSP node = 0;

    if (nodeType == PAINT_LAYER)
        node = loadPaintLayer(element, image, name, colorSpace, opacity);
    else if (nodeType == GROUP_LAYER)
        node = loadGroupLayer(element, image, name, colorSpace, opacity);
    else if (nodeType == ADJUSTMENT_LAYER)
        node = loadAdjustmentLayer(element, image, name, colorSpace, opacity);
    else if (nodeType == SHAPE_LAYER)
        node = loadShapeLayer(element, image, name, colorSpace, opacity);
    else if (nodeType == GENERATOR_LAYER)
        node = loadGeneratorLayer(element, image, name, colorSpace, opacity);
    else if (nodeType == CLONE_LAYER)
        node = loadCloneLayer(element, image, name, colorSpace, opacity);
    else if (nodeType == FILTER_MASK)
        node = loadFilterMask(element);
    else if (nodeType == TRANSFORM_MASK)
        node = loadTransformMask(element);
    else if (nodeType == TRANSPARENCY_MASK)
        node = loadTransparencyMask(element);
    else if (nodeType == SELECTION_MASK)
        node = loadSelectionMask(image, element);
    else if (nodeType == COLORIZE_MASK)
        node = loadColorizeMask(image, element, colorSpace);
    else if (nodeType == FILE_LAYER)
        node = loadFileLayer(element, image, name, opacity);
    else if (nodeType == REFERENCE_IMAGES_LAYER)
        node = loadReferenceImagesLayer(element, image);
    else {
        m_d->warningMessages << i18n("Layer %1 has an unsupported type: %2.", name, nodeType);
        return 0;
    }

    // Loading the node went wrong. Return empty node and leave to
    // upstream to complain to the user
    if (!node) {
        m_d->warningMessages << i18n("Failure loading layer %1 of type: %2.", name, nodeType);
        return 0;
    }

    node->setVisible(visible, true);
    node->setUserLocked(locked);
    node->setCollapsed(collapsed);
    node->setColorLabelIndex(colorLabelIndex);
    node->setX(x);
    node->setY(y);
    node->setName(name);

    if (! id.isNull())          // if no uuid in file, new one has been generated already
        node->setUuid(id);

    if (node->inherits("KisLayer") || node->inherits("KisColorizeMask")) {
        QString compositeOpName = element.attribute(COMPOSITE_OP, "normal");
        node->setCompositeOpId(compositeOpName);
    }

    if (node->inherits("KisLayer")) {
        KisLayer* layer           = qobject_cast<KisLayer*>(node.data());
        QBitArray channelFlags    = stringToFlags(element.attribute(CHANNEL_FLAGS, ""), colorSpace->channelCount());
        layer->setChannelFlags(channelFlags);

        if (element.hasAttribute(LAYER_STYLE_UUID)) {
            QString uuidString = element.attribute(LAYER_STYLE_UUID);
            QUuid uuid(uuidString);
            if (!uuid.isNull()) {
                KisPSDLayerStyleSP dumbLayerStyle(new KisPSDLayerStyle());
                dumbLayerStyle->setUuid(uuid);
                layer->setLayerStyle(dumbLayerStyle);
            } else {
                warnKrita << "WARNING: Layer style for layer" << layer->name() << "contains invalid UUID" << uuidString;
            }
        }
    }

    if (node->inherits("KisGroupLayer")) {
        if (element.hasAttribute(PASS_THROUGH_MODE)) {
            bool value = element.attribute(PASS_THROUGH_MODE, "0") != "0";

            KisGroupLayer *group = qobject_cast<KisGroupLayer*>(node.data());
            group->setPassThroughMode(value);
        }
    }

    const bool timelineEnabled = element.attribute(VISIBLE_IN_TIMELINE, "0") == "0" ? false : true;
    node->setUseInTimeline(timelineEnabled);

    if (node->inherits("KisPaintLayer")) {
        KisPaintLayer* layer = qobject_cast<KisPaintLayer*>(node.data());
        QBitArray channelLockFlags = stringToFlags(element.attribute(CHANNEL_LOCK_FLAGS, ""), colorSpace->channelCount());
        layer->setChannelLockFlags(channelLockFlags);

        bool onionEnabled = element.attribute(ONION_SKIN_ENABLED, "0") == "0" ? false : true;
        layer->setOnionSkinEnabled(onionEnabled);
    }

    if (element.attribute(FILE_NAME).isNull()) {
        m_d->layerFilenames[node.data()] = name;
    }
    else {
        m_d->layerFilenames[node.data()] = element.attribute(FILE_NAME);
    }

    if (element.hasAttribute("selected") && element.attribute("selected") == "true")  {
        m_d->selectedNodes.append(node);
    }

    if (element.hasAttribute(KEYFRAME_FILE)) {
        m_d->keyframeFilenames.insert(node.data(), element.attribute(KEYFRAME_FILE));
    }

    return node;
}


KisNodeSP KisKraLoader::loadPaintLayer(const KoXmlElement& element, KisImageSP image,
                                      const QString& name, const KoColorSpace* cs, quint32 opacity)
{
    Q_UNUSED(element);
    KisPaintLayer* layer;

    layer = new KisPaintLayer(image, name, opacity, cs);
    Q_CHECK_PTR(layer);
    return layer;

}

KisNodeSP KisKraLoader::loadFileLayer(const KoXmlElement& element, KisImageSP image, const QString& name, quint32 opacity)
{
    QString filename = element.attribute("source", QString());
    if (filename.isNull()) return 0;
    bool scale = (element.attribute("scale", "true")  == "true");
    int scalingMethod = element.attribute("scalingmethod", "-1").toInt();
    if (scalingMethod < 0) {
        if (scale) {
            scalingMethod = KisFileLayer::ToImagePPI;
        }
        else {
            scalingMethod = KisFileLayer::None;
        }
    }

    QString documentPath;
    if (m_d->document) {
        documentPath = m_d->document->url().toLocalFile();
    }
    QFileInfo info(documentPath);
    QString basePath = info.absolutePath();

    QString fullPath = QDir(basePath).filePath(QDir::cleanPath(filename));
    if (!QFileInfo(fullPath).exists()) {

        qApp->setOverrideCursor(Qt::ArrowCursor);
        QString msg = i18nc(
            "@info",
            "The file associated to a file layer with the name \"%1\" is not found.\n\n"
            "Expected path:\n"
            "%2\n\n"
            "Do you want to locate it manually?", name, fullPath);

        int result = QMessageBox::warning(0, i18nc("@title:window", "File not found"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

        if (result == QMessageBox::Yes) {

            KoFileDialog dialog(0, KoFileDialog::OpenFile, "OpenDocument");
            dialog.setMimeTypeFilters(KisImportExportManager::supportedMimeTypes(KisImportExportManager::Import));
            dialog.setDefaultDir(basePath);
            QString url = dialog.filename();

            if (!QFileInfo(basePath).exists()) {
                filename = url;
            } else {
                QDir d(basePath);
                filename = d.relativeFilePath(url);
            }
        }

        qApp->restoreOverrideCursor();
    }

    KisLayer *layer = new KisFileLayer(image, basePath, filename, (KisFileLayer::ScalingMethod)scalingMethod, name, opacity);
    Q_CHECK_PTR(layer);

    return layer;
}

KisNodeSP KisKraLoader::loadGroupLayer(const KoXmlElement& element, KisImageSP image,
                                      const QString& name, const KoColorSpace* cs, quint32 opacity)
{
    Q_UNUSED(element);
    Q_UNUSED(cs);
    QString attr;
    KisGroupLayer* layer;

    layer = new KisGroupLayer(image, name, opacity);
    Q_CHECK_PTR(layer);

    return layer;

}

KisNodeSP KisKraLoader::loadAdjustmentLayer(const KoXmlElement& element, KisImageSP image,
        const QString& name, const KoColorSpace* cs, quint32 opacity)
{
    // XXX: do something with filterversion?
    Q_UNUSED(cs);
    QString attr;
    KisAdjustmentLayer* layer;
    QString filtername;
    QString legacy = filtername;

    if ((filtername = element.attribute(FILTER_NAME)).isNull()) {
        // XXX: Invalid adjustmentlayer! We should warn about it!
        warnFile << "No filter in adjustment layer";
        return 0;
    }

    //get deprecated filters.
    if (filtername=="brightnesscontrast") {
        legacy = filtername;
        filtername = "perchannel";
    }
    if (filtername=="left edge detections"
            || filtername=="right edge detections"
            || filtername=="top edge detections"
            || filtername=="bottom edge detections") {
        legacy = filtername;
        filtername = "edge detection";
    }

    KisFilterSP f = KisFilterRegistry::instance()->value(filtername);
    if (!f) {
        warnFile << "No filter for filtername" << filtername << "";
        return 0; // XXX: We don't have this filter. We should warn about it!
    }

    KisFilterConfigurationSP  kfc = f->defaultConfiguration();
    kfc->setProperty("legacy", legacy);
    if (legacy=="brightnesscontrast") {
        kfc->setProperty("colorModel", cs->colorModelId().id());
    }

    // We'll load the configuration and the selection later.
    layer = new KisAdjustmentLayer(image, name, kfc, 0);
    Q_CHECK_PTR(layer);

    layer->setOpacity(opacity);

    return layer;

}


KisNodeSP KisKraLoader::loadShapeLayer(const KoXmlElement& element, KisImageSP image,
                                      const QString& name, const KoColorSpace* cs, quint32 opacity)
{

    Q_UNUSED(element);
    Q_UNUSED(cs);

    QString attr;
    KoShapeControllerBase * shapeController = 0;
    if (m_d->document) {
        shapeController = m_d->document->shapeController();
    }
    KisShapeLayer* layer = new KisShapeLayer(shapeController, image, name, opacity);
    Q_CHECK_PTR(layer);

    return layer;

}


KisNodeSP KisKraLoader::loadGeneratorLayer(const KoXmlElement& element, KisImageSP image,
        const QString& name, const KoColorSpace* cs, quint32 opacity)
{
    Q_UNUSED(cs);
    // XXX: do something with generator version?
    KisGeneratorLayer* layer;
    QString generatorname = element.attribute(GENERATOR_NAME);

    if (generatorname.isNull()) {
        // XXX: Invalid generator layer! We should warn about it!
        warnFile << "No generator in generator layer";
        return 0;
    }

    KisGeneratorSP generator = KisGeneratorRegistry::instance()->value(generatorname);
    if (!generator) {
        warnFile << "No generator for generatorname" << generatorname << "";
        return 0; // XXX: We don't have this generator. We should warn about it!
    }

    KisFilterConfigurationSP  kgc = generator->defaultConfiguration();

    // We'll load the configuration and the selection later.
    layer = new KisGeneratorLayer(image, name, kgc, 0);
    Q_CHECK_PTR(layer);

    layer->setOpacity(opacity);

    return layer;

}

KisNodeSP KisKraLoader::loadCloneLayer(const KoXmlElement& element, KisImageSP image,
                                      const QString& name, const KoColorSpace* cs, quint32 opacity)
{
    Q_UNUSED(cs);

    KisCloneLayerSP layer = new KisCloneLayer(0, image, name, opacity);

    KisNodeUuidInfo info;
    if (! (element.attribute(CLONE_FROM_UUID)).isNull()) {
        info = KisNodeUuidInfo(QUuid(element.attribute(CLONE_FROM_UUID)));
    } else {
        if ((element.attribute(CLONE_FROM)).isNull()) {
            return 0;
        } else {
            info = KisNodeUuidInfo(element.attribute(CLONE_FROM));
        }
    }
    layer->setCopyFromInfo(info);

    if ((element.attribute(CLONE_TYPE)).isNull()) {
        return 0;
    } else {
        layer->setCopyType((CopyLayerType) element.attribute(CLONE_TYPE).toInt());
    }

    return layer;
}


KisNodeSP KisKraLoader::loadFilterMask(const KoXmlElement& element)
{
    QString attr;
    KisFilterMask* mask;
    QString filtername;

    // XXX: should we check the version?

    if ((filtername = element.attribute(FILTER_NAME)).isNull()) {
        // XXX: Invalid filter layer! We should warn about it!
        warnFile << "No filter in filter layer";
        return 0;
    }

    KisFilterSP f = KisFilterRegistry::instance()->value(filtername);
    if (!f) {
        warnFile << "No filter for filtername" << filtername << "";
        return 0; // XXX: We don't have this filter. We should warn about it!
    }

    KisFilterConfigurationSP  kfc = f->defaultConfiguration();

    // We'll load the configuration and the selection later.
    mask = new KisFilterMask();
    mask->setFilter(kfc);
    Q_CHECK_PTR(mask);

    return mask;
}

KisNodeSP KisKraLoader::loadTransformMask(const KoXmlElement& element)
{
    Q_UNUSED(element);

    KisTransformMask* mask;

    /**
     * We'll load the transform configuration later on a stage
     * of binary data loading
     */
    mask = new KisTransformMask();
    Q_CHECK_PTR(mask);

    return mask;
}

KisNodeSP KisKraLoader::loadTransparencyMask(const KoXmlElement& element)
{
    Q_UNUSED(element);
    KisTransparencyMask* mask = new KisTransparencyMask();
    Q_CHECK_PTR(mask);

    return mask;
}

KisNodeSP KisKraLoader::loadSelectionMask(KisImageSP image, const KoXmlElement& element)
{
    KisSelectionMaskSP mask = new KisSelectionMask(image);
    bool active = element.attribute(ACTIVE, "1") == "0" ? false : true;
    mask->setActive(active);
    Q_CHECK_PTR(mask);

    return mask;
}

KisNodeSP KisKraLoader::loadColorizeMask(KisImageSP image, const KoXmlElement& element, const KoColorSpace *colorSpace)
{
    KisColorizeMaskSP mask = new KisColorizeMask();
    const bool editKeystrokes = element.attribute(COLORIZE_EDIT_KEYSTROKES, "1") == "0" ? false : true;
    const bool showColoring = element.attribute(COLORIZE_SHOW_COLORING, "1") == "0" ? false : true;

    KisLayerPropertiesIcons::setNodeProperty(mask, KisLayerPropertiesIcons::colorizeEditKeyStrokes, editKeystrokes, image);
    KisLayerPropertiesIcons::setNodeProperty(mask, KisLayerPropertiesIcons::colorizeShowColoring, showColoring, image);

    const bool useEdgeDetection = KisDomUtils::toInt(element.attribute(COLORIZE_USE_EDGE_DETECTION, "0"));
    const qreal edgeDetectionSize = KisDomUtils::toDouble(element.attribute(COLORIZE_EDGE_DETECTION_SIZE, "4"));
    const qreal radius = KisDomUtils::toDouble(element.attribute(COLORIZE_FUZZY_RADIUS, "0"));
    const int cleanUp = KisDomUtils::toInt(element.attribute(COLORIZE_CLEANUP, "0"));
    const bool limitToDevice = KisDomUtils::toInt(element.attribute(COLORIZE_LIMIT_TO_DEVICE, "0"));

    mask->setUseEdgeDetection(useEdgeDetection);
    mask->setEdgeDetectionSize(edgeDetectionSize);
    mask->setFuzzyRadius(radius);
    mask->setCleanUpAmount(qreal(cleanUp) / 100.0);
    mask->setLimitToDeviceBounds(limitToDevice);

    delete mask->setColorSpace(colorSpace);
    mask->setImage(image);

    return mask;
}

void KisKraLoader::loadCompositions(const KoXmlElement& elem, KisImageSP image)
{
    KoXmlNode child;

    for (child = elem.firstChild(); !child.isNull(); child = child.nextSibling()) {

        KoXmlElement e = child.toElement();
        QString name = e.attribute("name");
        bool exportEnabled = e.attribute("exportEnabled", "1") == "0" ? false : true;

        KisLayerCompositionSP composition(new KisLayerComposition(image, name));
        composition->setExportEnabled(exportEnabled);

        KoXmlNode value;
        for (value = child.lastChild(); !value.isNull(); value = value.previousSibling()) {
            KoXmlElement e = value.toElement();
            QUuid uuid(e.attribute("uuid"));
            bool visible = e.attribute("visible", "1") == "0" ? false : true;
            composition->setVisible(uuid, visible);
            bool collapsed = e.attribute("collapsed", "1") == "0" ? false : true;
            composition->setCollapsed(uuid, collapsed);
        }

        image->addComposition(composition);
    }
}

void KisKraLoader::loadAssistantsList(const KoXmlElement &elem)
{
    KoXmlNode child;
    int count = 0;
    for (child = elem.firstChild(); !child.isNull(); child = child.nextSibling()) {
        KoXmlElement e = child.toElement();
        QString type = e.attribute("type");
        QString file_name = e.attribute("filename");
        m_d->assistantsFilenames.insert(file_name,type);
        count++;

    }
}

void KisKraLoader::loadGrid(const KoXmlElement& elem)
{
    QDomDocument dom;
    KoXml::asQDomElement(dom, elem);
    QDomElement domElement = dom.firstChildElement();

    KisGridConfig config;
    config.loadDynamicDataFromXml(domElement);
    config.loadStaticData();
    m_d->document->setGridConfig(config);
}

void KisKraLoader::loadGuides(const KoXmlElement& elem)
{
    QDomDocument dom;
    KoXml::asQDomElement(dom, elem);
    QDomElement domElement = dom.firstChildElement();

    KisGuidesConfig guides;
    guides.loadFromXml(domElement);
    m_d->document->setGuidesConfig(guides);
}

void KisKraLoader::loadMirrorAxis(const KoXmlElement &elem)
{
    QDomDocument dom;
    KoXml::asQDomElement(dom, elem);
    QDomElement domElement = dom.firstChildElement();

    KisMirrorAxisConfig mirrorAxis;
    mirrorAxis.loadFromXml(domElement);
    m_d->document->setMirrorAxisConfig(mirrorAxis);
}

void KisKraLoader::loadAudio(const KoXmlElement& elem, KisImageSP image)
{
    QDomDocument dom;
    KoXml::asQDomElement(dom, elem);
    QDomElement qElement = dom.firstChildElement();

    QString fileName;
    if (KisDomUtils::loadValue(qElement, "masterChannelPath", &fileName)) {
        fileName = QDir::toNativeSeparators(fileName);

        QDir baseDirectory = QFileInfo(m_d->document->localFilePath()).absoluteDir();
        fileName = baseDirectory.absoluteFilePath(fileName);

        QFileInfo info(fileName);

        if (!info.exists()) {
            qApp->setOverrideCursor(Qt::ArrowCursor);
            QString msg = i18nc(
                "@info",
                "Audio channel file \"%1\" doesn't exist!\n\n"
                "Expected path:\n"
                "%2\n\n"
                "Do you want to locate it manually?", info.fileName(), info.absoluteFilePath());

            int result = QMessageBox::warning(0, i18nc("@title:window", "File not found"), msg, QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);

            if (result == QMessageBox::Yes) {
                info.setFile(KisImportExportManager::askForAudioFileName(info.absolutePath(), 0));
            }

            qApp->restoreOverrideCursor();
        }

        if (info.exists()) {
            image->animationInterface()->setAudioChannelFileName(info.absoluteFilePath());
        }
    }

    bool audioMuted = false;
    if (KisDomUtils::loadValue(qElement, "audioMuted", &audioMuted)) {
        image->animationInterface()->setAudioMuted(audioMuted);
    }

    qreal audioVolume = 0.5;
    if (KisDomUtils::loadValue(qElement, "audioVolume", &audioVolume)) {
        image->animationInterface()->setAudioVolume(audioVolume);
    }
}

KisNodeSP KisKraLoader::loadReferenceImagesLayer(const KoXmlElement &elem, KisImageSP image)
{
    KisSharedPtr<KisReferenceImagesLayer> layer =
        new KisReferenceImagesLayer(m_d->document->shapeController(), image);

    m_d->document->setReferenceImagesLayer(layer, false);

    for (QDomElement child = elem.firstChildElement(); !child.isNull(); child = child.nextSiblingElement()) {
        if (child.nodeName().toLower() == "referenceimage") {
            auto* reference = KisReferenceImage::fromXml(child);
            layer->addShape(reference);
        }
    }

    return layer;
}
