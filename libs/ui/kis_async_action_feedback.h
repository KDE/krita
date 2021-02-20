/*
 *  SPDX-FileCopyrightText: 2016 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#ifndef __KIS_ASYNC_ACTION_FEEDBACK_H
#define __KIS_ASYNC_ACTION_FEEDBACK_H

#include <QScopedPointer>
#include <functional>
#include "KisImportExportFilter.h"

class QWidget;
class QMutex;

class KisAsyncActionFeedback
{
public:
    KisAsyncActionFeedback(const QString &message, QWidget *parent);
    ~KisAsyncActionFeedback();

    KisImportExportErrorCode runAction(std::function<KisImportExportErrorCode()> func);
    void runVoidAction(std::function<void()> func);
    void waitForMutex(QMutex *mutex);

private:
    struct Private;
    const QScopedPointer<Private> m_d;
};

#endif /* __KIS_ASYNC_ACTION_FEEDBACK_H */
