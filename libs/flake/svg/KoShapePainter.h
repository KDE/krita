/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPEPAINTER_H
#define KOSHAPEPAINTER_H

#include <QList>
#include <QRectF>
#include "kritaflake_export.h"

class KoShape;
class QPainter;
class QImage;

/**
 * A utility class to paint a subset of shapes onto a QPainter.
 * Notice that using setShapes repeatedly is very expensive, as it populates
 * the shapeManager and all its caching every time.  If at all possible use
 * a shapeManager directly and avoid losing the cache between usages.
 */
class KRITAFLAKE_EXPORT KoShapePainter
{
public:
    explicit KoShapePainter();
    ~KoShapePainter();

    /**
     * Sets the shapes to be painted.
     * @param shapes the shapes to paint
     */
    void setShapes(const QList<KoShape*> &shapes);

    /**
     * Paints the shapes on the given painter and using the zoom handler.
     * @param painter the painter to paint on
     * @param converter the view converter defining the zoom to use
     */
    void paint(QPainter &painter);

    /**
     * Paints the shapes on the given painter.
     * The given document rectangle is painted to fit into the given painter rectangle.
     *
     * @param painter the painter to paint on
     * @param painterRect the destination rectangle on the painter
     * @param documentRect the document region to paint
     */
    void paint(QPainter &painter, const QRect &painterRect, const QRectF &documentRect);

    /**
     * Paints shapes to the given image, so that all shapes fit onto it.
     * @param image the image to paint into
     * @return false if image is empty, else true
     */
    void paint(QImage &image);

    /// Returns the bounding rect of the shapes to paint
    QRectF contentRect() const;

private:
    class Private;
    Private * const d;
};

#endif // KOSHAPEPAINTER_H
