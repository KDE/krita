/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISAUTOBRUSHMODEL_H
#define KISAUTOBRUSHMODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include <KisBrushModel.h>
#include <KisWidgetConnectionUtils.h>

using namespace KisBrushModel;
using namespace KisWidgetConnectionUtils;


class KisAutoBrushModel : public QObject
{
    Q_OBJECT
public:
    KisAutoBrushModel(lager::cursor<CommonData> commonData,
                      lager::cursor<AutoBrushData> autoBrushData,
                      lager::cursor<qreal> commonBrushSizeData);

    // the state must be declared **before** any cursors or readers
    lager::cursor<KisBrushModel::CommonData> m_commonData;
    lager::cursor<KisBrushModel::AutoBrushData> m_autoBrushData;
    lager::cursor<qreal> m_commonBrushSizeData;

    LAGER_QT_CURSOR(qreal, diameter);
    LAGER_QT_CURSOR(qreal, ratio);
    LAGER_QT_CURSOR(qreal, horizontalFade);
    LAGER_QT_CURSOR(qreal, verticalFade);
    LAGER_QT_CURSOR(int, spikes);
    LAGER_QT_CURSOR(bool, antialiasEdges);
    LAGER_QT_CURSOR(int, shape);
    LAGER_QT_CURSOR(int, type);
    LAGER_QT_CURSOR(QString, curveString);
    LAGER_QT_CURSOR(qreal, randomness);
    LAGER_QT_CURSOR(qreal, density);
    LAGER_QT_CURSOR(qreal, angle);
    LAGER_QT_CURSOR(qreal, spacing);
    LAGER_QT_CURSOR(bool, useAutoSpacing);
    LAGER_QT_CURSOR(qreal, autoSpacingCoeff);
    LAGER_QT_CURSOR(SpacingState, aggregatedSpacing);

    AutoBrushData bakedOptionData() const;
};

#endif // KISAUTOBRUSHMODEL_H
