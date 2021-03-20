/*
 *  SPDX-FileCopyrightText: 2017 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef KISRUNNABLESTROKEJOBDATA_H
#define KISRUNNABLESTROKEJOBDATA_H

#include "kritaimage_export.h"
#include "KisRunnableStrokeJobDataBase.h"
#include <functional>

class QRunnable;

class KRITAIMAGE_EXPORT KisRunnableStrokeJobData : public KisRunnableStrokeJobDataBase {
public:
    KisRunnableStrokeJobData(QRunnable *runnable, KisStrokeJobData::Sequentiality sequentiality = KisStrokeJobData::SEQUENTIAL,
                             KisStrokeJobData::Exclusivity exclusivity = KisStrokeJobData::NORMAL);

    KisRunnableStrokeJobData(std::function<void()> func, KisStrokeJobData::Sequentiality sequentiality = KisStrokeJobData::SEQUENTIAL,
                             KisStrokeJobData::Exclusivity exclusivity = KisStrokeJobData::NORMAL);

    ~KisRunnableStrokeJobData();

    void run() override;

private:
    QRunnable *m_runnable = 0;
    std::function<void()> m_func;
};

#endif // KISRUNNABLESTROKEJOBDATA_H
