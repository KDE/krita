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

    KPopupMenu *mnu;

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

    mnu = new KPopupMenu();
    mnu -> insertItem(i18n("Raise %1").arg(label), RAISE);
    mnu -> insertItem(i18n("Lower %1").arg(label), LOWER);
    mnu -> insertItem(i18n("Top %1").arg(label), FRONT);
    mnu -> insertItem(i18n("Bottom %1").arg(label), BACK);

    m_contextMnu = new KPopupMenu();
    m_contextMnu -> setCheckable(true);

    if (f & SHOWVISIBLE)
        m_contextMnu -> insertItem(i18n("Visible" ), VISIBLE);

    m_contextMnu -> insertItem(i18n("Selection"), SELECTION);

    if (f & SHOWLINKED)
        m_contextMnu -> insertItem(i18n("Linked"), LINKING);


    m_contextMnu -> insertItem(i18n("Locked"), LOCKING);
    m_contextMnu -> insertItem(i18n("Properties"), PROPERTIES);
    m_contextMnu -> insertSeparator();

    m_contextMnu -> insertItem(i18n("Add %1...").arg(label), ADD);
    m_contextMnu -> insertItem(i18n("Remove %1").arg(label), REMOVE);

    if (f & SHOWMASK) {
        m_contextMnu -> insertItem(i18n("Add Mask"), ADDMASK);
        m_contextMnu -> insertItem(i18n("Remove Mask"), REMOVEMASK);
    }

    connect(m_contextMnu, SIGNAL(activated(int)),
            SLOT(slotMenuAction(int)));
    connect(m_contextMnu, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
    connect(mnu, SIGNAL(activated(int)), SLOT(slotMenuAction(int)));

    connect(m_lst -> listLayers, SIGNAL(contextMenuRequested(QListBoxItem *, const QPoint&)), SLOT(slotShowContextMenu(QListBoxItem*, const QPoint&)));
    connect(m_lst -> listLayers, SIGNAL(clicked(QListBoxItem *, const QPoint&)), SLOT(slotClicked(QListBoxItem*, const QPoint&)));
    connect(m_lst -> listLayers, SIGNAL(doubleClicked(QListBoxItem*)), SLOT(slotDoubleClicked(QListBoxItem*)));
    connect(m_lst -> listLayers, SIGNAL(returnPressed(QListBoxItem*)), SLOT(slotDoubleClicked(QListBoxItem*)));
    connect(m_lst -> listLayers, SIGNAL(highlighted(int)), SIGNAL(itemSelected(int)));

    connect(m_lst -> bnAdd, SIGNAL(clicked()), SLOT(slotAddClicked()));
    connect(m_lst -> bnDelete, SIGNAL(clicked()), SLOT(slotRmClicked()));
    connect(m_lst -> bnRaise, SIGNAL(clicked()), SLOT(slotRaiseClicked()));
    connect(m_lst -> bnLower, SIGNAL(clicked()), SLOT(slotLowerClicked()));
    connect(m_lst -> bnProperties, SIGNAL(clicked()), SIGNAL(itemProperties()));
    connect(m_lst -> intOpacity, SIGNAL(valueChanged(int)), SIGNAL(opacityChanged(int)));
    connect(m_lst -> cmbComposite, SIGNAL(activated(const KisCompositeOp&)), SIGNAL(itemComposite(const KisCompositeOp&)));
}

KisLayerBox::~KisLayerBox()
{
}

void KisLayerBox::slotMenuAction(int mnuId)
{
    int n = m_lst -> listLayers -> currentItem();
    KisLayerBoxItem *p;

    if (n == -1 && mnuId != ADD) {
        slotSetCurrentItem(n);
        return;
    }

    p = dynamic_cast<KisLayerBoxItem*>(m_lst -> listLayers -> item(n));

    switch (mnuId) {
    case VISIBLE:
        emit itemToggleVisible();
        break;
    case SELECTION:
        emit itemSelected(n);
        break;
    case LINKING:
        emit itemToggleLinked();
        break;
    case PROPERTIES:
        emit itemProperties();
        break;
    case ADD:
        emit itemAdd();
        break;
    case REMOVE:
        emit itemRemove();
        break;
    case ADDMASK:
        emit itemAddMask(n);
        break;
    case REMOVEMASK:
        emit itemRmMask(n);
        break;
    case RAISE:
        emit itemRaise();
        break;
    case LOWER:
        emit itemLower();
        break;
    case FRONT:
        emit itemFront();
        break;
    case BACK:
        emit itemBack();
        break;
    case LOCKING:
        emit itemToggleLocked();
    }

    m_lst -> bnDelete -> setEnabled(m_lst -> listLayers -> count());
    m_lst -> bnRaise -> setEnabled(m_lst -> listLayers -> selectedItem() && m_lst -> listLayers ->
                                   selectedItem() != m_lst -> listLayers -> item(0));
    m_lst -> bnLower -> setEnabled(m_lst -> listLayers -> selectedItem() && m_lst -> listLayers ->
                                   currentItem() != -1 && static_cast<uint>(m_lst -> listLayers -> currentItem()) != m_lst -> listLayers ->
                                   count() - 1);
    m_lst -> listLayers -> triggerUpdate(false);
}

void KisLayerBox::slotAboutToShow()
{
    bool enabled = m_lst -> listLayers -> isSelected(m_lst -> listLayers -> currentItem());

    m_contextMnu -> setItemEnabled(VISIBLE, enabled);
    m_contextMnu -> setItemEnabled(SELECTION, enabled);
    m_contextMnu -> setItemEnabled(LINKING, enabled);
    m_contextMnu -> setItemEnabled(LOCKING, enabled);
    m_contextMnu -> setItemEnabled(PROPERTIES, enabled);
    m_contextMnu -> setItemEnabled(REMOVE, enabled);
    m_contextMnu -> setItemEnabled(ADDMASK, enabled);
    m_contextMnu -> setItemEnabled(REMOVEMASK, enabled);
    m_contextMnu -> setItemEnabled(RAISE, m_lst -> listLayers -> item(m_lst -> listLayers ->
                                                                      currentItem()) != m_lst -> listLayers -> firstItem());
    m_contextMnu -> setItemEnabled(LOWER, m_lst -> listLayers -> item(m_lst -> listLayers ->
                                                                      currentItem()) != m_lst -> listLayers -> item(m_lst -> listLayers -> count() - 1));
}

void KisLayerBox::slotShowContextMenu(QListBoxItem *item, const QPoint& pos)
{
    m_lst -> listLayers -> setCurrentItem(item);
    m_contextMnu -> popup(pos);
    m_lst -> bnDelete -> setEnabled(item != 0);
    m_lst -> bnRaise -> setEnabled(item && item != m_lst -> listLayers -> item(0));
    m_lst -> bnLower -> setEnabled(item != 0);
}

void KisLayerBox::slotClicked(QListBoxItem *item, const QPoint& pos)
{
    int n = m_lst -> listLayers -> currentItem();

    if (item) {
        KisLayerBoxItem *p = dynamic_cast<KisLayerBoxItem*>(item);
        int m = n - m_lst -> listLayers -> topItem();

        if (p -> intersectVisibleRect(pos, m))
            slotMenuAction(VISIBLE);
        else if (p -> intersectLinkedRect(pos, m))
            slotMenuAction(LINKING);
        else if (p -> intersectLockedRect(pos, m))
            slotMenuAction(LOCKING);
    }
}

void KisLayerBox::slotDoubleClicked(QListBoxItem * /*item*/)
{
    slotMenuAction(PROPERTIES);
}

void KisLayerBox::slotSetCurrentItem(int n)
{
    if (n != m_lst -> listLayers -> currentItem()) {
        m_lst -> listLayers -> setSelected(n, true);
    }
    m_lst -> bnRaise -> setEnabled(n > 0);
    m_lst -> bnLower -> setEnabled((Q_UINT32)n < m_lst -> listLayers -> count() - 1);
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


void KisLayerBox::setTopItem(int n)
{
    m_lst -> listLayers -> setTopItem(n);
    m_lst -> listLayers -> triggerUpdate(false);
}

void KisLayerBox::insertItem(const QString& name, bool visible, bool linked, bool locked)
{
    KisLayerBoxItem *p = new KisLayerBoxItem(name, m_lst -> listLayers, m_flags);

    p -> setVisible(visible);
    p -> setLinked(linked);
    p -> setLocked(locked);

    m_lst -> listLayers -> insertItem(p);
    m_lst -> listLayers -> setCurrentItem(p);
}

void KisLayerBox::clear()
{
    m_lst -> listLayers -> clear();
}

void KisLayerBox::slotAddClicked()
{
    slotMenuAction(ADD);
}

void KisLayerBox::slotRmClicked()
{
    slotMenuAction(REMOVE);
}

void KisLayerBox::slotRaiseClicked()
{
    slotMenuAction(RAISE);
}

void KisLayerBox::slotLowerClicked()
{
    slotMenuAction(LOWER);
}


int KisLayerBox::getCurrentItem() const
{
    QListBoxItem *p = 0;
    int n = 0;

    for (p = m_lst -> listLayers -> firstItem(); p; p = p -> next()) {
        if (p -> isSelected())
            return n;

        n++;
    }

    return -1;
}

void KisLayerBox::setSelected(int index)
{
    m_lst -> listLayers -> setSelected(index, true);
    m_lst -> listLayers -> setCurrentItem(index);
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

// ===========================================================================

KisLayerBoxItem::KisLayerBoxItem(const QString& label, QListBox *parent,
                                 KisLayerBox::flags f)
{
    init(label, parent, f);
}

void KisLayerBoxItem::init(const QString& label, QListBox *parent,
                           KisLayerBox::flags f)
{
    KIconLoader il( "krita" );

    m_label = label;

    m_visiblePix = loadPixmap("visible.png", il, 21);
    m_visibleRect = QRect(QPoint(3, (HEIGHT - 24) / 2), QSize(24,24));
    m_invisiblePix = loadPixmap("novisible.png", il, 21);

    m_linkedPix = loadPixmap("linked.png", il, 21);
    m_linkedRect = QRect(QPoint(30, (HEIGHT - 24) / 2), QSize(24,24));
    m_unlinkedPix = loadPixmap("unlinked.png", il, 21);

    m_lockedPix = loadPixmap("locked.png", il, 21);
    m_lockedRect = QRect(QPoint(57, (HEIGHT - 24) / 2), QSize(24,24));
    m_unlockedPix = loadPixmap("unlocked.png", il, 21);

    m_previewRect = QRect(QPoint(84, (HEIGHT - 24) / 2), QSize(24,24));

    m_parent = parent;
    m_visible = true;
    m_linked = false;
    m_locked = false;

    m_flags = f;
}

KisLayerBoxItem::~KisLayerBoxItem()
{
}

int KisLayerBoxItem::height(const QListBox * /*lb*/) const
{
    return HEIGHT;
}

int KisLayerBoxItem::width(const QListBox *lb) const
{
    const QFont& font = lb -> font();
    QFontMetrics fm(font);

    m_size.setWidth(kMax(fm.maxWidth() * m_label.length(),
                         static_cast<uint>(lb -> width())));
    return m_size.width();
}

int KisLayerBoxItem::height() const
{
    return HEIGHT;
}

int KisLayerBoxItem::width() const
{
    return m_parent ? m_parent -> width() : m_size.width();
}

void KisLayerBoxItem::paint(QPainter *gc)
{
    QBrush br = isSelected() ? m_parent -> colorGroup().highlight() : m_parent -> colorGroup().base();
    QPoint pt;
    QPixmap *pix;

    gc -> fillRect(0, 0, width(), height() - 1, br);

    m_parent -> style().drawPrimitive(QStyle::PE_Panel, gc, m_visibleRect,
                                      m_parent -> colorGroup());
    pt = QPoint(m_visibleRect.left() + 2, m_visibleRect.top() + 2);
    pix = m_visible ? &m_visiblePix : &m_invisiblePix;
    gc -> drawPixmap(pt, *pix, QRect(0, 0, m_visibleRect.width(),
                                     m_visibleRect.height()));

    m_parent -> style().drawPrimitive(QStyle::PE_Panel, gc, m_linkedRect,
                                      m_parent -> colorGroup());
    pt = QPoint(m_linkedRect.left() + 2, m_linkedRect.top() + 2);
    pix = m_linked ? &m_linkedPix : &m_unlinkedPix;
    gc -> drawPixmap(pt, *pix, QRect(0, 0, m_linkedRect.width(),
                                     m_linkedRect.height()));

    m_parent -> style().drawPrimitive(QStyle::PE_Panel, gc, m_lockedRect,
                                      m_parent -> colorGroup());
    pt = QPoint(m_lockedRect.left() + 2, m_lockedRect.top() + 2);
    pix = m_locked ? &m_lockedPix : &m_unlockedPix;
    gc -> drawPixmap(pt, *pix, QRect(0, 0, m_lockedRect.width(),
                                     m_lockedRect.height()));

    m_parent -> style().drawPrimitive(QStyle::PE_Panel, gc, m_previewRect,
                                      m_parent -> colorGroup());
    gc -> drawRect(0, 0, width() - 1, height() - 1);

    QPen pen = isSelected() ? m_parent -> colorGroup().highlightedText() : m_parent -> colorGroup().text();
    gc -> setPen(pen);

    gc -> drawText(HEIGHT * 4 + 3 * 3, 20, m_label);
}

QPixmap KisLayerBoxItem::loadPixmap(const QString& filename, const KIconLoader&
                                    il, int size)
{
    QPixmap pixmap = il.loadIcon(filename, KIcon::NoGroup, size);

    if (pixmap.isNull())
        KMessageBox::error(0, i18n("Can't find %1").arg(filename),
                           i18n("Canvas"));

    return pixmap;
}

bool KisLayerBoxItem::intersectVisibleRect(const QPoint& pos, int yOffset) const
{
    return intersectRect(m_visibleRect, pos, yOffset);
}

bool KisLayerBoxItem::intersectLinkedRect(const QPoint& pos, int yOffset) const
{
    return intersectRect(m_linkedRect, pos, yOffset);
}

bool KisLayerBoxItem::intersectLockedRect(const QPoint& pos, int yOffset) const
{
    return intersectRect(m_lockedRect, pos, yOffset);
}

bool KisLayerBoxItem::intersectPreviewRect(const QPoint& pos, int yOffset) const
{
    return intersectRect(m_previewRect, pos, yOffset);
}

bool KisLayerBoxItem::intersectRect(const QRect& rc, const QPoint& pos, int yOffset) const
{
    QRect global(rc.x(), rc.y() + height() * yOffset, rc.width(), rc.height());

    global = QRect(m_parent -> mapToGlobal(global.topLeft()), m_parent -> mapToGlobal(global.bottomRight()));
    return global.contains(pos);
}

#include "kis_layerbox.moc"
