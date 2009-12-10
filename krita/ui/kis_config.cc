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

#include "kis_config.h"
#include <limits.h>

#include <kglobalsettings.h>
#include <kglobal.h>
#include <kis_debug.h>
#include <kconfig.h>
#include <QFont>
#include <QThread>

#include <lcms.h>

#include "kis_global.h"


namespace
{
const double IMAGE_DEFAULT_RESOLUTION = 100.0; // dpi
const qint32 IMAGE_DEFAULT_WIDTH = 1600;
const qint32 IMAGE_DEFAULT_HEIGHT = 1200;
const enumCursorStyle DEFAULT_CURSOR_STYLE = CURSOR_STYLE_TOOLICON;
const qint32 DEFAULT_MAX_TILES_MEM = 5000;
const qint32 DEFAULT_SWAPPINESS = 100;
}

KisConfig::KisConfig()
        : m_cfg(KGlobal::config()->group(""))
{
}

KisConfig::~KisConfig()
{
    m_cfg.sync();
}


bool KisConfig::useProjections() const
{
    return m_cfg.readEntry("useProjections", true);
}

void KisConfig::setUseProjections(bool useProj)
{
    m_cfg.writeEntry("useProjections", useProj);
}

bool KisConfig::undoEnabled() const
{
    return m_cfg.readEntry("undoEnabled", true);
}

void KisConfig::setUndoEnabled(bool undo)
{
    m_cfg.writeEntry("undoEnabled", undo);
}

qint32 KisConfig::defImageWidth() const
{
    return m_cfg.readEntry("imageWidthDef", IMAGE_DEFAULT_WIDTH);
}

qint32 KisConfig::defImageHeight() const
{
    return m_cfg.readEntry("imageHeightDef", IMAGE_DEFAULT_HEIGHT);
}

double KisConfig::defImageResolution() const
{
    return m_cfg.readEntry("imageResolutionDef", IMAGE_DEFAULT_RESOLUTION) / 72.0;
}

void KisConfig::defImageWidth(qint32 width)
{
    m_cfg.writeEntry("imageWidthDef", width);
}

void KisConfig::defImageHeight(qint32 height)
{
    m_cfg.writeEntry("imageHeightDef", height);
}

void KisConfig::defImageResolution(double res)
{
    m_cfg.writeEntry("imageResolutionDef", res*72.0);
}

enumCursorStyle KisConfig::cursorStyle() const
{
    return (enumCursorStyle) m_cfg.readEntry("cursorStyleDef", int(DEFAULT_CURSOR_STYLE));
}

enumCursorStyle KisConfig::getDefaultCursorStyle() const
{
    return DEFAULT_CURSOR_STYLE;
}

void KisConfig::setCursorStyle(enumCursorStyle style)
{
    m_cfg.writeEntry("cursorStyleDef", (int)style);
}


QString KisConfig::monitorProfile() const
{
    return m_cfg.readEntry("monitorProfile", "");
}

void KisConfig::setMonitorProfile(const QString & monitorProfile)
{
    m_cfg.writeEntry("monitorProfile", monitorProfile);
}


QString KisConfig::workingColorSpace() const
{
    return m_cfg.readEntry("workingColorSpace", "RGBA");
}

void KisConfig::setWorkingColorSpace(const QString & workingColorSpace)
{
    m_cfg.writeEntry(workingColorSpace, workingColorSpace);
}


QString KisConfig::printerColorSpace() const
{
    return m_cfg.readEntry("printerColorSpace", "RGBA");
}

void KisConfig::setPrinterColorSpace(const QString & printerColorSpace)
{
    m_cfg.writeEntry("printerColorSpace", printerColorSpace);
}


QString KisConfig::printerProfile() const
{
    return m_cfg.readEntry("printerProfile", "");
}

void KisConfig::setPrinterProfile(const QString & printerProfile)
{
    m_cfg.writeEntry("printerProfile", printerProfile);
}


bool KisConfig::useBlackPointCompensation() const
{
    return m_cfg.readEntry("useBlackPointCompensation", false);
}

void KisConfig::setUseBlackPointCompensation(bool useBlackPointCompensation)
{
    m_cfg.writeEntry("useBlackPointCompensation", useBlackPointCompensation);
}


bool KisConfig::showRulers() const
{
    return m_cfg.readEntry("showrulers", false);
}

void KisConfig::setShowRulers(bool rulers)
{
    m_cfg.writeEntry("showrulers", rulers);
}


qint32 KisConfig::pasteBehaviour() const
{
    return m_cfg.readEntry("pasteBehaviour", 2);
}

void KisConfig::setPasteBehaviour(qint32 renderIntent)
{
    m_cfg.writeEntry("pasteBehaviour", renderIntent);
}


qint32 KisConfig::renderIntent() const
{
    return m_cfg.readEntry("renderIntent", INTENT_PERCEPTUAL);
}

void KisConfig::setRenderIntent(qint32 renderIntent)
{
    m_cfg.writeEntry("renderIntent", renderIntent);
}

bool KisConfig::useOpenGL() const
{
    return m_cfg.readEntry("useOpenGL", false);
}

void KisConfig::setUseOpenGL(bool useOpenGL)
{
    m_cfg.writeEntry("useOpenGL", useOpenGL);
}

bool KisConfig::useOpenGLShaders() const
{
    return m_cfg.readEntry("useOpenGLShaders", false);
}

void KisConfig::setUseOpenGLShaders(bool useOpenGLShaders)
{
    m_cfg.writeEntry("useOpenGLShaders", useOpenGLShaders);
}

qint32 KisConfig::maxNumberOfThreads()
{
    return m_cfg.readEntry("maxthreads", QThread::idealThreadCount());
}

void KisConfig::setMaxNumberOfThreads(qint32 maxThreads)
{
    m_cfg.writeEntry("maxthreads", maxThreads);
}

qint32 KisConfig::maxTilesInMem() const
{
    return m_cfg.readEntry("maxtilesinmem", DEFAULT_MAX_TILES_MEM);
}

void KisConfig::setMaxTilesInMem(qint32 tiles)
{
    m_cfg.writeEntry("maxtilesinmem", tiles);
}

qint32 KisConfig::swappiness() const
{
    return m_cfg.readEntry("swappiness", DEFAULT_SWAPPINESS);
}

void KisConfig::setSwappiness(qint32 swappiness)
{
    m_cfg.writeEntry("swappiness", swappiness);
}

quint32 KisConfig::getGridMainStyle()
{
    quint32 v = m_cfg.readEntry("gridmainstyle", 0);
    if (v > 2)
        v = 2;
    return v;
}

void KisConfig::setGridMainStyle(quint32 v)
{
    m_cfg.writeEntry("gridmainstyle", v);
}

quint32 KisConfig::getGridSubdivisionStyle()
{
    quint32 v = m_cfg.readEntry("gridsubdivisionstyle", 1);
    if (v > 2) v = 2;
    return v;
}

void KisConfig::setGridSubdivisionStyle(quint32 v)
{
    m_cfg.writeEntry("gridsubdivisionstyle", v);
}

QColor KisConfig::getGridMainColor()
{
    QColor col(99, 99, 99);
    return m_cfg.readEntry("gridmaincolor", col);
}

void KisConfig::setGridMainColor(const QColor & v)
{
    m_cfg.writeEntry("gridmaincolor", v);
}

QColor KisConfig::getGridSubdivisionColor()
{
    QColor col(150, 150, 150);
    return m_cfg.readEntry("gridsubdivisioncolor", col);
}

void KisConfig::setGridSubdivisionColor(const QColor & v)
{
    m_cfg.writeEntry("gridsubdivisioncolor", v);
}

quint32 KisConfig::getGridHSpacing()
{
    qint32 v = m_cfg.readEntry("gridhspacing", 10);
    return (quint32)qMax(1, v);
}

void KisConfig::setGridHSpacing(quint32 v)
{
    m_cfg.writeEntry("gridhspacing", v);
}

quint32 KisConfig::getGridVSpacing()
{
    qint32 v = m_cfg.readEntry("gridvspacing", 10);
    return (quint32)qMax(1, v);
}

void KisConfig::setGridVSpacing(quint32 v)
{
    m_cfg.writeEntry("gridvspacing", v);
}

quint32 KisConfig::getGridSubdivisions()
{
    qint32 v = m_cfg.readEntry("gridsubsivisons", 2);
    return (quint32)qMax(1, v);
}

void KisConfig::setGridSubdivisions(quint32 v)
{
    m_cfg.writeEntry("gridsubsivisons", v);
}

quint32 KisConfig::getGridOffsetX()
{
    qint32 v = m_cfg.readEntry("gridoffsetx", 0);
    return (quint32)qMax(0, v);
}

void KisConfig::setGridOffsetX(quint32 v)
{
    m_cfg.writeEntry("gridoffsetx", v);
}

quint32 KisConfig::getGridOffsetY()
{
    qint32 v = m_cfg.readEntry("gridoffsety", 0);
    return (quint32)qMax(0, v);
}

void KisConfig::setGridOffsetY(quint32 v)
{
    m_cfg.writeEntry("gridoffsety", v);
}

qint32 KisConfig::checkSize()
{
    return m_cfg.readEntry("checksize", 32);
}

void KisConfig::setCheckSize(qint32 checksize)
{
    m_cfg.writeEntry("checksize", checksize);
}

bool KisConfig::scrollCheckers() const
{
    return m_cfg.readEntry("scrollingcheckers", false);
}

void KisConfig::setScrollingCheckers(bool sc)
{
    m_cfg.writeEntry("scrollingcheckers", sc);
}

QColor KisConfig::checkersColor()
{
    QColor col(220, 220, 220);
    return m_cfg.readEntry("checkerscolor", col);
}

void KisConfig::setCheckersColor(const QColor & v)
{
    m_cfg.writeEntry("checkerscolor", v);
}

int KisConfig::numProjectionThreads()
{
    return m_cfg.readEntry("maxprojectionthreads", QThread::idealThreadCount());
}

void KisConfig::setNumProjectThreads(int num)
{
    m_cfg.writeEntry("maxprojectionthreads", num);
}

int KisConfig::projectionChunkSize()
{
    return m_cfg.readEntry("updaterectsize", 1024);
}

void KisConfig::setProjectionChunkSize(int num)
{
    m_cfg.writeEntry("updaterectsize", num);
}

bool KisConfig::aggregateDirtyRegionsInPainter()
{
    return m_cfg.readEntry("aggregate_dirty_regions", true);
}

void KisConfig::setAggregateDirtyRegionsInPainter(bool aggregate)
{
    m_cfg.writeEntry("aggregate_dirty_regions", aggregate);
}

bool KisConfig::useBoundingRectInProjection()
{
    return m_cfg.readEntry("use_bounding_rect_of_dirty_region", true);
}

void KisConfig::setUseBoundingRectInProjection(bool use)
{
    m_cfg.writeEntry("use_bounding_rect_of_dirty_region", use);
}

bool KisConfig::useRegionOfInterestInProjection()
{
    return m_cfg.readEntry("use_region_of_interest", false);
}

void KisConfig::setUseRegionOfInterestInProjection(bool use)
{
    m_cfg.writeEntry("use_region_of_interest", use);
}

bool KisConfig::useNearestNeighbour()
{
    return m_cfg.readEntry("fast_zoom", false);
}

void KisConfig::setUseNearestNeighbour(bool useNearestNeigbour)
{
    m_cfg.writeEntry("fast_zoom", useNearestNeigbour);
}

bool KisConfig::useMipmapping()
{
    return m_cfg.readEntry("useMipmapping", true);
}

void KisConfig::setUseMipmapping(bool useMipmapping)
{
    m_cfg.writeEntry("useMipmapping", useMipmapping);
}

bool KisConfig::useSampling()
{
    return m_cfg.readEntry("sampled_scaling", false);
}

void KisConfig::setSampling(bool sampling)
{
    m_cfg.writeEntry("sampled_scaling", sampling);
}

bool KisConfig::threadColorSpaceConversion()
{
    return m_cfg.readEntry("thread_colorspace_conversion", false);
}

void KisConfig::setThreadColorSpaceConversion(bool threadColorSpaceConversion)
{
    m_cfg.writeEntry("thread_colorspace_conversion", threadColorSpaceConversion);
}

bool KisConfig::cacheKisImageAsQImage()
{
    return m_cfg.readEntry("cache_kis_image_as_qimage", true);
}

void KisConfig::setCacheKisImageAsQImage(bool cacheKisImageAsQImage)
{
    m_cfg.writeEntry("cache_kis_image_as_qimage", cacheKisImageAsQImage);
}


bool KisConfig::drawMaskVisualisationOnUnscaledCanvasCache()
{
    return m_cfg.readEntry("drawMaskVisualisationOnUnscaledCanvasCache", false);
}

void KisConfig::setDrawMaskVisualisationOnUnscaledCanvasCache(bool drawMaskVisualisationOnUnscaledCanvasCache)
{
    m_cfg.writeEntry("drawMaskVisualisationOnUnscaledCanvasCache", drawMaskVisualisationOnUnscaledCanvasCache);
}

bool KisConfig::noXRender()
{
    return m_cfg.readEntry("NoXRender",  false);
}

void KisConfig::setNoXRender(bool noXRender)
{
    m_cfg.writeEntry("NoXRender",  noXRender);
}

bool KisConfig::showRootLayer()
{
    return m_cfg.readEntry("ShowRootLayer", false);
}

void KisConfig::setShowRootLayer(bool showRootLayer)
{
    m_cfg.writeEntry("ShowRootLayer", showRootLayer);
}


quint32 KisConfig::maxCachedImageSize()
{
    // Let's say, 5 megapixels
    return m_cfg.readEntry("maxCachedImageSize", 5);
}

void KisConfig::setMaxCachedImageSize(quint32 size)
{
    m_cfg.writeEntry("maxCachedImageSize", size);
}


bool KisConfig::showFilterGallery()
{
    return m_cfg.readEntry("showFilterGallery", true);
}

void KisConfig::setShowFilterGallery(bool showFilterGallery)
{
    m_cfg.writeEntry("showFilterGallery", showFilterGallery);
}

QString KisConfig::defaultPainterlyColorSpace()
{
    return m_cfg.readEntry("defaultpainterlycolorspace", "KS6F32");;
}

void KisConfig::setDefaultPainterlyColorSpace(const QString& def)
{
    m_cfg.writeEntry("defaultpainterlycolorspace", def);;
}
