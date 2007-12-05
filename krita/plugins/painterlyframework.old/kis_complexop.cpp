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
    KisPaintInformation adjustedInfo(info);
    float pressure = maths::sigmoid(info.pressure);

    adjustedInfo.pressure = pressure;

    if (!painter()->device())
        return;

    KisPaintDeviceSP canvas = painter()->device();
    KoColorSpace *cs = canvas->colorSpace();

    if (!painter()->brush())
        return;

    KisBrush *brush = painter()->brush();

    if (!canvas->painterlyOverlay())
        canvas->createPainterlyOverlay();

    KisPaintDeviceSP buffer = new KisPaintDevice(cs);
    buffer->createPainterlyOverlay();

    QPointF hotSpot = brush->hotSpot(adjustedInfo);
    QPointF pt = info.pos - hotSpot;

    qint32 x;
    double xFraction;
    qint32 y;
    double yFraction;

    splitCoordinate(pt.x(), &x, &xFraction);
    splitCoordinate(pt.y(), &y, &yFraction);

    KisPaintDeviceSP dab = KisPaintDeviceSP(0);

    if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        dab = brush->image(cs, adjustedInfo, xFraction, yFraction);
    } else {
        KisQImagemaskSP mask = brush->mask(adjustedInfo, xFraction, yFraction);
        dab = computeDab(mask);
        dab->convertTo(cs);
    }
    {
        KisPainter p(buffer);
        p.bitBlt(dab->exactBounds().topLeft(), dab, dab->exactBounds());
        p.end();
    }

    painter()->setPressure(pressure);

    QSize sz(brush->maskWidth(adjustedInfo),
             brush->maskHeight(adjustedInfo));

    QRect dstRect(x, y, sz.width(), sz.height());

    if ( painter()->bounds().isValid() ) {
        dstRect &= painter()->bounds();
    }

    if (dstRect.isNull() || dstRect.isEmpty() || !dstRect.isValid())
        return;

    if (!painter()->complexColor())
        return;

    KisComplexColor *complex = painter()->complexColor();
    complex->setSize(sz);

    qint32 sx = dstRect.x() - x;
    qint32 sy = dstRect.y() - y;
    qint32 sw = dstRect.width();
    qint32 sh = dstRect.height();

    int channels = cs->channelCount();

    KisHLineIteratorPixel bufferIt = buffer->createHLineIterator(0,0,sz.width());
    KisHLineIteratorPixel buOverIt = buffer->painterlyOverlay()->createHLineIterator(0,0,sz.width());
    KisHLineIteratorPixel canvasIt = canvas->createHLineIterator(x,y,sz.width());
    KisHLineIteratorPixel caOverIt = canvas->painterlyOverlay()->createHLineIterator(x,y,sz.width());

    QVector<float> bufferColor(channels), brushColor(channels), canvasColor(channels);
    PropertyCell *bufferCell, *brushCell, *canvasCell;

    if (complex->colorSpace()->id() != cs->id())
        complex->convertTo(cs);

    for (int y = complex->top(); y < complex->bottom(); y++) {
        for (int x = complex->left(); x < complex->right(); x++) {

            if (cs->alpha(bufferIt.rawData())) { // If alpha != 0

                cs->normalisedChannelsValue(bufferIt.rawData(), bufferColor);        // bufferColor
                bufferCell = reinterpret_cast<PropertyCell *>(buOverIt.rawData());    // bufferCell

                cs->normalisedChannelsValue(complex->rawData(x,y), brushColor);        // brushColor
                brushCell = complex->property(x,y);                                    // brushCell

                cs->normalisedChannelsValue(canvasIt.rawData(), canvasColor);        // canvasColor
                canvasCell = reinterpret_cast<PropertyCell *>(caOverIt.rawData());    // canvasCell

                float alpha = bufferColor[channels-1] >= canvasColor[channels-1] ?
                        bufferColor[channels-1] : canvasColor[channels-1];
/*
                float volume_bc, volume_cb;
                computePaintTransferAmount(brushCell, canvasCell, pressure, 0.6, volume_bc, volume_cb);

                if (volume_bc) { // Paint transfer is from complex to canvas
                    // The result of the mixing goes in the buffer.
                    mixChannels(bufferColor, canvasColor, canvasCell->volume, brushColor, volume_bc);
                    mixProperty(bufferCell, canvasCell, canvasCell->volume, brushCell, volume_bc);

                    bufferCell->volume += volume_bc;
                    brushCell->volume -= volume_bc;
                }
                if (volume_cb) {
                    // The result of the mixing goes in the complex; the buffer gets the color of the canvas!
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
//                 if (brushCell->volume < 0)
//                     brushCell->volume = 0;
                if (bufferCell->volume > 1000.0)
                    bufferCell->volume = 1000.0;
//                 if (brushCell->volume > 1000.0)
//                     brushCell->volume = 1000.0;

                cs->fromNormalisedChannelsValue(bufferIt.rawData(), bufferColor);
                cs->fromNormalisedChannelsValue(complex->rawData(x,y), bufferColor);
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

    if (source()->hasSelection()) {
        painter()->bltSelection(dstRect.x(), dstRect.y(), painter()->compositeOp(), buffer,
                                source()->selection(), painter()->opacity(), sx, sy, sw, sh);
    }
    else {
        painter()->bitBlt(dstRect.x(), dstRect.y(), painter()->compositeOp(), buffer, painter()->opacity(), sx, sy, sw, sh);
    }

    KisPainter over(canvas->painterlyOverlay());
    over.bitBlt(dstRect.topLeft(), buffer->painterlyOverlay(), QRect(sx, sy, sw, sh));
    over.end();

    complex->setDefaultColor(complex->simpleColor());
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

void KisComplexOp::computePaintTransferAmount(PropertyCell *complex,
                                                PropertyCell *canvas,
                                                float pressure, float velocity,
                                                float &volume_bc, float &volume_cb)
{
    float ac = pressure * canvas->volume;
    float ab = pressure * complex->volume;
    float v = velocity;

    float amt;

    const float XFER_FRACTION = 0.1;
    const float MAX_XFER_QUANTITY = 1.0;
    const float EQUAL_PAINT_CUTOFF = 1.0/60.0;

    float paintDiff = ab - ac;
    float equalPaintCutoff = maths::clamp(0, 1, fabs(paintDiff)/EQUAL_PAINT_CUTOFF);
    float velocityCutoff = maths::smoothstep(0.2, 0.3, v);
    int xferDir = maths::sign(paintDiff);

    if (xferDir > 0)
        amt = ab;
    else
        amt = ac;

    amt = amt * XFER_FRACTION;
    amt = amt * equalPaintCutoff * velocityCutoff;
    amt = maths::clamp(0, MAX_XFER_QUANTITY, amt);

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
    Q_UNUSED(image)
    KisPaintOp *op = new KisComplexOp(painter);
    Q_CHECK_PTR(op);
    return op;
}
