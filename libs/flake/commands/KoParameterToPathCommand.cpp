/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2006, 2008 Jan Hambrecht <jaham@gmx.net>
 * SPDX-FileCopyrightText: 2006, 2007 Thorsten Zachmann <zachmann@kde.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoParameterToPathCommand.h"
#include "KoPathPoint.h"
#include "KoParameterShape.h"
#include <klocalizedstring.h>

class KoParameterToPathCommandPrivate
{
public:
    ~KoParameterToPathCommandPrivate() {
        qDeleteAll(copies);
    }
    void initialize();
    void copyPath(KoPathShape *destination, KoPathShape *source);
    QList<KoParameterShape*> shapes;
    QList<KoPathShape*> copies;
};

KoParameterToPathCommand::KoParameterToPathCommand(KoParameterShape *shape, KUndo2Command *parent)
    : KUndo2Command(parent),
    d(new KoParameterToPathCommandPrivate())
{
    d->shapes.append(shape);
    d->initialize();
    setText(kundo2_i18n("Convert to Path"));
}

KoParameterToPathCommand::KoParameterToPathCommand(const QList<KoParameterShape*> &shapes, KUndo2Command *parent)
    : KUndo2Command(parent),
    d(new KoParameterToPathCommandPrivate())
{
    d->shapes = shapes;
    d->initialize();
    setText(kundo2_i18n("Convert to Path"));
}

KoParameterToPathCommand::~KoParameterToPathCommand()
{
    delete d;
}

void KoParameterToPathCommand::redo()
{
    KUndo2Command::redo();
    for (int i = 0; i < d->shapes.size(); ++i) {
        KoParameterShape *parameterShape = d->shapes.at(i);
        parameterShape->update();
        parameterShape->setParametricShape(false);
        parameterShape->update();
    }
}

void KoParameterToPathCommand::undo()
{
    KUndo2Command::undo();
    for (int i = 0; i < d->shapes.size(); ++i) {
        KoParameterShape * parameterShape = d->shapes.at(i);
        parameterShape->update();
        parameterShape->setParametricShape(true);
        d->copyPath(parameterShape, d->copies[i]);
        parameterShape->update();
    }
}

void KoParameterToPathCommandPrivate::initialize()
{
    Q_FOREACH (KoParameterShape *shape, shapes) {
        KoPathShape *p = new KoPathShape();
        copyPath(p, shape);
        copies.append(p);
    }
}

void KoParameterToPathCommandPrivate::copyPath(KoPathShape *destination, KoPathShape *source)
{
    destination->clear();

    int subpathCount = source->subpathCount();
    for (int subpathIndex = 0; subpathIndex < subpathCount; ++subpathIndex) {
        int pointCount = source->subpathPointCount(subpathIndex);
        if (! pointCount)
            continue;

        KoSubpath * subpath = new KoSubpath;
        for (int pointIndex = 0; pointIndex < pointCount; ++pointIndex) {
            KoPathPoint * p = source->pointByIndex(KoPathPointIndex(subpathIndex, pointIndex));
            KoPathPoint * c = new KoPathPoint(*p, destination);
            subpath->append(c);
        }
        destination->addSubpath(subpath, subpathIndex);
    }
    destination->setTransformation(source->transformation());
}
