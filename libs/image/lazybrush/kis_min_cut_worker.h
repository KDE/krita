/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_MIN_CUT_WORKER_H
#define __KIS_MIN_CUT_WORKER_H

#include <QScopedPointer>


class KisMinCutWorker
{
public:
    KisMinCutWorker();
    ~KisMinCutWorker();

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_MIN_CUT_WORKER_H */
