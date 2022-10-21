/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSMUDGEOVERLAYMODEOPTIONDATA_H
#define KISSMUDGEOVERLAYMODEOPTIONDATA_H

#include <QtGlobal>
#include <boost/operators.hpp>

class KisPropertiesConfiguration;
class KisPaintopLodLimitations;

struct KisSmudgeOverlayModeOptionData : boost::equality_comparable<KisSmudgeOverlayModeOptionData>
{
    inline friend bool operator==(const KisSmudgeOverlayModeOptionData &lhs, const KisSmudgeOverlayModeOptionData &rhs) {
        return lhs.isChecked == rhs.isChecked;
    }

    bool isChecked {false};

    bool read(const KisPropertiesConfiguration *setting);
    void write(KisPropertiesConfiguration *setting) const;

    KisPaintopLodLimitations lodLimitations() const;
};

#endif // KISSMUDGEOVERLAYMODEOPTIONDATA_H
