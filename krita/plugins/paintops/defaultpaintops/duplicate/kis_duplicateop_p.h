/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
 *  Copyright (c) 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  Copyright (c) 2004 Clarence Dang <dang@kde.org>
 *  Copyright (c) 2004 Adrian Page <adrian@pagenet.plus.com>
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
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

#ifndef KIS_DUPLICATEOP_P_H_
#define KIS_DUPLICATEOP_P_H_

namespace DuplicateOpUtils {

inline qreal minimizeEnergy(const qreal* m, qreal* sol, int w, int h)
{
    qreal err = 0;

    if (h < 3 || w < 3) {
        int size = 3 * w * h;

        for (int i = 0; i < size; i++) {
            sol[i] = 1.0;
        }

        qWarning() << "WARNING: healing width or height are smaller than 3 px. The result will have artifacts!";

    } else {
        int rowstride = 3 * w;

        memcpy(sol, m, 3 * sizeof(qreal) * w);
        m += rowstride;
        sol += rowstride;
        for (int i = 1; i < h - 1; i++) {
            memcpy(sol, m, 3 * sizeof(qreal));
            m += 3; sol += 3;
            for (int j = 3; j < rowstride - 3; j++) {
                qreal tmp = *sol;
                *sol = ((*(m - 3) + * (m + 3) + * (m - rowstride) + * (m + rowstride)) + 2 * *m) / 6;
                qreal diff = *sol - tmp;
                err += diff * diff;
                m ++; sol ++;
            }
            memcpy(sol, m, 3 * sizeof(qreal));
            m += 3; sol += 3;
        }
        memcpy(sol, m, 3 * sizeof(qreal) * w);
    }

    return err;
}
}

#endif // KIS_DUPLICATEOP_P_H_
