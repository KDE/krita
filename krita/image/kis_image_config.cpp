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
#include <KoConfig.h>

KisImageConfig::KisImageConfig()
    : m_config(KGlobal::config()->group(""))
{
}

KisImageConfig::~KisImageConfig()
{
    m_config.sync();
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

int KisImageConfig::maxSwapSize() const
{
    return m_config.readEntry("maxSwapSize", 4096); // in MiB
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
    return totalRAM() * (memoryHardLimitPercent() - memoryPoolLimitPercent()) / 100;
}

int KisImageConfig::tilesSoftLimit() const
{
    return totalRAM() * (memorySoftLimitPercent() - memoryPoolLimitPercent()) / 100;
}

int KisImageConfig::poolLimit() const
{
    return totalRAM() * memoryPoolLimitPercent() / 100;
}

qreal KisImageConfig::memoryHardLimitPercent() const
{
    return m_config.readEntry("memoryHardLimitPercent", 50.);
}

void KisImageConfig::setMemoryHardLimitPercent(qreal value)
{
    m_config.writeEntry("memoryHardLimitPercent", value);
}

qreal KisImageConfig::memorySoftLimitPercent() const
{
    return m_config.readEntry("memorySoftLimitPercent", 25.);
}

void KisImageConfig::setMemorySoftLimitPercent(qreal value)
{
    m_config.writeEntry("memorySoftLimitPercent", value);
}

qreal KisImageConfig::memoryPoolLimitPercent() const
{
    return m_config.readEntry("memoryPoolLimitPercent", 20.);
}

void KisImageConfig::setMemoryPoolLimitPercent(qreal value)
{
    m_config.writeEntry("memoryPoolLimitPercent", value);
}

QString KisImageConfig::swapDir()
{
    return m_config.readEntry("swaplocation", QString());
}

int KisImageConfig::numberOfOnionSkins() const
{
    return m_config.readEntry("numberOfOnionSkins", 1);
}

void KisImageConfig::setNumberOfOnionSkins(int value)
{
    m_config.writeEntry("numberOfOnionSkins", value);
}

int KisImageConfig::onionSkinTintFactor() const
{
    return m_config.readEntry("onionSkinTintFactor", 0);
}

void KisImageConfig::setOnionSkinTintFactor(int value)
{
    m_config.writeEntry("onionSkinTintFactor", value);
}

int KisImageConfig::onionSkinOpacity(int offset) const
{
    return m_config.readEntry("onionSkinOpacity_" + QString::number(offset), 128);
}

void KisImageConfig::setOnionSkinOpacity(int offset, int value)
{
    m_config.writeEntry("onionSkinOpacity_" + QString::number(offset), value);
}

QColor KisImageConfig::onionSkinTintColorBackward() const
{
    return m_config.readEntry("onionSkinTintColorBackward", QColor(Qt::red));
}

void KisImageConfig::setOnionSkinTintColorBackward(const QColor &value)
{
    m_config.writeEntry("onionSkinTintColorBackward", value);
}

QColor KisImageConfig::onionSkinTintColorForward() const
{
    return m_config.readEntry("oninSkinTintColorForward", QColor(Qt::green));
}

void KisImageConfig::setOnionSkinTintColorForward(const QColor &value)
{
    m_config.writeEntry("oninSkinTintColorForward", value);
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
