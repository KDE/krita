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

#include <gsl/gsl_matrix.h>
#include <gsl/gsl_vector.h>

KisIlluminantProfile::KisIlluminantProfile(const QString &fileName)
    : KoColorProfile(fileName), m_T(0), m_P(0), m_valid(false)
{
    load();
}

KisIlluminantProfile::KisIlluminantProfile(const KisIlluminantProfile &profile)
    : KoColorProfile(profile.fileName()), m_T(0), m_P(0)
{
    setName(profile.name());
    setInfo(profile.info());
    m_valid = profile.valid();
    if (profile.valid()) {
        m_T = gsl_matrix_alloc(profile.m_T->size1, profile.m_T->size2);
        m_P = gsl_vector_alloc(profile.m_P->size);
        gsl_matrix_memcpy(m_T, profile.m_T);
        gsl_vector_memcpy(m_P, profile.m_P);
        S_w = profile.S_w;
        K_b = profile.K_b;
    }
}

KisIlluminantProfile::~KisIlluminantProfile()
{
    if (m_T)
        gsl_matrix_free(m_T);
    if (m_P)
        gsl_vector_free(m_P);
}

KoColorProfile *KisIlluminantProfile::clone() const
{
    return (new KisIlluminantProfile(*this));
}

bool KisIlluminantProfile::load() // TODO Info
{
    if (fileName().isEmpty())
        return false;

    QFile file(fileName());
    if (!file.open(QIODevice::ReadOnly))
        return false;

    QDataStream data(&file);

    {
        // Profile name
        QString name;
        data >> name;
        setName(name);
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
            gsl_vector_set(m_P, i, (double)c);
        }
    }
    {
        // Scattering of white lead and absorption of black lead
        double c;
        data >> c;
        S_w = (float)c;
        data >> c;
        K_b = (float)c;
    }

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

    data << name();
    data << (int)m_T->size1 << (int)m_T->size2;
    for (int i = 0; i < (int)m_T->size1; i++)
        for (int j = 0; j < (int)m_T->size2; j++)
            data << gsl_matrix_get(m_T, i, j);

    data << (int)m_P->size;
    for (int i = 0; i < (int)m_P->size; i++)
        data << (int)gsl_vector_get(m_P, i);

    data << S_w << K_b;

    return true;
}
