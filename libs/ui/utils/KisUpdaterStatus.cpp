/*
 *  SPDX-FileCopyrightText: 2019 Anna Medonosova <anna.medonosova@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
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
