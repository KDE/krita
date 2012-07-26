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
#include <QList>

#include "kis_types.h"


class KAction;
class QAction;
class KToggleAction;
class KActionCollection;

class KoCompositeOp;
class KoColorSpace;

class KisDoc2;
class KisFilterStrategy;
class KisView2;
class KisFilterConfiguration;
class KisNodeCommandsAdapter;


/**
 * KisLayerManager takes care of the gui around working with layers:
 * adding, removing, editing. It also keeps track of the active layer
 * for this view.
 */
class KisLayerManager : public QObject
{

    Q_OBJECT

public:

    KisLayerManager(KisView2 * view,  KisDoc2 * doc);
    ~KisLayerManager();
signals:

    void sigLayerActivated(KisLayerSP layer);

private:
    
    friend class KisNodeManager;
    
    /**
     * Activate the specified layer. The layer may be 0.
     */
    void activateLayer(KisLayerSP layer);

    KisLayerSP activeLayer();
    KisPaintDeviceSP activeDevice();
    
    
    void setup(KActionCollection * collection);
    void addAction(QAction * action);

    void updateGUI();
    

    void scaleLayer(double sx, double sy, KisFilterStrategy *filterStrategy);
    void rotateLayer(double radians);
    void shearLayer(double angleX, double angleY);
    
private slots:

    void mergeLayer();
    
    void imageResizeToActiveLayer();

    void actLayerVisChanged(int show);
    void layerProperties();

    void layerRemove();
    void layerDuplicate();
    void layerRaise();
    void layerLower();
    void layerFront();
    void layerBack();

    void flattenImage();
    
    void flattenLayer();
    void rasterizeLayer();

    void layersUpdated();

    void saveLayerAsImage();
    void saveGroupLayers();
    bool activeLayerHasSelection();

    void layerAdd();
    void addLayer(KisNodeSP parent, KisNodeSP above);
    void addGroupLayer(KisNodeSP parent, KisNodeSP above);

    void addCloneLayer();
    void addCloneLayer(KisNodeSP parent, KisNodeSP above);

    void addShapeLayer();
    void addShapeLayer(KisNodeSP parent, KisNodeSP above);

    void addAdjustmentLayer();
    void addAdjustmentLayer(KisNodeSP parent, KisNodeSP above);
    KisAdjustmentLayerSP addAdjustmentLayer(KisNodeSP parent, KisNodeSP above, const QString & name, KisFilterConfiguration * filter, KisSelectionSP selection);

    void addGeneratorLayer();
    void addGeneratorLayer(KisNodeSP parent, KisNodeSP above);
    void addGeneratorLayer(KisNodeSP parent, KisNodeSP above, const QString & name, KisFilterConfiguration * filter, KisSelectionSP selection);


private:

    KisView2 * m_view;
    KisDoc2 * m_doc;

    QList<QAction*> m_pluginActions;

    KAction *m_imageFlatten;
    KAction *m_imageMergeLayer;
    KAction *m_layerSaveAs;
    KAction *m_groupLayersSave;
    bool m_actLayerVis;
    KAction *m_imageResizeToLayer;
    KAction *m_flattenLayer;
    KAction *m_rasterizeLayer;
    KAction *m_duplicateLayer;
    KAction *m_addPaintLayer;
    KisLayerSP m_activeLayer;
    KisNodeCommandsAdapter* m_commandsAdapter;
};

#endif
