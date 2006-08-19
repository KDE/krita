/*
 *  kis_layerbox.cc - part of Krita aka Krayon aka KimageShop
 *
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (C) 2006 Gábor Lehel <illissius@gmail.com>
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

#include <QtDebug>
#include <QToolButton>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>
#include <QLayout>
#include <QMouseEvent>
#include <QPainter>
#include <QPoint>
#include <QRect>
#include <QString>
#include <QStyle>
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
#include <k3popupmenu.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <klocale.h>
#include <khbox.h>
#include <kicon.h> 
#include <KoDocumentSectionView.h>
#include <KoPartSelectAction.h>

#include "kis_cmb_composite.h"
#include "kis_int_spinbox.h"
#include "KoColorSpace.h"
#include "kis_paint_device.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_image.h"

#include "kis_layerbox.h"

KisLayerBox::KisLayerBox(QWidget *parent, const char *name)
    : super(parent), m_image(0)
{
    setObjectName(name);
    QVBoxLayout *vbox = new QVBoxLayout(this);

    m_lst = new WdgLayerBox(this);
    setMinimumSize(m_lst->minimumSizeHint());
    vbox->addWidget(m_lst);

    m_lst->listLayers->viewport()->installEventFilter(this);
    connect(m_lst->listLayers, SIGNAL(contextMenuRequested(const QPoint&, const QModelIndex&)),
            this, SLOT(slotContextMenuRequested(const QPoint&, const QModelIndex&)));

    m_viewModeMenu = new KMenu( this );
    QActionGroup *group = new QActionGroup( this );
    QList<QAction*> actions;
    actions << m_viewModeMenu->addAction(KIcon("view_text"),
               i18n("Minimal View"), this, SLOT(slotMinimalView()));
    actions << m_viewModeMenu->addAction(KIcon("view_detailed"),
               i18n("Detailed View"), this, SLOT(slotDetailedView()));
    actions << m_viewModeMenu->addAction(KIcon("view_icon"),
               i18n("Thumbnail View"), this, SLOT(slotThumbnailView()));
    for( int i = 0, n = actions.count(); i < n; ++i )
    {
        actions[i]->setCheckable( true );
        actions[i]->setActionGroup( group );
    }
    actions[1]->trigger(); //TODO save/load previous state

    m_lst->bnViewMode->setMenu(m_viewModeMenu);
    m_lst->bnViewMode->setPopupMode(QToolButton::InstantPopup);
    m_lst->bnViewMode->setIcon(KIcon("view_choose"));
    m_lst->bnViewMode->setText(i18n("View mode"));

    m_lst->bnAdd->setToolTip(i18n("Create new layer"));

    m_lst->bnDelete->setToolTip(i18n("Remove current layer"));

    m_lst->bnRaise->setToolTip(i18n("Raise current layer"));
    m_lst->bnRaise->setEnabled(false);

    m_lst->bnLower->setEnabled(false);
    m_lst->bnLower->setToolTip(i18n("Lower current layer"));

    m_lst->bnProperties->setToolTip(i18n("Properties for layer"));

    m_newLayerMenu = new KMenu(this);
    m_lst->bnAdd->setMenu(m_newLayerMenu);
    m_lst->bnAdd->setPopupMode(QToolButton::InstantPopup);

    m_newLayerMenu->addAction(SmallIconSet("filenew"), i18n("&New Layer..."), this, SLOT(slotNewLayer()));
    m_newLayerMenu->addAction(SmallIconSet("folder"), i18n("New &Group Layer..."), this, SLOT(slotNewGroupLayer()));
    m_newLayerMenu->addAction(SmallIconSet("tool_filter"), i18n("New &Adjustment Layer..."), this, SLOT(slotNewAdjustmentLayer()));

    m_partLayerAction = new KoPartSelectAction( i18n("New &Object Layer"), "gear" /*, this - KDE4*/);
    m_newLayerMenu->addAction(m_partLayerAction);
    connect(m_partLayerAction, SIGNAL(triggered()), this, SLOT(slotNewPartLayer()));

    connect(m_lst->bnDelete, SIGNAL(clicked()), SLOT(slotRmClicked()));
    connect(m_lst->bnRaise, SIGNAL(clicked()), SLOT(slotRaiseClicked()));
    connect(m_lst->bnLower, SIGNAL(clicked()), SLOT(slotLowerClicked()));
    connect(m_lst->bnProperties, SIGNAL(clicked()), SLOT(slotPropertiesClicked()));
    connect(m_lst->intOpacity, SIGNAL(valueChanged(int, bool)), SIGNAL(sigOpacityChanged(int, bool)));
    connect(m_lst->intOpacity, SIGNAL(finishedChanging(int, int)), SIGNAL(sigOpacityFinishedChanging(int, int)));
    connect(m_lst->cmbComposite, SIGNAL(activated(const KoCompositeOp&)), SIGNAL(sigItemComposite(const KoCompositeOp&)));
}

KisLayerBox::~KisLayerBox()
{
}

void KisLayerBox::setImage(KisImageSP img)
{
    if (m_image == img)
        return;

    if (m_image)
        m_image->disconnect(this);

    m_image = img;

    if (img)
    {

        connect(img.data(), SIGNAL(sigLayerActivated(KisLayerSP)), this, SLOT(updateUI()));
        connect(img.data(), SIGNAL(sigLayerAdded(KisLayerSP)), this, SLOT(updateUI()));
        connect(img.data(), SIGNAL(sigLayerRemoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)),
                this, SLOT(updateUI()));
        connect(img.data(), SIGNAL(sigLayerPropertiesChanged(KisLayerSP)),
                this, SLOT(updateUI()));
        connect(img.data(), SIGNAL(sigLayerMoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)),
                this, SLOT(updateUI()));
        connect(img.data(), SIGNAL(sigLayersChanged(KisGroupLayerSP)), this, SLOT(updateUI()));
        m_lst->listLayers->setModel(m_image->rootLayer().data());
    }
    else
    {
        m_lst->listLayers->setModel(0);
    }
}

bool KisLayerBox::eventFilter(QObject *o, QEvent *e)
{
    Q_ASSERT(o == m_lst->listLayers->viewport());

    if (e->type() == QEvent::MouseButtonDblClick)
    {
        QMouseEvent *me = static_cast<QMouseEvent*>(e);
        QModelIndex mi = m_lst->listLayers->indexAt(me->pos());
        if (mi.isValid())
            slotPropertiesClicked();
        else
            slotNewLayer();
        return true;
    }

    return super::eventFilter(o, e);
}

void KisLayerBox::updateUI()
{
    m_lst->bnDelete->setEnabled(m_image->activeLayer());
    m_lst->bnRaise->setEnabled(m_image->activeLayer() && (m_image->activeLayer()->prevSibling() || m_image->activeLayer()->parent()));
    m_lst->bnLower->setEnabled(m_image->activeLayer() && m_image->activeLayer()->nextSibling());
    m_lst->intOpacity->setEnabled(m_image->activeLayer());
    m_lst->cmbComposite->setEnabled(m_image->activeLayer());
    if (m_image)
        if (KisLayerSP active = m_image->activeLayer())
        {
            if (m_image->activeDevice())
                slotSetColorSpace(m_image->activeDevice()->colorSpace());
            else
                slotSetColorSpace(m_image->colorSpace());
            slotSetOpacity(int(float(active->opacity() * 100) / 255 + 0.5));
            slotSetCompositeOp(active->compositeOp());
        }
}

void KisLayerBox::slotSetCompositeOp(const KoCompositeOp& compositeOp)
{
    m_lst->cmbComposite->blockSignals(true);
    m_lst->cmbComposite->setCurrent(compositeOp);
    m_lst->cmbComposite->blockSignals(false);
}

void KisLayerBox::slotSetColorSpace(const KoColorSpace * colorSpace)
{
    m_lst->cmbComposite->blockSignals(true);
    m_lst->cmbComposite->setCompositeOpList(colorSpace->userVisiblecompositeOps());
    m_lst->cmbComposite->blockSignals(false);
}

// range: 0-100
void KisLayerBox::slotSetOpacity(int opacity)
{
    m_lst->intOpacity->blockSignals(true);
    m_lst->intOpacity->setValue(opacity);
    m_lst->intOpacity->blockSignals(false);
}

void KisLayerBox::slotContextMenuRequested(const QPoint &pos, const QModelIndex &index)
{
    QMenu menu;
    if (index.isValid())
    {
        m_lst->listLayers->addPropertyActions(&menu, index);
        menu.addAction(KIcon("info"), i18n("&Properties..."), this, SLOT(slotPropertiesClicked()));
        menu.addSeparator();
        menu.addAction(KIcon("editdelete"), i18n("&Remove Layer"), this, SLOT(slotRmClicked()));
        QMenu *sub = menu.addMenu(KIcon("filenew"), i18n("&New"));
        sub->addAction(KIcon("file"), i18n("&Layer..."), this, SLOT(slotNewLayer()));
        sub->addAction(KIcon("folder"), i18n("&Group Layer..."), this, SLOT(slotNewGroupLayer()));
        sub->addAction(KIcon("tool_filter"), i18n("&Adjustment Layer..."), this, SLOT(slotNewAdjustmentLayer()));
        sub->addAction(m_partLayerAction);
        m_partLayerAction->setText(i18n("&Object Layer..."));
    }
    else
    {
        menu.addAction(KIcon("filenew"), i18n("&New Layer..."), this, SLOT(slotNewLayer()));
        menu.addAction(KIcon("folder"), i18n("New &Group Layer..."), this, SLOT(slotNewGroupLayer()));
        menu.addAction(KIcon("tool_filter"), i18n("New &Adjustment Layer..."), this, SLOT(slotNewAdjustmentLayer()));
        menu.addAction(m_partLayerAction);
    }
    menu.exec(pos);
    m_partLayerAction->setText(i18n("New &Object Layer..."));
}

void KisLayerBox::slotMinimalView()
{
    m_lst->listLayers->setDisplayMode(KoDocumentSectionView::MinimalMode);
}

void KisLayerBox::slotDetailedView()
{
    m_lst->listLayers->setDisplayMode(KoDocumentSectionView::DetailedMode);
}

void KisLayerBox::slotThumbnailView()
{
    m_lst->listLayers->setDisplayMode(KoDocumentSectionView::ThumbnailMode);
}

void KisLayerBox::getNewLayerLocation(KisGroupLayerSP &parent, KisLayerSP &above)
{
    KisGroupLayerSP root = m_image->rootLayer();
    if (KisLayerSP active = m_image->activeLayer())
    {
        if (KisGroupLayer* pactive = qobject_cast<KisGroupLayer*>(active.data()))
        {
            parent = pactive;
            above = parent->firstChild();
        }
        else
        {
            parent = root;
            above = active;
            if (active->parent())
                parent = active->parent();
        }
    }
    else
    {
        parent = root;
        above = m_image->rootLayer()->firstChild();
    }
}

void KisLayerBox::slotNewLayer()
{
    KisGroupLayerSP parent;
    KisLayerSP above;

    getNewLayerLocation(parent, above);

    emit sigRequestLayer(parent, above);
}

void KisLayerBox::slotNewGroupLayer()
{
    KisGroupLayerSP parent;
    KisLayerSP above;

    getNewLayerLocation(parent, above);

    emit sigRequestGroupLayer(parent, above);
}

void KisLayerBox::slotNewAdjustmentLayer()
{
    KisGroupLayerSP parent;
    KisLayerSP above;

    getNewLayerLocation(parent, above);

    emit sigRequestAdjustmentLayer(parent, above);
}

void KisLayerBox::slotNewPartLayer()
{
    KisGroupLayerSP parent;
    KisLayerSP above;

    getNewLayerLocation(parent, above);

    emit sigRequestPartLayer(parent, above, m_partLayerAction->documentEntry());
}

void KisLayerBox::slotRmClicked()
{
    QModelIndexList l = selectedLayers();

    for (int i = 0, n = l.count(); i < n; ++i)
        m_image->removeLayer(m_image->rootLayer()->layerFromIndex(l.at(i)));
}

void KisLayerBox::slotRaiseClicked()
{
    QModelIndexList l = selectedLayers();

    KisLayerSP layer = m_image->rootLayer()->layerFromIndex(l.first());
    if( l.count() == 1 && layer == layer->parent()->firstChild() && layer->parent() != m_image->rootLayer())
    {
        if (KisGroupLayerSP grandparent = layer->parent()->parent())
            m_image->moveLayer(layer, grandparent, KisLayerSP(layer->parent().data()));
    }
    else
    {
        for (int i = 0, n = l.count(); i < n; ++i)
            if (KisLayerSP li = m_image->rootLayer()->layerFromIndex(l[i]))
                if (li->prevSibling())
                    m_image->moveLayer(li, li->parent(), li->prevSibling());
    }

    if( !l.isEmpty() )
        m_lst->listLayers->scrollTo( l.first() );
}

void KisLayerBox::slotLowerClicked()
{
    QModelIndexList l = selectedLayers();

    for (int i = l.count() - 1; i >= 0; --i)
        if (KisLayerSP layer = m_image->rootLayer()->layerFromIndex(l[i]))
            if (layer->nextSibling())
            {
                if (layer->nextSibling()->nextSibling())
                    m_image->moveLayer(layer, layer->parent(), layer->nextSibling()->nextSibling());
                else
                    m_image->moveLayer(layer, layer->parent(), KisLayerSP(0));
            }

    if( !l.isEmpty() )
        m_lst->listLayers->scrollTo( l.last() );
}

void KisLayerBox::slotPropertiesClicked()
{
    if (KisLayerSP active = m_image->activeLayer())
        emit sigRequestLayerProperties(active);
}

void KisLayerBox::setUpdatesAndSignalsEnabled(bool enable)
{
    setUpdatesEnabled(enable);
    m_lst->intOpacity->setUpdatesEnabled(enable);
    m_lst->cmbComposite->setUpdatesEnabled(enable);

    m_lst->intOpacity->blockSignals(!enable);
    m_lst->cmbComposite->blockSignals(!enable);
}

QModelIndexList KisLayerBox::selectedLayers() const
{
    QModelIndexList l = m_lst->listLayers->selectionModel()->selectedIndexes();
    if (l.count() < 2 && m_image->activeLayer() && !l.contains(m_lst->listLayers->currentIndex()))
    {
        l.clear();
        l.append(m_lst->listLayers->currentIndex());
    }
    return l;
}

#include "kis_layerbox.moc"
