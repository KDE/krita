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

#ifndef KIS_ILLUMINANT_PROFILE_H_
#define KIS_ILLUMINANT_PROFILE_H_

#include <KoColorProfile.h>
#include <QString>

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

#include "channel_converter.h"

class KisIlluminantProfile : public KoColorProfile {

    public:
        // REMEMBER TO CALL setName in subclasses!
        KisIlluminantProfile(const QString &fileName = "");
        virtual ~KisIlluminantProfile();

        // Repeat here from KoColorProfile, to remember it in subclasses
        virtual KoColorProfile *clone() const = 0;

        ///////////////////////////////////////////////////
        // KoColorProfile interface

        virtual bool load();
        bool save(const QString &fileName);

        bool valid() const { return m_valid; }
        bool isSuitableForOutput() const   { return true; }
        bool isSuitableForPrinting() const { return true; }
        bool isSuitableForDisplay() const  { return true; }
        bool operator==(const KoColorProfile &op2) const
            { return (name() == op2.name()); }

        ///////////////////////////////////////////////////

        // INTROSPECTION
        QString illuminant() const { return m_illuminant; }
        int wavelengths() const { return m_P->size; }

        // UTILITY
        void fromRgb(gsl_vector *rgbvec, gsl_vector *ksvec) const;
        void toRgb(gsl_vector *ksvec, gsl_vector *rgbvec) const;

    protected:
        virtual void rgbToReflectance() const = 0;

        virtual void reflectanceToRgb() const;
        virtual void reflectanceToKS() const;
        virtual void KSToReflectance() const;

    protected:

        gsl_matrix *m_T;
        gsl_vector *m_P;
        double Kb, Sb;

        // Temporary
        mutable gsl_vector *m_rgbvec;
        mutable gsl_vector *m_refvec;
        mutable gsl_vector *m_ksvec;

        ChannelConverter<double> *m_converter;

    private:
        void deleteAll();

    private:

        QString m_illuminant;
        bool m_valid;

};

#endif // KIS_ILLUMINANT_PROFILE_H_
