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


class KisAnimationPlaybackControlsModel : public QObject
{
    Q_OBJECT
public:
    KisAnimationPlaybackControlsModel();
private:
    lager::state<bool, lager::automatic_tag> m_dropFramesMode;

public:
    LAGER_QT_CURSOR(bool, dropFramesMode);
};

#endif // KISANIMATIONPLAYBACKCONTROLSMODEL_H
