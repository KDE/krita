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

#include <KoColor.h>
#include <KoColorModelStandardIds.h>
#include <KoCompositeOp.h>
#include <KoColorSpace.h>
#include "kis_auto_brush.h"
#include "kis_brush.h"
#include "kis_layer.h"
#include "kis_mask_generator.h"
#include "kis_painter.h"
#include "kis_paint_information.h"
#include "kis_paintop_registry.h"
#include "kis_recorded_action_factory_registry.h"
#include "kis_resourceserverprovider.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_paintop_settings.h"
#include "kis_paint_device.h"
#include "kis_image.h"

class KisRecordedPaintActionFactory : public KisRecordedActionFactory {
    public:
        KisRecordedPaintActionFactory(const QString & id) : KisRecordedActionFactory(id) {}
        virtual ~KisRecordedPaintActionFactory(){}
    protected:
        KisBrush* brushFromXML(const QDomElement& elt);
        KisPaintOpSettings* settingsFromXML(const QString& paintOpId, const QDomElement& elt, KisImageSP image);
};

class KisRecordedPolyLinePaintActionFactory : public KisRecordedPaintActionFactory {
    public:
        KisRecordedPolyLinePaintActionFactory();
        virtual ~KisRecordedPolyLinePaintActionFactory();
        virtual KisRecordedAction* fromXML(KisImageSP img, const QDomElement& elt);
};

class KisRecordedBezierCurvePaintActionFactory : public KisRecordedPaintActionFactory {
    public:
        KisRecordedBezierCurvePaintActionFactory();
        virtual ~KisRecordedBezierCurvePaintActionFactory();
        virtual KisRecordedAction* fromXML(KisImageSP img, const QDomElement& elt);
};


class KisRecordedPaintActionsFactory {
    public:
        KisRecordedPaintActionsFactory()
        {
            KisRecordedActionFactoryRegistry::instance()->add(new KisRecordedPolyLinePaintActionFactory);
            KisRecordedActionFactoryRegistry::instance()->add(new KisRecordedBezierCurvePaintActionFactory);
        }

};

KisRecordedPaintActionsFactory factory;

//--- KisRecordedPolyLinePaintAction ---//

struct KisRecordedPaintAction::Private {
    KisLayerSP layer;
    KisBrush* brush;
    QString paintOpId;
    KisPaintOpSettings *settings;
    KoColor foregroundColor;
    KoColor backgroundColor;
    int opacity;
    bool paintIncremental;
    const KoCompositeOp * compositeOp;
};

KisRecordedPaintAction::KisRecordedPaintAction(const QString & name, const QString & id, KisLayerSP layer, KisBrush* brush, const QString & paintOpId, const KisPaintOpSettings *settings, KoColor foregroundColor, KoColor backgroundColor, int opacity, bool paintIncremental, const KoCompositeOp * compositeOp) : KisRecordedAction(name, id), d(new Private)
{
    d->layer = layer;
    d->brush = brush;
    d->paintOpId = paintOpId;
    d->settings = settings ? d->settings = settings->clone() : 0;
    d->foregroundColor = foregroundColor;
    d->backgroundColor = backgroundColor;
    d->opacity = opacity;
    d->paintIncremental = paintIncremental;
    d->compositeOp = compositeOp;
}

KisRecordedPaintAction::KisRecordedPaintAction(const KisRecordedPaintAction& rhs) : KisRecordedAction(rhs), d(new Private(*rhs.d))
{
    
}

KisRecordedPaintAction::~KisRecordedPaintAction()
{
    delete d->settings;
    delete d;
}

void KisRecordedPaintAction::toXML(QDomDocument& doc, QDomElement& elt) const
{
    KisRecordedAction::toXML(doc, elt);
    elt.setAttribute("layer", KisRecordedAction::layerToIndexPath(d->layer));
    elt.setAttribute("paintop", d->paintOpId);
    // Paintop settings
    if(d->settings)
    {
        QDomElement settingsElt = doc.createElement( "PaintOpSettings" );
        d->settings->toXML( doc, settingsElt);
        elt.appendChild( settingsElt );
    }
    // Brush
    QDomElement ressourceElt = doc.createElement( "Brush");
    d->brush->toXML( doc, ressourceElt);
    elt.appendChild( ressourceElt);
    // ForegroundColor
    QDomElement foregroundColorElt = doc.createElement( "ForegroundColor" );
    d->foregroundColor.toXML( doc, foregroundColorElt );
    elt.appendChild( foregroundColorElt );
    // BackgroundColor
    QDomElement backgroundColorElt = doc.createElement( "BackgroundColor" );
    d->backgroundColor.toXML(doc, backgroundColorElt );
    elt.appendChild( backgroundColorElt );
    // Opacity
    elt.setAttribute("opacity", d->opacity);
    // paintIncremental
    elt.setAttribute("paintIncremental", d->paintIncremental);
    // compositeOp
    elt.setAttribute("compositeOp", d->compositeOp->id());
}

KisLayerSP KisRecordedPaintAction::layer() const
{
    return d->layer;
}
KisBrush* KisRecordedPaintAction::brush() const
{
    return d->brush;
}
QString KisRecordedPaintAction::paintOpId() const
{
    return d->paintOpId;
}

void KisRecordedPaintAction::play(KisUndoAdapter* adapter) const
{
    dbgUI << "Play recorded paint action on layer : " << layer()->name() ;
    KisTransaction * cmd = 0;
    if (adapter) cmd = new KisTransaction("", layer()->paintDevice());
    
    KisPaintDeviceSP target = 0;
    if(d->paintIncremental)
    {
        target = layer()->paintDevice();
    } else {
        target = new KisPaintDevice( layer()->paintDevice()->colorSpace());
    }
    
    KisPainter painter( target );
    Q_ASSERT(brush());
    painter.setBrush( brush() );
    painter.setPaintOp( KisPaintOpRegistry::instance()->paintOp( paintOpId(), (KisPaintOpSettings*)0, &painter, layer()->image() ) );
    
    if (d->paintIncremental) {
        painter.setCompositeOp(d->compositeOp);
        painter.setOpacity( d->opacity);
    } else {
        painter.setCompositeOp(layer()->paintDevice()->colorSpace()->compositeOp(COMPOSITE_ALPHA_DARKEN));
        painter.setOpacity( OPACITY_OPAQUE );

    }
    
    painter.setPaintColor( d->foregroundColor );
    painter.setFillColor( d->backgroundColor );
    
    playPaint(&painter);
    
    if(!d->paintIncremental)
    {
        KisPainter painter2( layer()->paintDevice());
        painter2.setCompositeOp(d->compositeOp);
        QRegion r = painter.dirtyRegion();
        QVector<QRect> dirtyRects = r.rects();
        QVector<QRect>::iterator it = dirtyRects.begin();
        QVector<QRect>::iterator end = dirtyRects.end();
        while (it != end) {

            painter2.bitBlt(it->x(), it->y(), d->compositeOp, target,
                            d->opacity,
                            it->x(), it->y(),
                            it->width(), it->height());
            ++it;
        }
        
        layer()->setDirty(painter2.dirtyRegion());
    } else {
        layer()->setDirty( painter.dirtyRegion() );
    }
    if (adapter) adapter->addCommand( cmd );
}


KisBrush* KisRecordedPaintActionFactory::brushFromXML(const QDomElement& elt)
{
    // TODO: support for autobrush
    
    QString name = elt.attribute("name","");
    QString type = elt.attribute("type","");
    if( type == "autobrush")
    {
        double width = elt.attribute("autobrush_width","1.0").toDouble();
        double height = elt.attribute("autobrush_height","1.0").toDouble();
        double hfade = elt.attribute("autobrush_hfade","1.0").toDouble();
        double vfade = elt.attribute("autobrush_vfade","1.0").toDouble();
        QString typeShape = elt.attribute("autobrush_type", "circle");
        if(typeShape == "circle")
        {
            return new KisAutoBrush(new KisCircleMaskGenerator(width, height, hfade, vfade) );
        } else {
            return new KisAutoBrush(new KisRectangleMaskGenerator(width, height, hfade, vfade) );
        }
    } else {
        dbgUI << "Looking for brush " << name;
        QList<KisBrush*> resources = KisResourceServerProvider::instance()->brushServer()->resources();
        foreach(KisBrush* r, resources)
        {
            if(r->name() == name)
            {
                return r;
            }
        }
    }
    dbgUI << "Brush " << name << " of type " << type << " not found.";
    return new KisAutoBrush(new KisCircleMaskGenerator(1.0, 1.0, 0.0, 0.0) );
}

KisPaintOpSettings* KisRecordedPaintActionFactory::settingsFromXML(const QString& paintOpId, const QDomElement& elt, KisImageSP image)
{
    KisPaintOpSettings* settings = KisPaintOpRegistry::instance()->get( paintOpId)->settings(image);
    if(settings)
    {
        settings->fromXML( elt );
    }
    return settings;
}

//--- KisRecordedPolyLinePaintAction ---//

struct KisRecordedPolyLinePaintAction::Private {
    QList<KisPaintInformation> infos;
};

KisRecordedPolyLinePaintAction::KisRecordedPolyLinePaintAction(const QString & name, KisLayerSP layer, KisBrush* brush, const QString & paintOpId, const KisPaintOpSettings *settings, KoColor foregroundColor, KoColor backgroundColor, int opacity, bool paintIncremental, const KoCompositeOp * compositeOp)
    : KisRecordedPaintAction(name, "PolyLinePaintAction", layer, brush, paintOpId, settings, foregroundColor, backgroundColor, opacity, paintIncremental, compositeOp), d(new Private)
{
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

void KisRecordedPolyLinePaintAction::playPaint(KisPainter* painter) const
{
    dbgUI << "play poly line paint with " << d->infos.size() << " points";
    if(d->infos.size() < 0) return;
    painter->paintAt(d->infos[0]);
    double savedDist = 0.0;
    for(int i = 0; i < d->infos.size() - 1; i++)
    {
        dbgUI << d->infos[i].pos() << " to " << d->infos[i+1].pos();
        savedDist = painter->paintLine(d->infos[i],d->infos[i+1], savedDist);
    }
}

void KisRecordedPolyLinePaintAction::toXML(QDomDocument& doc, QDomElement& elt) const
{
    KisRecordedPaintAction::toXML(doc,elt);
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
        KisRecordedPaintActionFactory("PolyLinePaintAction")
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
    int opacity = elt.attribute("opacity", "100").toInt();
    bool paintIncremental = elt.attribute("paintIncremental", "1").toInt();
    
    const KoCompositeOp * compositeOp = layer->colorSpace()->compositeOp( elt.attribute("compositeOp"));
    if(!compositeOp) {
        compositeOp = layer->colorSpace()->compositeOp( COMPOSITE_OVER );
    }
    
    KisPaintOpSettings* settings = 0;
    QDomElement settingsElt = elt.firstChildElement("PaintOpSettings");
    if(!settingsElt.isNull())
    {
        settings = settingsFromXML(paintOpId, settingsElt, img);
    }
    else {
        dbgUI << "No <PaintOpSettings /> found";
    }
    
    KisBrush* brush = 0;
    
    QDomElement brushElt = elt.firstChildElement("Brush");
    if(!brushElt.isNull())
    {
        brush = brushFromXML(brushElt);
    }
    else {
        dbgUI << "Warning: no <Brush /> found";
    }
    
    QDomElement backgroundColorElt = elt.firstChildElement("BackgroundColor");
    KoColor bC;
    if(!backgroundColorElt.isNull())
    {
        bC = KoColor::fromXML( backgroundColorElt.firstChildElement(""), Integer8BitsColorDepthID.id(), QHash<QString,QString>() );
        bC.setOpacity( 255 );
        dbgUI << "Background color : " << bC.toQColor();
    }
    else {
        dbgUI << "Warning: no <BackgroundColor /> found";
    }
    QDomElement foregroundColorElt = elt.firstChildElement("ForegroundColor");
    KoColor fC;
    if(!foregroundColorElt.isNull())
    {
        fC = KoColor::fromXML( foregroundColorElt.firstChildElement(""), Integer8BitsColorDepthID.id(), QHash<QString,QString>() );
        fC.setOpacity( 255 );
        dbgUI << "Foreground color : " << fC.toQColor();
    } else {
        dbgUI << "Warning: no <ForegroundColor /> found";
    }
    
    KisRecordedPolyLinePaintAction* rplpa = new KisRecordedPolyLinePaintAction(name, layer, brush, paintOpId, settings, fC, bC, opacity, paintIncremental, compositeOp);
    
    QDomElement wpElt = elt.firstChildElement("Waypoints");
    if(!wpElt.isNull())
    {
        QDomNode nWp = wpElt.firstChild();
        while(!nWp.isNull())
        {
            QDomElement eWp = nWp.toElement();
            if(!eWp.isNull() && eWp.tagName() == "Waypoint")
            {
                rplpa->addPoint( KisPaintInformation::fromXML(eWp) );
            }
            nWp = nWp.nextSibling();
        }
    }
    else {
        dbgUI << "Warning: no <Waypoints /> found";
    }
    return rplpa;
}

//--- KisRecordedBezierCurvePaintAction ---//

struct KisRecordedBezierCurvePaintAction::Private {
    struct BezierCurveSlice
    {
        KisPaintInformation point1;
        QPointF control1;
        QPointF control2;
        KisPaintInformation point2;
    };
    QList<BezierCurveSlice> infos;
};

KisRecordedBezierCurvePaintAction::KisRecordedBezierCurvePaintAction(const QString & name,
    KisLayerSP layer,
    KisBrush* brush,
    const QString & paintOpId,
    const KisPaintOpSettings *settings,
    KoColor foregroundColor,
    KoColor backgroundColor,
    int opacity,
    bool paintIncremental,
    const KoCompositeOp * compositeOp)
    : KisRecordedPaintAction(name, "BezierCurvePaintAction", layer, brush, paintOpId, settings, foregroundColor, backgroundColor, opacity, paintIncremental, compositeOp), d(new Private)
{
}

KisRecordedBezierCurvePaintAction::KisRecordedBezierCurvePaintAction(const KisRecordedBezierCurvePaintAction& rhs) : KisRecordedPaintAction(rhs), d(new Private(*rhs.d))
{
    
}

KisRecordedBezierCurvePaintAction::~KisRecordedBezierCurvePaintAction()
{
    delete d;
}

void KisRecordedBezierCurvePaintAction::addPoint(const KisPaintInformation& point1, const QPointF& control1, const QPointF& control2, const KisPaintInformation& point2)
{
    Private::BezierCurveSlice slice;
    slice.point1 = point1;
    slice.control1 = control1;
    slice.control2 = control2;
    slice.point2 = point2;
    d->infos.append(slice);
}

void KisRecordedBezierCurvePaintAction::playPaint(KisPainter* painter) const
{
    dbgUI << "play bezier curve paint with " << d->infos.size() << " points";
    if(d->infos.size() < 0) return;
    double savedDist = 0.0;
    for(int i = 0; i < d->infos.size(); i++)
    {
        dbgUI << d->infos[i].point1.pos() << " to " << d->infos[i].point2.pos();
        savedDist = painter->paintBezierCurve( d->infos[i].point1, d->infos[i].control1, d->infos[i].control2, d->infos[i].point2, savedDist);
    }
}

void KisRecordedBezierCurvePaintAction::toXML(QDomDocument& doc, QDomElement& elt) const
{
    KisRecordedPaintAction::toXML(doc,elt);
    QDomElement waypointsElt = doc.createElement( "Waypoints");
    foreach(Private::BezierCurveSlice info, d->infos)
    {
        QDomElement infoElt = doc.createElement( "Waypoint");
        // Point1
        QDomElement point1Elt = doc.createElement( "Point1");
        info.point1.toXML(doc, point1Elt);
        infoElt.appendChild(point1Elt);
        // Control1
        QDomElement control1Elt = doc.createElement( "Control1");
        control1Elt.setAttribute("x", info.control1.x());
        control1Elt.setAttribute("y", info.control1.y());
        infoElt.appendChild(control1Elt);
        // Control2
        QDomElement control2Elt = doc.createElement( "Control2");
        control2Elt.setAttribute("x", info.control2.x());
        control2Elt.setAttribute("y", info.control2.y());
        infoElt.appendChild(control2Elt);
        // Point2
        QDomElement point2Elt = doc.createElement( "Point2");
        info.point2.toXML(doc, point2Elt);
        infoElt.appendChild(point2Elt);
        
        waypointsElt.appendChild(infoElt);
    }
    elt.appendChild(waypointsElt);
}

KisRecordedAction* KisRecordedBezierCurvePaintAction::clone() const
{
    return new KisRecordedBezierCurvePaintAction(*this);
}

KisRecordedBezierCurvePaintActionFactory::KisRecordedBezierCurvePaintActionFactory() :
        KisRecordedPaintActionFactory("BezierCurvePaintAction")
{
}
KisRecordedBezierCurvePaintActionFactory::~KisRecordedBezierCurvePaintActionFactory()
{
    
}

KisRecordedAction* KisRecordedBezierCurvePaintActionFactory::fromXML(KisImageSP img, const QDomElement& elt)
{
    QString name = elt.attribute("name");
    KisLayerSP layer = KisRecordedActionFactory::indexPathToLayer(img, elt.attribute("layer"));
    QString paintOpId = elt.attribute("paintop");
    int opacity = elt.attribute("opacity", "100").toInt();
    bool paintIncremental = elt.attribute("paintIncremental", "1").toInt();
    
    const KoCompositeOp * compositeOp = layer->colorSpace()->compositeOp( elt.attribute("compositeOp"));
    if (!compositeOp) {
        compositeOp = layer->colorSpace()->compositeOp( COMPOSITE_OVER );
    }
    
    KisPaintOpSettings* settings = 0;
    QDomElement settingsElt = elt.firstChildElement("PaintOpSettings");
    if(!settingsElt.isNull())
    {
        settings = settingsFromXML(paintOpId, settingsElt, img);
    }
    else {
        dbgUI << "No <PaintOpSettings /> found";
    }

    KisBrush* brush = 0;
    
    QDomElement brushElt = elt.firstChildElement("Brush");
    if(!brushElt.isNull())
    {
        brush = brushFromXML(brushElt);
    }
    else {
        dbgUI << "Warning: no <Brush /> found";
    }
    Q_ASSERT(brush);
    QDomElement backgroundColorElt = elt.firstChildElement("BackgroundColor");
    KoColor bC;

    if(!backgroundColorElt.isNull())
    {
        bC = KoColor::fromXML( backgroundColorElt.firstChildElement(), Integer8BitsColorDepthID.id(), QHash<QString,QString>() );
        bC.setOpacity( 255 );
        dbgUI << "Background color : " << bC.toQColor();
    }
    else {
        dbgUI << "Warning: no <BackgroundColor /> found";
    }
    QDomElement foregroundColorElt = elt.firstChildElement("ForegroundColor");
    KoColor fC;
    if(!foregroundColorElt.isNull())
    {
        fC = KoColor::fromXML( foregroundColorElt.firstChildElement(), Integer8BitsColorDepthID.id(), QHash<QString,QString>() );
        dbgUI << "Foreground color : " << fC.toQColor();
        fC.setOpacity( 255 );
    } else {
        dbgUI << "Warning: no <ForegroundColor /> found";
    }
    
    KisRecordedBezierCurvePaintAction* rplpa = new KisRecordedBezierCurvePaintAction(name, layer, brush, paintOpId, settings, fC, bC, opacity, paintIncremental, compositeOp);
    
    QDomElement wpElt = elt.firstChildElement("Waypoints");
    if(!wpElt.isNull())
    {
        QDomNode nWp = wpElt.firstChild();
        while(!nWp.isNull())
        {
            QDomElement eWp = nWp.toElement();
            if(!eWp.isNull() && eWp.tagName() == "Waypoint")
            {
                QDomElement control1Elt = eWp.firstChildElement("Control1");
                QDomElement control2Elt = eWp.firstChildElement("Control2");
                rplpa->addPoint( KisPaintInformation::fromXML(eWp.firstChildElement("Point1") ), 
                                 QPointF(control1Elt.attribute("x","0.0").toDouble(), 
                                         control1Elt.attribute("y","0.0").toDouble()),
                                 QPointF(control2Elt.attribute("x","0.0").toDouble(), 
                                         control2Elt.attribute("y","0.0").toDouble()),
                                 KisPaintInformation::fromXML(eWp.firstChildElement("Point2") ) );
            }
            nWp = nWp.nextSibling();
        }
    } else {
        dbgUI << "Warning: no <Waypoints /> found";
    }
    return rplpa;
}


