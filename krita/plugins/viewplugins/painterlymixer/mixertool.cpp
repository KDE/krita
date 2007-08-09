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

#include <kdebug.h>

#include <KoCanvasBase.h>
#include <KoCanvasResourceProvider.h>
#include <KoColor.h>
#include <KoColorSpace.h>
#include <KoID.h>
#include <KoPointerEvent.h>

#include "kis_iterators_pixel.h"
#include "kis_paint_device.h"
#include "kis_paint_information.h"
#include "kis_painter.h"
#include "kis_painterly_overlay.h"
#include "kis_paintop.h"
#include "kis_paintop_registry.h"
#include "kis_resource_provider.h"

#include "kis_painterly_information.h"
#include "mathematics.h"
#include "mixercanvas.h"

#include "mixertool.h"

MixerTool::MixerTool(MixerCanvas *canvas, KisPaintDevice* device, KisPainterlyOverlay *overlay, KoCanvasResourceProvider *rp)
    : KoTool(canvas), m_canvasDevice(device), m_canvasOverlay(overlay), m_resources(rp)
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
    KisPaintDeviceSP stroke = new KisPaintDevice(m_canvasDevice->colorSpace());
    KisPainterlyOverlaySP overlay = new KisPainterlyOverlay;

	KisPainter painter(stroke);

	KisPaintOp *current = KisPaintOpRegistry::instance()->paintOp(
                          m_resources->resource(KisResourceProvider::CurrentPaintop).value<KoID>().id(),
                          static_cast<KisPaintOpSettings*>(m_resources->resource(KisResourceProvider::CurrentPaintopSettings).value<void*>()),
                          &painter, 0);

    if (current->painterly()) {
        QRect rc = m_canvasDevice->exactBounds();
		painter.end();
		painter.begin(overlay);
		painter.bitBlt(rc.topLeft(), m_canvasOverlay, rc);
        painter.end();
		painter.begin(stroke);
        painter.bitBlt(rc.topLeft(), m_canvasDevice, rc);
    }

	painter.setPaintColor(m_resources->foregroundColor());
    painter.setBackgroundColor(m_resources->backgroundColor());
    painter.setBrush(static_cast<KisBrush*>(m_resources->resource(KisResourceProvider::CurrentBrush).value<void*>()));
	painter.setPaintOp(current); // The painter now has the paintop and will destroy it.

    current->paintLine(KisPaintInformation(lastPos, e->pressure(), e->xTilt(), e->yTilt()),
                       KisPaintInformation(e->pos(), e->pressure(), e->xTilt(), e->yTilt()));

    painter.end();

    lastPos = e->pos();

    if (!current->painterly()) {
        mixPaint(stroke, overlay, e);
        updateResources(stroke, overlay);
    } else
        preserveProperties(overlay);

    QRect rc = stroke->exactBounds() & m_canvas->canvasWidget()->rect();
    painter.begin(m_canvasDevice);
    painter.bitBlt(rc.topLeft(), stroke, rc);
    painter.end();
	painter.begin(m_canvasOverlay);
    painter.bitBlt(rc.topLeft(), overlay, rc);
    painter.end();

    m_canvas->updateCanvas(rc);
}

float activeVolume(float volume, float wetness, float force)
{
    return volume * maths::sigmoid(wetness) * maths::sigmoid(force);
}

void mixColorChannels(uint nchannels, float *stroke, float *canvas, PainterlyCell *strCell, PainterlyCell *canCell, float force)
{
	float V_c, V_s; // Volumes in Canvas and Stroke
    float w_c, w_s; // Wetness in the Canvas and in the Stroke
    float V_ac, V_as; // Active Volumes in Canvas and Stroke

    V_c = canCell->volume;
    V_s = strCell->volume;

    w_c = canCell->wetness;
    w_s = strCell->wetness;

    V_ac = activeVolume(V_c, w_c, force);
    V_as = activeVolume(V_s, w_s, force);

	for (quint32 i = 0; i < nchannels; i++) {
		stroke[i] = (V_ac * canvas[i] + V_as * stroke[i]) / (V_ac + V_as);
	}
}

void mixProperties(PainterlyCell *strCell, PainterlyCell *canCell, float force)
{
	float V_c, V_s; // Volumes in Canvas and Stroke
    float w_c, w_s; // Wetness in the Canvas and in the Stroke
    float V_ac, V_as; // Active Volumes in Canvas and Stroke
    float a; // Adsorbency
    float V_f, w_f; // Finals

	V_c = canCell->volume;
    V_s = strCell->volume;

    w_c = canCell->wetness;
    w_s = strCell->wetness;

    V_ac = activeVolume(V_c, w_c, force);
    V_as = activeVolume(V_s, w_s, force);

    V_ac = activeVolume(V_c, w_c, force);
    V_as = activeVolume(V_s, w_s, force);

    a = strCell->adsorbency;

    w_f = (V_ac * w_c + V_as * w_s) / (V_ac + V_as);
    V_f = ((1 - a) * V_c) + V_as;

    if (V_f > 255.0)
        V_f = 255.0;

    strCell->wetness = w_f;
    strCell->volume = V_f;
}

#define FORCE_COEFF 0.01

void MixerTool::mixPaint(KisPaintDeviceSP stroke, KisPainterlyOverlaySP overlay, KoPointerEvent *e)
{
    float pressure, force;
    QColor strokeColor, canvasColor;
	KoColorSpace *cs = stroke->colorSpace();

    QRect rc = stroke->exactBounds();
	KisRectIteratorPixel it_stroke_main = stroke->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height());
	KisRectIteratorPixel it_stroke_over = overlay->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height());
	KisRectIteratorPixel it_canvas_main = m_canvasDevice->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height());
	KisRectIteratorPixel it_canvas_over = m_canvasOverlay->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height());

    pressure = e->pressure();
    force = pressure + FORCE_COEFF * pow(pressure, 2);
    while (!it_stroke_main.isDone()) {
		PainterlyCell *strokeCell =
                reinterpret_cast<PainterlyCell *>( it_stroke_over.rawData() );
		PainterlyCell *canvasCell =
                reinterpret_cast<PainterlyCell *>( it_canvas_over.rawData() );

        if (cs->alpha(it_stroke_main.rawData())) {
            strokeCell->wetness = m_info["Wetness"];
            strokeCell->mixability = m_info["Mixability"] * strokeCell->wetness;
            strokeCell->pigment_concentration = m_info["PigmentConcentration"];
            strokeCell->viscosity = m_info["Viscosity"] / strokeCell->wetness;
            strokeCell->volume = m_info["PaintVolume"];
            strokeCell->adsorbency = m_info["CanvasAdsorbency"];

            if (cs->alpha(it_canvas_main.rawData())) {
                mixColorChannels(cs->colorChannelCount(),
								 reinterpret_cast<float *>(it_stroke_main.rawData()),
								 reinterpret_cast<float *>(it_canvas_main.rawData()),
								 strokeCell, canvasCell, force);

                mixProperties(strokeCell, canvasCell, force);
            }
        }

        ++it_stroke_main; ++it_canvas_main;
        ++it_stroke_over; ++it_canvas_over;
    }
}

void MixerTool::updateResources(KisPaintDeviceSP stroke, KisPainterlyOverlaySP /*overlay*/)
{
    // TODO Update tool's own KisPainterlyInformation structure.
	int channels = stroke->colorSpace()->channelCount();

	float *current;
	float final[channels];
	int total = 0;

    QRect rc = stroke->exactBounds();
    KisRectIteratorPixel
        it_main = stroke->createRectIterator(rc.x(),rc.y(),rc.width(),rc.height());

	for (int i = 0; i < channels-1; i++)
		final[i] = 0;
	final[channels-1] = 1;

    while (!it_main.isDone()) {
		current = reinterpret_cast<float *>(it_main.rawData());
        if (current[channels-1]) { // opacity
            for (int i = 0; i < channels-1; i++)
				final[i] += current[i];

            ++total;
        }
        ++it_main;
    }

    if (total) {
        for (int i = 0; i < channels - 1; i++)
			final[i] /= (float)total;

        m_resources->setForegroundColor(KoColor(reinterpret_cast<quint8 *>(final), stroke->colorSpace()));
    }
}

void MixerTool::preserveProperties(KisPainterlyOverlaySP /*overlay*/)
{

}


#include "mixertool.moc"
