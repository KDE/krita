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
#include "Bar.h"
#include "VoiceBar.h"
#include "StaffElement.h"
#include "Sheet.h"

#include <QtCore/QHash>
#include <QtCore/QList>

namespace MusicCore {

class Bar::Private
{
public:
    QHash<Voice*, VoiceBar*> voices;
    QPointF position;
    qreal size;
    qreal naturalSize;
    qreal sizeFactor;
    qreal prefix;
    QPointF prefixPosition;
    QList<StaffElement*> staffElements;
};

Bar::Bar(Sheet* sheet) : QObject(sheet), d(new Private)
{
    d->size = 0;
    setSize(100);
    d->prefix = 0;
    d->sizeFactor = 2.0;
}

Bar::~Bar()
{
    delete d;
}

Sheet* Bar::sheet()
{
    return qobject_cast<Sheet*>(parent());
}

VoiceBar* Bar::voice(Voice* voice)
{
    VoiceBar* vb = d->voices.value(voice);
    if (!vb) {
        vb = new VoiceBar(this);
        d->voices.insert(voice, vb);
    }
    return vb;
}

QPointF Bar::position() const
{
    return d->position;
}

void Bar::setPosition(const QPointF& position, bool setPrefix)
{
    if (d->position == position) return;
    d->position = position;
    if (setPrefix) {
        setPrefixPosition(position - QPointF(prefix(), 0));
    }
    emit positionChanged(position);
}

qreal Bar::size() const
{
    return d->size;
}

void Bar::setSize(qreal size)
{
    if (d->size == size) return;
    d->size = size;
    emit sizeChanged(size);
}

qreal Bar::naturalSize() const
{
    return d->size;
}

qreal Bar::prefix() const
{
    return d->prefix;
}

void Bar::setPrefix(qreal prefix)
{
    d->prefix = prefix;
}

QPointF Bar::prefixPosition() const
{
    return d->prefixPosition;
}

void Bar::setPrefixPosition(const QPointF& position)
{
    d->prefixPosition = position;
}

int Bar::staffElementCount(Staff* staff) const
{
    Q_ASSERT( staff );
    int cnt = 0;
    foreach (StaffElement* e, d->staffElements) {
        if (e->staff() == staff) {
            cnt++;
        }
    }
    return cnt;
}

StaffElement* Bar::staffElement(Staff* staff, int index)
{
    Q_ASSERT( staff );
    int cnt = 0;
    foreach (StaffElement* e, d->staffElements) {
        if (e->staff() == staff) {
            if (cnt == index) return e;
            cnt++;
        }
    }
    Q_ASSERT( false );
    return 0;
}

int Bar::indexOfStaffElement(StaffElement* element)
{
    Q_ASSERT( element );
    return d->staffElements.indexOf(element);
}

void Bar::addStaffElement(StaffElement* element, int index)
{
    Q_ASSERT( element );
    element->setBar(this);
    if (index >= 0) {
        bool correct = true;
        if (index > 0) {
            StaffElement* se = d->staffElements[index-1];
            if (se->startTime() > element->startTime()) correct = false;
        }
        if (index < d->staffElements.size()) {
            StaffElement* se = d->staffElements[index];
            if (se->startTime() < element->startTime()) correct = false;
        }
        
        if (correct) {
            d->staffElements.insert(index, element);
            return;
        }
    }
    
    for (int i = 0; i < d->staffElements.size(); i++) {
        StaffElement* se = d->staffElements[i];
        if (se->startTime() > element->startTime() || (se->startTime() == element->startTime() && se->priority() < element->priority())) {
            d->staffElements.insert(i, element);
            return;
        }
    }
    d->staffElements.append(element);
}

void Bar::removeStaffElement(StaffElement* element, bool deleteElement)
{
    Q_ASSERT( element );
    int index = d->staffElements.indexOf(element);
    Q_ASSERT( index != -1 );
    d->staffElements.removeAt(index);
    if (deleteElement) {
        delete element;
    }
}

qreal Bar::sizeFactor() const
{
    return d->sizeFactor;
}

} // namespace MusicCore

#include <Bar.moc>
