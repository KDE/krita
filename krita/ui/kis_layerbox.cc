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

#include <QToolButton>
#include <QBrush>
#include <QFont>
#include <QFontMetrics>
#include <QLayout>
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

#include <KoPartSelectAction.h>

#include "kis_layerlist.h"
#include "kis_cmb_composite.h"
#include "kis_int_spinbox.h"
#include "KoColorSpace.h"
#include "kis_paint_device.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_image.h"

#include "kis_populate_visitor.h"

#include "kis_layerbox.h"

KisLayerBox::KisLayerBox(KisCanvasSubject *subject, QWidget *parent, const char *name)
    : super(parent), m_image(0)
{
    setObjectName(name);
    QVBoxLayout *vbox = new QVBoxLayout(this);

    m_lst = new WdgLayerBox(this);
    setMinimumSize(m_lst->minimumSizeHint());
    vbox->addWidget(m_lst);

    m_lst->bnAdd->setToolTip( i18n("Create new layer"));

    m_lst->bnDelete->setToolTip( i18n("Remove current layer"));

    m_lst->bnRaise->setToolTip( i18n("Raise current layer"));
    m_lst->bnRaise->setEnabled(false);

    m_lst->bnLower->setEnabled(false);
    m_lst->bnLower->setToolTip( i18n("Lower current layer"));

    m_lst->bnProperties->setToolTip( i18n("Properties for layer"));

    KIconLoader il( "krita" );

    list()->setPreviewsShown(true);

    list()->setFoldersCanBeActive(true);

    list()->addProperty("visible", i18n("Visible"), loadPixmap("visible.png", il, K3Icon::SizeSmallMedium),
                                                      loadPixmap("novisible.png", il, K3Icon::SizeSmallMedium), true);

    list()->addProperty("locked", i18n("Locked"), loadPixmap("locked.png", il, K3Icon::SizeSmallMedium),
                                                    loadPixmap("unlocked.png", il, K3Icon::SizeSmallMedium));


    connect(list()->contextMenu(), SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
    connect(list(), SIGNAL(activated(LayerItem*)),
                    SLOT(slotLayerActivated(LayerItem*)));
    connect(list(), SIGNAL(displayNameChanged(LayerItem*, const QString&)),
                    SLOT(slotLayerDisplayNameChanged(LayerItem*, const QString&)));
    connect(list(), SIGNAL(propertyChanged(LayerItem*, const QString&, bool)),
                    SLOT(slotLayerPropertyChanged(LayerItem*, const QString&, bool)));
    connect(list(), SIGNAL(layerMoved(LayerItem*, LayerItem*, LayerItem*)),
                    SLOT(slotLayerMoved(LayerItem*, LayerItem*, LayerItem*)));
    connect(list(), SIGNAL(requestNewLayer(LayerItem*, LayerItem*)),
                    SLOT(slotRequestNewLayer(LayerItem*, LayerItem*)));
    connect(list(), SIGNAL(requestNewFolder(LayerItem*, LayerItem*)),
                    SLOT(slotRequestNewFolder(LayerItem*, LayerItem*)));
    connect(list(), SIGNAL(requestNewAdjustmentLayer(LayerItem*, LayerItem*)),
                    SLOT(slotRequestNewAdjustmentLayer(LayerItem*, LayerItem*)));
    connect(list(), SIGNAL(requestNewObjectLayer(LayerItem*, LayerItem*, const KoDocumentEntry&)),
                    SLOT(slotRequestNewObjectLayer(LayerItem*, LayerItem*, const KoDocumentEntry&)));
    connect(list(), SIGNAL(requestRemoveLayer(LayerItem*)),
                    SLOT(slotRequestRemoveLayer(LayerItem*)));
    connect(list(), SIGNAL(requestLayerProperties(LayerItem*)),
                    SLOT(slotRequestLayerProperties(LayerItem*)));

    m_newLayerMenu = new KMenu(this);
    m_lst->bnAdd->setMenu(m_newLayerMenu);
    m_lst->bnAdd->setPopupMode(QToolButton::InstantPopup);

    m_newLayerMenu->addAction(SmallIconSet( "filenew" ), i18n( "&New Layer..." ), this, SLOT(slotNewLayer()));
    m_newLayerMenu->addAction(SmallIconSet( "folder" ), i18n( "New &Group Layer..." ), this, SLOT(slotNewGroupLayer()));
    m_newLayerMenu->addAction(SmallIconSet( "tool_filter" ), i18n( "New &Adjustment Layer..." ), this, SLOT(slotNewAdjustmentLayer()));

    m_partLayerAction = new KoPartSelectAction( i18n( "New &Object Layer" ), "gear" /*, this - KDE4*/);
    m_newLayerMenu->addAction(m_partLayerAction);
    connect(m_partLayerAction, SIGNAL(triggered()), this, SLOT(slotNewPartLayer()));

    connect(m_lst->bnDelete, SIGNAL(clicked()), SLOT(slotRmClicked()));
    connect(m_lst->bnRaise, SIGNAL(clicked()), SLOT(slotRaiseClicked()));
    connect(m_lst->bnLower, SIGNAL(clicked()), SLOT(slotLowerClicked()));
    connect(m_lst->bnProperties, SIGNAL(clicked()), SLOT(slotPropertiesClicked()));
    connect(m_lst->intOpacity, SIGNAL(valueChanged(int, bool)), SIGNAL(sigOpacityChanged(int, bool)));
    connect(m_lst->intOpacity, SIGNAL(finishedChanging(int, int)), SIGNAL(sigOpacityFinishedChanging(int, int)));
    connect(m_lst->cmbComposite, SIGNAL(activated(const KoCompositeOp&)), SIGNAL(sigItemComposite(const KoCompositeOp&)));

    Q_ASSERT(subject->document() != 0);

    if (subject->document()) {
        connect(subject->document(), SIGNAL(sigCommandExecuted()), SLOT(updateThumbnails()));
    }
}

KisLayerBox::~KisLayerBox()
{
}

KisLayerList* KisLayerBox::list() const
{
    return m_lst->listLayers;
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
        connect(img.data(), SIGNAL(sigLayerActivated(KisLayerSP)), this, SLOT(slotLayerActivated(KisLayerSP)));
        connect(img.data(), SIGNAL(sigLayerAdded(KisLayerSP)), this, SLOT(slotLayerAdded(KisLayerSP)));
        connect(img.data(), SIGNAL(sigLayerRemoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)),
                this, SLOT(slotLayerRemoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)));
        connect(img.data(), SIGNAL(sigLayerPropertiesChanged(KisLayerSP)),
                this, SLOT(slotLayerPropertiesChanged(KisLayerSP)));
        connect(img.data(), SIGNAL(sigLayerMoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)),
                this, SLOT(slotLayerMoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)));
        connect(img.data(), SIGNAL(sigLayersChanged(KisGroupLayerSP)), this, SLOT(slotLayersChanged(KisGroupLayerSP)));
        connect(img.data(), SIGNAL(sigLayerUpdated(KisLayerSP, QRect)), this, SLOT(slotLayerUpdated(KisLayerSP, QRect)));
        slotLayersChanged(img->rootLayer());
        updateThumbnails();
    }
    else
    {
        clear();
    }
}

void KisLayerBox::slotLayerActivated(KisLayerSP layer)
{
    if (layer)
        list()->setActiveLayer(layer->id());
    else
        list()->setActiveLayer(-1);
    updateUI();
}

void KisLayerBox::slotLayerAdded(KisLayerSP layer)
{
    if (layer.data() == m_image->rootLayer().data() || list()->layer(layer->id()))
        return;

    vKisLayerSP layersAdded;

    if (layer->parent() == m_image->rootLayer())
    {
        KisPopulateVisitor visitor(list());
        layer->accept(visitor);
        layersAdded = visitor.layersAdded();
    }
    else
    {
        KisPopulateVisitor visitor(static_cast<KisLayerItem*>(list()->layer(layer->parent()->id())));
        layer->accept(visitor);
        layersAdded = visitor.layersAdded();
    }

    for (vKisLayerSP::iterator it = layersAdded.begin(); it != layersAdded.end(); ++it) {
        markModified((*it).data());
    }
    updateUI();
}

void KisLayerBox::slotLayerRemoved(KisLayerSP layer, KisGroupLayerSP wasParent, KisLayerSP)
{
    list()->removeLayer(layer->id());
    m_modified.removeAll(layer->id());
    markModified(wasParent.data());
    updateUI();
}

void KisLayerBox::slotLayerMoved(KisLayerSP layer, KisGroupLayerSP wasParent, KisLayerSP)
{
    int parentID = layer->parent()->id();
    if (layer->parent() == m_image->rootLayer())
        parentID = -1;

    int siblingID = -1;
    if (layer->prevSibling())
        siblingID = layer->prevSibling()->id();

    list()->moveLayer(layer->id(), parentID, siblingID);

    markModified(layer->parent().data());
    markModified(wasParent.data());
    updateUI();
}

void KisLayerBox::slotLayerPropertiesChanged(KisLayerSP layer)
{
    if (KisLayerItem* item = dynamic_cast<KisLayerItem*>(list()->layer(layer->id())))
    {
        Q_ASSERT(item->layer() == layer.data());
        item->sync();
        updateUI();
        markModified(layer.data());
    }
}

void KisLayerBox::slotLayersChanged(KisGroupLayerSP rootLayer)
{
    list()->clear();
    KisPopulateVisitor visitor(list());
    for (KisLayerSP layer = rootLayer->firstChild(); layer; layer = layer->nextSibling())
        layer->accept(visitor);
    m_modified.clear();
    for (Q3ListViewItemIterator it(list()->lastItem()); *it; --it)
        m_modified.append(static_cast<LayerItem*>(*it)->id());
    updateUI();
}

void KisLayerBox::slotLayerUpdated(KisLayerSP layer, QRect)
{
    markModified(layer.data());
}

void KisLayerBox::slotLayerActivated(LayerItem* item)
{
    if (item)
        m_image->activate(m_image->findLayer(item->id()));
    else
        m_image->activate(KisLayerSP(0));
    updateUI();
}

void KisLayerBox::slotLayerDisplayNameChanged(LayerItem* item, const QString& displayName)
{
    if(KisLayerSP layer = m_image->findLayer(item->id()))
        layer->setName(displayName);
    updateUI();
}

void KisLayerBox::slotLayerPropertyChanged(LayerItem* item, const QString& name, bool on)
{
    if (KisLayerSP layer = m_image->findLayer(item->id()))
    {
        if (name == "visible")
            layer->setVisible(on);
        else if (name == "locked")
            layer->setLocked(on);
    }
}

void KisLayerBox::slotLayerMoved(LayerItem* item, LayerItem*, LayerItem*)
{
    KisLayerSP layer = m_image->findLayer(item->id());
    KisGroupLayerSP parent;
    if( item->parent() )
        parent = dynamic_cast<KisGroupLayer*>(m_image->findLayer(item->parent()->id()).data());
    if( !parent )
        parent = m_image->rootLayer();
    KisLayerSP above;
    if (item->nextSibling())
        above = m_image->findLayer(item->nextSibling()->id());
    if (layer)
        m_image->moveLayer(layer, parent, above);
    updateUI();
}

void KisLayerBox::slotRequestNewLayer(LayerItem* p, LayerItem* after)
{
    KisLayer* l = m_image->rootLayer().data();
    if (p)
        l = m_image->findLayer(p->id()).data();
    KisGroupLayerSP parent = KisGroupLayerSP(dynamic_cast<KisGroupLayer*>(l));

    KisLayerSP above;
    if (after && after->nextSibling())
        above = m_image->findLayer(after->nextSibling()->id());
    else if (after)
        above = 0;
    else if (p && p->firstChild())
        above = parent->firstChild();
    else if (!p && m_image->rootLayer()->childCount())
        above = m_image->rootLayer()->firstChild();
    emit sigRequestLayer(parent, above);
}

void KisLayerBox::slotRequestNewFolder(LayerItem* p, LayerItem* after)
{
    KisLayer* l = m_image->rootLayer().data(); //FIXME I hate copy-pasting like this.
    if (p)
        l = m_image->findLayer(p->id()).data();
    KisGroupLayerSP parent = KisGroupLayerSP(dynamic_cast<KisGroupLayer*>(l));

    KisLayerSP above;
    if (after && after->nextSibling())
        above = m_image->findLayer(after->nextSibling()->id());
    else if (after)
        above = 0;
    else if (p && p->firstChild())
        above = parent->firstChild();
    else if (!p && m_image->rootLayer()->childCount())
        above = m_image->rootLayer()->firstChild();
    emit sigRequestGroupLayer(parent, above);
}

void KisLayerBox::slotRequestNewAdjustmentLayer(LayerItem* p, LayerItem* after)
{
    KisLayer* l = m_image->rootLayer().data(); //FIXME here too.
    if (p)
        l = m_image->findLayer(p->id()).data();
    KisGroupLayerSP parent = KisGroupLayerSP(dynamic_cast<KisGroupLayer*>(l));

    KisLayerSP above;
    if (after && after->nextSibling())
        above = m_image->findLayer(after->nextSibling()->id());
    else if (after)
        above = 0;
    else if (p && p->firstChild())
        above = parent->firstChild();
    else if (!p && m_image->rootLayer()->childCount())
        above = m_image->rootLayer()->firstChild();
    emit sigRequestAdjustmentLayer(parent, above);
}

void KisLayerBox::slotRequestNewObjectLayer(LayerItem* p, LayerItem* after, const KoDocumentEntry& entry)
{
    KisLayer* l = m_image->rootLayer().data(); //FIXME and here.
    if (p)
        l = m_image->findLayer(p->id()).data();
    KisGroupLayerSP parent = KisGroupLayerSP(dynamic_cast<KisGroupLayer*>(l));

    KisLayerSP above;
    if (after && after->nextSibling())
        above = m_image->findLayer(after->nextSibling()->id());
    else if (after)
        above = 0;
    else if (p && p->firstChild())
        above = parent->firstChild();
    else if (!p && m_image->rootLayer()->childCount())
        above = m_image->rootLayer()->firstChild();
    emit sigRequestPartLayer(parent, above, entry);
}

void KisLayerBox::slotRequestRemoveLayer(LayerItem* item)
{
    if (KisLayerSP layer = m_image->findLayer(item->id())) {
        m_image->removeLayer(layer);
    }
    updateUI();
}

void KisLayerBox::slotRequestLayerProperties(LayerItem* item)
{
    if (KisLayerSP layer = m_image->findLayer(item->id()))
    {
        emit sigRequestLayerProperties(layer);
    }
}

void KisLayerBox::updateUI()
{
    m_lst->bnDelete->setEnabled(list()->activeLayer());
    m_lst->bnRaise->setEnabled(list()->activeLayer() && (list()->activeLayer()->prevSibling() || list()->activeLayer()->parent()));
    m_lst->bnLower->setEnabled(list()->activeLayer() && list()->activeLayer()->nextSibling());
    m_lst->intOpacity->setEnabled(list()->activeLayer());
    m_lst->cmbComposite->setEnabled(list()->activeLayer());
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

void KisLayerBox::slotAboutToShow()
{
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

void KisLayerBox::clear()
{
    list()->clear();
    updateUI();
}

void KisLayerBox::getNewLayerLocation(KisGroupLayerSP &parent, KisLayerSP &above)
{
    KisGroupLayerSP root = m_image->rootLayer();
    if (KisLayerSP active = m_image->activeLayer())
    {
        parent = root;
        above = active;
        if (active->parent())
            parent = active->parent();
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
    QList<int> l = list()->selectedLayerIDs();
    if (l.count() < 2 && list()->activeLayer() && !l.contains(list()->activeLayer()->id()))
    {
        l.clear();
        l.append(list()->activeLayer()->id());
    }

    for (int i = 0, n = l.count(); i < n; ++i)
    {
        m_modified.removeAll(l[i]);
        m_image->removeLayer(m_image->findLayer(l[i]));
    }
}

void KisLayerBox::slotRaiseClicked()
{
    QList<int> l = list()->selectedLayerIDs();
    if (l.count() < 2 && list()->activeLayer() && !l.contains(list()->activeLayer()->id()))
    {
        l.clear();
        l.append(list()->activeLayer()->id());
    }

    KisLayerSP layer = m_image->findLayer(l.first());
    if( l.count() == 1 && layer == layer->parent()->firstChild() && layer->parent() != m_image->rootLayer())
    {
        if (KisGroupLayerSP grandparent = layer->parent()->parent())
            m_image->moveLayer(layer, grandparent, KisLayerSP(layer->parent().data()));
    }
    else
    {
        for (int i = 0, n = l.count(); i < n; ++i)
            if (KisLayerSP li = m_image->findLayer(l[i]))
                if (li->prevSibling())
                    m_image->moveLayer(li, li->parent(), li->prevSibling());
    }

    if( !l.isEmpty() )
        list()->ensureItemVisible( list()->layer( l.first() ) );
}

void KisLayerBox::slotLowerClicked()
{
    QList<LayerItem*> l = list()->selectedLayers();
    if (l.count() < 2 && list()->activeLayer() && !l.contains(list()->activeLayer()))
    {
        l.clear();
        l.append(list()->activeLayer());
    }

    for (int i = l.count() - 1; i >= 0; --i)
        if (LayerItem *layer = l[i])
            if (layer->nextSibling())
                list()->moveLayer(layer, layer->parent(), layer->nextSibling());

    if( !l.isEmpty() )
        list()->ensureItemVisible( l.last() );
}

void KisLayerBox::slotPropertiesClicked()
{
    if (KisLayerSP active = m_image->activeLayer())
        emit sigRequestLayerProperties(active);
}

void KisLayerBox::updateThumbnails()
{
    bool again = true;
    while (m_modified.count() && again)
    {
        //again = false;
        KisLayerItem* item = static_cast<KisLayerItem*>(list()->layer(m_modified.last()));
        m_modified.pop_back();
        if (!item || !item->updatePreview())
            again = true;
    }
}

void KisLayerBox::setUpdatesAndSignalsEnabled(bool enable)
{
    setUpdatesEnabled(enable);
    m_lst->intOpacity->setUpdatesEnabled(enable);
    m_lst->cmbComposite->setUpdatesEnabled(enable);

    list()->blockSignals(!enable);
    m_lst->intOpacity->blockSignals(!enable);
    m_lst->cmbComposite->blockSignals(!enable);
}


QPixmap KisLayerBox::loadPixmap(const QString& filename, const KIconLoader&
                                    il, int size)
{
    QPixmap pixmap = il.loadIcon(filename, K3Icon::NoGroup, size);

    if (pixmap.isNull())
        KMessageBox::error(0, i18n("Cannot find %1", filename),
                           i18n("Canvas"));

    return pixmap;
}

void KisLayerBox::markModified(KisLayer* layer)
{
    if( !layer )
        return;

    QList<int> v;
    while (layer && layer != m_image->rootLayer().data())
    {
        v.append(layer->id());
        layer = layer->parent().data();
    }
    for (int i = v.count() - 1; i >= 0; --i)
        if (!m_modified.contains(v[i]))
            m_modified.append(v[i]);
}

void KisLayerBox::printKritaLayers() const
{
    static int indent = 0;
    static KisLayerSP root;
    if( !root )
        root = KisLayerSP(m_image->rootLayer().data());
    if( !root )
        return;
    QString s = root->name();
    if( dynamic_cast<KisGroupLayer*>( root.data() ) )
        s = QString("[%1]").arg( s );
    if( m_image->activeLayer() == root )
        s.prepend("*");
    kDebug() << (QString().fill(' ', indent) +  s) << endl;
    for (KisLayerSP layer = root->firstChild(); layer; layer = layer->nextSibling())
    {
        indent += 2;
        root = layer;
        printKritaLayers();
        indent -= 2;
        root = KisLayerSP(layer->parent().data());
    }
}

void KisLayerBox::printLayerboxLayers() const
{
    static int indent = 0;
    static LayerItem *root = 0;
    if( !root )
    {
        for (LayerItem* layer = list()->firstChild(); layer; layer = layer->nextSibling())
        {
            indent += 2;
            root = layer;
            printLayerboxLayers();
            indent -= 2;
            root = layer->parent();
        }
        return;
    }
    QString s = root->displayName();
    if( root->isFolder() )
        s = QString("[%1]").arg( s );
    if( list()->activeLayer() == root )
        s.prepend("*");
    kDebug() << (QString().fill(' ', indent) +  s) << endl;
    for (LayerItem* layer = root->firstChild(); layer; layer = layer->nextSibling())
    {
        indent += 2;
        root = layer;
        printLayerboxLayers();
        indent -= 2;
        root = layer->parent();
    }
}

#include "kis_layerbox.moc"
