/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  this program is free software; you can redistribute it and/or modify
 *  it under the terms of the gnu general public license as published by
 *  the free software foundation; either version 2 of the license, or
 *  (at your option) any later version.
 *
 *  this program is distributed in the hope that it will be useful,
 *  but without any warranty; without even the implied warranty of
 *  merchantability or fitness for a particular purpose.  see the
 *  gnu general public license for more details.
 *
 *  you should have received a copy of the gnu general public license
 *  along with this program; if not, write to the free software
 *  foundation, inc., 675 mass ave, cambridge, ma 02139, usa.
 */
#ifndef KIS_SHAPE_CONTROLLER
#define KIS_SHAPE_CONTROLLER

#include <QObject>

#include <KoShapeControllerBase.h>

#include "kis_types.h"

class KoShape;

class KisView2;
class KisDoc2;
class KisNameServer;
/**
 * KisShapeController keeps track of new layers, shapes, masks and
 * selections -- everything that needs to be wrapped as a shape for
 * the tools to work on.
 */
class KisShapeController : public QObject, public KoShapeControllerBase {

    Q_OBJECT

public:

    KisShapeController( KisDoc2 * doc, KisNameServer *nameServer);
    ~KisShapeController();

    void setImage( KisImageSP image );

    void addShape( KoShape* shape );
    void removeShape( KoShape* shape );

    void setInitialShapeForView( KisView2 * view );

private slots:

    // These slots keep track of changes in the layer stack and make
    // sure that the shape stack doesn't get out of sync
    void slotLayerAdded( KisLayerSP layer );
    void slotLayerRemoved( KisLayerSP layer,  KisGroupLayerSP wasParent,  KisLayerSP wasAboveThis );
    void slotLayerMoved( KisLayerSP layer,  KisGroupLayerSP previousParent, KisLayerSP wasAboveThis );
    void slotLayersChanged( KisGroupLayerSP rootLayer );
    void slotLayerActivated( KisLayerSP layer );

    // XXX: The same is necessary for selections, masks etc.
private:

    KoShape * activeLayerShape();


private:

    class Private;
    Private * const m_d;

};

#endif
