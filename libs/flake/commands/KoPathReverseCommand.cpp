/* This file is part of the KDE project
 * Copyright (C) 2007 Jan Hambrecht <jaham@gmx.net>
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

        foreach(KoPathShape* shape, paths) {
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
