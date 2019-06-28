/* This file is part of the KDE project
 * Copyright (C) 2009 Thomas Zander <zander@kde.org>
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
#ifndef KOSHAPECONTAINERPRIVATE_H
#define KOSHAPECONTAINERPRIVATE_H

#include "KoShapeContainer.h"
#include "kritaflake_export.h"

#include <QSharedData>

class KoShapeContainerModel;

/**
 * \internal used private d-pointer class for the \a KoShapeContainer class.
 */
class KRITAFLAKE_EXPORT KoShapeContainer::Private : public QSharedData
{
public:
    explicit Private(KoShapeContainer *q);
    virtual ~Private();

    Private(const Private &rhs, KoShapeContainer *q);

    KoShapeContainer::ShapeInterface shapeInterface;
    KoShapeContainerModel *model;
};

#endif
