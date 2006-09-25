/*
 *  Copyright (c) 2005 Boudewijn Rempt <boud@valdyas.org>
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

#include <QRect>
#include <QCheckBox>

#include "KoIntegerMaths.h"

#include <kis_brush.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <kis_types.h>
#include <kis_paintop.h>
#include <kis_iterators_pixel.h>
#include <kis_layer.h>
#include <kis_meta_registry.h>
#include <KoColorSpaceRegistry.h>
#include "kis_input_device.h"

#include "kis_wetop.h"
#include "kis_wet_colorspace.h"

KisWetOpSettings::KisWetOpSettings(QWidget *parent)
    : super(parent)
{
    m_options = new WetPaintOptions(parent, "wet option widget");
}

bool KisWetOpSettings::varySize() const
{
    return m_options->checkSize->isChecked();
}

bool KisWetOpSettings::varyWetness() const
{
    return m_options->checkWetness->isChecked();
}

bool KisWetOpSettings::varyStrength() const
{
    return m_options->checkStrength->isChecked();
}

KisPaintOp * KisWetOpFactory::createOp(const KisPaintOpSettings *settings, KisPainter * painter)
{
    const KisWetOpSettings *wetopSettings = dynamic_cast<const KisWetOpSettings *>(settings);
    Q_ASSERT(settings == 0 || wetopSettings != 0);

    KisPaintOp * op = new KisWetOp(wetopSettings, painter);
    Q_CHECK_PTR(op);
    return op;
}

KisPaintOpSettings* KisWetOpFactory::settings(QWidget * parent, const KisInputDevice& inputDevice)
{
    if (inputDevice == KisInputDevice::mouse()) {
        // No options for mouse, only tablet devices
        return 0;
    } else {
        return new KisWetOpSettings(parent);
    }
}

KisWetOp::KisWetOp(const KisWetOpSettings * settings, KisPainter * painter)
    : super(painter)
{
    if (settings) {
        m_size = settings->varySize();
        m_wetness = settings->varyWetness();
        m_strength = settings->varyStrength();
    } else {
        m_size = false;
        m_wetness = false;
        m_strength = false;
    }
}

KisWetOp::~KisWetOp()
{
}

void KisWetOp::paintAt(const KisPoint &pos, const KisPaintInformation& info)
{
    if (!m_painter) return;

    if (!m_painter->device()) return;
    KisPaintDeviceSP device = m_painter->device();

    if (!m_painter->device()) return;

    KisBrush *brush = m_painter->brush();
    Q_ASSERT(brush);

    if (! brush->canPaintFor(info) )
        return;

    KisPaintInformation inf(info);

    if (!m_size)
        inf.pressure = PRESSURE_DEFAULT;

    KisPaintDeviceSP dab = KisPaintDeviceSP(0);

    if (brush->brushType() == IMAGE || brush->brushType() == PIPE_IMAGE) {
        dab = brush->image(KisMetaRegistry::instance()->csRegistry()->alpha8(), inf);
    }
    else {
        KisAlphaMaskSP mask = brush->mask(inf);
        dab = computeDab(mask, KisMetaRegistry::instance()->csRegistry()->alpha8());
    }

    KoColorSpace * cs = device->colorSpace();

    if (cs->id() != KoID("WET","")) {
        return;
    }

    KoColor paintColor = m_painter->paintColor();
    paintColor.convertTo(cs);
    // hopefully this does
    // nothing, conversions are bad ( wet->rgb->wet gives horrible mismatches, due to
    // the conversion to rgb actually rendering the paint above white

    WetPack* paintPack = reinterpret_cast<WetPack*>(paintColor.data());
    WetPix paint = paintPack->paint;

    // Get the paint info (we store the strength in the otherwise unused (?) height field of
    // the paint
    // double wetness = paint.w; // XXX: Was unused
    // strength is a double in the 0 - 2 range, but upscaled to quint16:

    double strength = 2.0 * static_cast<double>(paint.h) / (double)(0xffff);

    kDebug() << "Before strength: " << strength << endl;

    if (m_strength)
        strength = strength * (strength + info.pressure) * 0.5;
    else
        strength = strength * (strength + PRESSURE_DEFAULT) * 0.5;

    double pressure = 0.75 + 0.25 * info.pressure;

    kDebug() << "info.pressure " << info.pressure << ", local pressure: " << pressure << ", strength: " << strength << endl;

    WetPack currentPack;
    WetPix currentPix;
    double eff_height;
    double press, contact;

    int maskW = brush->maskWidth(inf);
    int maskH = brush->maskHeight(inf);
    KoPoint dest = (pos - (brush->hotSpot(inf)));
    int xStart = (int)dest.x();
    int yStart = (int)dest.y();

    for (int y = 0; y < maskH; y++) {
        KisHLineIteratorPixel dabIt = dab->createHLineIterator(0, y, maskW, false);
        KisHLineIteratorPixel it = device->createHLineIterator(xStart, yStart+y, maskW, true);

        while (!dabIt.isDone()) {
            // This only does something with .paint, and not with adsorb.
            currentPack = *(reinterpret_cast<WetPack*>(it.rawData()));
            WetPix currentData = currentPack.adsorb;
            currentPix = currentPack.paint;

            // Hardcoded threshold for the dab 'strength': above it, it will get painted
            if (*dabIt.rawData() > 125)
                press = pressure * 0.25;
            else
                press = -1;
            //kDebug() << "After mysterious line, press becomes: " << press << ", this is the same as in the original. Good" << endl;
            // XXX - 192 is probably only useful for paper with a texture...
            eff_height = (currentData.h + currentData.w - 192.0) * (1.0 / 255.0);
            contact = (press + eff_height) * 0.2;
            //double old_contact = contact;
            if (contact > 0.5)
                contact = 1.0 - 0.5 * exp(-2.0 * contact - 1.0);

            //kDebug() << "Contact was " << old_contact << " and has become: " << contact << endl;
            if (contact > 0.0001) {
                int v;
                double rnd = rand() * (1.0 / RAND_MAX);

                v = currentPix.rd;
                currentPix.rd = (quint16)floor(v + (paint.rd * strength - v) * contact + rnd);
                //kDebug() << "Rd was " << v << " and has become " << currentPix.rd << endl;
                v = currentPix.rw;
                currentPix.rw = (quint16)floor(v + (paint.rw * strength - v) * contact + rnd);
                v = currentPix.gd;
                currentPix.gd = (quint16)floor(v + (paint.gd * strength - v) * contact + rnd);
                v = currentPix.gw;
                currentPix.gw = (quint16)floor(v + (paint.gw * strength - v) * contact + rnd);
                v = currentPix.bd;
                currentPix.bd = (quint16)floor(v + (paint.bd * strength - v) * contact + rnd);
                v = currentPix.bw;
                currentPix.bw = (quint16)floor(v + (paint.bw * strength - v) * contact + rnd);
                v = currentPix.w;
                if (m_wetness)
                    currentPix.w = (quint16)(CLAMP(floor(
                          v + (paint.w * (0.5 + pressure) - v) * contact + rnd), 0, 512));
                else
                    currentPix.w = (quint16)floor(v + (paint.w - v) * contact + rnd);

                currentPack.paint = currentPix;
                *(reinterpret_cast<WetPack*>(it.rawData())) = currentPack;
            }
            ++dabIt;
            ++it;
        }
    }

    m_painter->addDirtyRect(QRect(xStart, yStart, maskW, maskH));
}

#include "kis_wetop.moc"

