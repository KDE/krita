/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisCompositeOpOptionModel.h"

KisCompositeOpOptionModel::KisCompositeOpOptionModel(lager::cursor<KisCompositeOpOptionData> _optionData)
    : optionData(_optionData)
    , LAGER_QT(compositeOpId) {optionData[&KisCompositeOpOptionData::compositeOpId]}
    , LAGER_QT(eraserMode) {optionData[&KisCompositeOpOptionData::eraserMode]}

{
}
