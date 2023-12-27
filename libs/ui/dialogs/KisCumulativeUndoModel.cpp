/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisCumulativeUndoModel.h"

#include <KisLager.h>

KisCumulativeUndoModel::KisCumulativeUndoModel(lager::cursor<KisCumulativeUndoData> _data)
    : data(_data)
    , LAGER_QT(excludeFromMerge) {data[&KisCumulativeUndoData::excludeFromMerge]}
    , LAGER_QT(mergeTimeout) {data[&KisCumulativeUndoData::mergeTimeout]
                                 .zoom(kislager::lenses::scale_int_to_real(0.001))}
    , LAGER_QT(maxGroupSeparation) {data[&KisCumulativeUndoData::maxGroupSeparation]
                                       .zoom(kislager::lenses::scale_int_to_real(0.001))}
    , LAGER_QT(maxGroupDuration) {data[&KisCumulativeUndoData::maxGroupDuration]
                                     .zoom(kislager::lenses::scale_int_to_real(0.001))}
{
}
