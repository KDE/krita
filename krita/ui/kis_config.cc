/*
 *  Copyright (c) 2002 Patrick Julien <freak@codepimps.org>
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
#include <limits.h>

#include <kglobalsettings.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kdebug.h>
#include <config.h>

#include LCMS_HEADER

#include "kis_global.h"
#include "kis_config.h"

namespace {
    const Q_INT32 IMG_WIDTH_MAX = USHRT_MAX;
    const Q_INT32 IMG_HEIGHT_MAX = USHRT_MAX;
    const Q_INT32 IMG_DEFAULT_WIDTH = 512;
    const Q_INT32 IMG_DEFAULT_HEIGHT = 512;
    const enumCursorStyle DEFAULT_CURSOR_STYLE = CURSOR_STYLE_OUTLINE;
    const Q_INT32 DEFAULT_MAX_THREADS = 4;
    const Q_INT32 DEFAULT_MAX_TILES_MEM = 500; // 8192 kilobytes given 64x64 tiles with 32bpp
    const Q_INT32 DEFAULT_SWAPPINESS = 100;
    const Q_INT32 DEFAULT_PRESSURE_CORRECTION = 50;
    const Q_INT32 DEFAULT_DOCKABILITY = 0;
}

KisConfig::KisConfig()
{

    m_cfg = KGlobal::config();
    if (!m_cfg) {
        // Allow unit tests to test parts of the code without having to run the
        // full application.
        m_cfg = new KConfig();
    }
    m_cfg->setGroup("");
}

KisConfig::~KisConfig()
{
    m_cfg -> sync();
}

Q_INT32 KisConfig::maxImgWidth() const
{
    return m_cfg -> readNumEntry("imgWidthMax", IMG_WIDTH_MAX);
}

Q_INT32 KisConfig::defImgWidth() const
{
    return m_cfg -> readNumEntry("imgWidthDef", IMG_DEFAULT_WIDTH);
}

Q_INT32 KisConfig::maxImgHeight() const
{
    return m_cfg -> readNumEntry("imgHeightMax", IMG_HEIGHT_MAX);
}

Q_INT32 KisConfig::defImgHeight() const
{
    return m_cfg -> readNumEntry("imgHeightDef", IMG_DEFAULT_HEIGHT);
}

void KisConfig::defImgWidth(Q_INT32 width)
{
    m_cfg -> writeEntry("imgWidthDef", width);
}

void KisConfig::defImgHeight(Q_INT32 height)
{
    m_cfg -> writeEntry("imgHeightDef", height);
}

enumCursorStyle KisConfig::cursorStyle() const
{
    return (enumCursorStyle) m_cfg -> readNumEntry("cursorStyleDef", DEFAULT_CURSOR_STYLE);
}

enumCursorStyle KisConfig::getDefaultCursorStyle() const
{
    return DEFAULT_CURSOR_STYLE;
}

void KisConfig::setCursorStyle(enumCursorStyle style)
{
    m_cfg -> writeEntry("cursorStyleDef", style);
}


QString KisConfig::monitorProfile() const
{
//    kdDebug () << "Profile: " << m_cfg -> readEntry("monitorProfile", "None") << "\n";
    return m_cfg -> readEntry("monitorProfile", "None");
}

void KisConfig::setMonitorProfile(QString monitorProfile)
{
    m_cfg -> writeEntry("monitorProfile", monitorProfile);
}


QString KisConfig::workingColorSpace() const
{
    return m_cfg -> readEntry("workingColorSpace", "RGBA");
}

void KisConfig::setWorkingColorSpace(QString workingColorSpace)
{
    m_cfg -> writeEntry(workingColorSpace, workingColorSpace);
}


QString KisConfig::importProfile() const
{
    return m_cfg -> readEntry("importProfile", "None");
}

void KisConfig::setImportProfile(QString importProfile)
{
    m_cfg -> writeEntry("importProfile", importProfile);
}


QString KisConfig::printerColorSpace() const
{
    return m_cfg -> readEntry("printerColorSpace", "CMYK");
}

void KisConfig::setPrinterColorSpace(QString printerColorSpace)
{
    m_cfg -> writeEntry("printerColorSpace", printerColorSpace);
}


QString KisConfig::printerProfile() const
{
    return m_cfg -> readEntry("printerProfile", "None");
}

void KisConfig::setPrinterProfile(QString printerProfile)
{
    m_cfg -> writeEntry("printerProfile", printerProfile);
}


bool KisConfig::useBlackPointCompensation() const
{
    return m_cfg -> readBoolEntry("useBlackPointCompensation", false);
}

void KisConfig::setUseBlackPointCompensation(bool useBlackPointCompensation)
{
    m_cfg -> writeEntry("useBlackPointCompensation", useBlackPointCompensation);
}


bool KisConfig::dither8Bit() const
{
    return m_cfg -> readBoolEntry("dither8Bit", false);
}

void KisConfig::setDither8Bit(bool dither8Bit)
{
    m_cfg -> writeEntry("dither8Bit", dither8Bit);
}

bool KisConfig::showRulers() const
{
    return m_cfg->readBoolEntry("showrulers", false);
}

void KisConfig::setShowRulers(bool rulers)
{
    m_cfg->writeEntry("showrulers", rulers);
}

bool KisConfig::askProfileOnOpen() const
{
    return m_cfg -> readBoolEntry("askProfileOnOpen", true);
}

void KisConfig::setAskProfileOnOpen(bool askProfileOnOpen)
{
    m_cfg -> writeEntry("askProfileOnOpen", askProfileOnOpen);
}


bool KisConfig::askProfileOnPaste() const
{
    return m_cfg -> readBoolEntry("askProfileOnPaste", true);
}

void KisConfig::setAskProfileOnPaste(bool askProfileOnPaste)
{
    m_cfg -> writeEntry("askProfileOnPaste", askProfileOnPaste);
}


bool KisConfig::applyMonitorProfileOnCopy() const
{
    return m_cfg -> readBoolEntry("applyMonitorProfileOnCopy", false);
}

void KisConfig::setApplyMonitorProfileOnCopy(bool applyMonitorProfileOnCopy)
{
    m_cfg -> writeEntry("applyMonitorProfileOnCopy", applyMonitorProfileOnCopy);
}


Q_INT32 KisConfig::renderIntent()
{
    return m_cfg -> readNumEntry("renderIntent", INTENT_PERCEPTUAL);
}

void KisConfig::setRenderIntent(Q_INT32 renderIntent)
{
    m_cfg -> writeEntry("renderIntent", renderIntent);
}

bool KisConfig::useOpenGL() const
{
    return m_cfg -> readBoolEntry("useOpenGL", false);
}

void KisConfig::setUseOpenGL(bool useOpenGL)
{
    m_cfg -> writeEntry("useOpenGL", useOpenGL);
}

bool KisConfig::useOpenGLShaders() const
{
    return m_cfg -> readBoolEntry("useOpenGLShaders", false);
}

void KisConfig::setUseOpenGLShaders(bool useOpenGLShaders)
{
    m_cfg -> writeEntry("useOpenGLShaders", useOpenGLShaders);
}

Q_INT32 KisConfig::maxNumberOfThreads()
{
    return m_cfg -> readNumEntry("maxthreads", DEFAULT_MAX_THREADS);
}

void KisConfig::setMaxNumberOfThreads(Q_INT32 maxThreads)
{
    m_cfg -> writeEntry("maxthreads", maxThreads);
}

Q_INT32 KisConfig::maxTilesInMem() const
{
    return m_cfg -> readNumEntry("maxtilesinmem", DEFAULT_MAX_TILES_MEM);
}

void KisConfig::setMaxTilesInMem(Q_INT32 tiles)
{
    m_cfg -> writeEntry("maxtilesinmem", tiles);
}

Q_INT32 KisConfig::swappiness() const
{
    return m_cfg -> readNumEntry("swappiness", DEFAULT_SWAPPINESS);
}

void KisConfig::setSwappiness(Q_INT32 swappiness)
{
    m_cfg -> writeEntry("swappiness", swappiness);
}

Q_INT32 KisConfig::getPressureCorrection()
{
    return m_cfg->readNumEntry( "pressurecorrection", DEFAULT_PRESSURE_CORRECTION );
}

void KisConfig::setPressureCorrection( Q_INT32 correction )
{
    m_cfg->writeEntry( "pressurecorrection",  correction );
}

Q_INT32 KisConfig::getDefaultPressureCorrection()
{
    return DEFAULT_PRESSURE_CORRECTION;
}

void KisConfig::setDockability( Q_INT32 dockability )
{
    m_cfg->writeEntry( "palettesdockability", dockability );
}

Q_INT32 KisConfig::dockability()
{
    return m_cfg->readNumEntry("palettesdockability", DEFAULT_DOCKABILITY);
}

Q_INT32 KisConfig::getDefaultDockability()
{
    return DEFAULT_DOCKABILITY;
}

float KisConfig::dockerFontSize()
{
    return (float) m_cfg->readNumEntry("palettefontsize", (int)getDefaultDockerFontSize());
}

float KisConfig::getDefaultDockerFontSize()
{
    float ps = QMIN(9, KGlobalSettings::generalFont().pointSize() * 0.8);
    return ps;
}

void KisConfig::setDockerFontSize(float size)
{
    m_cfg->writeEntry("palettefontsize", size);
}
