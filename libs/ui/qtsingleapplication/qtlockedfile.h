// Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
// SPDX-License-Identifier: BSD-3-Clause

#ifndef QTLOCKEDFILE_H
#define QTLOCKEDFILE_H

#include <QFile>
#ifdef Q_OS_WIN
#include <QVector>
#endif

namespace QtLP_Private {

class QtLockedFile : public QFile
{
public:
    enum LockMode { NoLock = 0, ReadLock, WriteLock };

    QtLockedFile();
    QtLockedFile(const QString &name);
    ~QtLockedFile();

    bool open(OpenMode mode);

    bool lock(LockMode mode, bool block = true);
    bool unlock();
    bool isLocked() const;
    LockMode lockMode() const;

private:
#ifdef Q_OS_WIN
    Qt::HANDLE wmutex;
    Qt::HANDLE rmutex;
    QVector<Qt::HANDLE> rmutexes;
    QString mutexname;

    Qt::HANDLE getMutexHandle(int idx, bool doCreate);
    bool waitMutex(Qt::HANDLE mutex, bool doBlock);

#endif
    LockMode m_lock_mode;
};
}
#endif
