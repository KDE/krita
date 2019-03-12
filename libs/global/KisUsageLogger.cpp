/*
 *  Copyright (c) 2019 Boudewijn Rempt <boud@valdyas.org>
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
#include "KisUsageLogger.h"

#include <QGlobalStatic>
#include <QDebug>
#include <QDateTime>
#include <QSysInfo>
#include <QStandardPaths>
#include <QFile>
#include <QDesktopWidget>
#include <QClipboard>
#include <QThread>
#include <QApplication>
#include <klocalizedstring.h>
#include <KritaVersionWrapper.h>


Q_GLOBAL_STATIC(KisUsageLogger, s_instance)

const QString KisUsageLogger::s_sectionHeader("================================================================================\n");

struct KisUsageLogger::Private {
    bool active {false};
    QFile logFile;
};

KisUsageLogger::KisUsageLogger()
    : d(new Private)
{
    d->logFile.setFileName(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/krita.log");

    rotateLog();
    d->logFile.open(QFile::Append);
}

KisUsageLogger::~KisUsageLogger()
{
    if (d->active) {
        close();
    }
}

void KisUsageLogger::initialize()
{
    s_instance->d->active = true;
}

void KisUsageLogger::close()
{
    log("Closing.");
    s_instance->d->active = false;
    s_instance->d->logFile.flush();
    s_instance->d->logFile.close();
}

void KisUsageLogger::log(const QString &message)
{
    if (!s_instance->d->active) return;
    if (!s_instance->d->logFile.isOpen()) return;

    s_instance->d->logFile.write(QDateTime::currentDateTime().toString(Qt::RFC2822Date).toUtf8());
    s_instance->d->logFile.write(": ");
    write(message);
}

void KisUsageLogger::write(const QString &message)
{
    if (!s_instance->d->active) return;
    if (!s_instance->d->logFile.isOpen()) return;

    s_instance->d->logFile.write(message.toUtf8());
    s_instance->d->logFile.write("\n");

    s_instance->d->logFile.flush();
}

void KisUsageLogger::writeHeader()
{
    Q_ASSERT(s_instance->d->logFile.isOpen());

    QString sessionHeader = QString("SESSION: %1. Executing %2\n\n")
            .arg(QDateTime::currentDateTime().toString(Qt::RFC2822Date))
            .arg(qApp->arguments().join(' '));

    QString disclaimer = i18n("WARNING: This file contains information about your system and the\n"
                              "images you have been working with.\n"
                              "\n"
                              "If you have problems with Krita, the Krita developers might ask\n"
                              "you to share this file with them. The information in this file is\n"
                              "not shared automatically with the Krita developers in any way. You\n"
                              "can disable logging to this file in Krita's Configure Krita Dialog.\n"
                              "\n"
                              "Please review the contents of this file before sharing this file with\n"
                              "anyone.\n\n");

    QString systemInfo;

    // NOTE: This is intentionally not translated!

    // Krita version info
    systemInfo.append("Krita\n");
    systemInfo.append("\n Version: ").append(KritaVersionWrapper::versionString(true));
    systemInfo.append("\n Languages: ").append(KLocalizedString::languages().join(", "));
    systemInfo.append("\n Hidpi: ").append(QCoreApplication::testAttribute(Qt::AA_EnableHighDpiScaling) ? "true" : "false");
    systemInfo.append("\n\n");

    systemInfo.append("Qt\n");
    systemInfo.append("\n  Version (compiled): ").append(QT_VERSION_STR);
    systemInfo.append("\n  Version (loaded): ").append(qVersion());
    systemInfo.append("\n\n");

    // OS information
    systemInfo.append("OS Information\n");
    systemInfo.append("\n  Build ABI: ").append(QSysInfo::buildAbi());
    systemInfo.append("\n  Build CPU: ").append(QSysInfo::buildCpuArchitecture());
    systemInfo.append("\n  CPU: ").append(QSysInfo::currentCpuArchitecture());
    systemInfo.append("\n  Kernel Type: ").append(QSysInfo::kernelType());
    systemInfo.append("\n  Kernel Version: ").append(QSysInfo::kernelVersion());
    systemInfo.append("\n  Pretty Productname: ").append(QSysInfo::prettyProductName());
    systemInfo.append("\n  Product Type: ").append(QSysInfo::productType());
    systemInfo.append("\n  Product Version: ").append(QSysInfo::productVersion());
    systemInfo.append("\n\n");

    s_instance->d->logFile.write(s_sectionHeader.toUtf8());
    s_instance->d->logFile.write(sessionHeader.toUtf8());
    s_instance->d->logFile.write(disclaimer.toUtf8());
    s_instance->d->logFile.write(systemInfo.toUtf8());


}

void KisUsageLogger::rotateLog()
{
    d->logFile.open(QFile::ReadOnly);
    QString log = QString::fromUtf8(d->logFile.readAll());
    int sectionCount = log.count(s_sectionHeader);
    int nextSectionIndex = log.indexOf(s_sectionHeader, s_sectionHeader.length());
    while(sectionCount >= s_maxLogs) {
        log = log.remove(0, log.indexOf(s_sectionHeader, nextSectionIndex));
        nextSectionIndex = log.indexOf(s_sectionHeader, s_sectionHeader.length());
        sectionCount = log.count(s_sectionHeader);
    }
    d->logFile.close();
    d->logFile.open(QFile::WriteOnly);
    d->logFile.write(log.toUtf8());
    d->logFile.close();
}

