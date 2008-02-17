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

#include "kis_illuminant_profile.h"

#include <KoColorProfile.h>

#include <QDataStream>
#include <QFile>
#include <QString>

#include <gsl/gsl_blas.h>
#include <gsl/gsl_cblas.h>
#include <gsl/gsl_linalg.h>
#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

KisIlluminantProfile::KisIlluminantProfile(const QString &fileName)
    : KoColorProfile(fileName), m_T(0), m_P(0), m_rgbvec(0), m_refvec(0), m_ksvec(0), m_converter(0)
{

}

KisIlluminantProfile::~KisIlluminantProfile()
{
    deleteAll();
}

bool KisIlluminantProfile::load()
{
    m_valid = false;

    if (fileName().isEmpty())
        return false;

    QFile file(fileName());
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QDataStream data(&file);

    deleteAll();

    {
        // Illuminant name
        QString illuminant;
        data >> illuminant;
        m_illuminant = illuminant;
    }
    {
        // T matrix
        int m, n;
        double c;
        data >> m >> n;
        m_T = gsl_matrix_alloc(m, n);
        for (int i = 0; i < m; i++) {
            for (int j = 0; j < n; j++) {
                data >> c;
                gsl_matrix_set(m_T, i, j, c);
            }
        }
    }
    {
        // P vector
        int l;
        int c;
        data >> l;
        if ((size_t)l != m_T->size2) {
            gsl_matrix_free(m_T); m_T = 0;
            return false;
        }
        m_P = gsl_vector_alloc(l);
        for (int i = 0; i < l; i++) {
            data >> c;
            gsl_vector_set(m_P, i, c);
        }
    }
    {
        // Absorption and scattering of standard black
        double c;
        data >> c;
        Kb = c;
        data >> c;
        Sb = c;
    }

    // Initialize the reflectance vector and channel converter
    m_refvec = gsl_vector_calloc(wavelengths());
    m_converter = new ChannelConverter<double>(Kb, Sb);

    m_valid = true;

    return true;
}

bool KisIlluminantProfile::save(const QString &fileName)
{
    if (!valid())
        return false;

    QFile file(fileName);
    if (!file.open(QIODevice::WriteOnly))
        return false;
    QDataStream data(&file);

    data << m_illuminant;
    data << (int)m_T->size1 << (int)m_T->size2;
    for (int i = 0; i < (int)m_T->size1; i++)
        for (int j = 0; j < (int)m_T->size2; j++)
            data << gsl_matrix_get(m_T, i, j);

    data << (int)m_P->size;
    for (int i = 0; i < (int)m_P->size; i++)
        data << (int)gsl_vector_get(m_P, i);

    data << Kb << Sb;

    return true;
}

void KisIlluminantProfile::fromRgb(gsl_vector *rgbvec, gsl_vector *ksvec) const
{
    // TODO: add cache!
    m_rgbvec = rgbvec;
    m_ksvec = ksvec;

    rgbToReflectance();
    reflectanceToKS();

    m_rgbvec = 0;
    m_ksvec = 0;
}

void KisIlluminantProfile::toRgb(gsl_vector *ksvec, gsl_vector *rgbvec) const
{
    m_ksvec = ksvec;
    m_rgbvec = rgbvec;

    KSToReflectance();
    reflectanceToRgb();

    m_rgbvec = 0;
    m_ksvec = 0;
}

void KisIlluminantProfile::reflectanceToKS() const
{
    for (int i = 0; i < wavelengths(); i++) {
        m_converter->reflectanceToKS( gsl_vector_get(m_refvec,i),
                                      *gsl_vector_ptr(m_ksvec,2*i+0),
                                      *gsl_vector_ptr(m_ksvec,2*i+1) );
    }
}

void KisIlluminantProfile::KSToReflectance() const
{
    for (int i = 0; i < wavelengths(); i++) {
        gsl_vector_set(m_refvec, i, m_converter->KSToReflectance( gsl_vector_get(m_ksvec, 2*i+0),
                                                                  gsl_vector_get(m_ksvec, 2*i+1) ) );
    }
}

void KisIlluminantProfile::reflectanceToRgb() const
{

    double min, max;
    gsl_vector_minmax(m_refvec, &min, &max);
    if (min == 1.0) {
        gsl_vector_set_all(m_rgbvec, 1.0);
        return;
    }
    if (max == 0.0) {
        gsl_vector_set_all(m_rgbvec, 0.0);
        return;
    }

    gsl_blas_dgemv(CblasNoTrans, 1.0, m_T, m_refvec, 0.0, m_rgbvec);
}

void KisIlluminantProfile::deleteAll()
{
    if (m_T)
        gsl_matrix_free(m_T);
    if (m_P)
        gsl_vector_free(m_P);
    if (m_refvec)
        gsl_vector_free(m_refvec);
    if (m_converter)
        delete m_converter;
}
