/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef RECORDER_CONST_H
#define RECORDER_CONST_H

class QString;
class QRegularExpression;

namespace RecorderConst
{

constexpr int waitThreadTimeoutMs = 5000;

QRegularExpression snapshotFilePatternFor(const QString &extension);

}

#endif // RECORDER_CONST_H
