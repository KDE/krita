/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KOSVGTEXTPROPERTIESMODEL_H
#define KOSVGTEXTPROPERTIESMODEL_H

#include <QObject>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>
#include <lager/state.hpp>

#include "KoSvgText.h"
#include "KoSvgTextPropertyData.h"
#include "CssLengthPercentageModel.h"
#include "LineHeightModel.h"

#include <kritaflake_export.h>

class KRITAFLAKE_EXPORT KoSvgTextPropertiesModel : public QObject
{
    Q_OBJECT
public:
    KoSvgTextPropertiesModel(lager::cursor<KoSvgTextPropertyData> _textData = lager::make_state(KoSvgTextPropertyData(), lager::automatic_tag{}));

    lager::cursor<KoSvgTextPropertyData> textData;
    lager::cursor<KoSvgTextProperties> commonProperties;
    lager::cursor<KoSvgText::CssLengthPercentage> fontSizeData;
    lager::cursor<KoSvgText::LineHeightInfo> lineHeightData;

    CssLengthPercentageModel fontSizeModel;
    LineHeightModel lineHeightModel;

    LAGER_QT_CURSOR(int, writingMode);
    LAGER_QT_CURSOR(int, direction);
    LAGER_QT_CURSOR(int, textAlignAll);

    LAGER_QT_CURSOR(int, fontWeight);
    LAGER_QT_CURSOR(int, fontWidth);
    LAGER_QT_CURSOR(int, fontStyle);
    LAGER_QT_CURSOR(bool, fontOpticalSizeLink);

    LAGER_QT_CURSOR(QStringList, fontFamilies);


Q_SIGNALS:
    void textPropertyChanged();
    void fontSizeChanged();
};

#endif // KOSVGTEXTPROPERTIESMODEL_H
