/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISMIRROROPTIONMODEL_H
#define KISMIRROROPTIONMODEL_H

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisMirrorOptionData.h"

class PAINTOP_EXPORT KisMirrorOptionModel : public QObject
{
    Q_OBJECT
public:
    KisMirrorOptionModel(lager::cursor<KisMirrorOptionMixIn> optionData);
    lager::cursor<KisMirrorOptionMixIn> mirrorOptionData;
    LAGER_QT_CURSOR(bool, enableVerticalMirror);
    LAGER_QT_CURSOR(bool, enableHorizontalMirror);
};

#endif // KISMIRROROPTIONMODEL_H
