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

#include "recorder/kis_recorded_shape_paint_action.h"

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

struct KisRecordedShapePaintAction::Private {
    Shape shape;
    QRectF rectangle;
};

QString selectName(KisRecordedShapePaintAction::Shape s)
{
    switch(s)
    {
        case KisRecordedShapePaintAction::Ellipse:
            return i18n("Ellipse");
        case KisRecordedShapePaintAction::Rectangle:
            return i18n("Rectangle");
    }
}

KisRecordedShapePaintAction::KisRecordedShapePaintAction(
    const KisNodeQueryPath& path,
    const KisPaintOpPresetSP preset,
    Shape shape,
    const QRectF& rect)
        : KisRecordedPaintAction("ShapePaintAction", selectName(shape), path, preset)
        , d(new Private)
{
    d->shape = shape;
    d->rectangle = rect;
}

KisRecordedShapePaintAction::KisRecordedShapePaintAction(const KisRecordedShapePaintAction& rhs) : KisRecordedPaintAction(rhs), d(new Private(*rhs.d))
{

}

KisRecordedShapePaintAction::~KisRecordedShapePaintAction()
{
    delete d;
}

void KisRecordedShapePaintAction::playPaint(const KisPlayInfo&, KisPainter* painter) const
{
    switch(d->shape)
    {
        case Ellipse:
            painter->paintEllipse(d->rectangle);
            break;
        case Rectangle:
            painter->paintRect(d->rectangle);
            break;
    }
}

void KisRecordedShapePaintAction::toXML(QDomDocument& doc, QDomElement& elt, KisRecordedActionSaveContext* context) const
{
    KisRecordedPaintAction::toXML(doc, elt, context);
    QDomElement rectangleElt = doc.createElement("Rectangle");
    rectangleElt.setAttribute("x", d->rectangle.x());
    rectangleElt.setAttribute("y", d->rectangle.y());
    rectangleElt.setAttribute("width", d->rectangle.width());
    rectangleElt.setAttribute("height", d->rectangle.height());
    elt.appendChild(rectangleElt);
    switch(d->shape)
    {
        case Ellipse:
            elt.setAttribute("shape", "Ellipse");
            break;
        case Rectangle:
            elt.setAttribute("shape", "Rectangle");
            break;
    }
}

KisRecordedAction* KisRecordedShapePaintAction::clone() const
{
    return new KisRecordedShapePaintAction(*this);
}


KisRecordedShapePaintActionFactory::KisRecordedShapePaintActionFactory() :
        KisRecordedPaintActionFactory("EllipsePaintAction")
{
}

KisRecordedShapePaintActionFactory::~KisRecordedShapePaintActionFactory()
{

}

KisRecordedAction* KisRecordedShapePaintActionFactory::fromXML(const QDomElement& elt, const KisRecordedActionLoadContext* context)
{
    KisNodeQueryPath pathnode = nodeQueryPathFromXML(elt);

    // Decode pressets
    KisPaintOpPresetSP paintOpPreset = paintOpPresetFromXML(elt);

    QDomElement ellipseElt = elt.firstChildElement("Rectangle");
    qreal x, y, width, height;
    if (!ellipseElt.isNull()) {
        x = ellipseElt.attribute("x", "0.0").toDouble();
        y = ellipseElt.attribute("y", "0.0").toDouble();
        width = ellipseElt.attribute("width", "0.0").toDouble();
        height = ellipseElt.attribute("height", "0.0").toDouble();
    } else {
        x = 0;
        y = 0;
        width = 0;
        height = 0;
        dbgImage << "Warning: no <Rectangle /> found";
    }

    KisRecordedShapePaintAction::Shape shape = KisRecordedShapePaintAction::Ellipse;
    QString shapeStr = elt.attribute("shape", "Ellipse");
    if (shapeStr == "Ellipse")
    {
        shape = KisRecordedShapePaintAction::Ellipse;
    } else { // shapeStr == "Rectangle"
        shape = KisRecordedShapePaintAction::Rectangle;
    }

    KisRecordedShapePaintAction* rplpa = new KisRecordedShapePaintAction(pathnode, paintOpPreset, shape, QRectF(x, y, width, height));

    setupPaintAction(rplpa, elt, context);
    return rplpa;
}
