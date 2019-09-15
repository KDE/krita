/*
 *  Copyright (c) 2017 Dmitry Kazakov <dimula73@gmail.com>
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

#ifndef KISPAINTINGSTROKEDATA_H
#define KISPAINTINGSTROKEDATA_H

#include "kritaui_export.h"

class KisPainter;
class KisDistanceInformation;


/**
 * The distance information should be associated with each
 * painting stroke individually, so we store and manipulate
 * with them together using KisPaintingStrokeInfo structure
 */
class KRITAUI_EXPORT KisFreehandStrokeInfo {
public:
    KisFreehandStrokeInfo();
    KisFreehandStrokeInfo(const KisDistanceInformation &startDist);
    KisFreehandStrokeInfo(KisFreehandStrokeInfo *rhs, int levelOfDetail);
    ~KisFreehandStrokeInfo();

    KisPainter *painter;
    KisDistanceInformation *dragDistance;

    /**
     * The distance inforametion of the associated LodN
     * stroke. Returns zero if LodN stroke has already finished
     * execution or does not exist.
     */
    KisDistanceInformation* buddyDragDistance();

private:
    KisFreehandStrokeInfo *m_parentStrokeInfo {0};
    KisFreehandStrokeInfo *m_childStrokeInfo {0};
};


#endif // KISPAINTINGSTROKEDATA_H
