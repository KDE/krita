/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISPREDEFINEDBRUSHMODEL_H
#define KISPREDEFINEDBRUSHMODEL_H

#include <lager/cursor.hpp>
#include <lager/constant.hpp>
#include <lager/lenses.hpp>
#include <lager/extra/qt.hpp>

#include <KisBrushModel.h>
#include <KisWidgetConnectionUtils.h>

using namespace KisBrushModel;
using namespace KisWidgetConnectionUtils;


class KisPredefinedBrushModel : public QObject
{
    Q_OBJECT
public:
    KisPredefinedBrushModel(lager::cursor<CommonData> commonData,
                            lager::cursor<PredefinedBrushData> predefinedBrushData,
                            lager::cursor<qreal> commonBrushSizeData,
                            bool supportsHSLBrushTips);

    // the state must be declared **before** any cursors or readers
    lager::cursor<CommonData> m_commonData;
    lager::cursor<PredefinedBrushData> m_predefinedBrushData;
    lager::constant<bool> m_supportsHSLBrushTips;
    lager::cursor<qreal> m_commonBrushSizeData;

    lager::cursor<PredefinedBrushData> m_effectivePredefinedData;


    LAGER_QT_CURSOR(KoResourceSignature, resourceSignature);
    LAGER_QT_CURSOR(QSize, baseSize);
    LAGER_QT_CURSOR(qreal, brushSize);
    LAGER_QT_CURSOR(int, application);
    LAGER_QT_CURSOR(bool, hasColorAndTransparency);
    LAGER_QT_CURSOR(bool, autoAdjustMidPoint);
    LAGER_QT_CURSOR(int, adjustmentMidPoint);
    LAGER_QT_CURSOR(int, brightnessAdjustment);
    LAGER_QT_CURSOR(int, contrastAdjustment);

    LAGER_QT_CURSOR(qreal, angle);
    LAGER_QT_CURSOR(qreal, spacing);
    LAGER_QT_CURSOR(bool, useAutoSpacing);
    LAGER_QT_CURSOR(qreal, autoSpacingCoeff);
    LAGER_QT_CURSOR(SpacingState, aggregatedSpacing);
    LAGER_QT_READER(ComboBoxState, applicationSwitchState);
    LAGER_QT_READER(bool, adjustmentsEnabled);
    LAGER_QT_READER(QString, brushName);
    LAGER_QT_READER(QString, brushDetails);
    LAGER_QT_READER(bool, lightnessModeEnabled);

    PredefinedBrushData bakedOptionData() const;

    static enumBrushApplication effectiveBrushApplication(PredefinedBrushData predefinedData, bool supportsHSLBrushTips);
    static qreal effectiveBrushSize(PredefinedBrushData predefinedData);
    static void  setEffectiveBrushSize(PredefinedBrushData &predefinedData, qreal value);
};

#endif // KISPREDEFINEDBRUSHMODEL_H
