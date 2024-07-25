/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "MyPaintBasicOptionModel.h"


MyPaintBasicOptionModel::MyPaintBasicOptionModel(lager::cursor<MyPaintBasicOptionData> _optionData,
                                                 lager::cursor<qreal> radiusCursor,
                                                 lager::cursor<qreal> hardnessCursor,
                                                 lager::cursor<qreal> opacityCursor)
    : optionData(_optionData)
    , LAGER_QT(eraserMode) {optionData[&MyPaintBasicOptionData::eraserMode]}
    , LAGER_QT(radius) {radiusCursor}
    , LAGER_QT(hardness) {hardnessCursor}
    , LAGER_QT(opacity) {opacityCursor}
{
}
