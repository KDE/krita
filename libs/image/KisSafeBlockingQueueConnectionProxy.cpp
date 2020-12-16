/*
 *  SPDX-FileCopyrightText: 2020 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisSafeBlockingQueueConnectionProxy.h"

#include <QThread>
#include <QApplication>
#include <KisBusyWaitBroker.h>

void KisSafeBlockingQueueConnectionProxyPrivate::passBlockingSignalSafely(FunctionToSignalProxy &source, SignalToFunctionProxy &destination)
{
    if (QThread::currentThread() == qApp->thread() ||
        KisBusyWaitBroker::instance()->guiThreadIsWaitingForBetterWeather()) {

        destination.start();
    } else {
        source.start();
    }
}

void KisSafeBlockingQueueConnectionProxyPrivate::initProxyObject(QObject *object)
{
    object->moveToThread(qApp->thread());
}
