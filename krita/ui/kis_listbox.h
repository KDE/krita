/*
 *  kis_listbox.h - part of Krita aka Krayon aka KimageShop
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

#if !defined KIS_LISTBOX_H
#define KIS_LISTBOX_H

#include <qframe.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>

#include <klistbox.h>

class QButton;
class QPainter;
class QWidget;
class KIconLoader;
class KPopupMenu;

class KisListBoxView : public QFrame {
	typedef QFrame super;
	Q_OBJECT

public:
	enum action {VISIBLE, SELECTION, LINKING, PROPERTIES, ADD, REMOVE, ADDMASK, REMOVEMASK, RAISE, LOWER, FRONT, BACK, LEVEL};
	enum flags {SHOWVISIBLE = 1, SHOWLINKED = (1 << 1), SHOWPREVIEW = (1 << 2), SHOWMASK = (1 << 3), SHOWALL = (SHOWMASK|SHOWPREVIEW|SHOWLINKED|SHOWVISIBLE)};

	KisListBoxView(const QString& label, flags f = SHOWALL, QWidget *parent = 0, const char *name = 0);
	virtual ~KisListBoxView();

	void insertItem(const QString& name);
	void setCurrentItem(int n);
	void setTopItem(int n);
	void clear();

signals:
	void itemToggleVisible(int n);
	void itemSelected(int n);
	void itemToggleLinked(int n);
	void itemProperties(int n);
	void itemAdd();
	void itemRemove(int n);
	void itemAddMask(int n);
	void itemRmMask(int n);
	void itemRaise(int n);
	void itemLower(int n);
	void itemFront(int n);
	void itemBack(int n);
	void itemLevel(int n);

private slots:
	void slotMenuAction(int mnuId);
	void slotAboutToShow();
	void slotShowContextMenu(QListBoxItem *item, const QPoint& pos);
	void slotClicked(QListBoxItem *item, const QPoint& pos);
	void slotCurrentChanged(QListBoxItem *item);
	void slotDoubleClicked(QListBoxItem* item);

private:
	flags m_flags;
	KListBox *m_lst;
	KPopupMenu *m_contextMnu;
	QButton *m_btnRm;
	QButton *m_btnRaise;
	QButton *m_btnLower;
};

class KisListBoxItem : public QListBoxItem {
	typedef QListBoxItem super;

public:
	KisListBoxItem(const QString& label, QListBox *parent, int flags = 0);
	virtual ~KisListBoxItem();

	virtual int height(const QListBox *lb) const;
	virtual int width(const QListBox *lb) const;
	virtual int height() const;
	virtual int width() const;
	virtual void paint(QPainter *gc);

	inline bool visible();
	inline bool linked();
	inline void toggleVisible();
	inline void toggleLinked();
	
	bool intersectVisibleRect(const QPoint& pos, int yOffset) const;
	bool intersectLinkedRect(const QPoint& pos, int yOffset) const;
	bool intersectPreviewRect(const QPoint& pos, int yOffset) const;

private:
	void init(const QString& label, QListBox *parent, int flags);
	QPixmap loadPixmap(const QString& filename, const KIconLoader& il);
	bool intersectRect(const QRect& rc, const QPoint& pos, int yOffset) const;

private:
	mutable QSize m_size;
	QString m_label;
	QPixmap m_visiblePix;
	QPixmap m_invisiblePix;
	QPixmap m_linkedPix;
	QPixmap m_unlinkedPix;
	QPixmap m_preview;
	QRect m_visibleRect;
	QRect m_linkedRect;
	QRect m_previewRect;
	QWidget *m_parent;
	bool m_visible;
	bool m_linked;
};

bool KisListBoxItem::visible()
{
	return m_visible;
}

bool KisListBoxItem::linked()
{
	return m_linked;
}

void KisListBoxItem::toggleVisible()
{
	m_visible = !m_visible;
}

void KisListBoxItem::toggleLinked()
{
	m_linked = !m_linked;
}

#endif // KIS_LISTBOX_H

