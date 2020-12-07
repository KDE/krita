/*
 *  SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KIS_TIMING_INFORMATION_H
#define KIS_TIMING_INFORMATION_H

#include "kritaimage_export.h"

/**
 * A time in milliseconds that is assumed to be longer than any stroke (or other paint operation)
 * will ever last. This is used instead of infinity to avoid potential errors. The value is
 * approximately ten years.
 */
const qreal LONG_TIME = 320000000000.0;

/**
 * Contains information about timing settings in a stroke (mainly for airbrushing effects). The
 * timing settings may be different at different parts of a stroke, e.g. if the airbrush rate is
 * linked to pressure; a KisTimingInformation represents the effective timing at a single specific
 * part of a stroke.
 */
class KRITAIMAGE_EXPORT KisTimingInformation
{
public:

    /** Makes a KisTimingInformation with timed spacing disabled. */
    explicit KisTimingInformation()
        : m_timedSpacingEnabled(false)
        , m_timedSpacingInterval(LONG_TIME)
    {
    }

    /**
     * Makes a KisTimingInformation with timed spacing enabled, using the specified interval in
     * milliseconds.
     */
    explicit KisTimingInformation(qreal interval)
        : m_timedSpacingEnabled(true)
        , m_timedSpacingInterval(interval)
    {
    }

    /**
     * @return True if and only if time-based spacing is enabled.
     */
    inline bool isTimedSpacingEnabled() const {
        return m_timedSpacingEnabled;
    }

    /**
     * @return The desired maximum amount of time between dabs, in milliseconds. Returns LONG_TIME
     * if time-based spacing is disabled.
     */
    inline qreal timedSpacingInterval() const {
        return isTimedSpacingEnabled() ?
                    m_timedSpacingInterval :
                    LONG_TIME;
    }

private:
    // Time-interval-based spacing
    bool m_timedSpacingEnabled;
    qreal m_timedSpacingInterval;
};

#endif // KIS_TIMING_INFORMATION_H
