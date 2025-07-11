/*
 *  SPDX-FileCopyrightText: 2025 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KOZOOMACTIONSTATE_H
#define KOZOOMACTIONSTATE_H

#include "kritawidgets_export.h"

#include <QVector>
#include <tuple>
#include "KoZoomState.h"


class KRITAWIDGETS_EXPORT KoZoomActionState
{
public:
    using ZoomItem = std::tuple<KoZoomMode::Mode, qreal, QString>;

    KoZoomState zoomState;
    QVector<qreal> standardLevels;
    QVector<ZoomItem> guiLevels;
    QVector<ZoomItem> realGuiLevels;
    int currentRealLevelIndex = 0;
    QString currentRealLevelText;

    KoZoomActionState(const KoZoomState &state);
    void setZoomState(const KoZoomState &state);

    int calcNearestStandardLevel(qreal zoom) const;
    int calcNearestStandardLevel() const;
};

KRITAWIDGETS_EXPORT
QDebug operator<<(QDebug dbg, const KoZoomActionState::ZoomItem &item);

#endif // KOZOOMACTIONSTATE_H