/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#include "recorder_directory_cleaner.h"
#include "recorder_const.h"

#include <QDebug>
#include <QDir>


RecorderDirectoryCleaner::RecorderDirectoryCleaner(const QStringList &directories_)
    : directories(directories_)
{
    moveToThread(this);
}

void RecorderDirectoryCleaner::stop() {
    if (!isRunning())
        return;

    terminate();
    if (!wait(RecorderConst::waitThreadTimeoutMs)) {
        qCritical() << "Unable to stop BackgroundDirectoryRemover";
    }
}

void RecorderDirectoryCleaner::run()
{
    for (const QString &directory : directories) {
        QDir(directory).removeRecursively();
    }
}
