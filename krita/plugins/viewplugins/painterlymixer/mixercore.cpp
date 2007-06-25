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

#include <cmath>

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
#include "kis_paintop_registry.h"
#include "kis_resource_provider.h"

#include "kis_adsorbency_mask.h"
#include "kis_mixability_mask.h"
#include "kis_pigment_concentration_mask.h"
#include "kis_reflectivity_mask.h"
#include "kis_volume_mask.h"
#include "kis_viscosity_mask.h"
#include "kis_wetness_mask.h"

#include "mixercore.h"
#include "kis_painterly_information.h"

/*
Just add all the painterly overlays that we need in only one call
*/
void addPainterlyOverlays(KisPaintDevice* dev)
{
    dev->addPainterlyChannel(new KisAdsorbencyMask(dev));
    dev->addPainterlyChannel(new KisMixabilityMask(dev));
    dev->addPainterlyChannel(new KisPigmentConcentrationMask(dev));
    dev->addPainterlyChannel(new KisReflectivityMask(dev));
    dev->addPainterlyChannel(new KisVolumeMask(dev));
    dev->addPainterlyChannel(new KisViscosityMask(dev));
    dev->addPainterlyChannel(new KisWetnessMask(dev));
}

MixerCanvas::MixerCanvas(QWidget *parent)
    : QFrame(parent), KoCanvasBase(0), m_tool(0), m_toolProxy(0)
{

}

MixerCanvas::~MixerCanvas()
{
    if (m_toolProxy)
        delete m_toolProxy;
    if (m_tool)
        delete m_tool;
}

void MixerCanvas::initDevice(KoColorSpace *cs, KoCanvasResourceProvider *rp)
{
    m_canvasDev = new KisPaintDevice(cs);
    addPainterlyOverlays(m_canvasDev.data());

    m_tool = new MixerTool(this, m_canvasDev.data(), rp);
    m_toolProxy = new KoToolProxy(this);
    m_toolProxy->setActiveTool(m_tool);
}

void MixerCanvas::initSpots(QFrame *sf)
{
    // TODO Initialize spots
}

void MixerCanvas::mouseDoubleClickEvent(QMouseEvent *event)
{
//     m_toolProxy->mouseDoubleClickEvent(event, event->pos());
}

void MixerCanvas::mouseMoveEvent(QMouseEvent *event)
{
    m_toolProxy->mouseMoveEvent(event, event->pos());
}

void MixerCanvas::mousePressEvent(QMouseEvent *event)
{
    m_toolProxy->mousePressEvent(event, event->pos());
}

void MixerCanvas::mouseReleaseEvent(QMouseEvent *event)
{
    m_toolProxy->mouseReleaseEvent(event, event->pos());
}

void MixerCanvas::tabletEvent(QTabletEvent *event)
{
    m_toolProxy->tabletEvent(event, event->pos());
}

void MixerCanvas::paintEvent(QPaintEvent *event)
{
    QFrame::paintEvent(event);

    QRect r = rect();
    QPainter p(this);
    p.drawImage(r, m_canvasDev->convertToQImage(0, r.x(), r.y(), r.width(), r.height()), r);
    p.end();
}

void MixerCanvas::updateCanvas(const QRectF& rc)
{
    update(rc.toRect());
}

/////////////////
// THE MIXER TOOL
/////////////////

MixerTool::MixerTool(MixerCanvas *canvas, KisPaintDevice* device, KoCanvasResourceProvider *rp)
    : KoTool(canvas), m_canvasDev(device), m_resources(rp)
{
    //{{ - Just for testing!
    m_info.PaintType = "Test";
    m_info.Mixability = 0.9f;
    m_info.PigmentConcentration = 0.9f;
    m_info.PaintVolume = 200.0f;
    m_info.Reflectivity = 0.1f;
    m_info.Viscosity = 0.2f;
    m_info.Wetness = 0.8f;
    //}}
}

MixerTool::~MixerTool()
{
}

void MixerTool::mousePressEvent(KoPointerEvent *e)
{
//     kDebug() << "MOUSE PRESSED!! " << event->pos() << endl;
    mouseMoveEvent(e);
}

void MixerTool::mouseReleaseEvent(KoPointerEvent */*e*/)
{
//     kDebug() << "MOUSE RELEASED!! " << event->pos() << endl;
}

void MixerTool::mouseMoveEvent(KoPointerEvent *e)
{
    KisPaintDeviceSP stroke = new KisPaintDevice(m_canvasDev->colorSpace());
    addPainterlyOverlays(stroke.data());

    //{{ KisPainter initialization - Put it in another function?
    KisPainter painter(stroke);
    KisPaintOp *current = KisPaintOpRegistry::instance()->paintOp(
                          m_resources->resource(KisResourceProvider::CurrentPaintop).value<KoID>(),
                          static_cast<KisPaintOpSettings*>(m_resources->resource(KisResourceProvider::CurrentPaintopSettings).value<void*>()),
                          &painter, 0);
    painter.setPaintOp(current); // The painter now has the paintop and will destroy it.
    painter.setPaintColor(m_resources->resource(KoCanvasResource::ForegroundColor).value<KoColor>());
    painter.setBackgroundColor(m_resources->resource(KoCanvasResource::BackgroundColor).value<KoColor>());
    painter.setBrush(static_cast<KisBrush*>(m_resources->resource(KisResourceProvider::CurrentBrush).value<void*>()));
    //}}

    if (current->painterly()) {
        QRect rc = m_canvasDev->exactBounds();
        painter.bitBlt(rc.topLeft(), m_canvasDev, rc);
        painter.copyMasks(rc.topLeft(), m_canvasDev, rc);
        painter.end();
    }

    painter.paintAt(KisPaintInformation(e->pos(), e->pressure(), e->xTilt(), e->yTilt()));
    painter.end();

    if (!current->painterly()) {
        updatePainterlyOverlays(stroke, e);
//         mixColors(stroke, e);
        updateResources(stroke);
    }

    QRect rc = stroke->exactBounds();
    painter.begin(m_canvasDev);
    painter.bitBlt(rc.topLeft(), stroke, rc);
    painter.copyMasks(rc.topLeft(), stroke, rc);
    painter.end();

    m_canvas->updateCanvas(rc);
}

#define RHO2(x,x0,y,y0) (x-x0)*(x-x0) + (y-y0)*(y-y0)
#define C_EXP 0.0001f
#define C_FRICTION 0.1f
#define MIN_ALPHA 1

void MixerTool::updatePainterlyOverlays(KisPaintDeviceSP stroke, KoPointerEvent *e)
{
    float p, f; // Pressure and force
    float rho2; // Square of the distance from the event point
    int x, y; // Actual point
    float pig_con, can_pig_con, fin_pig_con;
    float wetness, can_wetness, fin_wetness; // Actual point wetness
    float pvolume, can_pvolume, fin_pvolume, act_pvolume, ave_pvolume = 0, ave_act_pvolume = 0;
    float reflect, can_reflect, fin_reflect;
    float pviscos, can_pviscos, fin_pviscos;
    float mixabil, can_mixabil, fin_mixabil;
    float p0 = e->pressure();
    int x0 = e->x(),
        y0 = e->y();
    QRect rc = stroke->exactBounds();
    QColor color, can_color, fin_color, ave_color;
    qreal hue, sat, val;
    qreal can_hue, can_sat, can_val;
    qreal fin_hue, fin_sat, fin_val;
    qreal ave_hue = 0, ave_sat = 0, ave_val = 0;
    quint8 opacity, can_opacity, fin_opacity;
    int total = 0;

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

    while (!it_main.isDone()) {
        stroke->colorSpace()->toQColor(it_main.rawData(), &color, &opacity); // Is this too expensive?
        m_canvasDev->colorSpace()->toQColor(can_it_main.rawData(), &can_color, &can_opacity);

        // Do something if there is actually some paint
        if (opacity > 0) {
//             kDebug() << "---------- STARTING!! --------" << endl;
//             kDebug() << "STROKE COLOR: " << color << " OPACITY: " << (int)opacity << endl;
//             kDebug() << "CANVAS COLOR: " << can_color <<  " OPACITY: " << (int)can_opacity << endl;

            x = it_main.x();
            y = it_main.y();
            rho2 = RHO2(x,x0,y,y0);
            p = p0*exp(-rho2*C_EXP);
            // TODO Make the friction coefficient user configurable! (And find a good and complete meaning for it)
            f = p + C_FRICTION*pow(p,2);

//             kDebug() << "PRESSURE: " << p << " - FORCE: " << f << endl;

            wetness = p * m_info.Wetness; // Wetness doesn't depend on friction.
            pvolume = f * m_info.PaintVolume;
            pviscos = f * m_info.Viscosity / wetness;
            reflect = wetness * m_info.Reflectivity;
            pig_con = f * m_info.PigmentConcentration;
            mixabil = m_info.Mixability;

            *it_wetn.rawData() = (quint8)(wetness*256.0f);
            *it_volu.rawData() = (quint8)(pvolume);
            *it_visc.rawData() = (quint8)(pviscos*256.0f);
            *it_refl.rawData() = (quint8)(reflect*256.0f);
            *it_pigm.rawData() = (quint8)(pig_con*256.0f);
            *it_mixa.rawData() = (quint8)(mixabil*256.0f);

//             kDebug() << "STROKE: " << endl;
//             kDebug() << "\tWETN: " << (int)*it_wetn.rawData() << endl
//                      << "\tVOLU: " << (int)*it_volu.rawData() << endl
//                      << "\tVISC: " << (int)*it_visc.rawData() << endl
//                      << "\tREFL: " << (int)*it_refl.rawData() << endl
//                      << "\tPIGM: " << (int)*it_pigm.rawData() << endl
//                      << "\tMIXA: " << (int)*it_mixa.rawData() << endl;

            /*
            * The Active Volume is directly proportional to pressure and to the wetness.
            * We mix colors taking only the active volume of the canvas. The volume in the
            * stroke is obviously already the active volume.
            * Here we came to the question: what color we pick up in the paintop?
            * My current idea is to take the average color in the stroke after the mixing
            * and set it as the current color. More in the updateResources() method.
            */

//             kDebug() << "CANVAS: " << endl;
//             kDebug() << "\tWETN: " << (int)*can_it_wetn.rawData() << endl
//                      << "\tVOLU: " << (int)*can_it_volu.rawData() << endl
//                      << "\tVISC: " << (int)*can_it_visc.rawData() << endl
//                      << "\tREFL: " << (int)*can_it_refl.rawData() << endl
//                      << "\tPIGM: " << (int)*can_it_pigm.rawData() << endl
//                      << "\tMIXA: " << (int)*can_it_mixa.rawData() << endl;

            can_wetness = (float)*can_it_wetn.rawData()/256.0f;
            can_pvolume = (float)*can_it_volu.rawData();
            can_pviscos = (float)*can_it_visc.rawData()/256.0f;
            can_reflect = (float)*can_it_refl.rawData()/256.0f;
            can_pig_con = (float)*can_it_pigm.rawData()/256.0f;
            can_mixabil = (float)*can_it_mixa.rawData()/256.0f;

            act_pvolume = can_pvolume;

//             kDebug() << "\tACTV: " << act_pvolume << endl;

//             red = (float)color.red();   can_red = (float)can_color.red();
//             sat = (float)color.saten(); can_sat = (float)can_color.saten();
//             val = (float)color.vale();  can_val = (float)can_color.vale();

            color.getHsvF(&hue, &sat, &val);
            can_color.getHsvF(&can_hue, &can_sat, &can_val);

//             kDebug() << "STROKE - hue: " << red << " sat: " << sat << " val: " << val << endl;
//             kDebug() << "CANVAS - RED: " << can_red << " sat: " << can_sat << " val: " << can_val << endl;

            fin_hue = (pvolume*hue + act_pvolume*can_hue) / (act_pvolume + pvolume);
            fin_sat = (pvolume*sat + act_pvolume*can_sat) / (act_pvolume + pvolume);
            fin_val = (pvolume*val + act_pvolume*can_val) / (act_pvolume + pvolume);

            ave_hue += fin_hue;
            ave_sat += fin_sat;
            ave_val += fin_val;
            ave_pvolume += pvolume;
            ave_act_pvolume += act_pvolume;
            total += 1;

//             kDebug() << "FINAL  - hue: " << fin_hue << " sat: " << fin_sat << " val: " << fin_val << endl;

            fin_wetness = (wetness + can_wetness)/2.0f;
            fin_pig_con = (pig_con + can_pig_con)/2.0f;
            fin_pvolume = (pvolume + act_pvolume)/2.0f;
            fin_pviscos = (pviscos + can_pviscos)/2.0f;
            fin_reflect = (reflect + can_reflect)/2.0f;
            fin_mixabil = (mixabil + can_mixabil)/2.0f;

            fin_opacity = (opacity + can_opacity)/2;
            fin_color.setHsvF(fin_hue, fin_sat, fin_val);

//             kDebug() << "FINAL COLOR: " << fin_color << " OPACITY: " << (int)fin_opacity << endl;

            stroke->colorSpace()->fromQColor(fin_color, fin_opacity, it_main.rawData());

            *it_wetn.rawData() = (quint8)(fin_wetness*256.0f);
            *it_volu.rawData() = (quint8)(fin_pvolume);
            *it_visc.rawData() = (quint8)(fin_pviscos*256.0f);
            *it_refl.rawData() = (quint8)(fin_reflect*256.0f);
            *it_pigm.rawData() = (quint8)(fin_pig_con*256.0f);
            *it_mixa.rawData() = (quint8)(fin_mixabil*256.0f);

//             kDebug() << "FINAL: " << endl;
//             kDebug() << "\tWETN: " << (int)*it_wetn.rawData() << endl
//                      << "\tVOLU: " << (int)*it_volu.rawData() << endl
//                      << "\tVISC: " << (int)*it_visc.rawData() << endl
//                      << "\tREFL: " << (int)*it_refl.rawData() << endl
//                      << "\tPIGM: " << (int)*it_pigm.rawData() << endl
//                      << "\tMIXA: " << (int)*it_mixa.rawData() << endl;
        }

        for (int _i=0;_i<7;_i++) {++(*iters[_i]); ++(*can_iters[_i]);}
        ++it_main; ++can_it_main;
    }

    if (total) {
        qreal pre_hue, pre_sat, pre_val;
        QColor pre_color;

        pre_color = m_resources->resource(KoCanvasResource::ForegroundColor).value<KoColor>().toQColor();
        pre_color.getHsvF(&pre_hue, &pre_sat, &pre_val);

        ave_hue = ave_hue/total;
        ave_sat = ave_sat/total;
        ave_val = ave_val/total;
        ave_pvolume = ave_pvolume/total;
        ave_act_pvolume = ave_act_pvolume/total;

        ave_hue = (ave_pvolume*pre_hue + ave_act_pvolume*ave_hue) / (ave_act_pvolume + ave_pvolume);
        ave_sat = (ave_pvolume*pre_sat + ave_act_pvolume*ave_sat) / (ave_act_pvolume + ave_pvolume);
        ave_val = (ave_pvolume*pre_val + ave_act_pvolume*ave_val) / (ave_act_pvolume + ave_pvolume);

        ave_color.setHsvF(ave_hue, ave_sat, ave_val);

        m_resources->setResource(KoCanvasResource::ForegroundColor, KoColor(ave_color, stroke->colorSpace()));
    }
}

void MixerTool::mixColors(KisPaintDeviceSP stroke, KoPointerEvent *e)
{

}

void MixerTool::updateResources(KisPaintDeviceSP stroke)
{
    // TODO Update the color and the tool's own KisPainterlyInformation structure.
}


#include "mixercore.moc"
