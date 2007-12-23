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

class KisIlluminantProfile : public KoColorProfile {

    public:
        KisIlluminantProfile(const QString &fileName = "");
        KisIlluminantProfile(const KisIlluminantProfile &profile);
        ~KisIlluminantProfile();

        // KoColorProfile interface
        KoColorProfile *clone() const;
        bool load();
        bool save(const QString &fileName);
        bool valid() const { return m_valid; }
        bool isSuitableForOutput() const { return true; }
        bool isSuitableForPrinting() const { return true; }
        bool isSuitableForDisplay() const { return true; }
        bool operator==(const KoColorProfile &op2) const
            { return (name()+info() == op2.name()+op2.info()); }

        gsl_matrix_float *T() const { return m_T; }
        gsl_vector_float *P() const { return m_P; }
        int wavelenghts() const { return m_P->size; }
        double Swhite() { return S_w; }
        double Kblack() { return K_b; }

    private:
        gsl_matrix_float *m_T;
        gsl_vector_float *m_P;
        bool m_valid;
        float S_w, K_b;

};

#endif // KIS_ILLUMINANT_PROFILE_H_
