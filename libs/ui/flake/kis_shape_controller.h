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

#include <QMap>

#include "kis_dummies_facade_base.h"
#include <KoShapeControllerBase.h>


class KisNodeDummy;
class KoShapeLayer;

class KisCanvas2;
class KisDocument;
class KisNameServer;

/**
 * KisShapeController keeps track of new layers, shapes, masks and
 * selections -- everything that needs to be wrapped as a shape for
 * the tools to work on.
 */
class KRITAUI_EXPORT KisShapeController : public KisDummiesFacadeBase, public KoShapeControllerBase
{

    Q_OBJECT

public:

    KisShapeController(KisDocument *doc, KisNameServer *nameServer);
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
    void addShapes(const QList<KoShape*> shapes) override;
    void removeShape(KoShape* shape) override;

    QRectF documentRectInPixels() const override;
    qreal pixelsPerInch() const override;

private:
    struct Private;
    Private * const m_d;
};

#endif
