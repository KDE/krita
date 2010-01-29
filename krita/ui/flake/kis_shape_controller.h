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
#include <QMap>

#include <KoShapeControllerBase.h>

#include "kis_types.h"
#include <krita_export.h>

class KoShape;

class KisView2;
class KisDoc2;
class KisNameServer;
class KoDataCenterBase;

/**
 * KisShapeController keeps track of new layers, shapes, masks and
 * selections -- everything that needs to be wrapped as a shape for
 * the tools to work on.
 */
class KRITAUI_EXPORT KisShapeController : public QObject, public KoShapeControllerBase
{

    Q_OBJECT

public:

    KisShapeController(KisDoc2 * doc, KisNameServer *nameServer);
    ~KisShapeController();

    void setImage(KisImageWSP image);
    KoShape * shapeForNode(KisNodeSP layer) const;
    void setInitialShapeForView(KisView2 * view);
    virtual QMap<QString, KoDataCenterBase *> dataCenterMap() const;

    // Prepares the shape controller to add the next shape to a shape selection
    void prepareAddingSelectionShape();
private:

    void addShape(KoShape* shape);
    void removeShape(KoShape* shape);

private slots:

    friend class KisShapeControllerTest;
    friend class KisDoc2;
    // These slots keep track of changes in the layer stack and make
    // sure that the shape stack doesn't get out of sync
    void slotNodeAdded(KisNode* node, int index);
    void slotNodeRemoved(KisNode*node, int index);
    void slotLayersChanged(KisGroupLayerSP rootLayer);

    void slotNotifySelectionChanged(QList<KoShape*> shapes);

private:

    int layerMapSize();

private:

    class Private;
    Private * const m_d;

};

#endif
