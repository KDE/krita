/*
 *  SPDX-FileCopyrightText: 2022 Agata Cacko <cacko.azh@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_SPRAY_SHAPE_OPTION_MODEL_H
#define KIS_SPRAY_SHAPE_OPTION_MODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisSprayShapeOptionData.h"
#include "KisWidgetConnectionUtils.h"

struct SprayShapeSizePack
{
    QSize pxSize;
    QSize proportionalSize;
    qreal scale {1.0};
    int diameter {0};
    bool isProportional {false};
};

class KisSprayShapeOptionModel : public QObject
{
    Q_OBJECT
public:
    KisSprayShapeOptionModel(lager::cursor<KisSprayShapeOptionData> optionData, lager::cursor<int> diameter, lager::cursor<qreal> scale);

    lager::cursor<KisSprayShapeOptionData> optionData;

    lager::cursor<SprayShapeSizePack> sizePack;

    LAGER_QT_CURSOR(int, shape);
    LAGER_QT_CURSOR(QSize, effectiveSize);
    LAGER_QT_CURSOR(bool, effectiveProportional);
    
    LAGER_QT_CURSOR(bool, enabled);
    
    LAGER_QT_CURSOR(QString, imageUrl);
    
    LAGER_QT_READER(QString, sizeSuffix);
};

#endif // KIS_SPRAY_SHAPE_OPTION_MODEL_H
