/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef TEXTINDENTMODEL_H
#define TEXTINDENTMODEL_H

#include <KoSvgText.h>
#include <lager/state.hpp>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include <kritaflake_export.h>

#include "CssLengthPercentageModel.h"

class KRITAFLAKE_EXPORT TextIndentModel : public QObject
{
    Q_OBJECT
    Q_PROPERTY(CssLengthPercentageModel *length READ length NOTIFY lengthChanged)
public:
    explicit TextIndentModel(lager::cursor<KoSvgText::TextIndentInfo> _textData = lager::make_state(KoSvgText::TextIndentInfo(), lager::automatic_tag{}));

    lager::cursor<KoSvgText::TextIndentInfo> data;
    lager::cursor<KoSvgText::CssLengthPercentage> lengthData;
    CssLengthPercentageModel lengthModel;

    CssLengthPercentageModel *length();

    LAGER_QT_CURSOR(bool, hanging);
    LAGER_QT_CURSOR(bool, eachLine);

Q_SIGNALS:
    void lengthChanged();

};

#endif // TEXTINDENTMODEL_H
