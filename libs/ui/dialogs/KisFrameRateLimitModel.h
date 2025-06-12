/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISFRAMERATELIMITMODEL_H
#define KISFRAMERATELIMITMODEL_H

#include <QObject>
#include <tuple>
#include <lager/state.hpp>
#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>
#include <KisWidgetConnectionUtils.h>

class KisFrameRateLimitModel : public QObject
{
    Q_OBJECT

public:
    using Data = std::tuple<bool, int>;
public:
    KisFrameRateLimitModel(lager::cursor<Data> _data = lager::make_state(Data(true, 60), lager::automatic_tag{}));

    lager::cursor<Data> data;

    LAGER_QT_CURSOR(bool, detectFrameRate);
    LAGER_QT_CURSOR(int, frameRate);
    LAGER_QT_READER(IntSpinBoxState, frameRateState);
};

#endif // KISFRAMERATELIMITMODEL_H