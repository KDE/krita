/*
 *  SPDX-FileCopyrightText: 2023 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KISCUMULATIVEUNDOMODEL_H
#define KISCUMULATIVEUNDOMODEL_H

#include <QObject>

#include <lager/cursor.hpp>
#include <lager/extra/qt.hpp>

#include "KisCumulativeUndoData.h"

class KisCumulativeUndoModel : public QObject
{
    Q_OBJECT
public:
    KisCumulativeUndoModel(lager::cursor<KisCumulativeUndoData> _data);

    lager::cursor<KisCumulativeUndoData> data;

    LAGER_QT_CURSOR(int, excludeFromMerge);
    LAGER_QT_CURSOR(qreal, mergeTimeout);
    LAGER_QT_CURSOR(qreal, maxGroupSeparation);
    LAGER_QT_CURSOR(qreal, maxGroupDuration);
};

#endif // KISCUMULATIVEUNDOMODEL_H
