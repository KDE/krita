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

#include <QtDebug>
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

#include <kdebug.h>
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

#include "kis_cmb_composite.h"
#include "kis_view2.h"
#include "kis_node_manager.h"
#include "kis_node_model.h"

KisLayerBox::KisLayerBox()
    : QDockWidget( i18n("Layers" ) )
    , Ui::WdgLayerBox()
    , m_image( 0 )
    , m_nodeManager( 0 )
{
    setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);

    QWidget* mainWidget = new QWidget(this);
    setWidget(mainWidget);

    setupUi(mainWidget);

    setMinimumSize(mainWidget->minimumSizeHint());

    listLayers->viewport()->installEventFilter(this);
    connect(listLayers, SIGNAL(contextMenuRequested(const QPoint&, const QModelIndex&)),
            this, SLOT(slotContextMenuRequested(const QPoint&, const QModelIndex&)));
    connect(listLayers, SIGNAL(clicked(const QModelIndex&)), SLOT(slotNodeActivated(const QModelIndex&)));
    m_viewModeMenu = new KMenu( this );
    QActionGroup *group = new QActionGroup( this );
    QList<QAction*> actions;

    actions << m_viewModeMenu->addAction(KIcon("fileview-text"),
               i18n("Minimal View"), this, SLOT(slotMinimalView()));
    actions << m_viewModeMenu->addAction(KIcon("fileview-detailed"),
               i18n("Detailed View"), this, SLOT(slotDetailedView()));
    actions << m_viewModeMenu->addAction(KIcon("view_icon"),
               i18n("Thumbnail View"), this, SLOT(slotThumbnailView()));

    for( int i = 0, n = actions.count(); i < n; ++i )
    {
        actions[i]->setCheckable( true );
        actions[i]->setActionGroup( group );
    }
    actions[1]->trigger(); //TODO save/load previous state

    bnViewMode->setMenu(m_viewModeMenu);
    bnViewMode->setPopupMode(QToolButton::InstantPopup);
    bnViewMode->setIcon(KIcon("view_choose"));
    bnViewMode->setText(i18n("View mode"));

    bnAdd->setToolTip(i18n("Create new layer"));

    bnDelete->setToolTip(i18n("Remove current layer"));

    bnRaise->setToolTip(i18n("Raise current layer"));
    bnRaise->setEnabled(false);

    bnLower->setEnabled(false);
    bnLower->setToolTip(i18n("Lower current layer"));

    bnProperties->setToolTip(i18n("Properties for layer"));

    m_newLayerMenu = new KMenu(this);
    bnAdd->setMenu(m_newLayerMenu);
    bnAdd->setPopupMode(QToolButton::MenuButtonPopup);
    connect( bnAdd, SIGNAL(clicked()), SLOT(slotNewPaintLayer()) );
    m_newLayerMenu->addAction(KIcon("document-new"), i18n("&New Layer"), this, SLOT(slotNewPaintLayer()));
    m_newLayerMenu->addAction(KIcon("folder"), i18n("New &Group Layer"), this, SLOT(slotNewGroupLayer()));
    m_newLayerMenu->addAction(KIcon("edit-copy"), i18n("New &Clone Layer"), this, SLOT(slotNewCloneLayer()));
    m_newLayerMenu->addAction(KIcon("bookmark"), i18n("New &Shape Layer"), this, SLOT(slotNewShapeLayer()));
    m_newLayerMenu->addAction(KIcon("tool_filter"), i18n("New &Adjustment Layer..."), this, SLOT(slotNewAdjustmentLayer()));
    m_newLayerMenu->addSeparator();
    m_newLayerMenu->addAction(KIcon("edit-copy"), i18n("&Transparency Mask"), this, SLOT(slotNewTransparencyMask()));
    m_newLayerMenu->addAction(KIcon("bookmark"), i18n("&Effect Mask..."), this, SLOT(slotNewEffectMask()));
    m_newLayerMenu->addAction(KIcon("tool_filter"), i18n("&Transformation Mask..."), this, SLOT(slotNewTransformationMask()));

    connect(bnDelete, SIGNAL(clicked()), SLOT(slotRmClicked()));
    connect(bnRaise, SIGNAL(clicked()), SLOT(slotRaiseClicked()));
    connect(bnLower, SIGNAL(clicked()), SLOT(slotLowerClicked()));
    connect(bnProperties, SIGNAL(clicked()), SLOT(slotPropertiesClicked()));
    connect(doubleOpacity, SIGNAL(valueChanged(double, bool)), SIGNAL(sigOpacityChanged(double, bool)));
    connect(cmbComposite, SIGNAL(activated(const KoCompositeOp*)), SIGNAL(sigItemComposite(const KoCompositeOp*)));
}

KisLayerBox::~KisLayerBox()
{
}

void KisLayerBox::setImage(KisNodeManager * nodeManager, KisImageSP img, KisNodeModel * nodeModel)
{

    if (m_image == img)
        return;

    m_nodeModel = nodeModel;
    m_nodeManager = nodeManager;

    if (m_image)
        m_image->disconnect(this);

    if (img) {

        connect(m_nodeModel, SIGNAL(nodeActivated(KisNodeSP)), this, SLOT(updateUI()));
        connect(m_nodeModel, SIGNAL(rowsInserted(const QModelIndex &, int, int)), this, SLOT(updateUI()));
        connect(m_nodeModel, SIGNAL(rowsRemoved(const QModelIndex &, int, int)), this, SLOT(updateUI()));
        connect(m_nodeModel, SIGNAL(modelReset()), this, SLOT(updateUI()));

        listLayers->setModel( nodeModel );

        m_image = img;

        updateUI();
    }
    else {
        listLayers->setModel(0);
    }
}

bool KisLayerBox::eventFilter(QObject *o, QEvent *e)
{
    Q_ASSERT(o == listLayers->viewport());

    if (e->type() == QEvent::MouseButtonDblClick) {

        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        QModelIndex mi = listLayers->indexAt(me->pos());
        if (mi.isValid())
            slotPropertiesClicked();
        else
            slotNewPaintLayer();
        return true;
    }

    return QDockWidget::eventFilter(o, e);
}

void KisLayerBox::updateUI()
{
    Q_ASSERT(! m_image.isNull());
    kDebug(41007)  <<"###### KisLayerBox::updateUI" << m_nodeManager->activeNode();

    bnDelete->setEnabled(m_nodeManager->activeNode());
    bnRaise->setEnabled(m_nodeManager->activeNode() && (m_nodeManager->activeNode()->prevSibling() || m_nodeManager->activeNode()->parent()));
    bnLower->setEnabled(m_nodeManager->activeNode() && m_nodeManager->activeNode()->nextSibling());
    doubleOpacity->setEnabled(m_nodeManager->activeNode());
    cmbComposite->setEnabled(m_nodeManager->activeNode());
    if (KisNodeSP active = m_nodeManager->activeNode())
    {
        if (m_nodeManager->activePaintDevice())
            slotSetColorSpace(m_nodeManager->activePaintDevice()->colorSpace());
        else
            slotSetColorSpace(m_image->colorSpace());
#if 0 // XXX_NODE
        slotSetOpacity(active->opacity() * 100.0 / 255);
        slotSetCompositeOp(active->compositeOp());
#endif
    }
}

void KisLayerBox::setCurrentNode( KisNodeSP node )
{
    if ( node && m_nodeModel ) {
        listLayers->setCurrentIndex( m_nodeModel->indexFromNode( node ) );
    }

}

void KisLayerBox::slotSetCompositeOp(const KoCompositeOp* compositeOp)
{
    cmbComposite->blockSignals(true);
    cmbComposite->setCurrent(compositeOp);
    cmbComposite->blockSignals(false);
}

void KisLayerBox::slotSetColorSpace(const KoColorSpace * colorSpace)
{
    cmbComposite->blockSignals(true);
    cmbComposite->setCompositeOpList(colorSpace->userVisiblecompositeOps());
    cmbComposite->blockSignals(false);
}

// range: 0-100
void KisLayerBox::slotSetOpacity(double opacity)
{
    Q_ASSERT( opacity >= 0 && opacity <= 100 );
    doubleOpacity->blockSignals(true);
    doubleOpacity->setValue(opacity);
    doubleOpacity->blockSignals(false);
}

void KisLayerBox::slotContextMenuRequested(const QPoint &pos, const QModelIndex &index)
{
    QMenu menu;
    if (index.isValid())
    {
        listLayers->addPropertyActions(&menu, index);
        menu.addAction(KIcon("document-properties"), i18n("&Properties..."), this, SLOT(slotPropertiesClicked()));
        menu.addSeparator();
        menu.addAction(KIcon("edit-delete"), i18n("&Remove Layer"), this, SLOT(slotRmClicked()));
        QMenu *sub = menu.addMenu(KIcon("document-new"), i18n("&New"));

        sub->addAction(KIcon("document-new"), i18n("&Paint Layer"), this, SLOT(slotNewPaintLayer()));
        sub->addAction(KIcon("folder"), i18n("&Group Layer"), this, SLOT(slotNewGroupLayer()));
        sub->addAction(KIcon("edit-copy"), i18n("&Clone Layer"), this, SLOT(slotNewCloneLayer()));
        sub->addAction(KIcon("bookmark"), i18n("&Shape Layer"), this, SLOT(slotNewShapeLayer()));
        sub->addAction(KIcon("tool_filter"), i18n("&Adjustment Layer..."), this, SLOT(slotNewAdjustmentLayer()));
        menu.addSeparator();
        sub->addAction(KIcon("edit-copy"), i18n("&Transparency Mask"), this, SLOT(slotNewTransparencyMask()));
        sub->addAction(KIcon("bookmark"), i18n("&Effect Mask..."), this, SLOT(slotNewEffectMask()));
        sub->addAction(KIcon("tool_filter"), i18n("&Transformation Mask..."), this, SLOT(slotNewTransformationMask()));

    }
    else
    {
        menu.addAction(KIcon("document-new"), i18n("&New Layer"), this, SLOT(slotNewPaintLayer()));
        menu.addAction(KIcon("folder"), i18n("New &Group Layer"), this, SLOT(slotNewGroupLayer()));
        menu.addAction(KIcon("edit-copy"), i18n("New &Clone Layer"), this, SLOT(slotNewCloneLayer()));
        menu.addAction(KIcon("bookmark"), i18n("New &Shape Layer"), this, SLOT(slotNewShapeLayer()));
        menu.addAction(KIcon("tool_filter"), i18n("New &Adjustment Layer..."), this, SLOT(slotNewAdjustmentLayer()));
        menu.addSeparator();
        menu.addAction(KIcon("edit-copy"), i18n("&Transparency Mask"), this, SLOT(slotNewTransparencyMask()));
        menu.addAction(KIcon("bookmark"), i18n("&Effect Mask..."), this, SLOT(slotNewEffectMask()));
        menu.addAction(KIcon("tool_filter"), i18n("&Transformation Mask..."), this, SLOT(slotNewTransformationMask()));
    }
    menu.exec(pos);
}

void KisLayerBox::slotMinimalView()
{
    listLayers->setDisplayMode(KoDocumentSectionView::MinimalMode);
}

void KisLayerBox::slotDetailedView()
{
    listLayers->setDisplayMode(KoDocumentSectionView::DetailedMode);
}

void KisLayerBox::slotThumbnailView()
{
    listLayers->setDisplayMode(KoDocumentSectionView::ThumbnailMode);
}

bool allowAsChild( const QString & parentType, const QString & childType )
{
    // XXX_NODE: do we want to allow masks to apply on masks etc? Selections on masks?
    if ( parentType == "KisPaintLayer" || parentType == "KisGroupLayer" || parentType == "KisAdjustmentLayer" || parentType == "KisShapeLayer" ) {
        if (childType == "KisFilterMask" || childType == "KisTransformationMask" || childType == "KisTransparencyMask" || childType == "KisSelectionMask") {
            return true;
        }
        return false;
    }
    else if ( parentType == "KisGroupLayer" ) {
        return true;
    }
    else if ( parentType == "KisFilterMask" || parentType == "KisTransformationMask" || parentType == "KisTransparencyMask" || parentType == "KisSelectionMask")
    {
        return false;
    }

    return true;
}

void KisLayerBox::getNewNodeLocation(const QString & nodeType, KisNodeSP &parent, KisNodeSP &above)
{
    KisNodeSP root = m_image->root();
    KisNodeSP active =  m_nodeManager->activeNode();

    if (!active)
        active = root->firstChild();
    // Find the first node above the current node that can have the desired
    // layer type as child. XXX_NODE: disable the menu entries for node types
    // that are not compatible with the active node type.
    while (active) {
        if ( allowAsChild(active->metaObject()->className(), nodeType) ) {
            parent = active;
            if ( m_nodeManager->activeNode()->parent() == parent ) {
                above = m_nodeManager->activeNode();
            }
            else {
                above = parent->firstChild();
            }
            return;
        }
        active = active->parent();
    }
    parent = root;
    above = parent->firstChild();
    kDebug() << parent << ", above " << above;
}

void KisLayerBox::slotNewPaintLayer()
{
    KisNodeSP parent;
    KisNodeSP above;

    getNewNodeLocation("KisPaintLayer", parent, above);

    emit sigRequestNewNode("KisPaintLayer", parent, above);
}

void KisLayerBox::slotNewGroupLayer()
{
    KisNodeSP parent;
    KisNodeSP above;

    getNewNodeLocation("KisGroupLayer", parent, above);

    emit sigRequestNewNode("KisGroupLayer", parent, above);
}

void KisLayerBox::slotNewCloneLayer()
{
    KisNodeSP parent;
    KisNodeSP above;

    getNewNodeLocation("KisCloneLayer", parent, above);

    emit sigRequestNewNode( "KisCloneLayer", parent, above );
}


void KisLayerBox::slotNewShapeLayer()
{
    KisNodeSP parent;
    KisNodeSP above;

    getNewNodeLocation("KisShapeLayer", parent, above);

    emit sigRequestNewNode("KisShapeLayer", parent, above );
}


void KisLayerBox::slotNewAdjustmentLayer()
{
    KisNodeSP parent;
    KisNodeSP above;

    getNewNodeLocation("KisAdjustmentLayer", parent, above);

    emit sigRequestNewNode("KisAdjustmentLayer", parent, above);
}

void KisLayerBox::slotNewTransparencyMask()
{
    KisNodeSP parent;
    KisNodeSP above;

    getNewNodeLocation("KisTransparencyMask", parent, above);

    emit sigRequestNewNode("KisTransparencyMask", parent, above);
}

void KisLayerBox::slotNewEffectMask()
{
    KisNodeSP parent;
    KisNodeSP above;

    getNewNodeLocation("KisFilterMask", parent, above);

    emit sigRequestNewNode("KisFilterMask", parent, above);
}


void KisLayerBox::slotNewTransformationMask()
{
    KisNodeSP parent;
    KisNodeSP above;

    getNewNodeLocation("KisTransformationMask", parent, above);

    emit sigRequestNewNode("KisTransformationMask", parent, above);
}


void KisLayerBox::slotRmClicked()
{
    QModelIndexList l = selectedNodes();

    for (int i = 0, n = l.count(); i < n; ++i)
        m_image->removeNode(m_nodeModel->nodeFromIndex(l.at(i)));
}

void KisLayerBox::slotRaiseClicked()
{
    QModelIndexList l = selectedNodes();

    KisNodeSP layer = m_nodeModel->nodeFromIndex(l.first());
    if( l.count() == 1 && layer == layer->parent()->firstChild() && layer->parent() != m_image->root())
    {
        if (KisGroupLayerSP grandparent = dynamic_cast<KisGroupLayer* >( layer->parent()->parent().data() ) )
            m_image->moveNode(layer, grandparent, layer->parent());
    }
    else
    {
        for (int i = 0, n = l.count(); i < n; ++i)
            if (KisNodeSP li = m_nodeModel->nodeFromIndex(l[i]))
                if (li->prevSibling())
                    m_image->moveNode(li, li->parent(), li->prevSibling());
    }

    if( !l.isEmpty() )
        listLayers->scrollTo( l.first() );
}

void KisLayerBox::slotLowerClicked()
{
    QModelIndexList l = selectedNodes();

    for (int i = l.count() - 1; i >= 0; --i)
        if (KisNodeSP layer = m_nodeModel->nodeFromIndex(l[i]))
            if (layer->nextSibling())
            {
                if (layer->nextSibling()->nextSibling())
                    m_image->moveNode(layer, layer->parent(), layer->nextSibling()->nextSibling());
                else
                    m_image->moveNode(layer, layer->parent(), KisLayerSP(0));
            }

    if( !l.isEmpty() )
        listLayers->scrollTo( l.last() );
}

void KisLayerBox::slotPropertiesClicked()
{
    if (KisNodeSP active = m_nodeManager->activeNode())
        emit sigRequestNodeProperties(active);
}

void KisLayerBox::slotNodeActivated(const QModelIndex & node)
{
    kDebug() << "slotNodeActivated " << node;
    m_nodeManager->activateNode( m_nodeModel->nodeFromIndex( node ) );
}

void KisLayerBox::setUpdatesAndSignalsEnabled(bool enable)
{
    setUpdatesEnabled(enable);
    //doubleOpacity->setUpdatesEnabled(enable);
    cmbComposite->setUpdatesEnabled(enable);

    doubleOpacity->blockSignals(!enable);
    cmbComposite->blockSignals(!enable);
}

QModelIndexList KisLayerBox::selectedNodes() const
{
    QModelIndexList l = listLayers->selectionModel()->selectedIndexes();
    if (l.count() < 2 && m_nodeManager->activeNode() && !l.contains(listLayers->currentIndex()))
    {
        l.clear();
        l.append(listLayers->currentIndex());
    }
    return l;
}

#include "kis_layer_box.moc"
