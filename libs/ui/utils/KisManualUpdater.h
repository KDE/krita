/*
 *  Copyright (c) 2019 Anna Medonosova <anna.medonosova@gmail.com>
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
