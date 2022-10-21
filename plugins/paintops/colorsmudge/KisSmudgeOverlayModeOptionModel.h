/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISSMUDGEOVERLAYMODEOPTIONMODEL_H
#define KISSMUDGEOVERLAYMODEOPTIONMODEL_H

#include <QObject>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisSmudgeOverlayModeOptionData.h"


class KisSmudgeOverlayModeOptionModel : public QObject
{
    Q_OBJECT
public:
    KisSmudgeOverlayModeOptionModel(lager::cursor<KisSmudgeOverlayModeOptionData> optionData,
                                    lager::reader<bool> overlayModeAllowed);
    lager::cursor<KisSmudgeOverlayModeOptionData> optionData;
    lager::reader<bool> overlayModeAllowed;

    LAGER_QT_CURSOR(bool, isChecked);

    KisSmudgeOverlayModeOptionData bakedOptionData() const;
};

#endif // KISSMUDGEOVERLAYMODEOPTIONMODEL_H
