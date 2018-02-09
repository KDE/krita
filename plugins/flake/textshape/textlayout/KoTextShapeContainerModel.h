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

#include "kritatextlayout_export.h"

class KoShapeAnchor;
class KoShapeContainer;

/**
 *  A model to position children of the text shape.
 * All anchored frames are children of the text shape, and they get positioned
 * by the text layouter.
 */
class KRITATEXTLAYOUT_EXPORT KoTextShapeContainerModel : public KoShapeContainerModel
{
public:
    /// constructor
    KoTextShapeContainerModel();
    ~KoTextShapeContainerModel() override;

    /// reimplemented from KoShapeContainerModel
    void add(KoShape *child) override;
    /// reimplemented from KoShapeContainerModel
    void remove(KoShape *child) override;
    /// reimplemented from KoShapeContainerModel
    void setClipped(const KoShape *child, bool clipping) override;
    /// reimplemented from KoShapeContainerModel
    bool isClipped(const KoShape *child) const override;
    /// reimplemented from KoShapeContainerModel
    int count() const override;
    /// reimplemented from KoShapeContainerModel
    QList<KoShape*> shapes() const override;
    /// reimplemented from KoShapeContainerModel
    void containerChanged(KoShapeContainer *container, KoShape::ChangeType type) override;
    /// reimplemented from KoShapeContainerModel
    void proposeMove(KoShape *child, QPointF &move) override;
    /// reimplemented from KoShapeContainerModel
    void childChanged(KoShape *child, KoShape::ChangeType type) override;
    /// reimplemented from KoShapeContainerModel
    void setInheritsTransform(const KoShape *shape, bool inherit) override;
    /// reimplemented from KoShapeContainerModel
    bool inheritsTransform(const KoShape *shape) const override;

    /// each child that is added due to being anchored in the text has an anchor; register it for rules based placement.
    void addAnchor(KoShapeAnchor *anchor);
    /// When a shape is removed or stops being anchored, remove it.
    void removeAnchor(KoShapeAnchor *anchor);

private:
    // reset child position and relayout shape to which this shape is linked
    void relayoutInlineObject(KoShape *child);

    class Private;
    Private * const d;
};

#endif
