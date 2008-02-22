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

#include "kis_illuminant_profile_lc.h"

#include <QString>

#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

KisIlluminantProfileLC::KisIlluminantProfileLC(const QString &fileName)
    : parent(fileName, "LC"), m_red(0), m_green(0), m_blue(0)
{

}

KisIlluminantProfileLC::~KisIlluminantProfileLC()
{
    reset();
}

bool KisIlluminantProfileLC::load()
{
    reset();

    bool success;

    if (success = parent::load()) {
        m_red = gsl_vector_calloc(wavelengths());
        m_green = gsl_vector_calloc(wavelengths());
        m_blue = gsl_vector_calloc(wavelengths());

        gsl_vector *tmp = gsl_vector_calloc(3);

        gsl_vector_set(tmp, 0, 1); // RED
        m_rgbvec = tmp;
        parent::rgbToReflectance();
        gsl_vector_memcpy(m_red, m_refvec);
        gsl_vector_set(tmp, 0, 0);

        gsl_vector_set(tmp, 2, 1); // BLUE
        m_rgbvec = tmp;
        // Here, we set the interval of reflectance to be between 0 and (1 - red)
        // Why? Because we want to obtain green as 1 - red - blue
        // so that the linear combination works: white is then red + green + white = 1
        for (int i = 0; i < wavelengths(); i++)
            gsl_vector_set(m_data->d, 2*i+1, -(1.0-gsl_vector_get(m_red,i)));
        parent::rgbToReflectance();
        gsl_vector_memcpy(m_blue, m_refvec);
        gsl_vector_set(tmp, 0, 0);

        for (int i = 0; i < wavelengths(); i++)
            gsl_vector_set(m_green, i, 1.0 - gsl_vector_get(m_red, i) - gsl_vector_get(m_blue, i));

        gsl_vector_free(tmp);
    }

    return success;
}

void KisIlluminantProfileLC::rgbToReflectance() const
{
    // Each reflectance is a linear combination of three base colors.
    for (int i = 0; i < wavelengths(); i++) {
        gsl_vector_set(m_refvec, i, gsl_vector_get(m_rgbvec, 0) * gsl_vector_get(m_red, i) +
                                    gsl_vector_get(m_rgbvec, 1) * gsl_vector_get(m_green, i) +
                                    gsl_vector_get(m_rgbvec, 2) * gsl_vector_get(m_blue, i));
    }
}

void KisIlluminantProfileLC::reset()
{
    if (m_red)
        gsl_vector_free(m_red);
    if (m_green)
        gsl_vector_free(m_green);
    if (m_blue)
        gsl_vector_free(m_blue);
}
