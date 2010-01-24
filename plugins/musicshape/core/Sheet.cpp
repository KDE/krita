/* This file is part of the KDE project
 * Copyright (C) 2007 Marijn Kruisselbrink <m.kruisselbrink@student.tue.nl>
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
#include "Sheet.h"
#include "Part.h"
#include "PartGroup.h"
#include "Bar.h"
#include "StaffSystem.h"
#include "Staff.h"

#include <QtCore/QList>

namespace MusicCore {

class Sheet::Private
{
public:
    QList<Part*> parts;
    QList<PartGroup*> partGroups;
    QList<Bar*> bars;
    QList<StaffSystem*> staffSystems;
};

Sheet::Sheet(QObject* parent) : QObject(parent), d(new Private)
{
}

Sheet::~Sheet()
{
    delete d;
}

int Sheet::partCount() const
{
    return d->parts.size();
}

Part* Sheet::part(int index)
{
    Q_ASSERT( index >= 0 && index < partCount() );
    return d->parts[index];
}

int Sheet::partIndex(Part* part)
{
    return d->parts.indexOf(part);
}

Part* Sheet::addPart(const QString& name)
{
    Part* part = new Part(this, name);
    d->parts.append(part);
    emit partAdded(d->parts.size(), part);
    return part;
}

void Sheet::addPart(Part* part)
{
    Q_ASSERT( part );
    part->setParent(this);
    d->parts.append(part);
    emit partAdded(d->parts.size(), part);
}

Part* Sheet::insertPart(int before, const QString& name)
{
    Q_ASSERT( before >= 0 && before <= partCount() );
    Part* part = new Part(this, name);
    d->parts.insert(before, part);
    emit partAdded(before, part);
    return part;
}

void Sheet::insertPart(int before, Part* part)
{
    Q_ASSERT( before >= 0 && before <= partCount() );
    Q_ASSERT( part );
    part->setParent(this);
    d->parts.insert(before, part);
    emit partAdded(before, part);
}

void Sheet::removePart(int index, bool deletePart)
{
    Q_ASSERT( index >= 0 && index < partCount() );
    Part* part = d->parts.takeAt(index);
    emit partRemoved(index, part);
    if (deletePart) {
        delete part;
    }
}

void Sheet::removePart(Part* part, bool deletePart)
{
    Q_ASSERT( part && part->sheet() == this);
    int index = d->parts.indexOf(part);
    Q_ASSERT( index != -1 );
    removePart(index, deletePart);
}

int Sheet::partGroupCount() const
{
    return d->partGroups.size();
}

PartGroup* Sheet::partGroup(int index)
{
    Q_ASSERT( index >= 0 && index < partGroupCount() );
    return d->partGroups[index];
}

PartGroup* Sheet::addPartGroup(int firstPart, int lastPart)
{
    Q_ASSERT( firstPart >= 0 && firstPart < partCount() );
    Q_ASSERT( lastPart >= 0 && lastPart < partCount() );
    PartGroup *group = new PartGroup(this, firstPart, lastPart);
    d->partGroups.append(group);
    return group;
}

void Sheet::removePartGroup(PartGroup* group, bool deleteGroup)
{
    Q_ASSERT( group && group->sheet() == this );
    int index = d->partGroups.indexOf(group);
    Q_ASSERT( index != -1 );
    d->partGroups.removeAt(index);
    if (deleteGroup) {
        delete group;
    }
}

int Sheet::barCount() const
{
    return d->bars.size();
}

Bar* Sheet::bar(int index)
{
    Q_ASSERT( index >= 0 && index < barCount() );
    return d->bars[index];
}

int Sheet::indexOfBar(Bar* bar)
{
    Q_ASSERT( bar );
    return d->bars.indexOf(bar);
}

void Sheet::addBars(int count)
{
    for (int i = 0; i < count; i++) {
	d->bars.append(new Bar(this));
    }
}

Bar* Sheet::addBar()
{
    Bar* bar = new Bar(this);
    d->bars.append(bar);
    return bar;
}

Bar* Sheet::insertBar(int before)
{
    Q_ASSERT( before >= 0 && before <= barCount() );
    Bar* bar = new Bar(this);
    d->bars.insert(before, bar);
    return bar;
}

void Sheet::insertBar(int before, Bar* bar)
{
    Q_ASSERT( before >= 0 && before <= barCount() );
    d->bars.insert(before, bar);
}

void Sheet::removeBar(int index, bool deleteBar)
{
    Q_ASSERT( index >= 0 && index < barCount() );
    Bar* bar = d->bars.takeAt(index);
    if (deleteBar) {
        delete bar;
    }
}

void Sheet::removeBars(int index, int count, bool deleteBar)
{
    Q_ASSERT( index >= 0 && count > 0 && index + count <= barCount() );
    for (int i = 0; i < count; i++) {
        Bar* b = d->bars.takeAt(index);
        if (deleteBar) {
            delete b;
        }
    }
}

StaffSystem* Sheet::staffSystem(int index)
{
    Q_ASSERT( index >= 0 );
    int idx = d->staffSystems.size();
    qreal ssHeight = 0;
    if (partCount() > 0) {
        Part* prt = part(partCount() - 1);
        ssHeight = prt->staff(prt->staffCount() - 1)->bottom() + 30;
    }
    while (index >= d->staffSystems.size()) {
        StaffSystem *ss = new StaffSystem(this);
        ss->setHeight(ssHeight);
        if (idx > 0 && partCount() > 0) {
            Part* prt = part(partCount() - 1);
            ss->setTop(d->staffSystems[idx-1]->top() + prt->staff(prt->staffCount() - 1)->bottom() + 30);
        }
        d->staffSystems.append(ss);
        idx++;
    }
    return d->staffSystems[index];
}

void Sheet::setStaffSystemCount(int count)
{
    Q_ASSERT( count >= 0 );
    while (count < d->staffSystems.size()) {
        d->staffSystems.removeLast();
    }
}

int Sheet::staffSystemCount()
{
    return d->staffSystems.size();
}

void Sheet::updateAccidentals()
{
    foreach (Part* part, d->parts) {
        for (int i = 0; i < part->staffCount(); i++) {
            part->staff(i)->updateAccidentals();
        }
    }
}

} // namespace MusicCore

#include <Sheet.moc>
