/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "recorder_const.h"

#include <QRegularExpression>

namespace RecorderConst
{

QRegularExpression snapshotFilePatternFor(const QString &extension)
{
    return QRegularExpression("^([0-9]{7})\\." % extension % "$");
}

}
