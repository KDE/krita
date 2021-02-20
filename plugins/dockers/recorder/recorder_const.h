/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef RECORDER_CONST_H
#define RECORDER_CONST_H

class QRegularExpression;

namespace RecorderConst
{

constexpr int waitThreadTimeoutMs = 5000;

extern const QRegularExpression snapshotFilePattern;

}

#endif // RECORDER_CONST_H
