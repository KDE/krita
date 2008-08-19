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

#include "recorder/kis_recorded_paint_action.h"
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


struct KisRecordedPaintAction::Private {
    KisNodeSP node;
    KisPaintOpPresetSP paintOpPreset;
    KoColor foregroundColor;
    KoColor backgroundColor;
    int opacity;
    bool paintIncremental;
    const KoCompositeOp * compositeOp;
};

KisRecordedPaintAction::KisRecordedPaintAction(const QString & name,
                                               const QString & id,
                                               KisNodeSP node,
                                               const KisPaintOpPresetSP paintOpPreset,
                                               KoColor foregroundColor,
                                               KoColor backgroundColor,
                                               int opacity,
                                               bool paintIncremental,
                                               const KoCompositeOp * compositeOp)
    : KisRecordedAction(name, id)
    , d(new Private)
{
    d->node = node;
    d->paintOpPreset = paintOpPreset ? d->paintOpPreset = paintOpPreset->clone() : 0;
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
    delete d;
}

void KisRecordedPaintAction::toXML(QDomDocument& doc, QDomElement& elt) const
{
    KisRecordedAction::toXML(doc, elt);
    elt.setAttribute("node", KisRecordedAction::nodeToIndexPath(d->node));
#if 0 // XXX
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
#endif

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

KisNodeSP KisRecordedPaintAction::node() const
{
    return d->node;
}

void KisRecordedPaintAction::play(KisUndoAdapter* adapter) const
{
    dbgUI << "Play recorded paint action on node : " << node()->name() ;
    KisTransaction * cmd = 0;
    if (adapter) cmd = new KisTransaction("", node()->paintDevice());

    KisPaintDeviceSP target = 0;
    if(d->paintIncremental)
    {
        target = node()->paintDevice();
    } else {
        target = new KisPaintDevice( node()->paintDevice()->colorSpace());
    }

    KisPainter painter( target );

    KisImageSP image;
    KisNodeSP parent = node();
    while (image == 0 && parent->parent() ) {
        // XXX: ugly!
        KisLayerSP layer = dynamic_cast<KisLayer*>(parent.data());
        if (layer) {
            image = layer->image();
        }
        parent = parent->parent();
    }

    painter.setPaintOpPreset(d->paintOpPreset, image );

    if (d->paintIncremental) {
        painter.setCompositeOp(d->compositeOp);
        painter.setOpacity( d->opacity);
    } else {
        painter.setCompositeOp(node()->paintDevice()->colorSpace()->compositeOp(COMPOSITE_ALPHA_DARKEN));
        painter.setOpacity( OPACITY_OPAQUE );

    }

    painter.setPaintColor( d->foregroundColor );
    painter.setFillColor( d->backgroundColor );

    playPaint(&painter);

    if(!d->paintIncremental)
    {
        KisPainter painter2( node()->paintDevice());
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

        node()->setDirty(painter2.dirtyRegion());
    } else {
        node()->setDirty( painter.dirtyRegion() );
    }
    if (adapter) adapter->addCommand( cmd );
}


KisPaintOpPresetSP KisRecordedPaintActionFactory::paintOpPresetFromXML(const QString& paintOpId, const QDomElement& elt, KisImageSP image)
{
#if 0
    KisPaintOpSettingsSP settings = KisPaintOpRegistry::instance()->get( paintOpId)->settings(image);
    if(settings)
    {
        settings->fromXML( elt );
    }
    return settings;
#endif
}
