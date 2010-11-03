/* This file is part of the KDE project
 * Copyright (C) 2010 KO GmbH <boud@kogmbh.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (  at your option ) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "KoShapeManagerCachedPaintingStrategy.h"

#include <kdebug.h>

#include "KoShape.h"
#include "KoShape_p.h"
#include "KoShapeManager.h"
#include "KoCanvasBase.h"
#include "KoViewConverter.h"
#include "KoFilterEffectStack.h"

#include <QApplication>
#include <QDesktopWidget>
#include <QPainter>
#include <QPixmap>
#include <QPixmapCache>
#include <QTransform>
#include <QRegion>

/*
  Notes:

  We have two coordinate systems:

  * shape coordinate system: in pts. There are two origins: the internal 0,0 origin which is the
    top-left of the cache pixmap, and the position of the shape on the canvas.
  * view coordinate system: pts per inch multiplied by zoom

  The shape registers with areas of the shape's surface are exposed in shape update events. This uses
  the 0,0 = top-left origin and is registered in shape coordinates. We try not to paint outside those
  update events.

  The shapemanager has a painter and the painter has a clipregion in view coordinates. We cannot paint
  outside them and should be careful not to do that once the shape cache has been initialized.

  Qt's QGraphicsView only paints the cache of the exposed parts of the shape and does not care about
  the QPainter's clipRegion. But it does not do a full initialization at the current zoomlevel of
  the shape's cache, which is what we should do.

 */


/// Returns the bounding rect of the shape including all the filters
/// This has an origin of 0,0
static QRectF shapeBoundingRect(KoShape *shape)
{
    // First step, compute the rectangle used for the pixmap if there are effects
    // -- they make the pixmap bigger. And then convert clip region to view coordinates.

    QRectF rc;

    if (shape->filterEffectStack() && !shape->filterEffectStack()->isEmpty()) {
        rc = shape->filterEffectStack()->clipRectForBoundingRect(QRectF(QPointF(), shape->size()));
    }
    else {
        rc = QRectF(QPointF(), shape->size());
    }
    return rc;
}

/// Create an empty, transparent pixmap
static QPixmap createCachePixmap(KoShape *shape, const KoViewConverter &viewConverter)
{
    QRectF zoomedClipRect = viewConverter.documentToView(shapeBoundingRect(shape));

    // Initialize the buffer image
    QPixmap pix(zoomedClipRect.size().toSize());
    pix.fill(Qt::transparent);

    kDebug(30006) << "Created empty pixmap for shape "
             << shape << shape->shapeId()
             << "size" << pix.size();
    return pix;
}

/**
 * Ask the shape manager to paint (part of) the current shape onto the pixmap.
 * We need to setup the painter clipping very carefully to make this work correctly.
 *
 * @param shape the shape that is going to be painted
 * @shapeManager the shapeManager that provides the logic for painting shapes
 *               and their associated effects
 * @param viewConverter convert between view and shape (document coordinates)
 * @param pixmapExposed the exposed region of the shape in document coordinates
 * @param pix the cached pixmap
 */
static void paintIntoCache(KoShape *shape,
                           KoShapeManager *shapeManager,
                           const KoViewConverter &viewConverter,
                           QRegion pixmapExposed,
                           QPixmap *pix)
{
    kDebug(30006) << "paintIntoCache. Shape:" << shape << "Zoom:" << viewConverter.zoom() << "clip" << pixmapExposed.boundingRect();
    QPainter pixmapPainter(pix);
    pixmapPainter.setClipRegion(pixmapExposed);
    pixmapPainter.fillRect(pixmapExposed.boundingRect(), Qt::transparent);
    shapeManager->paintShape(shape, pixmapPainter, viewConverter, false);
}

// QRectF::intersects() returns false always if either the source or target
// rectangle's width or height are 0. This works around that problem.
static inline void adjustRect(QRectF *rect)
{
    Q_ASSERT(rect);
    if (!rect->width()) {
        rect->adjust(qreal(-0.00001), 0, qreal(0.00001), 0);
    }
    if (!rect->height()) {
        rect->adjust(0, qreal(-0.00001), 0, qreal(0.00001));
    }
}

KoShapeManagerCachedPaintingStrategy::KoShapeManagerCachedPaintingStrategy(KoShapeManager *shapeManager)
    : KoShapeManagerPaintingStrategy(shapeManager)
{
}

KoShapeManagerCachedPaintingStrategy::~KoShapeManagerCachedPaintingStrategy()
{
}

void KoShapeManagerCachedPaintingStrategy::paint(KoShape *shape, QPainter &painter, const KoViewConverter &viewConverter, bool forPrint)
{
    kDebug(30006) << ">>>>>>>>>>>>>>>>>>>>>>>>>>>\n\tshape" << shape->shapeId() << "viewConverter zoom" << viewConverter.zoom();
    kDebug(30006) << "\tpainter clipregion" << painter.clipRegion().boundingRect();

    if (!shapeManager()) {
        return;
    }

    KoShape::CacheMode cacheMode = shape->cacheMode();

    // For printing, we never use the cache. It would be in the wrong resolution.
    if (cacheMode == KoShape::NoCache || forPrint) {
        painter.save();
        painter.setTransform(shape->absoluteTransformation(&viewConverter) * painter.transform());
        shapeManager()->paintShape(shape, painter, viewConverter, forPrint);
        painter.restore();  // for the matrix
        return;
    }

    if (cacheMode == KoShape::ScaledCache) {

        // Shape's (local) bounding rect in document coordinates
        QRectF boundingRect = shapeBoundingRect(shape);
        QRectF adjustedBoundingRect(boundingRect);
        adjustRect(&adjustedBoundingRect);
        if (adjustedBoundingRect.isEmpty()) {
            return;
        }

        // transpose to boundingrect to where in the document the shape is, to
        // make it comparable to the exposed area of the painter.
        adjustedBoundingRect.translate(shape->position());

        kDebug(30006) << "\tshape->boundingRect" << boundingRect <<
                    "\n\tadjustedrect" << adjustedBoundingRect;

        // view coordinates
        QRectF painterClippingBounds = painter.clipRegion().boundingRect();

        // document coordinates
        QRectF painterShapeClippingBounds = viewConverter.viewToDocument(painterClippingBounds);

        kDebug(30006) << "\tpainterClippingBounds in view coordinates:" << painterClippingBounds
                    << "in document coordinates:" << painterShapeClippingBounds;

        QRectF exposedRect = adjustedBoundingRect.intersected(painterShapeClippingBounds);
        if (exposedRect.isEmpty()) {
            kDebug(30006) << "\t\tNo exposed area for shape" << shape << shape->shapeId();
            return;
        }

        kDebug(30006) << "\tIntersection of shape and painter clipping in document coordinates" << exposedRect;

        // get or create the devicedata object
        KoShapeCache *shapeCache = shape->d_ptr->shapeCache();

        if (!shapeCache->deviceData.contains(shapeManager())) {
            shapeCache->deviceData[shapeManager()] = new KoShapeCache::DeviceData();
        }
        KoShapeCache::DeviceData *deviceData = shapeCache->deviceData[shapeManager()];

        kDebug(30006) << "\tdevicedata all exposed" << deviceData->allExposed
                 << "exposed rects" << deviceData->exposed.size();

        // Find pixmap in cache.
        QPixmap pix;
        bool pixmapFound = QPixmapCache::find(deviceData->key, &pix);

        kDebug(30006) << "Found pixmap" << pixmapFound << "size" << pix.size();

        bool pixModified = false;

        QSize viewShapeSize = viewConverter.documentToView(adjustedBoundingRect).toRect().size();
        kDebug(30006) << "\tShape size in view coordinates"  << viewShapeSize << "cached pixmap size" << pix.size();

        // some parts or all of this shape need to repainted
        if (deviceData->allExposed                // complete repaint
                || !deviceData->exposed.isEmpty() // part update of the shape requested
                || viewShapeSize != pix.size())   // zoomlevel changed
        {
            kDebug(30006) << "\tSome or all parts of this shape need to be repainted";

            // We know that we will modify the pixmap, removing it from the cache.
            // This will detach the one we have and avoid a deep copy.
            if (pixmapFound) {
                QPixmapCache::remove(deviceData->key);
            }

            // Auto-adjust the pixmap size because the shape seems to have grown or shrunk.
            if (viewShapeSize != pix.size()) {
                kDebug(30006) << "\tAuto-adjust the pixmap size because the shape seems to have grown or shrunk." << viewShapeSize << pix.size();
                // exposed needs to cover the whole pixmap
                pix = createCachePixmap(shape, viewConverter);
                pixModified = true;
                deviceData->allExposed = true;
                deviceData->exposed.clear();
            }

            // Will contain the exposed rects that overlap the painter's clipregion's rects in view
            // coordinates, translated  to the shape position = 0,0
            QRegion pixmapExposed;

            // this vector contains the areas that the shape wanted repainted but that are
            // outside the current qpainter's exposed regions' bounds.
            QVector<QRectF> remainingExposed;

            if (deviceData->allExposed) {
                // For now, repaint the whole shape, unclipped by the painterclipregion.
                // XXX: optimize this by applying the painter clip region.
                QRect rc(QPoint(0,0), viewShapeSize);

                kDebug(30006) << "\tFully exposed, do not clip, repainting " << rc;

                pixmapExposed += rc;

                deviceData->allExposed = false;
                deviceData->exposed.clear();
            }
            else {
                // get the bounding rect for the clip region
                QRectF painterClipRegionBounds = painterClippingBounds.translated(-viewConverter.documentToView(shape->position()));

                kDebug(30006) << "\tTrying to do partial updates within bounds" << painterClipRegionBounds;

                const QVector<QRectF> &exposed = deviceData->exposed;
                for (int i = 0; i < exposed.size(); ++i) {
                    QRectF rc = exposed.at(i);
                    rc = viewConverter.documentToView(rc);
                    kDebug(30006) << "\t\tExposed rect in view coor:" << rc << "in doc coor" << exposed.at(i);
                    if (rc.intersects(painterClipRegionBounds)) {
                        pixmapExposed += rc.toRect().adjusted(-1, -1, 1, 1);
                    }
                    else {
                        // this is exposed but not yet visible, so we do not cache it
                        remainingExposed << rc;
                    }
                }
                deviceData->allExposed = false;
                deviceData->exposed = remainingExposed;
            }

            // Render the exposed areas.
            paintIntoCache(shape, shapeManager(), viewConverter, pixmapExposed, &pix);

            // Reset expose data.
            pixModified = true;

        }

        if (pixModified) {
            // Insert this pixmap into the cache
            kDebug(30006) << "\t PIX MODIFIED";
            deviceData->key = QPixmapCache::insert(pix);
        }

        // Paint the cache on the original painter
        painter.drawPixmap(viewConverter.documentToView(adjustedBoundingRect.topLeft()), pix);
    }

    kDebug(30006) << "<<<<<<<<<<<<<<<<<<<<<<<<<<<";
}

void KoShapeManagerCachedPaintingStrategy::adapt(KoShape *shape, QRectF &rect)
{
    Q_UNUSED(shape);
    Q_UNUSED(rect);
}
