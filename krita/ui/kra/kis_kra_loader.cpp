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

#include "kra/kis_kra_loader.h"

#include "kis_kra_tags.h"
#include "kis_kra_utils.h"
#include "kis_kra_load_visitor.h"

#include <KoStore.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorProfile.h>
#include <KoDocumentInfo.h>

#include "kis_doc2.h"
#include "kis_config.h"

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
#include <kis_external_layer_iface.h>
#include <kis_filter_mask.h>
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


using namespace KRA;

struct KisKraLoader::Private
{
public:

    KisDoc2* document;
    QString imageName; // used to be stored in the image, is now in the documentInfo block
    QString imageComment; // used to be stored in the image, is now in the documentInfo block
    QMap<KisNode*, QString> layerFilenames; // temp storage during loading
    int syntaxVersion; // version of the fileformat we are loading
    vKisNodeSP selectedNodes; // the nodes that were active when saving the document.
    QMap<QString, QString> assistantsFilenames;
    QList<KisPaintingAssistant*> assistants;
};

KisKraLoader::KisKraLoader(KisDoc2 * document, int syntaxVersion)
        : m_d(new Private())
{
    m_d->document = document;
    m_d->syntaxVersion = syntaxVersion;
}


KisKraLoader::~KisKraLoader()
{
    delete m_d;
}


KisImageWSP KisKraLoader::loadXML(const KoXmlElement& element)
{
    QString attr;
    KisImageWSP image = 0;
    QString name;
    qint32 width;
    qint32 height;
    QString profileProductName;
    double xres;
    double yres;
    QString colorspacename;
    const KoColorSpace * cs;

    if ((attr = element.attribute(MIME)) == NATIVE_MIMETYPE) {

        if ((m_d->imageName = element.attribute(NAME)).isNull())
            return KisImageWSP(0);

        if ((attr = element.attribute(WIDTH)).isNull()) {
            return KisImageWSP(0);
        }
        width = attr.toInt();

        if ((attr = element.attribute(HEIGHT)).isNull()) {
            return KisImageWSP(0);
        }
        height = attr.toInt();

        m_d->imageComment = element.attribute(DESCRIPTION);

        xres = 100.0 / 72.0;
        if (!(attr = element.attribute(X_RESOLUTION)).isNull()) {
            if (attr.toDouble() > 1.0)
                xres = attr.toDouble() / 72.0;
        }

        yres = 100.0;
        if (!(attr = element.attribute(Y_RESOLUTION)).isNull()) {
            if (attr.toDouble() > 1.0)
                yres = attr.toDouble() / 72.0;
        }

        if ((colorspacename = element.attribute(COLORSPACE_NAME)).isNull()) {
            // An old file: take a reasonable default.
            // Krita didn't support anything else in those
            // days anyway.
            colorspacename = "RGBA";
        }

        // A hack for an old colorspacename
        if (colorspacename  == "Grayscale + Alpha")
            colorspacename  = "GRAYA";

        QString colorspaceModel = KoColorSpaceRegistry::instance()->colorSpaceColorModelId(colorspacename).id();
        QString colorspaceDepth = KoColorSpaceRegistry::instance()->colorSpaceColorDepthId(colorspacename).id();

        if ((profileProductName = element.attribute(PROFILE)).isNull()) {
            // no mention of profile so get default profile
            cs = KoColorSpaceRegistry::instance()->colorSpace(colorspaceModel, colorspaceDepth, "");
        } else {
            cs = KoColorSpaceRegistry::instance()->colorSpace(colorspaceModel, colorspaceDepth, profileProductName);
        }

        if (cs == 0) {
            warnFile << "Could not open colorspace";
            return KisImageWSP(0);
        }

        image = new KisImage(m_d->document->createUndoStore(), width, height, cs, name);
        image->setResolution(xres, yres);
        loadNodes(element, image, const_cast<KisGroupLayer*>(image->rootLayer().data()));

        KoXmlNode child;
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
        if (e.tagName() == "assistants") {
            loadAssistantsList(e);
        }
    }
    return image;
}

void KisKraLoader::loadBinaryData(KoStore * store, KisImageWSP image, const QString & uri, bool external)
{

    // icc profile: if present, this overrides the profile product name loaded in loadXML.
    QString location = external ? QString::null : uri;
    location += m_d->imageName + ICC_PATH;
    if (store->hasFile(location)) {
        store->open(location);
        QByteArray data; data.resize(store->size());
        store->read(data.data(), store->size());
        store->close();
        image->assignImageProfile(KoColorSpaceRegistry::instance()->createColorProfile(image->colorSpace()->colorModelId().id(), image->colorSpace()->colorDepthId().id(), data));
    }


    // Load the layers data: if there is a profile associated with a layer it will be set now.
    KisKraLoadVisitor visitor(image, store, m_d->layerFilenames, m_d->imageName, m_d->syntaxVersion);

    if (external)
        visitor.setExternalUri(uri);

    image->rootLayer()->accept(visitor);



    // annotations
    // exif
    location = external ? QString::null : uri;
    location += m_d->imageName + EXIF_PATH;
    if (store->hasFile(location)) {
        QByteArray data;
        store->open(location);
        data = store->read(store->size());
        store->close();
        image->addAnnotation(KisAnnotationSP(new KisAnnotation("exif", "", data)));
    }


    if (m_d->document->documentInfo()->aboutInfo("title").isNull())
        m_d->document->documentInfo()->setAboutInfo("title", m_d->imageName);
    if (m_d->document->documentInfo()->aboutInfo("comment").isNull())
        m_d->document->documentInfo()->setAboutInfo("comment", m_d->imageComment);
    loadAssistants(store, uri, external);
}

vKisNodeSP KisKraLoader::selectedNodes() const
{
    return m_d->selectedNodes;
}

QList<KisPaintingAssistant *> KisKraLoader::assistants() const
{
    return m_d->assistants;
}

void KisKraLoader::loadAssistants(KoStore *store, const QString &uri, bool external)
{
    QString file_path;
    QString location;
    QMap<int ,KisPaintingAssistantHandleSP> handleMap;
    KisPaintingAssistant* assistant = 0;
    QMap<QString,QString>::const_iterator loadedAssistant = m_d->assistantsFilenames.constBegin();
    while (loadedAssistant != m_d->assistantsFilenames.constEnd()){
        const KisPaintingAssistantFactory* factory = KisPaintingAssistantFactoryRegistry::instance()->get(loadedAssistant.value());
        if (factory) {
            assistant = factory->createPaintingAssistant();
            location = external ? QString::null : uri;
            location += m_d->imageName + ASSISTANTS_PATH;
            file_path = location + loadedAssistant.key();
            assistant->loadXml(store, handleMap, file_path);
            m_d->assistants.append(assistant);
        }
        loadedAssistant++;
    }
}

KisNodeSP KisKraLoader::loadNodes(const KoXmlElement& element, KisImageWSP image, KisNodeSP parent)
{

    KoXmlNode node = element.firstChild();
    KoXmlNode child;

    QDomDocument doc;

    if (!node.isNull()) {

        if (node.isElement()) {

            if (node.nodeName().toUpper() == LAYERS.toUpper() || node.nodeName().toUpper() == MASKS.toUpper()) {
                for (child = node.lastChild(); !child.isNull(); child = child.previousSibling()) {
                    KisNodeSP node = loadNode(child.toElement(), image);
                    if (!node) {
#ifdef __GNUC__
#warning "KisKraLoader::loadNodes: report node load failures back to the user!"
#endif
                    } else {
                        image->nextLayerName(); // Make sure the nameserver is current with the number of nodes.
                        image->addNode(node, parent);
                        if (node->inherits("KisLayer") && child.childNodesCount() > 0) {
                            loadNodes(child.toElement(), image, node);
                        }

                    }
                }
            }
        }
    }

    return parent;
}

KisNodeSP KisKraLoader::loadNode(const KoXmlElement& element, KisImageWSP image)
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
    } else {
        QString colorspacename = element.attribute(COLORSPACE_NAME);
        QString colorspaceModel = KoColorSpaceRegistry::instance()->colorSpaceColorModelId(colorspacename).id();
        QString colorspaceDepth = KoColorSpaceRegistry::instance()->colorSpaceColorDepthId(colorspacename).id();
        dbgFile << "Searching color space: " << colorspacename << colorspaceModel << colorspaceDepth << " for layer: " << name;
        // use default profile - it will be replaced later in completeLoading
        colorSpace = KoColorSpaceRegistry::instance()->colorSpace(colorspaceModel, colorspaceDepth, "");
        dbgFile << "found colorspace" << colorSpace;
    }
    Q_ASSERT(colorSpace);

    bool visible = element.attribute(VISIBLE, "1") == "0" ? false : true;
    bool locked = element.attribute(LOCKED, "0") == "0" ? false : true;
    bool collapsed = element.attribute(COLLAPSED, "0") == "0" ? false : true;

    // Now find out the layer type and do specific handling
    QString nodeType;

    if (m_d->syntaxVersion == 1) {
        nodeType = element.attribute("layertype");
        if (nodeType.isEmpty()) {
            nodeType = PAINT_LAYER;
        }
    } else {
        nodeType = element.attribute(NODE_TYPE);
    }

    Q_ASSERT(!nodeType.isEmpty());
    if (nodeType.isEmpty()) return 0;


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
    else if (nodeType == TRANSPARENCY_MASK)
        node = loadTransparencyMask(element);
    else if (nodeType == SELECTION_MASK)
        node = loadSelectionMask(image, element);
    else
        warnKrita << "Trying to load layer of unsupported type " << nodeType;

    // Loading the node went wrong. Return empty node and leave to
    // upstream to complain to the user
    if (!node) return 0;

    node->setVisible(visible);
    node->setUserLocked(locked);
    node->setCollapsed(collapsed);
    node->setX(x);
    node->setY(y);
    node->setName(name);

    if (! id.isNull())          // if no uuid in file, new one has been generated already
        node->setUuid(id);

    if (node->inherits("KisLayer")) {
        KisLayer* layer           = qobject_cast<KisLayer*>(node.data());
        QBitArray channelFlags    = stringToFlags(element.attribute(CHANNEL_FLAGS, ""), colorSpace->channelCount());
        QString   compositeOpName = element.attribute(COMPOSITE_OP, "normal");

        layer->setChannelFlags(channelFlags);
        layer->setCompositeOp(compositeOpName);
    }

    if (node->inherits("KisPaintLayer")) {
        KisPaintLayer* layer            = qobject_cast<KisPaintLayer*>(node.data());
        QBitArray      channelLockFlags = stringToFlags(element.attribute(CHANNEL_LOCK_FLAGS, ""), colorSpace->channelCount());
        layer->setChannelLockFlags(channelLockFlags);
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

    return node;
}


KisNodeSP KisKraLoader::loadPaintLayer(const KoXmlElement& element, KisImageWSP image,
                                      const QString& name, const KoColorSpace* cs, quint32 opacity)
{
    Q_UNUSED(element);

    QString attr;
    KisPaintLayer* layer;

    QString colorspacename;
    QString profileProductName;

    layer = new KisPaintLayer(image, name, opacity, cs);
    Q_CHECK_PTR(layer);

    // Load exif info
    /*TODO: write and use the legacy stuff to load that exif tag
      for( KoXmlNode node = element.firstChild(); !node.isNull(); node = node.nextSibling() )
      {
      KoXmlElement e = node.toElement();
      if ( !e.isNull() && e.tagName() == "ExifInfo" )
      {
      layer->paintDevice()->exifInfo()->load(e);
      }
      }*/
    // TODO load metadata

    return layer;

}

KisNodeSP KisKraLoader::loadGroupLayer(const KoXmlElement& element, KisImageWSP image,
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

KisNodeSP KisKraLoader::loadAdjustmentLayer(const KoXmlElement& element, KisImageWSP image,
        const QString& name, const KoColorSpace* cs, quint32 opacity)
{
    // XXX: do something with filterversion?
    Q_UNUSED(cs);
    QString attr;
    KisAdjustmentLayer* layer;
    QString filtername;

    if ((filtername = element.attribute(FILTER_NAME)).isNull()) {
        // XXX: Invalid adjustmentlayer! We should warn about it!
        warnFile << "No filter in adjustment layer";
        return 0;
    }

    KisFilterSP f = KisFilterRegistry::instance()->value(filtername);
    if (!f) {
        warnFile << "No filter for filtername" << filtername << "";
        return 0; // XXX: We don't have this filter. We should warn about it!
    }

    KisFilterConfiguration * kfc = f->defaultConfiguration(0);

    // We'll load the configuration and the selection later.
    layer = new KisAdjustmentLayer(image, name, kfc, 0);
    Q_CHECK_PTR(layer);

    layer->setOpacity(opacity);

    return layer;

}


KisNodeSP KisKraLoader::loadShapeLayer(const KoXmlElement& element, KisImageWSP image,
                                      const QString& name, const KoColorSpace* cs, quint32 opacity)
{

    Q_UNUSED(element);
    Q_UNUSED(cs);

    QString attr;

    KisShapeLayer* layer = new KisShapeLayer(0, m_d->document->shapeController(), image, name, opacity);
    Q_CHECK_PTR(layer);

    return layer;

}


KisNodeSP KisKraLoader::loadGeneratorLayer(const KoXmlElement& element, KisImageWSP image,
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

    KisFilterConfiguration * kgc = generator->defaultConfiguration(0);

    // We'll load the configuration and the selection later.
    layer = new KisGeneratorLayer(image, name, kgc, 0);
    Q_CHECK_PTR(layer);

    layer->setOpacity(opacity);

    return layer;

}

KisNodeSP KisKraLoader::loadCloneLayer(const KoXmlElement& element, KisImageWSP image,
                                      const QString& name, const KoColorSpace* cs, quint32 opacity)
{
    Q_UNUSED(cs);

    KisCloneLayer* layer = new KisCloneLayer(0, image, name, opacity);

    KisCloneInfo info;
    if (! (element.attribute(CLONE_FROM_UUID)).isNull()) {
        info = KisCloneInfo(QUuid(element.attribute(CLONE_FROM_UUID)));
    } else {
        if ((element.attribute(CLONE_FROM)).isNull()) {
            return 0;
        } else {
            info = KisCloneInfo(element.attribute(CLONE_FROM));
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

    KisFilterConfiguration * kfc = f->defaultConfiguration(0);

    // We'll load the configuration and the selection later.
    mask = new KisFilterMask();
    mask->setFilter(kfc);
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

KisNodeSP KisKraLoader::loadSelectionMask(KisImageWSP image, const KoXmlElement& element)
{
    Q_UNUSED(element);
    KisSelectionMaskSP mask = new KisSelectionMask(image);
    bool active = element.attribute(ACTIVE, "1") == "0" ? false : true;
    mask->setActive(active);
    Q_CHECK_PTR(mask);

    return mask;
}

void KisKraLoader::loadCompositions(const KoXmlElement& elem, KisImageWSP image)
{
    KoXmlNode child;
    for (child = elem.firstChild(); !child.isNull(); child = child.nextSibling()) {
        KoXmlElement e = child.toElement();
        QString name = e.attribute("name");
        KisLayerComposition* composition = new KisLayerComposition(image, name);
        composition->load(e);
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
