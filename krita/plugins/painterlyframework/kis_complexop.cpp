/*
 *  Copyright (c) 2007 Emanuele Tamponi (emanuele@valinor.it)
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

#include <QVector>

#include <kdebug.h>

#include "kis_brush.h"
#include "kis_complex_color.h"
#include "kis_global.h"
#include "kis_paint_device.h"
#include "kis_painterly_overlay.h"
#include "kis_painterly_overlay_colorspace.h"
#include "kis_layer.h"
#include "kis_painter.h"
#include "kis_types.h"
#include "kis_paintop.h"
#include "kis_iterator.h"
#include "kis_iterators_pixel.h"
#include "kis_selection.h"

#include "mathematics.h"

#include "kis_complexop.h"

KisComplexOp::KisComplexOp(KisPainter * painter)
    : super(painter)
{
}

KisComplexOp::~KisComplexOp()
{

}

void KisComplexOp::paintAt(const KisPaintInformation& info)
{
	if (!painter()->brush())
		return;

	if (!painter()->sourceLayer())
		return;

	KisPaintLayerSP layer = dynamic_cast<KisPaintLayer *>(painter()->sourceLayer());

	if (!layer)
		return;

	if (!layer->painterlyOverlay())
		layer->createPainterlyOverlay();

	KoColorSpace *cs = layer->paintDevice()->colorSpace();
	KisPaintLayerSP buffer = new KisPaintLayer(0, "Temporary Buffer", 255, cs);
	buffer->createPainterlyOverlay();

	float pressure = maths::sigmoid(info.pressure);

	{
		KisBrush *brush = painter()->brush();
		KisPaintDeviceSP dab;

		if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
			dab = brush->image(cs, info, 0, 0);
		} else {
			KisQImagemaskSP mask = brush->mask(info, 0, 0);
			dab = computeDab(mask);
		}

		{
			KisPainter p(buffer->paintDevice());
			p.bitBlt(QPoint(0,0), dab, dab->exactBounds());
			p.end();
		}
	}

	QSize sz = buffer->paintDevice()->exactBounds().size();

	if (!painter()->complexColor())
		return;

	KisComplexColor *brush = painter()->complexColor();
	brush->setSize(sz);

	QRect rc(QPoint( (int)info.pos.x() + brush->left(),
			         (int)info.pos.y() + brush->top() ), sz);

	int channels = cs->channelCount();

	KisHLineIteratorPixel bufferIt = buffer->paintDevice()->createHLineIterator(0,0,sz.width());
	KisHLineIteratorPixel buOverIt = buffer->painterlyOverlay()->createHLineIterator(0,0,sz.width());
	KisHLineIteratorPixel canvasIt = layer->paintDevice()->createHLineIterator(rc.x(),rc.y(),sz.width());
	KisHLineIteratorPixel caOverIt = layer->painterlyOverlay()->createHLineIterator(rc.x(),rc.y(),sz.width());

	QVector<float> bufferColor(channels), brushColor(channels), canvasColor(channels);
	PropertyCell *bufferCell, *brushCell, *canvasCell;

	if (brush->colorSpace()->id() != cs->id())
		brush->convertTo(cs);

	for (int y = brush->top(); y < brush->bottom(); y++) {
		for (int x = brush->left(); x < brush->right(); x++) {

			if (cs->alpha(bufferIt.rawData())) { // If alpha != 0

				cs->normalisedChannelsValue(bufferIt.rawData(), bufferColor);		// bufferColor
				bufferCell = reinterpret_cast<PropertyCell *>(buOverIt.rawData());	// bufferCell

				cs->normalisedChannelsValue(brush->rawData(x,y), brushColor);		// brushColor
				brushCell = brush->property(x,y);									// brushCell

				cs->normalisedChannelsValue(canvasIt.rawData(), canvasColor);		// canvasColor
				canvasCell = reinterpret_cast<PropertyCell *>(caOverIt.rawData());	// canvasCell

				float alpha = bufferColor[channels-1] >= canvasColor[channels-1] ?
						bufferColor[channels-1] : canvasColor[channels-1];
/*
				float volume_bc, volume_cb;
				computePaintTransferAmount(brushCell, canvasCell, pressure, 0.6, volume_bc, volume_cb);

				if (volume_bc) { // Paint transfer is from brush to canvas
					// The result of the mixing goes in the buffer.
					mixChannels(bufferColor, canvasColor, canvasCell->volume, brushColor, volume_bc);
					mixProperty(bufferCell, canvasCell, canvasCell->volume, brushCell, volume_bc);

					bufferCell->volume += volume_bc;
					brushCell->volume -= volume_bc;
				}
				if (volume_cb) {
					// The result of the mixing goes in the brush; the buffer gets the color of the canvas!
					bufferColor = canvasColor;
					*bufferCell = *canvasCell;

					mixChannels(brushColor, canvasColor, volume_cb, brushColor, brushCell->volume);
					mixProperty(brushCell, canvasCell, volume_cb, brushCell, brushCell->volume);

					bufferCell->volume -= volume_cb;
					brushCell->volume += volume_cb;
				}
*/

				mixChannels(bufferColor, canvasColor, pressure*canvasCell->volume, brushColor, pressure*brushCell->volume);
				mixProperty(bufferCell, canvasCell, pressure*canvasCell->volume, brushCell, pressure*brushCell->volume);

				bufferColor[channels-1] = alpha;

				if (bufferCell->volume < 0)
					bufferCell->volume = 0;
// 				if (brushCell->volume < 0)
// 					brushCell->volume = 0;
				if (bufferCell->volume > 1000.0)
					bufferCell->volume = 1000.0;
// 				if (brushCell->volume > 1000.0)
// 					brushCell->volume = 1000.0;

				cs->fromNormalisedChannelsValue(bufferIt.rawData(), bufferColor);
				cs->fromNormalisedChannelsValue(brush->rawData(x,y), bufferColor);
				*brushCell = *bufferCell;
			}

			++bufferIt; ++canvasIt;
			++buOverIt; ++caOverIt;
		}

		bufferIt.nextRow();
		canvasIt.nextRow();
		buOverIt.nextRow();
		caOverIt.nextRow();
	}
	painter()->setPressure(pressure);

    if (source()->hasSelection()) {
        painter()->bltSelection(rc.x(), rc.y(), painter()->compositeOp(), buffer->paintDevice(),
                                source()->selection(), painter()->opacity(), 0, 0, sz.width(), sz.height());
    }
    else {
        painter()->bitBlt(rc.x(), rc.y(), painter()->compositeOp(), buffer->paintDevice(), painter()->opacity(), 0, 0, sz.width(), sz.height());
    }

	KisPainter over(layer->painterlyOverlay());
	over.bitBlt(rc.topLeft(), buffer->painterlyOverlay(), QRect(QPoint(0,0),sz));
	over.end();

	brush->setDefaultColor(brush->simpleColor());
}

void KisComplexOp::mixChannels(QVector<float> &mixed, const QVector<float> &val1, float vol1, const QVector<float> &val2, float vol2)
{
	Q_ASSERT(mixed.count() == val1.count() && val1.count() == val2.count());
	Q_ASSERT(vol1 >= 0 && vol2 >= 0);

	for (int i = 0; i < mixed.count(); i++)
		mixed[i] = ( vol1 * val1[i] + vol2 * val2[i] ) / ( vol1 + vol2 );
}

void KisComplexOp::mixProperty(PropertyCell *mixed, const PropertyCell *cell1, float vol1, const PropertyCell *cell2, float vol2)
{
	KoColorSpace *overlaycs = KisPainterlyOverlayColorSpace::instance();

	QVector<float> vCell1(overlaycs->channelCount());
	QVector<float> vCell2(overlaycs->channelCount());
	QVector<float> vMixed(overlaycs->channelCount());

	overlaycs->normalisedChannelsValue(reinterpret_cast<const quint8 *>(cell1), vCell1);
	overlaycs->normalisedChannelsValue(reinterpret_cast<const quint8 *>(cell2), vCell2);
	overlaycs->normalisedChannelsValue(reinterpret_cast<quint8 *>(mixed), vMixed);

	mixChannels(vMixed, vCell1, vol1, vCell2, vol2);

	overlaycs->fromNormalisedChannelsValue(reinterpret_cast<quint8 *>(mixed), vMixed);
}

void KisComplexOp::computePaintTransferAmount(PropertyCell *brush,
												PropertyCell *canvas,
												float pressure, float velocity,
												float &volume_bc, float &volume_cb)
{
	float ac = pressure * canvas->volume;
	float ab = pressure * brush->volume;
	float v = velocity;

	float amt;

	const float XFER_FRACTION = 0.1;
	const float MAX_XFER_QUANTITY = 1.0;
	const float EQUAL_PAINT_CUTOFF = 1.0/60.0;

	float paintDiff = ab - ac;
// 	float equalPaintCutoff = maths::clamp(0, 1, fabs(paintDiff)/EQUAL_PAINT_CUTOFF);
// 	float velocityCutoff = maths::smoothstep(0.2, 0.3, v);
	int xferDir = maths::sign(paintDiff);

	if (xferDir > 0)
		amt = ab;
	else
		amt = ac;

	amt = amt * XFER_FRACTION;
// 	amt = amt * equalPaintCutoff * velocityCutoff;
// 	amt = maths::clamp(0, MAX_XFER_QUANTITY, amt);

	if (xferDir > 0) {
		volume_bc = amt;
		volume_cb = 0;
	} else {
		volume_bc = 0;
		volume_cb = amt;
	}
}


KisPaintOp *KisComplexOpFactory::createOp(const KisPaintOpSettings */*settings*/, KisPainter * painter, KisImageSP image)
{
    KisPaintOp *op = new KisComplexOp(painter);
    Q_CHECK_PTR(op);
    return op;
}
