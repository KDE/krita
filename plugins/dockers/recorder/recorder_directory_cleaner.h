/*
 *  SPDX-FileCopyrightText: 2020 Dmitrii Utkin <loentar@gmail.com>
 *
 *  SPDX-License-Identifier: LGPL-2.1-only
 */

#ifndef RECORDER_DIRECTORY_CLEANER_H
#define RECORDER_DIRECTORY_CLEANER_H

#include <QThread>
#include <QStringList>

class RecorderDirectoryCleaner : public QThread
{
public:
    RecorderDirectoryCleaner(const QStringList &directories);

    void stop();

protected:
    void run() override;

private:
    QStringList directories;
};


#endif // RECORDER_DIRECTORY_CLEANER_H
