/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *  SPDX-FileCopyrightText: 2004-2008 Boudewijn Rempt <boud@valdyas.org>
 *  SPDX-FileCopyrightText: 2004 Clarence Dang <dang@kde.org>
 *  SPDX-FileCopyrightText: 2004 Adrian Page <adrian@pagenet.plus.com>
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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

        warnKrita << "WARNING: healing width or height are smaller than 3 px. The result will have artifacts!";

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
