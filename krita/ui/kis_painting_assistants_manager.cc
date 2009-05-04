/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
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

#include "kis_painting_assistants_manager.h"

#include <cfloat>

#include <QList>
#include <QPointF>

#include "kis_painting_assistant.h"

struct KisPaintingAssistantsManager::Private {
    QList<KisPaintingAssistant*> assistants;
};

KisPaintingAssistantsManager::KisPaintingAssistantsManager() : d(new Private)
{
}

KisPaintingAssistantsManager::~KisPaintingAssistantsManager()
{
    qDeleteAll(d->assistants.begin(), d->assistants.end());
}

void KisPaintingAssistantsManager::addAssistant(KisPaintingAssistant* assistant) {
    if(d->assistants.contains(assistant)) return;
    d->assistants.push_back(assistant);
}

void KisPaintingAssistantsManager::removeAssistant(KisPaintingAssistant* assistant) {
    d->assistants.removeAll(assistant);
}

QPointF KisPaintingAssistantsManager::adjustPosition(const QPointF& point) const
{
    if(d->assistants.empty()) return point;
    if(d->assistants.count() == 1) return d->assistants.first()->adjustPosition(point);
    QPointF best;
    double distance = DBL_MAX;
    foreach(const KisPaintingAssistant* assistant, d->assistants)
    {
        QPointF pt = assistant->adjustPosition(point);
        double d = qAbs(pt.x() - point.x()) + qAbs(pt.y() - point.y());
        if( d < distance ) {
            best = point;
            distance = d;
        }
    }
}
