/*
   This file is part of the KDE libraries
   SPDX-FileCopyrightText: 2006 Thomas Braxton <brax108@cox.net>
   SPDX-FileCopyrightText: 1999 Preston Brown <pbrown@kde.org>
   SPDX-FileCopyrightText: 1997-1999 Matthias Kalle Dalheimer <kalle@kde.org>

   SPDX-License-Identifier: LGPL-2.0-or-later
*/

#include "kconfigbackend_p.h"

#include <QDateTime>
#include <QStringList>
#include <QDir>
#include <QFileInfo>
#include <QHash>
#include <QDebug>

#include "kconfigini_p.h"
#include "kconfigdata.h"

typedef QExplicitlySharedDataPointer<KConfigBackend> BackendPtr;

class KConfigBackendPrivate
{
public:
    QString localFileName;

    static QString whatSystem(const QString & /*fileName*/)
    {
        return QStringLiteral("INI");
    }
};

void KConfigBackend::registerMappings(const KEntryMap & /*entryMap*/)
{
}

BackendPtr KConfigBackend::create(const QString &file)
{
    //qDebug() << "creating a backend for file" << file << "with system" << sys;
    KConfigBackend *backend = nullptr;

    //qDebug() << "default creation of the Ini backend";
    backend = new KConfigIniBackend;
    backend->setFilePath(file);
    return BackendPtr(backend);
}

KConfigBackend::KConfigBackend()
    : d(new KConfigBackendPrivate)
{
}

KConfigBackend::~KConfigBackend()
{
    delete d;
}

QString KConfigBackend::filePath() const
{
    return d->localFileName;
}

void KConfigBackend::setLocalFilePath(const QString &file)
{
    d->localFileName = file;
}

