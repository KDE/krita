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
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>
#include <klistbox.h>
#include <knuminput.h>

#include "layerlist.h"
#include "kis_layerbox.h"
#include "kis_cmb_composite.h"
#include "wdglayerbox.h"
#include "kis_colorspace.h"

const int HEIGHT = 32;

KisLayerBox::KisLayerBox(const QString& label, flags f, QWidget *parent, const char *name)
    : super(parent, name)
{
    QVBoxLayout *vbox = new QVBoxLayout(this);
    vbox -> setAutoAdd(true);

    m_flags = f;

    m_lst = new WdgLayerBox(this);
    setMinimumSize(m_lst -> minimumSizeHint());

    QToolTip::add(m_lst -> bnAdd, i18n("Create new %1").arg(label));

    QToolTip::add(m_lst -> bnDelete, i18n("Remove current %1").arg(label));

    QToolTip::add(m_lst -> bnRaise, i18n("Upper current %1").arg(label));
    m_lst -> bnRaise -> setEnabled(false);

    m_lst -> bnLower -> setEnabled(false);
    QToolTip::add(m_lst -> bnLower, i18n("Lower current %1").arg(label));

    QToolTip::add(m_lst -> bnProperties, i18n("Properties for %1").arg(label));

    KIconLoader il( "krita" );

    m_lst -> listLayers -> addProperty("visible", i18n("Visible"), SmallIconSet("button_ok", KIcon::SizeSmallMedium), true);

    m_lst -> listLayers -> addProperty("linked", i18n("Linked"), SmallIconSet("edit_add", KIcon::SizeSmallMedium));

    m_lst -> listLayers -> addProperty("locked", i18n("Locked"), SmallIconSet("lock", KIcon::SizeSmallMedium));


    connect(m_lst -> listLayers -> contextMenu(), SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
    connect(m_lst -> listLayers, SIGNAL(propertyChanged(LayerItem*, const QString&, bool)),
                               SLOT(slotPropertyChanged(LayerItem*, const QString&, bool)));
    connect(m_lst -> listLayers, SIGNAL(requestLayerProperties(LayerItem*)), SLOT(slotRequestProperties(LayerItem*)));
    connect(m_lst -> listLayers, SIGNAL(activated(LayerItem*)), SLOT(slotActivated(LayerItem*)));
    connect(m_lst -> listLayers, SIGNAL(moved(QListViewItem*, QListViewItem*, QListViewItem*)),
                               SLOT(moved(QListViewItem*, QListViewItem*, QListViewItem*)));
    connect(m_lst -> listLayers, SIGNAL(requestNewLayer(LayerItem*, LayerItem*)),
                               SLOT(slotNewLayer(LayerItem*, LayerItem*)));
    connect(m_lst -> listLayers, SIGNAL(requestRemoveLayer(LayerItem*)), SLOT(slotRemoveLayer(LayerItem*)));


    connect(m_lst -> bnAdd, SIGNAL(clicked()), SLOT(slotAddClicked()));
    connect(m_lst -> bnDelete, SIGNAL(clicked()), SLOT(slotRmClicked()));
    connect(m_lst -> bnRaise, SIGNAL(clicked()), SLOT(slotRaiseClicked()));
    connect(m_lst -> bnLower, SIGNAL(clicked()), SLOT(slotLowerClicked()));
    connect(m_lst -> bnProperties, SIGNAL(clicked()), SIGNAL(itemProperties()));
    connect(m_lst -> intOpacity, SIGNAL(valueChanged(int)), SIGNAL(opacityChanged(int)));
    connect(m_lst -> cmbComposite, SIGNAL(activated(const KisCompositeOp&)), SIGNAL(itemComposite(const KisCompositeOp&)));
    connect(m_lst -> chkActLayerVis, SIGNAL(stateChanged(int)), SIGNAL(actLayerVisChanged(int)));
}

KisLayerBox::~KisLayerBox()
{
}

void KisLayerBox::slotMoved(QListViewItem* item, QListViewItem* afterBefore, QListViewItem*)
{
    const int indexNow = index(static_cast<LayerItem*>(item));
    const int indexBefore = index(static_cast<LayerItem*>(afterBefore));

    static_cast<LayerItem*>(item) -> setActive();
    if (indexNow == 0)
        emit itemFront();
    else if (indexNow == m_lst -> listLayers -> childCount() - 1)
        emit itemBack();
    else if (indexNow > indexBefore)
        for (int i = 0; i < (indexNow - indexBefore); ++i)
            emit itemLower();
    else if (indexNow < indexBefore)
        for (int i = 0; i < (indexBefore - indexNow); ++i)
            emit itemRaise();
    slotUpdate();
}

void KisLayerBox::slotRemoveLayer(LayerItem* item)
{
    item -> setActive();
    emit itemRemove();
    slotUpdate();
}

void KisLayerBox::slotNewLayer(LayerItem*, LayerItem*)
{
    emit itemAdd();
    slotUpdate();
}

void KisLayerBox::slotRequestProperties(LayerItem* item)
{
    item->setActive();
    emit itemProperties();
    slotUpdate();
}

void KisLayerBox::slotActivated(LayerItem* active)
{
    emit itemSelected(index(active));
    slotUpdate();
}

void KisLayerBox::slotUpdate()
{
    m_lst -> bnDelete -> setEnabled(m_lst -> listLayers -> childCount());
    m_lst -> bnRaise -> setEnabled(m_lst -> listLayers -> activeLayer() && m_lst -> listLayers ->
                                   activeLayer() != m_lst -> listLayers -> firstChild());
    m_lst -> bnLower -> setEnabled(m_lst -> listLayers -> activeLayer() && m_lst -> listLayers ->
                                   activeLayer() != m_lst -> listLayers -> lastItem());
    m_lst -> listLayers -> triggerUpdate();
}

void KisLayerBox::slotAboutToShow()
{
    m_lst -> listLayers -> contextMenu() -> setItemVisible(LayerList::MenuItems::NewFolder, false);
}

void KisLayerBox::slotPropertyChanged(LayerItem* layer, const QString& name, bool)
{
    layer->setActive();
    if (name == "linked")
        emit itemToggleLinked();
    else if (name == "locked")
        emit itemToggleLocked();
    else if (name == "visible")
        emit itemToggleVisible();
    slotUpdate();
}

void KisLayerBox::slotSetCurrentItem(int n)
{
    itemAtIndex(n) -> setActive();
    m_lst -> bnRaise -> setEnabled(n > 0);
    m_lst -> bnLower -> setEnabled(n < m_lst -> listLayers -> childCount() - 1);
}

void KisLayerBox::setCompositeOp(const KisCompositeOp& compositeOp)
{
    m_lst -> cmbComposite -> blockSignals(true);
    m_lst -> cmbComposite -> setCurrentItem(compositeOp);
    m_lst -> cmbComposite -> blockSignals(false);
}

void KisLayerBox::setColorSpace(const KisColorSpace * colorSpace)
{
    m_lst -> cmbComposite -> blockSignals(true);
    m_lst -> cmbComposite -> setCompositeOpList(colorSpace -> userVisiblecompositeOps());
    m_lst -> cmbComposite -> blockSignals(false);
}

// range: 0-100
void KisLayerBox::setOpacity(int opacity)
{
    m_lst -> intOpacity -> blockSignals(true);
    m_lst -> intOpacity -> setValue(opacity);
    m_lst -> intOpacity -> blockSignals(false);
}

void KisLayerBox::insertItem(const QString& name, bool visible, bool linked, bool locked)
{
    LayerItem* item = m_lst -> listLayers -> addLayer( name );
    item -> setProperty("visible", visible);
    item -> setProperty("linked", linked);
    item -> setProperty("locked", locked);
    item -> setActive();
}

void KisLayerBox::clear()
{
    m_lst -> listLayers -> clear();
}

void KisLayerBox::slotAddClicked()
{
    emit itemAdd();
    slotUpdate();
}

void KisLayerBox::slotRmClicked()
{
    emit itemRemove();
    slotUpdate();
}

void KisLayerBox::slotRaiseClicked()
{
    emit itemRaise();
    slotUpdate();
}

void KisLayerBox::slotLowerClicked()
{
    emit itemLower();
    slotUpdate();
}

int KisLayerBox::index(LayerItem* item) const
{
    int i = 0;
    LayerItem* it;
    for(it = m_lst -> listLayers -> firstChild(); it && it != item; it = it -> nextSibling())
        i++;

    if(it && it == item)
        return i;
    return -1;
}

LayerItem* KisLayerBox::itemAtIndex(int index) const
{
    LayerItem* item = m_lst -> listLayers -> firstChild();
    for (int i = 0; item && i < index; ++i)
        item = item->nextSibling();
    return item;
}

int KisLayerBox::getCurrentItem() const
{
    return index(m_lst -> listLayers -> activeLayer());
}

void KisLayerBox::setSelected(int index)
{
    itemAtIndex(index) -> setActive();
}

void KisLayerBox::setUpdatesAndSignalsEnabled(bool enable)
{
    setUpdatesEnabled(enable);
    m_lst -> intOpacity -> setUpdatesEnabled(enable);
    m_lst -> cmbComposite -> setUpdatesEnabled(enable);

    m_lst -> listLayers -> blockSignals(!enable);
    m_lst -> intOpacity -> blockSignals(!enable);
    m_lst -> cmbComposite -> blockSignals(!enable);
}

void KisLayerBox::updateAll()
{
    update();
    m_lst -> intOpacity -> update();
    m_lst -> cmbComposite -> update();
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
