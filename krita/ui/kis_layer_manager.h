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

class QRect;

class KAction;
class KToggleAction;
class KActionCollection;

class KoPartSelectAction;
class KoCompositeOp;
class KoDocumentEntry;
class KoColorSpace;

class KisDoc2;
class KisFilterStrategy;
class KisView2;
class KisFilterConfiguration;

/**
   KisLayerManager takes care of the gui around working with layers:
   adding, removing, editing.
*/
class KisLayerManager : public QObject {

    Q_OBJECT

public:

    KisLayerManager(KisView2 * view,  KisDoc2 * doc);
    ~KisLayerManager();

    void setup(KActionCollection * collection);
    void addAction(KAction * action);

signals:

    /// XXX: Move this to kisview or to kisresourceprovider? (BSAR)
    void currentColorSpaceChanged(KoColorSpace * cs);

public slots:

    // Calls the updateGUI methods of the other managers: the layer
    // manager is leading.
    void updateGUI(bool enable);

    void layerCompositeOp(const KoCompositeOp* compositeOp);
    void layerOpacity(int opacity, bool dontundo);
    void layerOpacityFinishedChanging(int previous, int opacity);
    void layerToggleVisible();
    void layerToggleLocked();
    void actLayerVisChanged(int show);
    void layerProperties();
    void showLayerProperties(KisLayerSP layer);

    void layerAdd();
    void addLayer(KisGroupLayerSP parent, KisLayerSP above);

    void addGroupLayer(KisGroupLayerSP parent, KisLayerSP above);

    void insertPart( const QRect& viewRect, const KoDocumentEntry& entry, KisGroupLayerSP parent, KisLayerSP above );
    void addPartLayer();
    void addPartLayer(KisGroupLayerSP parent, KisLayerSP above, const KoDocumentEntry& entry);
    void reconnectAfterPartInsert();

    void addAdjustmentLayer();
    void addAdjustmentLayer(KisGroupLayerSP parent, KisLayerSP above);
    void addAdjustmentLayer(KisGroupLayerSP parent, KisLayerSP above, const QString & name, KisFilterConfiguration * filter, KisSelectionSP selection);

    void layerRemove();
    void layerDuplicate();
    void layerRaise();
    void layerLower();
    void layerFront();
    void layerBack();

    void rotateLayer180();
    void rotateLayerLeft90();
    void rotateLayerRight90();
    void mirrorLayerX();
    void mirrorLayerY();
    void scaleLayer( double sx, double sy, KisFilterStrategy *filterStrategy );
    void rotateLayer( double angle );
    void shearLayer(double angleX, double angleY)        ;
    void flattenImage();
    void mergeLayer();

    void layersUpdated();

    void saveLayerAsImage();
    bool activeLayerHasSelection();
    void handlePartLayerAdded(KisLayerSP layer);

private:

    KisView2 * m_view;
    KisDoc2 * m_doc;

    QList<KAction*> m_pluginActions;

    KAction *m_imgFlatten;
    KAction *m_imgMergeLayer;
    KoPartSelectAction * m_actionPartLayer;
    KAction * m_actionAdjustmentLayer;
    KAction *m_layerAdd;
    KAction *m_layerBottom;
    KAction *m_layerDup;
    KToggleAction *m_layerHide;
    KAction *m_layerLower;
    KAction *m_layerProperties;
    KAction *m_layerRaise;
    KAction *m_layerRm;
    KAction *m_layerSaveAs;
    KAction *m_layerTop;
    bool m_actLayerVis;
};

#endif
