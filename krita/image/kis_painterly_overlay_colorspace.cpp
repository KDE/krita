/*
 *  Copyright (c) 2007 Boudewijn Rempt <boud@valdyas.org>
 *
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

#include "kis_painterly_overlay_colorspace.h"

#include "KoColorSpaceRegistry.h"

#include <KoColorModelStandardIds.h>
#include <KoColorConversionTransformationFactory.h>
#include <compositeops/KoCompositeOpOver.h>
#include <compositeops/KoCompositeOpErase.h>

struct KisPainterlyOverlayColorSpace::Private {
    static KisPainterlyOverlayColorSpace* s_instance;
};

KisPainterlyOverlayColorSpace* KisPainterlyOverlayColorSpace::Private::s_instance  = 0;

const KoID painterlyOverlayColorModelID("painterlyoverlay", i18n("Painterly Overlay"));

KoColorSpace* KisPainterlyOverlayColorSpace::clone() const
{
    return new KisPainterlyOverlayColorSpace("painterlyoverlay", "");
}

const KisPainterlyOverlayColorSpace * KisPainterlyOverlayColorSpace::instance()
{
    if (!Private::s_instance) {
        Private::s_instance = new KisPainterlyOverlayColorSpace(painterlyOverlayColorModelID.id(), painterlyOverlayColorModelID.name());
    }
    return Private::s_instance;
}

KisPainterlyOverlayColorSpace::KisPainterlyOverlayColorSpace(const QString &id, const QString &name)
        : KoIncompleteColorSpace<PainterlyOverlayFloatTraits>(id, name, KoColorSpaceRegistry::instance()->rgb16("")), d(new Private)
{
    addChannel(new KoChannelInfo(i18n("Adsorbency"),
                                 PainterlyOverlayFloatTraits::adsorbency_pos * sizeof(float),
                                 KoChannelInfo::SUBSTRATE,
                                 KoChannelInfo::FLOAT32,
                                 sizeof(float),
                                 QColor(255, 0, 0)));

    addChannel(new KoChannelInfo(i18n("Gravity"),
                                 PainterlyOverlayFloatTraits::gravity_pos * sizeof(float),
                                 KoChannelInfo::SUBSTRATE,
                                 KoChannelInfo::FLOAT32,
                                 sizeof(float),
                                 QColor(255, 0, 0)));

    addChannel(new KoChannelInfo(i18n("Mixability"),
                                 PainterlyOverlayFloatTraits::mixability_pos * sizeof(float),
                                 KoChannelInfo::SUBSTANCE,
                                 KoChannelInfo::FLOAT32,
                                 sizeof(float),
                                 QColor(255, 0, 0)));

    addChannel(new KoChannelInfo(i18n("Height"),
                                 PainterlyOverlayFloatTraits::height_pos * sizeof(float),
                                 KoChannelInfo::SUBSTRATE,
                                 KoChannelInfo::FLOAT32,
                                 sizeof(float),
                                 QColor(255, 0, 0)));

    addChannel(new KoChannelInfo(i18n("Pigment Concentration"),
                                 PainterlyOverlayFloatTraits::pigment_concentration_pos * sizeof(float),
                                 KoChannelInfo::SUBSTANCE,
                                 KoChannelInfo::FLOAT32,
                                 sizeof(float),
                                 QColor(255, 0, 0)));

    addChannel(new KoChannelInfo(i18n("Viscosity"),
                                 PainterlyOverlayFloatTraits::viscosity_pos * sizeof(float),
                                 KoChannelInfo::SUBSTANCE,
                                 KoChannelInfo::FLOAT32,
                                 sizeof(float),
                                 QColor(255, 0, 0)));

    addChannel(new KoChannelInfo(i18n("Volume"),
                                 PainterlyOverlayFloatTraits::volume_pos * sizeof(float),
                                 KoChannelInfo::SUBSTANCE,
                                 KoChannelInfo::FLOAT32,
                                 sizeof(float),
                                 QColor(255, 0, 0)));

    addChannel(new KoChannelInfo(i18n("Wetness"),
                                 PainterlyOverlayFloatTraits::wetness_pos * sizeof(float),
                                 KoChannelInfo::SUBSTANCE,
                                 KoChannelInfo::FLOAT32,
                                 sizeof(float),
                                 QColor(255, 0, 0)));

    addCompositeOp(new KoCompositeOpOver<PainterlyOverlayFloatTraits>(this));
    addCompositeOp(new KoCompositeOpErase<PainterlyOverlayFloatTraits>(this));
}

KisPainterlyOverlayColorSpace::~KisPainterlyOverlayColorSpace()
{
    delete d;
}

KoID KisPainterlyOverlayColorSpace::colorModelId() const
{
    return painterlyOverlayColorModelID;
}
KoID KisPainterlyOverlayColorSpace::colorDepthId() const
{
    return Float32BitsColorDepthID;
}
