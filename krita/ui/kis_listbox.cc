/*
 *  kis_listbox.cc - part of Krita aka Krayon aka KimageShop
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

#include "kis_listbox.h"

const int HEIGHT = 32;

KisListBox::KisListBox(const QString& label, flags f, QWidget *parent, const char *name) : super(parent, name)
{
	QVBoxLayout *vbox = new QVBoxLayout(this);
	KPopupMenu *mnu;
	QHBox *hbox;
	QButton *btn;

	m_flags = f;
	vbox -> setAutoAdd(true);
	m_lst = new KListBox(this);

	hbox = new QHBox(this);
	btn = new KPushButton(hbox);
	btn -> setPixmap(BarIcon("newlayer"));
	QToolTip::add(btn, i18n("Create new %1").arg(label));
	m_btnRm = new KPushButton(hbox);
	m_btnRm -> setPixmap(BarIcon("deletelayer"));
	QToolTip::add(m_btnRm, i18n("Remove current %1").arg(label));
	m_btnRaise = new KPushButton(hbox);
	m_btnRaise -> setPixmap(BarIcon("raiselayer"));
	QToolTip::add(m_btnRaise, i18n("Upper current %1").arg(label));
	m_btnRaise -> setEnabled(false);
	m_btnLower = new KPushButton(hbox);
	m_btnLower -> setPixmap(BarIcon("lowerlayer"));
	m_btnLower -> setEnabled(false);
	QToolTip::add(m_btnLower, i18n("Lower current %1").arg(label));

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

	connect(m_contextMnu, SIGNAL(activated(int)), SLOT(slotMenuAction(int)));
	connect(m_contextMnu, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
	connect(mnu, SIGNAL(activated(int)), SLOT(slotMenuAction(int)));
	connect(m_lst, SIGNAL(contextMenuRequested(QListBoxItem *, const QPoint&)), SLOT(slotShowContextMenu(QListBoxItem*, const QPoint&)));
	connect(m_lst, SIGNAL(pressed(QListBoxItem*)), SLOT(slotSelectionChanged(QListBoxItem*)));
	connect(m_lst, SIGNAL(clicked(QListBoxItem *, const QPoint&)), SLOT(slotClicked(QListBoxItem*, const QPoint&)));
	connect(m_lst, SIGNAL(doubleClicked(QListBoxItem*)), SLOT(slotDoubleClicked(QListBoxItem*)));
	connect(m_lst, SIGNAL(returnPressed(QListBoxItem*)), SLOT(slotDoubleClicked(QListBoxItem*)));
	connect(btn, SIGNAL(clicked()), SLOT(slotAddClicked()));
	connect(m_btnRm, SIGNAL(clicked()), SLOT(slotRmClicked()));
	connect(m_btnRaise, SIGNAL(clicked()), SLOT(slotRaiseClicked()));
	connect(m_btnLower, SIGNAL(clicked()), SLOT(slotLowerClicked()));
}

KisListBox::~KisListBox()
{
}

void KisListBox::slotMenuAction(int mnuId)
{
	int n = m_lst -> currentItem();
	KisListBoxItem *p;

	if (n == -1 && mnuId != ADD) {
		setCurrentItem(n);
		return;
	}

	p = dynamic_cast<KisListBoxItem*>(m_lst -> item(n));

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
	m_btnRaise -> setEnabled(m_lst -> selectedItem() && m_lst -> selectedItem() != m_lst -> item(0));
	m_btnLower -> setEnabled(m_lst -> selectedItem() && m_lst -> currentItem() != -1 && static_cast<uint>(m_lst -> currentItem()) != m_lst -> count() - 1);
	m_lst -> triggerUpdate(false);
}

void KisListBox::slotAboutToShow()
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
	m_contextMnu -> setItemEnabled(RAISE, m_lst -> item(m_lst -> currentItem()) != m_lst -> firstItem());
	m_contextMnu -> setItemEnabled(LOWER, m_lst -> item(m_lst -> currentItem()) != m_lst -> item(m_lst -> count() - 1));
}

void KisListBox::slotShowContextMenu(QListBoxItem *item, const QPoint& pos)
{
	m_lst -> setCurrentItem(item);
	m_contextMnu -> popup(pos);
	m_btnRm -> setEnabled(item != 0);
	m_btnRaise -> setEnabled(item && item != m_lst -> item(0));
	m_btnLower -> setEnabled(item != 0);
}

void KisListBox::slotSelectionChanged(QListBoxItem *item)
{
	Q_INT32 n;

	if (item) {
		n = m_lst -> currentItem();
		slotMenuAction(SELECTION);
		m_btnLower -> setEnabled(item && static_cast<Q_UINT32>(n) != m_lst -> count() - 1);
	} else {
		emit itemSelected(-1);
	}

	m_btnRm -> setEnabled(item != 0);
	m_btnRaise -> setEnabled(item && item != m_lst -> item(0));
}

void KisListBox::slotClicked(QListBoxItem *item, const QPoint& pos)
{
	int n = m_lst -> currentItem();

	if (item) {
		KisListBoxItem *p = dynamic_cast<KisListBoxItem*>(item);
		int m = n - m_lst -> topItem();

		if (p -> intersectVisibleRect(pos, m))
			slotMenuAction(VISIBLE);
		else if (p -> intersectLinkedRect(pos, m))
			slotMenuAction(LINKING);
	}

	m_btnRm -> setEnabled(item != 0);
	m_btnRaise -> setEnabled(item && item != m_lst -> item(0));
	m_btnLower -> setEnabled(item && n != -1 && static_cast<uint>(n) != m_lst -> count() - 1);
}

void KisListBox::slotDoubleClicked(QListBoxItem * /*item*/)
{
	slotMenuAction(PROPERTIES);
}

void KisListBox::setCurrentItem(int n)
{
	m_lst -> setSelected(n, true);
}

void KisListBox::setTopItem(int n)
{
	m_lst -> setTopItem(n);
	m_lst -> triggerUpdate(false);
}

void KisListBox::insertItem(const QString& name, bool visible, bool linked)
{
	KisListBoxItem *p = new KisListBoxItem(name, m_lst, m_flags);

	p -> setVisible(visible);
	p -> setLinked(linked);
	m_lst -> insertItem(p);
	m_lst -> setCurrentItem(p);
}

void KisListBox::clear()
{
	m_lst -> clear();
}

void KisListBox::slotAddClicked()
{
	slotMenuAction(ADD);
}

void KisListBox::slotRmClicked()
{
	slotMenuAction(REMOVE);
}

void KisListBox::slotRaiseClicked()
{
	slotMenuAction(RAISE);
}

void KisListBox::slotLowerClicked()
{
	slotMenuAction(LOWER);
}

int KisListBox::getCurrentItem() const
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

void KisListBox::setSelected(int index)
{
	m_lst -> setSelected(index, true);
	m_lst -> setCurrentItem(index);
}

KisListBoxItem::KisListBoxItem(const QString& label, QListBox *parent, KisListBox::flags f)
{
	init(label, parent, f);
}

void KisListBoxItem::init(const QString& label, QListBox *parent, KisListBox::flags f)
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

KisListBoxItem::~KisListBoxItem()
{
}

int KisListBoxItem::height(const QListBox * /*lb*/) const
{
	return HEIGHT;
}

int KisListBoxItem::width(const QListBox *lb) const
{
	const QFont& font = lb -> font();
	QFontMetrics fm(font);

	m_size.setWidth(kMax(fm.maxWidth() * m_label.length(), static_cast<uint>(lb -> width())));
	return m_size.width();
}

int KisListBoxItem::height() const
{
	return HEIGHT;
}

int KisListBoxItem::width() const
{
	return m_parent ? m_parent -> width() : m_size.width();
}

void KisListBoxItem::paint(QPainter *gc)
{
	QBrush br = isSelected() ? m_parent -> colorGroup().highlight() : QBrush::white;
	QPoint pt;
	QPixmap *pix;

	gc -> fillRect(0, 0, width(), height() - 1, br);

	m_parent -> style().drawPrimitive(QStyle::PE_Panel, gc, m_visibleRect, m_parent -> colorGroup());
	pt = QPoint(m_visibleRect.left() + 2, m_visibleRect.top() + 2);
	pix = m_visible ? &m_visiblePix : &m_invisiblePix;
	gc -> drawPixmap(pt, *pix, QRect(0, 0, m_visibleRect.width(), m_visibleRect.height()));

	m_parent -> style().drawPrimitive(QStyle::PE_Panel, gc, m_linkedRect, m_parent -> colorGroup());
	pt = QPoint(m_linkedRect.left() + 2, m_linkedRect.top() + 2);
	pix = m_linked ? &m_linkedPix : &m_unlinkedPix;
	gc -> drawPixmap(pt, *pix, QRect(0, 0, m_linkedRect.width(), m_linkedRect.height()));

	m_parent -> style().drawPrimitive(QStyle::PE_Panel, gc, m_previewRect, m_parent -> colorGroup());
	gc -> drawRect(0, 0, width() - 1, height() - 1);
	gc -> drawText(HEIGHT * 3 + 3 * 3, 20, m_label);
}

QPixmap KisListBoxItem::loadPixmap(const QString& filename, const KIconLoader& il, int size)
{
	QPixmap pixmap = il.loadIcon(filename, KIcon::NoGroup, size);

	if (pixmap.isNull())
		KMessageBox::error(0, i18n("Can't find %1").arg(filename), i18n("Canvas"));

	return pixmap;
}

bool KisListBoxItem::intersectVisibleRect(const QPoint& pos, int yOffset) const
{
	return intersectRect(m_visibleRect, pos, yOffset);
}

bool KisListBoxItem::intersectLinkedRect(const QPoint& pos, int yOffset) const
{
	return intersectRect(m_linkedRect, pos, yOffset);
}

bool KisListBoxItem::intersectPreviewRect(const QPoint& pos, int yOffset) const
{
	return intersectRect(m_previewRect, pos, yOffset);
}

bool KisListBoxItem::intersectRect(const QRect& rc, const QPoint& pos, int yOffset) const
{
	QRect global(rc.x(), rc.y() + height() * yOffset, rc.width(), rc.height());

	global = QRect(m_parent -> mapToGlobal(global.topLeft()), m_parent -> mapToGlobal(global.bottomRight()));
	return global.contains(pos);
}

#include "kis_listbox.moc"

