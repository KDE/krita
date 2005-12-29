/*
 *  kis_layerbox.cc - part of Krita aka Krayon aka KimageShop
 *
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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

#include <qbutton.h>
#include <qtoolbutton.h>
#include <qbrush.h>
#include <qfont.h>
#include <qfontmetrics.h>
#include <qhbox.h>
#include <qlayout.h>
#include <qpainter.h>
#include <qpoint.h>
#include <qrect.h>
#include <qstring.h>
#include <qstyle.h>
#include <qtooltip.h>
#include <qwidget.h>
#include <qcombobox.h>
#include <qcheckbox.h>

#include <kdebug.h>
#include <kglobal.h>
#include <kpopupmenu.h>
#include <kmessagebox.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <klocale.h>
#include <knuminput.h>

#include "layerlist.h"
#include "kis_cmb_composite.h"
#include "wdglayerbox.h"
#include "kis_colorspace.h"
#include "kis_layer.h"
#include "kis_group_layer.h"
#include "kis_image.h"

#include "kis_populate_visitor.h"

#include "kis_layerbox.h"

KisLayerBox::KisLayerBox(QWidget *parent, const char *name)
    : super(parent, name), m_image(0)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox -> setAutoAdd(true);

    m_lst = new WdgLayerBox(this);
    setMinimumSize(m_lst -> minimumSizeHint());

    QToolTip::add(m_lst -> bnAdd, i18n("Create new layer"));

    QToolTip::add(m_lst -> bnDelete, i18n("Remove current layer"));

    QToolTip::add(m_lst -> bnRaise, i18n("Upper current layer"));
    m_lst -> bnRaise -> setEnabled(false);

    m_lst -> bnLower -> setEnabled(false);
    QToolTip::add(m_lst -> bnLower, i18n("Lower current layer"));

    QToolTip::add(m_lst -> bnProperties, i18n("Properties for layer"));

    KIconLoader il( "krita" );

    list() -> setFoldersCanBeActive(true);

    list() -> addProperty("visible", i18n("Visible"), SmallIconSet("button_ok", KIcon::SizeSmallMedium), true);

    list() -> addProperty("locked", i18n("Locked"), SmallIconSet("lock", KIcon::SizeSmallMedium));


    connect(list() -> contextMenu(), SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
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
    connect(list(), SIGNAL(requestRemoveLayer(LayerItem*)),
                    SLOT(slotRequestRemoveLayer(LayerItem*)));
    connect(list(), SIGNAL(requestLayerProperties(LayerItem*)),
                    SLOT(slotRequestLayerProperties(LayerItem*)));

    connect(m_lst -> bnAdd, SIGNAL(clicked()), SLOT(slotAddClicked()));
    connect(m_lst -> bnDelete, SIGNAL(clicked()), SLOT(slotRmClicked()));
    connect(m_lst -> bnRaise, SIGNAL(clicked()), SLOT(slotRaiseClicked()));
    connect(m_lst -> bnLower, SIGNAL(clicked()), SLOT(slotLowerClicked()));
    connect(m_lst -> bnProperties, SIGNAL(clicked()), SLOT(slotPropertiesClicked()));
    connect(m_lst -> intOpacity, SIGNAL(valueChanged(int)), SIGNAL(sigOpacityChanged(int)));
    connect(m_lst -> cmbComposite, SIGNAL(activated(const KisCompositeOp&)), SIGNAL(sigItemComposite(const KisCompositeOp&)));
    connect(m_lst -> chkActLayerVis, SIGNAL(stateChanged(int)), SIGNAL(sigActLayerVisChanged(int)));
}

KisLayerBox::~KisLayerBox()
{
}

LayerList* KisLayerBox::list() const
{
    return m_lst -> listLayers;
}

void KisLayerBox::setImage(KisImageSP img)
{
    if (m_image)
        m_image -> disconnect(this);

    m_image = img;

    if (img)
    {
        connect(img, SIGNAL(sigLayerActivated(KisLayerSP)), this, SLOT(slotLayerActivated(KisLayerSP)));
        connect(img, SIGNAL(sigLayerAdded(KisLayerSP)), this, SLOT(slotLayerAdded(KisLayerSP)));
        connect(img, SIGNAL(sigLayerRemoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)),
                this, SLOT(slotLayerRemoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)));
        connect(img, SIGNAL(sigLayerPropertiesChanged(KisLayerSP)),
                this, SLOT(slotLayerPropertiesChanged(KisLayerSP)));
        connect(img, SIGNAL(sigLayerMoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)),
                this, SLOT(slotLayerMoved(KisLayerSP, KisGroupLayerSP, KisLayerSP)));
        connect(img, SIGNAL(sigLayersChanged(KisLayerSP)), this, SLOT(slotLayersChanged(KisLayerSP)));
        slotLayersChanged(img -> rootLayer());
    }

    updateUI();
}

void KisLayerBox::slotLayerActivated(KisLayerSP layer)
{
    list() -> setActiveLayer(layer -> id());
    updateUI();
}

void KisLayerBox::slotLayerAdded(KisLayerSP layer)
{
    if (layer == m_image -> rootLayer() || list() -> layer(layer -> id()) < 0)
        return;
    if (layer -> parent() == m_image -> rootLayer())
    {
        KisPopulateVisitor visitor(list());
        layer -> accept(visitor);
    }
    else
    {
        KisPopulateVisitor visitor(list() -> layer(layer -> parent() -> id()));
        layer -> accept(visitor);
    }

    updateUI();
}

void KisLayerBox::slotLayerRemoved(KisLayerSP layer, KisGroupLayerSP, KisLayerSP)
{
    list() -> removeLayer(layer -> id());
    updateUI();
}

void KisLayerBox::slotLayerMoved(KisLayerSP layer, KisGroupLayerSP, KisLayerSP)
{
    const int parentID = layer -> parent() == m_image -> rootLayer() ? -1 : layer -> parent() -> id();
    const int siblingID = layer -> prevSibling() ? layer -> prevSibling() -> id() : -1;
    list() -> moveLayer(layer -> id(), parentID, siblingID);
    updateUI();
}

void KisLayerBox::slotLayerPropertiesChanged(KisLayerSP layer)
{
    list() -> setLayerDisplayName(layer -> id(), layer -> name());
    updateUI();
}

void KisLayerBox::slotLayersChanged(KisLayerSP rootLayer)
{
    list() -> clear();
    KisPopulateVisitor visitor(list());
    for (KisLayerSP layer = rootLayer -> firstChild(); layer; layer = layer -> nextSibling())
        layer -> accept(visitor);
    updateUI();
}

void KisLayerBox::slotLayerActivated(LayerItem* item)
{
    if (item)
        m_image -> activate(m_image -> findLayer(item -> id()));
    else
        m_image -> activate(0);
    updateUI();
}

void KisLayerBox::slotLayerDisplayNameChanged(LayerItem* item, const QString& displayName)
{
    if(KisLayerSP layer = m_image -> findLayer(item -> id()))
        layer -> setName(displayName);
    updateUI();
}

void KisLayerBox::slotLayerPropertyChanged(LayerItem* item, const QString& name, bool on)
{
    if (KisLayerSP layer = m_image -> findLayer(item -> id()))
    {
        if (name == "visible")
            layer -> setVisible(on);
        else if (name == "locked")
            layer -> setLocked(on);
    }
    updateUI();
}

void KisLayerBox::slotLayerMoved(LayerItem* item, LayerItem* p, LayerItem*)
{
    KisLayerSP layer = m_image -> findLayer(item -> id());
    KisGroupLayerSP parent = dynamic_cast<KisGroupLayer*>( p ? m_image -> findLayer(p -> id()).data()
                                                             : m_image -> rootLayer().data() );
    KisLayerSP above = item -> nextSibling() ? m_image -> findLayer(item -> nextSibling() -> id()) : 0;
    if (layer)
        m_image -> moveLayer(layer, parent.data(), above);
    updateUI();
}

void KisLayerBox::slotRequestNewLayer(LayerItem* p, LayerItem* after)
{
    //goddamn, one of them using 'above' and the other 'after' is a pain in the ass...
    KisGroupLayerSP parent = dynamic_cast<KisGroupLayer*>( p ? m_image -> findLayer(p -> id()).data()
                                                             : m_image -> rootLayer().data() );
    KisLayerSP above = (after && after -> nextSibling()) ? m_image -> findLayer(after -> nextSibling() -> id())
                     :  after ? 0
                     : (p && p -> firstChild()) ? parent -> firstChild()
                     :  p ? 0
                     :  m_image -> rootLayer() -> childCount() ? m_image -> rootLayer() -> firstChild()
                     :  0;
    emit sigRequestLayer(parent, above);
}

void KisLayerBox::slotRequestNewFolder(LayerItem* p, LayerItem* after)
{
    KisGroupLayerSP parent = dynamic_cast<KisGroupLayer*>( p ? m_image -> findLayer(p -> id()).data()
                                                             : m_image -> rootLayer().data() );
    KisLayerSP above = (after && after -> nextSibling()) ? m_image -> findLayer(after -> nextSibling() -> id())
                     :  after ? 0
                     : (p && p -> firstChild()) ? parent -> firstChild()
                     :  p ? 0
                     :  m_image -> rootLayer() -> childCount() ? m_image -> rootLayer() -> firstChild()
                     :  0;
    emit sigRequestGroupLayer(parent, above);
}

void KisLayerBox::slotRequestRemoveLayer(LayerItem* item)
{
    if (KisLayerSP layer = m_image -> findLayer(item -> id()))
        layer -> parent() -> removeLayer(layer);
    updateUI();
}

void KisLayerBox::slotRequestLayerProperties(LayerItem* item)
{
    if (KisLayerSP layer = m_image -> findLayer(item -> id()))
        emit sigRequestLayerProperties(layer);
}

void KisLayerBox::updateUI()
{
    m_lst -> bnDelete -> setEnabled(list() -> activeLayer());
    m_lst -> bnRaise -> setEnabled(list() -> activeLayer() && list() -> activeLayer() -> prevSibling());
    m_lst -> bnLower -> setEnabled(list() -> activeLayer() && list() -> activeLayer() -> nextSibling());
}

void KisLayerBox::slotAboutToShow()
{
}

void KisLayerBox::slotSetCompositeOp(const KisCompositeOp& compositeOp)
{
    m_lst -> cmbComposite -> blockSignals(true);
    m_lst -> cmbComposite -> setCurrentItem(compositeOp);
    m_lst -> cmbComposite -> blockSignals(false);
}

void KisLayerBox::slotSetColorSpace(const KisColorSpace * colorSpace)
{
    m_lst -> cmbComposite -> blockSignals(true);
    m_lst -> cmbComposite -> setCompositeOpList(colorSpace -> userVisiblecompositeOps());
    m_lst -> cmbComposite -> blockSignals(false);
}

// range: 0-100
void KisLayerBox::slotSetOpacity(int opacity)
{
    m_lst -> intOpacity -> blockSignals(true);
    m_lst -> intOpacity -> setValue(opacity);
    m_lst -> intOpacity -> blockSignals(false);
}

void KisLayerBox::clear()
{
    list() -> clear();
    updateUI();
}

void KisLayerBox::slotAddClicked()
{
    KisGroupLayerSP root = dynamic_cast<KisGroupLayer*>(m_image -> rootLayer().data());
    if (KisLayerSP active = m_image -> activeLayer())
        emit sigRequestLayer(active -> parent() ? active -> parent() : root, active);
    else
        emit sigRequestLayer(root, m_image -> rootLayer() -> firstChild());
}

void KisLayerBox::slotRmClicked()
{
    if (KisLayerSP active = m_image -> activeLayer())
        m_image -> removeLayer(active);
    updateUI();
}

void KisLayerBox::slotRaiseClicked()
{
    if (KisLayerSP active = m_image -> activeLayer())
        if (active -> prevSibling())
            m_image -> moveLayer(active, active -> parent().data(), active -> prevSibling());
    updateUI();
}

void KisLayerBox::slotLowerClicked()
{
    if (LayerItem* active = list() -> activeLayer())
        if (active -> nextSibling())
            list() -> moveLayer(active, active -> parent(), active -> nextSibling());
    updateUI();
}

void KisLayerBox::slotPropertiesClicked()
{
    if (KisLayerSP active = m_image -> activeLayer())
        emit sigRequestLayerProperties(active);
}

void KisLayerBox::setUpdatesAndSignalsEnabled(bool enable)
{
    setUpdatesEnabled(enable);
    m_lst -> intOpacity -> setUpdatesEnabled(enable);
    m_lst -> cmbComposite -> setUpdatesEnabled(enable);

    list() -> blockSignals(!enable);
    m_lst -> intOpacity -> blockSignals(!enable);
    m_lst -> cmbComposite -> blockSignals(!enable);
}

void KisLayerBox::updateAll()
{
    update();
    m_lst -> intOpacity -> update();
    m_lst -> cmbComposite -> update();
    updateUI();
}

QIconSet KisLayerBox::loadIconSet(const QString& filename, KIconLoader&
                                    il, int size)
{
    QIconSet icon = il.loadIconSet(filename, KIcon::NoGroup, size);

    if (icon.isNull())
        KMessageBox::error(0, i18n("Can't find %1").arg(filename),
                           i18n("Canvas"));

    return icon;
}

#include "kis_layerbox.moc"
