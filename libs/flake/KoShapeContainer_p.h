/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2009 Thomas Zander <zander@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
*/
#ifndef KOSHAPECONTAINERPRIVATE_H
#define KOSHAPECONTAINERPRIVATE_H

#include "KoShapeContainer.h"
#include "kritaflake_export.h"

class KoShapeContainerModel;

/**
 * \internal used private d-pointer class for the \a KoShapeContainer class.
 */
class KRITAFLAKE_EXPORT KoShapeContainer::Private
{
public:
    explicit Private(KoShapeContainer *q);
    virtual ~Private();

    Private(const Private &rhs, KoShapeContainer *q);

    KoShapeContainer::ShapeInterface shapeInterface;
    KoShapeContainerModel *model;
};

#endif
