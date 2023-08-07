/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAnimationPlaybackControlsModel.h"

#include "KisLager.h"

KisAnimationPlaybackControlsModel::KisAnimationPlaybackControlsModel()
    : m_dropFramesMode {true}
    , LAGER_QT(dropFramesMode) {m_dropFramesMode}
    , LAGER_QT(playbackSpeed) {m_playbackSpeed}
    , LAGER_QT(playbackSpeedDenorm) {m_playbackSpeed.zoom(kislager::lenses::scale_real_to_int(100.0))}
{
}
