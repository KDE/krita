/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "KisAnimationPlaybackControlsModel.h"

KisAnimationPlaybackControlsModel::KisAnimationPlaybackControlsModel()
    : m_dropFramesMode {true}
    , LAGER_QT(dropFramesMode) {m_dropFramesMode}
{
}
