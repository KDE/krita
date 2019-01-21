/*
 *  Copyright (c) 2016 Boudewijn Rempt <boud@valdyas.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include <QUrl>
#include <QScopedPointer>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorTransformation.h>

#include <KisDocument.h>
#include <KisMimeDatabase.h>
#include <KisPart.h>
#include <kis_change_profile_visitor.h>
#include <kis_colorspace_convert_visitor.h>
#include <kis_image.h>
#include <kis_types.h>
#include <kis_node.h>
#include <kis_paint_layer.h>
#include <kis_group_layer.h>
#include <kis_file_layer.h>
#include <kis_adjustment_layer.h>
#include <kis_generator_layer.h>
#include <kis_clone_layer.h>
#include <kis_shape_layer.h>
#include <KisReferenceImagesLayer.h>
#include <kis_transparency_mask.h>
#include <kis_filter_mask.h>
#include <kis_transform_mask.h>
#include <kis_selection_mask.h>
#include <lazybrush/kis_colorize_mask.h>
#include <kis_layer.h>
#include <kis_meta_data_merge_strategy.h>
#include <kis_meta_data_merge_strategy_registry.h>
#include <kis_filter_strategy.h>

#include <kis_raster_keyframe_channel.h>
#include <kis_keyframe.h>
#include "kis_selection.h"

#include "InfoObject.h"
#include "Krita.h"
#include "Node.h"
#include "Channel.h"
#include "Filter.h"
#include "Selection.h"

#include "GroupLayer.h"
#include "CloneLayer.h"
#include "FilterLayer.h"
#include "FillLayer.h"
#include "FileLayer.h"
#include "VectorLayer.h"
#include "FilterMask.h"
#include "SelectionMask.h"

#include "LibKisUtils.h"

struct Node::Private {
    Private() {}
    KisImageWSP image;
    KisNodeSP node;
};

Node::Node(KisImageSP image, KisNodeSP node, QObject *parent)
    : QObject(parent)
    , d(new Private)
{
    d->image = image;
    d->node = node;
}

Node::~Node()
{
    delete d;
}

bool Node::operator==(const Node &other) const
{
    return (d->node == other.d->node
            && d->image == other.d->image);
}

bool Node::operator!=(const Node &other) const
{
    return !(operator==(other));
}

Node *Node::clone() const
{
    KisNodeSP clone = d->node->clone();
    Node *node = new Node(0, clone);
    return node;
}


bool Node::alphaLocked() const
{
    if (!d->node) return false;
    KisPaintLayerSP paintLayer = qobject_cast<KisPaintLayer*>(d->node.data());
    if (paintLayer) {
        return paintLayer->alphaLocked();
    }
    return false;
}

void Node::setAlphaLocked(bool value)
{
    if (!d->node) return;
    KisPaintLayerSP paintLayer = qobject_cast<KisPaintLayer*>(d->node.data());
    if (paintLayer) {
        paintLayer->setAlphaLocked(value);
    }
}


QString Node::blendingMode() const
{
    if (!d->node) return QString();

    return d->node->compositeOpId();
}

void Node::setBlendingMode(QString value)
{
    if (!d->node) return;
    d->node->setCompositeOpId(value);
}


QList<Channel*> Node::channels() const
{
    QList<Channel*> channels;

    if (!d->node) return channels;
    if (!d->node->inherits("KisLayer")) return channels;

    Q_FOREACH(KoChannelInfo *info, d->node->colorSpace()->channels()) {
        Channel *channel = new Channel(d->node, info);
        channels << channel;
    }

    return channels;
}

QList<Node*> Node::childNodes() const
{
    QList<Node*> nodes;
    if (d->node) {
        KisNodeList nodeList;
        int childCount = d->node->childCount();
        for (int i = 0; i < childCount; ++i) {
            nodeList << d->node->at(i);
        }
        nodes = LibKisUtils::createNodeList(nodeList, d->image);
    }
    return nodes;
}

bool Node::addChildNode(Node *child, Node *above)
{
    if (!d->node) return false;
    if (above) {
        return d->image->addNode(child->node(), d->node, above->node());
    }
    else {
        return d->image->addNode(child->node(), d->node, d->node->childCount());
    }
}

bool Node::removeChildNode(Node *child)
{
    if (!d->node) return false;
    return d->image->removeNode(child->node());
}

void Node::setChildNodes(QList<Node*> nodes)
{
    if (!d->node) return;
    KisNodeSP node = d->node->firstChild();
    while (node) {
        d->image->removeNode(node);
        node = node->nextSibling();
    }
    Q_FOREACH(Node *node, nodes) {
        d->image->addNode(node->node(), d->node);
    }
}

int Node::colorLabel() const
{
    if (!d->node) return 0;
    return d->node->colorLabelIndex();
}

void Node::setColorLabel(int index)
{
    if (!d->node) return;
    d->node->setColorLabelIndex(index);
}

QString Node::colorDepth() const
{
    if (!d->node) return "";
    return d->node->colorSpace()->colorDepthId().id();
}

QString Node::colorModel() const
{
    if (!d->node) return "";
    return d->node->colorSpace()->colorModelId().id();
}


QString Node::colorProfile() const
{
    if (!d->node) return "";
    return d->node->colorSpace()->profile()->name();
}

bool Node::setColorProfile(const QString &colorProfile)
{
    if (!d->node) return false;
    if (!d->node->inherits("KisLayer")) return false;
    KisLayer *layer = qobject_cast<KisLayer*>(d->node.data());
    const KoColorProfile *profile = KoColorSpaceRegistry::instance()->profileByName(colorProfile);
    const KoColorSpace *srcCS = layer->colorSpace();
    const KoColorSpace *dstCs = KoColorSpaceRegistry::instance()->colorSpace(srcCS->colorModelId().id(),
                                                                             srcCS->colorDepthId().id(),
                                                                             profile);
    KisChangeProfileVisitor v(srcCS, dstCs);
    return layer->accept(v);
}

bool Node::setColorSpace(const QString &colorModel, const QString &colorDepth, const QString &colorProfile)
{
    if (!d->node) return false;
    if (!d->node->inherits("KisLayer")) return false;
    KisLayer *layer = qobject_cast<KisLayer*>(d->node.data());
    const KoColorProfile *profile = KoColorSpaceRegistry::instance()->profileByName(colorProfile);
    const KoColorSpace *srcCS = layer->colorSpace();
    const KoColorSpace *dstCs = KoColorSpaceRegistry::instance()->colorSpace(colorModel,
                                                                             colorDepth,
                                                                             profile);
    KisColorSpaceConvertVisitor v(d->image, srcCS, dstCs, KoColorConversionTransformation::internalRenderingIntent(), KoColorConversionTransformation::internalConversionFlags());
    return layer->accept(v);
}

bool Node::animated() const
{
    if (!d->node) return false;
    return d->node->isAnimated();
}

void Node::enableAnimation() const
{
    if (!d->node) return;
    d->node->enableAnimation();
}

void Node::setShowInTimeline(bool showInTimeline) const
{
    if (!d->node) return;
    d->node->setUseInTimeline(showInTimeline);
}

bool Node::showInTimeline() const
{
    if (!d->node) return false;
    return d->node->useInTimeline();
}

bool Node::collapsed() const
{
    if (!d->node) return false;
    return d->node->collapsed();
}

void Node::setCollapsed(bool collapsed)
{
    if (!d->node) return;
    d->node->setCollapsed(collapsed);
}

bool Node::inheritAlpha() const
{
    if (!d->node) return false;
    if (!d->node->inherits("KisLayer")) return false;
    return qobject_cast<const KisLayer*>(d->node)->alphaChannelDisabled();
}

void Node::setInheritAlpha(bool value)
{
    if (!d->node) return;
    if (!d->node->inherits("KisLayer")) return;
    const_cast<KisLayer*>(qobject_cast<const KisLayer*>(d->node))->disableAlphaChannel(value);
}

bool Node::locked() const
{
    if (!d->node) return false;
    return d->node->userLocked();
}

void Node::setLocked(bool value)
{
    if (!d->node) return;
    d->node->setUserLocked(value);
}

bool Node::hasExtents()
{
    return !d->node->extent().isEmpty();
}

QString Node::name() const
{
    if (!d->node) return QString();
    return d->node->name();
}

void Node::setName(QString name)
{
    if (!d->node) return;
    d->node->setName(name);
}


int Node::opacity() const
{
    if (!d->node) return 0;
    return d->node->opacity();
}

void Node::setOpacity(int value)
{
    if (!d->node) return;
    if (value < 0) value = 0;
    if (value > 255) value = 255;
    d->node->setOpacity(value);
}


Node* Node::parentNode() const
{
    if (!d->node) return 0;
    return new Node(d->image, d->node->parent());
}

QString Node::type() const
{
    if (!d->node) return QString();
    if (qobject_cast<const KisPaintLayer*>(d->node)) {
        return "paintlayer";
    }
    else if (qobject_cast<const KisGroupLayer*>(d->node)) {
        return "grouplayer";
    }
    if (qobject_cast<const KisFileLayer*>(d->node)) {
        return "filelayer";
    }
    if (qobject_cast<const KisAdjustmentLayer*>(d->node)) {
        return "filterlayer";
    }
    if (qobject_cast<const KisGeneratorLayer*>(d->node)) {
        return "filllayer";
    }
    if (qobject_cast<const KisCloneLayer*>(d->node)) {
        return "clonelayer";
    }
    if (qobject_cast<const KisReferenceImagesLayer*>(d->node)) {
        return "referenceimageslayer";
    }
    if (qobject_cast<const KisShapeLayer*>(d->node)) {
        return "vectorlayer";
    }
    if (qobject_cast<const KisTransparencyMask*>(d->node)) {
        return "transparencymask";
    }
    if (qobject_cast<const KisFilterMask*>(d->node)) {
        return "filtermask";
    }
    if (qobject_cast<const KisTransformMask*>(d->node)) {
        return "transformmask";
    }
    if (qobject_cast<const KisSelectionMask*>(d->node)) {
        return "selectionmask";
    }
    if (qobject_cast<const KisColorizeMask*>(d->node)) {
        return "colorizemask";
    }
    return QString();
}

QIcon Node::icon() const
{
    QIcon icon;
    if (d->node) {
        icon = d->node->icon();
    }
    return icon;
}

bool Node::visible() const
{
    if (!d->node) return false;
    return d->node->visible();
}

bool Node::hasKeyframeAtTime(int frameNumber)
{
    if (!d->node || !d->node->isAnimated()) return false;

    KisRasterKeyframeChannel *rkc = dynamic_cast<KisRasterKeyframeChannel*>(d->node->getKeyframeChannel(KisKeyframeChannel::Content.id()));
    if (!rkc) return false;

    KisKeyframeSP timeOfCurrentKeyframe = rkc->keyframeAt(frameNumber);

    if (!timeOfCurrentKeyframe) {
        return false;
    }

    // do an assert just to be careful
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(timeOfCurrentKeyframe->time() == frameNumber, false);
    return true;
}

void Node::setVisible(bool visible)
{
    if (!d->node) return;
    d->node->setVisible(visible);
}


QByteArray Node::pixelData(int x, int y, int w, int h) const
{
    QByteArray ba;

    if (!d->node) return ba;

    KisPaintDeviceSP dev = d->node->paintDevice();
    if (!dev) return ba;

    ba.resize(w * h * dev->pixelSize());
    dev->readBytes(reinterpret_cast<quint8*>(ba.data()), x, y, w, h);
    return ba;
}

QByteArray Node::pixelDataAtTime(int x, int y, int w, int h, int time) const
{
    QByteArray ba;

    if (!d->node || !d->node->isAnimated()) return ba;

    //
    KisRasterKeyframeChannel *rkc = dynamic_cast<KisRasterKeyframeChannel*>(d->node->getKeyframeChannel(KisKeyframeChannel::Content.id()));
    if (!rkc) return ba;
    KisKeyframeSP frame = rkc->keyframeAt(time);
    if (!frame) return ba;
    KisPaintDeviceSP dev = d->node->paintDevice();
    if (!dev) return ba;

    rkc->fetchFrame(frame, dev);

    ba.resize(w * h * dev->pixelSize());
    dev->readBytes(reinterpret_cast<quint8*>(ba.data()), x, y, w, h);
    return ba;
}


QByteArray Node::projectionPixelData(int x, int y, int w, int h) const
{
    QByteArray ba;

    if (!d->node) return ba;

    KisPaintDeviceSP dev = d->node->projection();
    if (!dev) return ba;

    ba.resize(w * h * dev->pixelSize());
    dev->readBytes(reinterpret_cast<quint8*>(ba.data()), x, y, w, h);
    return ba;
}

void Node::setPixelData(QByteArray value, int x, int y, int w, int h)
{
    if (!d->node) return;
    KisPaintDeviceSP dev = d->node->paintDevice();
    if (!dev) return;
    dev->writeBytes((const quint8*)value.constData(), x, y, w, h);
}

QRect Node::bounds() const
{
    if (!d->node) return QRect();
    return d->node->exactBounds();
}

void Node::move(int x, int y)
{
    if (!d->node) return;
    d->node->setX(x);
    d->node->setY(y);
}

QPoint Node::position() const
{
    if (!d->node) return QPoint();
    return QPoint(d->node->x(), d->node->y());
}

bool Node::remove()
{
    if (!d->node) return false;
    if (!d->node->parent()) return false;
    return d->image->removeNode(d->node);
}

Node* Node::duplicate()
{
    if (!d->node) return 0;
    return new Node(d->image, d->node->clone());
}

bool Node::save(const QString &filename, double xRes, double yRes, const InfoObject &exportConfiguration)
{
    if (!d->node) return false;
    if (filename.isEmpty()) return false;

    KisPaintDeviceSP projection = d->node->projection();
    QRect bounds = d->node->exactBounds();

    QString mimeType = KisMimeDatabase::mimeTypeForFile(filename, false);
    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

    KisImageSP dst = new KisImage(doc->createUndoStore(),
                                  bounds.right(),
                                  bounds.bottom(),
                                  projection->compositionSourceColorSpace(),
                                  d->node->name());
    dst->setResolution(xRes, yRes);
    doc->setFileBatchMode(Krita::instance()->batchmode());
    doc->setCurrentImage(dst);
    KisPaintLayer* paintLayer = new KisPaintLayer(dst, "paint device", d->node->opacity());
    paintLayer->paintDevice()->makeCloneFrom(projection, bounds);
    dst->addNode(paintLayer, dst->rootLayer(), KisLayerSP(0));
    dst->cropImage(bounds);
    dst->initialRefreshGraph();

    bool r = doc->exportDocumentSync(QUrl::fromLocalFile(filename), mimeType.toLatin1(), exportConfiguration.configuration());
    if (!r) {
        qWarning() << doc->errorMessage();
    }
    return r;
}

Node* Node::mergeDown()
{
    if (!d->node) return 0;
    if (!qobject_cast<KisLayer*>(d->node.data())) return 0;
    if (!d->node->prevSibling()) return 0;

    d->image->mergeDown(qobject_cast<KisLayer*>(d->node.data()), KisMetaData::MergeStrategyRegistry::instance()->get("Drop"));
    d->image->waitForDone();

    return new Node(d->image, d->node->prevSibling());
}

void Node::scaleNode(QPointF origin, int width, int height, QString strategy)
{
    if (!d->node) return;
    if (!qobject_cast<KisLayer*>(d->node.data())) return;
    if (!d->node->parent()) return;

    KisFilterStrategy *actualStrategy = KisFilterStrategyRegistry::instance()->get(strategy);
    if (!actualStrategy) actualStrategy = KisFilterStrategyRegistry::instance()->get("Bicubic");

    const QRect bounds(d->node->exactBounds());

    d->image->scaleNode(d->node,
                        origin,
                        qreal(width) / bounds.width(),
                        qreal(height) / bounds.height(),
                        actualStrategy, 0);
}

void Node::rotateNode(double radians)
{
    if (!d->node) return;
    if (!qobject_cast<KisLayer*>(d->node.data())) return;
    if (!d->node->parent()) return;

    d->image->rotateNode(d->node, radians, 0);
}

void Node::cropNode(int x, int y, int w, int h)
{
    if (!d->node) return;
    if (!qobject_cast<KisLayer*>(d->node.data())) return;
    if (!d->node->parent()) return;

    QRect rect = QRect(x, y, w, h);
    d->image->cropNode(d->node, rect);
}

void Node::shearNode(double angleX, double angleY)
{
    if (!d->node) return;
    if (!qobject_cast<KisLayer*>(d->node.data())) return;
    if (!d->node->parent()) return;

    d->image->shearNode(d->node, angleX, angleY, 0);
}

QImage Node::thumbnail(int w, int h)
{
    if (!d->node) return QImage();
    return d->node->createThumbnail(w, h);
}

KisPaintDeviceSP Node::paintDevice() const
{
    return d->node->paintDevice();
}

KisImageSP Node::image() const
{
    return d->image;
}

KisNodeSP Node::node() const
{
    return d->node;
}
