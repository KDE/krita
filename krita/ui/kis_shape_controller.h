/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
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
#ifndef KIS_SHAPE_CONTROLLER
#define KIS_SHAPE_CONTROLLER

#include <QObject>

#include <KoShapeControllerBase.h>

#include "kis_types.h"
#include <krita_export.h>

class KoShape;

class KisView2;
class KisDoc2;
class KisNameServer;

/**
 * KisShapeController keeps track of new layers, shapes, masks and
 * selections -- everything that needs to be wrapped as a shape for
 * the tools to work on.
 */
class KRITAUI_EXPORT KisShapeController : public QObject, public KoShapeControllerBase {

    Q_OBJECT

public:

    KisShapeController( KisDoc2 * doc, KisNameServer *nameServer);
    ~KisShapeController();

    void setImage( KisImageSP image );
    void addShape( KoShape* shape );
    void removeShape( KoShape* shape );

    void setInitialShapeForView( KisView2 * view );

    KoShape * shapeForLayer( KisLayerSP layer );

private slots:

    friend class KisShapeControllerTest;

    // These slots keep track of changes in the layer stack and make
    // sure that the shape stack doesn't get out of sync
    void slotLayerAdded( KisLayerSP layer );
    void slotLayerRemoved( KisLayerSP layer,  KisGroupLayerSP wasParent,  KisLayerSP wasAboveThis );
    void slotLayerMoved( KisLayerSP layer,  KisGroupLayerSP previousParent, KisLayerSP wasAboveThis );
    void slotLayersChanged( KisGroupLayerSP rootLayer );

    void slotMaskAdded( KisMaskSP mask );
    void slotMaskRemoved( KisMaskSP mask, KisLayerSP wasParent,  KisMaskSP wasAboveThis );
    void slotMaskMoved( KisMaskSP mask, KisLayerSP previousParent, KisMaskSP wasAboveThis );
    void slotMasksChanged( KisLayerSP layer );

private:

    int layerMapSize();
    int maskMapSize();


private:

    class Private;
    Private * const m_d;

};

#endif
