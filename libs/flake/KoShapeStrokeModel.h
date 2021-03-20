/* This file is part of the KDE project
 *
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 * SPDX-FileCopyrightText: 2007, 2009 Thorsten Zachmann <zachmann@kde.org>
 * SPDX-FileCopyrightText: 2012 Inge Wallin <inge@lysator.liu.se>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPESTROKEMODEL_H
#define KOSHAPESTROKEMODEL_H

#include "kritaflake_export.h"

#include <QtGlobal>

class KoShape;
class KoShapeSavingContext;
struct KoInsets;

class QColor;
class QPainter;

/**
 * A model for strokes of KoShapes.
 * Classes that implement this model will be allowed to draw the stroke of the outline
 * of a shape.
 * Note that since the important members take a KoShape as argument it is possible,
 * and preferred behavior, to have one instance of a stroke that is reused on several
 * objects.
 */
class KRITAFLAKE_EXPORT KoShapeStrokeModel
{
public:
    virtual ~KoShapeStrokeModel();

    /**
     * Return a strokeInsets object filled with the size inside the shape that this stroke takes.
     * @param shape the shape the insets will be calculated for
     * @param insets the insets object that will be filled and returned.
     */
    virtual void strokeInsets(const KoShape *shape, KoInsets &insets) const = 0;

    /**
     * Return a maximum distance that the markers of the shape can take outside the
     * shape itself
     */
    virtual qreal strokeMaxMarkersInset(const KoShape *shape) const = 0;

    /**
     * Returns true if there is some transparency, false if the stroke is fully opaque.
     * @return if the stroke is transparent.
     */
    virtual bool hasTransparency() const = 0;
    /**
     * Paint the stroke.
     * This method should paint the stroke around shape.
     * @param shape the shape to paint around
     * @param painter the painter to paint to, the painter will have the topleft of the
     *       shape as its start coordinate.
     */
    virtual void paint(const KoShape *shape, QPainter &painter) const = 0;

    virtual bool compareFillTo(const KoShapeStrokeModel *other) = 0;
    virtual bool compareStyleTo(const KoShapeStrokeModel *other) = 0;
    virtual bool isVisible() const = 0;
};

#endif
