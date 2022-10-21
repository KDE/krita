/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISPAINTTHICKNESSOPTIONMODEL_H
#define KISPAINTTHICKNESSOPTIONMODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisPaintThicknessOptionData.h"
#include "KisWidgetConnectionUtils.h"

class KisPaintThicknessOptionModel : public QObject
{
    Q_OBJECT
public:
    KisPaintThicknessOptionModel(lager::cursor<KisPaintThicknessOptionMixIn> optionData);
    lager::cursor<KisPaintThicknessOptionMixIn> optionData;
    LAGER_QT_CURSOR(int, mode);

};

#endif // KISPAINTTHICKNESSOPTIONMODEL_H
