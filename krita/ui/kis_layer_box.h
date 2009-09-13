/*
 *  kis_layer_box.h - part of Krita aka Krayon aka KimageShop
 *
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (C) 2006 Gábor Lehel <illissius@gmail.com>
 *  Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *  Copyright (C) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#include <KoDockFactory.h>
#include <KoCanvasObserver.h>

#include <kis_types.h>

#include "kis_view2.h"

class QModelIndex;

typedef QList<QModelIndex> QModelIndexList;

class KMenu;
class KoCompositeOp;
class KisNodeModel;
class KisLayerManager;
class Ui_WdgLayerBox;

/**
 * A widget that visualized the layer structure.
 *
 */
class KisLayerBox : public QDockWidget, public KoCanvasObserver
{

    Q_OBJECT

public:
    KisLayerBox();
    virtual ~KisLayerBox();

    void setUpdatesAndSignalsEnabled(bool enable);
    void setImage(KisNodeManager * nodeManager, KisImageSP image, KisNodeModel * nodeModel);

    virtual bool eventFilter(QObject *object, QEvent *event);

    /// reimplemented from KoCanvasObserver
    virtual void setCanvas(KoCanvasBase *canvas);
public slots:

    void slotSetCompositeOp(const KoCompositeOp* compositeOp);
    void slotSetOpacity(double opacity);
    void slotFillCompositeOps(const KoColorSpace * colorSpace);
    void updateUI();
    void setCurrentNode(KisNodeSP node);

signals:

    // XXX: create a node factory and a node factory registry in for now just
    //      use strings
    void sigRequestNewNode(const QString & nodetype);
    void sigRequestNodeProperties(KisNodeSP node);
    void sigOpacityChanged(qreal opacity, bool final);
    void sigItemComposite(const KoCompositeOp* _compositeOp);

private slots:

    void slotContextMenuRequested(const QPoint &pos, const QModelIndex &index);

    void slotMinimalView();
    void slotDetailedView();
    void slotThumbnailView();

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

    void slotNodeActivated(const QModelIndex &);

private:
    QModelIndexList selectedNodes() const;

    KMenu *m_viewModeMenu;
    KMenu *m_newLayerMenu;
    KisImageSP m_image;
    QPointer<KisNodeModel> m_nodeModel;
    QPointer<KisNodeManager> m_nodeManager;
    Ui_WdgLayerBox* m_wdgLayerBox;

};

class KisLayerBoxFactory : public KoDockFactory
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

