/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
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

#include "kis_recorded_action.h"

#include <QDomDocument>
#include <QDomElement>
#include <QString>

struct KisRecordedAction::Private {
    QString name;
    QString id;
};

KisRecordedAction::KisRecordedAction(QString name, QString id) : d(new Private)
{
    d->name = name;
    d->id = id;
}

KisRecordedAction::~KisRecordedAction()
{
    delete d;
}

QString KisRecordedAction::id()
{
    return d->id;
}

QString KisRecordedAction::name()
{
    return d->name;
}

void KisRecordedAction::toXML(QDomDocument& , QDomElement& elt)
{
    elt.setAttribute( "name", name() );
    elt.setAttribute( "id", id() );
}


/*

void KisRecordedAction::fromXML(QDomElement elt)
{
    d->name = elt.attribute("name","");
    Q_ASSERT(d->id == elt.attribute( "id", ""));
}
*/

struct KisRecordedActionFactory::Private {
    QString id;
};

KisRecordedActionFactory::KisRecordedActionFactory(QString id) : d(new Private)
{
    d->id = id;
}

KisRecordedActionFactory::~KisRecordedActionFactory()
{
    delete d;
}

QString KisRecordedActionFactory::id()
{
    return d->id;
}
