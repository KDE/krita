/*
 *  Copyright (c) 2008 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "kis_painting_assistant.h"

struct KisPaintingAssistant::Private {
    QString id;
    QString name;
    static KisPaintingAssistant* s_currentAssistant;
};

KisPaintingAssistant* KisPaintingAssistant::Private::s_currentAssistant = 0;

KisPaintingAssistant::KisPaintingAssistant(const QString& id, const QString& name) : d(new Private)
{
    d->id = id;
    d->name = name;
}

KisPaintingAssistant::~KisPaintingAssistant()
{
    delete d;
}

const QString& KisPaintingAssistant::id() const
{
    return d->id;
}

const QString& KisPaintingAssistant::name() const
{
    return d->name;
}

KisPaintingAssistant* KisPaintingAssistant::currentAssistant()
{
    return Private::s_currentAssistant;
}

void KisPaintingAssistant::setCurrentAssistant(KisPaintingAssistant* _assistant)
{
    Private::s_currentAssistant = _assistant;
}
