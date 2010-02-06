/*
 *  Copyright (c) 2007,2010 Cyrille Berger <cberger@cberger.net>
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
#include "kis_resource_server_provider.h"
#include "kis_transaction.h"
#include "kis_undo_adapter.h"
#include "kis_paintop_settings.h"
#include "kis_paintop_preset.h"
#include "kis_paint_device.h"
#include "kis_image.h"
#include "kis_layer.h"
#include "kis_play_info.h"
#include "kis_node_query_path.h"

// Recorder
#include "kis_recorded_action_factory_registry.h"
#include "kis_recorded_action_load_context.h"
#include "kis_recorded_action_save_context.h"
#include <KoAbstractGradient.h>
#include <kis_pattern.h>
#include <filter/kis_filter_configuration.h>
#include <generator/kis_generator_registry.h>
#include <generator/kis_generator.h>

struct KisRecordedPaintAction::Private {
    KisPaintOpPresetSP paintOpPreset;
    KoColor foregroundColor;
    KoColor backgroundColor;
    qreal opacity; ///< opacity in the range 0.0 -> 100.0
    bool paintIncremental;
    QString compositeOp;
    KisPainter::StrokeStyle strokeStyle;
    KisPainter::FillStyle fillStyle;
    const KisPattern* pattern;
    const KoAbstractGradient* gradient;
    const KisFilterConfiguration* generator;
};

KisRecordedPaintAction::KisRecordedPaintAction(const QString & id,
        const QString & name,
        const KisNodeQueryPath& path,
        const KisPaintOpPresetSP paintOpPreset)
        : KisRecordedAction(id, name, path)
        , d(new Private)
{
    Q_ASSERT(paintOpPreset);
    d->paintOpPreset = paintOpPreset->clone();
    d->opacity = 1.0;
    d->paintIncremental = true;
    d->compositeOp = COMPOSITE_OVER;
    d->strokeStyle = KisPainter::StrokeStyleBrush;
    d->fillStyle = KisPainter::FillStyleNone;
    d->pattern = 0;
    d->gradient = 0;
    d->generator = 0;
}

KisRecordedPaintAction::KisRecordedPaintAction(const KisRecordedPaintAction& rhs) : KisRecordedAction(rhs), d(new Private(*rhs.d))
{

}

KisRecordedPaintAction::~KisRecordedPaintAction()
{
    delete d;
}

void KisRecordedPaintAction::toXML(QDomDocument& doc, QDomElement& elt, KisRecordedActionSaveContext* context) const
{
    KisRecordedAction::toXML(doc, elt, context);

    // Paint op presset
    QDomElement paintopPressetElt = doc.createElement("PaintopPreset");
    d->paintOpPreset->toXML(doc, paintopPressetElt);
    elt.appendChild(paintopPressetElt);

    // ForegroundColor
    QDomElement foregroundColorElt = doc.createElement("ForegroundColor");
    d->foregroundColor.toXML(doc, foregroundColorElt);
    elt.appendChild(foregroundColorElt);

    // BackgroundColor
    QDomElement backgroundColorElt = doc.createElement("BackgroundColor");
    d->backgroundColor.toXML(doc, backgroundColorElt);
    elt.appendChild(backgroundColorElt);

    // Opacity
    elt.setAttribute("opacity", d->opacity);

    // paintIncremental
    elt.setAttribute("paintIncremental", d->paintIncremental);

    // compositeOp
    elt.setAttribute("compositeOp", d->compositeOp);
    
    // Save stroke style
    switch(d->strokeStyle)
    {
        case KisPainter::StrokeStyleNone:
            elt.setAttribute("strokeStyle", "None");
            break;
        case KisPainter::StrokeStyleBrush:
            elt.setAttribute("strokeStyle", "Brush");
            break;
    }
    // Save fill style
    switch(d->fillStyle)
    {
        case KisPainter::FillStyleNone:
            elt.setAttribute("fillStyle", "None");
            break;
        case KisPainter::FillStyleForegroundColor:
            elt.setAttribute("fillStyle", "PaintColor");
            break;
        case KisPainter::FillStyleBackgroundColor:
            elt.setAttribute("fillStyle", "AlternativeColor");
            break;
        case KisPainter::FillStylePattern:
            elt.setAttribute("fillStyle", "Pattern");
            context->savePattern(d->pattern);
            elt.setAttribute("pattern", d->pattern->name());
            break;
        case KisPainter::FillStyleGradient:
            elt.setAttribute("fillStyle", "Gradient");
            context->saveGradient(d->gradient);
            elt.setAttribute("gradient", d->gradient->name());
            break;
        case KisPainter::FillStyleStrokes:
            elt.setAttribute("fillStyle", "Strokes");
            break;
        case KisPainter::FillStyleGenerator:
            elt.setAttribute("fillStyle", "Generator");
            if (d->generator)
            {
                elt.setAttribute("generator", d->generator->name());
                QDomElement filterConfigElt = doc.createElement("Generator");
                d->generator->toXML(doc, filterConfigElt);
                elt.appendChild(filterConfigElt);
            }
            break;
    }
}

KisPaintOpPresetSP KisRecordedPaintAction::paintOpPreset() const
{
    return d->paintOpPreset;
}

void KisRecordedPaintAction::setPaintOpPreset(KisPaintOpPresetSP preset)
{
    d->paintOpPreset = preset;
}

qreal KisRecordedPaintAction::opacity() const
{
    return d->opacity;
}

void KisRecordedPaintAction::setOpacity(qreal opacity)
{
    d->opacity = opacity;
}

KoColor KisRecordedPaintAction::paintColor() const
{
    return d->foregroundColor;
}

void KisRecordedPaintAction::setPaintColor(const KoColor& color)
{
    d->foregroundColor = color;
}

KoColor KisRecordedPaintAction::backgroundColor() const
{
    return d->backgroundColor;
}

void KisRecordedPaintAction::setBackgroundColor(const KoColor& color)
{
    d->backgroundColor = color;
}

QString KisRecordedPaintAction::compositeOp()
{
    return d->compositeOp;
}

void KisRecordedPaintAction::setCompositeOp(const QString& id)
{
    d->compositeOp = id;
}

void KisRecordedPaintAction::setPaintIncremental(bool v)
{
    d->paintIncremental = v;
}

void KisRecordedPaintAction::setStrokeStyle(KisPainter::StrokeStyle strokeStyle)
{
  d->strokeStyle = strokeStyle;
}

void KisRecordedPaintAction::setFillStyle(KisPainter::FillStyle fillStyle)
{
  d->fillStyle = fillStyle;
}

void KisRecordedPaintAction::setPattern(const KisPattern* pattern)
{
  d->pattern = pattern;
}

void KisRecordedPaintAction::setGradient(const KoAbstractGradient* gradient)
{
  d->gradient = gradient;
}

void KisRecordedPaintAction::setGenerator(const KisFilterConfiguration * generator)
{
  d->generator = generator;
}

void KisRecordedPaintAction::play(KisNodeSP node, const KisPlayInfo& info) const
{
    Q_UNUSED(node);

    KisImageWSP image = info.image();

    QList<KisNodeSP> nodes = nodeQueryPath().queryNodes(info.image(), info.currentNode());
    foreach(KisNodeSP node, nodes) {
        dbgUI << "Play recorded paint action on node : " << node->name() ;
        KisTransaction * cmd = 0;
        if (info.undoAdapter()) cmd = new KisTransaction("", node->paintDevice());

        KisPaintDeviceSP target = 0;
        if (d->paintIncremental) {
            target = node->paintDevice();
        } else {
            target = new KisPaintDevice(node->paintDevice()->colorSpace());
        }

        KisPainter painter(target);

        if (d->paintIncremental) {
            painter.setCompositeOp(d->compositeOp);
            painter.setOpacity(d->opacity * 255);
        } else {
            painter.setCompositeOp(node->paintDevice()->colorSpace()->compositeOp(COMPOSITE_ALPHA_DARKEN));
            painter.setOpacity(OPACITY_OPAQUE);

        }

        painter.setPaintColor(d->foregroundColor);
        painter.setBackgroundColor(d->backgroundColor);
        d->paintOpPreset->settings()->setNode(node);
        painter.setPaintOpPreset(d->paintOpPreset, info.image());

        painter.setStrokeStyle(d->strokeStyle);
        painter.setFillStyle(d->fillStyle);
        painter.setPattern(d->pattern);
        painter.setGradient(d->gradient);
        painter.setGenerator(d->generator);
        
        playPaint(info, &painter);

        if (!d->paintIncremental) {
            KisPainter painter2(node->paintDevice());
            painter2.setCompositeOp(d->compositeOp);
            painter2.setOpacity(d->opacity * 255);

            QRegion r = painter.dirtyRegion();
            QVector<QRect> dirtyRects = r.rects();
            QVector<QRect>::iterator it = dirtyRects.begin();
            QVector<QRect>::iterator end = dirtyRects.end();
            while (it != end) {
                painter2.bitBlt(it->topLeft(), target, *it);
                ++it;
            }

            node->setDirty(painter2.dirtyRegion());
        } else {
            node->setDirty(painter.dirtyRegion());
        }
        if (info.undoAdapter()) info.undoAdapter()->addCommand(cmd);
        d->paintOpPreset->settings()->setNode(0);
    }
}

void KisRecordedPaintActionFactory::setupPaintAction(KisRecordedPaintAction* action, const QDomElement& elt, const KisRecordedActionLoadContext* context)
{
    QString name = elt.attribute("name");

    qreal opacity = opacityFromXML(elt);
    dbgKrita << ppVar(opacity);
    bool paintIncremental = paintIncrementalFromXML(elt);

    QString compositeOp = compositeOpFromXML(elt);
    // Decode colors

    KoColor bC = backgroundColorFromXML(elt);
    KoColor fC = paintColorFromXML(elt);
    
    action->setName(name);
    action->setBackgroundColor(bC);
    action->setPaintColor(fC);
    action->setOpacity(opacity);
    action->setPaintIncremental(paintIncremental);
    action->setCompositeOp(compositeOp);

    
    // Load stroke style
    QString strokeAttr = elt.attribute("strokeStyle", "None");
    if (strokeAttr == "Brush" )
    {
        action->setStrokeStyle(KisPainter::StrokeStyleBrush);
    } else { // "None"
        action->setStrokeStyle(KisPainter::StrokeStyleNone);
    }
    // Save fill style
    QString fillAttr = elt.attribute("fillStyle", "None");
    if (fillAttr == "PaintColor")
    {
        action->setFillStyle(KisPainter::FillStyleForegroundColor);
    } else if(fillAttr == "AlternativeColor")
    {
        action->setFillStyle(KisPainter::FillStyleBackgroundColor);
    } else if(fillAttr == "Pattern")
    {
        const KisPattern* pattern = context->pattern(elt.attribute("pattern"));
        if (pattern)
        {
            action->setFillStyle(KisPainter::FillStylePattern);
            action->setPattern(pattern);
        } else {
            action->setFillStyle(KisPainter::FillStyleNone);
        }
    } else if(fillAttr == "Gradient")
    {
        const KoAbstractGradient* gradient = context->gradient(elt.attribute("gradient"));
        if (gradient)
        {
            action->setFillStyle(KisPainter::FillStyleGradient);
            action->setGradient(gradient);
        } else {
            action->setFillStyle(KisPainter::FillStyleNone);
        }
    } else if(fillAttr == "Strokes")
    {
        action->setFillStyle(KisPainter::FillStyleStrokes);
    } else if(fillAttr == "Generator")
    {
        KisGeneratorSP g = KisGeneratorRegistry::instance()->value(elt.attribute("generator"));
        KisFilterConfiguration* config = 0;
        if (g)
        {
            config = g->defaultConfiguration(0);
            QDomElement paramsElt = elt.firstChildElement("Generator");
            if (config && !paramsElt.isNull()) {
                config->fromXML(paramsElt);
            }
        }
        if(config)
        {
            action->setFillStyle(KisPainter::FillStyleGenerator);
            action->setGenerator(config);
        } else {
            action->setFillStyle(KisPainter::FillStyleNone);
        }
    }
}

KisPaintOpPresetSP KisRecordedPaintActionFactory::paintOpPresetFromXML(const QDomElement& elt)
{

    QDomElement settingsElt = elt.firstChildElement("PaintopPreset");
    if (!settingsElt.isNull()) {
        KisPaintOpPresetSP settings = new KisPaintOpPreset;
        settings->fromXML(settingsElt);
        return settings;
    } else {
        errImage << "No <PaintopPreset /> found";
        return 0;
    }
}

KoColor KisRecordedPaintActionFactory::paintColorFromXML(const QDomElement& elt)
{
    return colorFromXML(elt, "ForegroundColor");
}

KoColor KisRecordedPaintActionFactory::backgroundColorFromXML(const QDomElement& elt)
{
    return colorFromXML(elt, "BackgroundColor");
}

KoColor KisRecordedPaintActionFactory::colorFromXML(const QDomElement& elt, const QString& elementName)
{
    QDomElement colorElt = elt.firstChildElement(elementName);
    KoColor bC;

    if (!colorElt.isNull()) {
        bC = KoColor::fromXML(colorElt.firstChildElement(), Integer8BitsColorDepthID.id(), QHash<QString, QString>());
        bC.setOpacity(255);
        dbgImage << elementName << " color : " << bC.toQColor();
    } else {
        dbgImage << "Warning: no <" << elementName << " /> found";
    }
    return bC;
}

qreal KisRecordedPaintActionFactory::opacityFromXML(const QDomElement& elt)
{
    return elt.attribute("opacity", "1.0").toDouble();
}

bool KisRecordedPaintActionFactory::paintIncrementalFromXML(const QDomElement& elt)
{
    return elt.attribute("paintIncremental", "1").toInt();
}

QString KisRecordedPaintActionFactory::compositeOpFromXML(const QDomElement& elt)
{
    return elt.attribute("compositeOp", COMPOSITE_OVER);
}

KisNodeQueryPath KisRecordedPaintActionFactory::nodeQueryPathFromXML(const QDomElement& elt)
{
    return KisNodeQueryPath::fromString(elt.attribute("path"));
}
