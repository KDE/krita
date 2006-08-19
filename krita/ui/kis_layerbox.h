/*
 *  kis_layerbox.h - part of Krita aka Krayon aka KimageShop
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

#ifndef KIS_LAYERBOX_H
#define KIS_LAYERBOX_H

#include <QFrame>
#include <QList>

#include <kdebug.h>

#include "kis_types.h"
#include "KoColorSpace.h"

#include "ui_wdglayerbox.h"

class WdgLayerBox : public QWidget, public Ui::WdgLayerBox
{
    Q_OBJECT

public:
    WdgLayerBox(QWidget *parent) : QWidget(parent) { setupUi(this); }
};

class QModelIndex;
typedef QList<QModelIndex> QModelIndexList;
class QPainter;
class QWidget;
class KIconLoader;
class KMenu;
class KoDocumentEntry;
class KoCompositeOp;
class KisCanvasSubject;

class KisLayerBox : public QFrame {
        typedef QFrame super;
        Q_OBJECT

public:
    KisLayerBox(QWidget *parent = 0, const char *name = 0);
    virtual ~KisLayerBox();

    void setUpdatesAndSignalsEnabled(bool enable);
    void setImage(KisImageSP image);

    virtual bool eventFilter(QObject *object, QEvent *event);

public slots:
    void slotSetCompositeOp(const KoCompositeOp& compositeOp);
    void slotSetOpacity(int opacity);
    void slotSetColorSpace(const KoColorSpace * colorSpace);

signals:
    void sigRequestLayer(KisGroupLayerSP parent, KisLayerSP above);
    void sigRequestGroupLayer(KisGroupLayerSP parent, KisLayerSP above);
    void sigRequestAdjustmentLayer(KisGroupLayerSP parent, KisLayerSP above);
    void sigRequestPartLayer(KisGroupLayerSP parent, KisLayerSP above, const KoDocumentEntry& entry);
    void sigRequestLayerProperties(KisLayerSP layer);

    void sigOpacityChanged(int opacity, bool withSlider);
    void sigOpacityFinishedChanging(int previous, int opacity);
    void sigItemComposite(const KoCompositeOp&);

private:
    enum LayerTypes { PAINT_LAYER, GROUP_LAYER, ADJUSTMENT_LAYER, OBJECT_LAYER };

private slots:
    void slotContextMenuRequested( const QPoint &pos, const QModelIndex &index );

    void slotMinimalView();
    void slotDetailedView();
    void slotThumbnailView();

    void slotRmClicked();
    void slotRaiseClicked();
    void slotLowerClicked();
    void slotPropertiesClicked();

    void slotNewLayer();
    void slotNewGroupLayer();
    void slotNewAdjustmentLayer();
    void slotNewPartLayer();

    void updateUI();

private:
    void getNewLayerLocation(KisGroupLayerSP &parent, KisLayerSP &above);
    QModelIndexList selectedLayers() const;

    KMenu *m_viewModeMenu;
    KMenu *m_newLayerMenu;
    KoPartSelectAction *m_partLayerAction;
    KisImageSP m_image;
    WdgLayerBox *m_lst;
};

#endif // KIS_LAYERBOX_H

