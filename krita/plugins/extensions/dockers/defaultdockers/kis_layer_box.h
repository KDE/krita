/*
 *  kis_layer_box.h - part of Krita aka Krayon aka KimageShop
 *
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (C) 2006 Gábor Lehel <illissius@gmail.com>
 *  Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *  Copyright (C) 2007-2009 Boudewijn Rempt <boud@valdyas.org>
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
#include <QDockWidget>
#include <QPointer>
#include <QTimer>

#include <kis_debug.h>

#include <KoColorSpace.h>
#include <KoDockFactoryBase.h>

#include <kis_types.h>

#include "kis_action.h"
#include "KisViewManager.h"
#include "kis_mainwindow_observer.h"
#include "kis_signal_compressor.h"

class QModelIndex;

typedef QList<QModelIndex> QModelIndexList;

class QMenu;
class QAbstractButton;
class KoCompositeOp;
class KisCanvas2;
class KisNodeModel;
class Ui_WdgLayerBox;
class KisNodeJugglerCompressed;

/**
 * A widget that shows a visualization of the layer structure.
 * 
 * The center of the layer box is KisNodeModel, which shows the actual layers.
 * This widget adds docking functionality and command buttons.
 *
 */
class KisLayerBox : public QDockWidget, public KisMainwindowObserver
{

    Q_OBJECT

public:

    KisLayerBox();
    virtual ~KisLayerBox();
    QString observerName() { return "KisLayerBox"; }
    /// reimplemented from KisMainwindowObserver
    virtual void setMainWindow(KisViewManager* kisview);
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas();
private Q_SLOTS:

    void notifyImageDeleted();

    void slotContextMenuRequested(const QPoint &pos, const QModelIndex &index);

    void slotMinimalView();
    void slotDetailedView();
    void slotThumbnailView();

    // From the node manager to the layerbox
    void slotSetCompositeOp(const KoCompositeOp* compositeOp);
    void slotSetOpacity(double opacity);
    void slotFillCompositeOps(const KoColorSpace * colorSpace);
    void updateUI();
    void setCurrentNode(KisNodeSP node);
    void slotModelReset();


    // from the layerbox to the node manager
    void slotRmClicked();
    void slotRaiseClicked();
    void slotLowerClicked();
    void slotPropertiesClicked();

    void slotMergeLayer();
    void slotCompositeOpChanged(int index);
    void slotOpacityChanged();
    void slotOpacitySliderMoved(qreal opacity);

    void slotCollapsed(const QModelIndex &index);
    void slotExpanded(const QModelIndex &index);

    void slotSelectOpaque();
    void slotNodeCollapsedChanged();

    void slotEditGlobalSelection(bool showSelections);

    void selectionChanged(const QModelIndexList selection);
    void slotNodeManagerChangedSelection(const QList<KisNodeSP> &nodes);

    void updateThumbnail();

private:
    inline void connectActionToButton(KisViewManager* view, QAbstractButton *button, const QString &id);
    inline void addActionToMenu(QMenu *menu, const QString &id);

    KisNodeSP findNonHidableNode(KisNodeSP startNode);
private:

    KisCanvas2* m_canvas;
    QMenu *m_viewModeMenu;
    QMenu *m_newLayerMenu;
    KisImageWSP m_image;
    QPointer<KisNodeModel> m_nodeModel;
    QPointer<KisNodeManager> m_nodeManager;
    Ui_WdgLayerBox* m_wdgLayerBox;
    QTimer m_opacityDelayTimer;
    int m_newOpacity;

    QVector<KisAction*> m_actions;
    KisAction* m_removeAction;
    KisAction* m_propertiesAction;
    KisAction* m_selectOpaque;
    QPointer<KisNodeJugglerCompressed> m_nodeJuggler;
    KisSignalCompressor m_thumbnailCompressor;
};

class KisLayerBoxFactory : public KoDockFactoryBase
{

public:
    KisLayerBoxFactory() { }

    virtual QString id() const {
        return QString("KisLayerBox");
    }

    virtual QDockWidget* createDockWidget() {
        KisLayerBox * dockWidget = new KisLayerBox();

        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const {
        return DockRight;
    }
};



#endif // KIS_LAYERBOX_H

