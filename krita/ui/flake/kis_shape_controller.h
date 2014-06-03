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
#include <KoShapeBasedDocumentBase.h>


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
class KRITAUI_EXPORT KisShapeController : public KisDummiesFacadeBase, public KoShapeBasedDocumentBase
{

    Q_OBJECT

public:

    KisShapeController(KisDoc2 * doc, KisNameServer *nameServer);
    ~KisShapeController();

    bool hasDummyForNode(KisNodeSP node) const;
    KisNodeDummy* dummyForNode(KisNodeSP layer) const;
    KisNodeDummy* rootDummy() const;
    int dummiesCount() const;

    KoShapeLayer* shapeForNode(KisNodeSP layer) const;
    void setInitialShapeForView(KisView2 * view);

private:
    void addNodeImpl(KisNodeSP node, KisNodeSP parent, KisNodeSP aboveThis);
    void removeNodeImpl(KisNodeSP node);

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

protected:
    void addShape(KoShape* shape);
    void removeShape(KoShape* shape);

private:
    struct Private;
    Private * const m_d;
};

#endif
