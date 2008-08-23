/*
 *  Copyright (c) 2007 Cyrille Berger <cberger@cberger.net>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
*/

#ifndef _KO_HDR_COLOR_PROFILE_H_
#define _KO_HDR_COLOR_PROFILE_H_

#include "KoColorProfile.h"

class KoIccColorProfile;

/**
 * This object contains the color profile associated to an HDR color
 * space : the exposure settings used for LDR to HDR conversion and
 * an eventual ICC color profile for color correction of the LDR values.
 */
class PIGMENTCMS_EXPORT KoHdrColorProfile : public KoColorProfile {
    public:
        /**
         * Create an HDR profile with a null exposure and no ICC profile.
         */
        KoHdrColorProfile(const QString &name, const QString &info);
        KoHdrColorProfile(const KoHdrColorProfile&);
        virtual ~KoHdrColorProfile();

        /**
         * @return the current ICC Profile (it can be null)
         */
        const KoIccColorProfile* iccProfile() const;
        /**
         * Set the ICC Profile for this color profile. This
         * profile will take ownership over the ICC profile, so
         * clone it before.
         */
        void setIccColorProfile(KoIccColorProfile* profile);

        virtual KoColorProfile* clone() const;
        virtual bool valid() const;
        virtual bool isSuitableForOutput() const;
        virtual bool isSuitableForPrinting() const;
        virtual bool isSuitableForDisplay() const;
        virtual QVariant property( const QString& _name) const;
        virtual void setProperty( const QString& _name, const QVariant& _variant);

        /**
         * @return the current exposure
         */
        qreal hdrExposure() const;
        /**
         * Set the exposure for this profile. The exposure is a settings
         * that allows to simulate the exposure time of a silver film
         * camera.
         */
        void setHdrExposure(qreal exposure);
        virtual bool operator==(const KoColorProfile&) const;
    public:
        quint16 channelToDisplay(qreal value) const;
        qreal displayToChannel(quint16 value) const;

        qreal channelToDisplayDouble(qreal value) const;
        qreal displayToChannelDouble(qreal value) const;
    private:
        struct Private;
        Private* const d;
};

#endif
