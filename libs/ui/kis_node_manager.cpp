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

#include <QStandardPaths>
#include <QMessageBox>
#include <QSignalMapper>
#include <QApplication>
#include <QMessageBox>

#include <kactioncollection.h>

#include <QKeySequence>

#include <kis_icon.h>
#include <KoSelection.h>
#include <KoShapeManager.h>
#include <KoShape.h>
#include <KoShapeLayer.h>
#include <KisImportExportManager.h>
#include <KoFileDialog.h>
#include <KoToolManager.h>
#include <KoProperties.h>

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
#include <KisMimeDatabase.h>
#include <KisReferenceImagesLayer.h>

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
#include "kis_node_selection_adapter.h"
#include "kis_node_insertion_adapter.h"
#include "kis_node_juggler_compressed.h"
#include "KisNodeDisplayModeAdapter.h"
#include "kis_clipboard.h"
#include "kis_node_dummies_graph.h"
#include "kis_mimedata.h"
#include "kis_layer_utils.h"
#include "krita_utils.h"
#include "kis_shape_layer.h"

#include "processing/kis_mirror_processing_visitor.h"
#include "KisView.h"

#include <kis_signals_blocker.h>
#include <libs/image/kis_layer_properties_icons.h>
#include <libs/image/commands/kis_node_property_list_command.h>

struct KisNodeManager::Private {

    Private(KisNodeManager *_q, KisViewManager *v)
        : q(_q)
        , view(v)
        , imageView(0)
        , layerManager(v)
        , maskManager(v)
        , commandsAdapter(v)
        , nodeSelectionAdapter(new KisNodeSelectionAdapter(q))
        , nodeInsertionAdapter(new KisNodeInsertionAdapter(q))
        , nodeDisplayModeAdapter(new KisNodeDisplayModeAdapter())
    {
    }

    KisNodeManager * q;
    KisViewManager * view;
    QPointer<KisView>imageView;
    KisLayerManager layerManager;
    KisMaskManager maskManager;
    KisNodeCommandsAdapter commandsAdapter;
    QScopedPointer<KisNodeSelectionAdapter> nodeSelectionAdapter;
    QScopedPointer<KisNodeInsertionAdapter> nodeInsertionAdapter;
    QScopedPointer<KisNodeDisplayModeAdapter> nodeDisplayModeAdapter;

    KisAction *showInTimeline;

    KisNodeList selectedNodes;
    QPointer<KisNodeJugglerCompressed> nodeJuggler;

    KisNodeWSP previouslyActiveNode;

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
    KisNodeJugglerCompressed* lazyGetJuggler(const KUndo2MagicString &actionName);
};

bool KisNodeManager::Private::activateNodeImpl(KisNodeSP node)
{
    Q_ASSERT(view);
    Q_ASSERT(view->canvasBase());
    Q_ASSERT(view->canvasBase()->globalShapeManager());
    Q_ASSERT(imageView);
    if (node && node == q->activeNode()) {
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
        maskManager.activateMask(0);
        layerManager.activateLayer(0);
        previouslyActiveNode = q->activeNode();
    } else {

        previouslyActiveNode = q->activeNode();

        KoShape * shape = view->document()->shapeForNode(node);

        //if (!shape) return false;
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(shape, false);

        selection->select(shape);
        KoShapeLayer * shapeLayer = dynamic_cast<KoShapeLayer*>(shape);

        //if (!shapeLayer) return false;
        KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(shapeLayer, false);

//         shapeLayer->setGeometryProtected(node->userLocked());
//         shapeLayer->setVisible(node->visible());
        selection->setActiveLayer(shapeLayer);

        imageView->setCurrentNode(node);
        if (KisLayerSP layer = qobject_cast<KisLayer*>(node.data())) {
            maskManager.activateMask(0);
            layerManager.activateLayer(layer);
        } else if (KisMaskSP mask = dynamic_cast<KisMask*>(node.data())) {
            maskManager.activateMask(mask);
            // XXX_NODE: for now, masks cannot be nested.
            layerManager.activateLayer(static_cast<KisLayer*>(node->parent().data()));
        }
    }
    return true;
}

KisNodeManager::KisNodeManager(KisViewManager *view)
    : m_d(new Private(this, view))
{

    connect(&m_d->layerManager, SIGNAL(sigLayerActivated(KisLayerSP)), SIGNAL(sigLayerActivated(KisLayerSP)));
}

KisNodeManager::~KisNodeManager()
{
    delete m_d;
}

void KisNodeManager::setView(QPointer<KisView>imageView)
{
    m_d->maskManager.setView(imageView);
    m_d->layerManager.setView(imageView);

    if (m_d->imageView) {
        KisShapeController *shapeController = dynamic_cast<KisShapeController*>(m_d->imageView->document()->shapeController());
        Q_ASSERT(shapeController);
        shapeController->disconnect(SIGNAL(sigActivateNode(KisNodeSP)), this);
        m_d->imageView->image()->disconnect(this);
    }

    m_d->imageView = imageView;

    if (m_d->imageView) {
        KisShapeController *shapeController = dynamic_cast<KisShapeController*>(m_d->imageView->document()->shapeController());
        Q_ASSERT(shapeController);
        connect(shapeController, SIGNAL(sigActivateNode(KisNodeSP)), SLOT(slotNonUiActivatedNode(KisNodeSP)));
        connect(m_d->imageView->image(), SIGNAL(sigIsolatedModeChanged()),this, SLOT(slotUpdateIsolateModeAction()));
        connect(m_d->imageView->image(), SIGNAL(sigRequestNodeReselection(KisNodeSP,KisNodeList)),this, SLOT(slotImageRequestNodeReselection(KisNodeSP,KisNodeList)));
        m_d->imageView->resourceProvider()->slotNodeActivated(m_d->imageView->currentNode());
    }

}

#define NEW_LAYER_ACTION(id, layerType)                                 \
    {                                                                   \
        action = actionManager->createAction(id);                       \
        m_d->nodeCreationSignalMapper.setMapping(action, layerType);    \
        connect(action, SIGNAL(triggered()),                            \
                &m_d->nodeCreationSignalMapper, SLOT(map()));           \
    }

#define CONVERT_NODE_ACTION_2(id, layerType, exclude)                   \
    {                                                                   \
        action = actionManager->createAction(id);                       \
        action->setExcludedNodeTypes(QStringList(exclude));             \
        actionManager->addAction(id, action);                           \
        m_d->nodeConversionSignalMapper.setMapping(action, layerType);  \
        connect(action, SIGNAL(triggered()),                            \
                &m_d->nodeConversionSignalMapper, SLOT(map()));         \
    }

#define CONVERT_NODE_ACTION(id, layerType)              \
    CONVERT_NODE_ACTION_2(id, layerType, layerType)

void KisNodeManager::setup(KActionCollection * actionCollection, KisActionManager* actionManager)
{
    m_d->layerManager.setup(actionManager);
    m_d->maskManager.setup(actionCollection, actionManager);

    KisAction * action = 0;

    action = actionManager->createAction("mirrorNodeX");
    connect(action, SIGNAL(triggered()), this, SLOT(mirrorNodeX()));

    action  = actionManager->createAction("mirrorNodeY");
    connect(action, SIGNAL(triggered()), this, SLOT(mirrorNodeY()));

    action = actionManager->createAction("mirrorAllNodesX");
    connect(action, SIGNAL(triggered()), this, SLOT(mirrorAllNodesX()));

    action  = actionManager->createAction("mirrorAllNodesY");
    connect(action, SIGNAL(triggered()), this, SLOT(mirrorAllNodesY()));

    action = actionManager->createAction("activateNextLayer");
    connect(action, SIGNAL(triggered()), this, SLOT(activateNextNode()));

    action = actionManager->createAction("activatePreviousLayer");
    connect(action, SIGNAL(triggered()), this, SLOT(activatePreviousNode()));

    action = actionManager->createAction("switchToPreviouslyActiveNode");
    connect(action, SIGNAL(triggered()), this, SLOT(switchToPreviouslyActiveNode()));

    action  = actionManager->createAction("save_node_as_image");
    connect(action, SIGNAL(triggered()), this, SLOT(saveNodeAsImage()));

    action  = actionManager->createAction("save_vector_node_to_svg");
    connect(action, SIGNAL(triggered()), this, SLOT(saveVectorLayerAsImage()));
    action->setActivationFlags(KisAction::ACTIVE_SHAPE_LAYER);

    action = actionManager->createAction("duplicatelayer");
    connect(action, SIGNAL(triggered()), this, SLOT(duplicateActiveNode()));

    action = actionManager->createAction("copy_layer_clipboard");
    connect(action, SIGNAL(triggered()), this, SLOT(copyLayersToClipboard()));

    action = actionManager->createAction("cut_layer_clipboard");
    connect(action, SIGNAL(triggered()), this, SLOT(cutLayersToClipboard()));

    action = actionManager->createAction("paste_layer_from_clipboard");
    connect(action, SIGNAL(triggered()), this, SLOT(pasteLayersFromClipboard()));

    action = actionManager->createAction("create_quick_group");
    connect(action, SIGNAL(triggered()), this, SLOT(createQuickGroup()));

    action = actionManager->createAction("create_quick_clipping_group");
    connect(action, SIGNAL(triggered()), this, SLOT(createQuickClippingGroup()));

    action = actionManager->createAction("quick_ungroup");
    connect(action, SIGNAL(triggered()), this, SLOT(quickUngroup()));

    action = actionManager->createAction("select_all_layers");
    connect(action, SIGNAL(triggered()), this, SLOT(selectAllNodes()));

    action = actionManager->createAction("select_visible_layers");
    connect(action, SIGNAL(triggered()), this, SLOT(selectVisibleNodes()));

    action = actionManager->createAction("select_locked_layers");
    connect(action, SIGNAL(triggered()), this, SLOT(selectLockedNodes()));

    action = actionManager->createAction("select_invisible_layers");
    connect(action, SIGNAL(triggered()), this, SLOT(selectInvisibleNodes()));

    action = actionManager->createAction("select_unlocked_layers");
    connect(action, SIGNAL(triggered()), this, SLOT(selectUnlockedNodes()));

    action = actionManager->createAction("new_from_visible");
    connect(action, SIGNAL(triggered()), this, SLOT(createFromVisible()));

    action = actionManager->createAction("show_in_timeline");
    action->setCheckable(true);
    connect(action, SIGNAL(toggled(bool)), this, SLOT(slotShowHideTimeline(bool)));
    m_d->showInTimeline = action;

    NEW_LAYER_ACTION("add_new_paint_layer", "KisPaintLayer");

    NEW_LAYER_ACTION("add_new_group_layer", "KisGroupLayer");

    NEW_LAYER_ACTION("add_new_clone_layer", "KisCloneLayer");

    NEW_LAYER_ACTION("add_new_shape_layer", "KisShapeLayer");

    NEW_LAYER_ACTION("add_new_adjustment_layer", "KisAdjustmentLayer");

    NEW_LAYER_ACTION("add_new_fill_layer", "KisGeneratorLayer");

    NEW_LAYER_ACTION("add_new_file_layer", "KisFileLayer");

    NEW_LAYER_ACTION("add_new_transparency_mask", "KisTransparencyMask");

    NEW_LAYER_ACTION("add_new_filter_mask", "KisFilterMask");

    NEW_LAYER_ACTION("add_new_colorize_mask", "KisColorizeMask");

    NEW_LAYER_ACTION("add_new_transform_mask", "KisTransformMask");

    NEW_LAYER_ACTION("add_new_selection_mask", "KisSelectionMask");

    connect(&m_d->nodeCreationSignalMapper, SIGNAL(mapped(QString)),
            this, SLOT(createNode(QString)));

    CONVERT_NODE_ACTION("convert_to_paint_layer", "KisPaintLayer");

    CONVERT_NODE_ACTION_2("convert_to_selection_mask", "KisSelectionMask", QStringList() << "KisSelectionMask" << "KisColorizeMask");

    CONVERT_NODE_ACTION_2("convert_to_filter_mask", "KisFilterMask", QStringList() << "KisFilterMask" << "KisColorizeMask");

    CONVERT_NODE_ACTION_2("convert_to_transparency_mask", "KisTransparencyMask", QStringList() << "KisTransparencyMask" << "KisColorizeMask");

    CONVERT_NODE_ACTION("convert_to_animated", "animated");

    CONVERT_NODE_ACTION_2("convert_to_file_layer", "KisFileLayer", QStringList() << "KisGroupLayer" << "KisFileLayer" << "KisCloneLayer");

    connect(&m_d->nodeConversionSignalMapper, SIGNAL(mapped(QString)),
            this, SLOT(convertNode(QString)));

    action = actionManager->createAction("isolate_layer");
    connect(action, SIGNAL(triggered(bool)), this, SLOT(toggleIsolateMode(bool)));

    action = actionManager->createAction("toggle_layer_visibility");
    connect(action, SIGNAL(triggered()), this, SLOT(toggleVisibility()));

    action = actionManager->createAction("toggle_layer_lock");
    connect(action, SIGNAL(triggered()), this, SLOT(toggleLock()));

    action = actionManager->createAction("toggle_layer_inherit_alpha");
    connect(action, SIGNAL(triggered()), this, SLOT(toggleInheritAlpha()));

    action = actionManager->createAction("toggle_layer_alpha_lock");
    connect(action, SIGNAL(triggered()), this, SLOT(toggleAlphaLock()));

    action  = actionManager->createAction("split_alpha_into_mask");
    connect(action, SIGNAL(triggered()), this, SLOT(slotSplitAlphaIntoMask()));

    action  = actionManager->createAction("split_alpha_write");
    connect(action, SIGNAL(triggered()), this, SLOT(slotSplitAlphaWrite()));

    // HINT: we can save even when the nodes are not editable
    action  = actionManager->createAction("split_alpha_save_merged");
    connect(action, SIGNAL(triggered()), this, SLOT(slotSplitAlphaSaveMerged()));

    connect(this, SIGNAL(sigNodeActivated(KisNodeSP)), SLOT(slotUpdateIsolateModeAction()));
    connect(this, SIGNAL(sigNodeActivated(KisNodeSP)), SLOT(slotTryRestartIsolatedMode()));
}

void KisNodeManager::updateGUI()
{
    // enable/disable all relevant actions
    m_d->layerManager.updateGUI();
    m_d->maskManager.updateGUI();
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
    return m_d->layerManager.activeLayer();
}

const KoColorSpace* KisNodeManager::activeColorSpace()
{
    if (m_d->maskManager.activeDevice()) {
        return m_d->maskManager.activeDevice()->colorSpace();
    } else {
        Q_ASSERT(m_d->layerManager.activeLayer());
        if (m_d->layerManager.activeLayer()->parentLayer())
            return m_d->layerManager.activeLayer()->parentLayer()->colorSpace();
        else
            return m_d->view->image()->colorSpace();
    }
}

void KisNodeManager::moveNodeAt(KisNodeSP node, KisNodeSP parent, int index)
{
    if (parent->allowAsChild(node)) {
        if (node->inherits("KisSelectionMask") && parent->inherits("KisLayer")) {
            KisSelectionMask *m = dynamic_cast<KisSelectionMask*>(node.data());
            KisLayer *l = qobject_cast<KisLayer*>(parent.data());
            if (m && m->active() && l && l->selectionMask()) {
                l->selectionMask()->setActive(false);
            }
        }
        m_d->commandsAdapter.moveNode(node, parent, index);
    }
}

void KisNodeManager::moveNodesDirect(KisNodeList nodes, KisNodeSP parent, KisNodeSP aboveThis)
{
    KUndo2MagicString actionName = kundo2_i18n("Move Nodes");
    KisNodeJugglerCompressed *juggler = m_d->lazyGetJuggler(actionName);
    juggler->moveNode(nodes, parent, aboveThis);
}

void KisNodeManager::copyNodesDirect(KisNodeList nodes, KisNodeSP parent, KisNodeSP aboveThis)
{
    KUndo2MagicString actionName = kundo2_i18n("Copy Nodes");
    KisNodeJugglerCompressed *juggler = m_d->lazyGetJuggler(actionName);
    juggler->copyNode(nodes, parent, aboveThis);
}

void KisNodeManager::addNodesDirect(KisNodeList nodes, KisNodeSP parent, KisNodeSP aboveThis)
{
    KUndo2MagicString actionName = kundo2_i18n("Add Nodes");
    KisNodeJugglerCompressed *juggler = m_d->lazyGetJuggler(actionName);
    juggler->addNode(nodes, parent, aboveThis);
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

    KisNodeSP activeNode = this->activeNode();
    if (checked && activeNode) {

        // Transform and colorize masks don't have pixel data...
        if (activeNode->inherits("KisTransformMask") ||
            activeNode->inherits("KisColorizeMask")) return;

        if (!image->startIsolatedMode(activeNode)) {
            KisAction *action = m_d->view->actionManager()->actionByName("isolate_layer");
            action->setChecked(false);
        }
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

void KisNodeManager::slotTryRestartIsolatedMode()
{
    KisNodeSP isolatedRootNode = m_d->view->image()->isolatedModeRoot();
    if (!isolatedRootNode) return;

    this->toggleIsolateMode(true);
}

KisNodeSP  KisNodeManager::createNode(const QString & nodeType, bool quiet, KisPaintDeviceSP copyFrom)
{
    if (!m_d->view->blockUntilOperationsFinished(m_d->view->image())) {
        return 0;
    }

    KisNodeSP activeNode = this->activeNode();
    if (!activeNode) {
        activeNode = m_d->view->image()->root();
    }

    KIS_ASSERT_RECOVER_RETURN_VALUE(activeNode, 0);

    // XXX: make factories for this kind of stuff,
    //      with a registry

    if (nodeType == "KisPaintLayer") {
        return m_d->layerManager.addPaintLayer(activeNode);
    } else if (nodeType == "KisGroupLayer") {
        return m_d->layerManager.addGroupLayer(activeNode);
    } else if (nodeType == "KisAdjustmentLayer") {
        return m_d->layerManager.addAdjustmentLayer(activeNode);
    } else if (nodeType == "KisGeneratorLayer") {
        return m_d->layerManager.addGeneratorLayer(activeNode);
    } else if (nodeType == "KisShapeLayer") {
        return m_d->layerManager.addShapeLayer(activeNode);
    } else if (nodeType == "KisCloneLayer") {
        return m_d->layerManager.addCloneLayer(activeNode);
    } else if (nodeType == "KisTransparencyMask") {
        return m_d->maskManager.createTransparencyMask(activeNode, copyFrom, false);
    } else if (nodeType == "KisFilterMask") {
        return m_d->maskManager.createFilterMask(activeNode, copyFrom, quiet, false);
    } else if (nodeType == "KisColorizeMask") {
        return m_d->maskManager.createColorizeMask(activeNode);
    } else if (nodeType == "KisTransformMask") {
        return m_d->maskManager.createTransformMask(activeNode);
    } else if (nodeType == "KisSelectionMask") {
        return m_d->maskManager.createSelectionMask(activeNode, copyFrom, false);
    } else if (nodeType == "KisFileLayer") {
        return m_d->layerManager.addFileLayer(activeNode);
    }
    return 0;
}

void KisNodeManager::createFromVisible()
{
    KisLayerUtils::newLayerFromVisible(m_d->view->image(), m_d->view->image()->root()->lastChild());
}

void KisNodeManager::slotShowHideTimeline(bool value)
{
    Q_FOREACH (KisNodeSP node, selectedNodes()) {
        node->setUseInTimeline(value);
    }
}

KisLayerSP KisNodeManager::createPaintLayer()
{
    KisNodeSP activeNode = this->activeNode();
    if (!activeNode) {
        activeNode = m_d->view->image()->root();
    }

    return m_d->layerManager.addPaintLayer(activeNode);
}

void KisNodeManager::convertNode(const QString &nodeType)
{
    KisNodeSP activeNode = this->activeNode();
    if (!activeNode) return;

    if (nodeType == "KisPaintLayer") {
        m_d->layerManager.convertNodeToPaintLayer(activeNode);
    } else if (nodeType == "KisSelectionMask" ||
               nodeType == "KisFilterMask" ||
               nodeType == "KisTransparencyMask") {

        KisPaintDeviceSP copyFrom = activeNode->paintDevice() ?
            activeNode->paintDevice() : activeNode->projection();

        m_d->commandsAdapter.beginMacro(kundo2_i18n("Convert to a Selection Mask"));

        bool result = false;

        if (nodeType == "KisSelectionMask") {
            result = !m_d->maskManager.createSelectionMask(activeNode, copyFrom, true).isNull();
        } else if (nodeType == "KisFilterMask") {
            result = !m_d->maskManager.createFilterMask(activeNode, copyFrom, false, true).isNull();
        } else if (nodeType == "KisTransparencyMask") {
            result = !m_d->maskManager.createTransparencyMask(activeNode, copyFrom, true).isNull();
        }

        m_d->commandsAdapter.endMacro();

        if (!result) {
            m_d->view->blockUntilOperationsFinishedForced(m_d->imageView->image());
            m_d->commandsAdapter.undoLastCommand();
        }

    } else if (nodeType == "KisFileLayer") {
            m_d->layerManager.convertLayerToFileLayer(activeNode);
    } else {
        warnKrita << "Unsupported node conversion type:" << nodeType;
    }
}

void KisNodeManager::slotSomethingActivatedNodeImpl(KisNodeSP node)
{
    KisDummiesFacadeBase *dummiesFacade = dynamic_cast<KisDummiesFacadeBase*>(m_d->imageView->document()->shapeController());
    KIS_SAFE_ASSERT_RECOVER_RETURN(dummiesFacade);

    const bool nodeVisible = !isNodeHidden(node, !m_d->nodeDisplayModeAdapter->showGlobalSelectionMask());
    if (!nodeVisible) {
        return;
    }

    KIS_ASSERT_RECOVER_RETURN(node != activeNode());
    if (m_d->activateNodeImpl(node)) {
        emit sigUiNeedChangeActiveNode(node);
        emit sigNodeActivated(node);
        nodesUpdated();
        if (node) {
            bool toggled =  m_d->view->actionCollection()->action("view_show_canvas_only")->isChecked();
            if (toggled) {
                m_d->view->showFloatingMessage( activeLayer()->name(), QIcon(), 1600, KisFloatingMessage::Medium, Qt::TextSingleLine);
            }
        }
    }
}

void KisNodeManager::slotNonUiActivatedNode(KisNodeSP node)
{
    // the node must still be in the graph, some asynchronous
    // signals may easily break this requirement
    if (node && !node->graphListener()) {
        node = 0;
    }

    if (node == activeNode()) return;

    slotSomethingActivatedNodeImpl(node);

    if (node) {
        bool toggled =  m_d->view->actionCollection()->action("view_show_canvas_only")->isChecked();
        if (toggled) {
            m_d->view->showFloatingMessage( activeLayer()->name(), QIcon(), 1600, KisFloatingMessage::Medium, Qt::TextSingleLine);
        }
    }
}

void KisNodeManager::slotUiActivatedNode(KisNodeSP node)
{
    // the node must still be in the graph, some asynchronous
    // signals may easily break this requirement
    if (node && !node->graphListener()) {
        node = 0;
    }

    if (node) {
        QStringList vectorTools = QStringList()
                << "InteractionTool"
                << "KarbonPatternTool"
                << "KarbonGradientTool"
                << "KarbonCalligraphyTool"
                << "CreateShapesTool"
                << "PathTool";

        QStringList pixelTools = QStringList()
                << "KritaShape/KisToolBrush"
                << "KritaShape/KisToolDyna"
                << "KritaShape/KisToolMultiBrush"
                << "KritaFill/KisToolFill"
                << "KritaFill/KisToolGradient";


        KisSelectionMask *selectionMask = dynamic_cast<KisSelectionMask*>(node.data());
        const bool nodeHasVectorAbilities = node->inherits("KisShapeLayer") ||
            (selectionMask && selectionMask->selection()->hasShapeSelection());

        if (nodeHasVectorAbilities) {
            if (pixelTools.contains(KoToolManager::instance()->activeToolId())) {
                KoToolManager::instance()->switchToolRequested("InteractionTool");
            }
        }
        else {
            if (vectorTools.contains(KoToolManager::instance()->activeToolId())) {
                KoToolManager::instance()->switchToolRequested("KritaShape/KisToolBrush");
            }
        }
    }

    if (node == activeNode()) return;

    slotSomethingActivatedNodeImpl(node);
}

void KisNodeManager::nodesUpdated()
{
    KisNodeSP node = activeNode();
    if (!node) return;

    m_d->layerManager.layersUpdated();
    m_d->maskManager.masksUpdated();

    m_d->view->updateGUI();
    m_d->view->selectionManager()->selectionChanged();

    {
        KisSignalsBlocker b(m_d->showInTimeline);
        m_d->showInTimeline->setChecked(node->useInTimeline());
    }
}

KisPaintDeviceSP KisNodeManager::activePaintDevice()
{
    return m_d->maskManager.activeMask() ?
        m_d->maskManager.activeDevice() :
        m_d->layerManager.activeDevice();
}

void KisNodeManager::nodeProperties(KisNodeSP node)
{
    if ((selectedNodes().size() > 1 && node->inherits("KisLayer")) || node->inherits("KisLayer")) {
        m_d->layerManager.layerProperties();
    }
    else if (node->inherits("KisMask")) {
        m_d->maskManager.maskProperties();
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
        m_d->commandsAdapter.setOpacity(node, opacity);
    }
}

void KisNodeManager::setNodeCompositeOp(KisNodeSP node,
                                        const KoCompositeOp* compositeOp)
{
    if (!node) return;
    if (node->compositeOp() == compositeOp) return;

    m_d->commandsAdapter.setCompositeOp(node, compositeOp);
}

void KisNodeManager::slotImageRequestNodeReselection(KisNodeSP activeNode, const KisNodeList &selectedNodes)
{
    if (activeNode) {
        slotNonUiActivatedNode(activeNode);
    }
    if (!selectedNodes.isEmpty()) {
        slotSetSelectedNodes(selectedNodes);
    }
}

void KisNodeManager::slotSetSelectedNodes(const KisNodeList &nodes)
{
    m_d->selectedNodes = nodes;
    emit sigUiNeedChangeSelectedNodes(nodes);
}

KisNodeList KisNodeManager::selectedNodes()
{
    return m_d->selectedNodes;
}

KisNodeSelectionAdapter* KisNodeManager::nodeSelectionAdapter() const
{
    return m_d->nodeSelectionAdapter.data();
}

KisNodeInsertionAdapter* KisNodeManager::nodeInsertionAdapter() const
{
    return m_d->nodeInsertionAdapter.data();
}

KisNodeDisplayModeAdapter *KisNodeManager::nodeDisplayModeAdapter() const
{
    return m_d->nodeDisplayModeAdapter.data();
}

bool KisNodeManager::isNodeHidden(KisNodeSP node, bool isGlobalSelectionHidden)
{
    if (dynamic_cast<KisReferenceImagesLayer *>(node.data())) {
        return true;
    }

    if (isGlobalSelectionHidden && dynamic_cast<KisSelectionMask *>(node.data()) &&
        (!node->parent() || !node->parent()->parent())) {
        return true;
    }

    return false;
}

bool KisNodeManager::trySetNodeProperties(KisNodeSP node, KisImageSP image, KisBaseNode::PropertyList properties) const
{
    const KisPaintLayer *paintLayer = dynamic_cast<KisPaintLayer*>(node.data());
    if (paintLayer) {
        const auto onionSkinOn = KisLayerPropertiesIcons::getProperty(KisLayerPropertiesIcons::onionSkins, true);

        if (properties.contains(onionSkinOn)) {
            const KisPaintDeviceSP &paintDevice = paintLayer->paintDevice();
            if (paintDevice && paintDevice->defaultPixel().opacityU8() == 255) {
                m_d->view->showFloatingMessage(i18n("Onion skins require a layer with transparent background."), QIcon());
                return false;
            }
        }
    }

    KisNodePropertyListCommand::setNodePropertiesNoUndo(node, image, properties);

    return true;
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
    KUndo2MagicString actionName = kundo2_i18n("Duplicate Nodes");
    KisNodeJugglerCompressed *juggler = m_d->lazyGetJuggler(actionName);
    juggler->duplicateNode(selectedNodes());
}

KisNodeJugglerCompressed* KisNodeManager::Private::lazyGetJuggler(const KUndo2MagicString &actionName)
{
    KisImageWSP image = view->image();

    if (!nodeJuggler ||
        (nodeJuggler &&
         (nodeJuggler->isEnded() ||
          !nodeJuggler->canMergeAction(actionName)))) {

        nodeJuggler = new KisNodeJugglerCompressed(actionName, image, q, 750);
        nodeJuggler->setAutoDelete(true);
    }

    return nodeJuggler;
}

void KisNodeManager::raiseNode()
{
    KUndo2MagicString actionName = kundo2_i18n("Raise Nodes");
    KisNodeJugglerCompressed *juggler = m_d->lazyGetJuggler(actionName);
    juggler->raiseNode(selectedNodes());
}

void KisNodeManager::lowerNode()
{
    KUndo2MagicString actionName = kundo2_i18n("Lower Nodes");
    KisNodeJugglerCompressed *juggler = m_d->lazyGetJuggler(actionName);
    juggler->lowerNode(selectedNodes());
}

void KisNodeManager::removeSingleNode(KisNodeSP node)
{
    if (!node || !node->parent()) {
        return;
    }

    KisNodeList nodes;
    nodes << node;
    removeSelectedNodes(nodes);
}

void KisNodeManager::removeSelectedNodes(KisNodeList nodes)
{
    KUndo2MagicString actionName = kundo2_i18n("Remove Nodes");
    KisNodeJugglerCompressed *juggler = m_d->lazyGetJuggler(actionName);
    juggler->removeNode(nodes);
}

void KisNodeManager::removeNode()
{
    removeSelectedNodes(selectedNodes());
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
    mirrorNode(node, commandName, Qt::Horizontal, m_d->view->selection());
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
    mirrorNode(node, commandName, Qt::Vertical, m_d->view->selection());
}

void KisNodeManager::mirrorAllNodesX()
{
    KisNodeSP node = m_d->view->image()->root();
    mirrorNode(node, kundo2_i18n("Mirror All Layers X"),
               Qt::Vertical, m_d->view->selection());
}

void KisNodeManager::mirrorAllNodesY()
{
    KisNodeSP node = m_d->view->image()->root();
    mirrorNode(node, kundo2_i18n("Mirror All Layers Y"),
               Qt::Vertical, m_d->view->selection());
}

void KisNodeManager::activateNextNode()
{
    KisNodeSP activeNode = this->activeNode();
    if (!activeNode) return;

    KisNodeSP node = activeNode->nextSibling();

    while (node && node->childCount() > 0) {
           node = node->firstChild();
    }

    if (!node && activeNode->parent() && activeNode->parent()->parent()) {
        node = activeNode->parent();
    }

    while(node && isNodeHidden(node, true)) {
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

    KisNodeSP node;

    if (activeNode->childCount() > 0) {
        node = activeNode->lastChild();
    }
    else {
        node = activeNode->prevSibling();
    }

    while (!node && activeNode->parent()) {
        node = activeNode->parent()->prevSibling();
        activeNode = activeNode->parent();
    }

    while(node && isNodeHidden(node, true)) {
        node = node->prevSibling();
    }

    if (node) {
        slotNonUiActivatedNode(node);
    }
}

void KisNodeManager::switchToPreviouslyActiveNode()
{
    if (m_d->previouslyActiveNode && m_d->previouslyActiveNode->parent()) {
        slotNonUiActivatedNode(m_d->previouslyActiveNode);
    }
}

void KisNodeManager::mirrorNode(KisNodeSP node,
                                const KUndo2MagicString& actionName,
                                Qt::Orientation orientation,
                                KisSelectionSP selection)
{
    KisImageSignalVector emitSignals;
    emitSignals << ModifiedSignal;

    KisProcessingApplicator applicator(m_d->view->image(), node,
                                       KisProcessingApplicator::RECURSIVE,
                                       emitSignals, actionName);

    KisProcessingVisitorSP visitor;

    if (selection) {
        visitor = new KisMirrorProcessingVisitor(selection, orientation);
    } else {
        visitor = new KisMirrorProcessingVisitor(m_d->view->image()->bounds(), orientation);
    }

    if (!selection) {
        applicator.applyVisitorAllFrames(visitor, KisStrokeJobData::CONCURRENT);
    } else {
        applicator.applyVisitor(visitor, KisStrokeJobData::CONCURRENT);
    }

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
    KoFileDialog dialog(view->mainWindow(), KoFileDialog::SaveFile, "savenodeasimage");
    dialog.setCaption(i18n("Export \"%1\"", defaultName));
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    dialog.setMimeTypeFilters(KisImportExportManager::supportedMimeTypes(KisImportExportManager::Export));
    QString filename = dialog.filename();

    if (filename.isEmpty()) return;

    QUrl url = QUrl::fromLocalFile(filename);

    if (url.isEmpty()) return;

    QString mimefilter = KisMimeDatabase::mimeTypeForFile(filename, false);

    QScopedPointer<KisDocument> doc(KisPart::instance()->createDocument());

    KisImageSP dst = new KisImage(doc->createUndoStore(),
                                  bounds.width(),
                                  bounds.height(),
                                  device->compositionSourceColorSpace(),
                                  defaultName);
    dst->setResolution(xRes, yRes);
    doc->setCurrentImage(dst);
    KisPaintLayer* paintLayer = new KisPaintLayer(dst, "paint device", opacity);
    paintLayer->paintDevice()->makeCloneFrom(device, bounds);
    dst->addNode(paintLayer, dst->rootLayer(), KisLayerSP(0));

    dst->initialRefreshGraph();

    if (!doc->exportDocumentSync(url, mimefilter.toLatin1())) {
        QMessageBox::warning(0,
                             i18nc("@title:window", "Krita"),
                             i18n("Could not save the layer. %1", doc->errorMessage().toUtf8().data()),
                             QMessageBox::Ok);

    }
}

void KisNodeManager::saveNodeAsImage()
{
    KisNodeSP node = activeNode();

    if (!node) {
        warnKrita << "BUG: Save Node As Image was called without any node selected";
        return;
    }

    KisImageWSP image = m_d->view->image();
    QRect saveRect = image->bounds() | node->exactBounds();

    m_d->saveDeviceAsImage(node->projection(),
                           node->name(),
                           saveRect,
                           image->xRes(), image->yRes(),
                           node->opacity());
}

#include "SvgWriter.h"

void KisNodeManager::saveVectorLayerAsImage()
{
    KisShapeLayerSP shapeLayer = qobject_cast<KisShapeLayer*>(activeNode().data());
    if (!shapeLayer) {
        return;
    }

    KoFileDialog dialog(m_d->view->mainWindow(), KoFileDialog::SaveFile, "savenodeasimage");
    dialog.setCaption(i18nc("@title:window", "Export to SVG"));
    dialog.setDefaultDir(QStandardPaths::writableLocation(QStandardPaths::PicturesLocation));
    dialog.setMimeTypeFilters(QStringList() << "image/svg+xml", "image/svg+xml");
    QString filename = dialog.filename();

    if (filename.isEmpty()) return;

    QUrl url = QUrl::fromLocalFile(filename);

    if (url.isEmpty()) return;

    const QSizeF sizeInPx = m_d->view->image()->bounds().size();
    const QSizeF sizeInPt(sizeInPx.width() / m_d->view->image()->xRes(),
                          sizeInPx.height() / m_d->view->image()->yRes());

    QList<KoShape*> shapes = shapeLayer->shapes();
    std::sort(shapes.begin(), shapes.end(), KoShape::compareShapeZIndex);

    SvgWriter writer(shapes);
    if (!writer.save(filename, sizeInPt, true)) {
        QMessageBox::warning(qApp->activeWindow(), i18nc("@title:window", "Krita"), i18n("Could not save to svg: %1", filename));
    }
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

    m_d->commandsAdapter.beginMacro(kundo2_i18n("Split Alpha into a Mask"));
    KisTransaction transaction(kundo2_noi18n("__split_alpha_channel__"), srcDevice);

    KisSequentialIterator srcIt(srcDevice, processRect);
    KisSequentialIterator dstIt(selectionDevice, processRect);

    while (srcIt.nextPixel() && dstIt.nextPixel()) {
        quint8 *srcPtr = srcIt.rawData();
        quint8 *alpha8Ptr = dstIt.rawData();

        *alpha8Ptr = srcCS->opacityU8(srcPtr);
        srcCS->setOpacity(srcPtr, OPACITY_OPAQUE_U8, 1);
    }

    m_d->commandsAdapter.addExtraCommand(transaction.endAndTake());

    createNode("KisTransparencyMask", false, selectionDevice);
    m_d->commandsAdapter.endMacro();
}

void KisNodeManager::Private::mergeTransparencyMaskAsAlpha(bool writeToLayers)
{
    KisNodeSP node = q->activeNode();
    KisNodeSP parentNode = node->parent();

    // guaranteed by KisActionManager
    KIS_ASSERT_RECOVER_RETURN(node->inherits("KisTransparencyMask"));

    if (writeToLayers && !parentNode->hasEditablePaintDevice()) {
        QMessageBox::information(view->mainWindow(),
                                 i18nc("@title:window", "Layer %1 is not editable", parentNode->name()),
                                 i18n("Cannot write alpha channel of "
                                      "the parent layer \"%1\".\n"
                                      "The operation will be cancelled.", parentNode->name()));
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
        commandsAdapter.beginMacro(kundo2_i18n("Write Alpha into a Layer"));
        transaction.reset(new KisTransaction(kundo2_noi18n("__write_alpha_channel__"), dstDevice));
    }

    KisSequentialIterator srcIt(selectionDevice, processRect);
    KisSequentialIterator dstIt(dstDevice, processRect);

    while (srcIt.nextPixel() && dstIt.nextPixel()) {
        quint8 *alpha8Ptr = srcIt.rawData();
        quint8 *dstPtr = dstIt.rawData();

        dstCS->setOpacity(dstPtr, *alpha8Ptr, 1);
    }

    if (writeToLayers) {
        commandsAdapter.addExtraCommand(transaction->endAndTake());
        commandsAdapter.removeNode(node);
        commandsAdapter.endMacro();
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

void KisNodeManager::toggleLock()
{
    KisNodeList nodes = this->selectedNodes();
    KisNodeSP active = activeNode();
    if (nodes.isEmpty() || !active) return;

    bool isLocked = active->userLocked();

    for (auto &node : nodes) {
        node->setUserLocked(!isLocked);
    }
}

void KisNodeManager::toggleVisibility()
{
    KisNodeList nodes = this->selectedNodes();
    KisNodeSP active = activeNode();
    if (nodes.isEmpty() || !active) return;

    bool isVisible = active->visible();

    for (auto &node : nodes) {
        node->setVisible(!isVisible);
        node->setDirty();
    }
}

void KisNodeManager::toggleAlphaLock()
{
    KisNodeList nodes = this->selectedNodes();
    KisNodeSP active = activeNode();
    if (nodes.isEmpty() || !active) return;

    auto layer = qobject_cast<KisPaintLayer*>(active.data());
    if (!layer) {
        return;
    }

    bool isAlphaLocked = layer->alphaLocked();
    for (auto &node : nodes) {
        auto layer = qobject_cast<KisPaintLayer*>(node.data());
        if (layer) {
            layer->setAlphaLocked(!isAlphaLocked);
        }
    }
}

void KisNodeManager::toggleInheritAlpha()
{
    KisNodeList nodes = this->selectedNodes();
    KisNodeSP active = activeNode();
    if (nodes.isEmpty() || !active) return;

    auto layer = qobject_cast<KisLayer*>(active.data());
    if (!layer) {
        return;
    }

    bool isAlphaDisabled = layer->alphaChannelDisabled();
    for (auto &node : nodes) {
        auto layer = qobject_cast<KisLayer*>(node.data());
        if (layer) {
            layer->disableAlphaChannel(!isAlphaDisabled);
            node->setDirty();
        }
    }
}

void KisNodeManager::cutLayersToClipboard()
{
    KisNodeList nodes = this->selectedNodes();
    if (nodes.isEmpty()) return;

    KisClipboard::instance()->setLayers(nodes, m_d->view->image(), false);

    KUndo2MagicString actionName = kundo2_i18n("Cut Nodes");
    KisNodeJugglerCompressed *juggler = m_d->lazyGetJuggler(actionName);
    juggler->removeNode(nodes);
}

void KisNodeManager::copyLayersToClipboard()
{
    KisNodeList nodes = this->selectedNodes();
    KisClipboard::instance()->setLayers(nodes, m_d->view->image(), true);
}

void KisNodeManager::pasteLayersFromClipboard()
{
    const QMimeData *data = KisClipboard::instance()->layersMimeData();
    if (!data) return;

    KisNodeSP activeNode = this->activeNode();

    KisShapeController *shapeController = dynamic_cast<KisShapeController*>(m_d->imageView->document()->shapeController());
    Q_ASSERT(shapeController);

    KisDummiesFacadeBase *dummiesFacade = dynamic_cast<KisDummiesFacadeBase*>(m_d->imageView->document()->shapeController());
    Q_ASSERT(dummiesFacade);

    const bool copyNode = false;
    KisImageSP image = m_d->view->image();
    KisNodeDummy *parentDummy = dummiesFacade->dummyForNode(activeNode);
    KisNodeDummy *aboveThisDummy = parentDummy ? parentDummy->lastChild() : 0;

    KisMimeData::insertMimeLayers(data,
                                  image,
                                  shapeController,
                                  parentDummy,
                                  aboveThisDummy,
                                  copyNode,
                                  nodeInsertionAdapter());
}

void KisNodeManager::createQuickGroupImpl(KisNodeJugglerCompressed *juggler,
                                          const QString &overrideGroupName,
                                          KisNodeSP *newGroup,
                                          KisNodeSP *newLastChild)
{
    KisNodeSP active = activeNode();
    if (!active) return;

    KisImageSP image = m_d->view->image();
    QString groupName = !overrideGroupName.isEmpty() ? overrideGroupName : image->nextLayerName();
    KisGroupLayerSP group = new KisGroupLayer(image.data(), groupName, OPACITY_OPAQUE_U8);

    KisNodeList nodes1;
    nodes1 << group;

    KisNodeList nodes2;
    nodes2 = KisLayerUtils::sortMergableNodes(image->root(), selectedNodes());
    KisLayerUtils::filterMergableNodes(nodes2);

    if (nodes2.size() == 0) return;

    if (KisLayerUtils::checkIsChildOf(active, nodes2)) {
        active = nodes2.first();
    }

    KisNodeSP parent = active->parent();
    KisNodeSP aboveThis = active;

    juggler->addNode(nodes1, parent, aboveThis);
    juggler->moveNode(nodes2, group, 0);

    *newGroup = group;
    *newLastChild = nodes2.last();
}

void KisNodeManager::createQuickGroup()
{
    KUndo2MagicString actionName = kundo2_i18n("Quick Group");
    KisNodeJugglerCompressed *juggler = m_d->lazyGetJuggler(actionName);

    KisNodeSP parent;
    KisNodeSP above;

    createQuickGroupImpl(juggler, "", &parent, &above);
}

void KisNodeManager::createQuickClippingGroup()
{
    KUndo2MagicString actionName = kundo2_i18n("Quick Clipping Group");
    KisNodeJugglerCompressed *juggler = m_d->lazyGetJuggler(actionName);

    KisNodeSP parent;
    KisNodeSP above;

    KisImageSP image = m_d->view->image();
    createQuickGroupImpl(juggler, image->nextLayerName(i18nc("default name for a clipping group layer", "Clipping Group")), &parent, &above);

    KisPaintLayerSP maskLayer = new KisPaintLayer(image.data(), i18nc("default name for quick clip group mask layer", "Mask Layer"), OPACITY_OPAQUE_U8, image->colorSpace());
    maskLayer->disableAlphaChannel(true);

    juggler->addNode(KisNodeList() << maskLayer, parent, above);
}

void KisNodeManager::quickUngroup()
{
    KisNodeSP active = activeNode();
    if (!active) return;

    KisNodeSP parent = active->parent();
    KisNodeSP aboveThis = active;

    KUndo2MagicString actionName = kundo2_i18n("Quick Ungroup");

    if (parent && dynamic_cast<KisGroupLayer*>(active.data())) {
        KisNodeList nodes = active->childNodes(QStringList(), KoProperties());

        KisNodeJugglerCompressed *juggler = m_d->lazyGetJuggler(actionName);
        juggler->moveNode(nodes, parent, active);
        juggler->removeNode(KisNodeList() << active);
    } else if (parent && parent->parent()) {
        KisNodeSP grandParent = parent->parent();

        KisNodeList allChildNodes = parent->childNodes(QStringList(), KoProperties());
        KisNodeList allSelectedNodes = selectedNodes();

        const bool removeParent = KritaUtils::compareListsUnordered(allChildNodes, allSelectedNodes);

        KisNodeJugglerCompressed *juggler = m_d->lazyGetJuggler(actionName);
        juggler->moveNode(allSelectedNodes, grandParent, parent);
        if (removeParent) {
            juggler->removeNode(KisNodeList() << parent);
        }
    }
}

void KisNodeManager::selectLayersImpl(const KoProperties &props, const KoProperties &invertedProps)
{
    KisImageSP image = m_d->view->image();
    KisNodeList nodes = KisLayerUtils::findNodesWithProps(image->root(), props, true);

    KisNodeList selectedNodes = this->selectedNodes();

    if (KritaUtils::compareListsUnordered(nodes, selectedNodes)) {
        nodes = KisLayerUtils::findNodesWithProps(image->root(), invertedProps, true);
    }

    if (!nodes.isEmpty()) {
        slotImageRequestNodeReselection(nodes.last(), nodes);
    }
}

void KisNodeManager::selectAllNodes()
{
    KoProperties props;
    selectLayersImpl(props, props);
}

void KisNodeManager::selectVisibleNodes()
{
    KoProperties props;
    props.setProperty("visible", true);

    KoProperties invertedProps;
    invertedProps.setProperty("visible", false);

    selectLayersImpl(props, invertedProps);
}

void KisNodeManager::selectLockedNodes()
{
    KoProperties props;
    props.setProperty("locked", true);

    KoProperties invertedProps;
    invertedProps.setProperty("locked", false);

    selectLayersImpl(props, invertedProps);
}

void KisNodeManager::selectInvisibleNodes()
{
    KoProperties props;
    props.setProperty("visible", false);

    KoProperties invertedProps;
    invertedProps.setProperty("visible", true);

    selectLayersImpl(props, invertedProps);
}

void KisNodeManager::selectUnlockedNodes()
{
    KoProperties props;
    props.setProperty("locked", false);

    KoProperties invertedProps;
    invertedProps.setProperty("locked", true);

    selectLayersImpl(props, invertedProps);
}
