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

#include "recorder/kis_recorded_polyline_paint_action.h"
#include <QDomDocument>
#include <QDomElement>

#include <KoColor.h>
#include <KoColorModelStandardIds.h>
#include <KoCompositeOp.h>
#include <KoColorSpace.h>
#include "kis_node.h"
#include "kis_mask_generator.h"
#include "kis_painter.h"
#include "kis_paint_information.h"
#include "kis_paintop_registry.h"
#include "recorder/kis_recorded_action_factory_registry.h"
#include "kis_resource_server_provider.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_paintop_settings.h"
#include "kis_paintop_preset.h"
#include "kis_paint_device.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_node_query_path.h"

struct KisRecordedPolyLinePaintAction::Private {
    QList<KisPaintInformation> paintInformationObjects;
};

KisRecordedPolyLinePaintAction::KisRecordedPolyLinePaintAction(
    const KisNodeQueryPath& path,
    const KisPaintOpPresetSP paintOpPreset)
        : KisRecordedPaintAction("PolyLinePaintAction", i18n("Poly line"), path, paintOpPreset)
        , d(new Private)
{
}

KisRecordedPolyLinePaintAction::KisRecordedPolyLinePaintAction(const KisRecordedPolyLinePaintAction& rhs)
        : KisRecordedPaintAction(rhs), d(new Private(*rhs.d))
{

}

KisRecordedPolyLinePaintAction::~KisRecordedPolyLinePaintAction()
{
    delete d;
}

void KisRecordedPolyLinePaintAction::addPoint(const KisPaintInformation& info)
{
    d->paintInformationObjects.append(info);
}

void KisRecordedPolyLinePaintAction::playPaint(const KisPlayInfo&, KisPainter* painter) const
{
    dbgUI << "play poly line paint with " << d->paintInformationObjects.size() << " points";
    if (d->paintInformationObjects.size() <= 0) return;
    painter->paintAt(d->paintInformationObjects[0]);
    double savedDist = 0.0;
    for (int i = 0; i < d->paintInformationObjects.size() - 1; i++) {
        dbgUI << d->paintInformationObjects[i].pos() << " to " << d->paintInformationObjects[i+1].pos();
        savedDist = painter->paintLine(d->paintInformationObjects[i], d->paintInformationObjects[i+1], savedDist);
    }
}

void KisRecordedPolyLinePaintAction::toXML(QDomDocument& doc, QDomElement& elt, KisRecordedActionSaveContext* context) const
{
    KisRecordedPaintAction::toXML(doc, elt, context);
    QDomElement waypointsElt = doc.createElement("Waypoints");
    foreach(KisPaintInformation info, d->paintInformationObjects) {
        QDomElement infoElt = doc.createElement("Waypoint");
        info.toXML(doc, infoElt);
        waypointsElt.appendChild(infoElt);
    }
    elt.appendChild(waypointsElt);
}

KisRecordedAction* KisRecordedPolyLinePaintAction::clone() const
{
    return new KisRecordedPolyLinePaintAction(*this);
}



KisRecordedPolyLinePaintActionFactory::KisRecordedPolyLinePaintActionFactory() :
        KisRecordedPaintActionFactory("PolyLinePaintAction")
{
}

KisRecordedPolyLinePaintActionFactory::~KisRecordedPolyLinePaintActionFactory()
{

}

KisRecordedAction* KisRecordedPolyLinePaintActionFactory::fromXML(const QDomElement& elt, const KisRecordedActionLoadContext*)
{
    KisNodeQueryPath pathnode = nodeQueryPathFromXML(elt);

    // Decode pressets
    KisPaintOpPresetSP paintOpPreset = paintOpPresetFromXML(elt);

    KisRecordedPolyLinePaintAction* rplpa = new KisRecordedPolyLinePaintAction(pathnode, paintOpPreset);
    setupPaintAction(rplpa, elt);

    QDomElement wpElt = elt.firstChildElement("Waypoints");
    if (!wpElt.isNull()) {
        QDomNode nWp = wpElt.firstChild();
        while (!nWp.isNull()) {
            QDomElement eWp = nWp.toElement();
            if (!eWp.isNull() && eWp.tagName() == "Waypoint") {
                rplpa->addPoint(KisPaintInformation::fromXML(eWp));
            }
            nWp = nWp.nextSibling();
        }
    } else {
        dbgUI << "Warning: no <Waypoints /> found";
    }
    return rplpa;
}
