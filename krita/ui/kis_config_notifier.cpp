/*
 *  Copyright (c) 2007 Adrian Page <adrian@pagenet.plus.com>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */
#include "kis_config_notifier.h"

#include <QTimer>
#include <QFileInfo>
#include <QFileSystemWatcher>

#include <kglobal.h>
#include <ksharedconfig.h>
#include <kstandarddirs.h>

#include <kis_debug.h>
#include <kis_config.h>
#include <kis_signal_compressor.h>


struct KisConfigNotifier::Private
{
    Private()
        : fileChangedSignalCompressor(1000 /* ms */, KisSignalCompressor::POSTPONE)
    {
    }

    QFileSystemWatcher fileWatcher;
    KisSignalCompressor fileChangedSignalCompressor;
};

KisConfigNotifier::KisConfigNotifier()
    : m_d(new Private())
{
    /**
     * KConfig can either edit the config file of remove it completely
     * and create again, that is why pure watching on a file will not
     * work in our case. We watch the directory containing the file and
     * on every change try to re-add a file for watching. Given that our
     * signals are compressed, multiple sequential updates cannot
     * frighten us
     */

    m_d->fileWatcher.addPath(QFileInfo(configFileName()).absolutePath());

    configFileDirectoryChanged();

    connect(&m_d->fileWatcher, SIGNAL(fileChanged(QString)),
            &m_d->fileChangedSignalCompressor, SLOT(start()));

    connect(&m_d->fileChangedSignalCompressor, SIGNAL(timeout()),
            SLOT(fileChangedCompressed()));

    connect(&m_d->fileWatcher, SIGNAL(directoryChanged(const QString&)),
            SLOT(configFileDirectoryChanged()));
}

KisConfigNotifier::~KisConfigNotifier()
{
    dbgRegistry << "deleting KisConfigNotifier";
}

KisConfigNotifier *KisConfigNotifier::instance()
{
    K_GLOBAL_STATIC(KisConfigNotifier, s_instance);
    return s_instance;
}

void KisConfigNotifier::notifyConfigChanged(void)
{
    emit configChanged();
}

inline QString KisConfigNotifier::configFileName() const
{
    return KStandardDirs::locateLocal("config", KGlobal::config()->name());
}

void KisConfigNotifier::configFileDirectoryChanged()
{
    if (m_d->fileWatcher.files().isEmpty()) {
        m_d->fileWatcher.addPath(configFileName());
        m_d->fileChangedSignalCompressor.start();
    }
}

void KisConfigNotifier::fileChangedCompressed()
{
    KGlobal::config()->reparseConfiguration();
    notifyConfigChanged();
}

void KisConfigNotifier::forceNotifyOtherInstances()
{
    KisConfig cfg;
    cfg.updateModificationSeqNo();
}

#include "kis_config_notifier.moc"

