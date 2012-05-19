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
#include <KoCanvasObserverBase.h>

#include <kis_types.h>

#include "kis_view2.h"

class QModelIndex;

typedef QList<QModelIndex> QModelIndexList;

class KMenu;
class KoCompositeOp;
class KisCanvas2;
class KisNodeModel;
class Ui_WdgLayerBox;

/**
 * A widget that visualized the layer structure.
 *
 */
class KisLayerBox : public QDockWidget, public KoCanvasObserverBase
{

    Q_OBJECT

public:

    KisLayerBox();
    virtual ~KisLayerBox();

    /// reimplemented from KoCanvasObserverBase
    virtual void setCanvas(KoCanvasBase *canvas);
    virtual void unsetCanvas();
private slots:

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


    // from the layerbox to the node manager
    void slotRmClicked();
    void slotRaiseClicked();
    void slotLowerClicked();
    void slotLeftClicked();
    void slotRightClicked();
    void slotPropertiesClicked();
    void slotDuplicateClicked();

    void slotMergeLayer();
    void slotNewPaintLayer();
    void slotNewGroupLayer();
    void slotNewAdjustmentLayer();
    void slotNewGeneratorLayer();
    void slotNewCloneLayer();
    void slotNewShapeLayer();
    void slotNewTransparencyMask();
    void slotNewEffectMask();
    void slotNewSelectionMask();
    void slotCompositeOpChanged(int index);
    void slotOpacityChanged();
    void slotOpacitySliderMoved(qreal opacity);

    void slotCollapsed(const QModelIndex &index);
    void slotExpanded(const QModelIndex &index);

private:

    KisCanvas2* m_canvas;
    KMenu *m_viewModeMenu;
    KMenu *m_newLayerMenu;
    KisImageWSP m_image;
    QPointer<KisNodeModel> m_nodeModel;
    QPointer<KisNodeManager> m_nodeManager;
    Ui_WdgLayerBox* m_wdgLayerBox;
    QTimer m_delayTimer;
    int m_newOpacity;

    KAction* m_newPainterLayerAction;
    KAction* m_newGroupLayerAction;
    KAction* m_newCloneLayerAction;
    KAction* m_newShapeLayerAction;
    KAction* m_newAdjustmentLayerAction;
    KAction* m_newGeneratorLayerAction;
    KAction* m_newTransparencyMaskAction;
    KAction* m_newEffectMaskAction;
    KAction* m_newSelectionMaskAction;

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

