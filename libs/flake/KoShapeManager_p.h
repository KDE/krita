/* This file is part of the KDE project

   SPDX-FileCopyrightText: 2006-2008 Thorsten Zachmann <zachmann@kde.org>
   SPDX-FileCopyrightText: 2006-2010 Thomas Zander <zander@kde.org>
   SPDX-FileCopyrightText: 2009-2010 Jan Hambrecht <jaham@gmx.net>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KoShapeManager_p_h
#define KoShapeManager_p_h

#include "KoSelection.h"
#include "KoShape.h"
#include "KoShape_p.h"
#include "KoShapeContainer.h"
#include "KoShapeManager.h"
#include <KoRTree.h>
#include <QMutex>
#include "kis_thread_safe_signal_compressor.h"

class KoCanvasBase;
class KoShapeGroup;
class KoShapePaintingContext;
class QPainter;

class Q_DECL_HIDDEN KoShapeManager::Private
{
public:
    Private(KoShapeManager *shapeManager, KoCanvasBase *c)
        : selection(new KoSelection(shapeManager)),
          canvas(c),
          tree(4, 2),
          q(shapeManager),
          shapeInterface(shapeManager),
          updateCompressor(100, KisSignalCompressor::FIRST_ACTIVE)
    {
    }

    ~Private() {
        delete selection;
    }

    /**
     * Update the tree when there are shapes in m_aggregate4update. This is done so not all
     * updates to the tree are done when they are asked for but when they are needed.
     */
    void updateTree();

    void forwardCompressedUdpate();


    /**
     * Recursively detach the shapes from this shape manager
     */
    void unlinkFromShapesRecursively(const QList<KoShape *> &shapes);

    QList<KoShape *> shapes;
    KoSelection *selection;
    KoCanvasBase *canvas;
    KoRTree<KoShape *> tree;
    QSet<KoShape *> aggregate4update;
    QHash<KoShape*, int> shapeIndexesBeforeUpdate;
    KoShapeManager *q;
    KoShapeManager::ShapeInterface shapeInterface;
    QMutex shapesMutex;
    QMutex treeMutex;

    KisThreadSafeSignalCompressor updateCompressor;
    QRectF compressedUpdate;
    QSet<const KoShape*> compressedUpdatedShapes;

    bool updatesBlocked = false;
};

#endif
