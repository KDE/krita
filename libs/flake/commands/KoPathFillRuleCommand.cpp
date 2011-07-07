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

#include "KoPathFillRuleCommand.h"
#include "KoPathShape.h"

#include <klocale.h>

class KoPathFillRuleCommand::Private
{
public:
    Private(Qt::FillRule fillRule) : newFillRule(fillRule) {
    }

    QList<KoPathShape*> shapes;       ///< the shapes to set fill rule for
    QList<Qt::FillRule> oldFillRules; ///< the old fill rules, one for each shape
    Qt::FillRule newFillRule;         ///< the new fill rule to set
};

KoPathFillRuleCommand::KoPathFillRuleCommand(const QList<KoPathShape*> &shapes, Qt::FillRule fillRule, KUndo2Command *parent)
        : KUndo2Command(parent)
        , d(new Private(fillRule))
{
    d->shapes = shapes;
    foreach(KoPathShape *shape, d->shapes)
        d->oldFillRules.append(shape->fillRule());

    setText(i18nc("(qtundo-format)", "Set fill rule"));
}

KoPathFillRuleCommand::~KoPathFillRuleCommand()
{
    delete d;
}

void KoPathFillRuleCommand::redo()
{
    KUndo2Command::redo();
    foreach(KoPathShape *shape, d->shapes) {
        shape->setFillRule(d->newFillRule);
        shape->update();
    }
}

void KoPathFillRuleCommand::undo()
{
    KUndo2Command::undo();
    QList<Qt::FillRule>::iterator ruleIt = d->oldFillRules.begin();
    foreach(KoPathShape *shape, d->shapes) {
        shape->setFillRule(*ruleIt);
        shape->update();
        ++ruleIt;
    }
}
