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

#include <QFont>

#include <kglobalsettings.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kdebug.h>
#include <config.h>

#include <lcms.h>

#include "kis_global.h"
#include "kis_config.h"

namespace {
    const double IMG_DEFAULT_RESOLUTION = 100.0;
    const qint32 IMG_DEFAULT_WIDTH = 512;
    const qint32 IMG_DEFAULT_HEIGHT = 512;
    const enumCursorStyle DEFAULT_CURSOR_STYLE = CURSOR_STYLE_OUTLINE;
    const qint32 DEFAULT_MAX_THREADS = 4;
    const qint32 DEFAULT_MAX_TILES_MEM = 5000; 
    const qint32 DEFAULT_SWAPPINESS = 100;
    const qint32 DEFAULT_PRESSURE_CORRECTION = 50;
    const qint32 DEFAULT_DOCKABILITY = 0;
    const qint32 DEFAULT_UNDO_LIMIT = 50;
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
    m_cfg->sync();
}


bool KisConfig::fixDockerWidth() const
{
    return m_cfg->readEntry("fixDockerWidth", true);
}

void KisConfig::setFixedDockerWidth(bool fix)
{
    m_cfg->writeEntry("fixDockerWidth", fix);
}

bool KisConfig::undoEnabled() const
{
    return m_cfg->readEntry("undoEnabled", true);
}

void KisConfig::setUndoEnabled(bool undo)
{
    m_cfg->writeEntry("undoEnabled", undo);
}


qint32 KisConfig::defUndoLimit() const
{
    return m_cfg->readEntry("undolimit", DEFAULT_UNDO_LIMIT);
}

void KisConfig::defUndoLimit(qint32 limit)
{
    m_cfg->writeEntry("undolimit", limit);
}

qint32 KisConfig::defImgWidth() const
{
    return m_cfg->readEntry("imgWidthDef", IMG_DEFAULT_WIDTH);
}

qint32 KisConfig::defImgHeight() const
{
    return m_cfg->readEntry("imgHeightDef", IMG_DEFAULT_HEIGHT);
}

double KisConfig::defImgResolution() const
{
    return m_cfg->readEntry("imgResolutionDef", IMG_DEFAULT_RESOLUTION);
}

void KisConfig::defImgWidth(qint32 width)
{
    m_cfg->writeEntry("imgWidthDef", width);
}

void KisConfig::defImgHeight(qint32 height)
{
    m_cfg->writeEntry("imgHeightDef", height);
}

void KisConfig::defImgResolution(double res)
{
    m_cfg->writeEntry("imgResolutionDef", res);
}

enumCursorStyle KisConfig::cursorStyle() const
{
    return (enumCursorStyle) m_cfg->readEntry("cursorStyleDef", int(DEFAULT_CURSOR_STYLE));
}

enumCursorStyle KisConfig::getDefaultCursorStyle() const
{
    return DEFAULT_CURSOR_STYLE;
}

void KisConfig::setCursorStyle(enumCursorStyle style)
{
    m_cfg->writeEntry("cursorStyleDef", (int)style);
}


QString KisConfig::monitorProfile() const
{
    return m_cfg->readEntry("monitorProfile", "None");
}

void KisConfig::setMonitorProfile(QString monitorProfile)
{
    m_cfg->writeEntry("monitorProfile", monitorProfile);
}


QString KisConfig::workingColorSpace() const
{
    return m_cfg->readEntry("workingColorSpace", "RGBA");
}

void KisConfig::setWorkingColorSpace(QString workingColorSpace)
{
    m_cfg->writeEntry(workingColorSpace, workingColorSpace);
}


QString KisConfig::printerColorSpace() const
{
    return m_cfg->readEntry("printerColorSpace", "CMYK");
}

void KisConfig::setPrinterColorSpace(QString printerColorSpace)
{
    m_cfg->writeEntry("printerColorSpace", printerColorSpace);
}


QString KisConfig::printerProfile() const
{
    return m_cfg->readEntry("printerProfile", "None");
}

void KisConfig::setPrinterProfile(QString printerProfile)
{
    m_cfg->writeEntry("printerProfile", printerProfile);
}


bool KisConfig::useBlackPointCompensation() const
{
    return m_cfg->readEntry("useBlackPointCompensation", false);
}

void KisConfig::setUseBlackPointCompensation(bool useBlackPointCompensation)
{
    m_cfg->writeEntry("useBlackPointCompensation", useBlackPointCompensation);
}


bool KisConfig::showRulers() const
{
    return m_cfg->readEntry("showrulers", false);
}

void KisConfig::setShowRulers(bool rulers)
{
    m_cfg->writeEntry("showrulers", rulers);
}


qint32 KisConfig::pasteBehaviour() const
{
    return m_cfg->readEntry("pasteBehaviour", 2);
}

void KisConfig::setPasteBehaviour(qint32 renderIntent)
{
    m_cfg->writeEntry("pasteBehaviour", renderIntent);
}


qint32 KisConfig::renderIntent() const
{
    return m_cfg->readEntry("renderIntent", INTENT_PERCEPTUAL);
}

void KisConfig::setRenderIntent(qint32 renderIntent)
{
    m_cfg->writeEntry("renderIntent", renderIntent);
}

bool KisConfig::useOpenGL() const
{
    return m_cfg->readEntry("useOpenGL", false);
}

void KisConfig::setUseOpenGL(bool useOpenGL)
{
    m_cfg->writeEntry("useOpenGL", useOpenGL);
}

bool KisConfig::useOpenGLShaders() const
{
    return m_cfg->readEntry("useOpenGLShaders", false);
}

void KisConfig::setUseOpenGLShaders(bool useOpenGLShaders)
{
    m_cfg->writeEntry("useOpenGLShaders", useOpenGLShaders);
}

qint32 KisConfig::maxNumberOfThreads()
{
    return m_cfg->readEntry("maxthreads", DEFAULT_MAX_THREADS);
}

void KisConfig::setMaxNumberOfThreads(qint32 maxThreads)
{
    m_cfg->writeEntry("maxthreads", maxThreads);
}

qint32 KisConfig::maxTilesInMem() const
{
    return m_cfg->readEntry("maxtilesinmem", DEFAULT_MAX_TILES_MEM);
}

void KisConfig::setMaxTilesInMem(qint32 tiles)
{
    m_cfg->writeEntry("maxtilesinmem", tiles);
}

qint32 KisConfig::swappiness() const
{
    return m_cfg->readEntry("swappiness", DEFAULT_SWAPPINESS);
}

void KisConfig::setSwappiness(qint32 swappiness)
{
    m_cfg->writeEntry("swappiness", swappiness);
}

qint32 KisConfig::getPressureCorrection()
{
    return m_cfg->readEntry( "pressurecorrection", DEFAULT_PRESSURE_CORRECTION );
}

void KisConfig::setPressureCorrection( qint32 correction )
{
    m_cfg->writeEntry( "pressurecorrection",  correction );
}

qint32 KisConfig::getDefaultPressureCorrection()
{
    return DEFAULT_PRESSURE_CORRECTION;
}

bool KisConfig::tabletDeviceEnabled(const QString& tabletDeviceName) const
{
    return m_cfg->readEntry("TabletDevice" + tabletDeviceName + "Enabled", false);
}

void KisConfig::setTabletDeviceEnabled(const QString& tabletDeviceName, bool enabled)
{
    m_cfg->writeEntry("TabletDevice" + tabletDeviceName + "Enabled", enabled);
}

qint32 KisConfig::tabletDeviceAxis(const QString& tabletDeviceName, const QString& axisName, qint32 defaultAxis) const
{
    return m_cfg->readEntry("TabletDevice" + tabletDeviceName + axisName, defaultAxis);
}

void KisConfig::setTabletDeviceAxis(const QString& tabletDeviceName, const QString& axisName, qint32 axis) const
{
    m_cfg->writeEntry("TabletDevice" + tabletDeviceName + axisName, axis);
}

void KisConfig::setDockability( qint32 dockability )
{
    m_cfg->writeEntry( "palettesdockability", dockability );
}

qint32 KisConfig::dockability()
{
    return m_cfg->readEntry("palettesdockability", DEFAULT_DOCKABILITY);
}

qint32 KisConfig::getDefaultDockability()
{
    return DEFAULT_DOCKABILITY;
}

float KisConfig::dockerFontSize()
{
    return (float) m_cfg->readEntry("palettefontsize", (int)getDefaultDockerFontSize());
}

float KisConfig::getDefaultDockerFontSize()
{
    float ps = qMin((double)9, KGlobalSettings::generalFont().pointSize() * 0.8);
    if (ps < 6) ps = 6;
    return ps;
}

void KisConfig::setDockerFontSize(float size)
{
    m_cfg->writeEntry("palettefontsize", (double)size);
}

quint32 KisConfig::getGridMainStyle()
{
    quint32 v = m_cfg->readEntry("gridmainstyle", 0);
    if (v > 2)
        v = 2;
    return v;
}

void KisConfig::setGridMainStyle(quint32 v)
{
    m_cfg->writeEntry("gridmainstyle", v);
}

quint32 KisConfig::getGridSubdivisionStyle()
{
    quint32 v = m_cfg->readEntry("gridsubdivisionstyle", 1);
    if (v > 2) v = 2;
    return v;
}

void KisConfig::setGridSubdivisionStyle(quint32 v)
{
    m_cfg->writeEntry("gridsubdivisionstyle", v);
}

QColor KisConfig::getGridMainColor()
{
	QColor col(99,99,99);
    return m_cfg->readEntry("gridmaincolor", col);
}

void KisConfig::setGridMainColor(QColor v)
{
    m_cfg->writeEntry("gridmaincolor", v);
}

QColor KisConfig::getGridSubdivisionColor()
{
	QColor col(150,150,150);
    return m_cfg->readEntry("gridsubdivisioncolor",col);
}

void KisConfig::setGridSubdivisionColor(QColor v)
{
    m_cfg->writeEntry("gridsubdivisioncolor", v);
}

quint32 KisConfig::getGridHSpacing()
{
    qint32 v = m_cfg->readEntry("gridhspacing", 10);
    return (quint32)qMax(1, v );
}

void KisConfig::setGridHSpacing(quint32 v)
{
    m_cfg->writeEntry("gridhspacing", v);
}

quint32 KisConfig::getGridVSpacing()
{
    qint32 v = m_cfg->readEntry("gridvspacing", 10);
    return (quint32)qMax(1, v );
}

void KisConfig::setGridVSpacing(quint32 v)
{
    m_cfg->writeEntry("gridvspacing", v);
}

quint32 KisConfig::getGridSubdivisions()
{
    qint32 v = m_cfg->readEntry("gridsubsivisons", 2);
    return (quint32)qMax(1, v );
}

void KisConfig::setGridSubdivisions(quint32 v)
{
    return m_cfg->writeEntry("gridsubsivisons", v);
}

quint32 KisConfig::getGridOffsetX()
{
    qint32 v = m_cfg->readEntry("gridoffsetx", 0);
    return (quint32)qMax(0, v );
}

void KisConfig::setGridOffsetX(quint32 v)
{
    m_cfg->writeEntry("gridoffsetx", v);
}

quint32 KisConfig::getGridOffsetY()
{
    qint32 v = m_cfg->readEntry("gridoffsety", 0);
    return (quint32)qMax(0, v );
}

void KisConfig::setGridOffsetY(quint32 v)
{
    m_cfg->writeEntry("gridoffsety", v);
}

