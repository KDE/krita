/* This file is part of the KDE project
 * SPDX-FileCopyrightText: 2007 Jan Hambrecht <jaham@gmx.net>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KoPathReverseCommand.h"
#include "KoPathShape.h"
#include <klocalizedstring.h>

class Q_DECL_HIDDEN KoPathReverseCommand::Private
{
public:
    Private(const QList<KoPathShape*> &p)
            : paths(p) {
    }
    ~Private() {
    }

    void reverse() {
        if (! paths.size())
            return;

        Q_FOREACH (KoPathShape* shape, paths) {
            int subpathCount = shape->subpathCount();
            for (int i = 0; i < subpathCount; ++i)
                shape->reverseSubpath(i);
        }
    }

    QList<KoPathShape*> paths;
};

KoPathReverseCommand::KoPathReverseCommand(const QList<KoPathShape*> &paths, KUndo2Command *parent)
        : KUndo2Command(parent),
        d(new Private(paths))
{
    setText(kundo2_i18n("Reverse paths"));
}

KoPathReverseCommand::~KoPathReverseCommand()
{
    delete d;
}

void KoPathReverseCommand::redo()
{
    KUndo2Command::redo();

    d->reverse();
}

void KoPathReverseCommand::undo()
{
    KUndo2Command::undo();

    d->reverse();
}
