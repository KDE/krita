/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#ifndef __KISDUPLICATEOPTIONDATA_H_
#define __KISDUPLICATEOPTIONDATA_H_

#include <QPointF>

#include <boost/operators.hpp>

#include <KoID.h>
#include <kritapaintop_export.h>

const QString DUPLICATE_HEALING = "Duplicateop/Healing";
const QString DUPLICATE_CORRECT_PERSPECTIVE = "Duplicateop/CorrectPerspective";
const QString DUPLICATE_MOVE_SOURCE_POINT = "Duplicateop/MoveSourcePoint";
const QString DUPLICATE_RESET_SOURCE_POINT = "Duplicateop/ResetSourcePoint";
const QString DUPLICATE_CLONE_FROM_PROJECTION = "Duplicateop/CloneFromProjection";

class KisPropertiesConfiguration;

struct KisDuplicateOptionData : boost::equality_comparable<KisDuplicateOptionData> {
    inline friend bool operator==(const KisDuplicateOptionData &lhs, const KisDuplicateOptionData &rhs)
    {
        return lhs.healing == rhs.healing && lhs.correctPerspective == rhs.correctPerspective
            && lhs.moveSourcePoint == rhs.moveSourcePoint && lhs.resetSourcePoint == rhs.resetSourcePoint
            && lhs.cloneFromProjection == rhs.cloneFromProjection;
    }

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;

    bool healing {false};
    bool correctPerspective {false};
    bool moveSourcePoint     {true};
    bool resetSourcePoint    {false};
    bool cloneFromProjection {false};
};

#endif // __KISDUPLICATEOPTIONDATA_H_
