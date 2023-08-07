/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISANIMATIONPLAYBACKCONTROLSMODEL_H
#define KISANIMATIONPLAYBACKCONTROLSMODEL_H

#include <QObject>

#include <lager/state.hpp>
#include <lager/extra/qt.hpp>

class KisPlaybackEngine;
class KisCanvasAnimationState;

class KisAnimationPlaybackControlsModel : public QObject
{
    Q_OBJECT
public:
    KisAnimationPlaybackControlsModel();

    void connectAnimationState(KisCanvasAnimationState *state);
    void connectPlaybackEngine(KisPlaybackEngine *engine);

private:
    lager::state<bool, lager::automatic_tag> m_dropFramesMode;
    lager::state<qreal, lager::automatic_tag> m_playbackSpeed;

public:
    LAGER_QT_CURSOR(bool, dropFramesMode);
    LAGER_QT_CURSOR(qreal, playbackSpeed);
    LAGER_QT_CURSOR(int, playbackSpeedDenorm);
};

#endif // KISANIMATIONPLAYBACKCONTROLSMODEL_H
