/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisColorSourceOptionModel.h"

#include <KisZug.h>

KisColorSourceOptionModel::KisColorSourceOptionModel(lager::cursor<KisColorSourceOptionData> optionData)
    : optionData(optionData)
    , LAGER_QT(type) {optionData[&KisColorSourceOptionData::type]
            .zoom(kiszug::lenses::do_static_cast<
                  KisColorSourceOptionData::Type, int>)}
{
}

