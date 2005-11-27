/*
 *  kis_layerbox.h - part of Krita aka Krayon aka KimageShop
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

#ifndef KIS_LAYERBOX_H
#define KIS_LAYERBOX_H

#include <qframe.h>
#include <qpixmap.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>

#include <kdebug.h>
#include <klistbox.h>

#include "kis_colorspace.h"

class WdgLayerBox;
class QButton;
class QPainter;
class QWidget;
class KIconLoader;
class KPopupMenu;
class KisCompositeOp;

// XXX: Add layer locking, previews
class KisLayerBox : public QFrame {
        typedef QFrame super;
        Q_OBJECT

public:
    // XXX: Use a list of KAction for these, possibly wrapped with some extra flags for visible in a column number
    enum action {VISIBLE, SELECTION, LINKING, PROPERTIES, ADD, REMOVE, ADDMASK, REMOVEMASK, RAISE, LOWER, FRONT, BACK, LOCKING};
    enum flags {SHOWVISIBLE = 1, SHOWLINKED = (1 << 1), SHOWPREVIEW = (1 << 2), SHOWMASK = (1 << 3), SHOWALL = (SHOWPREVIEW|SHOWLINKED|SHOWVISIBLE)};

    KisLayerBox(const QString& label, flags f = SHOWALL, QWidget *parent = 0, const char *name = 0);
    virtual ~KisLayerBox();

    void insertItem(const QString& name, bool visible = true, bool linked = false, bool locked = false);
    int getCurrentItem() const;
    void setTopItem(int n);
    void clear();
    void setSelected(int index);
    void setUpdatesAndSignalsEnabled(bool enable);
    void updateAll();

public slots:

    void slotSetCurrentItem(int n);
    void setCompositeOp(const KisCompositeOp& compositeOp);
    void setOpacity(int opacity);
    void setColorSpace(const KisColorSpace * colorSpace);

signals:
    void itemToggleVisible();
    void itemSelected(int n);
    void itemToggleLinked();
    void itemToggleLocked();
    void itemProperties();
    void itemAdd();
    void itemRemove();
    void itemAddMask(int n);
    void itemRmMask(int n);
    void itemLockMask(int n);
    void itemRaise();
    void itemLower();
    void itemFront();
    void itemBack();
    void opacityChanged(int opacity);
    void actLayerVisChanged(int visibility);
    void itemComposite(const KisCompositeOp&);

private slots:
    void slotMenuAction(int mnuId);
    void slotAboutToShow();
    void slotShowContextMenu(QListBoxItem *item, const QPoint& pos);
    void slotClicked(QListBoxItem *item, const QPoint& pos);
    void slotDoubleClicked(QListBoxItem* item);
    void slotAddClicked();
    void slotRmClicked();
    void slotRaiseClicked();
    void slotLowerClicked();


private:
    flags m_flags;
    WdgLayerBox *m_lst;
    KPopupMenu *m_contextMnu;
};

class KisLayerBoxItem : public QListBoxItem {
    typedef QListBoxItem super;

public:
    KisLayerBoxItem(const QString& label, QListBox *parent, KisLayerBox::flags f = KisLayerBox::SHOWALL);
    virtual ~KisLayerBoxItem();

    virtual int height(const QListBox *lb) const;
    virtual int width(const QListBox *lb) const;
    virtual int height() const;
    virtual int width() const;
    virtual void paint(QPainter *gc);

    bool visible();
    bool linked();
    bool locked();

    void toggleVisible();
    void toggleLinked();
    void toggleLocked();

    void setVisible(bool v);
    void setLinked(bool v);
    void setLocked(bool v);

    bool intersectVisibleRect(const QPoint& pos, int yOffset) const;
    bool intersectLinkedRect(const QPoint& pos, int yOffset) const;
    bool intersectPreviewRect(const QPoint& pos, int yOffset) const;
    bool intersectLockedRect(const QPoint& pos, int yOffset) const;

private:
    void init(const QString& label, QListBox *parent, KisLayerBox::flags f);
    QPixmap loadPixmap(const QString& filename, const KIconLoader& il, int
size);
    bool intersectRect(const QRect& rc, const QPoint& pos, int yOffset)
const;

private:
    mutable QSize m_size;
    QString m_label;
    QPixmap m_visiblePix;
    QPixmap m_invisiblePix;
    QPixmap m_linkedPix;
    QPixmap m_unlinkedPix;
    QPixmap m_lockedPix;
    QPixmap m_unlockedPix;

    QPixmap m_preview;
    QRect m_visibleRect;
    QRect m_linkedRect;
    QRect m_lockedRect;
    QRect m_previewRect;
    QWidget *m_parent;
    bool m_visible;
    bool m_linked;
    bool m_locked;
    KisLayerBox::flags m_flags;
};

inline
bool KisLayerBoxItem::visible()
{
    return m_visible;
}

inline
bool KisLayerBoxItem::linked()
{
    return m_linked;
}

inline
bool KisLayerBoxItem::locked()
{
    return m_locked;
}

inline
void KisLayerBoxItem::toggleVisible()
{
    m_visible = !m_visible;
}

inline
void KisLayerBoxItem::toggleLinked()
{
    m_linked = !m_linked;
}


inline
void KisLayerBoxItem::toggleLocked()
{
    m_locked = !m_locked;
}


inline
void KisLayerBoxItem::setVisible(bool v)
{
    m_visible = v;
}

inline
void KisLayerBoxItem::setLinked(bool v)
{
    m_linked = v;
}


inline
void KisLayerBoxItem::setLocked(bool v)
{
    m_locked = v;
}


#endif // KIS_LAYERBOX_H

