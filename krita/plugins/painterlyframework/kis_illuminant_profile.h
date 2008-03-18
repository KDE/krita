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

class KisIlluminantProfile : public KoColorProfile {

    public:
        KisIlluminantProfile(const QString &fileName = "");
        ~KisIlluminantProfile();

        // Repeat here from KoColorProfile, to remember it in subclasses
        KoColorProfile *clone() const
        {
            KoColorProfile *p = new KisIlluminantProfile(fileName());
            if (valid())
                p->load();
            return p;
        }

        ///////////////////////////////////////////////////
        // KoColorProfile interface

        bool load();
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
        int wavelengths() const { return m_wl; }

        // UTILITY
        void fromRgb(const double *rgbvec, double *ksvec) const;
        void toRgb(const double *ksvec, double *rgbvec) const;

    private:
        double falpha(double R, double L) const;
        double fsigma(double R, double L) const;

        void rgbToReflectance(const double *rgbvec) const;
        void reflectanceToRgb(double *rgbvec) const;

        void reflectanceToKS(double *ksvec) const;
        void KSToReflectance(const double *ksvec) const;

        void reset();

    private:

        int m_wl;
        double **m_T;
        double **m_L;
        double *m_red;
        double *m_green;
        double *m_blue;

        double *m_refvec;

        int Np;
        double Cla, Nla, *Ca;
        double Cls, Nls, *Cs;
        double Rh;

        QString m_illuminant;
        bool m_valid;

};

#endif // KIS_ILLUMINANT_PROFILE_H_
