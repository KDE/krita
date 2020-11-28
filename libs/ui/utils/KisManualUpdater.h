/*
 *  SPDX-FileCopyrightText: 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */


#ifndef KISMANUALUPDATER_H
#define KISMANUALUPDATER_H

#include <QScopedPointer>

#include <KisUpdaterBase.h>
#include <KisMultiFeedRSSModel.h>
#include "kritaui_export.h"


class KRITAUI_EXPORT KisManualUpdater : public KisUpdaterBase
{
    Q_OBJECT

public:
    KisManualUpdater();
    /**
     * @brief KisManualUpdater - constructor for testing
     * @param rssModel
     * @param currentVersion
     */
    explicit KisManualUpdater(MultiFeedRssModel* rssModel, QString& currentVersion);

    void checkForUpdate() override;

    // this updater can only check for updates
    /**
     * @brief the manual updater can only check for available versions
     * @return false
     */
    inline bool hasUpdateCapability() override { return false; }
    inline void doUpdate() override { return; }

public Q_SLOTS:
    void rssDataChanged();

private:
    bool availableVersionIsHigher(QString currentVersion, QString availableVersion);

    QScopedPointer<MultiFeedRssModel> m_rssModel;
    QString m_currentVersion;

    friend class KisManualUpdaterTest;
};

#endif // KISMANUALUPDATER_H
