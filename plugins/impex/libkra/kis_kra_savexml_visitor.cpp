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

#include "kis_kra_savexml_visitor.h"
#include "kis_kra_tags.h"
#include "kis_kra_utils.h"
#include "kis_layer_properties_icons.h"

#include <QTextStream>
#include <QDir>

#include <KoProperties.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>

#include <kis_debug.h>
#include <filter/kis_filter_configuration.h>
#include <generator/kis_generator_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_clone_layer.h>
#include <kis_filter_mask.h>
#include <kis_transform_mask.h>
#include <kis_group_layer.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_paint_device.h>
#include <kis_paint_layer.h>
#include <kis_selection_mask.h>
#include <kis_shape_layer.h>
#include <kis_transparency_mask.h>
#include <lazybrush/kis_colorize_mask.h>
#include <kis_file_layer.h>
#include <kis_psd_layer_style.h>
#include <KisReferenceImage.h>
#include <KisReferenceImagesLayer.h>
#include "kis_keyframe_channel.h"
#include "kis_dom_utils.h"

using namespace KRA;

KisSaveXmlVisitor::KisSaveXmlVisitor(QDomDocument doc, const QDomElement & element, quint32 &count, const QString &url, bool root)
    : KisNodeVisitor()
    , m_doc(doc)
    , m_count(count)
    , m_url(url)
    , m_root(root)
{
    Q_ASSERT(!element.isNull());
    m_elem = element;
}

void KisSaveXmlVisitor::setSelectedNodes(vKisNodeSP selectedNodes)
{
    m_selectedNodes = selectedNodes;
}

QStringList KisSaveXmlVisitor::errorMessages() const
{
    return m_errorMessages;
}

bool KisSaveXmlVisitor::visit(KisExternalLayer * layer)
{
    if (layer->inherits("KisReferenceImagesLayer")) {
        return saveReferenceImagesLayer(layer);
    } else if (layer->inherits("KisShapeLayer")) {
        QDomElement layerElement = m_doc.createElement(LAYER);
        saveLayer(layerElement, SHAPE_LAYER, layer);
        m_elem.appendChild(layerElement);
        m_count++;
        return saveMasks(layer, layerElement);
    }
    else if (layer->inherits("KisFileLayer")) {
        QDomElement layerElement = m_doc.createElement(LAYER);
        saveLayer(layerElement, FILE_LAYER, layer);

        KisFileLayer *fileLayer = dynamic_cast<KisFileLayer*>(layer);

        QString path = fileLayer->path();

        QDir d(QFileInfo(m_url).absolutePath());

        layerElement.setAttribute("source", d.relativeFilePath(path));

        if (fileLayer->scalingMethod() == KisFileLayer::ToImagePPI) {
            layerElement.setAttribute("scale", "true");
        }
        else {
            layerElement.setAttribute("scale", "false");
        }
        layerElement.setAttribute("scalingmethod", (int)fileLayer->scalingMethod());
        layerElement.setAttribute(COLORSPACE_NAME, layer->original()->colorSpace()->id());

        m_elem.appendChild(layerElement);
        m_count++;
        return saveMasks(layer, layerElement);
    }
    return false;
}

QDomElement KisSaveXmlVisitor::savePaintLayerAttributes(KisPaintLayer *layer, QDomDocument &doc)
{
    QDomElement element = doc.createElement(LAYER);
    saveLayer(element, PAINT_LAYER, layer);
    element.setAttribute(CHANNEL_LOCK_FLAGS, flagsToString(layer->channelLockFlags()));
    element.setAttribute(COLORSPACE_NAME, layer->paintDevice()->colorSpace()->id());

    element.setAttribute(ONION_SKIN_ENABLED, layer->onionSkinEnabled());
    element.setAttribute(VISIBLE_IN_TIMELINE, layer->useInTimeline());

    return element;
}

void KisSaveXmlVisitor::loadPaintLayerAttributes(const QDomElement &el, KisPaintLayer *layer)
{
    loadLayerAttributes(el, layer);

    if (el.hasAttribute(CHANNEL_LOCK_FLAGS)) {
        layer->setChannelLockFlags(stringToFlags(el.attribute(CHANNEL_LOCK_FLAGS)));
    }
}

bool KisSaveXmlVisitor::visit(KisPaintLayer *layer)
{
    QDomElement layerElement = savePaintLayerAttributes(layer, m_doc);
    m_elem.appendChild(layerElement);
    m_count++;
    return saveMasks(layer, layerElement);
}

bool KisSaveXmlVisitor::visit(KisGroupLayer *layer)
{
    QDomElement layerElement;

    if (m_root) // if this is the root we fake so not to save it
        layerElement = m_elem;
    else {
        layerElement = m_doc.createElement(LAYER);
        saveLayer(layerElement, GROUP_LAYER, layer);
        layerElement.setAttribute(PASS_THROUGH_MODE, layer->passThroughMode());
        m_elem.appendChild(layerElement);
    }
    QDomElement elem = m_doc.createElement(LAYERS);
    Q_ASSERT(!layerElement.isNull());
    layerElement.appendChild(elem);
    KisSaveXmlVisitor visitor(m_doc, elem, m_count, m_url, false);
    visitor.setSelectedNodes(m_selectedNodes);
    m_count++;
    bool success = visitor.visitAllInverse(layer);

    m_errorMessages.append(visitor.errorMessages());
    if (!m_errorMessages.isEmpty()) {
        return false;
    }

    QMapIterator<const KisNode*, QString> i(visitor.nodeFileNames());
    while (i.hasNext()) {
        i.next();
        m_nodeFileNames[i.key()] = i.value();
    }

    i = QMapIterator<const KisNode*, QString>(visitor.keyframeFileNames());
    while (i.hasNext()) {
        i.next();
        m_keyframeFileNames[i.key()] = i.value();
    }

    return success;
}

bool KisSaveXmlVisitor::visit(KisAdjustmentLayer* layer)
{
    if (!layer->filter()) {
        return false;
    }
    QDomElement layerElement = m_doc.createElement(LAYER);
    saveLayer(layerElement, ADJUSTMENT_LAYER, layer);
    layerElement.setAttribute(FILTER_NAME, layer->filter()->name());
    layerElement.setAttribute(FILTER_VERSION, layer->filter()->version());
    m_elem.appendChild(layerElement);

    m_count++;
    return saveMasks(layer, layerElement);
}

bool KisSaveXmlVisitor::visit(KisGeneratorLayer *layer)
{
    QDomElement layerElement = m_doc.createElement(LAYER);
    saveLayer(layerElement, GENERATOR_LAYER, layer);
    layerElement.setAttribute(GENERATOR_NAME, layer->filter()->name());
    layerElement.setAttribute(GENERATOR_VERSION, layer->filter()->version());
    m_elem.appendChild(layerElement);

    m_count++;
    return saveMasks(layer, layerElement);
}

bool KisSaveXmlVisitor::visit(KisCloneLayer *layer)
{
    QDomElement layerElement = m_doc.createElement(LAYER);
    saveLayer(layerElement, CLONE_LAYER, layer);
    layerElement.setAttribute(CLONE_FROM, layer->copyFromInfo().name());
    layerElement.setAttribute(CLONE_FROM_UUID, layer->copyFromInfo().uuid().toString());
    layerElement.setAttribute(CLONE_TYPE, layer->copyType());
    m_elem.appendChild(layerElement);

    m_count++;
    return saveMasks(layer, layerElement);
}

bool KisSaveXmlVisitor::visit(KisFilterMask *mask)
{
    Q_ASSERT(mask);
    if (!mask->filter()) {
        return false;
    }
    QDomElement el = m_doc.createElement(MASK);
    saveMask(el, FILTER_MASK, mask);
    el.setAttribute(FILTER_NAME, mask->filter()->name());
    el.setAttribute(FILTER_VERSION, mask->filter()->version());

    m_elem.appendChild(el);

    m_count++;
    return true;
}

bool KisSaveXmlVisitor::visit(KisTransformMask *mask)
{
    Q_ASSERT(mask);

    QDomElement el = m_doc.createElement(MASK);
    saveMask(el, TRANSFORM_MASK, mask);

    m_elem.appendChild(el);

    m_count++;
    return true;
}

bool KisSaveXmlVisitor::visit(KisTransparencyMask *mask)
{
    Q_ASSERT(mask);
    QDomElement el = m_doc.createElement(MASK);
    saveMask(el, TRANSPARENCY_MASK, mask);
    m_elem.appendChild(el);
    m_count++;
    return true;
}

bool KisSaveXmlVisitor::visit(KisColorizeMask *mask)
{
    Q_ASSERT(mask);
    QDomElement el = m_doc.createElement(MASK);
    saveMask(el, COLORIZE_MASK, mask);
    m_elem.appendChild(el);
    m_count++;
    return true;
}

bool KisSaveXmlVisitor::visit(KisSelectionMask *mask)
{
    Q_ASSERT(mask);

    QDomElement el = m_doc.createElement(MASK);
    saveMask(el, SELECTION_MASK, mask);
    m_elem.appendChild(el);
    m_count++;
    return true;
}


void KisSaveXmlVisitor::loadLayerAttributes(const QDomElement &el, KisLayer *layer)
{
    if (el.hasAttribute(NAME)) {
        QString layerName = el.attribute(NAME);
        if (layerName != layer->name()) {
            // Make the EXR layername leading in case of conflicts
            layer->setName(layerName);
        }
    }

    if (el.hasAttribute(CHANNEL_FLAGS)) {
        layer->setChannelFlags(stringToFlags(el.attribute(CHANNEL_FLAGS)));
    }

    if (el.hasAttribute(OPACITY)) {
        layer->setOpacity(el.attribute(OPACITY).toInt());
    }

    if (el.hasAttribute(COMPOSITE_OP)) {
        layer->setCompositeOpId(el.attribute(COMPOSITE_OP));
    }

    if (el.hasAttribute(VISIBLE)) {
        layer->setVisible(el.attribute(VISIBLE).toInt());
    }

    if (el.hasAttribute(LOCKED)) {
        layer->setUserLocked(el.attribute(LOCKED).toInt());
    }

    if (el.hasAttribute(X)) {
        layer->setX(el.attribute(X).toInt());
    }

    if (el.hasAttribute(Y)) {
        layer->setY(el.attribute(Y).toInt());
    }

    if (el.hasAttribute(UUID)) {
        layer->setUuid(el.attribute(UUID));
    }

    if (el.hasAttribute(COLLAPSED)) {
        layer->setCollapsed(el.attribute(COLLAPSED).toInt());
    }

    if (el.hasAttribute(COLOR_LABEL)) {
        layer->setColorLabelIndex(el.attribute(COLOR_LABEL).toInt());
    }

    if (el.hasAttribute(VISIBLE_IN_TIMELINE)) {
        layer->setUseInTimeline(el.attribute(VISIBLE_IN_TIMELINE).toInt());
    }

    if (el.hasAttribute(LAYER_STYLE_UUID)) {
        QString uuidString = el.attribute(LAYER_STYLE_UUID);
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

void KisSaveXmlVisitor::saveNodeKeyframes(const KisNode* node, QString nodeFilename, QDomElement& nodeElement)
{
    if (node->isAnimated()) {
        QString keyframeFile = nodeFilename + ".keyframes.xml";
        m_keyframeFileNames[node] = keyframeFile;
        nodeElement.setAttribute(KEYFRAME_FILE, keyframeFile);
    }
}

void KisSaveXmlVisitor::saveLayer(QDomElement & el, const QString & layerType, const KisLayer * layer)
{
    QString filename = LAYER + QString::number(m_count);

    el.setAttribute(CHANNEL_FLAGS, flagsToString(layer->channelFlags()));
    el.setAttribute(NAME, layer->name());
    el.setAttribute(OPACITY, layer->opacity());
    el.setAttribute(COMPOSITE_OP, layer->compositeOp()->id());
    el.setAttribute(VISIBLE, layer->visible());
    el.setAttribute(LOCKED, layer->userLocked());
    el.setAttribute(NODE_TYPE, layerType);
    el.setAttribute(FILE_NAME, filename);
    el.setAttribute(X, layer->x());
    el.setAttribute(Y, layer->y());
    el.setAttribute(UUID, layer->uuid().toString());
    el.setAttribute(COLLAPSED, layer->collapsed());
    el.setAttribute(COLOR_LABEL, layer->colorLabelIndex());
    el.setAttribute(VISIBLE_IN_TIMELINE, layer->useInTimeline());

    if (layer->layerStyle()) {
        el.setAttribute(LAYER_STYLE_UUID, layer->layerStyle()->uuid().toString());
    }

    Q_FOREACH (KisNodeSP node, m_selectedNodes) {
        if (node.data() == layer) {
            el.setAttribute("selected", "true");
            break;
        }
    }

    saveNodeKeyframes(layer, filename, el);

    m_nodeFileNames[layer] = filename;

    dbgFile << "Saved layer "
            << layer->name()
            << " of type " << layerType
            << " with filename " << LAYER + QString::number(m_count);
}

void KisSaveXmlVisitor::saveMask(QDomElement & el, const QString & maskType, const KisMaskSP mask)
{
    QString filename = MASK + QString::number(m_count);

    el.setAttribute(NAME, mask->name());
    el.setAttribute(VISIBLE, mask->visible());
    el.setAttribute(LOCKED, mask->userLocked());
    el.setAttribute(NODE_TYPE, maskType);
    el.setAttribute(FILE_NAME, filename);
    el.setAttribute(X, mask->x());
    el.setAttribute(Y, mask->y());
    el.setAttribute(UUID, mask->uuid().toString());

    if (maskType == SELECTION_MASK) {
        el.setAttribute(ACTIVE, mask->nodeProperties().boolProperty("active"));
    } else if (maskType == COLORIZE_MASK) {
        el.setAttribute(COLORSPACE_NAME, mask->colorSpace()->id());
        el.setAttribute(COMPOSITE_OP, mask->compositeOpId());
        el.setAttribute(COLORIZE_EDIT_KEYSTROKES, KisLayerPropertiesIcons::nodeProperty(mask, KisLayerPropertiesIcons::colorizeEditKeyStrokes, true).toBool());
        el.setAttribute(COLORIZE_SHOW_COLORING, KisLayerPropertiesIcons::nodeProperty(mask, KisLayerPropertiesIcons::colorizeShowColoring, true).toBool());

        const KisColorizeMask *colorizeMask = dynamic_cast<const KisColorizeMask*>(mask.data());
        KIS_SAFE_ASSERT_RECOVER_NOOP(colorizeMask);

        if (colorizeMask) {
            el.setAttribute(COLORIZE_USE_EDGE_DETECTION, colorizeMask->useEdgeDetection());
            el.setAttribute(COLORIZE_EDGE_DETECTION_SIZE, KisDomUtils::toString(colorizeMask->edgeDetectionSize()));
            el.setAttribute(COLORIZE_FUZZY_RADIUS, KisDomUtils::toString(colorizeMask->fuzzyRadius()));
            el.setAttribute(COLORIZE_CLEANUP, int(100 * colorizeMask->cleanUpAmount()));
            el.setAttribute(COLORIZE_LIMIT_TO_DEVICE, colorizeMask->limitToDeviceBounds());
        }
    }

    saveNodeKeyframes(mask, filename, el);

    m_nodeFileNames[mask] = filename;

    dbgFile << "Saved mask "
            << mask->name()
            << " of type " << maskType
            << " with filename " << filename;
}

bool KisSaveXmlVisitor::saveMasks(KisNode * node, QDomElement & layerElement)
{
    if (node->childCount() > 0) {
        QDomElement elem = m_doc.createElement(MASKS);
        Q_ASSERT(!layerElement.isNull());
        layerElement.appendChild(elem);
        KisSaveXmlVisitor visitor(m_doc, elem, m_count, m_url, false);
        visitor.setSelectedNodes(m_selectedNodes);
        bool success = visitor.visitAllInverse(node);
        m_errorMessages.append(visitor.errorMessages());
        if (!m_errorMessages.isEmpty()) {
            return false;
        }

        QMapIterator<const KisNode*, QString> i(visitor.nodeFileNames());
        while (i.hasNext()) {
            i.next();
            m_nodeFileNames[i.key()] = i.value();
        }

        i = QMapIterator<const KisNode*, QString>(visitor.keyframeFileNames());
        while (i.hasNext()) {
            i.next();
            m_keyframeFileNames[i.key()] = i.value();
        }

        return success;
    }
    return true;
}

bool KisSaveXmlVisitor::saveReferenceImagesLayer(KisExternalLayer *layer)
{
    auto *referencesLayer = dynamic_cast<KisReferenceImagesLayer*>(layer);
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(referencesLayer, false);

    QDomElement layerElement = m_doc.createElement(LAYER);
    layerElement.setAttribute(NODE_TYPE, REFERENCE_IMAGES_LAYER);

    int nextId = 0;
    Q_FOREACH(KoShape *shape, referencesLayer->shapes()) {
        auto *reference = dynamic_cast<KisReferenceImage*>(shape);
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(reference, false);
        reference->saveXml(m_doc, layerElement, nextId);
        nextId++;
    }

    m_elem.appendChild(layerElement);
    m_count++;
    return true;
}


