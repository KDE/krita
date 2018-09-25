/*
 *  Copyright (c) 2015 Lukáš Tvrdý <lukast.dev@gmail.com>
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

#ifndef KIS_QMIC_DATA
#define KIS_QMIC_DATA

#include <QDebug>

class KisQmicData
{
public:
    KisQmicData();
    ~KisQmicData();

    float progress() const { return m_progress; }
    bool isCancelled() const { return m_cancel; }
    void setCancel(bool cancel) { m_cancel = cancel; }

    bool & cancelPtr() { return m_cancel; }
    float & progressPtr() { return m_progress; }

    void reset();

    static const float INVALID_PROGRESS_VALUE;

private:
    float m_progress;
    bool m_cancel;
};

#include <QSharedPointer>
typedef QSharedPointer<KisQmicData> KisQmicDataSP;

#endif
