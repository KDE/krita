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

class PIGMENTCMS_EXPORT KoHdrColorProfile : public KoColorProfile {
    public:
        KoHdrColorProfile();
        KoHdrColorProfile(const KoHdrColorProfile&);
        virtual ~KoHdrColorProfile();

        const KoIccColorProfile* iccProfile() const;
        void setIccColorProfile(KoIccColorProfile* profile);
    
        virtual KoColorProfile* clone() const;
        virtual bool valid() const;
        virtual bool isSuitableForOutput() const;
        virtual bool isSuitableForPrinting() const;
        virtual bool isSuitableForDisplay() const;
    private:
        struct Private;
        Private* const d;
};


#endif
