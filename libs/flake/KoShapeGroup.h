/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef KOSHAPEGROUP_H
#define KOSHAPEGROUP_H

#include "KoShapeContainer.h"

#include <QList>

#include "kritaflake_export.h"

class KoShapeSavingContext;
class KoShapeLoadingContext;
class KoShapeGroupPrivate;

/**
 * Provide grouping for shapes.
 * The group shape allows you to add children which will then be grouped in selections
 * and actions.
 * <p>If you have a set of shapes that together make up a bigger shape it is often
 * useful to group them together so the user will perceive the different shapes as
 * actually being one.  This means that if the user clicks on one shape, all shapes
 * in the group will be selected at once, making the tools that works on
 * selections alter all of them at the same time.
 *
 * <p>Note that while this object is also a shape, it is not actually visible and the user
 * can't interact with it.
 *
 * <p>WARNING: this class is NOT threadsafe, it caches the size in an unsafe way
 */
class KRITAFLAKE_EXPORT KoShapeGroup : public KoShapeContainer
{
public:
    /// Constructor
    KoShapeGroup();
    /// destructor
    ~KoShapeGroup() override;

    KoShape* cloneShape() const override;

    /// This implementation is empty since a group is itself not visible.
    void paintComponent(QPainter &painter, KoShapePaintingContext &paintcontext) const override;
    /// always returns false since the group itself can't be selected or hit
    bool hitTest(const QPointF &position) const override;
    QSizeF size() const override;
    void setSize(const QSizeF &size) override;
    QRectF outlineRect() const override;
    /// a group's boundingRect
    QRectF boundingRect() const override;

private:
    friend class ShapeGroupContainerModel;

    /**
     * @brief Invalidate the size cache of the group
     *
     * The group shape caches the size of itself as it can be quite expensive to recalculate
     * the size if there are a lot of subshapes. This function is called when the cache needs
     * to be invalidated.
     */
    void invalidateSizeCache();

private:
    KoShapeGroup(const KoShapeGroup &rhs);

private:
    void tryUpdateCachedSize() const;

    void shapeChanged(ChangeType type, KoShape *shape = 0) override;

    class Private;
    QScopedPointer<Private> d;
};

#endif
