/*
 *  Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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

#include "kis_node_manager.h"

#include <QDesktopServices>
#include <QMessageBox>

#include <kactioncollection.h>
#include <kmimetype.h>

#include <KoIcon.h>
#include <KoProperties.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoShape.h>
#include <KoShapeLayer.h>
#include <KisImportExportManager.h>
#include <KoFileDialog.h>

#include <KoColorSpace.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>

#include <kis_types.h>
#include <kis_node.h>
#include <kis_selection.h>
#include <kis_selection_mask.h>
#include <kis_layer.h>
#include <kis_mask.h>
#include <kis_image.h>
#include <kis_painter.h>
#include <kis_paint_layer.h>

#include "KisPart.h"
#include "canvas/kis_canvas2.h"
#include "kis_shape_controller.h"
#include "kis_canvas_resource_provider.h"
#include "KisViewManager.h"
#include "KisDocument.h"
#include "kis_mask_manager.h"
#include "kis_group_layer.h"
#include "kis_layer_manager.h"
#include "kis_selection_manager.h"
#include "kis_node_commands_adapter.h"
#include "kis_action.h"
#include "kis_action_manager.h"
#include "kis_processing_applicator.h"
#include "kis_sequential_iterator.h"
#include "kis_transaction.h"

#include "processing/kis_mirror_processing_visitor.h"
#include "KisView.h"

struct KisNodeManager::Private {

    Private(KisNodeManager *_q)
        : q(_q)
        , view(0)
        , imageView(0)
        , layerManager(0)
        , maskManager(0)
        , self(0)
        , commandsAdapter(0)
    {
    }

    ~Private() {
        delete layerManager;
        delete maskManager;
    }

    KisNodeManager *q;
    KisViewManager * view;
    QPointer<KisView>imageView;
    KisLayerManager * layerManager;
    KisMaskManager * maskManager;
    KisNodeManager* self;
    KisNodeCommandsAdapter* commandsAdapter;
    KAction *mergeSelectedLayers;

    QList<KisNodeSP> selectedNodes;

    bool activateNodeImpl(KisNodeSP node);

    QSignalMapper nodeCreationSignalMapper;
    QSignalMapper nodeConversionSignalMapper;

    void saveDeviceAsImage(KisPaintDeviceSP device,
                           const QString &defaultName,
                           const QRect &bounds,
                           qreal xRes,
                           qreal yRes,
                           quint8 opacity);

    void mergeTransparencyMaskAsAlpha(bool writeToLayers);
};

bool KisNodeManager::Private::activateNodeImpl(KisNodeSP node)
{
    Q_ASSERT(view);
    Q_ASSERT(view->canvasBase());
    Q_ASSERT(view->canvasBase()->globalShapeManager());
    Q_ASSERT(imageView);
    if (node && node == self->activeNode()) {
        return false;
    }

    // Set the selection on the shape manager to the active layer
    // and set call KoSelection::setActiveLayer( KoShapeLayer* layer )
    // with the parent of the active layer.
    KoSelection *selection = view->canvasBase()->globalShapeManager()->selection();
    Q_ASSERT(selection);
    selection->deselectAll();

    if (!node) {
        selection->setActiveLayer(0);
        imageView->setCurrentNode(0);
        maskManager->activateMask(0);
        layerManager->activateLayer(0);
    } else {

        KoShape * shape = view->document()->shapeForNode(node);
        Q_ASSERT(shape);

        selection->select(shape);
        KoShapeLayer * shapeLayer = dynamic_cast<KoShapeLayer*>(shape);

        Q_ASSERT(shapeLayer);
//         shapeLayer->setGeometryProtected(node->userLocked());
//         shapeLayer->setVisible(node->visible());
        selection->setActiveLayer(shapeLayer);

        imageView->setCurrentNode(node);
        if (KisLayerSP layer = dynamic_cast<KisLayer*>(node.data())) {
            maskManager->activateMask(0);
            layerManager->activateLayer(layer);
        } else if (KisMaskSP mask = dynamic_cast<KisMask*>(node.data())) {
            maskManager->activateMask(mask);
            // XXX_NODE: for now, masks cannot be nested.
            layerManager->activateLayer(static_cast<KisLayer*>(node->parent().data()));
        }

    }
    return true;
}

KisNodeManager::KisNodeManager(KisViewManager *view)
    : m_d(new Private(this))
{
    m_d->view = view;
    m_d->layerManager = new KisLayerManager(view);
    m_d->maskManager = new KisMaskManager(view);
    m_d->self = this;
    m_d->commandsAdapter = new KisNodeCommandsAdapter(view);

    connect(m_d->layerManager, SIGNAL(sigLayerActivated(KisLayerSP)), SIGNAL(sigLayerActivated(KisLayerSP)));

    m_d->mergeSelectedLayers = new KAction(i18n("&Merge Selected Layers"), this);
    view->actionCollection()->addAction("merge_selected_layers", m_d->mergeSelectedLayers);
    connect(m_d->mergeSelectedLayers, SIGNAL(triggered()), this, SLOT(mergeLayerDown()));

}

KisNodeManager::~KisNodeManager()
{
    delete m_d->commandsAdapter;
    delete m_d;
}

void KisNodeManager::setView(QPointer<KisView>imageView)
{
    m_d->maskManager->setView(imageView);
    m_d->layerManager->setView(imageView);

    if (m_d->imageView) {
        KisShapeController *shapeController = dynamic_cast<KisShapeController*>(m_d->view->document()->shapeController());
        Q_ASSERT(shapeController);
        shapeController->disconnect(SIGNAL(sigActivateNode(KisNodeSP)), this);
        m_d->imageView->image()->disconnect(this);
    }

    m_d->imageView = imageView;

    if (m_d->imageView) {
        KisShapeController *shapeController = dynamic_cast<KisShapeController*>(m_d->view->document()->shapeController());
        Q_ASSERT(shapeController);
        connect(shapeController, SIGNAL(sigActivateNode(KisNodeSP)), SLOT(slotNonUiActivatedNode(KisNodeSP)));
        connect(m_d->imageView->image(), SIGNAL(sigIsolatedModeChanged()),this, SLOT(slotUpdateIsolateModeAction()));
        m_d->imageView->resourceProvider()->slotNodeActivated(m_d->imageView->currentNode());
    }

}

#define NEW_LAYER_ACTION(id, text, layerType, icon)                     \
    {                                                                   \
        action = new KisAction(icon, text, this);                       \
        actionManager->addAction(id, action);         \
        m_d->nodeCreationSignalMapper.setMapping(action, layerType);    \
        connect(action, SIGNAL(triggered()),                            \
                &m_d->nodeCreationSignalMapper, SLOT(map()));           \
    }

#define NEW_LAYER_ACTION_KEY(id, text, layerType, icon, shortcut)       \
    {                                                                   \
        NEW_LAYER_ACTION(id, text, layerType, icon);                    \
        action->setShortcut(KShortcut(shortcut));                       \
    }

#define NEW_MASK_ACTION(id, text, layerType, icon)                      \
    {                                                                   \
        NEW_LAYER_ACTION(id, text, layerType, icon);                    \
        action->setActivationFlags(KisAction::ACTIVE_LAYER);            \
    }

#define CONVERT_NODE_ACTION(id, text, layerType, icon)                  \
    {                                                                   \
        action = new KisAction(icon, text, this);                       \
        action->setActivationFlags(KisAction::ACTIVE_NODE);             \
        action->setExcludedNodeTypes(QStringList(layerType));           \
        actionManager->addAction(id, action);         \
        m_d->nodeConversionSignalMapper.setMapping(action, layerType);  \
        connect(action, SIGNAL(triggered()),                            \
                &m_d->nodeConversionSignalMapper, SLOT(map()));         \
    }

void KisNodeManager::setup(KActionCollection * actionCollection, KisActionManager* actionManager)
{
    m_d->layerManager->setup(actionManager);
    m_d->maskManager->setup(actionCollection, actionManager);

    KisAction * action  = new KisAction(koIcon("object-flip-horizontal"), i18n("Mirror Layer Horizontally"), this);
    action->setActivationFlags(KisAction::ACTIVE_NODE);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    actionManager->addAction("mirrorNodeX", action);
    connect(action, SIGNAL(triggered()), this, SLOT(mirrorNodeX()));

    action  = new KisAction(koIcon("object-flip-vertical"), i18n("Mirror Layer Vertically"), this);
    action->setActivationFlags(KisAction::ACTIVE_NODE);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    actionManager->addAction("mirrorNodeY", action);
    connect(action, SIGNAL(triggered()), this, SLOT(mirrorNodeY()));

    action = new KisAction(i18n("Activate next layer"), this);
    action->setActivationFlags(KisAction::ACTIVE_LAYER);
    action->setShortcut(KShortcut(Qt::Key_PageUp));
    actionManager->addAction("activateNextLayer", action);
    connect(action, SIGNAL(triggered()), this, SLOT(activateNextNode()));

    action = new KisAction(i18n("Activate previous layer"), this);
    action->setActivationFlags(KisAction::ACTIVE_LAYER);
    action->setShortcut(KShortcut(Qt::Key_PageDown));
    actionManager->addAction("activatePreviousLayer", action);
    connect(action, SIGNAL(triggered()), this, SLOT(activatePreviousNode()));

    action  = new KisAction(koIcon("document-save"), i18n("Save Layer/Mask..."), this);
    action->setActivationFlags(KisAction::ACTIVE_NODE);
    actionManager->addAction("save_node_as_image", action);
    connect(action, SIGNAL(triggered()), this, SLOT(saveNodeAsImage()));

    action = new KisAction(koIcon("edit-copy"), i18n("&Duplicate Layer or Mask"), this);
    action->setActivationFlags(KisAction::ACTIVE_NODE);
    action->setShortcut(KShortcut(Qt::ControlModifier + Qt::Key_J));
    actionManager->addAction("duplicatelayer", action);
    connect(action, SIGNAL(triggered()), this, SLOT(duplicateActiveNode()));


    NEW_LAYER_ACTION_KEY("add_new_paint_layer", i18n("&Paint Layer"),
                         "KisPaintLayer", koIcon("document-new"),
                         Qt::Key_Insert);

    NEW_LAYER_ACTION("add_new_group_layer", i18n("&Group Layer"),
                     "KisGroupLayer", koIcon("folder-new"));

    NEW_LAYER_ACTION("add_new_clone_layer", i18n("&Clone Layer"),
                     "KisCloneLayer", koIcon("edit-copy"));

    NEW_LAYER_ACTION("add_new_shape_layer", i18n("&Vector Layer"),
                     "KisShapeLayer", koIcon("bookmark-new"));

    NEW_LAYER_ACTION("add_new_adjustment_layer", i18n("&Filter Layer..."),
                     "KisAdjustmentLayer", koIcon("view-filter"));

    NEW_LAYER_ACTION("add_new_fill_layer", i18n("&Fill Layer..."),
                     "KisGeneratorLayer", koIcon("krita_tool_color_fill"));

    NEW_LAYER_ACTION("add_new_file_layer", i18n("&File Layer..."),
                     "KisFileLayer", koIcon("document-open"));

    NEW_MASK_ACTION("add_new_transparency_mask", i18n("&Transparency Mask"),
                    "KisTransparencyMask", koIcon("edit-copy"));

    NEW_MASK_ACTION("add_new_filter_mask", i18n("&Filter Mask..."),
                    "KisFilterMask", koIcon("bookmarks"));

    NEW_MASK_ACTION("add_new_transform_mask", i18n("&Transform Mask..."),
                    "KisTransformMask", koIcon("bookmarks"));

    NEW_MASK_ACTION("add_new_selection_mask", i18n("&Local Selection"),
                    "KisSelectionMask", koIcon("edit-paste"));

    connect(&m_d->nodeCreationSignalMapper, SIGNAL(mapped(const QString &)),
            this, SLOT(createNode(const QString &)));

    CONVERT_NODE_ACTION("convert_to_paint_layer", i18n("to &Paint Layer"),
                        "KisPaintLayer", koIcon("document-new"));

    CONVERT_NODE_ACTION("convert_to_selection_mask", i18n("to &Selection Mask"),
                        "KisSelectionMask", koIcon("edit-paste"));

    CONVERT_NODE_ACTION("convert_to_filter_mask", i18n("to &Filter Mask..."),
                        "KisFilterMask", koIcon("bookmarks"));

    CONVERT_NODE_ACTION("convert_to_transparency_mask", i18n("to &Transparency Mask"),
                        "KisTransparencyMask", koIcon("edit-copy"));

    connect(&m_d->nodeConversionSignalMapper, SIGNAL(mapped(const QString &)),
            this, SLOT(convertNode(const QString &)));

    action = new KisAction(koIcon("view-filter"), i18n("&Isolate Layer"), this);
    action->setCheckable(true);
    action->setActivationFlags(KisAction::ACTIVE_NODE);
    actionManager->addAction("isolate_layer", action);
    connect(action, SIGNAL(triggered(bool)), this, SLOT(toggleIsolateMode(bool)));

    action  = new KisAction(koIcon("edit-copy"), i18n("Alpha into Mask"), this);
    action->setActivationFlags(KisAction::ACTIVE_LAYER);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE_PAINT_DEVICE);
    actionManager->addAction("split_alpha_into_mask", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotSplitAlphaIntoMask()));

    action  = new KisAction(koIcon("transparency-enabled"), i18n("Write as Alpha"), this);
    action->setActivationFlags(KisAction::ACTIVE_TRANSPARENCY_MASK);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    actionManager->addAction("split_alpha_write", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotSplitAlphaWrite()));

    action  = new KisAction(koIcon("document-save"), i18n("Save Merged..."), this);
    action->setActivationFlags(KisAction::ACTIVE_TRANSPARENCY_MASK);
    // HINT: we can save even when the nodes are not editable
    actionManager->addAction("split_alpha_save_merged", action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotSplitAlphaSaveMerged()));

    connect(this, SIGNAL(sigNodeActivated(KisNodeSP)), SLOT(slotUpdateIsolateModeAction()));
    connect(this, SIGNAL(sigNodeActivated(KisNodeSP)), SLOT(slotTryFinishIsolatedMode()));
}

void KisNodeManager::updateGUI()
{
    // enable/disable all relevant actions
    m_d->layerManager->updateGUI();
    m_d->maskManager->updateGUI();
}


KisNodeSP KisNodeManager::activeNode()
{
    if (m_d->imageView) {
        return m_d->imageView->currentNode();
    }
    return 0;
}

KisLayerSP KisNodeManager::activeLayer()
{
    return m_d->layerManager->activeLayer();
}

const KoColorSpace* KisNodeManager::activeColorSpace()
{
    Q_ASSERT(m_d->maskManager);

    if (m_d->maskManager->activeDevice()) {
//        Q_ASSERT(m_d->maskManager->activeDevice());
        return m_d->maskManager->activeDevice()->colorSpace();
    } else {
        Q_ASSERT(m_d->layerManager);
        Q_ASSERT(m_d->layerManager->activeLayer());
        if (m_d->layerManager->activeLayer()->parentLayer())
            return m_d->layerManager->activeLayer()->parentLayer()->colorSpace();
        else
            return m_d->view->image()->colorSpace();
    }
}

void KisNodeManager::moveNodeAt(KisNodeSP node, KisNodeSP parent, int index)
{
    if (parent->allowAsChild(node)) {
        if (node->inherits("KisSelectionMask") && parent->inherits("KisLayer")) {
            KisSelectionMask *m = dynamic_cast<KisSelectionMask*>(node.data());
            KisLayer *l = dynamic_cast<KisLayer*>(parent.data());
            KisSelectionMaskSP selMask = l->selectionMask();
            if (m && m->active() && l && l->selectionMask())
                selMask->setActive(false);
        }
        m_d->commandsAdapter->moveNode(node, parent, index);
    }
}

void KisNodeManager::addNodeDirect(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis)
{
    Q_ASSERT(parent->allowAsChild(node));
    m_d->commandsAdapter->addNode(node, parent, aboveThis);
}

void KisNodeManager::moveNodeDirect(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis)
{
    Q_ASSERT(parent->allowAsChild(node));
    m_d->commandsAdapter->moveNode(node, parent, aboveThis);
}

void KisNodeManager::toggleIsolateActiveNode()
{
    KisImageWSP image = m_d->view->image();
    KisNodeSP activeNode = this->activeNode();
    KIS_ASSERT_RECOVER_RETURN(activeNode);

    if (activeNode == image->isolatedModeRoot()) {
        toggleIsolateMode(false);
    } else {
        toggleIsolateMode(true);
    }
}

void KisNodeManager::toggleIsolateMode(bool checked)
{
    KisImageWSP image = m_d->view->image();

    if (checked) {
        KisNodeSP activeNode = this->activeNode();
        KIS_ASSERT_RECOVER_RETURN(activeNode);

        image->startIsolatedMode(activeNode);
    } else {
        image->stopIsolatedMode();
    }
}

void KisNodeManager::slotUpdateIsolateModeAction()
{
    KisAction *action = m_d->view->actionManager()->actionByName("isolate_layer");
    Q_ASSERT(action);

    KisNodeSP activeNode = this->activeNode();
    KisNodeSP isolatedRootNode = m_d->view->image()->isolatedModeRoot();

    action->setChecked(isolatedRootNode && isolatedRootNode == activeNode);
}

void KisNodeManager::slotTryFinishIsolatedMode()
{
    KisNodeSP isolatedRootNode = m_d->view->image()->isolatedModeRoot();
    if (!isolatedRootNode) return;

    bool belongsToIsolatedGroup = false;

    KisNodeSP node = this->activeNode();
    while(node) {
        if (node == isolatedRootNode) {
            belongsToIsolatedGroup = true;
            break;
        }
        node = node->parent();
    }

    if (!belongsToIsolatedGroup) {
        m_d->view->image()->stopIsolatedMode();
    }
}

void KisNodeManager::createNode(const QString & nodeType, bool quiet, KisPaintDeviceSP copyFrom)
{
    KisNodeSP activeNode = this->activeNode();
    if (!activeNode) {
        activeNode = m_d->view->image()->root();
    }

    KIS_ASSERT_RECOVER_RETURN(activeNode);
    if (activeNode->systemLocked()) {
        return;
    }

    // XXX: make factories for this kind of stuff,
    //      with a registry

    if (nodeType == "KisPaintLayer") {
        m_d->layerManager->addLayer(activeNode);
    } else if (nodeType == "KisGroupLayer") {
        m_d->layerManager->addGroupLayer(activeNode);
    } else if (nodeType == "KisAdjustmentLayer") {
        m_d->layerManager->addAdjustmentLayer(activeNode);
    } else if (nodeType == "KisGeneratorLayer") {
        m_d->layerManager->addGeneratorLayer(activeNode);
    } else if (nodeType == "KisShapeLayer") {
        m_d->layerManager->addShapeLayer(activeNode);
    } else if (nodeType == "KisCloneLayer") {
        m_d->layerManager->addCloneLayer(activeNode);
    } else if (nodeType == "KisTransparencyMask") {
        m_d->maskManager->createTransparencyMask(activeNode, copyFrom, false);
    } else if (nodeType == "KisFilterMask") {
        m_d->maskManager->createFilterMask(activeNode, copyFrom, quiet, false);
    } else if (nodeType == "KisTransformMask") {
        m_d->maskManager->createTransformMask(activeNode);
    } else if (nodeType == "KisSelectionMask") {
        m_d->maskManager->createSelectionMask(activeNode, copyFrom, false);
    } else if (nodeType == "KisFileLayer") {
        m_d->layerManager->addFileLayer(activeNode);
    }

}

void KisNodeManager::convertNode(const QString &nodeType)
{
    KisNodeSP activeNode = this->activeNode();
    if (!activeNode) return;

    if (nodeType == "KisPaintLayer") {
        m_d->layerManager->convertNodeToPaintLayer(activeNode);
    } else if (nodeType == "KisSelectionMask" ||
               nodeType == "KisFilterMask" ||
               nodeType == "KisTransparencyMask") {

        KisPaintDeviceSP copyFrom = activeNode->paintDevice() ?
            activeNode->paintDevice() : activeNode->projection();

        m_d->commandsAdapter->beginMacro(kundo2_i18n("Convert to a Selection Mask"));

        if (nodeType == "KisSelectionMask") {
            m_d->maskManager->createSelectionMask(activeNode, copyFrom, true);
        } else if (nodeType == "KisFilterMask") {
            m_d->maskManager->createFilterMask(activeNode, copyFrom, false, true);
        } else if (nodeType == "KisTransparencyMask") {
            m_d->maskManager->createTransparencyMask(activeNode, copyFrom, true);
        }

        m_d->commandsAdapter->removeNode(activeNode);
        m_d->commandsAdapter->endMacro();

    } else {
        qWarning() << "Unsupported node conversion type:" << nodeType;
    }
}

void KisNodeManager::slotNonUiActivatedNode(KisNodeSP node)
{
    if(m_d->activateNodeImpl(node)) {
        emit sigUiNeedChangeActiveNode(node);
        emit sigNodeActivated(node);
        nodesUpdated();
        if (node) {
            bool toggled =  m_d->view->actionCollection()->action("view_show_just_the_canvas")->isChecked();
            if (toggled) {
                m_d->view->showFloatingMessage( activeLayer()->name(), QIcon(), 1600, KisFloatingMessage::Medium, Qt::TextSingleLine);
            }
        }
    }
}

void KisNodeManager::slotUiActivatedNode(KisNodeSP node)
{
    if(m_d->activateNodeImpl(node)) {
        emit sigNodeActivated(node);
        nodesUpdated();
    }
}

void KisNodeManager::nodesUpdated()
{
    KisNodeSP node = activeNode();
    if (!node) return;

    m_d->layerManager->layersUpdated();
    m_d->maskManager->masksUpdated();

    m_d->view->updateGUI();
    m_d->view->selectionManager()->selectionChanged();

}

KisPaintDeviceSP KisNodeManager::activePaintDevice()
{
    return m_d->maskManager->activeMask() ?
        m_d->maskManager->activeDevice() :
        m_d->layerManager->activeDevice();
}

void KisNodeManager::nodeProperties(KisNodeSP node)
{
    if (node->inherits("KisLayer")) {
        m_d->layerManager->layerProperties();
    } else if (node->inherits("KisMask")) {
        m_d->maskManager->maskProperties();
    }
}

qint32 KisNodeManager::convertOpacityToInt(qreal opacity)
{
    /**
     * Scales opacity from the range 0...100
     * to the integer range 0...255
     */

    return qMin(255, int(opacity * 2.55 + 0.5));
}

void KisNodeManager::setNodeOpacity(KisNodeSP node, qint32 opacity,
                                    bool finalChange)
{
    if (!node) return;
    if (node->opacity() == opacity) return;

    if (!finalChange) {
        node->setOpacity(opacity);
        node->setDirty();
    } else {
        m_d->commandsAdapter->setOpacity(node, opacity);
    }
}

void KisNodeManager::setNodeCompositeOp(KisNodeSP node,
                                        const KoCompositeOp* compositeOp)
{
    if (!node) return;
    if (node->compositeOp() == compositeOp) return;

    m_d->commandsAdapter->setCompositeOp(node, compositeOp);
}

void KisNodeManager::setSelectedNodes(QList<KisNodeSP> nodes)
{
    m_d->selectedNodes = nodes;
}

QList<KisNodeSP> KisNodeManager::selectedNodes()
{
    return m_d->selectedNodes;
}

void KisNodeManager::nodeOpacityChanged(qreal opacity, bool finalChange)
{
    KisNodeSP node = activeNode();

    setNodeOpacity(node, convertOpacityToInt(opacity), finalChange);
}

void KisNodeManager::nodeCompositeOpChanged(const KoCompositeOp* op)
{
    KisNodeSP node = activeNode();

    setNodeCompositeOp(node, op);
}

void KisNodeManager::duplicateActiveNode()
{
    KisNodeSP node = activeNode();

    // FIXME: can't imagine how it may happen
    Q_ASSERT(node);

    if (node->inherits("KisLayer")) {
        m_d->layerManager->layerDuplicate();
    } else if (node->inherits("KisMask")) {
        m_d->maskManager->duplicateMask();
    }
}

void KisNodeManager::raiseNode()
{
    // The user sees the layer stack topsy-turvy, as a tree with the
    // root at the bottom instead of on top.
    KisNodeSP node = activeNode();
    if (node->inherits("KisLayer")) {
        m_d->layerManager->layerLower();
    } else if (node->inherits("KisMask")) {
        m_d->maskManager->lowerMask();
    }
}

void KisNodeManager::lowerNode()
{
    // The user sees the layer stack topsy-turvy, as a tree with the
    // root at the bottom instead of on top.
    KisNodeSP node = activeNode();

    if (node->inherits("KisLayer")) {
        m_d->layerManager->layerRaise();
    } else if (node->inherits("KisMask")) {
        m_d->maskManager->raiseMask();
    }
}

void KisNodeManager::nodeToTop()
{
    KisNodeSP node = activeNode();
    if (node->inherits("KisLayer")) {
        m_d->layerManager->layerBack();
    } else if (node->inherits("KisMask")) {
        m_d->maskManager->maskToBottom();
    }

}

void KisNodeManager::nodeToBottom()
{
    KisNodeSP node = activeNode();
    if (node->inherits("KisLayer")) {
        m_d->layerManager->layerLower();
    } else if (node->inherits("KisMask")) {
        m_d->maskManager->maskToTop();
    }
}

bool scanForLastLayer(KisImageWSP image, KisNodeSP nodeToRemove)
{
    if (!dynamic_cast<KisLayer*>(nodeToRemove.data())) {
        return false;
    }

    bool lastLayer = true;
    KisNodeSP node = image->root()->firstChild();
    while (node) {
        if (node != nodeToRemove && dynamic_cast<KisLayer*>(node.data())) {
            lastLayer = false;
            break;
        }
        node = node->nextSibling();
    }

    return lastLayer;
}

/// Scan whether the node has a parent in the list of nodes
bool scanForParent(QList<KisNodeSP> nodeList, KisNodeSP node)
{
    KisNodeSP parent = node->parent();

    while (parent) {
        if (nodeList.contains(parent)) {
            return true;
        }
        parent = parent->parent();
    }
    return false;
}

void KisNodeManager::removeSingleNode(KisNodeSP node)
{
    if (!node || !node->parent()) {
        return;
    }

    if (scanForLastLayer(m_d->view->image(), node)) {
        m_d->commandsAdapter->beginMacro(kundo2_i18n("Remove Last Layer"));
        m_d->commandsAdapter->removeNode(node);
        // An oddity, but this is required as for some reason, we can end up in a situation
        // where our active node is still set to one of the layers removed above.
        activeNode().clear();
        createNode("KisPaintLayer");
        m_d->commandsAdapter->endMacro();
    } else {
        m_d->commandsAdapter->removeNode(node);
    }
}

void KisNodeManager::removeSelectedNodes(QList<KisNodeSP> selectedNodes)
{
    m_d->commandsAdapter->beginMacro(kundo2_i18n("Remove Multiple Layers and Masks"));
    foreach(KisNodeSP node, selectedNodes) {
        if (!scanForParent(selectedNodes, node)) {
            removeSingleNode(node);
        }
    }
    m_d->commandsAdapter->endMacro();
}

void KisNodeManager::removeNode()
{
    //do not delete root layer
    if (m_d->selectedNodes.count() > 1) {
        removeSelectedNodes(m_d->selectedNodes);
    }
    else {
        removeSingleNode(activeNode());
    }


}

void KisNodeManager::mirrorNodeX()
{
    KisNodeSP node = activeNode();

    KUndo2MagicString commandName;
    if (node->inherits("KisLayer")) {
        commandName = kundo2_i18n("Mirror Layer X");
    } else if (node->inherits("KisMask")) {
        commandName = kundo2_i18n("Mirror Mask X");
    }
    mirrorNode(node, commandName, Qt::Horizontal);
}

void KisNodeManager::mirrorNodeY()
{
    KisNodeSP node = activeNode();

    KUndo2MagicString commandName;
    if (node->inherits("KisLayer")) {
        commandName = kundo2_i18n("Mirror Layer Y");
    } else if (node->inherits("KisMask")) {
        commandName = kundo2_i18n("Mirror Mask Y");
    }
    mirrorNode(node, commandName, Qt::Vertical);
}

inline bool checkForGlobalSelection(KisNodeSP node) {
    return dynamic_cast<KisSelectionMask*>(node.data()) && node->parent() && !node->parent()->parent();
}

void KisNodeManager::activateNextNode()
{
    KisNodeSP activeNode = this->activeNode();
    if (!activeNode) return;

    KisNodeSP node = activeNode->nextSibling();

    if (!node && activeNode->parent() && activeNode->parent()->parent()) {
        node = activeNode->parent();
    }

    while(node && checkForGlobalSelection(node)) {
        node = node->nextSibling();
    }

    if (node) {
        slotNonUiActivatedNode(node);
    }
}

void KisNodeManager::activatePreviousNode()
{
    KisNodeSP activeNode = this->activeNode();
    if (!activeNode) return;

    KisNodeSP node = activeNode->prevSibling();

    if (!node && activeNode->parent()) {
        node = activeNode->parent()->prevSibling();
    }

    while(node && checkForGlobalSelection(node)) {
        node = node->prevSibling();
    }

    if (node) {
        slotNonUiActivatedNode(node);
    }
}

QList<KisNodeSP> hideLayers(KisNodeSP root, QList<KisNodeSP> selectedNodes)
{
    QList<KisNodeSP> alreadyHidden;

    foreach(KisNodeSP node, root->childNodes(QStringList(), KoProperties())) {
        if (!selectedNodes.contains(node)) {
            if (node->visible()) {
                node->setVisible(false);
            }
            else {
                alreadyHidden << node;
            }
        }
        if (node->childCount() > 0) {
            hideLayers(node, selectedNodes);
        }
    }

    return alreadyHidden;
}

void showNodes(KisNodeSP root, QList<KisNodeSP> keepHiddenNodes)
{
    foreach(KisNodeSP node, root->childNodes(QStringList(), KoProperties())) {
        if (!keepHiddenNodes.contains(node)) {
            node->setVisible(true);
        }
        if (node->childCount() > 0) {
            showNodes(node, keepHiddenNodes);
        }
    }
}

void KisNodeManager::mergeLayerDown()
{
    if (m_d->selectedNodes.size() > 1) {

        QList<KisNodeSP> selectedNodes = m_d->selectedNodes;
        KisLayerSP l = activeLayer();

        // hide every layer that's not in the list of selected nodes
        QList<KisNodeSP> alreadyHiddenNodes = hideLayers(m_d->imageView->image()->root(), selectedNodes);

        // render and copy the projection
        m_d->imageView->image()->refreshGraph();
        m_d->imageView->image()->waitForDone();

        // Copy the projections
        KisPaintDeviceSP dev = new KisPaintDevice(*m_d->imageView->image()->projection().data());
        // place the projection in a layer
        KisPaintLayerSP flattenLayer = new KisPaintLayer(m_d->imageView->image(), i18n("Merged"), OPACITY_OPAQUE_U8, dev);

        // start a big macro
        m_d->commandsAdapter->beginMacro(kundo2_i18n("Merge Selected Nodes"));

        // Add the new merged node on top of the active node
        m_d->commandsAdapter->addNode(flattenLayer, l->parent(), l);

        // remove all nodes in the selection but the active node
        removeSelectedNodes(selectedNodes);

        m_d->commandsAdapter->endMacro();

        // And unhide
        showNodes(m_d->imageView->image()->root(), alreadyHiddenNodes);

        m_d->imageView->image()->refreshGraph();
        m_d->imageView->image()->waitForDone();
        m_d->imageView->image()->notifyLayersChanged();
        m_d->imageView->image()->setModified();

        slotNonUiActivatedNode(flattenLayer);

    }
    else {
        m_d->layerManager->mergeLayer();
    }
}

void KisNodeManager::rotate(double radians)
{
    // XXX: implement rotation for masks as well
    m_d->layerManager->rotateLayer(radians);

}


void KisNodeManager::rotate180()
{
    rotate(M_PI);
}

void KisNodeManager::rotateLeft90()
{
   rotate(-M_PI / 2);
}

void KisNodeManager::rotateRight90()
{
    rotate(M_PI / 2);
}

void KisNodeManager::shear(double angleX, double angleY)
{
    // XXX: implement shear for masks as well
    m_d->layerManager->shearLayer(angleX, angleY);
}

void KisNodeManager::scale(double sx, double sy, KisFilterStrategy *filterStrategy)
{
    KisNodeSP node = activeNode();
    KIS_ASSERT_RECOVER_RETURN(node);

    m_d->view->image()->scaleNode(node, sx, sy, filterStrategy);

    nodesUpdated();
}

void KisNodeManager::mirrorNode(KisNodeSP node, const KUndo2MagicString& actionName, Qt::Orientation orientation)
{
    KisImageSignalVector emitSignals;
    emitSignals << ModifiedSignal;

    KisProcessingApplicator applicator(m_d->view->image(), node,
                                       KisProcessingApplicator::RECURSIVE,
                                       emitSignals, actionName);

    KisProcessingVisitorSP visitor =
        new KisMirrorProcessingVisitor(m_d->view->image()->bounds(), orientation);

    applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT);
    applicator.end();

    nodesUpdated();
}

void KisNodeManager::Private::saveDeviceAsImage(KisPaintDeviceSP device,
                                                const QString &defaultName,
                                                const QRect &bounds,
                                                qreal xRes,
                                                qreal yRes,
                                                quint8 opacity)
{
    KoFileDialog dialog(view->mainWindow(), KoFileDialog::SaveFile, "krita/savenodeasimage");
    dialog.setCaption(i18n("Export \"%1\"", defaultName));
    dialog.setDefaultDir(QDesktopServices::storageLocation(QDesktopServices::PicturesLocation));
    dialog.setMimeTypeFilters(KisImportExportManager::mimeFilter("application/x-krita", KisImportExportManager::Export));
    QString filename = dialog.url();

    if (filename.isEmpty()) return;

    KUrl url = KUrl::fromLocalFile(filename);

    if (url.isEmpty()) return;

    KMimeType::Ptr mime = KMimeType::findByUrl(url);
    QString mimefilter = mime->name();

    QScopedPointer<KisDocument> d(KisPart::instance()->createDocument());
    d->prepareForImport();

    KisImageSP dst = new KisImage(d->createUndoStore(),
                                  bounds.width(),
                                  bounds.height(),
                                  device->compositionSourceColorSpace(),
                                  defaultName);
    dst->setResolution(xRes, yRes);
    d->setCurrentImage(dst);
    KisPaintLayer* paintLayer = new KisPaintLayer(dst, "paint device", opacity);
    paintLayer->paintDevice()->makeCloneFrom(device, bounds);
    dst->addNode(paintLayer, dst->rootLayer(), KisLayerSP(0));

    dst->initialRefreshGraph();

    d->setOutputMimeType(mimefilter.toLatin1());
    d->exportDocument(url);
}

void KisNodeManager::saveNodeAsImage()
{
    KisNodeSP node = activeNode();

    if (!node) {
        qWarning() << "BUG: Save Node As Image was called without any node selected";
        return;
    }

    KisImageWSP image = m_d->view->image();
    QRect saveRect = image->bounds() | node->exactBounds();

    KisPaintDeviceSP device = node->paintDevice();
    if (!device) {
        device = node->projection();
    }

    m_d->saveDeviceAsImage(device, node->name(),
                           saveRect,
                           image->xRes(), image->yRes(),
                           node->opacity());
}

void KisNodeManager::slotSplitAlphaIntoMask()
{
    KisNodeSP node = activeNode();

    // guaranteed by KisActionManager
    KIS_ASSERT_RECOVER_RETURN(node->hasEditablePaintDevice());

    KisPaintDeviceSP srcDevice = node->paintDevice();
    const KoColorSpace *srcCS = srcDevice->colorSpace();
    const QRect processRect =
        srcDevice->exactBounds() |
        srcDevice->defaultBounds()->bounds();

    KisPaintDeviceSP selectionDevice =
        new KisPaintDevice(KoColorSpaceRegistry::instance()->alpha8());

    m_d->commandsAdapter->beginMacro(kundo2_i18n("Split Alpha into a Mask"));
    KisTransaction transaction(kundo2_noi18n("__split_alpha_channel__"), srcDevice);

    KisSequentialIterator srcIt(srcDevice, processRect);
    KisSequentialIterator dstIt(selectionDevice, processRect);

    do {
        quint8 *srcPtr = srcIt.rawData();
        quint8 *alpha8Ptr = dstIt.rawData();

        *alpha8Ptr = srcCS->opacityU8(srcPtr);
        srcCS->setOpacity(srcPtr, OPACITY_OPAQUE_U8, 1);
    } while (srcIt.nextPixel() && dstIt.nextPixel());

    m_d->commandsAdapter->addExtraCommand(transaction.endAndTake());

    createNode("KisTransparencyMask", false, selectionDevice);
    m_d->commandsAdapter->endMacro();
}

void KisNodeManager::Private::mergeTransparencyMaskAsAlpha(bool writeToLayers)
{
    KisNodeSP node = q->activeNode();
    KisNodeSP parentNode = node->parent();

    // guaranteed by KisActionManager
    KIS_ASSERT_RECOVER_RETURN(node->inherits("KisTransparencyMask"));

    if (writeToLayers && !parentNode->hasEditablePaintDevice()) {
        QMessageBox::information(view->mainWindow(),
                                 i18nc("@title:window", "Layer %1 is not editable").arg(parentNode->name()),
                                 i18n("Cannot write alpha channel of "
                                      "the parent layer \"%1\".\n"
                                      "The operation will be cancelled.").arg(parentNode->name()));
        return;
    }

    KisPaintDeviceSP dstDevice;
    if (writeToLayers) {
        KIS_ASSERT_RECOVER_RETURN(parentNode->paintDevice());
        dstDevice = parentNode->paintDevice();
    } else {
        KisPaintDeviceSP copyDevice = parentNode->paintDevice();
        if (!copyDevice) {
            copyDevice = parentNode->original();
        }
        dstDevice = new KisPaintDevice(*copyDevice);
    }

    const KoColorSpace *dstCS = dstDevice->colorSpace();

    KisPaintDeviceSP selectionDevice = node->paintDevice();
    KIS_ASSERT_RECOVER_RETURN(selectionDevice->colorSpace()->pixelSize() == 1);

    const QRect processRect =
        selectionDevice->exactBounds() |
        dstDevice->exactBounds() |
        selectionDevice->defaultBounds()->bounds();

    QScopedPointer<KisTransaction> transaction;

    if (writeToLayers) {
        commandsAdapter->beginMacro(kundo2_i18n("Write Alpha into a Layer"));
        transaction.reset(new KisTransaction(kundo2_noi18n("__write_alpha_channel__"), dstDevice));
    }

    KisSequentialIterator srcIt(selectionDevice, processRect);
    KisSequentialIterator dstIt(dstDevice, processRect);

    do {
        quint8 *alpha8Ptr = srcIt.rawData();
        quint8 *dstPtr = dstIt.rawData();

        dstCS->setOpacity(dstPtr, *alpha8Ptr, 1);
    } while (srcIt.nextPixel() && dstIt.nextPixel());

    if (writeToLayers) {
        commandsAdapter->addExtraCommand(transaction->endAndTake());
        commandsAdapter->removeNode(node);
        commandsAdapter->endMacro();
    } else {
        KisImageWSP image = view->image();
        QRect saveRect = image->bounds();

        saveDeviceAsImage(dstDevice, parentNode->name(),
                          saveRect,
                          image->xRes(), image->yRes(),
                          OPACITY_OPAQUE_U8);
    }
}


void KisNodeManager::slotSplitAlphaWrite()
{
    m_d->mergeTransparencyMaskAsAlpha(true);
}

void KisNodeManager::slotSplitAlphaSaveMerged()
{
    m_d->mergeTransparencyMaskAsAlpha(false);
}


#include "kis_node_manager.moc"

