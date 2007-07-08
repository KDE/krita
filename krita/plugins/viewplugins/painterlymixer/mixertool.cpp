/* This file is part of the KDE project
   Made by Emanuele Tamponi (emanuele@valinor.it)
   Copyright (C) 2007 Emanuele Tamponi

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#include <QCursor>
#include <QFrame>

#include <kdebug.h>

#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoID.h>
#include <KoPointerEvent.h>
#include <KoShapeManager.h>
#include <KoToolProxy.h>
#include <KoUnit.h>
#include <KoViewConverter.h>

#include "kis_iterators_pixel.h"
#include "kis_paint_device.h"
#include "kis_painter.h"
#include "kis_paintop.h"
#include "kis_paint_information.h"
#include "kis_paintop_registry.h"
#include "kis_resource_provider.h"

#include "kis_adsorbency_mask.h"
#include "kis_mixability_mask.h"
#include "kis_pigment_concentration_mask.h"
#include "kis_reflectivity_mask.h"
#include "kis_volume_mask.h"
#include "kis_viscosity_mask.h"
#include "kis_wetness_mask.h"

#include "mixercanvas.h"
#include "kis_painterly_information.h"
#include "utilities.h"

#include "mixertool.h"

MixerTool::MixerTool(MixerCanvas *canvas, KisPaintDevice* device, KoCanvasResourceProvider *rp)
    : KoTool(canvas), m_canvasDev(device), m_resources(rp)
{
    initBristleInformation();
}

MixerTool::~MixerTool()
{
}

#define INIT_CANVAS_ADSORBENCY 0.3
#define INIT_MIXABILITY 0.9
#define INIT_PIGMENT_CONCENTRATION 0.9
#define INIT_PAINT_VOLUME 120.0
#define INIT_REFLECTIVITY 0.1
#define INIT_VISCOSITY 0.4
#define INIT_WETNESS 0.5

void MixerTool::initBristleInformation()
{
    // TODO Save/Load of bristle information
    // Use hard-coded values for now
    m_info["CanvasAdsorbency"] = INIT_CANVAS_ADSORBENCY;
    m_info["Mixability"] = INIT_MIXABILITY;
    m_info["PigmentConcentration"] = INIT_PIGMENT_CONCENTRATION;
    m_info["PaintVolume"] = INIT_PAINT_VOLUME;
    m_info["Reflectivity"] = INIT_REFLECTIVITY;
    m_info["Viscosity"] = INIT_VISCOSITY;
    m_info["Wetness"] = INIT_WETNESS;
}

void MixerTool::mousePressEvent(KoPointerEvent *e)
{
    lastPos = e->pos();
    mouseMoveEvent(e);
}

void MixerTool::mouseReleaseEvent(KoPointerEvent */*e*/)
{
}

void MixerTool::mouseMoveEvent(KoPointerEvent *e)
{
    KisPaintDeviceSP stroke = new KisPaintDevice(m_canvasDev->colorSpace());
    addPainterlyOverlays(stroke.data());

    //{{ KisPainter initialization - Put it in another function?
    KisPainter painter(stroke);
    KisPaintOp *current = KisPaintOpRegistry::instance()->paintOp(
                          m_resources->resource(KisResourceProvider::CurrentPaintop).value<KoID>().id(),
                          static_cast<KisPaintOpSettings*>(m_resources->resource(KisResourceProvider::CurrentPaintopSettings).value<void*>()),
                          &painter, 0);
    painter.setPaintOp(current); // The painter now has the paintop and will destroy it.
    painter.setPaintColor(m_resources->foregroundColor());
    painter.setBackgroundColor(m_resources->backgroundColor());
    painter.setBrush(static_cast<KisBrush*>(m_resources->resource(KisResourceProvider::CurrentBrush).value<void*>()));
    //}}

    if (current->painterly()) {
        QRect rc = m_canvasDev->exactBounds();
        painter.bitBlt(rc.topLeft(), m_canvasDev, rc);
        painter.copyMasks(rc.topLeft(), m_canvasDev, rc);
        painter.end();
    }

    painter.paintLine(KisPaintInformation(lastPos, e->pressure(), e->xTilt(), e->yTilt()),
                      KisPaintInformation(e->pos(), e->pressure(), e->xTilt(), e->yTilt()));
    painter.end();

    lastPos = e->pos();

    if (!current->painterly()) {
        mixPaint(stroke, e);
        updateResources(stroke);
    } else
        preserveProperties(stroke);

    QRect rc = stroke->exactBounds();
    painter.begin(m_canvasDev);
    painter.bitBlt(rc.topLeft(), stroke, rc);
    painter.copyMasks(rc.topLeft(), stroke, rc);
    painter.end();

    m_canvas->updateCanvas(rc);
}


#define FORCE_COEFF 0.01

void MixerTool::mixPaint(KisPaintDeviceSP stroke, KoPointerEvent *e)
{
    float pressure, force;
    Cell strokeCell, canvasCell;
    QColor strokeColor, canvasColor;

    QRect rc = stroke->exactBounds();
    KisRectIteratorPixel             // Give a more or less clear name to each iterator.
        it_main = stroke->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        it_adso = stroke->painterlyChannel("KisAdsorbencyMask")->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        it_mixa = stroke->painterlyChannel("KisMixabilityMask")->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        it_pigm = stroke->painterlyChannel("KisPigmentConcentrationMask")->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        it_refl = stroke->painterlyChannel("KisReflectivityMask")->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        it_volu = stroke->painterlyChannel("KisVolumeMask")->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        it_visc = stroke->painterlyChannel("KisViscosityMask")->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        it_wetn = stroke->painterlyChannel("KisWetnessMask")->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height());
    KisRectIteratorPixel *iters[7] = // Used only to cleanly increase the iterators in the next cycle
        {&it_adso, &it_mixa, &it_pigm, &it_refl, &it_volu, &it_visc, &it_wetn};

    KisRectConstIteratorPixel        // Give a more or less clear name to each iterator.
        can_it_main = m_canvasDev->createRectConstIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        can_it_adso = m_canvasDev->painterlyChannel("KisAdsorbencyMask")->createRectConstIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        can_it_mixa = m_canvasDev->painterlyChannel("KisMixabilityMask")->createRectConstIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        can_it_pigm = m_canvasDev->painterlyChannel("KisPigmentConcentrationMask")->createRectConstIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        can_it_refl = m_canvasDev->painterlyChannel("KisReflectivityMask")->createRectConstIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        can_it_volu = m_canvasDev->painterlyChannel("KisVolumeMask")->createRectConstIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        can_it_visc = m_canvasDev->painterlyChannel("KisViscosityMask")->createRectConstIterator(rc.x(),rc.y(),rc.width(),rc.height()),
        can_it_wetn = m_canvasDev->painterlyChannel("KisWetnessMask")->createRectConstIterator(rc.x(),rc.y(),rc.width(),rc.height());
    KisRectConstIteratorPixel *can_iters[7] = // Used only to cleanly increase the iterators in the next cycle
        {&can_it_adso, &can_it_mixa, &can_it_pigm, &can_it_refl, &can_it_volu, &can_it_visc, &can_it_wetn};

    pressure = e->pressure();
    force = pressure + FORCE_COEFF * pow(pressure, 2);
    while (!it_main.isDone()) {
        stroke->colorSpace()->toQColor(it_main.rawData(), &strokeColor, &strokeCell.opacity); // This is not useful!
        m_canvasDev->colorSpace()->toQColor(can_it_main.rawData(), &canvasColor, &canvasCell.opacity);

        if (strokeCell.opacity) {
            // Initializing stroke cell
            strokeCell.wetness = m_info["Wetness"];
            strokeCell.mixability = m_info["Mixability"] * strokeCell.wetness;
            strokeCell.pigmentConcentration = m_info["PigmentConcentration"];
            strokeCell.reflectivity = m_info["Reflectivity"] * strokeCell.wetness;
            strokeCell.viscosity = m_info["Viscosity"] / strokeCell.wetness;
            strokeCell.volume = m_info["PaintVolume"];
            strokeCell.canvasAdsorbency = m_info["CanvasAdsorbency"];
            strokeCell.setRgb(strokeColor.red(),
                              strokeColor.green(),
                              strokeColor.blue());

            if (canvasCell.opacity) {
                // Loading canvas cell
                canvasCell.canvasAdsorbency = (float)*can_it_adso.rawData() / 255.0;
                canvasCell.mixability = (float)*can_it_mixa.rawData() / 255.0;
                canvasCell.pigmentConcentration = (float)*can_it_pigm.rawData();
                canvasCell.reflectivity = (float)*can_it_refl.rawData() / 255.0;
                canvasCell.viscosity = (float)*can_it_visc.rawData() / 255.0;
                canvasCell.volume = (float)*can_it_volu.rawData();
                canvasCell.wetness = (float)*can_it_wetn.rawData() / 255.0;
                canvasCell.setRgb(canvasColor.red(),
                                   canvasColor.green(),
                                   canvasColor.blue());

                strokeCell.mixColorsUsingRgb(canvasCell, force);
//                 strokeCell.mixColorsUsingHls(canvasCell, force);
//                 strokeCell.mixColorsUsingCmy(canvasCell, force);

                strokeCell.mixProperties(canvasCell, force);

                strokeColor.setRgb(strokeCell.red, strokeCell.green, strokeCell.blue);
            }
            stroke->colorSpace()->fromQColor(strokeColor, strokeCell.opacity, it_main.rawData());

            *it_adso.rawData() = (quint8)(strokeCell.canvasAdsorbency*255.0);
            *it_wetn.rawData() = (quint8)(strokeCell.wetness*255.0);
            *it_volu.rawData() = (quint8)(strokeCell.volume);
            *it_visc.rawData() = (quint8)(strokeCell.viscosity*255.0);
            *it_refl.rawData() = (quint8)(strokeCell.reflectivity*255.0);
            *it_pigm.rawData() = (quint8)(strokeCell.pigmentConcentration*255.0);
            *it_mixa.rawData() = (quint8)(strokeCell.mixability*255.0);
        }

        ++it_main; ++can_it_main;
        for (int _i = 0; _i < 7; _i++) {++(*iters[_i]); ++(*can_iters[_i]); }
    }
}

void MixerTool::updateResources(KisPaintDeviceSP stroke)
{
    // TODO Update tool's own KisPainterlyInformation structure.
    QColor current;
    int c_r, c_g, c_b;
    long r = 0, g = 0, b = 0;
    int total = 0;
    quint8 opacity;

    QRect rc = stroke->exactBounds();
    KisRectIteratorPixel
        it_main = stroke->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height());

    while (!it_main.isDone()) {
        stroke->colorSpace()->toQColor(it_main.rawData(), &current, &opacity);
        if (opacity) {
            current.getRgb(&c_r, &c_g, &c_b);
            r += c_r;
            g += c_g;
            b += c_b;
            ++total;
        }
        ++it_main;
    }

    if (total) {
        QColor final;

        r /= total;
        g /= total;
        b /= total;
        final.setRgb(r, g, b);

        m_resources->setForegroundColor(KoColor(final, stroke->colorSpace()));
    }
}

void MixerTool::preserveProperties(KisPaintDeviceSP /*stroke*/)
{

}


#include "mixertool.moc"
