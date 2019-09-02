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

#include <KoConfig.h>
#include <KoColorProfile.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorConversionTransformation.h>

#include "kis_debug.h"

#include <QThread>
#include <QApplication>
#include <QColor>
#include <QDir>

#include "kis_global.h"
#include <cmath>

#ifdef Q_OS_MACOS
#include <errno.h>
#endif

KisImageConfig::KisImageConfig(bool readOnly)
    : m_config(KSharedConfig::openConfig()->group(QString()))
    , m_readOnly(readOnly)
{
    if (!readOnly) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(qApp->thread() == QThread::currentThread());
    }
#ifdef Q_OS_MACOS
    // clear /var/folders/ swap path set by old broken Krita swap implementation in order to use new default swap dir.
    QString swap = m_config.readEntry("swaplocation", "");
    if (swap.startsWith("/var/folders/")) {
        m_config.deleteEntry("swaplocation");
    }
#endif
}

KisImageConfig::~KisImageConfig()
{
    if (m_readOnly) return;

    if (qApp->thread() != QThread::currentThread()) {
        dbgKrita << "KisImageConfig: requested config synchronization from nonGUI thread! Called from" << kisBacktrace();
        return;
    }

    m_config.sync();
}

bool KisImageConfig::enableProgressReporting(bool requestDefault) const
{
    return !requestDefault ?
        m_config.readEntry("enableProgressReporting", true) : true;
}

void KisImageConfig::setEnableProgressReporting(bool value)
{
    m_config.writeEntry("enableProgressReporting", value);
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
        m_config.readEntry("memoryPoolLimitPercent", 0.0) : 0.0;
}

void KisImageConfig::setMemoryPoolLimitPercent(qreal value)
{
    m_config.writeEntry("memoryPoolLimitPercent", value);
}

QString KisImageConfig::safelyGetWritableTempLocation(const QString &suffix, const QString &configKey, bool requestDefault) const
{
#ifdef Q_OS_MACOS
    // On OSX, QDir::tempPath() gives us a folder we cannot reply upon (usually
    // something like /var/folders/.../...) and that will have vanished when we
    // try to create the tmp file in KisMemoryWindow::KisMemoryWindow using
    // swapFileTemplate. thus, we just pick the home folder if swapDir does not
    // tell us otherwise.

    // the other option here would be to use a "garbled name" temp file (i.e. no name
    // KRITA_SWAP_FILE_XXXXXX) in an obscure /var/folders place, which is not
    // nice to the user. having a clearly named swap file in the home folder is
    // much nicer to Krita's users.

    // furthermore, this is just a default and swapDir can always be configured
    // to another location.

    QString swap = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + QDir::separator() + suffix;
#else
    Q_UNUSED(suffix);
    QString swap = QDir::tempPath();
#endif
    if (requestDefault) {
       return swap;
    }
    const QString configuredSwap = m_config.readEntry(configKey, swap);
    if (!configuredSwap.isEmpty()) {
        swap = configuredSwap;
    }

    // Gosh, everything fails, nothing is writable, swap to the homedir
    if (!QFileInfo(swap).isWritable()) {
        swap = QDir::homePath();
        Q_ASSERT(QFileInfo(swap).isWritable());
    }


    return swap;
}


QString KisImageConfig::swapDir(bool requestDefault)
{
    return safelyGetWritableTempLocation("swap", "swaplocation", requestDefault);
}

void KisImageConfig::setSwapDir(const QString &swapDir)
{
    m_config.writeEntry("swaplocation", swapDir);
}

int KisImageConfig::numberOfOnionSkins() const
{
    return m_config.readEntry("numberOfOnionSkins", 10);
}

void KisImageConfig::setNumberOfOnionSkins(int value)
{
    m_config.writeEntry("numberOfOnionSkins", value);
}

int KisImageConfig::onionSkinTintFactor() const
{
    return m_config.readEntry("onionSkinTintFactor", 192);
}

void KisImageConfig::setOnionSkinTintFactor(int value)
{
    m_config.writeEntry("onionSkinTintFactor", value);
}

int KisImageConfig::onionSkinOpacity(int offset) const
{
    int value = m_config.readEntry("onionSkinOpacity_" + QString::number(offset), -1);

    if (value < 0) {
        const int num = numberOfOnionSkins();
        const qreal dx = qreal(qAbs(offset)) / num;
        value = 0.7 * exp(-pow2(dx) / 0.5) * 255;
    }

    return value;
}

void KisImageConfig::setOnionSkinOpacity(int offset, int value)
{
    m_config.writeEntry("onionSkinOpacity_" + QString::number(offset), value);
}

bool KisImageConfig::onionSkinState(int offset) const
{
    bool enableByDefault = (qAbs(offset) <= 2);
    return m_config.readEntry("onionSkinState_" + QString::number(offset), enableByDefault);
}

void KisImageConfig::setOnionSkinState(int offset, bool value)
{
    m_config.writeEntry("onionSkinState_" + QString::number(offset), value);
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

bool KisImageConfig::lazyFrameCreationEnabled(bool requestDefault) const
{
    return !requestDefault ?
        m_config.readEntry("lazyFrameCreationEnabled", true) : true;
}

void KisImageConfig::setLazyFrameCreationEnabled(bool value)
{
    m_config.writeEntry("lazyFrameCreationEnabled", value);
}


#if defined Q_OS_LINUX
#include <sys/sysinfo.h>
#elif defined Q_OS_FREEBSD || defined Q_OS_NETBSD || defined Q_OS_OPENBSD
#include <sys/sysctl.h>
#elif defined Q_OS_WIN
#include <windows.h>
#elif defined Q_OS_MACOS
#include <sys/types.h>
#include <sys/sysctl.h>
#endif

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
#elif defined Q_OS_FREEBSD || defined Q_OS_NETBSD || defined Q_OS_OPENBSD
    u_long physmem;
#   if defined HW_PHYSMEM64 // NetBSD only
    int mib[] = {CTL_HW, HW_PHYSMEM64};
#   else
    int mib[] = {CTL_HW, HW_PHYSMEM};
#   endif
    size_t len = sizeof(physmem);

    error = sysctl(mib, 2, &physmem, &len, 0, 0);
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
#elif defined Q_OS_MACOS
    int mib[2] = { CTL_HW, HW_MEMSIZE };
    u_int namelen = sizeof(mib) / sizeof(mib[0]);
    uint64_t size;
    size_t len = sizeof(size);

    errno = 0;
    if (sysctl(mib, namelen, &size, &len, 0, 0) >= 0) {
        totalMemory = size >> 20;
        error = 0;
    }
    else {
        dbgKrita << "sysctl(\"hw.memsize\") raised error" << strerror(errno);
    }
#endif

    if (error) {
        warnKrita << "Cannot get the size of your RAM. Using 1 GiB by default.";
    }

    return totalMemory;
}

bool KisImageConfig::showAdditionalOnionSkinsSettings(bool requestDefault) const
{
    return !requestDefault ?
        m_config.readEntry("showAdditionalOnionSkinsSettings", true) : true;
}

void KisImageConfig::setShowAdditionalOnionSkinsSettings(bool value)
{
    m_config.writeEntry("showAdditionalOnionSkinsSettings", value);
}

int KisImageConfig::defaultFrameColorLabel() const
{
    return m_config.readEntry("defaultFrameColorLabel", 0);
}

void KisImageConfig::setDefaultFrameColorLabel(int label)
{
    m_config.writeEntry("defaultFrameColorLabel", label);
}

KisProofingConfigurationSP KisImageConfig::defaultProofingconfiguration()
{
    KisProofingConfiguration *proofingConfig= new KisProofingConfiguration();
    proofingConfig->proofingProfile = m_config.readEntry("defaultProofingProfileName", "Chemical proof");
    proofingConfig->proofingModel = m_config.readEntry("defaultProofingProfileModel", "CMYKA");
    proofingConfig->proofingDepth = m_config.readEntry("defaultProofingProfileDepth", "U8");
    proofingConfig->intent = (KoColorConversionTransformation::Intent)m_config.readEntry("defaultProofingProfileIntent", 3);
    if (m_config.readEntry("defaultProofingBlackpointCompensation", true)) {
        proofingConfig->conversionFlags  |= KoColorConversionTransformation::ConversionFlag::BlackpointCompensation;
    } else {
        proofingConfig->conversionFlags  = proofingConfig->conversionFlags & ~KoColorConversionTransformation::ConversionFlag::BlackpointCompensation;
    }
    QColor def(Qt::green);
    m_config.readEntry("defaultProofingGamutwarning", def);
    KoColor col(KoColorSpaceRegistry::instance()->rgb8());
    col.fromQColor(def);
    col.setOpacity(1.0);
    proofingConfig->warningColor = col;
    proofingConfig->adaptationState = (double)m_config.readEntry("defaultProofingAdaptationState", 1.0);
    return toQShared(proofingConfig);
}

void KisImageConfig::setDefaultProofingConfig(const KoColorSpace *proofingSpace, int proofingIntent, bool blackPointCompensation, KoColor warningColor, double adaptationState)
{
    m_config.writeEntry("defaultProofingProfileName", proofingSpace->profile()->name());
    m_config.writeEntry("defaultProofingProfileModel", proofingSpace->colorModelId().id());
    m_config.writeEntry("defaultProofingProfileDepth", proofingSpace->colorDepthId().id());
    m_config.writeEntry("defaultProofingProfileIntent", proofingIntent);
    m_config.writeEntry("defaultProofingBlackpointCompensation", blackPointCompensation);
    QColor c;
    c = warningColor.toQColor();
    m_config.writeEntry("defaultProofingGamutwarning", c);
    m_config.writeEntry("defaultProofingAdaptationState",adaptationState);
}

bool KisImageConfig::useLodForColorizeMask(bool requestDefault) const
{
    return !requestDefault ?
        m_config.readEntry("useLodForColorizeMask", false) : false;
}

void KisImageConfig::setUseLodForColorizeMask(bool value)
{
    m_config.writeEntry("useLodForColorizeMask", value);
}

int KisImageConfig::maxNumberOfThreads(bool defaultValue) const
{
    return (defaultValue ? QThread::idealThreadCount() : m_config.readEntry("maxNumberOfThreads", QThread::idealThreadCount()));
}

void KisImageConfig::setMaxNumberOfThreads(int value)
{
    if (value == QThread::idealThreadCount()) {
        m_config.deleteEntry("maxNumberOfThreads");
    } else {
        m_config.writeEntry("maxNumberOfThreads", value);
    }
}

int KisImageConfig::frameRenderingClones(bool defaultValue) const
{
    const int defaultClonesCount = qMax(1, maxNumberOfThreads(defaultValue) / 2);
    return defaultValue ? defaultClonesCount : m_config.readEntry("frameRenderingClones", defaultClonesCount);
}

void KisImageConfig::setFrameRenderingClones(int value)
{
    m_config.writeEntry("frameRenderingClones", value);
}

int KisImageConfig::fpsLimit(bool defaultValue) const
{
    int limit = defaultValue ? 100 : m_config.readEntry("fpsLimit", 100);
    return limit > 0 ? limit : 1;
}

void KisImageConfig::setFpsLimit(int value)
{
    m_config.writeEntry("fpsLimit", value);
}

bool KisImageConfig::useOnDiskAnimationCacheSwapping(bool defaultValue) const
{
    return defaultValue ? true : m_config.readEntry("useOnDiskAnimationCacheSwapping", true);
}

void KisImageConfig::setUseOnDiskAnimationCacheSwapping(bool value)
{
    m_config.writeEntry("useOnDiskAnimationCacheSwapping", value);
}

QString KisImageConfig::animationCacheDir(bool defaultValue) const
{
    return safelyGetWritableTempLocation("animation_cache", "animationCacheDir", defaultValue);
}

void KisImageConfig::setAnimationCacheDir(const QString &value)
{
    m_config.writeEntry("animationCacheDir", value);
}

bool KisImageConfig::useAnimationCacheFrameSizeLimit(bool defaultValue) const
{
    return defaultValue ? true : m_config.readEntry("useAnimationCacheFrameSizeLimit", true);
}

void KisImageConfig::setUseAnimationCacheFrameSizeLimit(bool value)
{
    m_config.writeEntry("useAnimationCacheFrameSizeLimit", value);
}

int KisImageConfig::animationCacheFrameSizeLimit(bool defaultValue) const
{
    return defaultValue ? 2500 : m_config.readEntry("animationCacheFrameSizeLimit", 2500);
}

void KisImageConfig::setAnimationCacheFrameSizeLimit(int value)
{
    m_config.writeEntry("animationCacheFrameSizeLimit", value);
}

bool KisImageConfig::useAnimationCacheRegionOfInterest(bool defaultValue) const
{
    return defaultValue ? true : m_config.readEntry("useAnimationCacheRegionOfInterest", true);
}

void KisImageConfig::setUseAnimationCacheRegionOfInterest(bool value)
{
    m_config.writeEntry("useAnimationCacheRegionOfInterest", value);
}

qreal KisImageConfig::animationCacheRegionOfInterestMargin(bool defaultValue) const
{
    return defaultValue ? 0.25 : m_config.readEntry("animationCacheRegionOfInterestMargin", 0.25);
}

void KisImageConfig::setAnimationCacheRegionOfInterestMargin(qreal value)
{
    m_config.writeEntry("animationCacheRegionOfInterestMargin", value);
}

QColor KisImageConfig::selectionOverlayMaskColor(bool defaultValue) const
{
    QColor def(255, 0, 0, 128);
    return (defaultValue ? def : m_config.readEntry("selectionOverlayMaskColor", def));
}

void KisImageConfig::setSelectionOverlayMaskColor(const QColor &color)
{
    m_config.writeEntry("selectionOverlayMaskColor", color);
}
