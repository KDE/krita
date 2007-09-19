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
#include "kis_recorded_action_factory_registry.h"
#include "kis_resourceserver.h"

class KisRecordedPaintActionsFactory {
    public:
        KisRecordedPaintActionsFactory()
        {
            KisRecordedActionFactoryRegistry::instance()->add(new KisRecordedPolyLinePaintActionFactory);
        }

};

KisRecordedPaintActionsFactory factory;

KisRecordedPaintAction::KisRecordedPaintAction(QString name, QString id) : KisRecordedAction(name, id)
{
}

KisRecordedPaintAction::KisRecordedPaintAction(const KisRecordedPaintAction& rhs) : KisRecordedAction(rhs)
{
    
}

//--- KisRecordedPolyLinePaintAction ---//

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

KisRecordedPolyLinePaintAction::KisRecordedPolyLinePaintAction(const KisRecordedPolyLinePaintAction& rhs) : KisRecordedPaintAction(rhs), d(new Private(*rhs.d))
{
    
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
    d->layer->setDirty( painter.dirtyRegion() );
}

void KisRecordedPolyLinePaintAction::toXML(QDomDocument& doc, QDomElement& elt)
{
    KisRecordedPaintAction::toXML(doc,elt);
    elt.setAttribute("layer", KisRecordedAction::layerToIndexPath(d->layer));
    elt.setAttribute("paintop", d->paintOpId);
    QDomElement ressourceElt = doc.createElement( "Brush");
    d->brush->toXML(doc, ressourceElt);
    elt.appendChild(ressourceElt);
    QDomElement waypointsElt = doc.createElement( "Waypoints");
    foreach(KisPaintInformation info, d->infos)
    {
        QDomElement infoElt = doc.createElement( "Waypoint");
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
        KisRecordedActionFactory("PolyLinePaintAction")
{
}
KisRecordedPolyLinePaintActionFactory::~KisRecordedPolyLinePaintActionFactory()
{
    
}

KisRecordedAction* KisRecordedPolyLinePaintActionFactory::fromXML(KisImageSP img, const QDomElement& elt)
{
    QString name = elt.attribute("name");
    KisLayerSP layer = KisRecordedActionFactory::indexPathToLayer(img, elt.attribute("layer"));
    QString paintOpId = elt.attribute("paintop");
    KisBrush* brush = 0;
    
    QDomElement brushElt = elt.firstChildElement("Brush");
    if(not brushElt.isNull())
    {
        brush = brushFromXML(brushElt);
    } else {
        kDebug() << "Warning: no <Brush /> found";
    }
    
    KisRecordedPolyLinePaintAction* rplpa = new KisRecordedPolyLinePaintAction(name, layer, brush, paintOpId);
    
    QDomElement wpElt = elt.firstChildElement("Waypoints");
    if(not wpElt.isNull())
    {
        QDomNode nWp = wpElt.firstChild();
        while(not nWp.isNull())
        {
            QDomElement eWp = nWp.toElement();
            if(not eWp.isNull() and eWp.tagName() == "Waypoint")
            {
                rplpa->addPoint( KisPaintInformation::fromXML(eWp) );
            }
            nWp = nWp.nextSibling();
        }
    } else {
        kDebug() << "Warning: no <Waypoints /> found";
    }
    return rplpa;
}

KisBrush* KisRecordedPolyLinePaintActionFactory::brushFromXML(const QDomElement& elt)
{
    // TODO: support for autobrush
    QString name = elt.attribute("name","");
    kDebug() << "Looking for brush " << name;
    QList<KoResource*> resources = KisResourceServerRegistry::instance()->get("BrushServer")->resources();
    foreach(KoResource* r, resources)
    {
        if(r->name() == name)
        {
            return static_cast<KisBrush*>(r);
        }
    }
    kDebug() << "Brush " << name << " not found.";
    return 0;
}
