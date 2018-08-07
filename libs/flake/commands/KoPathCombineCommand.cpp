/* This file is part of the KDE project
 * Copyright (C) 2006,2008 Jan Hambrecht <jaham@gmx.net>
 * Copyright (C) 2006,2007 Thorsten Zachmann <zachmann@kde.org>
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

#include "KoPathCombineCommand.h"
#include "KoShapeControllerBase.h"
#include "KoShapeContainer.h"
#include "KoPathShape.h"
#include <klocalizedstring.h>
#include "kis_assert.h"
#include <KoPathPointData.h>

#include <QHash>

class Q_DECL_HIDDEN KoPathCombineCommand::Private
{
public:
    Private(KoShapeControllerBase *c, const QList<KoPathShape*> &p)
        : controller(c), paths(p)
        , combinedPath(0)
        , combinedPathParent(0)
        , isCombined(false)
    {
        foreach (KoPathShape * path, paths) {
            oldParents.append(path->parent());
        }
    }
    ~Private() {
        if (isCombined && controller) {
            Q_FOREACH (KoPathShape* path, paths) {
                delete path;
            }
        } else {
            delete combinedPath;
        }
    }

    KoShapeControllerBase *controller;
    QList<KoPathShape*> paths;
    QList<KoShapeContainer*> oldParents;
    KoPathShape *combinedPath;
    KoShapeContainer *combinedPathParent;

    QHash<KoPathShape*, int> shapeStartSegmentIndex;

    bool isCombined;
};

KoPathCombineCommand::KoPathCombineCommand(KoShapeControllerBase *controller,
        const QList<KoPathShape*> &paths, KUndo2Command *parent)
    : KUndo2Command(kundo2_i18n("Combine paths"), parent)
    , d(new Private(controller, paths))
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(!paths.isEmpty());

    Q_FOREACH (KoPathShape* path, d->paths) {
        if (!d->combinedPath) {
            KoPathShape *clone = dynamic_cast<KoPathShape*>(path->cloneShape());
            KIS_ASSERT_RECOVER_BREAK(clone);

            d->combinedPath = clone;
            d->combinedPathParent = path->parent();
            d->shapeStartSegmentIndex[path] = 0;
        } else {
            const int startSegmentIndex = d->combinedPath->combine(path);
            d->shapeStartSegmentIndex[path] = startSegmentIndex;
        }
    }
}

KoPathCombineCommand::~KoPathCombineCommand()
{
    delete d;
}

void KoPathCombineCommand::redo()
{
    KUndo2Command::redo();
    if (d->paths.isEmpty()) return;

    d->isCombined = true;

    if (d->controller) {
        Q_FOREACH (KoPathShape* p, d->paths) {
            d->controller->removeShape(p);
            p->setParent(0);
        }
        d->combinedPath->setParent(d->combinedPathParent);
        d->controller->addShape(d->combinedPath);
    }
}

void KoPathCombineCommand::undo()
{
    if (! d->paths.size())
        return;

    d->isCombined = false;

    if (d->controller) {
        d->controller->removeShape(d->combinedPath);
        d->combinedPath->setParent(0);

        auto parentIt = d->oldParents.begin();
        Q_FOREACH (KoPathShape* p, d->paths) {
            p->setParent(*parentIt);
            d->controller->addShape(p);
            ++parentIt;
        }
    }
    KUndo2Command::undo();
}

KoPathShape *KoPathCombineCommand::combinedPath() const
{
    return d->combinedPath;
}

KoPathPointData KoPathCombineCommand::originalToCombined(KoPathPointData pd) const
{
    KIS_SAFE_ASSERT_RECOVER_RETURN_VALUE(d->shapeStartSegmentIndex.contains(pd.pathShape), pd);

    const int segmentOffet = d->shapeStartSegmentIndex[pd.pathShape];

    KoPathPointIndex newIndex(segmentOffet + pd.pointIndex.first, pd.pointIndex.second);
    return KoPathPointData(d->combinedPath, newIndex);
}

