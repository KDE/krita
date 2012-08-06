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

#include <kis_debug.h>
#include <kglobal.h>
#include <kmenu.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <klocale.h>
#include <khbox.h>
#include <kaction.h>

#include <KoIcon.h>
#include <KoDocumentSectionView.h>
#include <KoColorSpace.h>
#include <KoCompositeOp.h>

#include <kis_types.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_group_layer.h>
#include <kis_mask.h>
#include <kis_node.h>
#include <kis_composite_ops_model.h>

#include "widgets/kis_cmb_composite.h"
#include "widgets/kis_slider_spin_box.h"
#include "kis_view2.h"
#include "kis_node_manager.h"
#include "kis_node_model.h"
#include "canvas/kis_canvas2.h"
#include "kis_doc2.h"
#include "kis_dummies_facade_base.h"
#include <KoShapeBasedDocumentBase.h>


#include "ui_wdglayerbox.h"


KisLayerBox::KisLayerBox()
        : QDockWidget(i18n("Layers"))
        , m_canvas(0)
        , m_wdgLayerBox(new Ui_WdgLayerBox)
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);
    m_delayTimer.setSingleShot(true);

    m_wdgLayerBox->setupUi(mainWidget);

    m_wdgLayerBox->listLayers->setDefaultDropAction(Qt::MoveAction);
    m_wdgLayerBox->listLayers->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);
    m_wdgLayerBox->listLayers->setSelectionBehavior(QAbstractItemView::SelectRows);

    connect(m_wdgLayerBox->listLayers, SIGNAL(contextMenuRequested(const QPoint&, const QModelIndex&)),
            this, SLOT(slotContextMenuRequested(const QPoint&, const QModelIndex&)));
    connect(m_wdgLayerBox->listLayers, SIGNAL(collapsed(const QModelIndex&)), SLOT(slotCollapsed(const QModelIndex &)));
    connect(m_wdgLayerBox->listLayers, SIGNAL(expanded(const QModelIndex&)), SLOT(slotExpanded(const QModelIndex &)));

    m_viewModeMenu = new KMenu(this);
    QActionGroup *group = new QActionGroup(this);
    QList<QAction*> actions;

    actions << m_viewModeMenu->addAction(koIcon("view-list-text"),
                                         i18n("Minimal View"), this, SLOT(slotMinimalView()));
    actions << m_viewModeMenu->addAction(koIcon("view-list-details"),
                                         i18n("Detailed View"), this, SLOT(slotDetailedView()));
    actions << m_viewModeMenu->addAction(koIcon("view-preview"),
                                         i18n("Thumbnail View"), this, SLOT(slotThumbnailView()));

    for (int i = 0, n = actions.count(); i < n; ++i) {
        actions[i]->setCheckable(true);
        actions[i]->setActionGroup(group);
    }
    actions[1]->trigger(); //TODO save/load previous state

    m_wdgLayerBox->bnAdd->setIcon(koIcon("list-add"));

    m_wdgLayerBox->bnViewMode->setMenu(m_viewModeMenu);
    m_wdgLayerBox->bnViewMode->setPopupMode(QToolButton::InstantPopup);
    m_wdgLayerBox->bnViewMode->setIcon(koIcon("view-choose"));
    m_wdgLayerBox->bnViewMode->setText(i18n("View mode"));

    m_wdgLayerBox->bnDelete->setIcon(koIcon("list-remove"));
    m_wdgLayerBox->bnDelete->setIconSize(QSize(22, 22));

    m_wdgLayerBox->bnRaise->setEnabled(false);
    m_wdgLayerBox->bnRaise->setIcon(koIcon("go-up"));
    m_wdgLayerBox->bnRaise->setIconSize(QSize(22, 22));

    m_wdgLayerBox->bnLower->setEnabled(false);
    m_wdgLayerBox->bnLower->setIcon(koIcon("go-down"));
    m_wdgLayerBox->bnLower->setIconSize(QSize(22, 22));

    m_wdgLayerBox->bnLeft->setEnabled(true);
    m_wdgLayerBox->bnLeft->setIcon(koIcon("arrow-left"));
    m_wdgLayerBox->bnLeft->setIconSize(QSize(22, 22));

    m_wdgLayerBox->bnRight->setEnabled(true);
    m_wdgLayerBox->bnRight->setIcon(koIcon("arrow-right"));
    m_wdgLayerBox->bnRight->setIconSize(QSize(22, 22));

    m_wdgLayerBox->bnProperties->setIcon(koIcon("document-properties"));
    m_wdgLayerBox->bnProperties->setIconSize(QSize(22, 22));

    m_wdgLayerBox->bnDuplicate->setIcon(koIcon("edit-copy"));
    m_wdgLayerBox->bnDuplicate->setIconSize(QSize(22, 22));

    connect(m_wdgLayerBox->bnDelete, SIGNAL(clicked()), SLOT(slotRmClicked()));
    // NOTE: this is _not_ a mistake. The layerbox shows the layers in the reverse order
    connect(m_wdgLayerBox->bnRaise, SIGNAL(clicked()), SLOT(slotLowerClicked()));
    connect(m_wdgLayerBox->bnLower, SIGNAL(clicked()), SLOT(slotRaiseClicked()));
    // END NOTE
    connect(m_wdgLayerBox->bnLeft, SIGNAL(clicked()), SLOT(slotLeftClicked()));
    connect(m_wdgLayerBox->bnRight, SIGNAL(clicked()), SLOT(slotRightClicked()));

    connect(m_wdgLayerBox->bnProperties, SIGNAL(clicked()), SLOT(slotPropertiesClicked()));
    connect(m_wdgLayerBox->bnDuplicate, SIGNAL(clicked()), SLOT(slotDuplicateClicked()));

    connect(m_wdgLayerBox->doubleOpacity, SIGNAL(valueChanged(qreal)), SLOT(slotOpacitySliderMoved(qreal)));
    connect(&m_delayTimer, SIGNAL(timeout()), SLOT(slotOpacityChanged()));

    connect(m_wdgLayerBox->cmbComposite, SIGNAL(activated(int)), SLOT(slotCompositeOpChanged(int)));
    connect(m_wdgLayerBox->bnAdd, SIGNAL(clicked()), SLOT(slotNewPaintLayer()));

    m_newPainterLayerAction = new KAction(koIcon("document-new"), i18n("&Paint Layer"), this);
    connect(m_newPainterLayerAction, SIGNAL(triggered(bool)), this, SLOT(slotNewPaintLayer()));

    m_newGroupLayerAction = new KAction(koIcon("folder-new"), i18n("&Group Layer"), this);
    connect(m_newGroupLayerAction, SIGNAL(triggered(bool)), this, SLOT(slotNewGroupLayer()));

    m_newCloneLayerAction = new KAction(koIcon("edit-copy"), i18n("&Clone Layer"), this);
    connect(m_newCloneLayerAction, SIGNAL(triggered(bool)), this, SLOT(slotNewCloneLayer()));

    m_newShapeLayerAction = new KAction(koIcon("bookmark-new"), i18n("&Vector Layer"), this);
    connect(m_newShapeLayerAction, SIGNAL(triggered(bool)), this, SLOT(slotNewShapeLayer()));

    m_newAdjustmentLayerAction = new KAction(koIcon("view-filter"), i18n("&Filter Layer..."), this);
    connect(m_newAdjustmentLayerAction, SIGNAL(triggered(bool)), this, SLOT(slotNewAdjustmentLayer()));

    m_newGeneratorLayerAction = new KAction(koIcon("view-filter"), i18n("&Generated Layer..."), this);
    connect(m_newGeneratorLayerAction, SIGNAL(triggered(bool)), this, SLOT(slotNewGeneratorLayer()));

    m_newTransparencyMaskAction = new KAction(koIcon("edit-copy"), i18n("&Transparency Mask"), this);
    connect(m_newTransparencyMaskAction, SIGNAL(triggered(bool)), this, SLOT(slotNewTransparencyMask()));

    m_newEffectMaskAction = new KAction(koIcon("bookmarks"), i18n("&Filter Mask..."), this);
    connect(m_newEffectMaskAction, SIGNAL(triggered(bool)), this, SLOT(slotNewEffectMask()));

    m_newSelectionMaskAction = new KAction(koIcon("edit-paste"), i18n("&Local Selection"), this);
    connect(m_newSelectionMaskAction, SIGNAL(triggered(bool)), this, SLOT(slotNewSelectionMask()));

    m_newLayerMenu = new KMenu(this);
    m_wdgLayerBox->bnAdd->setMenu(m_newLayerMenu);
    m_wdgLayerBox->bnAdd->setPopupMode(QToolButton::MenuButtonPopup);

    m_newLayerMenu->addAction(m_newPainterLayerAction);
    m_newLayerMenu->addAction(m_newGroupLayerAction);
    m_newLayerMenu->addAction(m_newCloneLayerAction);
    m_newLayerMenu->addAction(m_newShapeLayerAction);
    m_newLayerMenu->addAction(m_newAdjustmentLayerAction);
    m_newLayerMenu->addAction(m_newGeneratorLayerAction);
    m_newLayerMenu->addSeparator();
    m_newLayerMenu->addAction(m_newTransparencyMaskAction);
    m_newLayerMenu->addAction(m_newEffectMaskAction);
#if 0 // XXX_2.0
    m_newLayerMenu->addAction(koIcon("view-filter"), i18n("&Transformation Mask..."), this, SLOT(slotNewTransformationMa
    sk()));
#endif
    m_newLayerMenu->addAction(m_newSelectionMaskAction);

    
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
    connect(m_nodeModel, SIGNAL(modelReset()), SLOT(updateUI()));

    m_wdgLayerBox->listLayers->setModel(m_nodeModel);
}

KisLayerBox::~KisLayerBox()
{
    delete m_wdgLayerBox;
}


void expandNodesRecursively(KisNodeSP root, QPointer<KisNodeModel> nodeModel, KoDocumentSectionView *sectionView)
{
    if (!root) return;
    if (nodeModel.isNull()) return;
    if (!sectionView) return;

    sectionView->blockSignals(true);

    KisNodeSP node = root->firstChild();
    while (node) {
        QModelIndex idx = nodeModel->indexFromNode(node);
        if (idx.isValid()) {
            if (node->collapsed()) {
                sectionView->collapse(idx);
            }
        }
        if (node->childCount() > 0) {
            expandNodesRecursively(node, nodeModel, sectionView);
        }
        node = node->nextSibling();
    }
    sectionView->blockSignals(false);
}

void KisLayerBox::setCanvas(KoCanvasBase *canvas)
{

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
        m_nodeModel->setDummiesFacade(0, 0);

        disconnect(m_image, 0, this, 0);
        disconnect(m_nodeManager, 0, this, 0);
        disconnect(m_nodeModel, 0, m_nodeManager, 0);
        disconnect(m_nodeModel, SIGNAL(nodeActivated(KisNodeSP)), this, SLOT(updateUI()));
    }

    m_canvas = dynamic_cast<KisCanvas2*>(canvas);

    if (m_canvas) {
        m_image = m_canvas->view()->image();

        m_nodeManager = m_canvas->view()->nodeManager();

        KisDummiesFacadeBase *kritaDummiesFacade = dynamic_cast<KisDummiesFacadeBase*>(m_canvas->view()->document()->shapeController());
        m_nodeModel->setDummiesFacade(kritaDummiesFacade, m_image);

        connect(m_image, SIGNAL(sigAboutToBeDeleted()), SLOT(notifyImageDeleted()));

        // cold start
        setCurrentNode(m_nodeManager->activeNode());

        // Connection KisNodeManager -> KisLayerBox
        connect(m_nodeManager, SIGNAL(sigUiNeedChangeActiveNode(KisNodeSP)), this, SLOT(setCurrentNode(KisNodeSP)));

        // Connection KisLayerBox -> KisNodeManager
        // The order of these connections is important! See comment in the ctor
        connect(m_nodeModel, SIGNAL(nodeActivated(KisNodeSP)), m_nodeManager, SLOT(slotUiActivatedNode(KisNodeSP)));
        connect(m_nodeModel, SIGNAL(nodeActivated(KisNodeSP)), SLOT(updateUI()));

        // Node manipulation methods are forwarded to the node manager
        connect(m_nodeModel, SIGNAL(requestAddNode(KisNodeSP, KisNodeSP, KisNodeSP)),
                m_nodeManager, SLOT(addNodeDirect(KisNodeSP, KisNodeSP, KisNodeSP)));
        connect(m_nodeModel, SIGNAL(requestMoveNode(KisNodeSP, KisNodeSP, KisNodeSP)),
                m_nodeManager, SLOT(moveNodeDirect(KisNodeSP, KisNodeSP, KisNodeSP)));

        m_wdgLayerBox->listLayers->expandAll();
        expandNodesRecursively(m_image->rootLayer(), m_nodeModel, m_wdgLayerBox->listLayers);
        m_wdgLayerBox->listLayers->scrollToBottom();
    }

}


void KisLayerBox::unsetCanvas()
{
    setCanvas(0);
}

void KisLayerBox::notifyImageDeleted()
{
    setCanvas(0);
}

void KisLayerBox::updateUI()
{
    if(!m_canvas) return;

    KisNodeSP active = m_nodeManager->activeNode();

    m_wdgLayerBox->bnDelete->setEnabled(active);
    m_wdgLayerBox->bnRaise->setEnabled(active && (active->nextSibling()
                                       || (active->parent() && active->parent() != m_image->root())));
    m_wdgLayerBox->bnLower->setEnabled(active && (active->prevSibling()
                                       || (active->parent() && active->parent() != m_image->root())));
    m_wdgLayerBox->bnLeft->setEnabled(active);
    m_wdgLayerBox->bnRight->setEnabled(active);
    m_wdgLayerBox->bnDuplicate->setEnabled(active);
    m_wdgLayerBox->bnProperties->setEnabled(active);

    m_wdgLayerBox->doubleOpacity->setEnabled(active);
    m_wdgLayerBox->doubleOpacity->setRange(0, 100, 0);

    m_wdgLayerBox->cmbComposite->setEnabled(active);

    if (active) {
        if (m_nodeManager->activePaintDevice()) {
            slotFillCompositeOps(m_nodeManager->activeColorSpace());
        } else {
            slotFillCompositeOps(m_image->colorSpace());
        }

        if (active->inherits("KisMask")) {
            active = active->parent(); // We need a layer to set opacity and composite op, which masks don't have
        }
        if (active->inherits("KisLayer")) {
            KisLayerSP l = qobject_cast<KisLayer*>(active.data());
            slotSetOpacity(l->opacity() * 100.0 / 255);

            const KoCompositeOp* compositeOp = l->compositeOp();
            if(compositeOp) {
                slotSetCompositeOp(compositeOp);
            } else {
                m_wdgLayerBox->cmbComposite->setEnabled(false);
            }
        }
    }
    m_newTransparencyMaskAction->setEnabled(active);
    m_newEffectMaskAction->setEnabled(active);
    m_newSelectionMaskAction->setEnabled(active);
    m_newCloneLayerAction->setEnabled(active);
}


/**
 * This method is callen *only* when non-GUI code requested the
 * change of the current node
 */
void KisLayerBox::setCurrentNode(KisNodeSP node)
{
    QModelIndex index = node ? m_nodeModel->indexFromNode(node) : QModelIndex();

    m_wdgLayerBox->listLayers->setCurrentIndex(index);
    updateUI();
}

void KisLayerBox::slotSetCompositeOp(const KoCompositeOp* compositeOp)
{
    KoID cmpOp = KoCompositeOpRegistry::instance().getKoID(compositeOp->id());
    int  index = m_wdgLayerBox->cmbComposite->indexOf(cmpOp);
    
    m_wdgLayerBox->cmbComposite->blockSignals(true);
    m_wdgLayerBox->cmbComposite->setCurrentIndex(index);
    m_wdgLayerBox->cmbComposite->blockSignals(false);
}

void KisLayerBox::slotFillCompositeOps(const KoColorSpace* colorSpace)
{
    m_wdgLayerBox->cmbComposite->getModel()->validateCompositeOps(colorSpace);
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
    QMenu menu;

    if (index.isValid()) {
        menu.addAction(koIcon("document-properties"), i18n("&Properties..."), this, SLOT(slotPropertiesClicked()));
        menu.addSeparator();
        menu.addAction(koIcon("edit-delete"), i18n("&Remove Layer"), this, SLOT(slotRmClicked()));
        menu.addAction(koIcon("edit-copy"), i18n("&Duplicate Layer or Mask"), this, SLOT(slotDuplicateClicked()));
        // TODO: missing icon "edit-merge"
        QAction* mergeLayerDown = menu.addAction(i18n("&Merge with Layer Below"), this, SLOT(slotMergeLayer()));
        if (!index.sibling(index.row() + 1, 0).isValid()) mergeLayerDown->setEnabled(false);
        menu.addSeparator();
    }
    menu.addSeparator();
    menu.addAction(m_newTransparencyMaskAction);
    menu.addAction(m_newEffectMaskAction);
    menu.addAction(m_newSelectionMaskAction);

    menu.exec(pos);
}

void KisLayerBox::slotMergeLayer()
{
    if(!m_canvas) return;
    m_nodeManager->mergeLayerDown();
}

void KisLayerBox::slotMinimalView()
{
    m_wdgLayerBox->listLayers->setDisplayMode(KoDocumentSectionView::MinimalMode);
}

void KisLayerBox::slotDetailedView()
{
    m_wdgLayerBox->listLayers->setDisplayMode(KoDocumentSectionView::DetailedMode);
}

void KisLayerBox::slotThumbnailView()
{
    m_wdgLayerBox->listLayers->setDisplayMode(KoDocumentSectionView::ThumbnailMode);
}

void KisLayerBox::slotNewPaintLayer()
{
    if(!m_canvas) return;
    m_nodeManager->createNode("KisPaintLayer");
}

void KisLayerBox::slotNewGroupLayer()
{
    if(!m_canvas) return;
    m_nodeManager->createNode("KisGroupLayer");
}

void KisLayerBox::slotNewCloneLayer()
{
    if(!m_canvas) return;
    m_nodeManager->createNode("KisCloneLayer");
}


void KisLayerBox::slotNewShapeLayer()
{
    if(!m_canvas) return;
    m_nodeManager->createNode("KisShapeLayer");
}


void KisLayerBox::slotNewAdjustmentLayer()
{
    if(!m_canvas) return;
    m_nodeManager->createNode("KisAdjustmentLayer");
}

void KisLayerBox::slotNewGeneratorLayer()
{
    if(!m_canvas) return;
    m_nodeManager->createNode("KisGeneratorLayer");
}

void KisLayerBox::slotNewTransparencyMask()
{
    if(!m_canvas) return;
    m_nodeManager->createNode("KisTransparencyMask");
}

void KisLayerBox::slotNewEffectMask()
{
    if(!m_canvas) return;
    m_nodeManager->createNode("KisFilterMask");
}


void KisLayerBox::slotNewSelectionMask()
{
    if(!m_canvas) return;
    m_nodeManager->createNode("KisSelectionMask");
}


void KisLayerBox::slotRmClicked()
{
    if(!m_canvas) return;

    KisNodeSP node = m_nodeManager->activeNode();
    m_nodeManager->removeNode(node);
}

void KisLayerBox::slotRaiseClicked()
{
    if(!m_canvas) return;
    KisNodeSP node = m_nodeManager->activeNode();
    KisNodeSP parent = node->parent();
    KisNodeSP grandParent = parent->parent();

    if (!m_nodeManager->activeNode()->prevSibling()) {
        if (!grandParent) return;  
        if (!grandParent->parent() && node->inherits("KisMask")) return;
        m_nodeManager->moveNodeAt(node, grandParent, grandParent->index(parent));
    } else {
        m_nodeManager->raiseNode();
    }
}

void KisLayerBox::slotLowerClicked()
{
    if(!m_canvas) return;
    KisNodeSP node = m_nodeManager->activeNode();
    KisNodeSP parent = node->parent();
    KisNodeSP grandParent = parent->parent();
    
    if (!m_nodeManager->activeNode()->nextSibling()) {
        if (!grandParent) return;  
        if (!grandParent->parent() && node->inherits("KisMask")) return;
        m_nodeManager->moveNodeAt(node, grandParent, grandParent->index(parent) + 1);
    } else {
        m_nodeManager->lowerNode();
    }
}

void KisLayerBox::slotLeftClicked()
{
    if(!m_canvas) return;
    KisNodeSP node = m_nodeManager->activeNode();
    KisNodeSP parent = node->parent();
    KisNodeSP grandParent = parent->parent();
    quint16 nodeIndex = parent->index(node);
    
    if (!grandParent) return;  
    if (!grandParent->parent() && node->inherits("KisMask")) return;

    if (nodeIndex <= parent->childCount() / 2) {
        m_nodeManager->moveNodeAt(node, grandParent, grandParent->index(parent));
    } else {
        m_nodeManager->moveNodeAt(node, grandParent, grandParent->index(parent) + 1);
    }
}

void KisLayerBox::slotRightClicked()
{
    if(!m_canvas) return;
    KisNodeSP node = m_nodeManager->activeNode();
    KisNodeSP parent = m_nodeManager->activeNode()->parent();
    KisNodeSP newParent;
    int nodeIndex = parent->index(node);
    int indexAbove = nodeIndex + 1;
    int indexBelow = nodeIndex - 1;

    if (parent->at(indexBelow) && parent->at(indexBelow)->allowAsChild(node)) {
        newParent = parent->at(indexBelow);
        m_nodeManager->moveNodeAt(node, newParent, newParent->childCount());
    } else if (parent->at(indexAbove) && parent->at(indexAbove)->allowAsChild(node)) {
        newParent = parent->at(indexAbove);
        m_nodeManager->moveNodeAt(node, newParent, 0);
    } else {
        return;
    }
}

void KisLayerBox::slotPropertiesClicked()
{
    if(!m_canvas) return;
    if (KisNodeSP active = m_nodeManager->activeNode()) {
        m_nodeManager->nodeProperties(active);
    }
}

void KisLayerBox::slotDuplicateClicked()
{
    if(!m_canvas) return;
    m_nodeManager->duplicateActiveNode();
}

void KisLayerBox::slotCompositeOpChanged(int index)
{
    if(!m_canvas) return;

    KoID compositeOp;
    if(m_wdgLayerBox->cmbComposite->entryAt(compositeOp, index))
        m_nodeManager->nodeCompositeOpChanged(m_nodeManager->activeColorSpace()->compositeOp(compositeOp.id()));
}

void KisLayerBox::slotOpacityChanged()
{
    if(!m_canvas) return;
    m_nodeManager->nodeOpacityChanged(m_newOpacity, true);
}

void KisLayerBox::slotOpacitySliderMoved(qreal opacity)
{
    m_newOpacity = opacity;
    m_delayTimer.start(200);
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


#include "kis_layer_box.moc"
