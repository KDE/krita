/*
 *  Copyright (c) 2013 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#ifndef __KIS_SPONTANEOUS_JOB_H
#define __KIS_SPONTANEOUS_JOB_H

#include "kis_runnable.h"

/**
 * This class represents a simple update just that should be
 * executed by the updates system from time to time, without
 * any undo support. Just some useful update that
 * can be run concurrently with other types updates.
 */
class KRITAIMAGE_EXPORT KisSpontaneousJob : public KisRunnable
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
