/*
 *  Copyright (c) 2011 Cyrille Berger <cberger@cberger.net>
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

#include "kis_recorded_node_action.h"

#include <QDomDocument>
#include <KoUpdater.h>

#include "kis_node_query_path.h"
#include "kis_play_info.h"
#include "kis_image.h"
#include "kis_paint_device.h"
#include <KoProgressUpdater.h>

struct KisRecordedNodeAction::Private
{
    Private(const KisNodeQueryPath& _path) : path(_path) {}
    KisNodeQueryPath path;
};

KisRecordedNodeAction::KisRecordedNodeAction(const QString& id, const QString& name, const KisNodeQueryPath& path) : KisRecordedAction(id, name), d(new Private(path))
{
}

KisRecordedNodeAction::KisRecordedNodeAction(const KisRecordedNodeAction& _rhs) : KisRecordedAction(_rhs), d(new Private(*_rhs.d))
{
}

KisRecordedNodeAction::~KisRecordedNodeAction()
{
    delete d;
}

void KisRecordedNodeAction::play(const KisPlayInfo& _info, KoUpdater* _updater) const
{
    QList<KisNodeSP> nodes = nodeQueryPath().queryNodes(_info.image(), _info.currentNode());
    KoProgressUpdater updater(_updater);
    updater.start(nodes.size());
    foreach(KisNodeSP node, nodes)
    {
        play(node, _info, updater.startSubtask());
    }
}

const KisNodeQueryPath& KisRecordedNodeAction::nodeQueryPath() const
{
    return d->path;
}

void KisRecordedNodeAction::setNodeQueryPath(const KisNodeQueryPath& nqp)
{
    d->path = nqp;
}

void KisRecordedNodeAction::toXML(QDomDocument& doc, QDomElement& elt, KisRecordedActionSaveContext* ) const
{
    Q_UNUSED(doc)
    elt.setAttribute("path", d->path.toString());    
}
