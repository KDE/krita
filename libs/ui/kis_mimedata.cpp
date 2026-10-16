/*
 *  SPDX-FileCopyrightText: 2011 Boudewijn Rempt <boud@valdyas.org>
 *
 * SPDX-License-Identifier: LGPL-2.1-or-later
 */

#include "kis_mimedata.h"
#include "kis_config.h"
#include "kis_node.h"
#include "kis_paint_device.h"
#include "kis_shared_ptr.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_shape_layer.h"
#include "kis_paint_layer.h"
#include "kis_clone_layer.h"
#include "KisDocument.h"
#include "kis_shape_controller.h"
#include "KisPart.h"
#include "kis_layer_utils.h"
#include "kis_generator_registry.h"
#include "KisGlobalResourcesInterface.h"
#include "kis_filter_configuration.h"
#include "kis_generator_layer.h"
#include "kis_selection.h"
#include "kis_node_insertion_adapter.h"
#include "kis_dummies_facade_base.h"
#include "kis_node_dummies_graph.h"
#include "KisImportExportManager.h"
#include "KisImageBarrierLock.h"
#include <commands/kis_image_layer_add_command.h>
#include <commands/kis_image_layer_move_command.h>
#include <kis_processing_applicator.h>


#include <KoProperties.h>
#include <KoStore.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KisDisplayConfig.h>
#include <KisPlatformPluginInterfaceFactory.h>
#include <opengl/KisOpenGLModeProber.h>

#include <QApplication>
#include <QImage>
#include <QByteArray>
#include <QBuffer>
#include <QDomDocument>
#include <QDomElement>
#include <QScreen>
#include <QDir>

namespace {
KisNodeSP safeCopyNode(KisNodeSP node, bool detachClones = true) {
    KisCloneLayerSP cloneLayer = dynamic_cast<KisCloneLayer*>(node.data());
    return cloneLayer && detachClones ?
        KisNodeSP(cloneLayer->reincarnateAsPaintLayer()) : node->clone();
}
}


KisMimeData::KisMimeData(QList<KisNodeSP> nodes, KisImageSP image, const QRect referenceRect, bool forceCopy)
    : QMimeData()
    , m_nodes(nodes)
    , m_forceCopy(forceCopy)
    , m_image(image)
{
    Q_ASSERT(m_nodes.size() > 0);

    KIS_SAFE_ASSERT_RECOVER_RETURN(image);

    m_referenceColorSpace = image->colorSpace();
    m_referenceResX = image->xRes();
    m_referenceResY = image->yRes();

    if (referenceRect.isEmpty()) {
        Q_FOREACH (KisNodeSP node, nodes) {
            m_referenceRect |= KisLayerUtils::recursiveTightNodeVisibleBounds(node);
        }
    } else {
        m_referenceRect = referenceRect;
    }
}

KisDisplayConfig KisMimeData::displayConfigForMimePastes()
{
    KisDisplayConfig result;

    if (KisPlatformPluginInterfaceFactory::instance()->surfaceColorManagedByOS() ||
        KisOpenGLModeProber::instance()->useHDRMode()) {
        /**
         * When the surface is managed, we just put the data in sRGB mode, because
         * the compositor expects untagged data to be sRGB.
         */
        result.profile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
    } else {
        KisConfig cfg(true);
        const KoColorProfile *profile = cfg.displayProfile(KisPortingUtils::getScreenNumberForWidget(QApplication::activeWindow()));
        KIS_SAFE_ASSERT_RECOVER(profile) {
            result.profile = KoColorSpaceRegistry::instance()->p709SRGBProfile();
        }
    }

    result.intent = KoColorConversionTransformation::internalRenderingIntent();
    result.conversionFlags = KoColorConversionTransformation::internalConversionFlags();
    result.isHDR = false;

    return result;
}

void KisMimeData::deepCopyNodes()
{
    KisNodeList newNodes;

    {
        KisImageReadOnlyBarrierLock lock(m_image, std::defer_lock);
        if (m_image) {
            lock.lock();
        }

        Q_FOREACH (KisNodeSP node, m_nodes) {
            KisNodeSP newNode = safeCopyNode(node);
            newNode->setImage(nullptr);

            newNodes << newNode;
        }
    }

    m_nodes = newNodes;
    m_image = 0;
}

QList<KisNodeSP> KisMimeData::nodes() const
{
    return m_nodes;
}

const KoColorSpace* KisMimeData::referenceColorSpace() const
{
    return m_referenceColorSpace;
}

QStringList KisMimeData::formats () const
{
    QStringList f = QMimeData::formats();
    if (m_nodes.size() > 0) {
        f << "application/x-qt-image"
          << "application/x-qt-image-has-alpha"
          << "application/zip"
          << "application/x-krita-node-internal-pointer";
    }
    return f;
}

KisPaintDeviceSP KisMimeData::clipFromKritaLayersImpl(const KoColorSpace *cs, const QRect &referenceImageRect) const
{
    KisNodeList nodes = m_nodes;

    if (nodes.size() > 1) {
        if (cs == nullptr) {
            cs = m_referenceColorSpace;
        }

        if (cs == nullptr) {
            // if no color space provided choose the most used one in the layer data
            QHash<const KoColorSpace *, int> colorSpacesCount;

            Q_FOREACH (KisNodeSP node, nodes) {
                const KoColorSpace *colorSpace = node->colorSpace();
                if (colorSpace != nullptr) {
                    colorSpacesCount.insert(colorSpace, colorSpacesCount.value(colorSpace) + 1);
                }
            }

            int max = 0;
            for (auto it = colorSpacesCount.begin(); it != colorSpacesCount.end(); ++it) {
                if (it.value() > max) {
                    max = it.value();
                    cs = it.key();
                }
            }
        }

        // fallback to rgb8 if no color space found
        // It is likely that this will never be triggered, but it's probably best to handle this just in case
        if (cs == nullptr) {
            cs = KoColorSpaceRegistry::instance()->rgb8();
        }

        KisImageSP tempImage = new KisImage(nullptr,
                                            referenceImageRect.width(),
                                            referenceImageRect.height(),
                                            cs,
                                            "ClipImage");

        tempImage->setResolution(m_referenceResX, m_referenceResY);

        {
            // The nodes are still connected to the original image,
            // so we should lock it before cloning
            KisImageReadOnlyBarrierLock lock(m_image, std::defer_lock);
            if (m_image) {
                lock.lock();
            }

            // If having multiple nodes, the masks are filtered out
            KisLayerUtils::filterMergeableNodes(nodes, false);

            // if nothing is left to paste, just return immediately
            if (nodes.isEmpty()) return nullptr;

            for (KisNodeSP node : nodes) {
                KisNodeSP clonedNode = safeCopyNode(node);

                if (referenceImageRect.x() != 0) {
                    clonedNode->setX(clonedNode->x() - referenceImageRect.x());
                }

                if (referenceImageRect.y() != 0) {
                    clonedNode->setY(clonedNode->y() - referenceImageRect.y());
                }

                /// HACK ALERT: here we just initialize parent image link
                ///             and skip initializing shapeController!
                ///             Ideally, we should call initializeExternalNode()
                ///             instead.
                tempImage->addNode(clonedNode, tempImage->root());
                KIS_SAFE_ASSERT_RECOVER(clonedNode->image() == KisImageWSP(tempImage))
                {
                    clonedNode->setImage(tempImage);
                }
            }
        }
        tempImage->refreshGraphAsync();
        KisLayerUtils::refreshHiddenAreaAsync(tempImage, tempImage->root(), tempImage->bounds());
        tempImage->waitForDone();

        // returns a device without default bounds set!
        return tempImage->projection();
    } else if (!nodes.isEmpty()) {

        KisPaintDeviceSP dev = new KisPaintDevice(*nodes.first()->projection());
        if (!referenceImageRect.topLeft().isNull()) {
            dev->moveTo(dev->offset() - referenceImageRect.topLeft());
        }

        // returns a device without default bounds set!
        return dev;
    }

    return nullptr;
}

KisPaintDeviceSP KisMimeData::clipFromKritaLayers(const KoColorSpace *cs, KisDefaultBoundsBaseSP defaultBounds) const
{
    KisPaintDeviceSP result;

    KisPaintDeviceSP dev = clipFromKritaLayersImpl(cs, defaultBounds->bounds());

    if (dev) {
        result = new KisPaintDevice(*dev);
        result->setDefaultBounds(defaultBounds);
    }
    return result;
}

KisImageSP KisMimeData::clipImageFromKritaLayers(const KoColorSpace *cs) const
{
    KisImageSP resultImage;

    const QRect referenceRect = m_referenceRect;
    if (referenceRect.isEmpty()) return resultImage;

    KisPaintDeviceSP clip = clipFromKritaLayersImpl(cs, referenceRect);

    if (clip) {
        resultImage =
            new KisImage(nullptr, referenceRect.width(), referenceRect.height(), clip->colorSpace(), "clip image");
        resultImage->setResolution(m_referenceResX, m_referenceResY);

        KisPaintLayerSP paintLayer = new KisPaintLayer(resultImage, "layer", OPACITY_OPAQUE_U8, clip);
        resultImage->addNode(paintLayer);
    }

    return resultImage;
}

#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
QVariant KisMimeData::retrieveData(const QString &mimetype, QVariant::Type preferredType) const
#else
QVariant KisMimeData::retrieveData(const QString &mimetype, QMetaType preferredType) const
#endif
{
    /**
     * HACK ALERT:
     *
     * Sometimes Qt requests the data *after* destruction of Krita,
     * we cannot load the nodes in that case, because we need signals
     * and timers. So we just skip serializing.
     */
    if (!QApplication::instance()) return QVariant();


    Q_ASSERT(m_nodes.size() > 0);

    if (mimetype == "application/x-qt-image") {
        KisImageSP image = clipImageFromKritaLayers(nullptr);

        const KisDisplayConfig displayConfig = this->displayConfigForMimePastes();
        QImage result = image->projection()->convertToQImage(displayConfig.profile,
                                                             image->bounds(),
                                                             displayConfig.intent,
                                                             displayConfig.conversionFlags);
        result.setDotsPerMeterX(qRound(image->xRes() * 72.0 / 2.54 * 100.0));
        result.setDotsPerMeterY(qRound(image->yRes() * 72.0 / 2.54 * 100.0));
        return result;
    }
    else if (mimetype == "application/x-qt-image-has-alpha") {
        /**
         * In Krita, when adding an image to the clipboard (via "application/x-qt-image"),
         * we **always** add alpha channel. We need to notify Qt about that separately, to
         * make sure Qt does not request "application/x-qt-image" to build the entire QImage
         * every time we add something to clipboard.
         *
         * [Windows-only] If we don't do that, for biggger imaged Krita may hang up for
         * 500-800ms every time we create a mime data, i.e. press Ctrl+C or start D&D
         * operation.
         *
         * NOTE: this mime type is a custom type that we patch into our version of Qt,
         * it is not present upstream!
         */
        return QByteArray("1");
    }
    else if (mimetype == "application/zip") {
        QByteArray result;

        {
            std::unique_ptr<KisDocument> doc(KisPart::instance()->createTemporaryDocument());
            KisImageSP image = clipImageFromKritaLayers(nullptr);
            doc->setCurrentImage(image, false);

            result = doc->serializeToNativeByteArray();
        }

        return result;

    }
    else if (mimetype == "application/x-krita-node-internal-pointer") {

        QDomDocument doc("krita_internal_node_pointer");
        QDomElement root = doc.createElement("pointer");
        root.setAttribute("application_pid", (qint64)QApplication::applicationPid());
        root.setAttribute("force_copy", m_forceCopy);
        root.setAttribute("image_pointer_value", (qint64)m_image.data());
        doc.appendChild(root);

        Q_FOREACH (KisNodeSP node, m_nodes) {
            QDomElement element = doc.createElement("node");
            element.setAttribute("pointer_value", (qint64)node.data());
            root.appendChild(element);
        }

        return doc.toByteArray();

    }
    else {
        return QMimeData::retrieveData(mimetype, preferredType);
    }
}

void KisMimeData::initializeExternalNode(KisNodeSP *node,
                                         KisImageSP srcImage,
                                         KisImageSP dstImage,
                                         KisShapeController *shapeController)
{
    KisShapeLayer *shapeLayer = dynamic_cast<KisShapeLayer*>(node->data());
    if (shapeLayer) {
        // attach the layer to a new shape controller
        KisShapeLayer *shapeLayer2 = new KisShapeLayer(*shapeLayer, shapeController);

        if (srcImage
            && (!qFuzzyCompare(dstImage->xRes(), srcImage->xRes())
                || !qFuzzyCompare(dstImage->yRes(), srcImage->yRes()))) {

            const QTransform t = QTransform::fromScale(srcImage->xRes() / dstImage->xRes(),
                                                       srcImage->yRes() / dstImage->yRes());

            shapeLayer2->setTransformation(shapeLayer2->transformation() * t);
        }

        *node = shapeLayer2;
    }
}

QList<KisNodeSP> KisMimeData::tryLoadInternalNodes(const QMimeData *data,
                                                   MimeSource mimeSource,
                                                   KisImageSP image,
                                                   KisShapeController *shapeController,
                                                   bool /* IN-OUT */ &copyNode)
{
    QList<KisNodeSP> nodes;
    bool forceCopy = false;
    KisImageSP sourceImage;

    // Qt 4.7 and Qt 5.5 way
    const KisMimeData *mimedata = qobject_cast<const KisMimeData*>(data);
    if (mimedata) {
        nodes = mimedata->nodes();
        forceCopy = mimedata->m_forceCopy;
        sourceImage = mimedata->m_image;
    }

    // Qt 4.8 way
    if (nodes.isEmpty()
        // The user might have some clipboard manager installed, which
        // could store this mime data indefinitely, so the pointers to
        // layers could have stunk already. That is why we can use the
        // internal pointer mime data **only** when it comes from drag
        // and drop event, which is usually not stored anywhere.
        && mimeSource == MimeFromDragAndDrop
        && data->hasFormat("application/x-krita-node-internal-pointer")) {
        QByteArray nodeXml = data->data("application/x-krita-node-internal-pointer");

        QDomDocument doc;
        doc.setContent(nodeXml);

        QDomElement root = doc.documentElement();
        qint64 pid = root.attribute("application_pid").toLongLong();
        forceCopy = root.attribute("force_copy").toInt();

        if (pid == QApplication::applicationPid()) {
            qint64 imagePointerValue = root.attribute("image_pointer_value").toLongLong();
            sourceImage = reinterpret_cast<KisImage*>(imagePointerValue);

            QDomElement e;
            for (e = root.firstChildElement(); !e.isNull(); e = e.nextSiblingElement()) {
                qint64 pointerValue = e.attribute("pointer_value").toLongLong();
                if (pointerValue) {
                    nodes << reinterpret_cast<KisNode*>(pointerValue);
                }
            }
        }
    }

    if (!nodes.isEmpty() && (forceCopy || copyNode || sourceImage != image)) {
        KisImageReadOnlyBarrierLock lock(sourceImage, std::defer_lock);
        if (sourceImage) {
            lock.lock();
        }

        QList<KisNodeSP> clones;
        Q_FOREACH (KisNodeSP node, nodes) {
            node = safeCopyNode(node, sourceImage != image);
            if ((forceCopy || copyNode) && sourceImage == image) {
                KisLayerUtils::addCopyOfNameTag(node);
            }
            initializeExternalNode(&node, sourceImage, image, shapeController);
            clones << node;
        }
        nodes = clones;
        copyNode = true;
    }
    return nodes;
}

QList<KisNodeSP> KisMimeData::loadNonNativeNodes(const QMimeData *data,
                                                 KisImageWSP image)
{
    bool doRecenter = false;
    QList<KisNodeSP> nodes;

    if (nodes.isEmpty() && (data->hasFormat("application/x-color") || data->hasFormat("krita/x-colorsetentry"))) {
        QColor color = data->hasColor() ? qvariant_cast<QColor>(data->colorData()) : QColor(255, 0, 255);
        if (!data->hasColor() && data->hasFormat("krita/x-colorsetentry")) {
            QByteArray byteData = data->data("krita/x-colorsetentry");
            KisSwatch s = KisSwatch::fromByteArray(byteData);
            color = s.color().toQColor();
        }
        KisGeneratorSP generator = KisGeneratorRegistry::instance()->value("color");
        KisFilterConfigurationSP defaultConfig = generator->factoryConfiguration(KisGlobalResourcesInterface::instance());
        defaultConfig->setProperty("color", color);
        defaultConfig->createLocalResourcesSnapshot(KisGlobalResourcesInterface::instance());

        if (image) {
            KisGeneratorLayerSP fillLayer = new KisGeneratorLayer(image, image->nextLayerName(i18n("Fill Layer")),
                                                                  defaultConfig, image->globalSelection());
            nodes << fillLayer;
        }
    }

    if (nodes.isEmpty() && data->hasImage()) {
        QImage qimage = qvariant_cast<QImage>(data->imageData());

        KisPaintDeviceSP device = new KisPaintDevice(KoColorSpaceRegistry::instance()->rgb8());
        device->convertFromQImage(qimage, 0);

        if (image) {
            nodes << new KisPaintLayer(image.data(), image->nextLayerName(), OPACITY_OPAQUE_U8, device);
        }

        doRecenter = true;
    }

    if (!nodes.isEmpty() && doRecenter) {
        const QRect imageBounds = image->bounds();

        Q_FOREACH (KisNodeSP node, nodes) {
            const QRect layerBounds = node->projection()->exactBounds();
            if (doRecenter) {
                QPoint pt = imageBounds.center() - layerBounds.center();
                node->setX(pt.x());
                node->setY(pt.y());
            }
        }
    }

    return nodes;
}

QMimeData* KisMimeData::mimeForLayers(const KisNodeList &nodes, KisImageSP image, bool forceCopy)
{
    KisNodeList inputNodes = nodes;
    KisNodeList sortedNodes;
    KisLayerUtils::sortMergeableNodes(image->root(), inputNodes, sortedNodes);
    if (sortedNodes.isEmpty()) return 0;

    KisMimeData* data = new KisMimeData(sortedNodes, image, QRect(), forceCopy);
    return data;
}

QMimeData* KisMimeData::mimeForLayersDeepCopy(const KisNodeList &nodes, KisImageSP image, const QRect referenceRect, bool forceCopy)
{
    KisNodeList inputNodes = nodes;
    KisNodeList sortedNodes;
    KisLayerUtils::sortMergeableNodes(image->root(), inputNodes, sortedNodes);
    if (sortedNodes.isEmpty()) return 0;

    KisMimeData* data = new KisMimeData(sortedNodes, image, referenceRect, forceCopy);
    data->deepCopyNodes();
    return data;
}

bool nodeAllowsAsChild(KisNodeSP parent, KisNodeList nodes)
{
    bool result = true;
    Q_FOREACH (KisNodeSP node, nodes) {
        if (!parent->allowAsChild(node) || !parent->isEditable(false)) {
            result = false;
            break;
        }
    }
    return result;
}

bool correctNewNodeLocation(KisNodeList nodes,
                            KisNodeDummy* &parentDummy,
                            KisNodeDummy* &aboveThisDummy)
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(parentDummy, false);

    KisNodeSP parentNode = parentDummy->node();
    bool result = true;

    if(!nodeAllowsAsChild(parentDummy->node(), nodes) ||
            (parentDummy->node()->inherits("KisGroupLayer") && parentDummy->node()->collapsed())) {
        aboveThisDummy = parentDummy;
        parentDummy = parentDummy->parent();

        result = (!parentDummy) ? false :
            correctNewNodeLocation(nodes, parentDummy, aboveThisDummy);
    }

    return result;
}

bool KisMimeData::isNodeMimeDataFromSameImage(const QMimeData *data, KisImageSP image)
{
    const KisMimeData *mimedata = qobject_cast<const KisMimeData*>(data);
    return mimedata && mimedata->m_image == image;
}

KisNodeList KisMimeData::loadNodesFast(
    const QMimeData *data,
    MimeSource mimeSource,
    KisImageSP image,
    KisShapeController *shapeController,
    bool &copyNode)
{
    QList<KisNodeSP> nodes =
        KisMimeData::tryLoadInternalNodes(data,
                                          mimeSource,
                                          image,
                                          shapeController,
                                          copyNode /* IN-OUT */);

    if (nodes.isEmpty()) {
        nodes = KisMimeData::loadNonNativeNodes(data, image);
        /**
         * Don't try to move a node originating from another image,
         * just copy it.
         */
        copyNode = true;
    }

    return nodes;
}

KisNodeList KisMimeData::loadNodesFastAndRecenter(const QPoint &preferredCenter, const QMimeData *data, MimeSource mimeSource, KisImageSP image, KisShapeController *shapeController, bool &copyNode)
{
    KisNodeList nodes = loadNodesFast(data, mimeSource, image, shapeController, copyNode);

    Q_FOREACH (KisNodeSP node, nodes) {
        const QRect layerBounds = node->exactBounds();
        const QPoint offset = preferredCenter - layerBounds.center();

        node->setX(node->x() + offset.x());
        node->setY(node->y() + offset.y());
    }

    return nodes;
}

bool KisMimeData::insertMimeLayers(const QMimeData *data,
                                   MimeSource mimeSource,
                                   KisImageSP image,
                                   KisShapeController *shapeController,
                                   KisNodeDummy *parentDummy,
                                   KisNodeDummy *aboveThisDummy,
                                   bool copyNode,
                                   KisNodeInsertionAdapter *nodeInsertionAdapter,
                                   std::optional<QPoint> preferredClipCenter,
                                   KisProcessingApplicator *applicator)
{
    QList<KisNodeSP> nodes = loadNodesFast(data, mimeSource, image, shapeController, copyNode /* IN-OUT */);

    const QRect clipVisualBounds = std::accumulate(nodes.begin(), nodes.end(), QRect(), [](QRect rect, KisNodeSP node) {
        // we don't use exactBounds() here, because the nodes are
        // not connected to any image and may be infinite
        return rect | node->projection()->nonDefaultPixelArea();
    });

    QRect clipBounds;

    if (const KisMimeData *mimedata = qobject_cast<const KisMimeData*>(data)) {
        clipBounds = mimedata->m_referenceRect;
    } else {
        clipBounds = clipVisualBounds;
    }

    std::optional<QPoint> offset;

    if (preferredClipCenter) {
        offset = *preferredClipCenter - clipBounds.center();
    } else if (!image->bounds().intersects(clipVisualBounds)) {
        offset = image->bounds().center() - clipVisualBounds.center();
    }

    if (offset) {
        Q_FOREACH (KisNodeSP node, nodes) {
            KisLayerUtils::recursiveApplyNodes(node, [offset = *offset] (KisNodeSP node){
                const QPoint newOffset(node->x() + offset.x(), node->y() + offset.y());
                node->setX(newOffset.x());
                node->setY(newOffset.y());
            });
        }
    }

    if (nodes.isEmpty()) return false;

    bool result = true;

    if (!correctNewNodeLocation(nodes, parentDummy, aboveThisDummy)) {
        return false;
    }

    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(nodeInsertionAdapter, false);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(parentDummy, false);

    KisNodeSP aboveThisNode = aboveThisDummy ? aboveThisDummy->node() : 0;

    if (!applicator) {
        if (copyNode) {
            nodeInsertionAdapter->addNodes(nodes, parentDummy->node(), aboveThisNode);
        }
        else {
            KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(nodes.first()->graphListener() == image.data(), false);
            nodeInsertionAdapter->moveNodes(nodes, parentDummy->node(), aboveThisNode);
        }
    } else {
        {
            if (copyNode) {
                KisNodeSP prevNode = aboveThisNode;
                Q_FOREACH (KisNodeSP node, nodes) {
                    applicator->applyCommand(
                        new KisImageLayerAddCommand(image, node, parentDummy->node(), prevNode));
                    prevNode = node;
                }
            } else {
                KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(nodes.first()->graphListener() == image.data(), false);

                KisNodeSP prevNode = aboveThisNode;
                Q_FOREACH (KisNodeSP node, nodes) {
                    applicator->applyCommand(
                        new KisImageLayerMoveCommand(image, node, parentDummy->node(), prevNode));
                    prevNode = node;
                }
            }
        }
    }

    const bool hasDelayedNodes =
        std::find_if(nodes.begin(), nodes.end(),
                     [] (KisNodeSP node) {
                         return bool(dynamic_cast<KisDelayedUpdateNodeInterface*>(node.data()));
                     }) != nodes.end();

    if (hasDelayedNodes) {
        /**
         * We have the node juggler running, so it will delay the update of the
         * generator layers that might be included into the paste. To avoid
         * that we should forcefully to make it stop
         */
        image->requestStrokeEnd();
    }

    return result;
}
