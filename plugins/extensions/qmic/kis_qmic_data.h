/*
 *  SPDX-FileCopyrightText: 2015 Lukáš Tvrdý <lukast.dev@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
