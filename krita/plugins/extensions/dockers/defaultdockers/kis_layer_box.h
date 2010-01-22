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
class KisLayerManager;
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

    virtual bool eventFilter(QObject *object, QEvent *event);

    /// reimplemented from KoCanvasObserverBase
    virtual void setCanvas(KoCanvasBase *canvas);

private slots:

    void setImage(KisImageWSP image);

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
    void slotPropertiesClicked();
    void slotDuplicateClicked();

    void slotNewPaintLayer();
    void slotNewGroupLayer();
    void slotNewAdjustmentLayer();
    void slotNewGeneratorLayer();
    void slotNewCloneLayer();
    void slotNewShapeLayer();
    void slotNewTransparencyMask();
    void slotNewEffectMask();
    void slotNewTransformationMask();
    void slotNewSelectionMask();
    void slotCompositeOpChanged(const QString&);
    void slotOpacityChanged(qreal opacity, bool final);
    void slotNodeActivated(const QModelIndex &);

private:

    KisCanvas2* m_canvas;
    KMenu *m_viewModeMenu;
    KMenu *m_newLayerMenu;
    KisImageWSP m_image;
    QPointer<KisNodeModel> m_nodeModel;
    QPointer<KisNodeManager> m_nodeManager;
    Ui_WdgLayerBox* m_wdgLayerBox;

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

