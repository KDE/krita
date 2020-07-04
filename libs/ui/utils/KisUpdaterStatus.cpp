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


#include "KisUpdaterStatus.h"
#include <kis_debug.h>

KisUpdaterStatus::KisUpdaterStatus()
{

}

KisUpdaterStatus::KisUpdaterStatus(const KisUpdaterStatus& rhs)
    : QObject(0)
    , m_status(rhs.m_status)
    , m_availableVersion(rhs.m_availableVersion)
    , m_downloadLink(rhs.m_downloadLink)
    , m_updaterOutput(rhs.m_updaterOutput)
    , m_details(rhs.m_details)
{

}

KisUpdaterStatus::~KisUpdaterStatus()
{

}

UpdaterStatus::StatusID KisUpdaterStatus::status() {
    return m_status;
}

QString KisUpdaterStatus::availableVersion() {
    return m_availableVersion;
}

QString KisUpdaterStatus::downloadLink() {
    return m_downloadLink;
}

QString KisUpdaterStatus::updaterOutput() {
    return m_updaterOutput;
}

QString KisUpdaterStatus::details()
{
    return m_details;
}

void KisUpdaterStatus::setStatus(const UpdaterStatus::StatusID& status)
{
    m_status = status;
}

void KisUpdaterStatus::setAvailableVersion(const QString& availableVersion)
{
    m_availableVersion = availableVersion;
}

void KisUpdaterStatus::setDownloadLink(const QString& downloadLink)
{
    m_downloadLink = downloadLink;
}

void KisUpdaterStatus::setUpdaterOutput(const QString& updaterOutput)
{
    m_updaterOutput = updaterOutput;
}

void KisUpdaterStatus::setDetails(const QString& details)
{
    m_details = details;
}

KisUpdaterStatus& KisUpdaterStatus::operator=(KisUpdaterStatus& secondArg)
{
    m_status = secondArg.status();
    m_availableVersion = secondArg.availableVersion();
    m_downloadLink = secondArg.downloadLink();
    m_updaterOutput = secondArg.updaterOutput();
    m_details = secondArg.details();

    return *this;
}

bool KisUpdaterStatus::operator==(KisUpdaterStatus& secondArg)
{
    if (m_status == secondArg.status()) {
        return true;
    } else {
        return false;
    }
}
