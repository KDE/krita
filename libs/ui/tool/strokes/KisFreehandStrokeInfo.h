/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
