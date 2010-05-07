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

#include "kotext_export.h"

class KoTextAnchor;

/**
 *  A model to position children of the text shape.
 * All anchored frames are children of the text shape, and they get positioned
 * by the text layouter (only KWord at this time).
 */
class KOTEXT_EXPORT KoTextShapeContainerModel : public KoShapeContainerModel
{
public:
    /// constructor
    KoTextShapeContainerModel();
    ~KoTextShapeContainerModel();

    /// reimplemented from KoShapeContainerModel
    virtual void add(KoShape *child);
    /// reimplemented from KoShapeContainerModel
    virtual void remove(KoShape *child);
    /// reimplemented from KoShapeContainerModel
    virtual void setClipped(const KoShape *child, bool clipping);
    /// reimplemented from KoShapeContainerModel
    virtual bool isClipped(const KoShape *child) const;
    /// reimplemented from KoShapeContainerModel
    virtual int count() const;
    /// reimplemented from KoShapeContainerModel
    virtual QList<KoShape*> shapes() const;
    /// reimplemented from KoShapeContainerModel
    virtual void containerChanged(KoShapeContainer *container);
    /// reimplemented from KoShapeContainerModel
    virtual void proposeMove(KoShape *child, QPointF &move);
    /// reimplemented from KoShapeContainerModel
    virtual void childChanged(KoShape *child, KoShape::ChangeType type);
    /// reimplemented from KoShapeContainerModel
    virtual bool isChildLocked(const KoShape *child) const;

    /// each child that is added due to being anchored in the text has an anchor; register it for rules based placement.
    void addAnchor(KoTextAnchor *anchor);
    /// When a shape is removed or stops being anchored, remove it.
    void removeAnchor(KoTextAnchor *anchor);

private:
    class Private;
    Private * const d;
};

#endif
