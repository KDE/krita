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
#include "Part.h"
#include "Staff.h"
#include "Voice.h"
#include "Sheet.h"
#include <QtCore/QList>

namespace MusicCore {

class Part::Private
{
public:
    QString name;
    QString shortName;
    QList<Staff*> staves;
    QList<Voice*> voices;
};

Part::Part(Sheet* sheet, const QString& name) : QObject(sheet), d(new Private)
{
    d->name = name;
}

Part::~Part()
{
    delete d;
}

Sheet* Part::sheet()
{
    return qobject_cast<Sheet*>(parent());
}

QString Part::name() const
{
    return d->name;
}

void Part::setName(const QString& name)
{
    if (d->name == name) return;
    d->name = name;
    emit nameChanged(name);
    if (d->shortName.isNull()) emit shortNameChanged(name);
}

QString Part::shortName(bool useFull) const
{
    if (d->shortName.isNull() && useFull) {
        return d->name;
    } else {
        return d->shortName;
    }
}

void Part::setShortName(const QString& name)
{
    if (d->shortName == name) return;
    d->shortName = name;
    emit shortNameChanged(shortName());
}

int Part::staffCount() const
{
    return d->staves.size();
}

Staff* Part::staff(int index)
{
    Q_ASSERT( index >= 0 && index < staffCount() );
    return d->staves[index];
}

Staff* Part::addStaff()
{
    Staff* staff = new Staff(this);
    d->staves.append(staff);
    return staff;
}

void Part::addStaff(Staff* staff)
{
    Q_ASSERT( staff );
    d->staves.append(staff);
}

Staff* Part::insertStaff(int before)
{
    Q_ASSERT( before >= 0 && before <= staffCount() );
    Staff* staff = new Staff(this);
    d->staves.insert(before, staff);
    return staff;
}

int Part::indexOfStaff(Staff* staff)
{
    Q_ASSERT(staff);
    return d->staves.indexOf(staff);
}

void Part::removeStaff(Staff* staff, bool deleteStaff)
{
    Q_ASSERT(staff);
    d->staves.removeAll(staff);
    if (deleteStaff) delete staff;
}

int Part::voiceCount() const
{
    return d->voices.size();
}

Voice* Part::voice(int index)
{
    Q_ASSERT( index >= 0 && index < voiceCount() );
    return d->voices[index];
}

Voice* Part::addVoice()
{
    Voice* voice = new Voice(this);
    d->voices.append(voice);
    return voice;
}

int Part::indexOfVoice(Voice* voice)
{
    Q_ASSERT(voice);
    return d->voices.indexOf(voice);
}

} // namespace

#include <Part.moc>
