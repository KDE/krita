/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2002 David Faure <faure@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#ifndef koDirectoryStore_h
#define koDirectoryStore_h

#include "KoStore.h"

class QFile;

class KoDirectoryStore : public KoStore
{
public:
    KoDirectoryStore(const QString& path, Mode _mode, bool writeMimetype);
    ~KoDirectoryStore() override;
protected:
    void init();
    bool openWrite(const QString &name) override {
        return openReadOrWrite(name, QIODevice::WriteOnly);
    }
    bool openRead(const QString &name) override {
        return openReadOrWrite(name, QIODevice::ReadOnly);
    }
    bool closeRead() override {
        return true;
    }
    bool closeWrite() override {
        return true;
    }
    bool enterRelativeDirectory(const QString &dirName) override;
    bool enterAbsoluteDirectory(const QString &path) override;
    bool fileExists(const QString &absPath) const override;

    bool openReadOrWrite(const QString &name, QIODevice::OpenModeFlag ioMode);
private:
    // Path to base directory (== the ctor argument)
    QString m_basePath;

    // Path to current directory
    QString m_currentPath;

    // Current File
    QFile* m_file;
    Q_DECLARE_PRIVATE(KoStore)
};

#endif
