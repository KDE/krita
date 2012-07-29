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

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <fixx11h.h>
#include <QX11Info>
#endif

#include <QFont>
#include <QThread>
#include <QStringList>

#include <kglobalsettings.h>
#include <kglobal.h>
#include <kconfig.h>

#include <KoDocument.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>

#include <kis_debug.h>

#include "kis_canvas_resource_provider.h"
#include "kis_global.h"

#include "config-ocio.h"


namespace
{
const double IMAGE_DEFAULT_RESOLUTION = 100.0; // dpi
const qint32 IMAGE_DEFAULT_WIDTH = 1600;
const qint32 IMAGE_DEFAULT_HEIGHT = 1200;
const enumCursorStyle DEFAULT_CURSOR_STYLE = CURSOR_STYLE_TOOLICON;
const qint32 DEFAULT_MAX_TILES_MEM = 5000;
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

int KisConfig::undoStackLimit() const
{
    return m_cfg.readEntry("undoStackLimit", 30);
}

void KisConfig::setUndoStackLimit(int limit)
{
    m_cfg.writeEntry("undoStackLimit", limit);
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

QString KisConfig::defColorModel() const
{
    return m_cfg.readEntry("colorModelDef", KoColorSpaceRegistry::instance()->rgb8()->colorModelId().id());
}

void KisConfig::defColorModel(const QString & model)
{
    m_cfg.writeEntry("colorModelDef", model);
}

QString KisConfig::defColorDepth() const
{
    return m_cfg.readEntry("colorDepthDef", KoColorSpaceRegistry::instance()->rgb8()->colorDepthId().id());
}

void KisConfig::defColorDepth(const QString & depth)
{
    m_cfg.writeEntry("colorDepthDef", depth);
}

QString KisConfig::defColorProfile() const
{
    return m_cfg.readEntry("colorProfileDef", KoColorSpaceRegistry::instance()->rgb8()->profile()->name());
}

void KisConfig::defColorProfile(const QString & profile)
{
    m_cfg.writeEntry("colorProfileDef", profile);
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

void KisConfig::setMonitorProfile(const QString & monitorProfile, bool override)
{
    m_cfg.writeEntry("monitorProfile/OverrideX11", override);
    m_cfg.writeEntry("monitorProfile", monitorProfile);
}

const KoColorProfile *KisConfig::getScreenProfile(int screen)
{
#ifdef Q_WS_X11

    Atom type;
    int format;
    unsigned long nitems;
    unsigned long bytes_after;
    quint8 * str;

    static Atom icc_atom = XInternAtom(QX11Info::display(), "_ICC_PROFILE", True);

    if (XGetWindowProperty(QX11Info::display(),
                           QX11Info::appRootWindow(screen),
                           icc_atom,
                           0,
                           INT_MAX,
                           False,
                           XA_CARDINAL,
                           &type,
                           &format,
                           &nitems,
                           &bytes_after,
                           (unsigned char **) &str) == Success
       ) {
        QByteArray bytes(nitems, '\0');
        bytes = QByteArray::fromRawData((char*)str, (quint32)nitems);
        // XXX: this assumes the screen is 8 bits -- which might not be true
        return KoColorSpaceRegistry::instance()->createColorProfile(RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), bytes);
    }
    else {
        return 0;
    }
#else
    return 0;

#endif
}

const KoColorProfile *KisConfig::displayProfile(int screen)
{
    // first try to get the screen profile set by the X11 _ICC_PROFILE atom (compatible with colord,
    // but colord can set the atom to none, in which case we cannot create a suitable profile)

    // if the user plays with the settings, they can override the display profile, in which case
    // we don't want the X11 atom setting.
    bool override = m_cfg.readEntry("monitorProfile/OverrideX11", false);
    const KoColorProfile *profile = 0;
    if (!override) {
        profile = KisConfig::getScreenProfile(screen);
    }

    // if it fails. check the configuration
    if (!profile || !profile->isSuitableForDisplay()) {
        QString monitorProfileName = monitorProfile();
        if (!monitorProfileName.isEmpty()) {
            profile = KoColorSpaceRegistry::instance()->profileByName(monitorProfileName);
        }
    }
    // if we still don't have a profile, or the profile isn't suitable for display,
    // we need to get a last-resort profile. the built-in sRGB is a good choice then.
    if (!profile || !profile->isSuitableForDisplay()) {
        profile = KoColorSpaceRegistry::instance()->profileByName("sRGB Built-in");
    }

    return profile;
}

QString KisConfig::workingColorSpace() const
{
    return m_cfg.readEntry("workingColorSpace", "RGBA");
}

void KisConfig::setWorkingColorSpace(const QString & workingColorSpace)
{
    m_cfg.writeEntry("workingColorSpace", workingColorSpace);
}


QString KisConfig::printerColorSpace() const
{
    //TODO currently only rgb8 is supported
    //return m_cfg.readEntry("printerColorSpace", "RGBA");
    return QString("RGBA");
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
    return m_cfg.readEntry("useBlackPointCompensation", true);
}

void KisConfig::setUseBlackPointCompensation(bool useBlackPointCompensation)
{
    m_cfg.writeEntry("useBlackPointCompensation", useBlackPointCompensation);
}

bool KisConfig::allowLCMSOptimization() const
{
    return m_cfg.readEntry("allowLCMSOptimization", true);
}

void KisConfig::setAllowLCMSOptimization(bool allowLCMSOptimization)
{
    m_cfg.writeEntry("allowLCMSOptimization", allowLCMSOptimization);
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
    qint32 intent = m_cfg.readEntry("renderIntent", INTENT_PERCEPTUAL);
    if (intent > 3) intent = 3;
    if (intent < 0) intent = 0;
    return intent;
}

void KisConfig::setRenderIntent(qint32 renderIntent)
{
    if (renderIntent > 3) renderIntent = 3;
    if (renderIntent < 0) renderIntent = 0;
    m_cfg.writeEntry("renderIntent", renderIntent);
}

bool KisConfig::useOpenGL() const
{
    QString canvasState = m_cfg.readEntry("canvasState");
    return (m_cfg.readEntry("useOpenGL", false) && (canvasState == "OPENGL_SUCCESS" || canvasState == "TRY_OPENGL"));
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

bool KisConfig::useOpenGLToolOutlineWorkaround() const
{
    return m_cfg.readEntry("useOpenGLToolOutlineWorkaround", false);
}

void KisConfig::setUseOpenGLToolOutlineWorkaround(bool useWorkaround)
{
    m_cfg.writeEntry("useOpenGLToolOutlineWorkaround", useWorkaround);
}

bool KisConfig::useOpenGLTrilinearFiltering() const
{
    return m_cfg.readEntry("useOpenGLTrilinearFiltering", true);
}

void KisConfig::setUseOpenGLTrilinearFiltering(bool useTrilinearFiltering)
{
    m_cfg.writeEntry("useOpenGLTrilinearFiltering", useTrilinearFiltering);
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

bool KisConfig::getGridSpacingAspect()
{
    bool v = m_cfg.readEntry("gridspacingaspect", false);
    return v;
}

void KisConfig::setGridSpacingAspect(bool v)
{
    m_cfg.writeEntry("gridspacingaspect", v);
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

bool KisConfig::getGridOffsetAspect()
{
    bool v = m_cfg.readEntry("gridoffsetaspect", false);
    return v;
}

void KisConfig::setGridOffsetAspect(bool v)
{
    m_cfg.writeEntry("gridoffsetaspect", v);
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

QColor KisConfig::canvasBorderColor()
{
    QColor color(Qt::gray);
    return m_cfg.readEntry("canvasBorderColor", color);
}

void KisConfig::setCanvasBorderColor(const QColor& color)
{
    m_cfg.writeEntry("canvasBorderColor", color);
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

bool KisConfig::antialiasCurves()
{
    return m_cfg.readEntry("antialiascurves", true);
}

void KisConfig::setAntialiasCurves(bool v)
{
    m_cfg.writeEntry("antialiascurves", v);
}

bool KisConfig::showRootLayer()
{
    return m_cfg.readEntry("ShowRootLayer", false);
}

void KisConfig::setShowRootLayer(bool showRootLayer)
{
    m_cfg.writeEntry("ShowRootLayer", showRootLayer);
}

bool KisConfig::showOutlineWhilePainting()
{
    return m_cfg.readEntry("ShowOutlineWhilePainting", true);
}

void KisConfig::setShowOutlineWhilePainting(bool showOutlineWhilePainting)
{
    m_cfg.writeEntry("ShowOutlineWhilePainting", showOutlineWhilePainting);
}

int KisConfig::autoSaveInterval() {
    return m_cfg.readEntry("AutoSaveInterval", KoDocument::defaultAutoSave());
}

void KisConfig::setAutoSaveInterval(int seconds) {
    return m_cfg.writeEntry("AutoSaveInterval", seconds);
}

bool KisConfig::backupFile()
{
    return m_cfg.readEntry("CreateBackupFile", true);
}

void KisConfig::setBackupFile(bool backupFile)
{
    m_cfg.writeEntry("CreateBackupFile", backupFile);
}

bool KisConfig::showFilterGallery()
{
    return m_cfg.readEntry("showFilterGallery", false);
}

void KisConfig::setShowFilterGallery(bool showFilterGallery)
{
    m_cfg.writeEntry("showFilterGallery", showFilterGallery);
}

bool KisConfig::showFilterGalleryLayerMaskDialog()
{
    return m_cfg.readEntry("showFilterGalleryLayerMaskDialog", true);
}

void KisConfig::setShowFilterGalleryLayerMaskDialog(bool showFilterGallery)
{
    m_cfg.writeEntry("setShowFilterGalleryLayerMaskDialog", showFilterGallery);
}

QString KisConfig::defaultPainterlyColorModelId()
{
    return m_cfg.readEntry("defaultpainterlycolormodel", "KS6");
}

void KisConfig::setDefaultPainterlyColorModelId(const QString& def)
{
    m_cfg.writeEntry("defaultpainterlycolormodel", def);
}

QString KisConfig::defaultPainterlyColorDepthId()
{
    return m_cfg.readEntry("defaultpainterlycolordepth", "F32");
}

void KisConfig::setDefaultPainterlyColorDepthId(const QString& def)
{
    m_cfg.writeEntry("defaultpainterlycolordepth", def);
}

QString KisConfig::canvasState() const
{
    return m_cfg.readEntry("canvasState", "OPENGL_NOT_TRIED");
}

void KisConfig::setCanvasState(const QString& state)
{
    static QStringList acceptableStates;
    if (acceptableStates.isEmpty()) {
        acceptableStates << "OPENGL_SUCCESS" << "TRY_OPENGL" << "OPENGL_NOT_TRIED" << "OPENGL_FAILED";
    }
    if (acceptableStates.contains(state)) {
        m_cfg.writeEntry("canvasState", state);
    }
}

bool KisConfig::paintopPopupDetached() const
{
    return m_cfg.readEntry("PaintopPopupDetached", false);
}

void KisConfig::setPaintopPopupDetached(bool detached)
{
    m_cfg.writeEntry("PaintopPopupDetached", detached);
}

QString KisConfig::pressureTabletCurve() const
{
    return m_cfg.readEntry("tabletPressureCurve","0,0;1,1;");
}

void KisConfig::setPressureTabletCurve(const QString& curveString) const
{
    m_cfg.writeEntry("tabletPressureCurve", curveString);
}

bool KisConfig::zoomWithWheel() const
{
    return m_cfg.readEntry("ZoomWithWheel", true);
}

void KisConfig::setZoomWithWheel(const bool zoom) const
{
    m_cfg.writeEntry("ZoomWithWheel", zoom);
}

qreal KisConfig::vastScrolling() const
{
    return m_cfg.readEntry("vastScrolling", 0.9);
}

void KisConfig::setVastScrolling(const qreal factor) const
{
    m_cfg.writeEntry("vastScrolling", factor);
}

int KisConfig::presetChooserViewMode() const
{
    return m_cfg.readEntry("presetChooserViewMode", 0);
}

void KisConfig::setPresetChooserViewMode(const int mode)
{
    m_cfg.writeEntry("presetChooserViewMode", mode);
}


bool KisConfig::presetShowAllMode() const
{
    return m_cfg.readEntry("presetChooserShowAllPresets", true);
}

void KisConfig::setPresetShowAllMode(bool showAll)
{
    m_cfg.writeEntry("presetChooserShowAllPresets", showAll);
}

bool KisConfig::firstRun() const
{
    return m_cfg.readEntry("firstRun", true);
}

void KisConfig::setFirstRun(const bool first) const
{
    m_cfg.writeEntry("firstRun", first);
}

int KisConfig::horizontalSplitLines() const
{
    return m_cfg.readEntry("horizontalSplitLines", 1);
}
void KisConfig::setHorizontalSplitLines(const int numberLines) const
{
    m_cfg.writeEntry("horizontalSplitLines", numberLines);
}

int KisConfig::verticalSplitLines() const
{
    return m_cfg.readEntry("verticalSplitLines", 1);
}

void KisConfig::setVerticalSplitLines(const int numberLines) const
{
    m_cfg.writeEntry("verticalSplitLines", numberLines);
}

bool KisConfig::clicklessSpacePan() const
{
    return m_cfg.readEntry("clicklessSpacePan", true);
}

void KisConfig::setClicklessSpacePan(const bool toggle) const
{
    m_cfg.writeEntry("clicklessSpacePan", toggle);
}


int KisConfig::hideDockersFullscreen()
{
    return m_cfg.readEntry("hideDockersFullScreen", (int)Qt::Checked);
}

void KisConfig::setHideDockersFullscreen(const int value) const
{
    m_cfg.writeEntry("hideDockersFullScreen", value);
}

int KisConfig::hideMenuFullscreen()
{
    return m_cfg.readEntry("hideMenuFullScreen", (int)Qt::Checked);
}

void KisConfig::setHideMenuFullscreen(const int value) const
{
    m_cfg.writeEntry("hideMenuFullScreen", value);
}

int KisConfig::hideScrollbarsFullscreen()
{
    return m_cfg.readEntry("hideScrollbarsFullScreen", (int)Qt::Checked);
}

void KisConfig::setHideScrollbarsFullscreen(const int value) const
{
    m_cfg.writeEntry("hideScrollbarsFullScreen", value);
}

int KisConfig::hideStatusbarFullscreen()
{
    return m_cfg.readEntry("hideStatusbarFullScreen", (int)Qt::Checked);
}

void KisConfig::setHideStatusbarFullscreen(const int value) const
{
    m_cfg.writeEntry("hideStatusbarFullScreen", value);
}


int KisConfig::hideTitlebarFullscreen()
{
    return m_cfg.readEntry("hideTitleBarFullscreen", (int)Qt::Checked);
}
void KisConfig::setHideTitlebarFullscreen(const int value) const
{
    m_cfg.writeEntry("hideTitleBarFullscreen", value);
}


int KisConfig::hideToolbarFullscreen()
{
    return m_cfg.readEntry("hideToolbarFullscreen", (int)Qt::Checked);
}

void KisConfig::setHideToolbarFullscreen(const int value) const
{
    m_cfg.writeEntry("hideToolbarFullscreen", value);
}


QStringList KisConfig::favoriteCompositeOps() const
{
    return m_cfg.readEntry("favoriteCompositeOps", QStringList());
}

void KisConfig::setFavoriteCompositeOps(const QStringList& compositeOps)
{
    m_cfg.writeEntry("favoriteCompositeOps", compositeOps);
}

QString KisConfig::exportConfiguration(const QString &filterId) const
{
    return m_cfg.readEntry("ExportConfiguration-" + filterId, QString());
}

void KisConfig::setExportConfiguration(const QString &filterId, const KisPropertiesConfiguration &properties)
{
    QString exportConfig = properties.toXML();
    m_cfg.writeEntry("ExportConfiguration-" + filterId, exportConfig);

}

bool KisConfig::useOcio()
{
#ifdef HAVE_OCIO
    return m_cfg.readEntry("Krita/Ocio/UseOcio", true);
#else
    return false;
#endif
}

void KisConfig::setUseOcio(bool useOCIO)
{
    m_cfg.writeEntry("Krita/Ocio/UseOcio", useOCIO);
}


bool KisConfig::useOcioEnvironmentVariable()
{
    return m_cfg.readEntry("Krita/Ocio/UseEnvironment", true);
}

void KisConfig::setUseOcioEnvironmentVariable(bool useOCIO)
{
    m_cfg.writeEntry("Krita/Ocio/UseEnvironment", useOCIO);
}

QString KisConfig::ocioConfigurationPath()
{
    return m_cfg.readEntry("Krita/Ocio/OcioConfigPath", QString());
}

void KisConfig::setOcioConfigurationPath(const QString &path)
{
    m_cfg.writeEntry("Krita/Ocio/OcioConfigPath", path);
}


QString KisConfig::ocioLutPath()
{
    return m_cfg.readEntry("Krita/Ocio/OcioLutPath", QString());
}

void KisConfig::setOcioLutPath(const QString &path)
{
    m_cfg.writeEntry("Krita/Ocio/OcioLutPath", path);
}

QString KisConfig::defaultPalette()
{
    return m_cfg.readEntry("defaultPalette", QString());
}

void KisConfig::setDefaultPalette(const QString& name)
{
    m_cfg.writeEntry("defaultPalette", name);
}

bool KisConfig::useSystemMonitorProfile() const
{
    return m_cfg.readEntry("ColorManagement/UseSystemMonitorProfile", false);
}

void KisConfig::setUseSystemMonitorProfile(bool _useSystemMonitorProfile)
{
    m_cfg.writeEntry("ColorManagement/UseSystemMonitorProfile", _useSystemMonitorProfile);
}
