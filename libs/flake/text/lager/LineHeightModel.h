/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef LINEHEIGHTMODEL_H
#define LINEHEIGHTMODEL_H

#include <QObject>
#include <KoSvgText.h>
#include <lager/state.hpp>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include <kritaflake_export.h>

class KRITAFLAKE_EXPORT LineHeightModel: public QObject
{
    Q_OBJECT
public:
    LineHeightModel(lager::cursor<KoSvgText::LineHeightInfo> _data = lager::make_state(KoSvgText::LineHeightInfo(), lager::automatic_tag{}));

    enum LineHeightType {
        Absolute,
        Em,
        Ex,
        Percentage,
        Lines
    };
    Q_ENUM(LineHeightType)

    lager::cursor<KoSvgText::LineHeightInfo> data;

    LAGER_QT_CURSOR(bool, isNormal);
    LAGER_QT_CURSOR(qreal, value);
    LAGER_QT_CURSOR(LineHeightType, unit);
};

#endif // LINEHEIGHTMODEL_H
