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
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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

#include <kdebug.h>
#include <kglobal.h>
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>

#include "kis_layerbox.h"

const int HEIGHT = 32;

KisLayerBox::KisLayerBox(const QString& label, flags f, QWidget *parent, const char *name) : super(parent, name)
{
        QVBoxLayout *vbox = new QVBoxLayout(this);
        KPopupMenu *mnu;
        QHBox *hbox;
        QButton *btn;

        m_flags = f;
        vbox -> setAutoAdd(true);
        m_lst = new KListBox(this);

        hbox = new QHBox(this);
        hbox -> setMargin(4);
        hbox -> setSpacing(4);
        hbox -> setMaximumHeight(30);

        btn = new QToolButton(hbox);
        btn -> setPixmap(BarIcon("newlayer"));
        btn -> setFixedSize(24, 24);
        QToolTip::add(btn, i18n("Create new %1").arg(label));

        m_btnRm = new QToolButton(hbox);
        m_btnRm -> setPixmap(BarIcon("deletelayer"));
        m_btnRm -> setFixedSize(24, 24);
        QToolTip::add(m_btnRm, i18n("Remove current %1").arg(label));

        m_btnRaise = new QToolButton(hbox);
        m_btnRaise -> setPixmap(BarIcon("raiselayer"));
        QToolTip::add(m_btnRaise, i18n("Upper current %1").arg(label));
        m_btnRaise -> setEnabled(false);
        m_btnRaise -> setFixedSize(24, 24);

        m_btnLower = new QToolButton(hbox);
        m_btnLower -> setPixmap(BarIcon("lowerlayer"));
        m_btnLower -> setEnabled(false);
        QToolTip::add(m_btnLower, i18n("Lower current %1").arg(label));
        m_btnLower -> setFixedSize(24, 24);

        QWidget * spacer = new QWidget(hbox);
        hbox -> setStretchFactor(spacer, 10);

        mnu = new KPopupMenu();
        mnu -> insertItem(i18n("Raise %1").arg(label), RAISE);
        mnu -> insertItem(i18n("Lower %1").arg(label), LOWER);
        mnu -> insertItem(i18n("Foremost %1").arg(label), FRONT);
        mnu -> insertItem(i18n("Hindmost %1").arg(label), BACK);

        m_contextMnu = new KPopupMenu();
        m_contextMnu -> setCheckable(true);

        if (f & SHOWVISIBLE)
                m_contextMnu -> insertItem(i18n("Visible" ), VISIBLE);

        m_contextMnu -> insertItem(i18n("Selection"), SELECTION);
        m_contextMnu -> insertItem(i18n("Level"), mnu, LEVEL);

        if (f & SHOWLINKED)
                m_contextMnu -> insertItem(i18n("Linked"), LINKING);

        m_contextMnu -> insertItem(i18n("Properties..."), PROPERTIES);
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
        connect(m_lst, SIGNAL(contextMenuRequested(QListBoxItem *, const
QPoint&)), SLOT(slotShowContextMenu(QListBoxItem*, const QPoint&)));
        connect(m_lst, SIGNAL(pressed(QListBoxItem*)),
SLOT(slotSelectionChanged(QListBoxItem*)));
        connect(m_lst, SIGNAL(clicked(QListBoxItem *, const QPoint&)),
SLOT(slotClicked(QListBoxItem*, const QPoint&)));
        connect(m_lst, SIGNAL(doubleClicked(QListBoxItem*)),
SLOT(slotDoubleClicked(QListBoxItem*)));
        connect(m_lst, SIGNAL(returnPressed(QListBoxItem*)),
SLOT(slotDoubleClicked(QListBoxItem*)));
        connect(btn, SIGNAL(clicked()), SLOT(slotAddClicked()));
        connect(m_btnRm, SIGNAL(clicked()), SLOT(slotRmClicked()));
        connect(m_btnRaise, SIGNAL(clicked()), SLOT(slotRaiseClicked()));
        connect(m_btnLower, SIGNAL(clicked()), SLOT(slotLowerClicked()));
}

KisLayerBox::~KisLayerBox()
{
}

void KisLayerBox::slotMenuAction(int mnuId)
{
        int n = m_lst -> currentItem();
        KisLayerBoxItem *p;

        if (n == -1 && mnuId != ADD) {
                setCurrentItem(n);
                return;
        }

        p = dynamic_cast<KisLayerBoxItem*>(m_lst -> item(n));

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
                case LEVEL:
                        emit itemLevel(n);
                        break;
        }

        m_btnRm -> setEnabled(m_lst -> count());
        m_btnRaise -> setEnabled(m_lst -> selectedItem() && m_lst ->
selectedItem() != m_lst -> item(0));
        m_btnLower -> setEnabled(m_lst -> selectedItem() && m_lst ->
currentItem() != -1 && static_cast<uint>(m_lst -> currentItem()) != m_lst ->
count() - 1);
        m_lst -> triggerUpdate(false);
}

void KisLayerBox::slotAboutToShow()
{
        bool enabled = m_lst -> isSelected(m_lst -> currentItem());

        m_contextMnu -> setItemEnabled(VISIBLE, enabled);
        m_contextMnu -> setItemEnabled(SELECTION, enabled);
        m_contextMnu -> setItemEnabled(LEVEL, m_lst -> count() > 1);
        m_contextMnu -> setItemEnabled(LINKING, enabled);
        m_contextMnu -> setItemEnabled(PROPERTIES, enabled);
        m_contextMnu -> setItemEnabled(REMOVE, enabled);
        m_contextMnu -> setItemEnabled(ADDMASK, enabled);
        m_contextMnu -> setItemEnabled(REMOVEMASK, enabled);
        m_contextMnu -> setItemEnabled(RAISE, m_lst -> item(m_lst ->
currentItem()) != m_lst -> firstItem());
        m_contextMnu -> setItemEnabled(LOWER, m_lst -> item(m_lst ->
currentItem()) != m_lst -> item(m_lst -> count() - 1));
}

void KisLayerBox::slotShowContextMenu(QListBoxItem *item, const QPoint& pos)
{
        m_lst -> setCurrentItem(item);
        m_contextMnu -> popup(pos);
        m_btnRm -> setEnabled(item != 0);
        m_btnRaise -> setEnabled(item && item != m_lst -> item(0));
        m_btnLower -> setEnabled(item != 0);
}

void KisLayerBox::slotSelectionChanged(QListBoxItem *item)
{
        Q_INT32 n;

        if (item) {
                n = m_lst -> currentItem();
                slotMenuAction(SELECTION);
                m_btnLower -> setEnabled(item && static_cast<Q_UINT32>(n) !=
m_lst -> count() - 1);
        } else {
                emit itemSelected(-1);
        }

        m_btnRm -> setEnabled(item != 0);
        m_btnRaise -> setEnabled(item && item != m_lst -> item(0));
}

void KisLayerBox::slotClicked(QListBoxItem *item, const QPoint& pos)
{
        int n = m_lst -> currentItem();

        if (item) {
                KisLayerBoxItem *p = dynamic_cast<KisLayerBoxItem*>(item);
                int m = n - m_lst -> topItem();

                if (p -> intersectVisibleRect(pos, m))
                        slotMenuAction(VISIBLE);
                else if (p -> intersectLinkedRect(pos, m))
                        slotMenuAction(LINKING);
        }

        m_btnRm -> setEnabled(item != 0);
        m_btnRaise -> setEnabled(item && item != m_lst -> item(0));
        m_btnLower -> setEnabled(item && n != -1 && static_cast<uint>(n) !=
m_lst -> count() - 1);
}

void KisLayerBox::slotDoubleClicked(QListBoxItem * /*item*/)
{
        slotMenuAction(PROPERTIES);
}

void KisLayerBox::setCurrentItem(int n)
{
        m_lst -> setSelected(n, true);
}

void KisLayerBox::setTopItem(int n)
{
        m_lst -> setTopItem(n);
        m_lst -> triggerUpdate(false);
}

void KisLayerBox::insertItem(const QString& name, bool visible, bool linked)
{
        KisLayerBoxItem *p = new KisLayerBoxItem(name, m_lst, m_flags);

        p -> setVisible(visible);
        p -> setLinked(linked);
        m_lst -> insertItem(p);
        m_lst -> setCurrentItem(p);
}

void KisLayerBox::clear()
{
        m_lst -> clear();
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

        for (p = m_lst -> firstItem(); p; p = p -> next()) {
                if (p -> isSelected())
                        return n;

                n++;
        }

        return -1;
}

void KisLayerBox::setSelected(int index)
{
        m_lst -> setSelected(index, true);
        m_lst -> setCurrentItem(index);
}

KisLayerBoxItem::KisLayerBoxItem(const QString& label, QListBox *parent,
KisLayerBox::flags f)
{
        init(label, parent, f);
}

void KisLayerBoxItem::init(const QString& label, QListBox *parent,
KisLayerBox::flags f)
{
        KIconLoader il;

        m_label = label;
        m_visiblePix = loadPixmap("visible.png", il, 21);
        m_visibleRect = QRect(QPoint(3, (HEIGHT - 24) / 2), QSize(24,24));
        m_invisiblePix = loadPixmap("novisible.png", il, 21);
        m_linkedPix = loadPixmap("linked.png", il, 21);
        m_linkedRect = QRect(QPoint(30, (HEIGHT - 24) / 2), QSize(24,24));
        m_unlinkedPix = loadPixmap("unlinked.png", il, 21);
        m_previewRect = QRect(QPoint(57, (HEIGHT - 24) / 2), QSize(24,24));
        m_parent = parent;
        m_visible = true;
        m_linked = false;
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
        QBrush br = isSelected() ? m_parent -> colorGroup().highlight() :
QBrush::white;
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

        m_parent -> style().drawPrimitive(QStyle::PE_Panel, gc, m_previewRect,
m_parent -> colorGroup());
        gc -> drawRect(0, 0, width() - 1, height() - 1);
        gc -> drawText(HEIGHT * 3 + 3 * 3, 20, m_label);
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

bool KisLayerBoxItem::intersectPreviewRect(const QPoint& pos, int yOffset) const
{
        return intersectRect(m_previewRect, pos, yOffset);
}

bool KisLayerBoxItem::intersectRect(const QRect& rc, const QPoint& pos, int
yOffset) const
{
        QRect global(rc.x(), rc.y() + height() * yOffset, rc.width(),
rc.height());

        global = QRect(m_parent -> mapToGlobal(global.topLeft()), m_parent ->
mapToGlobal(global.bottomRight()));
        return global.contains(pos);
}

#include "kis_layerbox.moc"
