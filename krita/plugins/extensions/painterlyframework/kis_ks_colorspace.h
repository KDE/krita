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

#include <QDomElement>
#include <kdebug.h>

template< typename _TYPE_, int _N_ >
class KisKSColorSpace : public KoIncompleteColorSpace< KisKSColorSpaceTrait<_TYPE_, _N_> >
{
    typedef KisKSColorSpaceTrait<_TYPE_, _N_> CSTrait;
    typedef KoIncompleteColorSpace< CSTrait > parent;

public:

    KisKSColorSpace(KoColorProfile *p);
    virtual ~KisKSColorSpace();

public:

    KoColorProfile *profile();
    const KoColorProfile *profile() const;
    bool profileIsCompatible(const KoColorProfile *profile) const;

    void colorToXML(const quint8*, QDomDocument&, QDomElement&) const;
    void colorFromXML(quint8*, const QDomElement&) const;

    virtual KoColorSpace* clone() const = 0;

    bool willDegrade(ColorSpaceIndependence) const {
        return false;
    }

    KoID colorModelId() const {
        return ColorModelId();
    }
    KoID colorDepthId() const {
        return ColorDepthId();
    }

public: // static

    static KoID ColorModelId() {
        return KoID(QString("KS%1").arg(_N_),
                    i18n("Painterly Color Space, %1 wavelengths", _N_));
    }

    static KoID ColorDepthId() {
        QString id;
        QString name;

        switch (KoColorSpaceMathsTraits<_TYPE_>::channelValueType) {
        case KoChannelInfo::FLOAT32:
            id = QString("F32");
            name = i18n("32 Bits Float");
            break;
        case KoChannelInfo::FLOAT16:
            id = QString("F16");
            name = i18n("16 Bits Float");
            break;
        default: break;
        }

        return KoID(id, name);
    }

    static KoID ColorSpaceId() {
        return KoID(ColorModelId().id()      +     ColorDepthId().id(),
                    ColorModelId().name() + " (" + ColorDepthId().name() + ")");
    }

private:

    KisIlluminantProfile *m_profile;

};

////////////////////////////////////////////
//            IMPLEMENTATION              //
////////////////////////////////////////////

template< typename _TYPE_, int _N_ >
KisKSColorSpace<_TYPE_, _N_>::KisKSColorSpace(KoColorProfile *p)
        : parent(ColorSpaceId().id(),
                 ColorSpaceId().name(),
                 KoColorSpaceRegistry::instance()->rgb16(""))
{
    Q_ASSERT(p);
    m_profile = static_cast<KisIlluminantProfile *>(p);

    const KoChannelInfo::enumChannelValueType channelValueType = KoColorSpaceMathsTraits<_TYPE_>::channelValueType;
    const int channelSize = sizeof(_TYPE_);

    for (int i = 0; i < 2*_N_; i += 2) {
        parent::addChannel(new KoChannelInfo(i18n("Absorption"),
                                             (i + 0) * channelSize,
                                             KoChannelInfo::COLOR,
                                             channelValueType,
                                             channelSize,
                                             QColor(0, 0, 255)));

        parent::addChannel(new KoChannelInfo(i18n("Scattering"),
                                             (i + 1) * channelSize,
                                             KoChannelInfo::COLOR,
                                             channelValueType,
                                             channelSize,
                                             QColor(255, 0, 0)));
    }

    parent::addChannel(new KoChannelInfo(i18n("Alpha"),
                                         2*_N_ * channelSize,
                                         KoChannelInfo::ALPHA,
                                         channelValueType,
                                         channelSize,
                                         QColor(0, 255, 0)));

    addCompositeOp(new KoCompositeOpOver< CSTrait >(this));
    addCompositeOp(new KoCompositeOpErase< CSTrait >(this));
    addCompositeOp(new KoCompositeOpMultiply< CSTrait >(this));
    addCompositeOp(new KoCompositeOpDivide< CSTrait >(this));
    addCompositeOp(new KoCompositeOpBurn< CSTrait >(this));
}

template< typename _TYPE_, int _N_ >
KisKSColorSpace<_TYPE_, _N_>::~KisKSColorSpace()
{
    delete m_profile;
}

template< typename _TYPE_, int _N_ >
KoColorProfile *KisKSColorSpace<_TYPE_, _N_>::profile()
{
    return m_profile;
}

template< typename _TYPE_, int _N_ >
const KoColorProfile *KisKSColorSpace<_TYPE_, _N_>::profile() const
{
    return m_profile;
}

template< typename _TYPE_, int _N_ >
bool KisKSColorSpace<_TYPE_, _N_>::profileIsCompatible(const KoColorProfile *profile) const
{
    const KisIlluminantProfile *p = static_cast<const KisIlluminantProfile *>(profile);
    if (p->wavelengths() != _N_) {
        return false;
    }
    return true;
}

template< typename _TYPE_, int _N_ >
void KisKSColorSpace<_TYPE_, _N_>::colorToXML(const quint8 *pixel, QDomDocument &doc, QDomElement &colorElt) const
{
    QDomElement labElt = doc.createElement(QString("KS%1").arg(_N_));
    for (int i = 0; i < _N_; i++) {
        labElt.setAttribute(QString("K%1").arg(i), KoColorSpaceMaths<_TYPE_, double>::scaleToA(CSTrait::K(pixel, i)));
        labElt.setAttribute(QString("S%1").arg(i), KoColorSpaceMaths<_TYPE_, double>::scaleToA(CSTrait::S(pixel, i)));
    }
    labElt.setAttribute("space", profile()->name());
    colorElt.appendChild(labElt);
}

template< typename _TYPE_, int _N_ >
void KisKSColorSpace<_TYPE_, _N_>::colorFromXML(quint8 *pixel, const QDomElement &elt) const
{
    for (int i = 0; i < _N_; i++) {
        CSTrait::K(pixel, i) = KoColorSpaceMaths<double, _TYPE_>::scaleToA(elt.attribute(QString("K%1").arg(i)).toDouble());
        CSTrait::S(pixel, i) = KoColorSpaceMaths<double, _TYPE_>::scaleToA(elt.attribute(QString("S%1").arg(i)).toDouble());
    }
}


template< typename _TYPE_, int _N_ >
class KisKSColorSpaceFactory : public KoColorSpaceFactory
{
public:
    QString id() const {
        return KisKSColorSpace<_TYPE_, _N_>::ColorSpaceId().id();
    }
    QString name() const {
        return KisKSColorSpace<_TYPE_, _N_>::ColorSpaceId().name();
    }
    KoID colorModelId() const {
        return KisKSColorSpace<_TYPE_, _N_>::ColorModelId();
    }
    KoID colorDepthId() const {
        return KisKSColorSpace<_TYPE_, _N_>::ColorDepthId();
    }
    bool userVisible() const {
        return _N_ >= 3;
    }

    int referenceDepth() const {
        return sizeof(_TYPE_)*8;
    }
    QString colorSpaceEngine() const {
        return QString("ks%1").arg(_N_);
    }
    bool isHdr() const {
        return false;
    }

    bool profileIsCompatible(const KoColorProfile *profile) const {
        const KisIlluminantProfile *p = dynamic_cast<const KisIlluminantProfile *>(profile);
        if ((!p) || (p->wavelengths() != _N_)) {
            return false;
        }

        return true;
    }

    QString defaultProfile() const {
        return QString("");
    }
    KoColorProfile* createColorProfile(const QByteArray& rawData) const {
        return 0;
    }
};

#endif // KIS_KS_COLORSPACE_H_
