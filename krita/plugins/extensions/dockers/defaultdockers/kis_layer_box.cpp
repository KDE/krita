/*
 *  kis_layer_box.cc - part of Krita aka Krayon aka KimageShop
 *
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (C) 2006 Gábor Lehel <illissius@gmail.com>
 *  Copyright (C) 2007 Thomas Zander <zander@kde.org>
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
#include <kiconloader.h>
#include <kicontheme.h>
#include <klocale.h>
#include <khbox.h>
#include <kicon.h>

#include "KoSliderCombo.h"
#include <KoDocumentSectionView.h>
#include "KoColorSpace.h"

#include <kis_types.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_layer.h>
#include <kis_group_layer.h>
#include <kis_mask.h>
#include <kis_node.h>

#include "widgets/kis_cmb_composite.h"
#include "kis_view2.h"
#include "kis_node_manager.h"
#include "kis_node_model.h"
#include "canvas/kis_canvas2.h"
#include "kis_doc2.h"

#include "ui_wdglayerbox.h"

KisLayerBox::KisLayerBox()
        : QDockWidget(i18n("Layers"))
        , m_canvas(0)
        , m_wdgLayerBox(new Ui_WdgLayerBox)
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);

    m_wdgLayerBox->setupUi(mainWidget);

    setMinimumSize(mainWidget->minimumSizeHint());

    m_wdgLayerBox->listLayers->viewport()->installEventFilter(this);
    m_wdgLayerBox->listLayers->setDragDropMode(QAbstractItemView::InternalMove);
    m_wdgLayerBox->listLayers->setVerticalScrollMode(QAbstractItemView::ScrollPerItem);

    connect(m_wdgLayerBox->listLayers, SIGNAL(contextMenuRequested(const QPoint&, const QModelIndex&)),
            this, SLOT(slotContextMenuRequested(const QPoint&, const QModelIndex&)));
    connect(m_wdgLayerBox->listLayers, SIGNAL(clicked(const QModelIndex&)), SLOT(slotNodeActivated(const QModelIndex&)));

    m_viewModeMenu = new KMenu(this);
    QActionGroup *group = new QActionGroup(this);
    QList<QAction*> actions;

    actions << m_viewModeMenu->addAction(KIcon("view-list-text"),
                                         i18n("Minimal View"), this, SLOT(slotMinimalView()));
    actions << m_viewModeMenu->addAction(KIcon("view-list-details"),
                                         i18n("Detailed View"), this, SLOT(slotDetailedView()));
    actions << m_viewModeMenu->addAction(KIcon("view-preview"),
                                         i18n("Thumbnail View"), this, SLOT(slotThumbnailView()));

    for (int i = 0, n = actions.count(); i < n; ++i) {
        actions[i]->setCheckable(true);
        actions[i]->setActionGroup(group);
    }
    actions[1]->trigger(); //TODO save/load previous state

    m_wdgLayerBox->bnViewMode->setMenu(m_viewModeMenu);
    m_wdgLayerBox->bnViewMode->setPopupMode(QToolButton::InstantPopup);
    m_wdgLayerBox->bnViewMode->setIcon(KIcon("view-choose"));
    m_wdgLayerBox->bnViewMode->setText(i18n("View mode"));

    m_wdgLayerBox->bnAdd->setIcon(SmallIcon("list-add"));

    m_wdgLayerBox->bnDelete->setIcon(SmallIcon("list-remove"));

    m_wdgLayerBox->bnRaise->setEnabled(false);
    m_wdgLayerBox->bnRaise->setIcon(SmallIcon("go-up"));

    m_wdgLayerBox->bnLower->setEnabled(false);
    m_wdgLayerBox->bnLower->setIcon(SmallIcon("go-down"));

    m_wdgLayerBox->bnProperties->setText("...");

    m_wdgLayerBox->bnDuplicate->setIcon(SmallIcon("edit-copy"));

    m_newLayerMenu = new KMenu(this);
    m_wdgLayerBox->bnAdd->setMenu(m_newLayerMenu);
    m_wdgLayerBox->bnAdd->setPopupMode(QToolButton::MenuButtonPopup);
    connect(m_wdgLayerBox->bnAdd, SIGNAL(clicked()), SLOT(slotNewPaintLayer()));
    m_newLayerMenu->addAction(KIcon("document-new"), i18n("&Paint Layer"), this, SLOT(slotNewPaintLayer()));
    m_newLayerMenu->addAction(KIcon("folder-new"), i18n("&Group Layer"), this, SLOT(slotNewGroupLayer()));
    m_newLayerMenu->addAction(KIcon("edit-copy"), i18n("&Clone Layer"), this, SLOT(slotNewCloneLayer()));
    m_newLayerMenu->addAction(KIcon("bookmark-new"), i18n("&Shape Layer"), this, SLOT(slotNewShapeLayer()));
    m_newLayerMenu->addAction(KIcon("view-filter"), i18n("&Filter Layer..."), this, SLOT(slotNewAdjustmentLayer()));
    m_newLayerMenu->addAction(KIcon("view-filter"), i18n("&Generated Layer..."), this, SLOT(slotNewGeneratorLayer()));
    m_newLayerMenu->addSeparator();
    m_newLayerMenu->addAction(KIcon("edit-copy"), i18n("&Transparency Mask"), this, SLOT(slotNewTransparencyMask()));
    m_newLayerMenu->addAction(KIcon("bookmarks"), i18n("&Filter Mask..."), this, SLOT(slotNewEffectMask()));
#if 0 // XXX_2.0
    m_newLayerMenu->addAction(KIcon("view-filter"), i18n("&Transformation Mask..."), this, SLOT(slotNewTransformationMask()));
#endif
    m_newLayerMenu->addAction(KIcon("edit-paste"), i18n("&Local Selection"), this, SLOT(slotNewSelectionMask()));
    connect(m_wdgLayerBox->bnDelete, SIGNAL(clicked()), SLOT(slotRmClicked()));

    // NOTE: this is _not_ a mistake. The layerbox shows the layers in the reverse order
    connect(m_wdgLayerBox->bnRaise, SIGNAL(clicked()), SLOT(slotLowerClicked()));
    connect(m_wdgLayerBox->bnLower, SIGNAL(clicked()), SLOT(slotRaiseClicked()));
    // END NOTE

    connect(m_wdgLayerBox->bnProperties, SIGNAL(clicked()), SLOT(slotPropertiesClicked()));
    connect(m_wdgLayerBox->bnDuplicate, SIGNAL(clicked()), SLOT(slotDuplicateClicked()));
    connect(m_wdgLayerBox->doubleOpacity, SIGNAL(valueChanged(qreal, bool)), SLOT(slotOpacityChanged(qreal, bool)));
    connect(m_wdgLayerBox->cmbComposite, SIGNAL(activated(const QString&)), SLOT(slotCompositeOpChanged(const QString&)));

}

KisLayerBox::~KisLayerBox()
{
    delete m_wdgLayerBox;
}

bool KisLayerBox::eventFilter(QObject *o, QEvent *e)
{
    Q_ASSERT(o == m_wdgLayerBox->listLayers->viewport());

    if (e->type() == QEvent::MouseButtonDblClick) {

        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        QModelIndex mi = m_wdgLayerBox->listLayers->indexAt(me->pos());
        if (mi.isValid())
            slotPropertiesClicked();
        else
            slotNewPaintLayer();
        return true;
    }

    return QDockWidget::eventFilter(o, e);
}

void KisLayerBox::setCanvas(KoCanvasBase * canvas)
{
    disconnect();
    m_canvas = dynamic_cast<KisCanvas2*>(canvas);
    connect(m_canvas, SIGNAL(imageChanged(KisImageWSP)), SLOT(setImage(KisImageWSP)));
}

void KisLayerBox::setImage(KisImageWSP image)
{
    if (!image) return;
    m_image = image;
    if (m_canvas && m_canvas->view()) {
        KisView2* view = m_canvas->view();

        if (!m_nodeManager.isNull()) {
            m_nodeManager->disconnect(this);
        }
        m_nodeManager = view->nodeManager();
        connect(m_nodeManager, SIGNAL(sigNodeActivated(KisNodeSP)), this, SLOT(setCurrentNode(KisNodeSP)));

        if (!m_nodeModel.isNull()) {
            m_nodeModel->disconnect(this);
        }
        m_nodeModel = view->document()->nodeModel();
        m_wdgLayerBox->listLayers->setModel(m_nodeModel);
        connect(m_nodeModel, SIGNAL(nodeActivated(KisNodeSP)), this, SLOT(updateUI()));
        connect(m_nodeModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(updateUI()));
        connect(m_nodeModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(updateUI()));
        connect(m_nodeModel, SIGNAL(modelReset()), this, SLOT(updateUI()));

        if (m_nodeManager->activeNode()) {
            setCurrentNode(m_nodeManager->activeNode());
        }

        m_image = view->image();

        updateUI();

        m_wdgLayerBox->listLayers->expandAll();
        m_wdgLayerBox->listLayers->scrollToBottom();
    }
}


void KisLayerBox::updateUI()
{
    Q_ASSERT(! m_image.isNull());

    m_wdgLayerBox->bnDelete->setEnabled(m_nodeManager->activeNode());
    m_wdgLayerBox->bnRaise->setEnabled(m_nodeManager->activeNode()
                                       && (m_nodeManager->activeNode()->nextSibling()
                                           || (m_nodeManager->activeNode()->parent()
                                               && m_nodeManager->activeNode()->parent() != m_image->root())));

    m_wdgLayerBox->bnLower->setEnabled(m_nodeManager->activeNode() && m_nodeManager->activeNode()->prevSibling());

    m_wdgLayerBox->doubleOpacity->setEnabled(m_nodeManager->activeNode());
    m_wdgLayerBox->doubleOpacity->setDecimals(0);

    m_wdgLayerBox->cmbComposite->setEnabled(m_nodeManager->activeNode());

    if (KisNodeSP active = m_nodeManager->activeNode()) {
        if (m_nodeManager->activePaintDevice())
            slotFillCompositeOps(m_nodeManager->activeColorSpace());
        else
            slotFillCompositeOps(m_image->colorSpace());
        if (active->inherits("KisMask")) {
            active = active->parent(); // We need a layer to set opacity and composite op, which masks don't have
        }
        if (active->inherits("KisLayer")) {
            KisLayerSP l = qobject_cast<KisLayer*>(active.data());
            slotSetOpacity(l->opacity() * 100.0 / 255);
            slotSetCompositeOp(l->compositeOp());
        }
    }
}

void KisLayerBox::setCurrentNode(KisNodeSP node)
{
    if (node && m_nodeModel) {
        m_wdgLayerBox->listLayers->setCurrentIndex(m_nodeModel->indexFromNode(node));
        updateUI();
    }

}

void KisLayerBox::slotSetCompositeOp(const KoCompositeOp* compositeOp)
{
    m_wdgLayerBox->cmbComposite->blockSignals(true);
    m_wdgLayerBox->cmbComposite->setCurrent(compositeOp);
    m_wdgLayerBox->cmbComposite->blockSignals(false);
}

void KisLayerBox::slotFillCompositeOps(const KoColorSpace * colorSpace)
{
    m_wdgLayerBox->cmbComposite->blockSignals(true);
    m_wdgLayerBox->cmbComposite->setCompositeOpList(colorSpace->compositeOps());
    m_wdgLayerBox->cmbComposite->blockSignals(false);
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
        m_wdgLayerBox->listLayers->addPropertyActions(&menu, index);
        menu.addAction(KIcon("document-properties"), i18n("&Properties..."), this, SLOT(slotPropertiesClicked()));
        menu.addSeparator();
        menu.addAction(KIcon("edit-delete"), i18n("&Remove Layer"), this, SLOT(slotRmClicked()));
        menu.addAction(KIcon("edit-duplicate"), i18n("&Duplicate Layer or Mask"), this, SLOT(slotDuplicateClicked()));
        menu.addSeparator();

    }
    menu.addAction(KIcon("document-new"), i18n("&Layer"), this, SLOT(slotNewPaintLayer()));
    menu.addAction(KIcon("folder-new"), i18n("&Group Layer"), this, SLOT(slotNewGroupLayer()));
    menu.addAction(KIcon("edit-copy"), i18n("&Clone Layer"), this, SLOT(slotNewCloneLayer()));
    menu.addAction(KIcon("bookmark-new"), i18n("&Shape Layer"), this, SLOT(slotNewShapeLayer()));
    menu.addAction(KIcon("view-filter"), i18n("&Filter Layer..."), this, SLOT(slotNewAdjustmentLayer()));
    menu.addAction(KIcon("view-filter"), i18n("&Generated Layer..."), this, SLOT(slotNewGeneratorLayer()));
    menu.addSeparator();
    menu.addAction(KIcon("edit-copy"), i18n("&Transparency Mask"), this, SLOT(slotNewTransparencyMask()));
    menu.addAction(KIcon("bookmarks"), i18n("&Effect Mask..."), this, SLOT(slotNewEffectMask()));
    //    menu.addAction(KIcon("view-filter"), i18n("&Transformation Mask..."), this, SLOT(slotNewTransformationMask()));
    menu.addAction(KIcon("edit-paste"), i18n("&Local Selection"), this, SLOT(slotNewSelectionMask()));

    menu.exec(pos);
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
    m_nodeManager->createNode("KisPaintLayer");
}

void KisLayerBox::slotNewGroupLayer()
{
    m_nodeManager->createNode("KisGroupLayer");
}

void KisLayerBox::slotNewCloneLayer()
{
    m_nodeManager->createNode("KisCloneLayer");
}


void KisLayerBox::slotNewShapeLayer()
{
    m_nodeManager->createNode("KisShapeLayer");
}


void KisLayerBox::slotNewAdjustmentLayer()
{
    m_nodeManager->createNode("KisAdjustmentLayer");
}

void KisLayerBox::slotNewGeneratorLayer()
{
    m_nodeManager->createNode("KisGeneratorLayer");
}

void KisLayerBox::slotNewTransparencyMask()
{
    m_nodeManager->createNode("KisTransparencyMask");
}

void KisLayerBox::slotNewEffectMask()
{
    m_nodeManager->createNode("KisFilterMask");
}


void KisLayerBox::slotNewTransformationMask()
{
    m_nodeManager->createNode("KisTransformationMask");
}


void KisLayerBox::slotNewSelectionMask()
{
    m_nodeManager->createNode("KisSelectionMask");
}


void KisLayerBox::slotRmClicked()
{
    QModelIndexList l = m_wdgLayerBox->listLayers->selectionModel()->selectedIndexes();
    if (l.count() < 2 && m_nodeManager->activeNode() && !l.contains(m_wdgLayerBox->listLayers->currentIndex())) {
        l.clear();
        l.append(m_wdgLayerBox->listLayers->currentIndex());
    }

    for (int i = 0, n = l.count(); i < n; ++i) {
        KisNodeSP node = m_nodeModel->nodeFromIndex(l.at(i));
        m_nodeManager->removeNode(node);
    }
    m_nodeManager->updateGUI();
}

void KisLayerBox::slotRaiseClicked()
{
    m_nodeManager->raiseNode();
}

void KisLayerBox::slotLowerClicked()
{
    m_nodeManager->lowerNode();
}

void KisLayerBox::slotPropertiesClicked()
{
    if (KisNodeSP active = m_nodeManager->activeNode()) {
        m_nodeManager->nodeProperties(active);
    }
}

void KisLayerBox::slotDuplicateClicked()
{
        m_nodeManager->duplicateActiveNode();
}

void KisLayerBox::slotNodeActivated(const QModelIndex & node)
{
    m_nodeManager->activateNode(m_nodeModel->nodeFromIndex(node));
}


void KisLayerBox::slotCompositeOpChanged(const QString& _compositeOp)
{
    m_nodeManager->nodeCompositeOpChanged(m_nodeManager->activeColorSpace()->compositeOp(_compositeOp));
}

void KisLayerBox::slotOpacityChanged(qreal opacity, bool final)
{
    m_nodeManager->nodeOpacityChanged(opacity, final);
}

#include "kis_layer_box.moc"
