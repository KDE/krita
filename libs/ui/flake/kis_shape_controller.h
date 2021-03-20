/*
 *  SPDX-FileCopyrightText: 2007 Boudewijn Rempt <boud@valdyas.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SHAPE_CONTROLLER
#define KIS_SHAPE_CONTROLLER

#include <QMap>

#include "kis_dummies_facade_base.h"
#include <KoShapeControllerBase.h>


class KisNodeDummy;
class KoShapeLayer;

class KisCanvas2;
class KisDocument;
class KisNameServer;
class KUndo2Stack;

/**
 * KisShapeController keeps track of new layers, shapes, masks and
 * selections -- everything that needs to be wrapped as a shape for
 * the tools to work on.
 */
class KRITAUI_EXPORT KisShapeController : public KisDummiesFacadeBase, public KoShapeControllerBase
{

    Q_OBJECT

public:

    KisShapeController(KisNameServer *nameServer, KUndo2Stack *undoStack, QObject *parent = 0);
    ~KisShapeController() override;

    bool hasDummyForNode(KisNodeSP node) const override;
    KisNodeDummy* dummyForNode(KisNodeSP layer) const override;
    KisNodeDummy* rootDummy() const override;
    int dummiesCount() const override;

    KoShapeLayer* shapeForNode(KisNodeSP layer) const;
    void setInitialShapeForCanvas(KisCanvas2 *canvas);

    void setImage(KisImageWSP image) override;


private:
    void addNodeImpl(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis) override;
    void removeNodeImpl(KisNodeSP node) override;

private Q_SLOTS:
    void slotUpdateDocumentResolution();
    void slotUpdateDocumentSize();

Q_SIGNALS:
    /**
     * These three signals are forwarded from the local shape manager of
     * KisShapeLayer. This is done because we switch KoShapeManager and
     * therefore KoSelection in KisCanvas2, so we need to connect local
     * managers to the UI as well.
     *
     * \see comment in the constructor of KisCanvas2
     */
    void selectionChanged();
    void selectionContentChanged();
    void currentLayerChanged(const KoShapeLayer*);

public:
    KoShapeContainer* createParentForShapes(const QList<KoShape*> shapes, KUndo2Command *parentCommand) override;

    QRectF documentRectInPixels() const override;
    qreal pixelsPerInch() const override;

private:
    struct Private;
    Private * const m_d;
};

#endif
