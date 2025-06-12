/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisFrameRateLimitModel.h"

#include <lager/lenses/tuple.hpp>
#include <lager/constant.hpp>
#include <KisLager.h>
#include <kis_assert.h>

#include <QGuiApplication>
#include <QScreen>

namespace {
    int calculateMaxScreenFrameRate() {
        const QList<QScreen*> screens = qGuiApp->screens();
        qreal maxRefreshRate = 30;
        for (QScreen *screen : screens) {
            if (screen->refreshRate() > maxRefreshRate) {
                maxRefreshRate = screen->refreshRate();
            }
        }
        return qRound(maxRefreshRate);
    }
}

auto frameRateLens = lager::lenses::getset(
    [](const KisFrameRateLimitModel::Data &x) -> int { return std::get<bool>(x) ? calculateMaxScreenFrameRate() : std::get<int>(x); },
    [](KisFrameRateLimitModel::Data x, int frameRate) -> KisFrameRateLimitModel::Data {
        if (!std::get<bool>(x)) {
            std::get<int>(x) = frameRate;
        }
        return x;
    });

KisFrameRateLimitModel::KisFrameRateLimitModel(lager::cursor<Data> _data)
    : data(_data)
    , LAGER_QT(detectFrameRate){data.zoom(lager::lenses::first)}
    , LAGER_QT(frameRate){data.zoom(frameRateLens)}
    , LAGER_QT(frameRateState) {
        lager::with(LAGER_QT(frameRate),
                    lager::make_constant(1),
                    lager::make_constant(300),
                    LAGER_QT(detectFrameRate).map(std::logical_not<>{}))
            .map(KisWidgetConnectionUtils::ToSpinBoxState{})
        }
{
}