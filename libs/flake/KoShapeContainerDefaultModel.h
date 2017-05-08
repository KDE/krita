/* This file is part of the KDE project
 * Copyright (C) 2006-2007, 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2009 Thorsten Zachmann <zachmann@kde.org>
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

#ifndef KOSHAPECONTAINERDEFAULTMODEL_H
#define KOSHAPECONTAINERDEFAULTMODEL_H

#include "KoShapeContainerModel.h"

#include "kritaflake_export.h"

/**
 * A default implementation of the KoShapeContainerModel.
 */
class KRITAFLAKE_EXPORT KoShapeContainerDefaultModel : public KoShapeContainerModel
{
public:
    KoShapeContainerDefaultModel();
    ~KoShapeContainerDefaultModel() override;

    void add(KoShape *shape) override;

    void proposeMove(KoShape *shape, QPointF &move) override;

    void setClipped(const KoShape *shape, bool clipping) override;

    bool isClipped(const KoShape *shape) const override;

    void setInheritsTransform(const KoShape *shape, bool inherit) override;

    bool inheritsTransform(const KoShape *shape) const override;

    void remove(KoShape *shape) override;

    int count() const override;

    QList<KoShape*> shapes() const override;

    bool isChildLocked(const KoShape *child) const override;

    /// empty implementation.
    void containerChanged(KoShapeContainer *container, KoShape::ChangeType type) override;

private:
    class Private;
    Private * const d;
};

#endif
