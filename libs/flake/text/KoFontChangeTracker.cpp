/*
 *  SPDX-FileCopyrightText: 2024 Wolthera van Hövell tot Westerflier <griffinvalley@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KoFontChangeTracker.h"

#include <QFileSystemWatcher>
#include <kis_signal_compressor.h>
#include <QDebug>

struct KoFontChangeTracker::Private {

    Private(QStringList paths = QStringList(), int refresh = 500)
        : fileSystemWatcher(paths) {

    }
    QFileSystemWatcher fileSystemWatcher;

    bool filesChanged = false;
    bool configStale = false;
};

KoFontChangeTracker::KoFontChangeTracker(QStringList paths, QObject *parent)
    : QObject(parent)
    , d(new Private(paths))
{
    connect(&d->fileSystemWatcher, SIGNAL(directoryChanged(QString)), SLOT(directoriesChanged(QString)));
}

KoFontChangeTracker::~KoFontChangeTracker()
{
}

void KoFontChangeTracker::resetChangeTracker()
{
    d->filesChanged = false;
}


void KoFontChangeTracker::directoriesChanged(QString path)
{
    if (!d->filesChanged) {
        d->filesChanged = true;
        emit (sigUpdateConfig());
    }
}
