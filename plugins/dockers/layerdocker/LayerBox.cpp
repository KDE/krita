/*
 *  LayerBox.cc - part of Krita aka Krayon aka KimageShop
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

#include "LayerBox.h"

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
#include <QWidgetAction>
#include <QProxyStyle>

#include <kis_debug.h>
#include <klocalizedstring.h>

#include <kis_icon.h>
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
#include <kis_base_node.h>
#include <kis_composite_ops_model.h>
#include <kis_keyframe_channel.h>
#include <kis_image_animation_interface.h>
#include <KoProperties.h>

#include <kis_action.h>
#include "kis_action_manager.h"
#include "widgets/kis_cmb_composite.h"
#include "widgets/kis_slider_spin_box.h"
#include "KisViewManager.h"
#include "kis_node_manager.h"
#include "kis_node_model.h"

#include "canvas/kis_canvas2.h"
#include "kis_dummies_facade_base.h"
#include "kis_shape_controller.h"
#include "kis_selection_mask.h"
#include "kis_config.h"
#include "KisView.h"
#include "krita_utils.h"
#include "kis_color_label_selector_widget.h"
#include "kis_signals_blocker.h"
#include "kis_color_filter_combo.h"
#include "kis_node_filter_proxy_model.h"

#include "kis_selection.h"
#include "kis_processing_applicator.h"
#include "commands/kis_set_global_selection_command.h"
#include "KisSelectionActionsAdapter.h"

#include "kis_layer_utils.h"

#include "ui_WdgLayerBox.h"
#include "NodeView.h"
#include "SyncButtonAndAction.h"


class LayerBoxStyle : public QProxyStyle
{
public:
    LayerBoxStyle(QStyle *baseStyle = 0) : QProxyStyle(baseStyle) {}

    void drawPrimitive(PrimitiveElement element, const QStyleOption *option,
                       QPainter *painter, const QWidget *widget) const
    {
        if (element == QStyle::PE_IndicatorItemViewItemDrop)
        {
            QColor color(widget->palette().color(QPalette::Highlight).lighter());

            if (option->rect.height() == 0) {
                QBrush brush(color);

                QRect r(option->rect);
                r.setTop(r.top() - 2);
                r.setBottom(r.bottom() + 2);

                painter->fillRect(r, brush);
            } else {
                color.setAlpha(200);
                QBrush brush(color);
                painter->fillRect(option->rect, brush);
            }
        }
        else
        {
            QProxyStyle::drawPrimitive(element, option, painter, widget);
        }
    }
};

inline void LayerBox::connectActionToButton(KisViewManager* viewManager, QAbstractButton *button, const QString &id)
{
    if (!viewManager || !button) return;

    KisAction *action = viewManager->actionManager()->actionByName(id);

    if (!action) return;

    connect(button, SIGNAL(clicked()), action, SLOT(trigger()));
    connect(action, SIGNAL(sigEnableSlaves(bool)), button, SLOT(setEnabled(bool)));
    connect(viewManager->mainWindow(), SIGNAL(themeChanged()), this, SLOT(slotUpdateIcons()));
}

inline void LayerBox::addActionToMenu(QMenu *menu, const QString &id)
{
    if (m_canvas) {
        menu->addAction(m_canvas->viewManager()->actionManager()->actionByName(id));
    }
}

LayerBox::LayerBox()
    : QDockWidget(i18n("Layers"))
    , m_canvas(0)
    , m_wdgLayerBox(new Ui_WdgLayerBox)
    , m_thumbnailCompressor(500, KisSignalCompressor::FIRST_INACTIVE)
    , m_colorLabelCompressor(900, KisSignalCompressor::FIRST_INACTIVE)
    , m_thumbnailSizeCompressor(100, KisSignalCompressor::FIRST_INACTIVE)
{
    KisConfig cfg(false);

    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);
    m_opacityDelayTimer.setSingleShot(true);

    m_wdgLayerBox->setupUi(mainWidget);

    m_wdgLayerBox->listLayers->setStyle(new LayerBoxStyle(m_wdgLayerBox->listLayers->style()));

    connect(m_wdgLayerBox->listLayers,
            SIGNAL(contextMenuRequested(QPoint,QModelIndex)),
            this, SLOT(slotContextMenuRequested(QPoint,QModelIndex)));
    connect(m_wdgLayerBox->listLayers,
            SIGNAL(collapsed(QModelIndex)), SLOT(slotCollapsed(QModelIndex)));
    connect(m_wdgLayerBox->listLayers,
            SIGNAL(expanded(QModelIndex)), SLOT(slotExpanded(QModelIndex)));
    connect(m_wdgLayerBox->listLayers,
            SIGNAL(selectionChanged(QModelIndexList)), SLOT(selectionChanged(QModelIndexList)));

    slotUpdateIcons();

    m_wdgLayerBox->bnDelete->setIconSize(QSize(22, 22));
    m_wdgLayerBox->bnRaise->setIconSize(QSize(22, 22));
    m_wdgLayerBox->bnLower->setIconSize(QSize(22, 22));
    m_wdgLayerBox->bnProperties->setIconSize(QSize(22, 22));
    m_wdgLayerBox->bnDuplicate->setIconSize(QSize(22, 22));

    m_wdgLayerBox->bnLower->setEnabled(false);
    m_wdgLayerBox->bnRaise->setEnabled(false);

    if (cfg.sliderLabels()) {
        m_wdgLayerBox->opacityLabel->hide();
        m_wdgLayerBox->doubleOpacity->setPrefix(QString("%1:  ").arg(i18n("Opacity")));
    }
    m_wdgLayerBox->doubleOpacity->setRange(0, 100, 0);
    m_wdgLayerBox->doubleOpacity->setSuffix("%");

    connect(m_wdgLayerBox->doubleOpacity, SIGNAL(valueChanged(qreal)), SLOT(slotOpacitySliderMoved(qreal)));
    connect(&m_opacityDelayTimer, SIGNAL(timeout()), SLOT(slotOpacityChanged()));

    connect(m_wdgLayerBox->cmbComposite, SIGNAL(activated(int)), SLOT(slotCompositeOpChanged(int)));

    m_newLayerMenu = new QMenu(this);
    m_wdgLayerBox->bnAdd->setMenu(m_newLayerMenu);
    m_wdgLayerBox->bnAdd->setPopupMode(QToolButton::MenuButtonPopup);

    m_nodeModel = new KisNodeModel(this);
    m_filteringModel = new KisNodeFilterProxyModel(this);
    m_filteringModel->setNodeModel(m_nodeModel);

    /**
     * Connect model updateUI() to enable/disable controls.
     * Note: nodeActivated() is connected separately in setImage(), because
     *       it needs particular order of calls: first the connection to the
     *       node manager should be called, then updateUI()
     */
    connect(m_nodeModel, SIGNAL(rowsInserted(QModelIndex,int,int)), SLOT(updateUI()));
    connect(m_nodeModel, SIGNAL(rowsRemoved(QModelIndex,int,int)), SLOT(updateUI()));
    connect(m_nodeModel, SIGNAL(rowsMoved(QModelIndex,int,int,QModelIndex,int)), SLOT(updateUI()));
    connect(m_nodeModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)), SLOT(updateUI()));
    connect(m_nodeModel, SIGNAL(modelReset()), SLOT(slotModelReset()));

    KisAction *showGlobalSelectionMask = new KisAction(i18n("&Show Global Selection Mask"), this);
    showGlobalSelectionMask->setObjectName("show-global-selection-mask");
    showGlobalSelectionMask->setActivationFlags(KisAction::ACTIVE_IMAGE);
    showGlobalSelectionMask->setToolTip(i18nc("@info:tooltip", "Shows global selection as a usual selection mask in <b>Layers</b> docker"));
    showGlobalSelectionMask->setCheckable(true);
    connect(showGlobalSelectionMask, SIGNAL(triggered(bool)), SLOT(slotEditGlobalSelection(bool)));
    m_actions.append(showGlobalSelectionMask);

    showGlobalSelectionMask->setChecked(cfg.showGlobalSelection());

    m_colorSelector = new KisColorLabelSelectorWidget(this);
    connect(m_colorSelector, SIGNAL(currentIndexChanged(int)), SLOT(slotColorLabelChanged(int)));
    m_colorSelectorAction = new QWidgetAction(this);
    m_colorSelectorAction->setDefaultWidget(m_colorSelector);

    connect(m_nodeModel, SIGNAL(dataChanged(QModelIndex,QModelIndex)),
            &m_colorLabelCompressor, SLOT(start()));

    m_wdgLayerBox->listLayers->setModel(m_filteringModel);
    // this connection should be done *after* the setModel() call to
    // happen later than the internal selection model
    connect(m_filteringModel.data(), &KisNodeFilterProxyModel::rowsAboutToBeRemoved,
            this, &LayerBox::slotAboutToRemoveRows);

    connect(m_wdgLayerBox->cmbFilter, SIGNAL(selectedColorsChanged()), SLOT(updateLayerFiltering()));

    setEnabled(false);

    connect(&m_thumbnailCompressor, SIGNAL(timeout()), SLOT(updateThumbnail()));
    connect(&m_colorLabelCompressor, SIGNAL(timeout()), SLOT(updateAvailableLabels()));



    // set up the configure menu for changing thumbnail size
    QMenu* configureMenu = new QMenu(this);
    configureMenu->setStyleSheet("margin: 6px");
    configureMenu->addSection(i18n("Thumbnail Size"));

    m_wdgLayerBox->configureLayerDockerToolbar->setMenu(configureMenu);
    m_wdgLayerBox->configureLayerDockerToolbar->setIcon(KisIconUtils::loadIcon("configure"));

    m_wdgLayerBox->configureLayerDockerToolbar->setPopupMode(QToolButton::InstantPopup);


    // add horizontal slider
    thumbnailSizeSlider = new QSlider(this);
    thumbnailSizeSlider->setOrientation(Qt::Horizontal);
    thumbnailSizeSlider->setRange(20, 80);

    thumbnailSizeSlider->setValue(cfg.layerThumbnailSize(false)); // grab this from the kritarc

    thumbnailSizeSlider->setMinimumHeight(20);
    thumbnailSizeSlider->setMinimumWidth(40);
    thumbnailSizeSlider->setTickInterval(5);


    QWidgetAction *sliderAction= new QWidgetAction(this);
    sliderAction->setDefaultWidget(thumbnailSizeSlider);
    configureMenu->addAction(sliderAction);


    connect(thumbnailSizeSlider, SIGNAL(sliderMoved(int)), &m_thumbnailSizeCompressor, SLOT(start()));
    connect(&m_thumbnailSizeCompressor, SIGNAL(timeout()), SLOT(slotUpdateThumbnailIconSize()));
}

LayerBox::~LayerBox()
{
    delete m_wdgLayerBox;
}


void expandNodesRecursively(KisNodeSP root, QPointer<KisNodeFilterProxyModel> filteringModel, NodeView *nodeView)
{
    if (!root) return;
    if (filteringModel.isNull()) return;
    if (!nodeView) return;

    nodeView->blockSignals(true);

    KisNodeSP node = root->firstChild();
    while (node) {
        QModelIndex idx = filteringModel->indexFromNode(node);
        if (idx.isValid()) {
            nodeView->setExpanded(idx, !node->collapsed());
        }
        if (!node->collapsed() && node->childCount() > 0) {
            expandNodesRecursively(node, filteringModel, nodeView);
        }
        node = node->nextSibling();
    }
    nodeView->blockSignals(false);
}

void LayerBox::slotAddLayerBnClicked()
{
    if (m_canvas) {
        KisNodeList nodes = m_nodeManager->selectedNodes();

        if (nodes.size() == 1) {
            KisAction *action = m_canvas->viewManager()->actionManager()->actionByName("add_new_paint_layer");
            action->trigger();
        } else {
            KisAction *action = m_canvas->viewManager()->actionManager()->actionByName("create_quick_group");
            action->trigger();
        }
    }
}

void LayerBox::setViewManager(KisViewManager* kisview)
{
    m_nodeManager = kisview->nodeManager();

    Q_FOREACH (KisAction *action, m_actions) {
        kisview->actionManager()->
                addAction(action->objectName(),
                          action);
    }

    connect(m_wdgLayerBox->bnAdd, SIGNAL(clicked()), this, SLOT(slotAddLayerBnClicked()));
    connectActionToButton(kisview, m_wdgLayerBox->bnDuplicate, "duplicatelayer");

    KisActionManager *actionManager = kisview->actionManager();

    KisAction *action = actionManager->createAction("RenameCurrentLayer");
    Q_ASSERT(action);
    connect(action, SIGNAL(triggered()), this, SLOT(slotRenameCurrentNode()));

    m_propertiesAction = actionManager->createAction("layer_properties");
    Q_ASSERT(m_propertiesAction);
    new SyncButtonAndAction(m_propertiesAction, m_wdgLayerBox->bnProperties, this);
    connect(m_propertiesAction, SIGNAL(triggered()), this, SLOT(slotPropertiesClicked()));

    m_removeAction = actionManager->createAction("remove_layer");
    Q_ASSERT(m_removeAction);
    new SyncButtonAndAction(m_removeAction, m_wdgLayerBox->bnDelete, this);
    connect(m_removeAction, SIGNAL(triggered()), this, SLOT(slotRmClicked()));

    action = actionManager->createAction("move_layer_up");
    Q_ASSERT(action);
    new SyncButtonAndAction(action, m_wdgLayerBox->bnRaise, this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotRaiseClicked()));

    action = actionManager->createAction("move_layer_down");
    Q_ASSERT(action);
    new SyncButtonAndAction(action, m_wdgLayerBox->bnLower, this);
    connect(action, SIGNAL(triggered()), this, SLOT(slotLowerClicked()));
}

void LayerBox::setCanvas(KoCanvasBase *canvas)
{
    if (m_canvas == canvas)
        return;

    setEnabled(canvas != 0);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_nodeModel->setDummiesFacade(0, 0, 0, 0, 0);
        m_selectionActionsAdapter.reset();

        if (m_image) {
            KisImageAnimationInterface *animation = m_image->animationInterface();
            animation->disconnect(this);
        }

        disconnect(m_image, 0, this, 0);
        disconnect(m_nodeManager, 0, this, 0);
        disconnect(m_nodeModel, 0, m_nodeManager, 0);
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


        m_selectionActionsAdapter.reset(new KisSelectionActionsAdapter(m_canvas->viewManager()->selectionManager()));
        m_nodeModel->setDummiesFacade(kritaDummiesFacade,
                                      m_image,
                                      kritaShapeController,
                                      m_selectionActionsAdapter.data(),
                                      m_nodeManager);

        connect(m_image, SIGNAL(sigAboutToBeDeleted()), SLOT(notifyImageDeleted()));
        connect(m_image, SIGNAL(sigNodeCollapsedChanged()), SLOT(slotNodeCollapsedChanged()));

        // cold start
        if (m_nodeManager) {
            setCurrentNode(m_nodeManager->activeNode());
            // Connection KisNodeManager -> LayerBox
            connect(m_nodeManager, SIGNAL(sigUiNeedChangeActiveNode(KisNodeSP)),
                    this, SLOT(setCurrentNode(KisNodeSP)));

            connect(m_nodeManager,
                    SIGNAL(sigUiNeedChangeSelectedNodes(QList<KisNodeSP>)),
                    SLOT(slotNodeManagerChangedSelection(QList<KisNodeSP>)));
        }
        else {
            setCurrentNode(m_canvas->imageView()->currentNode());
        }

        // Connection LayerBox -> KisNodeManager (isolate layer)
        connect(m_nodeModel, SIGNAL(toggleIsolateActiveNode()),
                m_nodeManager, SLOT(toggleIsolateActiveNode()));

        KisImageAnimationInterface *animation = m_image->animationInterface();
        connect(animation, &KisImageAnimationInterface::sigUiTimeChanged, this, &LayerBox::slotImageTimeChanged);

        expandNodesRecursively(m_image->rootLayer(), m_filteringModel, m_wdgLayerBox->listLayers);
        m_wdgLayerBox->listLayers->scrollTo(m_wdgLayerBox->listLayers->currentIndex());
        updateAvailableLabels();

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
        addActionToMenu(m_newLayerMenu, "add_new_colorize_mask");
        addActionToMenu(m_newLayerMenu, "add_new_transform_mask");
        addActionToMenu(m_newLayerMenu, "add_new_selection_mask");
    }

}


void LayerBox::unsetCanvas()
{
    setEnabled(false);
    if (m_canvas) {
        m_newLayerMenu->clear();
    }

    m_filteringModel->unsetDummiesFacade();
    disconnect(m_image, 0, this, 0);
    disconnect(m_nodeManager, 0, this, 0);
    disconnect(m_nodeModel, 0, m_nodeManager, 0);
    m_nodeManager->slotSetSelectedNodes(KisNodeList());

    m_canvas = 0;
}

void LayerBox::notifyImageDeleted()
{
    setCanvas(0);
}

void LayerBox::updateUI()
{
    if (!m_canvas) return;
    if (!m_nodeManager) return;

    KisNodeSP activeNode = m_nodeManager->activeNode();

    if (activeNode != m_activeNode) {
        if( !m_activeNode.isNull() )
            m_activeNode->disconnect(this);
        m_activeNode = activeNode;

        if (activeNode) {
            KisKeyframeChannel *opacityChannel = activeNode->getKeyframeChannel(KisKeyframeChannel::Opacity.id(), false);
            if (opacityChannel) {
                watchOpacityChannel(opacityChannel);
            } else {
                watchOpacityChannel(0);
                connect(activeNode.data(), &KisNode::keyframeChannelAdded, this, &LayerBox::slotKeyframeChannelAdded);
            }
        }
    }

    m_wdgLayerBox->bnRaise->setEnabled(activeNode && activeNode->isEditable(false) && (activeNode->nextSibling()
                                                                                       || (activeNode->parent() && activeNode->parent() != m_image->root())));
    m_wdgLayerBox->bnLower->setEnabled(activeNode && activeNode->isEditable(false) && (activeNode->prevSibling()
                                                                                       || (activeNode->parent() && activeNode->parent() != m_image->root())));

    m_wdgLayerBox->doubleOpacity->setEnabled(activeNode && activeNode->isEditable(false));

    m_wdgLayerBox->cmbComposite->setEnabled(activeNode && activeNode->isEditable(false));
    m_wdgLayerBox->cmbComposite->validate(m_image->colorSpace());

    if (activeNode) {
        if (activeNode->inherits("KisColorizeMask") ||
                activeNode->inherits("KisLayer")) {

            m_wdgLayerBox->doubleOpacity->setEnabled(true);

            slotSetOpacity(activeNode->opacity() * 100.0 / 255);

            const KoCompositeOp* compositeOp = activeNode->compositeOp();
            if (compositeOp) {
                slotSetCompositeOp(compositeOp);
            } else {
                m_wdgLayerBox->cmbComposite->setEnabled(false);
            }

            const KisGroupLayer *group = qobject_cast<const KisGroupLayer*>(activeNode.data());
            bool compositeSelectionActive = !(group && group->passThroughMode());

            m_wdgLayerBox->cmbComposite->setEnabled(compositeSelectionActive);
        } else if (activeNode->inherits("KisMask")) {
            m_wdgLayerBox->cmbComposite->setEnabled(false);
            m_wdgLayerBox->doubleOpacity->setEnabled(false);
        }
    }
}


/**
 * This method is called *only* when non-GUI code requested the
 * change of the current node
 */
void LayerBox::setCurrentNode(KisNodeSP node)
{
    m_filteringModel->setActiveNode(node);

    QModelIndex index = node ? m_filteringModel->indexFromNode(node) : QModelIndex();
    m_filteringModel->setData(index, true, KisNodeModel::ActiveRole);
    updateUI();
}

void LayerBox::slotModelReset()
{
    if(m_nodeModel->hasDummiesFacade()) {
        QItemSelection selection;
        Q_FOREACH (const KisNodeSP node, m_nodeManager->selectedNodes()) {
            const QModelIndex &idx = m_filteringModel->indexFromNode(node);
            if(idx.isValid()){
                QItemSelectionRange selectionRange(idx);
                selection << selectionRange;
            }
        }

        m_wdgLayerBox->listLayers->selectionModel()->select(selection, QItemSelectionModel::ClearAndSelect);
    }

    updateUI();
}

void LayerBox::slotSetCompositeOp(const KoCompositeOp* compositeOp)
{
    KoID opId = KoCompositeOpRegistry::instance().getKoID(compositeOp->id());

    m_wdgLayerBox->cmbComposite->blockSignals(true);
    m_wdgLayerBox->cmbComposite->selectCompositeOp(opId);
    m_wdgLayerBox->cmbComposite->blockSignals(false);
}

// range: 0-100
void LayerBox::slotSetOpacity(double opacity)
{
    Q_ASSERT(opacity >= 0 && opacity <= 100);
    m_wdgLayerBox->doubleOpacity->blockSignals(true);
    m_wdgLayerBox->doubleOpacity->setValue(opacity);
    m_wdgLayerBox->doubleOpacity->blockSignals(false);
}

void LayerBox::slotContextMenuRequested(const QPoint &pos, const QModelIndex &index)
{
    KisNodeList nodes = m_nodeManager->selectedNodes();
    KisNodeSP activeNode = m_nodeManager->activeNode();
    if (nodes.isEmpty() || !activeNode) return;

    if (m_canvas) {
        QMenu menu;

        const bool singleLayer = nodes.size() == 1;

        if (index.isValid()) {
            menu.addAction(m_propertiesAction);

            if (singleLayer) {
                addActionToMenu(&menu, "layer_style");
            }

            {
                KisSignalsBlocker b(m_colorSelector);
                m_colorSelector->setCurrentIndex(singleLayer ? activeNode->colorLabelIndex() : -1);
            }
            menu.addAction(m_colorSelectorAction);

            menu.addSeparator();

            addActionToMenu(&menu, "cut_layer_clipboard");
            addActionToMenu(&menu, "copy_layer_clipboard");
            addActionToMenu(&menu, "paste_layer_from_clipboard");
            menu.addAction(m_removeAction);
            addActionToMenu(&menu, "duplicatelayer");
            addActionToMenu(&menu, "merge_layer");

            if (singleLayer) {
                addActionToMenu(&menu, "flatten_image");
                addActionToMenu(&menu, "flatten_layer");
            }

            menu.addSeparator();
            QMenu *selectMenu = menu.addMenu(i18n("&Select"));
            addActionToMenu(selectMenu, "select_all_layers");
            addActionToMenu(selectMenu, "select_visible_layers");
            addActionToMenu(selectMenu, "select_invisible_layers");
            addActionToMenu(selectMenu, "select_locked_layers");
            addActionToMenu(selectMenu, "select_unlocked_layers");
            QMenu *groupMenu = menu.addMenu(i18n("&Group"));
            addActionToMenu(groupMenu, "create_quick_group");
            addActionToMenu(groupMenu, "create_quick_clipping_group");
            addActionToMenu(groupMenu, "quick_ungroup");
            QMenu *locksMenu = menu.addMenu(i18n("&Toggle Locks && Visibility"));
            addActionToMenu(locksMenu, "toggle_layer_visibility");
            addActionToMenu(locksMenu, "toggle_layer_lock");
            addActionToMenu(locksMenu, "toggle_layer_inherit_alpha");
            addActionToMenu(locksMenu, "toggle_layer_alpha_lock");

            if (singleLayer) {
                QMenu *addLayerMenu = menu.addMenu(i18n("&Add"));
                addActionToMenu(addLayerMenu, "add_new_transparency_mask");
                addActionToMenu(addLayerMenu, "add_new_filter_mask");
                addActionToMenu(addLayerMenu, "add_new_colorize_mask");
                addActionToMenu(addLayerMenu, "add_new_transform_mask");
                addActionToMenu(addLayerMenu, "add_new_selection_mask");

                QMenu *convertToMenu = menu.addMenu(i18n("&Convert"));
                addActionToMenu(convertToMenu, "convert_to_paint_layer");
                addActionToMenu(convertToMenu, "convert_to_transparency_mask");
                addActionToMenu(convertToMenu, "convert_to_filter_mask");
                addActionToMenu(convertToMenu, "convert_to_selection_mask");
                addActionToMenu(convertToMenu, "convert_to_file_layer");

                QMenu *splitAlphaMenu = menu.addMenu(i18n("S&plit Alpha"));
                addActionToMenu(splitAlphaMenu, "split_alpha_into_mask");
                addActionToMenu(splitAlphaMenu, "split_alpha_write");
                addActionToMenu(splitAlphaMenu, "split_alpha_save_merged");
            }

            menu.addSeparator();

            addActionToMenu(&menu, "show_in_timeline");

            if (singleLayer) {
                KisNodeSP node = m_filteringModel->nodeFromIndex(index);
                if (node && !node->inherits("KisTransformMask")) {
                    addActionToMenu(&menu, "isolate_layer");
                }

                addActionToMenu(&menu, "selectopaque");
            }
        }
        menu.exec(pos);
    }
}

void LayerBox::slotMinimalView()
{
    m_wdgLayerBox->listLayers->setDisplayMode(NodeView::MinimalMode);
}

void LayerBox::slotDetailedView()
{
    m_wdgLayerBox->listLayers->setDisplayMode(NodeView::DetailedMode);
}

void LayerBox::slotThumbnailView()
{
    m_wdgLayerBox->listLayers->setDisplayMode(NodeView::ThumbnailMode);
}

void LayerBox::slotRmClicked()
{
    if (!m_canvas) return;
    m_nodeManager->removeNode();
}

void LayerBox::slotRaiseClicked()
{
    if (!m_canvas) return;
    m_nodeManager->raiseNode();
}

void LayerBox::slotLowerClicked()
{
    if (!m_canvas) return;
    m_nodeManager->lowerNode();
}

void LayerBox::slotPropertiesClicked()
{
    if (!m_canvas) return;
    if (KisNodeSP active = m_nodeManager->activeNode()) {
        m_nodeManager->nodeProperties(active);
    }
}

void LayerBox::slotCompositeOpChanged(int index)
{
    Q_UNUSED(index);
    if (!m_canvas) return;

    QString compositeOp = m_wdgLayerBox->cmbComposite->selectedCompositeOp().id();
    m_nodeManager->nodeCompositeOpChanged(m_nodeManager->activeColorSpace()->compositeOp(compositeOp));
}

void LayerBox::slotOpacityChanged()
{
    if (!m_canvas) return;
    m_blockOpacityUpdate = true;
    m_nodeManager->nodeOpacityChanged(m_newOpacity, true);
    m_blockOpacityUpdate = false;
}

void LayerBox::slotOpacitySliderMoved(qreal opacity)
{
    m_newOpacity = opacity;
    m_opacityDelayTimer.start(200);
}

void LayerBox::slotCollapsed(const QModelIndex &index)
{
    KisNodeSP node = m_filteringModel->nodeFromIndex(index);
    if (node) {
        node->setCollapsed(true);
    }
}

void LayerBox::slotExpanded(const QModelIndex &index)
{
    KisNodeSP node = m_filteringModel->nodeFromIndex(index);
    if (node) {
        node->setCollapsed(false);
    }
}

void LayerBox::slotSelectOpaque()
{
    if (!m_canvas) return;
    QAction *action = m_canvas->viewManager()->actionManager()->actionByName("selectopaque");
    if (action) {
        action->trigger();
    }
}

void LayerBox::slotNodeCollapsedChanged()
{
    expandNodesRecursively(m_image->rootLayer(), m_filteringModel, m_wdgLayerBox->listLayers);
}

inline bool isSelectionMask(KisNodeSP node)
{
    return dynamic_cast<KisSelectionMask*>(node.data());
}

KisNodeSP LayerBox::findNonHidableNode(KisNodeSP startNode)
{
    if (KisNodeManager::isNodeHidden(startNode, true) &&
            startNode->parent() &&
            !startNode->parent()->parent()) {


        KisNodeSP node = startNode->prevSibling();
        while (node && KisNodeManager::isNodeHidden(node, true)) {
            node = node->prevSibling();
        }

        if (!node) {
            node = startNode->nextSibling();
            while (node && KisNodeManager::isNodeHidden(node, true)) {
                node = node->nextSibling();
            }
        }

        if (!node) {
            node = m_image->root()->lastChild();
            while (node && KisNodeManager::isNodeHidden(node, true)) {
                node = node->prevSibling();
            }
        }

        KIS_ASSERT_RECOVER_NOOP(node && "cannot activate any node!");
        startNode = node;
    }

    return startNode;
}

void LayerBox::slotEditGlobalSelection(bool showSelections)
{
    KisNodeSP lastActiveNode = m_nodeManager->activeNode();
    KisNodeSP activateNode = lastActiveNode;
    KisSelectionMaskSP globalSelectionMask;

    if (!showSelections) {
        activateNode = findNonHidableNode(activateNode);
    }

    m_nodeModel->setShowGlobalSelection(showSelections);

    globalSelectionMask = m_image->rootLayer()->selectionMask();

    // try to find deactivated, but visible masks
    if (!globalSelectionMask) {
        KoProperties properties;
        properties.setProperty("visible", true);
        QList<KisNodeSP> masks = m_image->rootLayer()->childNodes(QStringList("KisSelectionMask"), properties);
        if (!masks.isEmpty()) {
            globalSelectionMask = dynamic_cast<KisSelectionMask*>(masks.first().data());
        }
    }

    // try to find at least any selection mask
    if (!globalSelectionMask) {
        KoProperties properties;
        QList<KisNodeSP> masks = m_image->rootLayer()->childNodes(QStringList("KisSelectionMask"), properties);
        if (!masks.isEmpty()) {
            globalSelectionMask = dynamic_cast<KisSelectionMask*>(masks.first().data());
        }
    }

    if (globalSelectionMask) {
        if (showSelections) {
            activateNode = globalSelectionMask;
        }
    }

    if (activateNode != lastActiveNode) {
        m_nodeManager->slotNonUiActivatedNode(activateNode);
    } else if (lastActiveNode) {
        setCurrentNode(lastActiveNode);
    }

    if (showSelections && !globalSelectionMask) {
        KisProcessingApplicator applicator(m_image, 0,
                                           KisProcessingApplicator::NONE,
                                           KisImageSignalVector() << ModifiedSignal,
                                           kundo2_i18n("Quick Selection Mask"));

        applicator.applyCommand(
            new KisLayerUtils::KeepNodesSelectedCommand(
                m_nodeManager->selectedNodes(), KisNodeList(),
                lastActiveNode, 0, m_image, false),
            KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
        applicator.applyCommand(new KisSetEmptyGlobalSelectionCommand(m_image),
                                KisStrokeJobData::SEQUENTIAL,
                                KisStrokeJobData::EXCLUSIVE);
        applicator.applyCommand(new KisLayerUtils::SelectGlobalSelectionMask(m_image),
                                KisStrokeJobData::SEQUENTIAL,
                                KisStrokeJobData::EXCLUSIVE);

        applicator.end();
    } else if (!showSelections &&
               globalSelectionMask &&
               globalSelectionMask->selection()->selectedRect().isEmpty()) {

        KisProcessingApplicator applicator(m_image, 0,
                                           KisProcessingApplicator::NONE,
                                           KisImageSignalVector() << ModifiedSignal,
                                           kundo2_i18n("Cancel Quick Selection Mask"));
        applicator.applyCommand(new KisSetGlobalSelectionCommand(m_image, 0), KisStrokeJobData::SEQUENTIAL, KisStrokeJobData::EXCLUSIVE);
        applicator.end();
    }

}

void LayerBox::selectionChanged(const QModelIndexList selection)
{
    if (!m_nodeManager) return;

    /**
     * When the user clears the extended selection by clicking on the
     * empty area of the docker, the selection should be reset on to
     * the active layer, which might be even unselected(!).
     */
    if (selection.isEmpty() && m_nodeManager->activeNode()) {
        QModelIndex selectedIndex =
                m_filteringModel->indexFromNode(m_nodeManager->activeNode());

        m_wdgLayerBox->listLayers->selectionModel()->
                setCurrentIndex(selectedIndex, QItemSelectionModel::ClearAndSelect);
        return;
    }

    QList<KisNodeSP> selectedNodes;
    Q_FOREACH (const QModelIndex &idx, selection) {
        selectedNodes << m_filteringModel->nodeFromIndex(idx);
    }

    m_nodeManager->slotSetSelectedNodes(selectedNodes);
    updateUI();
}

void LayerBox::slotAboutToRemoveRows(const QModelIndex &parent, int start, int end)
{
    /**
     * Qt has changed its behavior when deleting an item. Previously
     * the selection priority was on the next item in the list, and
     * now it has shanged to the previous item. Here we just adjust
     * the selected item after the node removal. Please take care that
     * this method overrides what was done by the corresponding method
     * of QItemSelectionModel, which *has already done* its work. That
     * is why we use (start - 1) and (end + 1) in the activation
     * condition.
     *
     * See bug: https://bugs.kde.org/show_bug.cgi?id=345601
     */

    QModelIndex currentIndex = m_wdgLayerBox->listLayers->currentIndex();
    QAbstractItemModel *model = m_filteringModel;

    if (currentIndex.isValid() && parent == currentIndex.parent()
            && currentIndex.row() >= start - 1 && currentIndex.row() <= end + 1) {
        QModelIndex old = currentIndex;
        if (model && end < model->rowCount(parent) - 1) // there are rows left below the change
            currentIndex = model->index(end + 1, old.column(), parent);
        else if (start > 0) // there are rows left above the change
            currentIndex = model->index(start - 1, old.column(), parent);
        else // there are no rows left in the table
            currentIndex = QModelIndex();

        if (currentIndex.isValid() && currentIndex != old) {
            m_wdgLayerBox->listLayers->setCurrentIndex(currentIndex);
        }
    }
}

void LayerBox::slotNodeManagerChangedSelection(const KisNodeList &nodes)
{
    if (!m_nodeManager) return;

    QModelIndexList newSelection;
    Q_FOREACH(KisNodeSP node, nodes) {
        newSelection << m_filteringModel->indexFromNode(node);
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

void LayerBox::updateThumbnail()
{
    m_wdgLayerBox->listLayers->updateNode(m_wdgLayerBox->listLayers->currentIndex());
}

void LayerBox::slotRenameCurrentNode()
{
    m_wdgLayerBox->listLayers->edit(m_wdgLayerBox->listLayers->currentIndex());
}

void LayerBox::slotColorLabelChanged(int label)
{
    KisNodeList nodes = m_nodeManager->selectedNodes();

    Q_FOREACH(KisNodeSP node, nodes) {
        auto applyLabelFunc =
                [label](KisNodeSP node) {
            node->setColorLabelIndex(label);
        };

        KisLayerUtils::recursiveApplyNodes(node, applyLabelFunc);
    }
}

void LayerBox::updateAvailableLabels()
{
    if (!m_image) return;
    m_wdgLayerBox->cmbFilter->updateAvailableLabels(m_image->root());
}

void LayerBox::updateLayerFiltering()
{
    m_filteringModel->setAcceptedLabels(m_wdgLayerBox->cmbFilter->selectedColors());
}

void LayerBox::slotKeyframeChannelAdded(KisKeyframeChannel *channel)
{
    if (channel->id() == KisKeyframeChannel::Opacity.id()) {
        watchOpacityChannel(channel);
    }
}

void LayerBox::watchOpacityChannel(KisKeyframeChannel *channel)
{
    if (m_opacityChannel) {
        m_opacityChannel->disconnect(this);
    }

    m_opacityChannel = channel;
    if (m_opacityChannel) {
        connect(m_opacityChannel, SIGNAL(sigKeyframeAdded(KisKeyframeSP)), this, SLOT(slotOpacityKeyframeChanged(KisKeyframeSP)));
        connect(m_opacityChannel, SIGNAL(sigKeyframeRemoved(KisKeyframeSP)), this, SLOT(slotOpacityKeyframeChanged(KisKeyframeSP)));
        connect(m_opacityChannel, SIGNAL(sigKeyframeMoved(KisKeyframeSP)), this, SLOT(slotOpacityKeyframeMoved(KisKeyframeSP)));
        connect(m_opacityChannel, SIGNAL(sigKeyframeChanged(KisKeyframeSP)), this, SLOT(slotOpacityKeyframeChanged(KisKeyframeSP)));
    }
}

void LayerBox::slotOpacityKeyframeChanged(KisKeyframeSP keyframe)
{
    Q_UNUSED(keyframe);
    if (m_blockOpacityUpdate) return;
    updateUI();
}

void LayerBox::slotOpacityKeyframeMoved(KisKeyframeSP keyframe, int fromTime)
{
    Q_UNUSED(fromTime);
    slotOpacityKeyframeChanged(keyframe);
}

void LayerBox::slotImageTimeChanged(int time)
{
    Q_UNUSED(time);
    updateUI();
}

void LayerBox::slotUpdateIcons() {
    m_wdgLayerBox->bnAdd->setIcon(KisIconUtils::loadIcon("addlayer"));
    m_wdgLayerBox->bnRaise->setIcon(KisIconUtils::loadIcon("arrowupblr"));
    m_wdgLayerBox->bnDelete->setIcon(KisIconUtils::loadIcon("deletelayer"));
    m_wdgLayerBox->bnLower->setIcon(KisIconUtils::loadIcon("arrowdown"));
    m_wdgLayerBox->bnProperties->setIcon(KisIconUtils::loadIcon("properties"));
    m_wdgLayerBox->bnDuplicate->setIcon(KisIconUtils::loadIcon("duplicatelayer"));

    // call child function about needing to update icons
    m_wdgLayerBox->listLayers->slotUpdateIcons();
}

void LayerBox::slotUpdateThumbnailIconSize()
{
    KisConfig cfg(false);
    cfg.setLayerThumbnailSize(thumbnailSizeSlider->value());

    // this is a hack to force the layers list to update its display and
    // re-layout all the layers with the new thumbnail size
    resize(this->width()+1, this->height()+1);
    resize(this->width()-1, this->height()-1);
}


#include "moc_LayerBox.cpp"
