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

#include "recorder/kis_recorded_ellipse_paint_action.h"

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

struct KisRecordedEllipsePaintAction::Private {
    QRectF ellipse;
};

KisRecordedEllipsePaintAction::KisRecordedEllipsePaintAction(const QString & name,
        const KisNodeQueryPath& path,
        const KisPaintOpPresetSP preset,
        KoColor foregroundColor,
        KoColor backgroundColor,
        int opacity,
        bool paintIncremental,
        const QString& compositeOp,
        const QRectF& rect)
        : KisRecordedPaintAction("BezierCurvePaintAction", name, path, preset,
                                 foregroundColor, backgroundColor, opacity, paintIncremental, compositeOp)
        , d(new Private)
{
    d->ellipse = rect;
}

KisRecordedEllipsePaintAction::KisRecordedEllipsePaintAction(const KisRecordedEllipsePaintAction& rhs) : KisRecordedPaintAction(rhs), d(new Private(*rhs.d))
{

}

KisRecordedEllipsePaintAction::~KisRecordedEllipsePaintAction()
{
    delete d;
}

void KisRecordedEllipsePaintAction::playPaint(const KisPlayInfo&, KisPainter* painter) const
{
    painter->paintEllipse(d->ellipse);
}

void KisRecordedEllipsePaintAction::toXML(QDomDocument& doc, QDomElement& elt) const
{
    KisRecordedPaintAction::toXML(doc, elt);
    QDomElement ellipseElt = doc.createElement("Ellipse");
    ellipseElt.setAttribute("x", d->ellipse.x());
    ellipseElt.setAttribute("y", d->ellipse.y());
    ellipseElt.setAttribute("width", d->ellipse.width());
    ellipseElt.setAttribute("height", d->ellipse.height());
    elt.appendChild(ellipseElt);
}

KisRecordedAction* KisRecordedEllipsePaintAction::clone() const
{
    return new KisRecordedEllipsePaintAction(*this);
}


KisRecordedEllipsePaintActionFactory::KisRecordedEllipsePaintActionFactory() :
        KisRecordedPaintActionFactory("BezierCurvePaintAction")
{
}

KisRecordedEllipsePaintActionFactory::~KisRecordedEllipsePaintActionFactory()
{

}

KisRecordedAction* KisRecordedEllipsePaintActionFactory::fromXML(const QDomElement& elt)
{
    Q_UNUSED(elt);
    QString name = elt.attribute("name");
    KisNodeQueryPath pathnode = nodeQueryPathFromXML(elt);

    int opacity = opacityFromXML(elt);
    bool paintIncremental = paintIncrementalFromXML(elt);

    QString compositeOp = compositeOpFromXML(elt);

    // Decode pressets
    KisPaintOpPresetSP paintOpPreset = paintOpPresetFromXML(elt);

    // Decode colors
    KoColor bC = backgroundColorFromXML(elt);
    KoColor fC = paintColorFromXML(elt);

    QDomElement ellipseElt = elt.firstChildElement("Ellipse");
    qreal x, y, width, height;
    if (!ellipseElt.isNull() ) {
        x = ellipseElt.attribute("x", "0.0").toDouble();
        y = ellipseElt.attribute("y", "0.0").toDouble();
        width = ellipseElt.attribute("width", "0.0").toDouble();
        height = ellipseElt.attribute("height", "0.0").toDouble();
    } else {
        x = 0;
        y = 0;
        width = 0;
        height = 0;
        dbgImage << "Warning: no <Ellipse /> found";
    }
    
    KisRecordedEllipsePaintAction* rplpa = new KisRecordedEllipsePaintAction(name, pathnode, paintOpPreset, fC, bC, opacity, paintIncremental, compositeOp, QRectF(x, y, width, height));

    return rplpa;
}
