/*
 *  kis_layer_box.cc - part of Krita aka Krayon aka KimageShop
 *
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (C) 2006 Gábor Lehel <illissius@gmail.com>
 *  Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *  Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2011 José Luis Vergara <pentalis@gmail.com>
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

#include "kis_layer_box.h"


#include <QToolButton>
#include <QLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QToolTip>
#include <QWidget>
#include <QComboBox>
#include <QCheckBox>
#include <QVBoxLayout>
#include <QPixmap>
#include <QList>
#include <QVector>
#include <QLabel>
#include <QMenu>

#include <kis_debug.h>
#include <klocalizedstring.h>

#include <kis_icon.h>
#include <KisNodeView.h>
#include <KoColorSpace.h>
#include <KoCompositeOpRegistry.h>
#include <KisDocument.h>

#include <kis_types.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_group_layer.h>
#include <kis_mask.h>
#include <kis_node.h>
#include <kis_composite_ops_model.h>

#include "kis_action.h"
#include "kis_action_manager.h"
#include "widgets/kis_cmb_composite.h"
#include "widgets/kis_slider_spin_box.h"
#include "KisViewManager.h"
#include "kis_node_manager.h"
#include "kis_node_model.h"
#include "canvas/kis_canvas2.h"
#include "KisDocument.h"
#include "kis_dummies_facade_base.h"
#include "kis_shape_controller.h"
#include "kis_selection_mask.h"
#include "kis_config.h"
#include "KisView.h"
#include "krita_utils.h"

#include "ui_wdglayerbox.h"

class ButtonAction : public KisAction
{
public:
    ButtonAction(QAbstractButton* button, const QIcon& icon, const QString& text, QObject* parent)
        : KisAction(icon, text, parent)
        , m_button(button)
    {
        connect(m_button, SIGNAL(clicked()), this, SLOT(trigger()));
    }

    ButtonAction(QAbstractButton* button, QObject* parent) : KisAction(parent) , m_button(button)
    {
        connect(m_button, SIGNAL(clicked()), this, SLOT(trigger()));
    }

    virtual void setActionEnabled(bool enabled) {
        KisAction::setActionEnabled(enabled);
        m_button->setEnabled(enabled);
    }
private:
    QAbstractButton* m_button;
};

inline void KisLayerBox::connectActionToButton(KisViewManager* view, QAbstractButton *button, const QString &id)
{
    Q_ASSERT(view);
    KisAction *action = view->actionManager()->actionByName(id);

    connect(button, SIGNAL(clicked()), action, SLOT(trigger()));
    connect(action, SIGNAL(sigEnableSlaves(bool)), button, SLOT(setEnabled(bool)));
}

inline void KisLayerBox::addActionToMenu(QMenu *menu, const QString &id)
{
    if (m_canvas) {
        menu->addAction(m_canvas->viewManager()->actionManager()->actionByName(id));
    }
}

KisLayerBox::KisLayerBox()
        : QDockWidget(i18n("Layers"))
        , m_canvas(0)
        , m_wdgLayerBox(new Ui_WdgLayerBox)
        , m_thumbnailCompressor(500, KisSignalCompressor::FIRST_INACTIVE)
{
    KisConfig cfg;

    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);
    m_opacityDelayTimer.setSingleShot(true);

    m_wdgLayerBox->setupUi(mainWidget);

    connect(m_wdgLayerBox->listLayers,
            SIGNAL(contextMenuRequested(const QPoint&, const QModelIndex&)),
            this, SLOT(slotContextMenuRequested(const QPoint&, const QModelIndex&)));
    connect(m_wdgLayerBox->listLayers,
            SIGNAL(collapsed(const QModelIndex&)), SLOT(slotCollapsed(const QModelIndex &)));
    connect(m_wdgLayerBox->listLayers,
            SIGNAL(expanded(const QModelIndex&)), SLOT(slotExpanded(const QModelIndex &)));
    connect(m_wdgLayerBox->listLayers,
            SIGNAL(selectionChanged(const QModelIndexList&)), SLOT(selectionChanged(const QModelIndexList&)));

    m_viewModeMenu = new QMenu(this);
    QActionGroup *group = new QActionGroup(this);
    QList<QAction*> actions;

    actions << m_viewModeMenu->addAction(KisIconUtils::loadIcon("view-list-text"),
                                         i18n("Minimal View"), this, SLOT(slotMinimalView()));
    actions << m_viewModeMenu->addAction(KisIconUtils::loadIcon("view-list-details"),
                                         i18n("Detailed View"), this, SLOT(slotDetailedView()));
    actions << m_viewModeMenu->addAction(KisIconUtils::loadIcon("view-preview"),
                                         i18n("Thumbnail View"), this, SLOT(slotThumbnailView()));

    for (int i = 0, n = actions.count(); i < n; ++i) {
        actions[i]->setCheckable(true);
        actions[i]->setActionGroup(group);
    }

    m_wdgLayerBox->bnAdd->setIcon(KisIconUtils::loadIcon("addlayer"));

    m_wdgLayerBox->bnViewMode->setMenu(m_viewModeMenu);
    m_wdgLayerBox->bnViewMode->setPopupMode(QToolButton::InstantPopup);
    m_wdgLayerBox->bnViewMode->setIcon(KisIconUtils::loadIcon("view-choose"));
    m_wdgLayerBox->bnViewMode->setText(i18n("View mode"));

    m_wdgLayerBox->bnDelete->setIcon(KisIconUtils::loadIcon("deletelayer"));
    m_wdgLayerBox->bnDelete->setIconSize(QSize(22, 22));

    m_wdgLayerBox->bnRaise->setEnabled(false);
    m_wdgLayerBox->bnRaise->setIcon(KisIconUtils::loadIcon("arrowupblr"));
    m_wdgLayerBox->bnRaise->setIconSize(QSize(22, 22));

    m_wdgLayerBox->bnLower->setEnabled(false);
    m_wdgLayerBox->bnLower->setIcon(KisIconUtils::loadIcon("arrowdown"));
    m_wdgLayerBox->bnLower->setIconSize(QSize(22, 22));

    m_wdgLayerBox->bnProperties->setIcon(KisIconUtils::loadIcon("properties"));
    m_wdgLayerBox->bnProperties->setIconSize(QSize(22, 22));

    m_wdgLayerBox->bnDuplicate->setIcon(KisIconUtils::loadIcon("duplicatelayer"));
    m_wdgLayerBox->bnDuplicate->setIconSize(QSize(22, 22));

    m_removeAction  = new ButtonAction(m_wdgLayerBox->bnDelete,
                                       KisIconUtils::loadIcon("deletelayer"), i18n("&Remove Layer"), this);
    m_removeAction->setActivationFlags(KisAction::ACTIVE_NODE);
    m_removeAction->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    m_removeAction->setObjectName("remove_layer");
    m_removeAction->setShortcut(QKeySequence(Qt::SHIFT + Qt::Key_Delete));
    connect(m_removeAction, SIGNAL(triggered()), this, SLOT(slotRmClicked()));
    m_actions.append(m_removeAction);

    KisAction *action  = new ButtonAction(m_wdgLayerBox->bnRaise, this);
    action->setText(i18n("Move Layer or Mask Up"));
    action->setActivationFlags(KisAction::ACTIVE_NODE);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    action->setObjectName("move_layer_up");
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageUp));
    connect(action, SIGNAL(triggered()), this, SLOT(slotRaiseClicked()));
    m_actions.append(action);

    action  = new ButtonAction(m_wdgLayerBox->bnLower, this);
    action->setText(i18n("Move Layer or Mask Down"));
    action->setActivationFlags(KisAction::ACTIVE_NODE);
    action->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    action->setObjectName("move_layer_down");
    action->setShortcut(QKeySequence(Qt::CTRL + Qt::Key_PageDown));
    connect(action, SIGNAL(triggered()), this, SLOT(slotLowerClicked()));
    m_actions.append(action);

    m_propertiesAction  = new ButtonAction(m_wdgLayerBox->bnProperties,
                                           KisIconUtils::loadIcon("properties"), i18n("&Properties..."),this);
    m_propertiesAction->setActivationFlags(KisAction::ACTIVE_NODE);
    m_propertiesAction->setActivationConditions(KisAction::ACTIVE_NODE_EDITABLE);
    m_propertiesAction->setObjectName("layer_properties");
    connect(m_propertiesAction, SIGNAL(triggered()), this, SLOT(slotPropertiesClicked()));
    m_actions.append(m_propertiesAction);

    if (cfg.sliderLabels()) {
        m_wdgLayerBox->opacityLabel->hide();
        m_wdgLayerBox->doubleOpacity->setPrefix(QString("%1:  ").arg(i18n("Opacity")));
    }
    m_wdgLayerBox->doubleOpacity->setRange(0, 100, 0);
    m_wdgLayerBox->doubleOpacity->setSuffix("%");

    connect(m_wdgLayerBox->doubleOpacity, SIGNAL(valueChanged(qreal)), SLOT(slotOpacitySliderMoved(qreal)));
    connect(&m_opacityDelayTimer, SIGNAL(timeout()), SLOT(slotOpacityChanged()));

    connect(m_wdgLayerBox->cmbComposite, SIGNAL(activated(int)), SLOT(slotCompositeOpChanged(int)));

    m_selectOpaque = new KisAction(i18n("&Select Opaque"), this);
    m_selectOpaque->setActivationFlags(KisAction::ACTIVE_LAYER);
    m_selectOpaque->setActivationConditions(KisAction::SELECTION_EDITABLE);
    m_selectOpaque->setObjectName("select_opaque");
    connect(m_selectOpaque, SIGNAL(triggered(bool)), this, SLOT(slotSelectOpaque()));
    m_actions.append(m_selectOpaque);

    m_newLayerMenu = new QMenu(this);
    m_wdgLayerBox->bnAdd->setMenu(m_newLayerMenu);
    m_wdgLayerBox->bnAdd->setPopupMode(QToolButton::MenuButtonPopup);

    m_nodeModel = new KisNodeModel(this);

    /**
     * Connect model updateUI() to enable/disable controls.
     * Note: nodeActivated() is connected separately in setImage(), because
     *       it needs particular order of calls: first the connection to the
     *       node manager should be called, then updateUI()
     */
    connect(m_nodeModel, SIGNAL(rowsInserted(const QModelIndex&, int, int)), SLOT(updateUI()));
    connect(m_nodeModel, SIGNAL(rowsRemoved(const QModelIndex&, int, int)), SLOT(updateUI()));
    connect(m_nodeModel, SIGNAL(rowsMoved(const QModelIndex&, int, int, const QModelIndex&, int)), SLOT(updateUI()));
    connect(m_nodeModel, SIGNAL(dataChanged(const QModelIndex&, const QModelIndex&)), SLOT(updateUI()));
    connect(m_nodeModel, SIGNAL(modelReset()), SLOT(slotModelReset()));

    KisAction *showGlobalSelectionMask = new KisAction(i18n("&Show Global Selection Mask"), this);
    showGlobalSelectionMask->setObjectName("show-global-selection-mask");
    showGlobalSelectionMask->setToolTip(i18nc("@info:tooltip", "Shows global selection as a usual selection mask in <interface>Layers</interface> docker"));
    showGlobalSelectionMask->setCheckable(true);
    connect(showGlobalSelectionMask, SIGNAL(triggered(bool)), SLOT(slotEditGlobalSelection(bool)));
    m_actions.append(showGlobalSelectionMask);

    showGlobalSelectionMask->setChecked(cfg.showGlobalSelection());

    m_wdgLayerBox->listLayers->setModel(m_nodeModel);

    setEnabled(false);

    connect(&m_thumbnailCompressor, SIGNAL(timeout()), SLOT(updateThumbnail()));
}

KisLayerBox::~KisLayerBox()
{
    delete m_wdgLayerBox;
}


void expandNodesRecursively(KisNodeSP root, QPointer<KisNodeModel> nodeModel, KisNodeView *nodeView)
{
    if (!root) return;
    if (nodeModel.isNull()) return;
    if (!nodeView) return;

    nodeView->blockSignals(true);

    KisNodeSP node = root->firstChild();
    while (node) {
        QModelIndex idx = nodeModel->indexFromNode(node);
        if (idx.isValid()) {
            if (node->collapsed()) {
                nodeView->collapse(idx);
            }
        }
        if (node->childCount() > 0) {
            expandNodesRecursively(node, nodeModel, nodeView);
        }
        node = node->nextSibling();
    }
    nodeView->blockSignals(false);
}

void KisLayerBox::setMainWindow(KisViewManager* kisview)
{
    m_nodeManager = kisview->nodeManager();

    Q_FOREACH (KisAction *action, m_actions) {
        kisview->actionManager()->
            addAction(action->objectName(),
                      action);
    }

    connectActionToButton(kisview, m_wdgLayerBox->bnAdd, "add_new_paint_layer");
    connectActionToButton(kisview, m_wdgLayerBox->bnDuplicate, "duplicatelayer");
}

void KisLayerBox::setCanvas(KoCanvasBase *canvas)
{
    if(m_canvas == canvas)
        return;

    setEnabled(canvas != 0);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_nodeModel->setDummiesFacade(0, 0, 0, 0);

        disconnect(m_image, 0, this, 0);
        disconnect(m_nodeManager, 0, this, 0);
        disconnect(m_nodeModel, 0, m_nodeManager, 0);
        disconnect(m_nodeModel, SIGNAL(nodeActivated(KisNodeSP)), this, SLOT(updateUI()));
        m_nodeManager->slotSetSelectedNodes(KisNodeList());
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

    if (m_canvas) {
        m_image = m_canvas->image();

        connect(m_image, SIGNAL(sigImageUpdated(QRect)), &m_thumbnailCompressor, SLOT(start()));

        KisDocument* doc = static_cast<KisDocument*>(m_canvas->imageView()->document());
        KisShapeController *kritaShapeController =
            dynamic_cast<KisShapeController*>(doc->shapeController());
        KisDummiesFacadeBase *kritaDummiesFacade =
            static_cast<KisDummiesFacadeBase*>(kritaShapeController);
        m_nodeModel->setDummiesFacade(kritaDummiesFacade, m_image, kritaShapeController, m_nodeManager->nodeSelectionAdapter());

        connect(m_image, SIGNAL(sigAboutToBeDeleted()), SLOT(notifyImageDeleted()));
        connect(m_image, SIGNAL(sigNodeCollapsedChanged()), SLOT(slotNodeCollapsedChanged()));

        // cold start
        if (m_nodeManager) {
            setCurrentNode(m_nodeManager->activeNode());
        }
        else {
            setCurrentNode(m_canvas->imageView()->currentNode());
        }

        // Connection KisNodeManager -> KisLayerBox
        connect(m_nodeManager, SIGNAL(sigUiNeedChangeActiveNode(KisNodeSP)),
                this, SLOT(setCurrentNode(KisNodeSP)));

        connect(m_nodeManager,
                SIGNAL(sigUiNeedChangeSelectedNodes(const QList<KisNodeSP> &)),
                SLOT(slotNodeManagerChangedSelection(const QList<KisNodeSP> &)));

        // Connection KisLayerBox -> KisNodeManager (isolate layer)
        connect(m_nodeModel, SIGNAL(toggleIsolateActiveNode()),
                m_nodeManager, SLOT(toggleIsolateActiveNode()));

        // Node manipulation methods are forwarded to the node manager
        connect(m_nodeModel, SIGNAL(requestAddNode(KisNodeSP, KisNodeSP, KisNodeSP)),
                m_nodeManager, SLOT(addNodeDirect(KisNodeSP, KisNodeSP, KisNodeSP)));
        connect(m_nodeModel, SIGNAL(requestMoveNode(KisNodeSP, KisNodeSP, KisNodeSP)),
                m_nodeManager, SLOT(moveNodeDirect(KisNodeSP, KisNodeSP, KisNodeSP)));

        m_wdgLayerBox->listLayers->expandAll();
        expandNodesRecursively(m_image->rootLayer(), m_nodeModel, m_wdgLayerBox->listLayers);
        m_wdgLayerBox->listLayers->scrollTo(m_wdgLayerBox->listLayers->currentIndex());

        addActionToMenu(m_newLayerMenu, "add_new_paint_layer");
        addActionToMenu(m_newLayerMenu, "add_new_group_layer");
        addActionToMenu(m_newLayerMenu, "add_new_clone_layer");
        addActionToMenu(m_newLayerMenu, "add_new_shape_layer");
        addActionToMenu(m_newLayerMenu, "add_new_adjustment_layer");
        addActionToMenu(m_newLayerMenu, "add_new_fill_layer");
        addActionToMenu(m_newLayerMenu, "add_new_file_layer");
        m_newLayerMenu->addSeparator();
        addActionToMenu(m_newLayerMenu, "add_new_transparency_mask");
        addActionToMenu(m_newLayerMenu, "add_new_filter_mask");
        addActionToMenu(m_newLayerMenu, "add_new_transform_mask");
        addActionToMenu(m_newLayerMenu, "add_new_selection_mask");
    }

}


void KisLayerBox::unsetCanvas()
{
    setEnabled(false);
    if (m_canvas) {
        m_newLayerMenu->clear();
    }

    m_nodeModel->setDummiesFacade(0, 0, 0, 0);
    disconnect(m_image, 0, this, 0);
    disconnect(m_nodeManager, 0, this, 0);
    disconnect(m_nodeModel, 0, m_nodeManager, 0);
    disconnect(m_nodeModel, SIGNAL(nodeActivated(KisNodeSP)), this, SLOT(updateUI()));
    m_nodeManager->slotSetSelectedNodes(KisNodeList());

    m_canvas = 0;
}

void KisLayerBox::notifyImageDeleted()
{
    setCanvas(0);
}

void KisLayerBox::updateUI()
{
    if (!m_canvas) return;
    if (!m_nodeManager) return;

    KisNodeSP activeNode = m_nodeManager->activeNode();

    m_wdgLayerBox->bnRaise->setEnabled(activeNode && activeNode->isEditable(false) && (activeNode->nextSibling()
                                       || (activeNode->parent() && activeNode->parent() != m_image->root())));
    m_wdgLayerBox->bnLower->setEnabled(activeNode && activeNode->isEditable(false) && (activeNode->prevSibling()
                                       || (activeNode->parent() && activeNode->parent() != m_image->root())));

    m_wdgLayerBox->doubleOpacity->setEnabled(activeNode && activeNode->isEditable(false));

    m_wdgLayerBox->cmbComposite->setEnabled(activeNode && activeNode->isEditable(false));

    if (activeNode) {
        if (m_nodeManager->activePaintDevice()) {
            slotFillCompositeOps(m_nodeManager->activeColorSpace());
        } else {
            slotFillCompositeOps(m_image->colorSpace());
        }

        if (activeNode->inherits("KisMask")) {
            m_wdgLayerBox->cmbComposite->setEnabled(false);
            m_wdgLayerBox->doubleOpacity->setEnabled(false);
        } else if (activeNode->inherits("KisLayer")) {
            m_wdgLayerBox->doubleOpacity->setEnabled(true);

            KisLayerSP l = qobject_cast<KisLayer*>(activeNode.data());
            slotSetOpacity(l->opacity() * 100.0 / 255);

            const KoCompositeOp* compositeOp = l->compositeOp();
            if (compositeOp) {
                slotSetCompositeOp(compositeOp);
            } else {
                m_wdgLayerBox->cmbComposite->setEnabled(false);
            }

            const KisGroupLayer *group = qobject_cast<const KisGroupLayer*>(activeNode.data());
            bool compositeSelectionActive = !(group && group->passThroughMode());

            m_wdgLayerBox->cmbComposite->setEnabled(compositeSelectionActive);
        }
    }
}


/**
 * This method is callen *only* when non-GUI code requested the
 * change of the current node
 */
void KisLayerBox::setCurrentNode(KisNodeSP node)
{
    QModelIndex index = node ? m_nodeModel->indexFromNode(node) : QModelIndex();

    m_nodeModel->setData(index, true, KisNodeModel::ActiveRole);
    updateUI();
}

void KisLayerBox::slotModelReset()
{
    if(m_nodeModel->hasDummiesFacade()) {
        QItemSelection selection;
        Q_FOREACH (const KisNodeSP node, m_nodeManager->selectedNodes()) {
            const QModelIndex &idx = m_nodeModel->indexFromNode(node);
            if(idx.isValid()){
                QItemSelectionRange selectionRange(idx);
                selection << selectionRange;
            }
        }

        m_wdgLayerBox->listLayers->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
    }

    updateUI();
}

void KisLayerBox::slotSetCompositeOp(const KoCompositeOp* compositeOp)
{
    KoID opId = KoCompositeOpRegistry::instance().getKoID(compositeOp->id());

    m_wdgLayerBox->cmbComposite->blockSignals(true);
    m_wdgLayerBox->cmbComposite->selectCompositeOp(opId);
    m_wdgLayerBox->cmbComposite->blockSignals(false);
}

void KisLayerBox::slotFillCompositeOps(const KoColorSpace* colorSpace)
{
    m_wdgLayerBox->cmbComposite->validate(colorSpace);
}

// range: 0-100
void KisLayerBox::slotSetOpacity(double opacity)
{
    Q_ASSERT(opacity >= 0 && opacity <= 100);
    m_wdgLayerBox->doubleOpacity->blockSignals(true);
    m_wdgLayerBox->doubleOpacity->setValue(opacity);
    m_wdgLayerBox->doubleOpacity->blockSignals(false);
}

void KisLayerBox::slotContextMenuRequested(const QPoint &pos, const QModelIndex &index)
{
    if (m_canvas) {
        QMenu menu;

        if (index.isValid()) {
            menu.addAction(m_propertiesAction);
            addActionToMenu(&menu, "layer_style");

            menu.addSeparator();
            addActionToMenu(&menu, "show_in_timeline");
            menu.addSeparator();

            menu.addAction(m_removeAction);
            addActionToMenu(&menu, "duplicatelayer");
            addActionToMenu(&menu, "flatten_image");
            addActionToMenu(&menu, "flatten_layer");

            // TODO: missing icon "edit-merge"
            QAction* mergeLayerDown = menu.addAction(i18n("&Merge with Layer Below"),
                                                     this, SLOT(slotMergeLayer()));
            if (!index.sibling(index.row() + 1, 0).isValid()) mergeLayerDown->setEnabled(false);
            menu.addSeparator();

            QMenu *convertToMenu = menu.addMenu(i18n("&Convert"));
            addActionToMenu(convertToMenu, "convert_to_paint_layer");
            addActionToMenu(convertToMenu, "convert_to_transparency_mask");
            addActionToMenu(convertToMenu, "convert_to_filter_mask");
            addActionToMenu(convertToMenu, "convert_to_selection_mask");

            QMenu *splitAlphaMenu = menu.addMenu(i18n("S&plit Alpha"));
            addActionToMenu(splitAlphaMenu, "split_alpha_into_mask");
            addActionToMenu(splitAlphaMenu, "split_alpha_write");
            addActionToMenu(splitAlphaMenu, "split_alpha_save_merged");

            KisNodeSP node = m_nodeModel->nodeFromIndex(index);
            if (node && !node->inherits("KisTransformMask")) {
                addActionToMenu(&menu, "isolate_layer");
            }
        }
        menu.addSeparator();
        addActionToMenu(&menu, "add_new_transparency_mask");
        addActionToMenu(&menu, "add_new_filter_mask");
        addActionToMenu(&menu, "add_new_transform_mask");
        addActionToMenu(&menu, "add_new_selection_mask");
        menu.addSeparator();
        menu.addAction(m_selectOpaque);
        menu.exec(pos);
    }
}

void KisLayerBox::slotMergeLayer()
{
    if (!m_canvas) return;
    m_nodeManager->mergeLayer();
}

void KisLayerBox::slotMinimalView()
{
    m_wdgLayerBox->listLayers->setDisplayMode(KisNodeView::MinimalMode);
}

void KisLayerBox::slotDetailedView()
{
    m_wdgLayerBox->listLayers->setDisplayMode(KisNodeView::DetailedMode);
}

void KisLayerBox::slotThumbnailView()
{
    m_wdgLayerBox->listLayers->setDisplayMode(KisNodeView::ThumbnailMode);
}

void KisLayerBox::slotRmClicked()
{
    if (!m_canvas) return;
    m_nodeManager->removeNode();
}

void KisLayerBox::slotRaiseClicked()
{
    if (!m_canvas) return;
    m_nodeManager->raiseNode();
}

void KisLayerBox::slotLowerClicked()
{
    if (!m_canvas) return;
    m_nodeManager->lowerNode();
}

void KisLayerBox::slotPropertiesClicked()
{
    if (!m_canvas) return;
    if (KisNodeSP active = m_nodeManager->activeNode()) {
        m_nodeManager->nodeProperties(active);
    }
}

void KisLayerBox::slotCompositeOpChanged(int index)
{
    Q_UNUSED(index);
    if (!m_canvas) return;

    QString compositeOp = m_wdgLayerBox->cmbComposite->selectedCompositeOp().id();
    m_nodeManager->nodeCompositeOpChanged(m_nodeManager->activeColorSpace()->compositeOp(compositeOp));
}

void KisLayerBox::slotOpacityChanged()
{
    if (!m_canvas) return;
    m_nodeManager->nodeOpacityChanged(m_newOpacity, true);
}

void KisLayerBox::slotOpacitySliderMoved(qreal opacity)
{
    m_newOpacity = opacity;
    m_opacityDelayTimer.start(200);
}

void KisLayerBox::slotCollapsed(const QModelIndex &index)
{
    KisNodeSP node = m_nodeModel->nodeFromIndex(index);
    if (node) {
        node->setCollapsed(true);
    }
}

void KisLayerBox::slotExpanded(const QModelIndex &index)
{
    KisNodeSP node = m_nodeModel->nodeFromIndex(index);
    if (node) {
        node->setCollapsed(false);
    }
}

void KisLayerBox::slotSelectOpaque()
{
    if (!m_canvas) return;
    QAction *action = m_canvas->viewManager()->actionManager()->actionByName("selectopaque");
    if (action) {
        action->trigger();
    }
}

void KisLayerBox::slotNodeCollapsedChanged()
{
    m_wdgLayerBox->listLayers->expandAll();
    expandNodesRecursively(m_image->rootLayer(), m_nodeModel, m_wdgLayerBox->listLayers);
}

inline bool isSelectionMask(KisNodeSP node)
{
    return dynamic_cast<KisSelectionMask*>(node.data());
}

KisNodeSP KisLayerBox::findNonHidableNode(KisNodeSP startNode)
{
    if (isSelectionMask(startNode) &&
        startNode->parent() &&
        !startNode->parent()->parent()) {


        KisNodeSP node = startNode->prevSibling();
        while (node && isSelectionMask(node)) {
            node = node->prevSibling();
        }

        if (!node) {
            node = startNode->nextSibling();
            while (node && isSelectionMask(node)) {
                node = node->nextSibling();
            }
        }

        if (!node) {
            node = m_image->root()->lastChild();
            while (node && isSelectionMask(node)) {
                node = node->prevSibling();
            }
        }

        KIS_ASSERT_RECOVER_NOOP(node && "cannot activate any node!");
        startNode = node;
    }

    return startNode;
}

void KisLayerBox::slotEditGlobalSelection(bool showSelections)
{
    KisNodeSP lastActiveNode = m_nodeManager->activeNode();
    KisNodeSP activateNode = lastActiveNode;

    if (!showSelections) {
        activateNode = findNonHidableNode(activateNode);
    }

    m_nodeModel->setShowGlobalSelection(showSelections);

    if (showSelections) {
        KisNodeSP newMask = m_image->rootLayer()->selectionMask();
        if (newMask) {
            activateNode = newMask;
        }
    }

    if (activateNode) {
        if (lastActiveNode != activateNode) {
            m_nodeManager->slotNonUiActivatedNode(activateNode);
        } else {
            setCurrentNode(lastActiveNode);
        }
    }
}

void KisLayerBox::selectionChanged(const QModelIndexList selection)
{
    if (!m_nodeManager) return;

    /**
     * When the user clears the extended selection by clicking on the
     * empty area of the docker, the selection should be reset on to
     * the active layer, which might be even unselected(!).
     */
    if (selection.isEmpty() && m_nodeManager->activeNode()) {
        QModelIndex selectedIndex =
            m_nodeModel->indexFromNode(m_nodeManager->activeNode());

        m_wdgLayerBox->listLayers->selectionModel()->
            setCurrentIndex(selectedIndex, QItemSelectionModel::ClearAndSelect);
        return;
    }

    QList<KisNodeSP> selectedNodes;
    Q_FOREACH (const QModelIndex &idx, selection) {
        selectedNodes << m_nodeModel->nodeFromIndex(idx);
    }

    m_nodeManager->slotSetSelectedNodes(selectedNodes);
    updateUI();
}

void KisLayerBox::slotNodeManagerChangedSelection(const KisNodeList &nodes)
{
    if (!m_nodeManager) return;

    QModelIndexList newSelection;
    Q_FOREACH(KisNodeSP node, nodes) {
        newSelection << m_nodeModel->indexFromNode(node);
    }

    QItemSelectionModel *model = m_wdgLayerBox->listLayers->selectionModel();

    if (KritaUtils::compareListsUnordered(newSelection, model->selectedIndexes())) {
        return;
    }

    QItemSelection selection;
    Q_FOREACH(const QModelIndex &idx, newSelection) {
        selection.select(idx, idx);
    }

    model->select(selection, QItemSelectionModel::ClearAndSelect);
}

void KisLayerBox::updateThumbnail()
{
    m_wdgLayerBox->listLayers->updateNode(m_wdgLayerBox->listLayers->currentIndex());
}

#include "moc_kis_layer_box.cpp"

