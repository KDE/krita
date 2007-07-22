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

#include "KoColorModelStandardIds.h"

class KisPainterlyOverlayColorSpaceFactory : public KoColorSpaceFactory
{
public:
     QString id() const { return "painterlyoverlay"; }
     QString name() const { return i18n("Painterly Overlay (32 bit float/channel)"); }

     virtual KoID colorModelId() const { return KoID("painterlyoverlay", i18n("Painterly Overlay") ); }
     virtual KoID colorDepthId() const { return Integer8BitsColorDepthID; }

     bool profileIsCompatible(KoColorProfile* /*profile*/) const
        {
            return false;
        }

     KoColorSpace *createColorSpace(KoColorSpaceRegistry * parent, KoColorProfile * p)
        {
            Q_UNUSED( p );
            return new KisPainterlyOverlayColorSpace("painterlyoverlay", "", parent);
        }

     QString defaultProfile() { return ""; }

};

KisPainterlyOverlayColorSpace * KisPainterlyOverlayColorSpace::KisPainterlyOverlayColorSpace::instance()
{
    KoColorSpaceRegistry * registry = KoColorSpaceRegistry::instance();
    KisPainterlyOverlayColorSpace * cs =
        dynamic_cast<KisPainterlyOverlayColorSpace*>( registry->colorSpace( "painterlyoverlay", 0 ) );

    if ( !cs ) {
        KisPainterlyOverlayColorSpaceFactory * f = new KisPainterlyOverlayColorSpaceFactory();
        registry->add( f );
        cs = dynamic_cast<KisPainterlyOverlayColorSpace*>( registry->colorSpace( "painterlyoverlay", 0 ) );
    }

    return cs;
}



KisPainterlyOverlayColorSpace::KisPainterlyOverlayColorSpace(const QString &id, const QString &name, KoColorSpaceRegistry * parent)
    : KoIncompleteColorSpace<PainterlyOverlayFloatTraits, KoRGB16Fallback>(id, name, parent)
{
    addChannel(new KoChannelInfo(i18n("Adsorbency"),
                                 PainterlyOverlayFloatTraits::adsorbency_pos * sizeof(float),
                                 KoChannelInfo::SUBSTRATE,
                                 KoChannelInfo::FLOAT32,
                                 sizeof(float),
                                 QColor(255,0,0)));

    addChannel(new KoChannelInfo(i18n("Gravity"),
                                 PainterlyOverlayFloatTraits::gravity_pos * sizeof(float),
                                 KoChannelInfo::SUBSTRATE,
                                 KoChannelInfo::FLOAT32,
                                 sizeof(float),
                                 QColor(255,0,0)));

    addChannel(new KoChannelInfo(i18n("Mixability"),
                                 PainterlyOverlayFloatTraits::mixability_pos * sizeof(float),
                                 KoChannelInfo::SUBSTANCE,
                                 KoChannelInfo::FLOAT32,
                                 sizeof(float),
                                 QColor(255,0,0)));

    addChannel(new KoChannelInfo(i18n("Height"),
                                 PainterlyOverlayFloatTraits::height_pos * sizeof(float),
                                 KoChannelInfo::SUBSTRATE,
                                 KoChannelInfo::FLOAT32,
                                 sizeof(float),
                                 QColor(255,0,0)));

    addChannel(new KoChannelInfo(i18n("Pigment Concentration"),
                                 PainterlyOverlayFloatTraits::pigment_concentration_pos * sizeof(float),
                                 KoChannelInfo::SUBSTANCE,
                                 KoChannelInfo::FLOAT32,
                                 sizeof(float),
                                 QColor(255,0,0)));

    addChannel(new KoChannelInfo(i18n("Viscosity"),
                                 PainterlyOverlayFloatTraits::viscosity_pos * sizeof(float),
                                 KoChannelInfo::SUBSTANCE,
                                 KoChannelInfo::FLOAT32,
                                 sizeof(float),
                                 QColor(255,0,0)));

    addChannel(new KoChannelInfo(i18n("Volume"),
                                 PainterlyOverlayFloatTraits::volume_pos * sizeof(float),
                                 KoChannelInfo::SUBSTANCE,
                                 KoChannelInfo::FLOAT32,
                                 sizeof(float),
                                 QColor(255,0,0)));

    addChannel(new KoChannelInfo(i18n("Wetness"),
                                 PainterlyOverlayFloatTraits::wetness_pos * sizeof(float),
                                 KoChannelInfo::SUBSTANCE,
                                 KoChannelInfo::FLOAT32,
                                 sizeof(quint16),
                                 QColor(255,0,0)));

}

