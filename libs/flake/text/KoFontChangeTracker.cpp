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
        : fileSystemWatcher(paths)
        , configSignalCompressor(refresh, KisSignalCompressor::POSTPONE) {

    }
    QFileSystemWatcher fileSystemWatcher;
    KisSignalCompressor configSignalCompressor;

    bool filesChanged = false;
    bool configStale = false;
};

KoFontChangeTracker::KoFontChangeTracker(QStringList paths, int configRefreshValue, QObject *parent)
    : QObject(parent)
    , d(new Private(paths, configRefreshValue))
{
    d->fileSystemWatcher;

    connect(&d->fileSystemWatcher, SIGNAL(directoryChanged(QString)), SLOT(directoriesChanged(QString)));
    connect(&d->configSignalCompressor, SIGNAL(timeout()), this, SLOT(intervalElapsed()));
}

KoFontChangeTracker::~KoFontChangeTracker()
{
}

void KoFontChangeTracker::resetChangeTracker()
{
    d->configStale = false;
    d->filesChanged = false;
    d->configSignalCompressor.start();
}

void KoFontChangeTracker::connectToRegistery()
{
    connect(this, SIGNAL(sigUpdateConfig()), this, SLOT(updateFontRegistery()), Qt::UniqueConnection);
}

#include <KoFontRegistry.h>
void KoFontChangeTracker::updateFontRegistery()
{
    KoFontRegistry::instance()->updateConfig();
}

void KoFontChangeTracker::intervalElapsed()
{
    if (!d->configStale) {
        d->configStale = true;
        testUpdate();
    }
}

void KoFontChangeTracker::directoriesChanged(QString path)
{
    if (!d->filesChanged) {
        d->filesChanged = true;
        testUpdate();
    }
}

void KoFontChangeTracker::testUpdate()
{
    if (d->filesChanged && d->configStale) {
        emit (sigUpdateConfig());
    }
}
