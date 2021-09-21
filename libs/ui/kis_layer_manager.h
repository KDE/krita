/*
 *  SPDX-FileCopyrightText: 2006 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_LAYER_MANAGER
#define KIS_LAYER_MANAGER

#include <QObject>
#include <QPointer>
#include <QList>

#include "kis_adjustment_layer.h"
#include "kis_generator_layer.h"
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
    KisGeneratorLayerSP addGeneratorLayer(KisNodeSP activeNode, const QString &name, KisFilterConfigurationSP filter, KisSelectionSP selection, KisProcessingApplicator *applicator);

    KisNodeSP addFileLayer(KisNodeSP activeNode);

    void layerStyle();

    void changeCloneSource();

    void copyLayerStyle();

    void pasteLayerStyle();

private:
    void adjustLayerPosition(KisNodeSP node, KisNodeSP activeNode, KisNodeSP &parent, KisNodeSP &above);
    void addLayerCommon(KisNodeSP activeNode, KisNodeSP layer, bool updateImage = true, KisProcessingApplicator *applicator = 0);

private:

    KisViewManager * m_view;
    QPointer<KisView>m_imageView {0};

    KisAction *m_imageFlatten {0};
    KisAction *m_imageMergeLayer {0};
    KisAction *m_groupLayersSave {0};
    KisAction *m_convertGroupAnimated {0};
    KisAction *m_imageResizeToLayer {0};
    KisAction *m_flattenLayer {0};
    KisAction *m_rasterizeLayer {0};
    KisNodeCommandsAdapter* m_commandsAdapter;

    KisAction *m_layerStyle {0};
    KisAction *m_copyLayerStyle {0};
    KisAction *m_pasteLayerStyle {0};
};

#endif
