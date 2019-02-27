/*
 *  LayerBox.h - part of Krita aka Krayon aka KimageShop
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
#include <QSlider>

class QModelIndex;

typedef QList<QModelIndex> QModelIndexList;

class QMenu;
class QAbstractButton;
class KoCompositeOp;
class KisCanvas2;
class KisNodeModel;
class KisNodeFilterProxyModel;
class Ui_WdgLayerBox;
class KisNodeJugglerCompressed;
class KisColorLabelSelectorWidget;
class QWidgetAction;
class KisKeyframeChannel;
class KisSelectionActionsAdapter;

/**
 * A widget that shows a visualization of the layer structure.
 *
 * The center of the layer box is KisNodeModel, which shows the actual layers.
 * This widget adds docking functionality and command buttons.
 *
 */
class LayerBox : public QDockWidget, public KisMainwindowObserver
{

    Q_OBJECT

public:

    LayerBox();
    ~LayerBox() override;
    QString observerName() override { return "LayerBox"; }
    /// reimplemented from KisMainwindowObserver
    void setViewManager(KisViewManager* kisview) override;
    void setCanvas(KoCanvasBase *canvas) override;
    void unsetCanvas() override;
private Q_SLOTS:

    void notifyImageDeleted();

    void slotContextMenuRequested(const QPoint &pos, const QModelIndex &index);

    void slotMinimalView();
    void slotDetailedView();
    void slotThumbnailView();

    // From the node manager to the layerbox
    void slotSetCompositeOp(const KoCompositeOp* compositeOp);
    void slotSetOpacity(double opacity);
    void updateUI();
    void setCurrentNode(KisNodeSP node);
    void slotModelReset();


    // from the layerbox to the node manager
    void slotRmClicked();
    void slotRaiseClicked();
    void slotLowerClicked();
    void slotPropertiesClicked();

    void slotCompositeOpChanged(int index);
    void slotOpacityChanged();
    void slotOpacitySliderMoved(qreal opacity);

    void slotCollapsed(const QModelIndex &index);
    void slotExpanded(const QModelIndex &index);

    void slotSelectOpaque();
    void slotNodeCollapsedChanged();

    void slotEditGlobalSelection(bool showSelections);
    void slotRenameCurrentNode();

    void slotAboutToRemoveRows(const QModelIndex &parent, int first, int last);
    void selectionChanged(const QModelIndexList selection);
    void slotNodeManagerChangedSelection(const QList<KisNodeSP> &nodes);
    void slotColorLabelChanged(int index);
    void slotUpdateIcons();

    void slotAddLayerBnClicked();

    void updateThumbnail();
    void updateAvailableLabels();
    void updateLayerFiltering();

    void slotUpdateThumbnailIconSize();


    // Opacity keyframing
    void slotKeyframeChannelAdded(KisKeyframeChannel *channel);
    void slotOpacityKeyframeChanged(KisKeyframeSP keyframe);
    void slotOpacityKeyframeMoved(KisKeyframeSP keyframe, int fromTime);
    void slotImageTimeChanged(int time);

private:
    inline void connectActionToButton(KisViewManager* view, QAbstractButton *button, const QString &id);
    inline void addActionToMenu(QMenu *menu, const QString &id);
    void watchOpacityChannel(KisKeyframeChannel *channel);

    KisNodeSP findNonHidableNode(KisNodeSP startNode);
private:

    QPointer<KisCanvas2> m_canvas;
    QScopedPointer<KisSelectionActionsAdapter> m_selectionActionsAdapter;
    QMenu *m_newLayerMenu;
    KisImageWSP m_image;
    QPointer<KisNodeModel> m_nodeModel;
    QPointer<KisNodeFilterProxyModel> m_filteringModel;
    QPointer<KisNodeManager> m_nodeManager;
    QPointer<KisColorLabelSelectorWidget> m_colorSelector;
    QPointer<QWidgetAction> m_colorSelectorAction;
    Ui_WdgLayerBox* m_wdgLayerBox;
    QTimer m_opacityDelayTimer;
    int m_newOpacity;

    QVector<KisAction*> m_actions;
    KisAction* m_removeAction;
    KisAction* m_propertiesAction;
    KisSignalCompressor m_thumbnailCompressor;
    KisSignalCompressor m_colorLabelCompressor;
    KisSignalCompressor m_thumbnailSizeCompressor;

    QSlider* thumbnailSizeSlider;

    KisNodeSP m_activeNode;
    QPointer<KisKeyframeChannel> m_opacityChannel;
    bool m_blockOpacityUpdate {false};
};

class LayerBoxFactory : public KoDockFactoryBase
{

public:
    LayerBoxFactory() { }

    QString id() const override {
        return QString("LayerBox");
    }

    QDockWidget* createDockWidget() override {
        LayerBox * dockWidget = new LayerBox();

        dockWidget->setObjectName(id());

        return dockWidget;
    }

    DockPosition defaultDockPosition() const override {
        return DockRight;
    }
};

#endif // KIS_LAYERBOX_H

