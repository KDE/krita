/*
 * SPDX-FileCopyrightText: 2022 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#include "KisRecentFilesManager.h"

#include <QApplication>
#include <QDir>
#include <QFileInfo>
#include <QThread>
#include <QDebug>

#include <kconfig.h>
#include <kconfiggroup.h>

class KisRecentFilesManager::Private
{
    int m_maxItems {10};
    QVector<KisRecentFilesEntry> m_entries;

    friend class KisRecentFilesManager;

    bool containsUrl(const QUrl &url) const;
    int indexOfUrl(const QUrl &url) const;
}; /* class KisRecentFilesEntry::Private */

bool KisRecentFilesManager::Private::containsUrl(const QUrl &url) const
{
    return indexOfUrl(url) >= 0;
}

int KisRecentFilesManager::Private::indexOfUrl(const QUrl &url) const
{
    auto found = std::find_if(m_entries.constBegin(), m_entries.constEnd(), [url](const KisRecentFilesEntry &item) {
        return item.m_url == url;
    });
    if (found == m_entries.constEnd()) {
        return -1;
    } else {
        return found - m_entries.constBegin();
    }
}


KisRecentFilesManager::KisRecentFilesManager()
    : m_d(new Private)
{
}

KisRecentFilesManager::~KisRecentFilesManager()
{
    delete m_d;
}

KisRecentFilesManager *KisRecentFilesManager::instance()
{
    if (QThread::currentThread() != qApp->thread()) {
        qWarning() << "KisRecentFilesManager::instance() called from non-GUI thread!";
        return nullptr;
    }
    static KisRecentFilesManager s_instance;
    return &s_instance;
}

void KisRecentFilesManager::clear()
{
    m_d->m_entries.clear();
    emit listRenewed();
}

void KisRecentFilesManager::remove(const QUrl &url)
{
    int removeIndex = m_d->indexOfUrl(url);
    if (removeIndex >= 0) {
        m_d->m_entries.removeAt(removeIndex);
        emit fileRemoved(url);
    }
}

QVector<KisRecentFilesEntry> KisRecentFilesManager::recentFiles() const
{
    return m_d->m_entries;
}

QList<QUrl> KisRecentFilesManager::recentUrlsLatestFirst() const
{
    // switch order so last opened file is first
    QList<QUrl> sortedList;
    for (int i = m_d->m_entries.length() - 1; i >= 0; i--) {
        sortedList.append(m_d->m_entries[i].m_url);
    }

    return sortedList;
}

// The following file contains code copied and modified from LGPL-2.0-only
// source:
#include "KisRecentFilesManager_p.h"
