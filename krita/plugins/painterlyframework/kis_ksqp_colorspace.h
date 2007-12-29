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

#ifndef KIS_KSQP_COLORSPACE_H_
#define KIS_KSQP_COLORSPACE_H_

#include "kis_ks_colorspace.h"

#define KS9QPID KoID("KS9QP", i18n("%1-pairs Absorption-Scattering QP"))

class KisKSQPColorSpace : public KisKSColorSpace<9>
{
    typedef KisKSColorSpace<9> parent;

    public:

        KisKSQPColorSpace(KoColorProfile *p)
        : parent(p, colorSpaceId(),
                 i18n("9-pairs Absorption-Scattering QP (32 Bits Float)")) {}
        ~KisKSQPColorSpace() {}

        KoID colorModelId() const
        {
            return KS9QPID;
        }

        KoColorSpace *clone() const
        {
            return new KisKSQPColorSpace(profile()->clone());
        }

        static QString colorSpaceId()
        {
            return "KS9QPF32";
        }

};

class KisKSQPColorSpaceFactory : public KoColorSpaceFactory
{
    public:
        QString id() const { return KisKSQPColorSpace::colorSpaceId(); }
        QString name() const { return i18n("9-pairs Absorption-Scattering QP (32 Bits Float)"); }
        KoID colorModelId() const { return KS9QPID; }
        KoID colorDepthId() const { return Float32BitsColorDepthID; }
        bool userVisible() const { return true; }

        int referenceDepth() const { return 32; }
        bool isIcc() const { return false; }
        bool isHdr() const { return false; }

        QList<KoColorConversionTransformationFactory*> colorConversionLinks() const;

        KoColorConversionTransformationFactory *createICCColorConversionTransformationFactory(QString _colorModelId, QString _colorDepthId) const
        {
            Q_UNUSED(_colorModelId);
            Q_UNUSED(_colorDepthId);
            return 0;
        }

        KoColorSpace *createColorSpace(const KoColorProfile *p) const {
            return new KisKSQPColorSpace(p->clone());
        }

        bool profileIsCompatible(const KoColorProfile *profile) const
        {
            return KisKSQPColorSpace(0).profileIsCompatible(profile);
        }

        QString defaultProfile() const { return ""; } // TODO
};

#endif // KIS_KSQP_COLORSPACE_H_
