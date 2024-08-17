/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSharpnessOptionModel.h"

KisSharpnessOptionModel::KisSharpnessOptionModel(lager::cursor<KisSharpnessOptionMixIn> optionData)
    : sharpnessOptionData(optionData)
    , LAGER_QT(alignOutlinePixels) {sharpnessOptionData[&KisSharpnessOptionMixIn::alignOutlinePixels]}
    , LAGER_QT(softness) {sharpnessOptionData[&KisSharpnessOptionMixIn::softness]}
{
}
