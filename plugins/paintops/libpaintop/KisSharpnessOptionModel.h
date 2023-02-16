/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSHARPNESSOPTIONMODEL_H
#define KISSHARPNESSOPTIONMODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisSharpnessOptionData.h"

class KisSharpnessOptionModel : public QObject
{
    Q_OBJECT
public:
    KisSharpnessOptionModel(lager::cursor<KisSharpnessOptionMixIn> optionData);
    lager::cursor<KisSharpnessOptionMixIn> sharpnessOptionData;
    LAGER_QT_CURSOR(bool, alignOutlinePixels);
    LAGER_QT_CURSOR(int, softness);
};

#endif // KISSHARPNESSOPTIONMODEL_H
