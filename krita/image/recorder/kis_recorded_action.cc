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

#include <kis_image.h>
#include <kis_node.h>
#include <kis_layer.h>
#include <kis_group_layer.h>

#include "kis_node_query_path.h"

struct KisRecordedAction::Private {
    Private(const KisNodeQueryPath& _path) : path(_path) {}
    QString name;
    QString id;
    KisNodeQueryPath path;
};

KisRecordedAction::KisRecordedAction(const QString& id, const QString& name, const KisNodeQueryPath& path) : d(new Private(path))
{
    d->name = name;
    d->id = id;
}

KisRecordedAction::KisRecordedAction(const KisRecordedAction& rhs) : d(new Private(*rhs.d))
{

}

KisRecordedAction::~KisRecordedAction()
{
    delete d;
}

const QString& KisRecordedAction::id() const
{
    return d->id;
}

const QString& KisRecordedAction::name() const
{
    return d->name;
}

void KisRecordedAction::setName(const QString& name)
{
    d->name = name;
}


const KisNodeQueryPath& KisRecordedAction::nodeQueryPath() const
{
    return d->path;
}

void KisRecordedAction::toXML(QDomDocument& , QDomElement& elt, KisRecordedActionSaveContext* ) const
{
    elt.setAttribute("name", name());
    elt.setAttribute("id", id());
    elt.setAttribute("path", d->path.toString());
}

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

QString KisRecordedActionFactory::id() const
{
    return d->id;
}

QString KisRecordedActionFactory::name() const
{
    return QString();
}
