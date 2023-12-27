/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef MYPAINTBASICOPTIONMODEL_H
#define MYPAINTBASICOPTIONMODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "MyPaintBasicOptionData.h"

class MyPaintBasicOptionModel : public QObject
{
    Q_OBJECT
public:
    MyPaintBasicOptionModel(lager::cursor<MyPaintBasicOptionData> optionData,
                            lager::cursor<qreal> radiusCursor,
                            lager::cursor<qreal> hardnessCursor,
                            lager::cursor<qreal> opacityCursor);
    lager::cursor<MyPaintBasicOptionData> optionData;

    LAGER_QT_CURSOR(bool, eraserMode);
    LAGER_QT_CURSOR(qreal, radius);
    LAGER_QT_CURSOR(qreal, hardness);
    LAGER_QT_CURSOR(qreal, opacity);
};

#endif // MYPAINTBASICOPTIONMODEL_H
