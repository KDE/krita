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

#include "kis_recorded_paint_actions.h"

#include <QDomDocument>
#include <QDomElement>

#include "kis_brush.h"
#include "kis_layer.h"
#include "kis_painter.h"
#include "kis_paint_information.h"
#include "kis_paintop_registry.h"

KisRecordedPaintAction::KisRecordedPaintAction(QString name, QString id) : KisRecordedAction(name, id)
{
    
}


struct KisRecordedPolyLinePaintAction::Private {
    QList<KisPaintInformation> infos;
    KisLayerSP layer;
    KisBrush* brush;
    QString paintOpId;
};

KisRecordedPolyLinePaintAction::KisRecordedPolyLinePaintAction(QString name, KisLayerSP layer, KisBrush* brush, QString paintOpId)
    : KisRecordedPaintAction(name, "PolyLinePaintAction"), d(new Private)
{
    d->layer = layer;
    d->brush = brush;
    d->paintOpId = paintOpId;
}

KisRecordedPolyLinePaintAction::~KisRecordedPolyLinePaintAction()
{
    delete d;
}

void KisRecordedPolyLinePaintAction::addPoint(const KisPaintInformation& info)
{
    d->infos.append(info);
}

void KisRecordedPolyLinePaintAction::play()
{
    if(d->infos.size() < 0) return;
    KisPainter painter( d->layer->paintDevice());
    painter.setBrush( d->brush );
    painter.setPaintOp( KisPaintOpRegistry::instance()->paintOp( d->paintOpId, (KisPaintOpSettings*)0, &painter, d->layer->image() ) );
    painter.paintAt(d->infos[0]);
    double savedDist = 0.0;
    for(int i = 0; i < d->infos.size() - 1; i++)
    {
        savedDist = painter.paintLine(d->infos[i],d->infos[i+1]);
    }
}

void KisRecordedPolyLinePaintAction::toXML(QDomDocument& doc, QDomElement elt)
{
    elt.setAttribute("layer", d->layer->id());
    elt.setAttribute("paintop", d->paintOpId);
    QDomElement ressourceElt = doc.createElement( "Brush");
    d->brush->toXML(doc, ressourceElt);
    elt.appendChild(ressourceElt);
    foreach(KisPaintInformation info, d->infos)
    {
        QDomElement infoElt = doc.createElement( "Waypoint");
        info.toXML(doc, infoElt);
        elt.appendChild(infoElt);
    }
    KisRecordedPaintAction::toXML(doc,elt);
}
