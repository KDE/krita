/* This file is part of the KDE project
 * Copyright (C) 2010 Thomas Zander <zander@kde.org>
 * Copyright (C) 2010 KO GmbH <boud@kogmbh.com>
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
#ifndef KOTEXTONSHAPECONTAINER_P_H
#define KOTEXTONSHAPECONTAINER_P_H

#include <KoShapeContainer_p.h>
#include <KoTextOnShapeContainer.h>
#include "SimpleShapeContainerModel.h"

class KoTextOnShapeContainerPrivate : public KoShapeContainerPrivate
{
public:
    KoTextOnShapeContainerPrivate(KoShapeContainer *q);
    virtual ~KoTextOnShapeContainerPrivate();

    KoShape *textShape;
    KoTextOnShapeContainer::ResizeBehavior resizeBehavior;
};

class KoTextOnShapeContainerModel : public SimpleShapeContainerModel
{
public:
    KoTextOnShapeContainerModel(KoTextOnShapeContainer *qq, KoTextOnShapeContainerPrivate *containerData);
    virtual void containerChanged(KoShapeContainer *container, KoShape::ChangeType type);
    virtual void proposeMove(KoShape *child, QPointF &move);
    virtual void childChanged(KoShape *child, KoShape::ChangeType type);
    bool inheritsTransform(const KoShape *) const;

    KoTextOnShapeContainer *q;
    KoTextOnShapeContainerPrivate *containerData;
    bool lock;
};

#endif // KOTEXTONSHAPECONTAINER_P_H
