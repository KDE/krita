/*
 *  SPDX-FileCopyrightText: 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_SPONTANEOUS_JOB_H
#define __KIS_SPONTANEOUS_JOB_H

#include "kis_runnable_with_debug_name.h"

/**
 * This class represents a simple update just that should be
 * executed by the updates system from time to time, without
 * any undo support. Just some useful update that
 * can be run concurrently with other types updates.
 */
class KRITAIMAGE_EXPORT KisSpontaneousJob : public KisRunnableWithDebugName
{
public:
    virtual bool overrides(const KisSpontaneousJob *otherJob) = 0;
    virtual int levelOfDetail() const = 0;
    bool isExclusive() const {
        return m_isExclusive;
    }

protected:
    void setExclusive(bool value) {
        m_isExclusive = value;
    }

private:
    bool m_isExclusive = false;
};

#endif /* __KIS_SPONTANEOUS_JOB_H */
