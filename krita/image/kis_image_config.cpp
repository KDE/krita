/*
 *  Copyright (c) 2010 Dmitry Kazakov <dimula73@gmail.com>
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

#include "kis_image_config.h"

#include <ksharedconfig.h>
#include <kglobal.h>
#include <kstandarddirs.h>

#include <KoConfig.h>

#include "kis_debug.h"
#include <QThread>
#include <QApplication>


KisImageConfig::KisImageConfig()
    : m_config(KGlobal::config()->group(""))
{
}

KisImageConfig::~KisImageConfig()
{
    if (qApp->thread() != QThread::currentThread()) {
        qDebug() << "WARNING: KisImageConfig: requested config synchronization from nonGUI thread! Skipping...";
        return;
    }

    m_config.sync();
}

bool KisImageConfig::enablePerfLog(bool requestDefault) const
{
    return !requestDefault ?
        m_config.readEntry("enablePerfLog", false) :false;
}

void KisImageConfig::setEnablePerfLog(bool value)
{
    m_config.writeEntry("enablePerfLog", value);
}

qreal KisImageConfig::transformMaskOffBoundsReadArea() const
{
    return m_config.readEntry("transformMaskOffBoundsReadArea", 0.5);
}

int KisImageConfig::updatePatchHeight() const
{
    return m_config.readEntry("updatePatchHeight", 512);
}

void KisImageConfig::setUpdatePatchHeight(int value)
{
    m_config.writeEntry("updatePatchHeight", value);
}

int KisImageConfig::updatePatchWidth() const
{
    return m_config.readEntry("updatePatchWidth", 512);
}

void KisImageConfig::setUpdatePatchWidth(int value)
{
    m_config.writeEntry("updatePatchWidth", value);
}

qreal KisImageConfig::maxCollectAlpha() const
{
    return m_config.readEntry("maxCollectAlpha", 2.5);
}

qreal KisImageConfig::maxMergeAlpha() const
{
    return m_config.readEntry("maxMergeAlpha", 1.);
}

qreal KisImageConfig::maxMergeCollectAlpha() const
{
    return m_config.readEntry("maxMergeCollectAlpha", 1.5);
}

qreal KisImageConfig::schedulerBalancingRatio() const
{
    /**
     * updates-queue-size / strokes-queue-size
     */
    return m_config.readEntry("schedulerBalancingRatio", 100.);
}

void KisImageConfig::setSchedulerBalancingRatio(qreal value)
{
    m_config.writeEntry("schedulerBalancingRatio", value);
}

int KisImageConfig::maxSwapSize(bool requestDefault) const
{
    return !requestDefault ?
        m_config.readEntry("maxSwapSize", 4096) : 4096; // in MiB
}

void KisImageConfig::setMaxSwapSize(int value)
{
    m_config.writeEntry("maxSwapSize", value);
}

int KisImageConfig::swapSlabSize() const
{
    return m_config.readEntry("swapSlabSize", 64); // in MiB
}

void KisImageConfig::setSwapSlabSize(int value)
{
    m_config.writeEntry("swapSlabSize", value);
}

int KisImageConfig::swapWindowSize() const
{
    return m_config.readEntry("swapWindowSize", 16); // in MiB
}

void KisImageConfig::setSwapWindowSize(int value)
{
    m_config.writeEntry("swapWindowSize", value);
}

int KisImageConfig::tilesHardLimit() const
{
    qreal hp = qreal(memoryHardLimitPercent()) / 100.0;
    qreal pp = qreal(memoryPoolLimitPercent()) / 100.0;

    return totalRAM() * hp * (1 - pp);
}

int KisImageConfig::tilesSoftLimit() const
{
    qreal sp = qreal(memorySoftLimitPercent()) / 100.0;

    return tilesHardLimit() * sp;
}

int KisImageConfig::poolLimit() const
{
    qreal hp = qreal(memoryHardLimitPercent()) / 100.0;
    qreal pp = qreal(memoryPoolLimitPercent()) / 100.0;

    return totalRAM() * hp * pp;
}

qreal KisImageConfig::memoryHardLimitPercent(bool requestDefault) const
{
    return !requestDefault ?
        m_config.readEntry("memoryHardLimitPercent", 50.) : 50.;
}

void KisImageConfig::setMemoryHardLimitPercent(qreal value)
{
    m_config.writeEntry("memoryHardLimitPercent", value);
}

qreal KisImageConfig::memorySoftLimitPercent(bool requestDefault) const
{
    return !requestDefault ?
        m_config.readEntry("memorySoftLimitPercent", 2.) : 2.;
}

void KisImageConfig::setMemorySoftLimitPercent(qreal value)
{
    m_config.writeEntry("memorySoftLimitPercent", value);
}

qreal KisImageConfig::memoryPoolLimitPercent(bool requestDefault) const
{
    return !requestDefault ?
        m_config.readEntry("memoryPoolLimitPercent", 2.) : 2.;
}

void KisImageConfig::setMemoryPoolLimitPercent(qreal value)
{
    m_config.writeEntry("memoryPoolLimitPercent", value);
}

QString KisImageConfig::swapDir(bool requestDefault)
{
    QString swap = KGlobal::dirs()->locateLocal("tmp", "krita");
    return !requestDefault ?
            m_config.readEntry("swaplocation", swap) : swap;
}

void KisImageConfig::setSwapDir(const QString &swapDir)
{
    m_config.writeEntry("swaplocation", swapDir);
}


#if defined Q_OS_LINUX
#include <sys/sysinfo.h>
#elif defined Q_OS_FREEBSD
#include <sys/sysctl.h>
#elif defined Q_OS_WIN
#include <windows.h>
#elif defined Q_OS_MAC64
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

#include <kdebug.h>

int KisImageConfig::totalRAM()
{
    // let's think that default memory size is 1000MiB
    int totalMemory = 1000; // MiB
    int error = 1;

#if defined Q_OS_LINUX
    struct sysinfo info;

    error = sysinfo(&info);
    if(!error) {
        totalMemory = info.totalram * info.mem_unit / (1UL << 20);
    }
#elif defined Q_OS_FREEBSD
    u_long physmem;
    int mib[] = {CTL_HW, HW_PHYSMEM};
    size_t len = sizeof(physmem);

    error = sysctl(mib, 2, &physmem, &len, NULL, 0);
    if(!error) {
        totalMemory = physmem >> 20;
    }
#elif defined Q_OS_WIN
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    error  = !GlobalMemoryStatusEx(&status);

    if (!error) {
        totalMemory = status.ullTotalPhys >> 20;
    }

    // For 32 bit windows, the total memory available is at max the 2GB per process memory limit.
#   if defined ENV32BIT
    totalMemory = qMin(totalMemory, 2000);
#   endif
#elif defined Q_OS_MAC64
    int mib[2] = { CTL_HW, HW_MEMSIZE };
    u_int namelen = sizeof(mib) / sizeof(mib[0]);
    uint64_t size;
    size_t len = sizeof(size);

    if (sysctl(mib, namelen, &size, &len, NULL, 0) > 0) {
        totalMemory = size;
        error = 0;
    }
#endif

    if(error) {
        kWarning() << "Cannot get the size of your RAM."
                   << "Using default value of 1GiB.";
    }

    return totalMemory;
}
