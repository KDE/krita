/* This file is part of the KDE project
 * Copyright (C) 2006-2007 Thomas Zander <zander@kde.org>
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

#ifndef KOCHILDRENDATA_H
#define KOCHILDRENDATA_H

#include "KoShapeContainerModel.h"

#include "flake_export.h"

class FLAKE_EXPORT KoChildrenData : public KoShapeContainerModel
{
public:
    KoChildrenData();
    ~KoChildrenData();

    /// reimplemented
    virtual void add(KoShape *child);

    /// reimplemented
    void proposeMove( KoShape *shape, QPointF &move );

    /// reimplemented
    virtual void setClipping(const KoShape *child, bool clipping);

    /// reimplemented
    virtual bool childClipped(const KoShape *child) const;

    // reimplemented
    virtual void remove(KoShape *child);

    /// reimplemented
    virtual int count() const;

    /// reimplemented
    virtual QList<KoShape*> childShapes() const;

    /// reimplemented
    virtual bool isChildLocked(const KoShape *child) const;

    /// reimplemented
    virtual void containerChanged(KoShapeContainer *);

private:
    class Private;
    Private * const d;
};

#endif /* KOCHILDRENDATA_H */
