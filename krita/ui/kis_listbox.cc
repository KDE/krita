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
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */

#include <qbutton.h>
#include <qbrush.h>
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
#include <kpushbutton.h>
#include <kiconloader.h>
#include <kicontheme.h>
#include <klocale.h>
#include <kmessagebox.h>
#include <kpopupmenu.h>

#include "kis_listbox.h"

const int HEIGHT = 32;

KisListBoxView::KisListBoxView(const QString& label, flags f, QWidget *parent, const char *name) : super(parent, name)
{
	QVBoxLayout *vbox = new QVBoxLayout(this);
	KPopupMenu *mnu;
	QHBox *hbox;
	QButton *btn;

	vbox -> setAutoAdd(true);
	m_lst = new KListBox(this);

#if 1
	m_lst -> insertItem(new KisListBoxItem("Test 1", m_lst));
	m_lst -> insertItem(new KisListBoxItem("Test 2", m_lst));
#endif

	hbox = new QHBox(this);
	btn = new KPushButton(hbox);
	btn -> setPixmap(BarIcon("newlayer"));
	QToolTip::add(btn, i18n("Create New %1").arg(label));
	btn = new KPushButton(hbox);
	btn -> setPixmap(BarIcon("deletelayer"));
	QToolTip::add(btn, i18n("Remove Current %1").arg(label));
	btn = new KPushButton(hbox);
	btn -> setPixmap(BarIcon("raiselayer"));
	QToolTip::add(btn, i18n("Upper Current %1").arg(label));
	btn = new KPushButton(hbox);
	btn -> setPixmap(BarIcon("lowerlayer"));
	QToolTip::add(btn, i18n("Lower Current %1").arg(label));

	mnu = new KPopupMenu();

	mnu -> insertItem(i18n("Raise %1").arg(label), UPPER);
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

	m_contextMnu -> insertItem(i18n("Properties"), PROPERTIES);
	m_contextMnu -> insertSeparator();

	m_contextMnu -> insertItem(i18n("Add %1").arg(label), ADD);
	m_contextMnu -> insertItem(i18n("Remove %1").arg(label), REMOVE);

	if (f & SHOWMASK) {
		m_contextMnu -> insertItem(i18n("Add Mask"), ADDMASK);
		m_contextMnu -> insertItem(i18n("Remove Mask"), REMOVEMASK);
	}

	connect(m_contextMnu, SIGNAL(activated(int)), SLOT(slotMenuAction(int)));
	connect(m_contextMnu, SIGNAL(aboutToShow()), SLOT(slotAboutToShow()));
	connect(mnu, SIGNAL(activated(int)), SLOT(slotMenuAction(int)));
	connect(m_lst, SIGNAL(contextMenuRequested(QListBoxItem *, const QPoint&)), SLOT(slotShowContextMenu(QListBoxItem*, const QPoint&)));
	connect(m_lst, SIGNAL(executed(QListBoxItem*, const QPoint&)), SLOT(slotExecuted(QListBoxItem*, const QPoint&)));
}

KisListBoxView::~KisListBoxView()
{
}

void KisListBoxView::slotMenuAction(int mnuId)
{
}

void KisListBoxView::slotAboutToShow()
{
	bool enabled = m_lst -> isSelected(m_lst -> currentItem());

	m_contextMnu -> setItemEnabled(VISIBLE, enabled);
	m_contextMnu -> setItemEnabled(SELECTION, enabled);
	m_contextMnu -> setItemEnabled(LEVEL, enabled);
	m_contextMnu -> setItemEnabled(LINKING, enabled);
	m_contextMnu -> setItemEnabled(PROPERTIES, enabled);
}

void KisListBoxView::slotShowContextMenu(QListBoxItem *item, const QPoint& pos)
{
	m_lst -> setCurrentItem(item);
	m_contextMnu -> popup(pos);
}

void KisListBoxView::slotExecuted(QListBoxItem *item, const QPoint& pos)
{
	KisListBoxItem *p = dynamic_cast<KisListBoxItem*>(item);
	int n = m_lst -> currentItem();

	if (n == -1)
		return;

	kdDebug() << "n = " << n << endl;
	kdDebug() << "KisListBoxItem::intersectVisibleRect = " << p -> intersectVisibleRect(pos, n) << endl;
	kdDebug() << "KisListBoxItem::intersectLinkedRect = " << p -> intersectLinkedRect(pos, n) << endl;
	kdDebug() << "KisListBoxItem::intersectPreviewRect = " << p -> intersectPreviewRect(pos, n) << endl;
}

KisListBoxItem::KisListBoxItem(const QString& label, QListBox *parent, int flags)
{
	init(label, parent, flags);
}

void KisListBoxItem::init(const QString& label, QListBox *parent, int flags)
{
	KIconLoader il;

	m_label = label;
	m_visiblePix = loadPixmap("visible.png", il);
	m_visibleRect = QRect(QPoint(3, (HEIGHT - 24) / 2), QSize(24,24));
	m_invisiblePix = loadPixmap("novisible.png", il);
	m_linkedPix = loadPixmap("linked.png", il);
	m_linkedRect = QRect(QPoint(30, (HEIGHT - 24) / 2), QSize(24,24));
	m_unlinkedPix = loadPixmap("unlinked.png", il);
	m_previewRect = QRect(QPoint(57, (HEIGHT - 24) / 2), QSize(24,24));
	m_parent = parent;
	m_visible = true;
	m_linked = false;
}

KisListBoxItem::~KisListBoxItem()
{
}

int KisListBoxItem::height(const QListBox *lb) const
{
	return HEIGHT;
}

int KisListBoxItem::width(const QListBox *lb) const
{
	m_size.setWidth(lb -> width());
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
	QBrush br = isSelected() ? QBrush::gray : QBrush::lightGray;
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

QPixmap KisListBoxItem::loadPixmap(const QString& filename, const KIconLoader& il)
{
	QPixmap pixmap = il.loadIcon(filename, KIcon::NoGroup);

	if (pixmap.isNull()) {
		QString errmsg = "Can't find " + filename;

		KMessageBox::error(0, i18n(errmsg.latin1()), i18n("Canvas"));
	}

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

