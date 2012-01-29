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

#include <KoShapeBasedDocumentBase.h>

#include "kis_types.h"
#include <krita_export.h>

class KisNodeDummy;
class KoShapeLayer;

class KisView2;
class KisDoc2;
class KisNameServer;
class KoDataCenterBase;

/**
 * KisShapeController keeps track of new layers, shapes, masks and
 * selections -- everything that needs to be wrapped as a shape for
 * the tools to work on.
 */
class KRITAUI_EXPORT KisShapeController : public QObject, public KoShapeBasedDocumentBase
{

    Q_OBJECT

public:

    KisShapeController(KisDoc2 * doc, KisNameServer *nameServer);
    ~KisShapeController();

    void setImage(KisImageWSP image);
    KoShapeLayer* shapeForNode(KisNodeSP layer) const;
    KisNodeDummy* dummyForNode(KisNodeSP layer) const;
    KisNodeDummy* rootDummy() const;
    void setInitialShapeForView(KisView2 * view);



signals:
    /**
     * These two signals are forwarded from the local shape manager of
     * KisShapeLayer. This is done because we switch KoShapeManager and
     * therefore KoSelection in KisCanvas2, so we need to connect local
     * managers to the UI as well.
     *
     * \see comment in the constructor of KisCanvas2
     */
    void selectionChanged();
    void currentLayerChanged(const KoShapeLayer*);

    void sigContinueAddNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);
    void sigContinueMoveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);
    void sigContinueRemoveNode(KisNodeSP node);

    /**
     * This signal is emitted when the shape controller wants to request
     * the change of an active layer. E.g. when a new layer is added or
     * when the root layer of the image is changed. It should be forwarded
     * through a signal to allow queueing and synchronization of threads.
     */
    void sigActivateNode(KisNodeSP node);

    void sigBeginInsertDummy(KisNodeDummy *parent, int index);
    void sigEndInsertDummy(KisNodeDummy *dummy);

    void sigBeginRemoveDummy(KisNodeDummy *dummy);
    void sigEndRemoveDummy();

    void sigDummyChanged(KisNodeDummy *dummy);

protected:
    void addShape(KoShape* shape);
    void removeShape(KoShape* shape);

private slots:
    friend class KisShapeControllerTest;

    void slotNodeAdded(KisNodeSP node);
    void slotNodeMoved(KisNodeSP node);
    void slotRemoveNode(KisNodeSP node);
    void slotLayersChanged();
    void slotNodeChanged(KisNode* node);

    void slotContinueAddNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);
    void slotContinueMoveNode(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);
    void slotContinueRemoveNode(KisNodeSP node);

private:
    static KisNodeSP findFirstLayer(KisNodeSP root);

    int layerMapSize();
    QMap<QString, KoDataCenterBase *> dataCenterMap() const;

private:
    struct Private;
    Private * const m_d;
};

#endif
