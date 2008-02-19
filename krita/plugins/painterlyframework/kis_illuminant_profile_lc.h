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

#ifndef KIS_ILLUMINANT_PROFILE_LC_H_
#define KIS_ILLUMINANT_PROFILE_LC_H_

#include "kis_illuminant_profile_qp.h"

#include <QString>

extern "C" {
    #include "cqp/gsl_cqp.h"
}
#include <gsl/gsl_vector.h>

class KisIlluminantProfileLC : public KisIlluminantProfileQP {
    typedef KisIlluminantProfileQP parent;

    public:
        KisIlluminantProfileLC(const QString &fileName = "");
        ~KisIlluminantProfileLC();

        KoColorProfile *clone() const
        {
            KoColorProfile *p = new KisIlluminantProfileLC(fileName());
            if (valid())
                p->load();
            return p;
        }

        bool load();

    protected:
        void rgbToReflectance() const;

    protected:

        gsl_vector *m_red;
        gsl_vector *m_green;
        gsl_vector *m_blue;

    private:
        void reset();

};

#endif // KIS_ILLUMINANT_PROFILE_LC_H_
