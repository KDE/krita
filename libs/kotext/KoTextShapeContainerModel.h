/* This file is part of the KDE project
 * Copyright (C) 2007 Thomas Zander <zander@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
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
#ifndef KOTEXTSHAPECONTAINERMODEL_H
#define KOTEXTSHAPECONTAINERMODEL_H

#include <KoShapeContainerModel.h>
#include <KoShapeContainer.h>

#include <kotext_export.h>

class KoTextAnchor;

/// A model to position children of the text shape.
class KOTEXT_EXPORT KoTextShapeContainerModel : public KoShapeContainerModel {
public:
    KoTextShapeContainerModel();

    /// reimplemented from KoShapeContainerModel
    virtual void add(KoShape *child);
    /// reimplemented from KoShapeContainerModel
    virtual void remove(KoShape *child);
    /// reimplemented from KoShapeContainerModel
    virtual void setClipping(const KoShape *child, bool clipping);
    /// reimplemented from KoShapeContainerModel
    virtual bool childClipped(const KoShape *child) const;
    /// reimplemented from KoShapeContainerModel
    virtual int count() const;
    /// reimplemented from KoShapeContainerModel
    virtual QList<KoShape*> iterator() const;
    /// reimplemented from KoShapeContainerModel
    virtual void containerChanged(KoShapeContainer *container);

    virtual void proposeMove(KoShape *child, QPointF &move);

    /// each child that is added due to being anchored in the text has an anchor; register it for rules based placement.
    void addAnchor(KoTextAnchor *anchor);
    /// When a shape is removed or stops being anchored, remove it.
    void removeAnchor(KoTextAnchor *anchor);

    /// Check the anchor rules and move the shape to the right place.
    void reposition(KoShape *shape);

private:
    class Private;
    Private * const d;
};

#endif
