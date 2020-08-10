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


#include "KisManualUpdater.h"
#include <KisMultiFeedRSSModel.h>
#include <KritaVersionWrapper.h>

#include <QModelIndex>
#include <QRegularExpression>
#include <QRegularExpressionMatch>
#include <QVersionNumber>

#include <kis_debug.h>


KisManualUpdater::KisManualUpdater()
    : m_currentVersion(KritaVersionWrapper::versionString())
{
    m_rssModel.reset(new MultiFeedRssModel());
}

KisManualUpdater::KisManualUpdater(MultiFeedRssModel* rssModel, QString &currentVersion)
    : m_currentVersion(currentVersion)
{
    m_rssModel.reset(rssModel);
}

void KisManualUpdater::checkForUpdate()
{
    connect(m_rssModel.data(), SIGNAL(feedDataChanged()), this, SLOT(rssDataChanged()));
    m_rssModel->addFeed(QLatin1String("https://krita.org/en/feed/"));
}

void KisManualUpdater::rssDataChanged()
{
    // grab the latest release post and URL for reference later
    // if we need to update
    QString availableVersion;
    QString downloadLink;

    for (int i = 0; i < m_rssModel->rowCount(); i++) {
       const QModelIndex &idx = m_rssModel->index(i);

       if (idx.isValid()) {
           // only use official release announcements to get version number
           if ( idx.data(KisRssReader::RssRoles::CategoryRole).toString() !=  "Official Release") {
               continue;
           }

           QString linkTitle = idx.data(KisRssReader::RssRoles::TitleRole).toString();

           // regex to capture version number
           QRegularExpression versionRegex("\\d\\.\\d\\.?\\d?\\.?\\d");
           QRegularExpressionMatch matched = versionRegex.match(linkTitle);

           // only take the top match for release version since that is the newest
           if (matched.hasMatch()) {
               availableVersion = matched.captured(0);
               downloadLink = idx.data(KisRssReader::RssRoles::LinkRole).toString();
               break;
           }
       }
    }

    UpdaterStatus::StatusID status;

    if (availableVersionIsHigher(m_currentVersion, availableVersion)) {
        status = UpdaterStatus::StatusID::UPDATE_AVAILABLE;
    } else {
        status = UpdaterStatus::StatusID::UPTODATE;
    }

    m_updaterStatus.setStatus(status);
    m_updaterStatus.setAvailableVersion(availableVersion);
    m_updaterStatus.setDownloadLink(downloadLink);

    emit sigUpdateCheckStateChange(m_updaterStatus);
}

bool KisManualUpdater::availableVersionIsHigher(QString currentVersion, QString availableVersion)
{
    if (currentVersion.isEmpty() || availableVersion.isEmpty()) {
        return false;
    }

    int currentSuffixIndex {5};
    int availableSuffixIndex {5};

    QVersionNumber currentVersionNumber = QVersionNumber::fromString(currentVersion, &currentSuffixIndex);
    QVersionNumber availableVersionNumber = QVersionNumber::fromString(availableVersion, &availableSuffixIndex);

    QString currentSuffix = currentVersion.mid(currentSuffixIndex);
    QString availableSuffix = availableVersion.mid(availableSuffixIndex);

    if (currentVersionNumber.normalized() == availableVersionNumber.normalized()) {
        if (!currentSuffix.isEmpty() && availableSuffix.isEmpty()) {
            return true;
        } else {
            return false;
        }
    } else {
        return (currentVersionNumber.normalized() < availableVersionNumber.normalized());
    }
}
