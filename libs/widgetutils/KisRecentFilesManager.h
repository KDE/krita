/*
 * SPDX-FileCopyrightText: 2022 Alvin Wong <alvin@alvinhc.com>
 *
 * SPDX-License-Identifier: GPL-3.0-or-later
 */

#ifndef KIS_RECENT_FILES_MANAGER_H
#define KIS_RECENT_FILES_MANAGER_H

#include <QObject>
#include <QString>
#include <QUrl>
#include <QVector>

#include <kritawidgetutils_export.h>

class KConfigGroup;

struct KRITAWIDGETUTILS_EXPORT KisRecentFilesEntry
{
    QUrl m_url;
    QString m_displayName;
}; /* struct KisRecentFilesEntry */

class KRITAWIDGETUTILS_EXPORT KisRecentFilesManager : public QObject
{
    Q_OBJECT

    class Private;
    Private *m_d;

    KisRecentFilesManager();
    ~KisRecentFilesManager();

    Q_DISABLE_COPY(KisRecentFilesManager)

public:
    static KisRecentFilesManager *instance();

    void clear();

    void add(const QUrl &url);
    void remove(const QUrl &url);

    QVector<KisRecentFilesEntry> recentFiles() const;
    QList<QUrl> recentUrlsLatestFirst() const;

private:
    void loadEntries(const KConfigGroup &config);
    void saveEntries(const KConfigGroup &config);

Q_SIGNALS:
    void fileAdded(const QUrl &url);
    void fileRemoved(const QUrl &url);
    void listRenewed();
}; /* class KisRecentFileRegistry */

#endif /* KIS_RECENT_FILES_MANAGER_H */
