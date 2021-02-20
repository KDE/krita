/*
 *  SPDX-FileCopyrightText: 2017 Bernhard Liebl <poke1024@gmx.de>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_latency_tracker.h"

KisLatencyTracker::KisLatencyTracker(int windowSize) :
    KisScalarTracker<qint64>("event latency", windowSize)
{
}

void KisLatencyTracker::push(qint64 timestamp)
{
    const qint64 latency = currentTimestamp() - timestamp;
    KisScalarTracker<qint64>::push(latency);
}
