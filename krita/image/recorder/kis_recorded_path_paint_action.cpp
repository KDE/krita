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

#include "recorder/kis_recorded_path_paint_action.h"
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

struct KisRecordedPathPaintAction::Private {
    struct BezierCurveSlice {
        enum Type {
            Point,
            Line,
            Curve
        };
        Type type;
        KisPaintInformation point1;
        QPointF control1;
        QPointF control2;
        KisPaintInformation point2;
    };
    QList<BezierCurveSlice> infos;
};

KisRecordedPathPaintAction::KisRecordedPathPaintAction(
    const KisNodeQueryPath& path,
    const KisPaintOpPresetSP preset)
        : KisRecordedPaintAction("PathPaintAction", i18n("Path"), path, preset)
        , d(new Private)
{
}

KisRecordedPathPaintAction::KisRecordedPathPaintAction(const KisRecordedPathPaintAction& rhs) : KisRecordedPaintAction(rhs), d(new Private(*rhs.d))
{

}

KisRecordedPathPaintAction::~KisRecordedPathPaintAction()
{
    delete d;
}

void KisRecordedPathPaintAction::addPoint(const KisPaintInformation& info)
{
    Private::BezierCurveSlice slice;
    slice.type = Private::BezierCurveSlice::Point;
    slice.point1 = info;
    d->infos.append(slice);
}

void KisRecordedPathPaintAction::addLine(const KisPaintInformation& point1, const KisPaintInformation& point2)
{
    Private::BezierCurveSlice slice;
    slice.type = Private::BezierCurveSlice::Line;
    slice.point1 = point1;
    slice.point2 = point2;
    d->infos.append(slice);
}

void KisRecordedPathPaintAction::addPolyLine(const QList<QPointF>& points)
{
    QPointF previousPoint = points[0];
    for(int i = 1; i < points.size(); ++i) {
        QPointF pt = points[i];
        addLine(KisPaintInformation(previousPoint), KisPaintInformation(pt));
        previousPoint = pt;
    }
}


void KisRecordedPathPaintAction::addCurve(const KisPaintInformation& point1,
        const QPointF& control1,
        const QPointF& control2,
        const KisPaintInformation& point2)
{
    Private::BezierCurveSlice slice;
    slice.type = Private::BezierCurveSlice::Curve;
    slice.point1 = point1;
    slice.control1 = control1;
    slice.control2 = control2;
    slice.point2 = point2;
    d->infos.append(slice);
}

void KisRecordedPathPaintAction::playPaint(const KisPlayInfo&, KisPainter* painter) const
{
    dbgImage << "play path paint action with " << d->infos.size() << " slices";
    if (d->infos.size() <= 0) return;
    double savedDist = 0.0;
    
    foreach (Private::BezierCurveSlice slice, d->infos)
    {
        switch(slice.type)
        {
            case Private::BezierCurveSlice::Point:
                painter->paintAt(slice.point1);
                savedDist = 0.0;
                break;
            case Private::BezierCurveSlice::Line:
                savedDist = painter->paintLine(slice.point1, slice.point2, savedDist);
                break;
            case Private::BezierCurveSlice::Curve:
                savedDist = painter->paintBezierCurve(slice.point1, slice.control1, slice.control2, slice.point2, savedDist);
                break;
        }
    }
    
}

void KisRecordedPathPaintAction::toXML(QDomDocument& doc, QDomElement& elt, KisRecordedActionSaveContext* context) const
{
    KisRecordedPaintAction::toXML(doc, elt, context);
    QDomElement waypointsElt = doc.createElement("Slices");
    foreach(const Private::BezierCurveSlice & slice, d->infos) {
        switch(slice.type)
        {
            case Private::BezierCurveSlice::Point:
            {
                QDomElement infoElt = doc.createElement("Point");
                slice.point1.toXML(doc, infoElt);

                waypointsElt.appendChild(infoElt);
                break;
            }
            case Private::BezierCurveSlice::Line:
            {
                QDomElement infoElt = doc.createElement("Line");
                // Point1
                QDomElement point1Elt = doc.createElement("Point1");
                slice.point1.toXML(doc, point1Elt);
                infoElt.appendChild(point1Elt);
                // Point2
                QDomElement point2Elt = doc.createElement("Point2");
                slice.point2.toXML(doc, point2Elt);
                infoElt.appendChild(point2Elt);

                waypointsElt.appendChild(infoElt);
                break;
            }
            case Private::BezierCurveSlice::Curve:
            {
                QDomElement infoElt = doc.createElement("Curve");
                // Point1
                QDomElement point1Elt = doc.createElement("Point1");
                slice.point1.toXML(doc, point1Elt);
                infoElt.appendChild(point1Elt);
                // Control1
                QDomElement control1Elt = doc.createElement("Control1");
                control1Elt.setAttribute("x", slice.control1.x());
                control1Elt.setAttribute("y", slice.control1.y());
                infoElt.appendChild(control1Elt);
                // Control2
                QDomElement control2Elt = doc.createElement("Control2");
                control2Elt.setAttribute("x", slice.control2.x());
                control2Elt.setAttribute("y", slice.control2.y());
                infoElt.appendChild(control2Elt);
                // Point2
                QDomElement point2Elt = doc.createElement("Point2");
                slice.point2.toXML(doc, point2Elt);
                infoElt.appendChild(point2Elt);

                waypointsElt.appendChild(infoElt);
            }
        }
    }
    elt.appendChild(waypointsElt);
}

KisRecordedAction* KisRecordedPathPaintAction::clone() const
{
    return new KisRecordedPathPaintAction(*this);
}


KisRecordedPathPaintActionFactory::KisRecordedPathPaintActionFactory() :
        KisRecordedPaintActionFactory("PathPaintAction")
{
}

KisRecordedPathPaintActionFactory::~KisRecordedPathPaintActionFactory()
{

}

KisRecordedAction* KisRecordedPathPaintActionFactory::fromXML(const QDomElement& elt, const KisRecordedActionLoadContext* context)
{
    KisNodeQueryPath pathnode = nodeQueryPathFromXML(elt);

    // Decode pressets
    KisPaintOpPresetSP paintOpPreset = paintOpPresetFromXML(elt);

    KisRecordedPathPaintAction* rplpa = new KisRecordedPathPaintAction(pathnode, paintOpPreset);

    setupPaintAction(rplpa, elt, context);

    QDomElement wpElt = elt.firstChildElement("Slices");
    if (!wpElt.isNull()) {
        QDomNode nWp = wpElt.firstChild();
        while (!nWp.isNull()) {
            QDomElement eWp = nWp.toElement();
            if (!eWp.isNull()) {
                if( eWp.tagName() == "Point") {
                    rplpa->addPoint(KisPaintInformation::fromXML(eWp));
                } else if(eWp.tagName() == "Point") {
                    rplpa->addLine(KisPaintInformation::fromXML(eWp.firstChildElement("Point1")),
                                    KisPaintInformation::fromXML(eWp.firstChildElement("Point2")));
                } else if( eWp.tagName() == "Curve") {
                    QDomElement control1Elt = eWp.firstChildElement("Control1");
                    QDomElement control2Elt = eWp.firstChildElement("Control2");
                    rplpa->addCurve(KisPaintInformation::fromXML(eWp.firstChildElement("Point1")),
                                    QPointF(control1Elt.attribute("x", "0.0").toDouble(),
                                            control1Elt.attribute("y", "0.0").toDouble()),
                                    QPointF(control2Elt.attribute("x", "0.0").toDouble(),
                                            control2Elt.attribute("y", "0.0").toDouble()),
                                    KisPaintInformation::fromXML(eWp.firstChildElement("Point2")));
                } else {
                    dbgImage << "Unsupported <" << eWp.tagName() << " /> element";
                }
            }
            nWp = nWp.nextSibling();
        }
    } else {
        dbgImage << "Warning: no <Waypoints /> found";
    }
    return rplpa;
}


