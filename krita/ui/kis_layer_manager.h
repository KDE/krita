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
#include <krita_export.h>

class QRect;

class KAction;
class QAction;
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
class KRITAUI_EXPORT KisLayerManager : public QObject {

    Q_OBJECT

public:

    KisLayerManager(KisView2 * view,  KisDoc2 * doc);
    ~KisLayerManager();

    void setup(KActionCollection * collection);
    void addAction(QAction * action);

    void updateGUI();

signals:

    /// XXX: Move this to kisview or to kisresourceprovider? (BSAR)
    void currentColorSpaceChanged(KoColorSpace * cs);

public slots:

    void imgResizeToActiveLayer();

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
    void rotateLayer( double radians );
    void shearLayer(double angleX, double angleY)        ;
    void flattenImage();
    void mergeLayer();

    void layersUpdated();

    void saveLayerAsImage();
    bool activeLayerHasSelection();

private:

    KisView2 * m_view;
    KisDoc2 * m_doc;

    QList<QAction*> m_pluginActions;

    QAction *m_imgFlatten;
    QAction *m_imgMergeLayer;
    QAction * m_actionAdjustmentLayer;
    QAction *m_layerAdd;
    QAction *m_layerBottom;
    QAction *m_layerDup;
    KToggleAction *m_layerHide;
    QAction *m_layerLower;
    QAction *m_layerProperties;
    QAction *m_layerRaise;
    QAction *m_layerRm;
    QAction *m_layerSaveAs;
    QAction *m_layerTop;
    bool m_actLayerVis;
    QAction *m_imgResizeToLayer;
};

#endif
