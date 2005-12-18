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
class KisCompositeOp;
class LayerItem;

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
    void slotAboutToShow();
    void slotRequestProperties(LayerItem* item);
    void slotActivated(LayerItem* item);
    void slotUpdate();
    void slotPropertyChanged(LayerItem* item, const QString& name, bool);
    void slotMoved(QListViewItem* item, QListViewItem* afterBefore, QListViewItem*);
    void slotRemoveLayer(LayerItem* item);
    void slotNewLayer(LayerItem*, LayerItem* after);
    void slotAddClicked();
    void slotRmClicked();
    void slotRaiseClicked();
    void slotLowerClicked();


private:
    QIconSet loadIconSet(const QString& filename, KIconLoader& il, int size);
    int index(LayerItem* item) const;
    LayerItem* itemAtIndex(int index) const;
    flags m_flags;
    WdgLayerBox *m_lst;
};

#endif // KIS_LAYERBOX_H

