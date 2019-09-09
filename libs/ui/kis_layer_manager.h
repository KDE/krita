/*
 *  Copyright (C) 2006 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_LAYER_MANAGER
#define KIS_LAYER_MANAGER

#include <QObject>
#include <QPointer>
#include <QList>

#include "kis_adjustment_layer.h"
#include "kis_types.h"
#include "KisView.h"
#include <filter/kis_filter_configuration.h>

class KisViewManager;
class KisNodeCommandsAdapter;
class KisAction;
class KisActionManager;
class KisProcessingApplicator;

/**
 * KisLayerManager takes care of the gui around working with layers:
 * adding, removing, editing. It also keeps track of the active layer
 * for this view.
 */
class KisLayerManager : public QObject
{

    Q_OBJECT

public:

    KisLayerManager(KisViewManager * view);
    ~KisLayerManager() override;
    void setView(QPointer<KisView>view);

Q_SIGNALS:

    void sigLayerActivated(KisLayerSP layer);

private:

    friend class KisNodeManager;

    /**
     * Activate the specified layer. The layer may be 0.
     */
    void activateLayer(KisLayerSP layer);

    KisLayerSP activeLayer();
    KisPaintDeviceSP activeDevice();


    void setup(KisActionManager *actionManager);

    void updateGUI();

private Q_SLOTS:

    void mergeLayer();

    void imageResizeToActiveLayer();
    void trimToImage();

    void layerProperties();

    void flattenImage();

    void flattenLayer();
    void rasterizeLayer();

    void layersUpdated();

    void saveGroupLayers();
    bool activeLayerHasSelection();

    void convertNodeToPaintLayer(KisNodeSP source);
    void convertGroupToAnimated();

    void convertLayerToFileLayer(KisNodeSP source);

    KisLayerSP addPaintLayer(KisNodeSP activeNode);
    KisNodeSP addGroupLayer(KisNodeSP activeNode);

    KisNodeSP addCloneLayer(KisNodeList nodes);

    KisNodeSP addShapeLayer(KisNodeSP activeNode);

    KisNodeSP addAdjustmentLayer(KisNodeSP activeNode);
    KisAdjustmentLayerSP addAdjustmentLayer(KisNodeSP activeNode, const QString & name, KisFilterConfigurationSP  filter, KisSelectionSP selection, KisProcessingApplicator *applicator);

    KisNodeSP addGeneratorLayer(KisNodeSP activeNode);

    KisNodeSP addFileLayer(KisNodeSP activeNode);

    void layerStyle();

    void changeCloneSource();

private:
    void adjustLayerPosition(KisNodeSP node, KisNodeSP activeNode, KisNodeSP &parent, KisNodeSP &above);
    void addLayerCommon(KisNodeSP activeNode, KisNodeSP layer, bool updateImage = true, KisProcessingApplicator *applicator = 0);

private:

    KisViewManager * m_view;
    QPointer<KisView>m_imageView;

    KisAction *m_imageFlatten;
    KisAction *m_imageMergeLayer;
    KisAction *m_groupLayersSave;
    KisAction *m_convertGroupAnimated;
    KisAction *m_imageResizeToLayer;
    KisAction *m_flattenLayer;
    KisAction *m_rasterizeLayer;
    KisNodeCommandsAdapter* m_commandsAdapter;

    KisAction *m_layerStyle;
};

#endif
