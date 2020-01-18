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

#include <QScreen>
#include <QGlobalStatic>
#include <QDebug>
#include <QDateTime>
#include <QSysInfo>
#include <QStandardPaths>
#include <QFile>
#include <QFileInfo>
#include <QDesktopWidget>
#include <QClipboard>
#include <QThread>
#include <QApplication>
#include <klocalizedstring.h>
#include <KritaVersionWrapper.h>
#include <QGuiApplication>

Q_GLOBAL_STATIC(KisUsageLogger, s_instance)

const QString KisUsageLogger::s_sectionHeader("================================================================================\n");

struct KisUsageLogger::Private {
    bool active {false};
    QFile logFile;
    QFile sysInfoFile;
};

KisUsageLogger::KisUsageLogger()
    : d(new Private)
{
    d->logFile.setFileName(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/krita.log");
    d->sysInfoFile.setFileName(QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation) + "/krita-sysinfo.log");

    rotateLog();

    d->logFile.open(QFile::Append | QFile::Text);
    d->sysInfoFile.open(QFile::WriteOnly | QFile::Text);
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

    QString systemInfo = basicSystemInfo();
    s_instance->d->sysInfoFile.write(systemInfo.toUtf8());
}

QString KisUsageLogger::basicSystemInfo()
{
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

    return systemInfo;
}

void KisUsageLogger::close()
{
    log("CLOSING SESSION");
    s_instance->d->active = false;
    s_instance->d->logFile.flush();
    s_instance->d->logFile.close();
    s_instance->d->sysInfoFile.flush();
    s_instance->d->sysInfoFile.close();
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

void KisUsageLogger::writeSysInfo(const QString &message)
{
    if (!s_instance->d->active) return;
    if (!s_instance->d->sysInfoFile.isOpen()) return;

    s_instance->d->sysInfoFile.write(message.toUtf8());
    s_instance->d->sysInfoFile.write("\n");

    s_instance->d->sysInfoFile.flush();

}


void KisUsageLogger::writeHeader()
{
    Q_ASSERT(s_instance->d->sysInfoFile.isOpen());
    s_instance->d->logFile.write(s_sectionHeader.toUtf8());

    QString sessionHeader = QString("SESSION: %1. Executing %2\n\n")
            .arg(QDateTime::currentDateTime().toString(Qt::RFC2822Date))
            .arg(qApp->arguments().join(' '));

    s_instance->d->logFile.write(sessionHeader.toUtf8());

    QString KritaAndQtVersion;
    KritaAndQtVersion.append("Krita Version: ").append(KritaVersionWrapper::versionString(true))
            .append(", Qt version compiled: ").append(QT_VERSION_STR)
            .append(", loaded: ").append(qVersion())
            .append(". Process ID: ")
            .append(QString::number(qApp->applicationPid())).append("\n");

    KritaAndQtVersion.append("-- -- -- -- -- -- -- --\n");
    s_instance->d->logFile.write(KritaAndQtVersion.toUtf8());
    s_instance->d->logFile.flush();
}

QString KisUsageLogger::screenInformation()
{
    QList<QScreen*> screens = qApp->screens();

    QString info;
    info.append("Display Information");
    info.append("\nNumber of screens: ").append(QString::number(screens.size()));

    for (int i = 0; i < screens.size(); ++i ) {
        QScreen *screen = screens[i];
        info.append("\n\tScreen: ").append(QString::number(i));
        info.append("\n\t\tName: ").append(screen->name());
        info.append("\n\t\tDepth: ").append(QString::number(screen->depth()));
        info.append("\n\t\tScale: ").append(QString::number(screen->devicePixelRatio()));
        info.append("\n\t\tResolution in pixels: ").append(QString::number(screen->geometry().width()))
                .append("x")
                .append(QString::number(screen->geometry().height()));
        info.append("\n\t\tManufacturer: ").append(screen->manufacturer());
        info.append("\n\t\tModel: ").append(screen->model());
        info.append("\n\t\tRefresh Rate: ").append(QString::number(screen->refreshRate()));
    }
    info.append("\n");
    return info;
}

void KisUsageLogger::rotateLog()
{
    if (d->logFile.exists()) {
        {
            // Check for CLOSING SESSION
            d->logFile.open(QFile::ReadOnly);
            QString log = QString::fromUtf8(d->logFile.readAll());
            if (!log.split(s_sectionHeader).last().contains("CLOSING SESSION")) {
                log.append("\nKRITA DID NOT CLOSE CORRECTLY\n");
                QString crashLog = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation) + QStringLiteral("/kritacrash.log");
                if (QFileInfo(crashLog).exists()) {
                    QFile f(crashLog);
                    f.open(QFile::ReadOnly);
                    QString crashes = QString::fromUtf8(f.readAll());
                    f.close();

                    QStringList crashlist = crashes.split("-------------------");
                    log.append(QString("\nThere were %1 crashes in total in the crash log.\n").arg(crashlist.size()));

                    if (crashes.size() > 0) {
                        log.append(crashlist.last());
                    }
                }
                d->logFile.close();
                d->logFile.open(QFile::WriteOnly);
                d->logFile.write(log.toUtf8());
            }
            d->logFile.flush();
            d->logFile.close();
        }

        {
            // Rotate
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
            d->logFile.flush();
            d->logFile.close();
        }


    }
}

