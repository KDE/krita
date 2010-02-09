/*
 *  Copyright (c) 2010 Cyrille Berger <cberger@cberger.net>
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

#include "kis_recorded_fill_paint_action.h"

#include <QDomElement>

#include <klocale.h>

#include "kis_image.h"
#include <kis_fill_painter.h>
#include "kis_node.h"
#include "kis_node_query_path.h"
#include "kis_paintop_preset.h"
#include "kis_play_info.h"

struct KisRecordedFillPaintAction::Private {
    Private(const KisNodeQueryPath& _projectionPath) : projectionPath(_projectionPath) {}
    QPoint pt;
    KisNodeQueryPath projectionPath;
};

KisRecordedFillPaintAction::KisRecordedFillPaintAction(
    const KisNodeQueryPath& path,
    const QPoint& pt,
    const KisNodeQueryPath& projectionPath)
        : KisRecordedPaintAction("FillPaintAction", i18n("Fill"), path, 0)
        , d(new Private(projectionPath))
{
    d->pt = pt;
}

KisRecordedFillPaintAction::KisRecordedFillPaintAction(const KisRecordedFillPaintAction& rhs) : KisRecordedPaintAction(rhs), d(new Private(*rhs.d))
{

}

KisRecordedFillPaintAction::~KisRecordedFillPaintAction()
{
    delete d;
}

void KisRecordedFillPaintAction::toXML(QDomDocument& doc, QDomElement& elt, KisRecordedActionSaveContext* context) const
{
    KisRecordedPaintAction::toXML(doc, elt, context);
    elt.setAttribute("x", d->pt.x());
    elt.setAttribute("y", d->pt.y());
    elt.setAttribute("projectionPath", d->projectionPath.toString());
}

KisRecordedAction* KisRecordedFillPaintAction::clone() const
{
    return new KisRecordedFillPaintAction(*this);
}

KisPainter* KisRecordedFillPaintAction::createPainter(KisPaintDeviceSP device) const
{
    return new KisFillPainter(device);
}

void KisRecordedFillPaintAction::playPaint(const KisPlayInfo& info, KisPainter* _painter) const
{
    QList<KisNodeSP> nodes = d->projectionPath.queryNodes(info.image(), info.currentNode());
    KisPaintDeviceSP projection = 0;
    if (!nodes.isEmpty())
    {
        projection = nodes[0]->projection();
    }
    KisFillPainter* painter = static_cast<KisFillPainter*>(_painter);
    painter->setWidth(info.image()->width());
    painter->setHeight(info.image()->height());
    if (fillStyle() == KisPainter::FillStylePattern)
    {
        painter->fillPattern(d->pt.x(), d->pt.y(), projection);
    } else {
        painter->fillColor(d->pt.x(), d->pt.y(), projection);
    }
}

KisRecordedFillPaintActionFactory::KisRecordedFillPaintActionFactory() :
        KisRecordedPaintActionFactory("FillPaintAction")
{
}

KisRecordedFillPaintActionFactory::~KisRecordedFillPaintActionFactory()
{

}

KisRecordedAction* KisRecordedFillPaintActionFactory::fromXML(const QDomElement& elt, const KisRecordedActionLoadContext* context)
{
    KisNodeQueryPath pathnode = nodeQueryPathFromXML(elt);
    KisNodeQueryPath projectionPathnode = KisNodeQueryPath::fromString(elt.attribute("projectionPath"));

    int x = elt.attribute("x", "0").toInt();
    int y = elt.attribute("y", "0").toInt();

    KisRecordedFillPaintAction* rplpa = new KisRecordedFillPaintAction(pathnode, QPoint(x,y), projectionPathnode);

    setupPaintAction(rplpa, elt, context);
    return rplpa;
}
