/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef RECORDER_CONST_H
#define RECORDER_CONST_H

#include <QThread>

class QString;
class QRegularExpression;

namespace RecorderConst
{

constexpr int waitThreadTimeoutMs = 5000;

QRegularExpression snapshotFilePatternFor(const QString &extension);

}

namespace ThreadSystemValue
{
const unsigned int MaxThreadCount = QThread::idealThreadCount();
const unsigned int MaxRecordThreadCount = qMax(static_cast<int>(MaxThreadCount) - 1, 1);
const unsigned int IdealRecordThreadCount = qMax(static_cast<int>(MaxRecordThreadCount) - 2, 1);
}

#endif // RECORDER_CONST_H
