/* This file is part of the KDE project
 * Copyright (C) 2012 Dan Leinir Turthra Jensen <admin@leinir.dk>
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

#include "LayerModel.h"
#include "LayerThumbProvider.h"
#include <PropertyContainer.h>
#include <kis_node_model.h>
#include <KisViewManager.h>
#include <kis_canvas2.h>
#include <kis_node_manager.h>
#include <kis_dummies_facade_base.h>
#include <KisDocument.h>
#include <kis_composite_ops_model.h>
#include <kis_node.h>
#include <kis_image.h>
#include <kis_layer.h>
#include <kis_group_layer.h>
#include <kis_paint_layer.h>
#include <kis_filter_mask.h>
#include <kis_shape_controller.h>
#include <kis_adjustment_layer.h>
#include <kis_selection_manager.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <filter/kis_filter_registry.h>
#include <KoShapeControllerBase.h>
#include <KoProperties.h>
#include <QQmlEngine>
#include <kis_base_node.h>
#include "KisSelectionActionsAdapter.h"

struct LayerModelMetaInfo {
    LayerModelMetaInfo()
    : canMoveUp(false)
    , canMoveRight(false)
    , canMoveDown(false)
    , canMoveLeft(false)
    , depth(-1)
    {}
    bool canMoveUp;
    bool canMoveRight;
    bool canMoveDown;
    bool canMoveLeft;
    int depth;
};

class LayerModel::Private {
public:
    Private(LayerModel* qq)
        : q(qq)
        , nodeModel(new KisNodeModel(qq))
        , aboutToRemoveRoots(false)
        , canvas(0)
        , nodeManager(0)
        , image(0)
        , activeNode(0)
        , declarativeEngine(0)
        , thumbProvider(0)
        , updateActiveLayerWithNewFilterConfigTimer(new QTimer(qq))
        , imageChangedTimer(new QTimer(qq))
    {
        QList<KisFilterSP> tmpFilters = KisFilterRegistry::instance()->values();
        Q_FOREACH (const KisFilterSP& filter, tmpFilters)
        {
            filters[filter.data()->id()] = filter.data();
        }
        updateActiveLayerWithNewFilterConfigTimer->setInterval(0);
        updateActiveLayerWithNewFilterConfigTimer->setSingleShot(true);
        connect(updateActiveLayerWithNewFilterConfigTimer, SIGNAL(timeout()), qq, SLOT(updateActiveLayerWithNewFilterConfig()));

        imageChangedTimer->setInterval(250);
        imageChangedTimer->setSingleShot(true);
        connect(imageChangedTimer, SIGNAL(timeout()), qq, SLOT(imageHasChanged()));
    }

    LayerModel* q;
    QList<KisNodeSP> layers;
    QHash<const KisNode*, LayerModelMetaInfo> layerMeta;
    KisNodeModel* nodeModel;
    bool aboutToRemoveRoots;
    KisViewManager* view;
    KisCanvas2* canvas;
    QScopedPointer<KisSelectionActionsAdapter> selectionActionsAdapter;
    QPointer<KisNodeManager> nodeManager;
    KisImageWSP image;
    KisNodeSP activeNode;
    QQmlEngine* declarativeEngine;
    LayerThumbProvider* thumbProvider;
    QHash<QString, const KisFilter*> filters;

    KisFilterConfigurationSP newConfig;
    QTimer* updateActiveLayerWithNewFilterConfigTimer;

    QTimer* imageChangedTimer;
    static int counter()
    {
        static int count = 0;
        return count++;
    }

    static QStringList layerClassNames()
    {
        QStringList list;
        list << "KisGroupLayer";
        list << "KisPaintLayer";
        list << "KisFilterMask";
        list << "KisAdjustmentLayer";
        return list;
    }

    int deepChildCount(KisNodeSP layer)
    {
        quint32 childCount = layer->childCount();
        QList<KisNodeSP> children = layer->childNodes(layerClassNames(), KoProperties());
        for(quint32 i = 0; i < childCount; ++i)
            childCount += deepChildCount(children.at(i));
        return childCount;
    }

    void rebuildLayerList(KisNodeSP layer = 0)
    {
        bool refreshingFromRoot = false;
        if (!image)
        {
            layers.clear();
            return;
        }
        if (layer == 0)
        {
            refreshingFromRoot = true;
            layers.clear();
            layer = image->rootLayer();
        }
        // implementation node: The root node is not a visible node, and so
        // is never added to the list of layers
        QList<KisNodeSP> children = layer->childNodes(layerClassNames(), KoProperties());
        if (children.count() == 0)
            return;
        for(quint32 i = children.count(); i > 0; --i)
        {
            layers << children.at(i-1);
            rebuildLayerList(children.at(i-1));
        }

        if (refreshingFromRoot)
            refreshLayerMovementAbilities();
    }

    void refreshLayerMovementAbilities()
    {
        layerMeta.clear();
        if (layers.count() == 0)
            return;
        for(int i = 0; i < layers.count(); ++i)
        {
            const KisNodeSP layer = layers.at(i);
            LayerModelMetaInfo ability;

            if (i > 0)
                ability.canMoveUp = true;

            if (i < layers.count() - 1)
                ability.canMoveDown = true;

            KisNodeSP parent = layer;
            while(parent)
            {
                ++ability.depth;
                parent = parent->parent();
            }

            if (ability.depth > 1)
                ability.canMoveLeft = true;

            if (i < layers.count() - 1 && qobject_cast<const KisGroupLayer*>(layers.at(i + 1).constData()))
                ability.canMoveRight = true;

            layerMeta[layer] = ability;
        }
    }
};

LayerModel::LayerModel(QObject* parent)
    : QAbstractListModel(parent)
    , d(new Private(this))
{

    connect(d->nodeModel, SIGNAL(rowsAboutToBeInserted(QModelIndex,int,int)),
            this, SLOT(source_rowsAboutToBeInserted(QModelIndex,int,int)));
    connect(d->nodeModel, SIGNAL(rowsInserted(QModelIndex,int,int)),
            this, SLOT(source_rowsInserted(QModelIndex,int,int)));

    connect(d->nodeModel, SIGNAL(rowsAboutToBeRemoved(QModelIndex,int,int)),
            this, SLOT(source_rowsAboutToBeRemoved(QModelIndex,int,int)));
    connect(d->nodeModel, SIGNAL(rowsRemoved(QModelIndex,int,int)),
            this, SLOT(source_rowsRemoved(QModelIndex,int,int)));

    connect(d->nodeModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            this, SLOT(source_dataChanged(QModelIndex,QModelIndex)));
    connect(d->nodeModel, SIGNAL(modelReset()),
            this, SLOT(source_modelReset()));

    connect(d->nodeModel, SIGNAL(layoutAboutToBeChanged()), this, SIGNAL(layoutAboutToBeChanged()));
    connect(d->nodeModel, SIGNAL(layoutChanged()), this, SIGNAL(layoutChanged()));
}

LayerModel::~LayerModel()
{
    delete d;
}

QHash<int, QByteArray> LayerModel::roleNames() const
{
    QHash<int, QByteArray> roles;
    roles[IconRole] = "icon";
    roles[NameRole] = "name";
    roles[ActiveLayerRole] = "activeLayer";
    roles[OpacityRole] = "opacity";
    roles[PercentOpacityRole] = "percentOpacity";
    roles[VisibleRole] = "visible";
    roles[CompositeDetailsRole] = "compositeDetails";
    roles[FilterRole] = "filter";
    roles[ChildCountRole] = "childCount";
    roles[DeepChildCountRole] = "deepChildCount";
    roles[DepthRole] = "depth";
    roles[PreviousItemDepthRole] = "previousItemDepth";
    roles[NextItemDepthRole] = "nextItemDepth";
    roles[CanMoveDownRole] = "canMoveDown";
    roles[CanMoveLeftRole] = "canMoveLeft";
    roles[CanMoveRightRole] = "canMoveRight";
    roles[CanMoveUpRole] = "canMoveUp";

    return roles;
}

QObject* LayerModel::view() const
{
    return d->view;
}

void LayerModel::setView(QObject *newView)
{
    KisViewManager* view = qobject_cast<KisViewManager*>(newView);
    // This has to happen very early, and we will be filling it back up again soon anyway...
    if (d->canvas) {
        d->canvas->disconnectCanvasObserver(this);
        disconnect(d->image, 0, this, 0);
        disconnect(d->nodeManager, 0, this, 0);
        disconnect(d->nodeModel, 0, d->nodeManager, 0);
        disconnect(d->nodeModel, SIGNAL(nodeActivated(KisNodeSP)), this, SLOT(currentNodeChanged(KisNodeSP)));
        d->image = 0;
        d->nodeManager = 0;
        d->layers.clear();
        d->activeNode.clear();
        d->canvas = 0;
        d->nodeModel->setDummiesFacade(0, 0, 0, 0, 0);
        d->selectionActionsAdapter.reset();

    }

    d->view = view;
    if (!d->view) {
        return;
    }

    d->canvas = view->canvasBase();
    d->thumbProvider = new LayerThumbProvider();
    d->thumbProvider->setLayerModel(this);
    d->thumbProvider->setLayerID(Private::counter());
// QT5TODO: results in a crash
     d->declarativeEngine->addImageProvider(QString("layerthumb%1").arg(d->thumbProvider->layerID()), d->thumbProvider);

    if (d->canvas) {
        d->image = d->canvas->imageView()->image();
        d->nodeManager = d->canvas->viewManager()->nodeManager();

        KisDummiesFacadeBase *kritaDummiesFacade = dynamic_cast<KisDummiesFacadeBase*>(d->canvas->imageView()->document()->shapeController());
        KisShapeController *shapeController = dynamic_cast<KisShapeController*>(d->canvas->imageView()->document()->shapeController());

        d->selectionActionsAdapter.reset(new KisSelectionActionsAdapter(d->canvas->viewManager()->selectionManager()));
        d->nodeModel->setDummiesFacade(kritaDummiesFacade,
                                       d->image,
                                       shapeController,
                                       d->selectionActionsAdapter.data(),
                                       d->nodeManager);

        connect(d->image, SIGNAL(sigAboutToBeDeleted()), SLOT(notifyImageDeleted()));
        connect(d->image, SIGNAL(sigNodeChanged(KisNodeSP)), SLOT(nodeChanged(KisNodeSP)));
        connect(d->image, SIGNAL(sigImageUpdated(QRect)), SLOT(imageChanged()));
        connect(d->image, SIGNAL(sigRemoveNodeAsync(KisNodeSP)), SLOT(aboutToRemoveNode(KisNodeSP)));

        // cold start
        currentNodeChanged(d->nodeManager->activeNode());

        // Connection KisNodeManager -> KisLayerBox
        connect(d->nodeManager, SIGNAL(sigUiNeedChangeActiveNode(KisNodeSP)), this, SLOT(currentNodeChanged(KisNodeSP)));

        d->rebuildLayerList();
        beginResetModel();
        endResetModel();
    }
}

QObject* LayerModel::engine() const
{
    return d->declarativeEngine;
}

void LayerModel::setEngine(QObject* newEngine)
{
    d->declarativeEngine = qobject_cast<QQmlEngine*>(newEngine);
    emit engineChanged();
}

QString LayerModel::fullImageThumbUrl() const
{
    return QString("image://layerthumb%1/fullimage/%2").arg(d->thumbProvider->layerID()).arg(QDateTime::currentMSecsSinceEpoch());
}

void LayerModel::currentNodeChanged(KisNodeSP newActiveNode)
{
    if (!d->activeNode.isNull()) {
        QModelIndex oldIndex = d->nodeModel->indexFromNode(d->activeNode);
        source_dataChanged(oldIndex, oldIndex);
    }
    d->activeNode = newActiveNode;
    emitActiveChanges();
    if (!d->activeNode.isNull()) {
        QModelIndex oldIndex = d->nodeModel->indexFromNode(d->activeNode);
        source_dataChanged(oldIndex, oldIndex);
    }
}

QVariant LayerModel::data(const QModelIndex& index, int role) const
{
    QVariant data;
    if (index.isValid()) {
        index.internalPointer();
        KisNodeSP node = d->layers.at(index.row());
        if (node.isNull())
            return data;
        KisNodeSP parent;
        switch(role)
        {
        case IconRole:
            if (dynamic_cast<const KisGroupLayer*>(node.constData()))
                data = QLatin1String("../images/svg/icon-layer_group-black.svg");
            else if (dynamic_cast<const KisFilterMask*>(node.constData()))
                data = QLatin1String("../images/svg/icon-layer_filter-black.svg");
            else if (dynamic_cast<const KisAdjustmentLayer*>(node.constData()))
                data = QLatin1String("../images/svg/icon-layer_filter-black.svg");
            else
                // We add the currentMSecsSinceEpoch to ensure we force an update (even with cache turned
                // off, we still apparently get some caching behaviour on delegates in QML)
                data = QString("image://layerthumb%1/%2/%3").arg(d->thumbProvider->layerID()).arg(index.row()).arg(QDateTime::currentMSecsSinceEpoch());
            break;
        case NameRole:
            data = node->name();
            break;
        case ActiveLayerRole:
            data = (node == d->activeNode);
            break;
        case OpacityRole:
            data = node->opacity();
            break;
        case PercentOpacityRole:
            data = node->percentOpacity();
            break;
        case VisibleRole:
            data = node->visible();
            break;
        case CompositeDetailsRole:
            // composite op goes here...
            if (node->compositeOp())
                data = node->compositeOp()->description();
            break;
        case FilterRole:
            break;
        case ChildCountRole:
            data = node->childNodes(d->layerClassNames(), KoProperties()).count();
            break;
        case DeepChildCountRole:
            data = d->deepChildCount(d->layers.at(index.row()));
            break;
        case DepthRole:
            data = d->layerMeta[node.data()].depth;
            break;
        case PreviousItemDepthRole:
            if (index.row() == 0)
                data = -1;
            else
                data = d->layerMeta[d->layers[index.row() - 1].data()].depth;
            break;
        case NextItemDepthRole:
            if (index.row() == d->layers.count() - 1)
                data = -1;
            else
                data = d->layerMeta[d->layers[index.row() + 1].data()].depth;
            break;
        case CanMoveDownRole:
            data = (node == d->activeNode) && d->layerMeta[node.data()].canMoveDown;
            break;
        case CanMoveLeftRole:
            data = (node == d->activeNode) && d->layerMeta[node.data()].canMoveLeft;
            break;
        case CanMoveRightRole:
            data = (node == d->activeNode) && d->layerMeta[node.data()].canMoveRight;
            break;
        case CanMoveUpRole:
            data = (node == d->activeNode) && d->layerMeta[node.data()].canMoveUp;
            break;
        default:
            break;
        }
    }
    return data;
}

int LayerModel::rowCount(const QModelIndex& parent) const
{
    if ( parent.isValid() ) {
        return 0;
    }
    return d->layers.count();
}

QVariant LayerModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    return QAbstractItemModel::headerData(section, orientation, role);
}

void LayerModel::setActive(int index)
{
    if (index > -1 && index < d->layers.count()) {
        KisNodeSP newNode = d->layers.at(index);
        d->nodeManager->slotUiActivatedNode(newNode);
        currentNodeChanged(newNode);
    }
}

void LayerModel::moveUp()
{
    KisNodeSP node = d->nodeManager->activeNode();
    KisNodeSP parent = node->parent();
    KisNodeSP grandParent = parent->parent();

    if (!d->nodeManager->activeNode()->nextSibling()) {
        //dbgKrita << "Active node apparently has no next sibling, however that has happened...";
        if (!grandParent)
            return;
        //dbgKrita << "Node has grandparent";
        if (!grandParent->parent() && node->inherits("KisMask"))
            return;
        //dbgKrita << "Node isn't a mask";
        d->nodeManager->moveNodeAt(node, grandParent, grandParent->index(parent) + 1);
    }
    else {
        //dbgKrita << "Move node directly";
        d->nodeManager->lowerNode();
    }
}

void LayerModel::moveDown()
{
    KisNodeSP node = d->nodeManager->activeNode();
    KisNodeSP parent = node->parent();
    KisNodeSP grandParent = parent->parent();

    if (!d->nodeManager->activeNode()->prevSibling()) {
        //dbgKrita << "Active node apparently has no previous sibling, however that has happened...";
        if (!grandParent)
            return;
        //dbgKrita << "Node has grandparent";
        if (!grandParent->parent() && node->inherits("KisMask"))
            return;
        //dbgKrita << "Node isn't a mask";
        d->nodeManager->moveNodeAt(node, grandParent, grandParent->index(parent));
    } else
    {
        //dbgKrita << "Move node directly";
        d->nodeManager->raiseNode();
    }
}

void LayerModel::moveLeft()
{
    KisNodeSP node = d->nodeManager->activeNode();
    KisNodeSP parent = node->parent();
    KisNodeSP grandParent = parent->parent();
    quint16 nodeIndex = parent->index(node);

    if (!grandParent)
        return;
    if (!grandParent->parent() && node->inherits("KisMask"))
        return;

    if (nodeIndex <= parent->childCount() / 2) {
        d->nodeManager->moveNodeAt(node, grandParent, grandParent->index(parent));
    }
    else {
        d->nodeManager->moveNodeAt(node, grandParent, grandParent->index(parent) + 1);
    }
}

void LayerModel::moveRight()
{
    KisNodeSP node = d->nodeManager->activeNode();
    KisNodeSP parent = d->nodeManager->activeNode()->parent();
    KisNodeSP newParent;
    int nodeIndex = parent->index(node);
    int indexAbove = nodeIndex + 1;
    int indexBelow = nodeIndex - 1;

    if (parent->at(indexBelow) && parent->at(indexBelow)->allowAsChild(node)) {
        newParent = parent->at(indexBelow);
        d->nodeManager->moveNodeAt(node, newParent, newParent->childCount());
    }
    else if (parent->at(indexAbove) && parent->at(indexAbove)->allowAsChild(node)) {
        newParent = parent->at(indexAbove);
        d->nodeManager->moveNodeAt(node, newParent, 0);
    }
    else {
        return;
    }
}

void LayerModel::clear()
{
    d->canvas->viewManager()->selectionManager()->clear();
}

void LayerModel::clone()
{
    d->nodeManager->duplicateActiveNode();
}

void LayerModel::setLocked(int index, bool newLocked)
{
    if (index > -1 && index < d->layers.count()) {
        if(d->layers[index]->userLocked() == newLocked)
            return;
        d->layers[index]->setUserLocked(newLocked);
        QModelIndex idx = createIndex(index, 0);
        dataChanged(idx, idx);
    }
}

void LayerModel::setOpacity(int index, float newOpacity)
{
    if (index > -1 && index < d->layers.count()) {
        if(qFuzzyCompare(d->layers[index]->opacity() + 1, newOpacity + 1))
            return;
        d->layers[index]->setOpacity(newOpacity);
        d->layers[index]->setDirty();
        QModelIndex idx = createIndex(index, 0);
        dataChanged(idx, idx);
    }
}

void LayerModel::setVisible(int index, bool newVisible)
{
    if (index > -1 && index < d->layers.count()) {
        KisBaseNode::PropertyList props = d->layers[index]->sectionModelProperties();
        if(props[0].state == newVisible)
            return;
        KisBaseNode::Property prop = props[0];
        prop.state = newVisible;
        props[0] = prop;
        d->nodeModel->setData( d->nodeModel->indexFromNode(d->layers[index]), QVariant::fromValue<KisBaseNode::PropertyList>(props), KisNodeModel::PropertiesRole );
        d->layers[index]->setDirty(d->layers[index]->extent());
        QModelIndex idx = createIndex(index, 0);
        dataChanged(idx, idx);
    }
}

QImage LayerModel::layerThumbnail(QString layerID) const
{
    // So, yeah, this is a complete cheatery hack. However, it ensures
    // we actually get updates when we want them (every time the image is supposed
    // to be changed). Had hoped we could avoid it, but apparently not.
    int index = layerID.section(QChar('/'), 0, 0).toInt();
    QImage thumb;
    if (index > -1 && index < d->layers.count()) {
        if (d->thumbProvider)
            thumb = d->layers[index]->createThumbnail(120, 120);
    }
    return thumb;
}

void LayerModel::deleteCurrentLayer()
{
    d->activeNode.clear();
    d->nodeManager->removeNode();
}

void LayerModel::deleteLayer(int index)
{
    if (index > -1 && index < d->layers.count()) {
        if (d->activeNode == d->layers.at(index))
            d->activeNode.clear();
        d->nodeManager->slotUiActivatedNode(d->layers.at(index));
        d->nodeManager->removeNode();
        d->rebuildLayerList();
        beginResetModel();
        endResetModel();
    }
}

void LayerModel::addLayer(int layerType)
{
    switch(layerType) {
        case 0:
            d->nodeManager->createNode("KisPaintLayer");
            break;
        case 1:
            d->nodeManager->createNode("KisGroupLayer");
            break;
        case 2:
            d->nodeManager->createNode("KisFilterMask", true);
            break;
        default:
            break;
    }
}

void LayerModel::source_rowsAboutToBeInserted(QModelIndex /*p*/, int /*from*/, int /*to*/)
{
    beginResetModel();
}

void LayerModel::source_rowsInserted(QModelIndex /*p*/, int, int)
{
    d->rebuildLayerList();
    emit countChanged();
    endResetModel();
}

void LayerModel::source_rowsAboutToBeRemoved(QModelIndex /*p*/, int /*from*/, int /*to*/)
{
    beginResetModel();
}

void LayerModel::source_rowsRemoved(QModelIndex, int, int)
{
    d->rebuildLayerList();
    emit countChanged();
    endResetModel();
}

void LayerModel::source_dataChanged(QModelIndex /*tl*/, QModelIndex /*br*/)
{
        QModelIndex top = createIndex(0, 0);
        QModelIndex bottom = createIndex(d->layers.count() - 1, 0);
        dataChanged(top, bottom);
}

void LayerModel::source_modelReset()
{
    beginResetModel();
    d->rebuildLayerList();
    d->activeNode.clear();
    if (d->layers.count() > 0)
    {
        d->nodeManager->slotUiActivatedNode(d->layers.at(0));
        currentNodeChanged(d->layers.at(0));
    }
    emit countChanged();
    endResetModel();
}

void LayerModel::notifyImageDeleted()
{
}

void LayerModel::nodeChanged(KisNodeSP node)
{
    QModelIndex index = createIndex(d->layers.indexOf(node), 0);
    dataChanged(index, index);
}

void LayerModel::imageChanged()
{
    d->imageChangedTimer->start();
}

void LayerModel::imageHasChanged()
{
    QModelIndex top = createIndex(0, 0);
    QModelIndex bottom = createIndex(d->layers.count() - 1, 0);
    dataChanged(top, bottom);
}

void LayerModel::aboutToRemoveNode(KisNodeSP node)
{
    Q_UNUSED(node)
    QTimer::singleShot(0, this, SLOT(source_modelReset()));
}

void LayerModel::emitActiveChanges()
{
    emit activeFilterConfigChanged();
    emit activeNameChanged();
    emit activeTypeChanged();
    emit activeCompositeOpChanged();
    emit activeOpacityChanged();
    emit activeVisibleChanged();
    emit activeLockedChanged();
    emit activeRChannelActiveChanged();
    emit activeGChannelActiveChanged();
    emit activeBChannelActiveChanged();
    emit activeAChannelActiveChanged();
}

QString LayerModel::activeName() const
{
    if (d->activeNode.isNull())
        return QString();
    return d->activeNode->name();
}

void LayerModel::setActiveName(QString newName)
{
    if (d->activeNode.isNull())
        return;
    d->activeNode->setName(newName);
    emit activeNameChanged();
}

QString LayerModel::activeType() const
{
    return d->activeNode->metaObject()->className();
}

int LayerModel::activeCompositeOp() const
{
    if (d->activeNode.isNull())
        return 0;
    KoID entry(d->activeNode->compositeOp()->id());
    QModelIndex idx = KisCompositeOpListModel::sharedInstance()->indexOf(entry);
    if (idx.isValid())
        return idx.row();
    return 0;
}

void LayerModel::setActiveCompositeOp(int newOp)
{
    if (d->activeNode.isNull())
        return;
    KoID entry;
    if (KisCompositeOpListModel::sharedInstance()->entryAt(entry, KisCompositeOpListModel::sharedInstance()->index(newOp)))
    {
        d->activeNode->setCompositeOpId(entry.id());
        d->activeNode->setDirty();
        emit activeCompositeOpChanged();
    }
}

int LayerModel::activeOpacity() const
{
    if (d->activeNode.isNull())
        return 0;
    return d->activeNode->opacity();
}

void LayerModel::setActiveOpacity(int newOpacity)
{
    d->activeNode->setOpacity(newOpacity);
    d->activeNode->setDirty();
    emit activeOpacityChanged();
}

bool LayerModel::activeVisible() const
{    if (d->activeNode.isNull())
        return false;
    return d->activeNode->visible();
}

void LayerModel::setActiveVisible(bool newVisible)
{
    if (d->activeNode.isNull())
        return;
    setVisible(d->layers.indexOf(d->activeNode), newVisible);
    emit activeVisibleChanged();
}

bool LayerModel::activeLocked() const
{
    if (d->activeNode.isNull())
        return false;
    return d->activeNode->userLocked();
}

void LayerModel::setActiveLocked(bool newLocked)
{
    if (d->activeNode.isNull())
        return;
    d->activeNode->setUserLocked(newLocked);
    emit activeLockedChanged();
}

bool LayerModel::activeAChannelActive() const
{
    KisLayer* layer = qobject_cast<KisLayer*>(d->activeNode.data());
    bool state = false;
    if (layer)
        state = !layer->alphaChannelDisabled();
    return state;
}

void LayerModel::setActiveAChannelActive(bool newActive)
{
    KisLayer* layer = qobject_cast<KisLayer*>(d->activeNode.data());
    if (layer)
    {
        layer->disableAlphaChannel(!newActive);
        layer->setDirty();
        emit activeAChannelActiveChanged();
    }
}


bool getActiveChannel(KisNodeSP node, int channelIndex)
{
    KisLayer* layer = qobject_cast<KisLayer*>(node.data());
    bool flag = false;
    if (layer)
    {
        QBitArray flags = layer->channelFlags();
        if (channelIndex < flags.size()) {
            flag = flags[channelIndex];
        }
    }
    return flag;
}

void setChannelActive(KisNodeSP node, int channelIndex, bool newActive)
{
    KisLayer* layer = qobject_cast<KisLayer*>(node.data());
    if (layer) {
        QBitArray flags = layer->channelFlags();
        flags.setBit(channelIndex, newActive);
        layer->setChannelFlags(flags);
        layer->setDirty();
    }
}

bool LayerModel::activeBChannelActive() const
{
    return getActiveChannel(d->activeNode, 2);
}

void LayerModel::setActiveBChannelActive(bool newActive)
{
    setChannelActive(d->activeNode, 2, newActive);
    emit activeBChannelActiveChanged();
}

bool LayerModel::activeGChannelActive() const
{
    return getActiveChannel(d->activeNode, 1);
}

void LayerModel::setActiveGChannelActive(bool newActive)
{
    setChannelActive(d->activeNode, 1, newActive);
    emit activeGChannelActiveChanged();
}

bool LayerModel::activeRChannelActive() const
{
    return getActiveChannel(d->activeNode, 0);
}

void LayerModel::setActiveRChannelActive(bool newActive)
{
    setChannelActive(d->activeNode, 0, newActive);
    emit activeRChannelActiveChanged();
}

QObject* LayerModel::activeFilterConfig() const
{
    QMap<QString, QVariant> props;
    QString filterId;
    KisFilterMask* filterMask = qobject_cast<KisFilterMask*>(d->activeNode.data());
    if (filterMask) {
        props = filterMask->filter()->getProperties();
        filterId = filterMask->filter()->name();
    }
    else {
        KisAdjustmentLayer* adjustmentLayer = qobject_cast<KisAdjustmentLayer*>(d->activeNode.data());
        if (adjustmentLayer)
        {
            props = adjustmentLayer->filter()->getProperties();
            filterId = adjustmentLayer->filter()->name();
        }
    }
    PropertyContainer* config = new PropertyContainer(filterId, 0);
    QMap<QString, QVariant>::const_iterator i;
    for(i = props.constBegin(); i != props.constEnd(); ++i) {
        config->setProperty(i.key().toLatin1(), i.value());
        //dbgKrita << "Getting active config..." << i.key() << i.value();
    }
    return config;
}

void LayerModel::setActiveFilterConfig(QObject* newConfig)
{
    if (d->activeNode.isNull())
        return;
    PropertyContainer* config = qobject_cast<PropertyContainer*>(newConfig);
    if (!config)
        return;

    //dbgKrita << "Attempting to set new config" << config->name();
    KisFilterConfigurationSP realConfig = d->filters.value(config->name())->factoryConfiguration();
    QMap<QString, QVariant>::const_iterator i;
    for(i = realConfig->getProperties().constBegin(); i != realConfig->getProperties().constEnd(); ++i)
    {
        realConfig->setProperty(i.key(), config->property(i.key().toLatin1()));
        //dbgKrita << "Creating config..." << i.key() << i.value();
    }
// The following code causes sporadic crashes, and disabling causes leaks. So, leaks it must be, for now.
// The cause is the lack of a smart pointer interface for passing filter configs around
// Must be remedied, but for now...
//    if (d->newConfig)
//        delete(d->newConfig);
    d->newConfig = realConfig;
    //d->updateActiveLayerWithNewFilterConfigTimer->start();
    updateActiveLayerWithNewFilterConfig();
}

void LayerModel::updateActiveLayerWithNewFilterConfig()
{
    if (!d->newConfig)
        return;
    //dbgKrita << "Setting new config..." << d->newConfig->name();
    KisFilterMask* filterMask = qobject_cast<KisFilterMask*>(d->activeNode.data());
    if (filterMask)
    {
        //dbgKrita << "Filter mask";
        if (filterMask->filter() == d->newConfig)
            return;
        //dbgKrita << "Setting filter mask";
        filterMask->setFilter(d->newConfig);
    }
    else
    {
        KisAdjustmentLayer* adjustmentLayer = qobject_cast<KisAdjustmentLayer*>(d->activeNode.data());
        if (adjustmentLayer)
        {
            //dbgKrita << "Adjustment layer";
            if (adjustmentLayer->filter() == d->newConfig)
                return;
            //dbgKrita << "Setting filter on adjustment layer";
            adjustmentLayer->setFilter(d->newConfig);
        }
        else
        {
            //dbgKrita << "UNKNOWN, BAIL OUT!";
        }
    }
    d->newConfig = 0;
    d->activeNode->setDirty(d->activeNode->extent());
    d->image->setModified();
    QTimer::singleShot(100, this, SIGNAL(activeFilterConfigChanged()));
}

