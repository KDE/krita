/*
 *  Copyright (c) 2007 Emanuele Tamponi <emanuele@valinor.it>
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

#ifndef KIS_KS_COLORSPACE_H_
#define KIS_KS_COLORSPACE_H_

#include "kis_illuminant_profile.h"
#include "kis_ks_colorspace_traits.h"

#include <KoColorModelStandardIds.h>
#include <KoColorSpaceConstants.h>
#include <KoColorSpaceMaths.h>
#include <KoIncompleteColorSpace.h>
#include "compositeops/KoCompositeOpOver.h"
#include "compositeops/KoCompositeOpErase.h"
#include "compositeops/KoCompositeOpMultiply.h"
#include "compositeops/KoCompositeOpDivide.h"
#include "compositeops/KoCompositeOpBurn.h"

template<int _N_>
class KisKSColorSpace : public KoIncompleteColorSpace< KisKSColorSpaceTrait<_N_> >
{
    typedef KoIncompleteColorSpace< KisKSColorSpaceTrait<_N_> > parent;

    public:

        KisKSColorSpace(KoColorProfile *p, const QString &id, const QString &name);
        virtual ~KisKSColorSpace();

    public:

        KoColorProfile *profile();
        const KoColorProfile *profile() const;
        bool profileIsCompatible(const KoColorProfile *profile) const;

        void colorToXML(const quint8*, QDomDocument&, QDomElement&) const {} // TODO
        void colorFromXML(quint8*, const QDomElement&) const {} // TODO

        KoID colorDepthId() const { return Float32BitsColorDepthID; }

        virtual KoID colorModelId() const = 0;
        virtual KoColorSpace* clone() const = 0;
        virtual bool willDegrade(ColorSpaceIndependence) const { return false; } // TODO pure virtual

    private:

        KisIlluminantProfile *m_profile;

};

////////////////////////////////////////////
//            IMPLEMENTATION              //
////////////////////////////////////////////

template<int _N_>
KisKSColorSpace<_N_>::KisKSColorSpace(KoColorProfile *p, const QString &id, const QString &name)
: parent(id, name, KoColorSpaceRegistry::instance()->rgb16(""))
{
    if (!profileIsCompatible(p))
        return;

    m_profile = dynamic_cast<KisIlluminantProfile *>(p);

    for (quint32 i = 0; i < 2*_N_; i+=2) {
        parent::addChannel(new KoChannelInfo(i18n("Absorption"),
                           i+0 * sizeof(float),
                           KoChannelInfo::COLOR,
                           KoChannelInfo::FLOAT32,
                           sizeof(float),
                           QColor(0,0,255)));

        parent::addChannel(new KoChannelInfo(i18n("Scattering"),
                           i+1 * sizeof(float),
                           KoChannelInfo::COLOR,
                           KoChannelInfo::FLOAT32,
                           sizeof(float),
                           QColor(255,0,0)));
    }

    parent::addChannel(new KoChannelInfo(i18n("Alpha"),
                       2 * _N_ * sizeof(float),
                       KoChannelInfo::ALPHA,
                       KoChannelInfo::FLOAT32,
                       sizeof(float)));

    addCompositeOp( new KoCompositeOpOver< KisKSColorSpaceTrait<_N_> >(this) );
    addCompositeOp( new KoCompositeOpErase< KisKSColorSpaceTrait<_N_> >(this) );
    addCompositeOp( new KoCompositeOpMultiply< KisKSColorSpaceTrait<_N_> >(this) );
    addCompositeOp( new KoCompositeOpDivide< KisKSColorSpaceTrait<_N_> >(this) );
    addCompositeOp( new KoCompositeOpBurn< KisKSColorSpaceTrait<_N_> >(this) );
}

template<int _N_>
KisKSColorSpace<_N_>::~KisKSColorSpace()
{
}

template<int _N_>
KoColorProfile *KisKSColorSpace<_N_>::profile()
{
    return m_profile;
}

template<int _N_>
const KoColorProfile *KisKSColorSpace<_N_>::profile() const
{
    return const_cast<const KisIlluminantProfile *>(m_profile);
}

template<int _N_>
bool KisKSColorSpace<_N_>::profileIsCompatible(const KoColorProfile *profile) const
{
    const KisIlluminantProfile *p = dynamic_cast<const KisIlluminantProfile *>(profile);
    if (!p)
        return false;
    if (p->wavelenghts() != _N_)
        return false;
    return true;
}

#endif // KIS_KS_COLORSPACE_H_
