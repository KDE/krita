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

#include <QApplication>
#include <QDesktopWidget>
#include <QMutex>
#include <QFont>
#include <QThread>
#include <QStringList>

#include <kglobal.h>
#include <kconfig.h>

#include <KisDocument.h>

#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>

#include <kis_debug.h>

#include "kis_canvas_resource_provider.h"
#include "kis_global.h"
#include "kis_config_notifier.h"

#include <config-ocio.h>

#include <kis_color_manager.h>

namespace
{
const double IMAGE_DEFAULT_RESOLUTION = 100.0; // dpi
const qint32 IMAGE_DEFAULT_WIDTH = 1600;
const qint32 IMAGE_DEFAULT_HEIGHT = 1200;
const enumCursorStyle DEFAULT_CURSOR_STYLE = CURSOR_STYLE_OUTLINE;
const qint32 DEFAULT_MAX_TILES_MEM = 5000;

static QMutex s_synchLocker;

}

KisConfig::KisConfig()
    : m_cfg(KGlobal::config()->group(""))
{
}

KisConfig::~KisConfig()
{
    s_synchLocker.lock();
    m_cfg.sync();
    s_synchLocker.unlock();
}


bool KisConfig::disableTouchOnCanvas() const
{
    return m_cfg.readEntry("disableTouchOnCanvas", false);
}

void KisConfig::setDisableTouchOnCanvas(bool value) const
{
    m_cfg.writeEntry("disableTouchOnCanvas", value);
}

bool KisConfig::useProjections() const
{
    return m_cfg.readEntry("useProjections", true);
}

void KisConfig::setUseProjections(bool useProj) const
{
    m_cfg.writeEntry("useProjections", useProj);
}

bool KisConfig::undoEnabled() const
{
    return m_cfg.readEntry("undoEnabled", true);
}

void KisConfig::setUndoEnabled(bool undo) const
{
    m_cfg.writeEntry("undoEnabled", undo);
}

int KisConfig::undoStackLimit() const
{
    return m_cfg.readEntry("undoStackLimit", 30);
}

void KisConfig::setUndoStackLimit(int limit) const
{
    m_cfg.writeEntry("undoStackLimit", limit);
}
bool KisConfig::useCumulativeUndoRedo()
{
    return m_cfg.readEntry("useCumulativeUndoRedo",false);
}

void KisConfig::setCumulativeUndoRedo(bool value)
{
    m_cfg.writeEntry("useCumulativeUndoRedo", value);
}

double KisConfig::stackT1()
{
     return m_cfg.readEntry("stackT1",5);
}

void KisConfig::setStackT1(int T1)
{
    m_cfg.writeEntry("stackT1", T1);
}

double KisConfig::stackT2()
{
     return m_cfg.readEntry("stackT2",1);
}

void KisConfig::setStackT2(int T2)
{
    m_cfg.writeEntry("stackT2", T2);
}

int KisConfig::stackN()
{
    return m_cfg.readEntry("stackN",5);
}

void KisConfig::setStackN(int N)
{
     m_cfg.writeEntry("stackN", N);
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

void KisConfig::defColorModel(const QString & model) const
{
    m_cfg.writeEntry("colorModelDef", model);
}

QString KisConfig::defaultColorDepth() const
{
    return m_cfg.readEntry("colorDepthDef", KoColorSpaceRegistry::instance()->rgb8()->colorDepthId().id());
}

void KisConfig::setDefaultColorDepth(const QString & depth) const
{
    m_cfg.writeEntry("colorDepthDef", depth);
}

QString KisConfig::defColorProfile() const
{
    return m_cfg.readEntry("colorProfileDef", KoColorSpaceRegistry::instance()->rgb8()->profile()->name());
}

void KisConfig::defColorProfile(const QString & profile) const
{
    m_cfg.writeEntry("colorProfileDef", profile);
}

void KisConfig::defImageWidth(qint32 width) const
{
    m_cfg.writeEntry("imageWidthDef", width);
}

void KisConfig::defImageHeight(qint32 height) const
{
    m_cfg.writeEntry("imageHeightDef", height);
}

void KisConfig::defImageResolution(double res) const
{
    m_cfg.writeEntry("imageResolutionDef", res*72.0);
}

bool KisConfig::defAutoFrameBreakEnabled() const
{
    return m_cfg.readEntry("autoFrameBreakEnabled", false);
}

void KisConfig::defAutoFrameBreakEnabled(bool state) const
{
    m_cfg.writeEntry("autoFrameBreakEnabled", state);
}

bool KisConfig::defOnionSkinningEnabled() const
{
    return m_cfg.readEntry("onionSkinningEnabled", false);
}

void KisConfig::defOnionSkinningEnabled(bool state) const
{
    m_cfg.writeEntry("onionSkinningEnabled", state);
}

int KisConfig::defFps() const
{
    return m_cfg.readEntry("fps", 12);
}

void KisConfig::defFps(int value) const
{
    m_cfg.writeEntry("fps", value);
}

int KisConfig::defLocalPlaybackRange() const
{
    return m_cfg.readEntry("localPlaybackRange", 15);
}

void KisConfig::defLocalPlaybackRange(int value) const
{
    m_cfg.writeEntry("localPlaybackRange", value);
}

bool KisConfig::defLoopingEnabled() const
{
    return m_cfg.readEntry("loopingEnabled", true);
}

void KisConfig::defLoopingEnabled(bool state) const
{
    m_cfg.writeEntry("loopingEnabled", state);
}

enumCursorStyle KisConfig::cursorStyle() const
{
    return (enumCursorStyle) m_cfg.readEntry("cursorStyleDef", int(DEFAULT_CURSOR_STYLE));
}

enumCursorStyle KisConfig::getDefaultCursorStyle() const
{
    return DEFAULT_CURSOR_STYLE;
}

void KisConfig::setCursorStyle(enumCursorStyle style) const
{
    m_cfg.writeEntry("cursorStyleDef", (int)style);
}

bool KisConfig::useDirtyPresets() const
{
   return m_cfg.readEntry("useDirtyPresets",false);
}
void KisConfig::setUseDirtyPresets(bool value)
{
    m_cfg.writeEntry("useDirtyPresets",value);
    KisConfigNotifier::instance()->notifyConfigChanged();
}

bool KisConfig::useEraserBrushSize() const
{
   return m_cfg.readEntry("useEraserBrushSize",false);
}

void KisConfig::setUseEraserBrushSize(bool value)
{
    m_cfg.writeEntry("useEraserBrushSize",value);
    KisConfigNotifier::instance()->notifyConfigChanged();
}

QColor KisConfig::getMDIBackgroundColor() const
{
    QColor col(200, 200, 200);
    return m_cfg.readEntry("mdiBackgroundColor", col);
}

void KisConfig::setMDIBackgroundColor(const QColor &v) const
{
    m_cfg.writeEntry("mdiBackgroundColor", v);
}

QString KisConfig::getMDIBackgroundImage() const
{
    return m_cfg.readEntry("mdiBackgroundImage", "");
}

void KisConfig::setMDIBackgroundImage(const QString &filename) const
{
    m_cfg.writeEntry("mdiBackgroundImage", filename);
}

QString KisConfig::monitorProfile(int screen) const
{
    QString profile = m_cfg.readEntry("monitorProfile" + QString(screen == 0 ? "": QString("_%1").arg(screen)), "");
    //qDebug() << "KisConfig::monitorProfile()" << profile;
    return profile;
}

QString KisConfig::monitorForScreen(int screen, const QString &defaultMonitor) const
{
    return m_cfg.readEntry(QString("monitor_for_screen_%1").arg(screen), defaultMonitor);
}

void KisConfig::setMonitorForScreen(int screen, const QString& monitor)
{
    m_cfg.writeEntry(QString("monitor_for_screen_%1").arg(screen), monitor);
}

void KisConfig::setMonitorProfile(int screen, const QString & monitorProfile, bool override) const
{
    m_cfg.writeEntry("monitorProfile/OverrideX11", override);
    m_cfg.writeEntry("monitorProfile" + QString(screen == 0 ? "": QString("_%1").arg(screen)), monitorProfile);
}

const KoColorProfile *KisConfig::getScreenProfile(int screen)
{
    KisConfig cfg;
    QString monitorId = cfg.monitorForScreen(screen, "");
    if (monitorId.isEmpty()) {
        return 0;
    }

    QByteArray bytes = KisColorManager::instance()->displayProfile(monitorId);

    if (bytes.length() > 0) {
        const KoColorProfile *profile = KoColorSpaceRegistry::instance()->createColorProfile(RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), bytes);
        //qDebug() << "KisConfig::getScreenProfile for screen" << screen << profile->name();
        return profile;
    }
    else {
        //qDebug() << "Could not get a system monitor profile";
        return 0;
    }
}

const KoColorProfile *KisConfig::displayProfile(int screen) const
{
    // if the user plays with the settings, they can override the display profile, in which case
    // we don't want the system setting.
    bool override = m_cfg.readEntry("monitorProfile/OverrideX11", false);
    //qDebug() << "KisConfig::displayProfile(). Override X11:" << override;
    const KoColorProfile *profile = 0;
    if (override) {
        //qDebug() << "\tGoing to get the screen profile";
        profile = KisConfig::getScreenProfile(screen);
    }

    // if it fails. check the configuration
    if (!profile || !profile->isSuitableForDisplay()) {
        //ebug() << "\tGoing to get the monitor profile";
        QString monitorProfileName = monitorProfile(screen);
        //qDebug() << "\t\tmonitorProfileName:" << monitorProfileName;
        if (!monitorProfileName.isEmpty()) {
            profile = KoColorSpaceRegistry::instance()->profileByName(monitorProfileName);
        }
        if (profile) {
            //qDebug() << "\t\tsuitable for display6" << profile->isSuitableForDisplay();
        }
        else {
            //qDebug() << "\t\tstill no profile";
        }
    }
    // if we still don't have a profile, or the profile isn't suitable for display,
    // we need to get a last-resort profile. the built-in sRGB is a good choice then.
    if (!profile || !profile->isSuitableForDisplay()) {
        //qDebug() << "\tnothing worked, going to get sRGB built-in";
        profile = KoColorSpaceRegistry::instance()->profileByName("sRGB Built-in");
    }

    if (profile) {
        //qDebug() << "\tKisConfig::displayProfile for screen" << screen << "is" << profile->name();
    }
    else {
        //qDebug() << "\tCOuldn't get a display profile at all";
    }

    return profile;
}

QString KisConfig::workingColorSpace() const
{
    return m_cfg.readEntry("workingColorSpace", "RGBA");
}

void KisConfig::setWorkingColorSpace(const QString & workingColorSpace) const
{
    m_cfg.writeEntry("workingColorSpace", workingColorSpace);
}

QString KisConfig::printerColorSpace() const
{
    //TODO currently only rgb8 is supported
    //return m_cfg.readEntry("printerColorSpace", "RGBA");
    return QString("RGBA");
}

void KisConfig::setPrinterColorSpace(const QString & printerColorSpace) const
{
    m_cfg.writeEntry("printerColorSpace", printerColorSpace);
}


QString KisConfig::printerProfile() const
{
    return m_cfg.readEntry("printerProfile", "");
}

void KisConfig::setPrinterProfile(const QString & printerProfile) const
{
    m_cfg.writeEntry("printerProfile", printerProfile);
}


bool KisConfig::useBlackPointCompensation() const
{
    return m_cfg.readEntry("useBlackPointCompensation", true);
}

void KisConfig::setUseBlackPointCompensation(bool useBlackPointCompensation) const
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

void KisConfig::setShowRulers(bool rulers) const
{
    m_cfg.writeEntry("showrulers", rulers);
}


qint32 KisConfig::pasteBehaviour() const
{
    return m_cfg.readEntry("pasteBehaviour", 2);
}

void KisConfig::setPasteBehaviour(qint32 renderIntent) const
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

void KisConfig::setRenderIntent(qint32 renderIntent) const
{
    if (renderIntent > 3) renderIntent = 3;
    if (renderIntent < 0) renderIntent = 0;
    m_cfg.writeEntry("renderIntent", renderIntent);
}

bool KisConfig::useOpenGL() const
{
    if (qApp->applicationName() == "krita" || qApp->applicationName() == "kritaanimation") {
        //qDebug() << "use opengl" << m_cfg.readEntry("useOpenGL", true) << "success" << m_cfg.readEntry("canvasState", "OPENGL_SUCCESS");
        QString canvasState = m_cfg.readEntry("canvasState", "OPENGL_SUCCESS");
        return (m_cfg.readEntry("useOpenGL", true) && (canvasState == "OPENGL_SUCCESS" || canvasState == "TRY_OPENGL"));
    }
    else if (qApp->applicationName() == "kritasketch" || qApp->applicationName() == "kritagemini") {
        return true; // for sketch and gemini
    } else {
        return false;
    }
}

void KisConfig::setUseOpenGL(bool useOpenGL) const
{
    m_cfg.writeEntry("useOpenGL", useOpenGL);
}

int KisConfig::openGLFilteringMode() const
{
    return m_cfg.readEntry("OpenGLFilterMode", 3);
}

void KisConfig::setOpenGLFilteringMode(int filteringMode)
{
    m_cfg.writeEntry("OpenGLFilterMode", filteringMode);
}

bool KisConfig::useOpenGLTextureBuffer() const
{
    return m_cfg.readEntry("useOpenGLTextureBuffer", true);
}

void KisConfig::setUseOpenGLTextureBuffer(bool useBuffer)
{
    m_cfg.writeEntry("useOpenGLTextureBuffer", useBuffer);
}

int KisConfig::openGLTextureSize() const
{
    return m_cfg.readEntry("textureSize", 256);
}


bool KisConfig::disableDoubleBuffering() const
{
    return m_cfg.readEntry("disableDoubleBuffering", true);
}

void KisConfig::setDisableDoubleBuffering(bool disableDoubleBuffering)
{
    m_cfg.writeEntry("disableDoubleBuffering", disableDoubleBuffering);
}

bool KisConfig::disableVSync() const
{
    return m_cfg.readEntry("disableVSync", true);
}

void KisConfig::setDisableVSync(bool disableVSync)
{
    m_cfg.writeEntry("disableVSync", disableVSync);
}

bool KisConfig::showAdvancedOpenGLSettings() const
{
    return m_cfg.readEntry("showAdvancedOpenGLSettings", false);
}

bool KisConfig::forceOpenGLFenceWorkaround() const
{
    return m_cfg.readEntry("forceOpenGLFenceWorkaround", false);
}

int KisConfig::numMipmapLevels() const
{
    return m_cfg.readEntry("numMipmapLevels", 4);
}

int KisConfig::textureOverlapBorder() const
{
    return 1 << qMax(0, numMipmapLevels());
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

void KisConfig::setMaxTilesInMem(qint32 tiles) const
{
    m_cfg.writeEntry("maxtilesinmem", tiles);
}

quint32 KisConfig::getGridMainStyle() const
{
    quint32 v = m_cfg.readEntry("gridmainstyle", 0);
    if (v > 2)
        v = 2;
    return v;
}

void KisConfig::setGridMainStyle(quint32 v) const
{
    m_cfg.writeEntry("gridmainstyle", v);
}

quint32 KisConfig::getGridSubdivisionStyle() const
{
    quint32 v = m_cfg.readEntry("gridsubdivisionstyle", 1);
    if (v > 2) v = 2;
    return v;
}

void KisConfig::setGridSubdivisionStyle(quint32 v) const
{
    m_cfg.writeEntry("gridsubdivisionstyle", v);
}

QColor KisConfig::getGridMainColor() const
{
    QColor col(99, 99, 99);
    return m_cfg.readEntry("gridmaincolor", col);
}

void KisConfig::setGridMainColor(const QColor & v) const
{
    m_cfg.writeEntry("gridmaincolor", v);
}

QColor KisConfig::getGridSubdivisionColor() const
{
    QColor col(150, 150, 150);
    return m_cfg.readEntry("gridsubdivisioncolor", col);
}

void KisConfig::setGridSubdivisionColor(const QColor & v) const
{
    m_cfg.writeEntry("gridsubdivisioncolor", v);
}

quint32 KisConfig::getGridHSpacing() const
{
    qint32 v = m_cfg.readEntry("gridhspacing", 10);
    return (quint32)qMax(1, v);
}

void KisConfig::setGridHSpacing(quint32 v) const
{
    m_cfg.writeEntry("gridhspacing", v);
}

quint32 KisConfig::getGridVSpacing() const
{
    qint32 v = m_cfg.readEntry("gridvspacing", 10);
    return (quint32)qMax(1, v);
}

void KisConfig::setGridVSpacing(quint32 v) const
{
    m_cfg.writeEntry("gridvspacing", v);
}

bool KisConfig::getGridSpacingAspect() const
{
    bool v = m_cfg.readEntry("gridspacingaspect", false);
    return v;
}

void KisConfig::setGridSpacingAspect(bool v) const
{
    m_cfg.writeEntry("gridspacingaspect", v);
}

quint32 KisConfig::getGridSubdivisions() const
{
    qint32 v = m_cfg.readEntry("gridsubsivisons", 2);
    return (quint32)qMax(1, v);
}

void KisConfig::setGridSubdivisions(quint32 v) const
{
    m_cfg.writeEntry("gridsubsivisons", v);
}

quint32 KisConfig::getGridOffsetX() const
{
    qint32 v = m_cfg.readEntry("gridoffsetx", 0);
    return (quint32)qMax(0, v);
}

void KisConfig::setGridOffsetX(quint32 v) const
{
    m_cfg.writeEntry("gridoffsetx", v);
}

quint32 KisConfig::getGridOffsetY() const
{
    qint32 v = m_cfg.readEntry("gridoffsety", 0);
    return (quint32)qMax(0, v);
}

void KisConfig::setGridOffsetY(quint32 v) const
{
    m_cfg.writeEntry("gridoffsety", v);
}

bool KisConfig::getGridOffsetAspect() const
{
    bool v = m_cfg.readEntry("gridoffsetaspect", false);
    return v;
}

void KisConfig::setGridOffsetAspect(bool v) const
{
    m_cfg.writeEntry("gridoffsetaspect", v);
}

qint32 KisConfig::checkSize() const
{
    return m_cfg.readEntry("checksize", 32);
}

void KisConfig::setCheckSize(qint32 checksize) const
{
    m_cfg.writeEntry("checksize", checksize);
}

bool KisConfig::scrollCheckers() const
{
    return m_cfg.readEntry("scrollingcheckers", false);
}

void KisConfig::setScrollingCheckers(bool sc) const
{
    m_cfg.writeEntry("scrollingcheckers", sc);
}

QColor KisConfig::canvasBorderColor() const
{
    QColor color(QColor(128,128,128));
    return m_cfg.readEntry("canvasBorderColor", color);
}

void KisConfig::setCanvasBorderColor(const QColor& color) const
{
    m_cfg.writeEntry("canvasBorderColor", color);
}

bool KisConfig::hideScrollbars() const
{
    return m_cfg.readEntry("hideScrollbars", false);
}

void KisConfig::setHideScrollbars(bool value) const
{
    m_cfg.writeEntry("hideScrollbars", value);
}


QColor KisConfig::checkersColor1() const
{
    QColor col(220, 220, 220);
    return m_cfg.readEntry("checkerscolor", col);
}

void KisConfig::setCheckersColor1(const QColor & v) const
{
    m_cfg.writeEntry("checkerscolor", v);
}

QColor KisConfig::checkersColor2() const
{
    return m_cfg.readEntry("checkerscolor2", QColor(Qt::white));
}

void KisConfig::setCheckersColor2(const QColor & v) const
{
    m_cfg.writeEntry("checkerscolor2", v);
}

bool KisConfig::antialiasCurves() const
{
    return m_cfg.readEntry("antialiascurves", true);
}

void KisConfig::setAntialiasCurves(bool v) const
{
    m_cfg.writeEntry("antialiascurves", v);
}

QColor KisConfig::selectionOverlayMaskColor() const
{
    QColor def(255, 0, 0, 220);
    return m_cfg.readEntry("selectionOverlayMaskColor", def);
}

void KisConfig::setSelectionOverlayMaskColor(const QColor &color)
{
    m_cfg.writeEntry("selectionOverlayMaskColor", color);
}

bool KisConfig::antialiasSelectionOutline() const
{
    return m_cfg.readEntry("AntialiasSelectionOutline", false);
}

void KisConfig::setAntialiasSelectionOutline(bool v) const
{
    m_cfg.writeEntry("AntialiasSelectionOutline", v);
}

bool KisConfig::showRootLayer() const
{
    return m_cfg.readEntry("ShowRootLayer", false);
}

void KisConfig::setShowRootLayer(bool showRootLayer) const
{
    m_cfg.writeEntry("ShowRootLayer", showRootLayer);
}

bool KisConfig::showGlobalSelection() const
{
    return m_cfg.readEntry("ShowGlobalSelection", false);
}

void KisConfig::setShowGlobalSelection(bool showGlobalSelection) const
{
    m_cfg.writeEntry("ShowGlobalSelection", showGlobalSelection);
}

bool KisConfig::showOutlineWhilePainting() const
{
    return m_cfg.readEntry("ShowOutlineWhilePainting", true);
}

void KisConfig::setShowOutlineWhilePainting(bool showOutlineWhilePainting) const
{
    m_cfg.writeEntry("ShowOutlineWhilePainting", showOutlineWhilePainting);
}

bool KisConfig::hideSplashScreen() const
{
    KConfigGroup cfg(KGlobal::config(), "SplashScreen");
    return cfg.readEntry("HideSplashAfterStartup", true);
}

void KisConfig::setHideSplashScreen(bool hideSplashScreen) const
{
    KConfigGroup cfg(KGlobal::config(), "SplashScreen");
    cfg.writeEntry("HideSplashAfterStartup", hideSplashScreen);
}

qreal KisConfig::outlineSizeMinimum() const
{
    return m_cfg.readEntry("OutlineSizeMinimum", 1.0);
}

void KisConfig::setOutlineSizeMinimum(qreal outlineSizeMinimum) const
{
    m_cfg.writeEntry("OutlineSizeMinimum", outlineSizeMinimum);
}

int KisConfig::autoSaveInterval()  const
{
    return m_cfg.readEntry("AutoSaveInterval", KisDocument::defaultAutoSave());
}

void KisConfig::setAutoSaveInterval(int seconds)  const
{
    return m_cfg.writeEntry("AutoSaveInterval", seconds);
}

bool KisConfig::backupFile() const
{
    return m_cfg.readEntry("CreateBackupFile", true);
}

void KisConfig::setBackupFile(bool backupFile) const
{
    m_cfg.writeEntry("CreateBackupFile", backupFile);
}

bool KisConfig::showFilterGallery() const
{
    return m_cfg.readEntry("showFilterGallery", false);
}

void KisConfig::setShowFilterGallery(bool showFilterGallery) const
{
    m_cfg.writeEntry("showFilterGallery", showFilterGallery);
}

bool KisConfig::showFilterGalleryLayerMaskDialog() const
{
    return m_cfg.readEntry("showFilterGalleryLayerMaskDialog", true);
}

void KisConfig::setShowFilterGalleryLayerMaskDialog(bool showFilterGallery) const
{
    m_cfg.writeEntry("setShowFilterGalleryLayerMaskDialog", showFilterGallery);
}

QString KisConfig::defaultPainterlyColorModelId() const
{
    return m_cfg.readEntry("defaultpainterlycolormodel", "KS6");
}

void KisConfig::setDefaultPainterlyColorModelId(const QString& def) const
{
    m_cfg.writeEntry("defaultpainterlycolormodel", def);
}

QString KisConfig::defaultPainterlyColorDepthId() const
{
    return m_cfg.readEntry("defaultpainterlycolordepth", "F32");
}

void KisConfig::setDefaultPainterlyColorDepthId(const QString& def) const
{
    m_cfg.writeEntry("defaultpainterlycolordepth", def);
}

QString KisConfig::canvasState() const
{
    return m_cfg.readEntry("canvasState", "OPENGL_NOT_TRIED");
}

void KisConfig::setCanvasState(const QString& state) const
{
    static QStringList acceptableStates;
    if (acceptableStates.isEmpty()) {
        acceptableStates << "OPENGL_SUCCESS" << "TRY_OPENGL" << "OPENGL_NOT_TRIED" << "OPENGL_FAILED";
    }
    if (acceptableStates.contains(state)) {
        m_cfg.writeEntry("canvasState", state);
        m_cfg.sync();
    }
}

bool KisConfig::paintopPopupDetached() const
{
    return m_cfg.readEntry("PaintopPopupDetached", false);
}

void KisConfig::setPaintopPopupDetached(bool detached) const
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

void KisConfig::setPresetChooserViewMode(const int mode) const
{
    m_cfg.writeEntry("presetChooserViewMode", mode);
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


int KisConfig::hideDockersFullscreen() const
{
    return m_cfg.readEntry("hideDockersFullScreen", (int)Qt::Checked);
}

void KisConfig::setHideDockersFullscreen(const int value) const
{
    m_cfg.writeEntry("hideDockersFullScreen", value);
}

bool KisConfig::showDockerTitleBars() const
{
    return m_cfg.readEntry("showDockerTitleBars", true);
}

void KisConfig::setShowDockerTitleBars(const bool value) const
{
    m_cfg.writeEntry("showDockerTitleBars", value);
}

int KisConfig::hideMenuFullscreen() const
{
    return m_cfg.readEntry("hideMenuFullScreen", (int)Qt::Checked);
}

void KisConfig::setHideMenuFullscreen(const int value) const
{
    m_cfg.writeEntry("hideMenuFullScreen", value);
}

int KisConfig::hideScrollbarsFullscreen() const
{
    return m_cfg.readEntry("hideScrollbarsFullScreen", (int)Qt::Checked);
}

void KisConfig::setHideScrollbarsFullscreen(const int value) const
{
    m_cfg.writeEntry("hideScrollbarsFullScreen", value);
}

int KisConfig::hideStatusbarFullscreen() const
{
    return m_cfg.readEntry("hideStatusbarFullScreen", (int)Qt::Checked);
}

void KisConfig::setHideStatusbarFullscreen(const int value) const
{
    m_cfg.writeEntry("hideStatusbarFullScreen", value);
}

int KisConfig::hideTitlebarFullscreen() const
{
    return m_cfg.readEntry("hideTitleBarFullscreen", (int)Qt::Checked);
}

void KisConfig::setHideTitlebarFullscreen(const int value) const
{
    m_cfg.writeEntry("hideTitleBarFullscreen", value);
}

int KisConfig::hideToolbarFullscreen() const
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

void KisConfig::setFavoriteCompositeOps(const QStringList& compositeOps) const
{
    m_cfg.writeEntry("favoriteCompositeOps", compositeOps);
}

QString KisConfig::exportConfiguration(const QString &filterId) const
{
    return m_cfg.readEntry("ExportConfiguration-" + filterId, QString());
}

void KisConfig::setExportConfiguration(const QString &filterId, const KisPropertiesConfiguration &properties) const
{
    QString exportConfig = properties.toXML();
    m_cfg.writeEntry("ExportConfiguration-" + filterId, exportConfig);

}

bool KisConfig::useOcio() const
{
#ifdef HAVE_OCIO
    return m_cfg.readEntry("Krita/Ocio/UseOcio", false);
#else
    return false;
#endif
}

void KisConfig::setUseOcio(bool useOCIO) const
{
    m_cfg.writeEntry("Krita/Ocio/UseOcio", useOCIO);
}

int KisConfig::favoritePresets() const
{
    return m_cfg.readEntry("favoritePresets", 10);
}

void KisConfig::setFavoritePresets(const int value)
{
    m_cfg.writeEntry("favoritePresets", value);
}


KisConfig::OcioColorManagementMode
KisConfig::ocioColorManagementMode() const
{
    return (OcioColorManagementMode) m_cfg.readEntry("Krita/Ocio/OcioColorManagementMode", (int) INTERNAL);
}

void KisConfig::setOcioColorManagementMode(OcioColorManagementMode mode) const
{
    m_cfg.writeEntry("Krita/Ocio/OcioColorManagementMode", (int) mode);
}

QString KisConfig::ocioConfigurationPath() const
{
    return m_cfg.readEntry("Krita/Ocio/OcioConfigPath", QString());
}

void KisConfig::setOcioConfigurationPath(const QString &path) const
{
    m_cfg.writeEntry("Krita/Ocio/OcioConfigPath", path);
}

QString KisConfig::ocioLutPath() const
{
    return m_cfg.readEntry("Krita/Ocio/OcioLutPath", QString());
}

void KisConfig::setOcioLutPath(const QString &path) const
{
    m_cfg.writeEntry("Krita/Ocio/OcioLutPath", path);
}

int KisConfig::ocioLutEdgeSize() const
{
    return m_cfg.readEntry("Krita/Ocio/LutEdgeSize", 64);
}

void KisConfig::setOcioLutEdgeSize(int value)
{
    m_cfg.writeEntry("Krita/Ocio/LutEdgeSize", value);
}

bool KisConfig::ocioLockColorVisualRepresentation() const
{
    return m_cfg.readEntry("Krita/Ocio/OcioLockColorVisualRepresentation", false);
}

void KisConfig::setOcioLockColorVisualRepresentation(bool value)
{
    m_cfg.writeEntry("Krita/Ocio/OcioLockColorVisualRepresentation", value);
}

QString KisConfig::defaultPalette() const
{
    return m_cfg.readEntry("defaultPalette", QString());
}

void KisConfig::setDefaultPalette(const QString& name) const
{
    m_cfg.writeEntry("defaultPalette", name);
}

QString KisConfig::toolbarSlider(int sliderNumber)
{
    QString def = "flow";
    if (sliderNumber == 1) {
        def = "opacity";
    }
    if (sliderNumber == 2) {
        def = "size";
    }
    return m_cfg.readEntry(QString("toolbarslider_%1").arg(sliderNumber), def);
}

void KisConfig::setToolbarSlider(int sliderNumber, const QString &slider)
{
    m_cfg.writeEntry(QString("toolbarslider_%1").arg(sliderNumber), slider);
}

QString KisConfig::currentInputProfile() const
{
    return m_cfg.readEntry("currentInputProfile", QString());
}

void KisConfig::setCurrentInputProfile(const QString& name)
{
    m_cfg.writeEntry("currentInputProfile", name);
}

bool KisConfig::useSystemMonitorProfile() const
{
    return m_cfg.readEntry("ColorManagement/UseSystemMonitorProfile", false);
}

void KisConfig::setUseSystemMonitorProfile(bool _useSystemMonitorProfile) const
{
    m_cfg.writeEntry("ColorManagement/UseSystemMonitorProfile", _useSystemMonitorProfile);
}

bool KisConfig::presetStripVisible() const
{
    return m_cfg.readEntry("presetStripVisible", true);
}

void KisConfig::setPresetStripVisible(bool visible)
{
    m_cfg.writeEntry("presetStripVisible", visible);
}

bool KisConfig::scratchpadVisible() const
{
    return m_cfg.readEntry("scratchpadVisible", true);
}

void KisConfig::setScratchpadVisible(bool visible)
{
    m_cfg.writeEntry("scratchpadVisible", visible);
}


bool KisConfig::showSingleChannelAsColor() const
{
    return m_cfg.readEntry("showSingleChannelAsColor", false);
}

void KisConfig::setShowSingleChannelAsColor(bool asColor)
{
    m_cfg.writeEntry("showSingleChannelAsColor", asColor);
}

bool KisConfig::hidePopups() const
{
    return m_cfg.readEntry("hidePopups", false);
}

void KisConfig::setHidePopups(bool hidepopups)
{
    m_cfg.writeEntry("hidePopups", hidepopups);
}

int KisConfig::numDefaultLayers() const
{
    return m_cfg.readEntry("NumberOfLayersForNewImage", 2);
}

void KisConfig::setNumDefaultLayers(int num)
{
    m_cfg.writeEntry("NumberOfLayersForNewImage", num);
}

quint8 KisConfig::defaultBackgroundOpacity() const
{
  return m_cfg.readEntry("BackgroundOpacityForNewImage", (int)OPACITY_OPAQUE_U8);
}

void KisConfig::setDefaultBackgroundOpacity(quint8 value)
{
  m_cfg.writeEntry("BackgroundOpacityForNewImage", (int)value);
}

QColor KisConfig::defaultBackgroundColor() const
{
  return m_cfg.readEntry("BackgroundColorForNewImage", QColor(Qt::white)); 
}

void KisConfig::setDefaultBackgroundColor(QColor value) 
{
  m_cfg.writeEntry("BackgroundColorForNewImage", value);
}

KisConfig::BackgroundStyle KisConfig::defaultBackgroundStyle() const
{
  return (KisConfig::BackgroundStyle)m_cfg.readEntry("BackgroundStyleForNewImage", (int)LAYER);
}

void KisConfig::setDefaultBackgroundStyle(KisConfig::BackgroundStyle value)
{
  m_cfg.writeEntry("BackgroundStyleForNewImage", (int)value);
}

int KisConfig::lineSmoothingType() const
{
    return m_cfg.readEntry("LineSmoothingType", 1);
}

void KisConfig::setLineSmoothingType(int value)
{
    m_cfg.writeEntry("LineSmoothingType", value);
}

qreal KisConfig::lineSmoothingDistance() const
{
    return m_cfg.readEntry("LineSmoothingDistance", 50.0);
}

void KisConfig::setLineSmoothingDistance(qreal value)
{
    m_cfg.writeEntry("LineSmoothingDistance", value);
}

qreal KisConfig::lineSmoothingTailAggressiveness() const
{
    return m_cfg.readEntry("LineSmoothingTailAggressiveness", 0.15);
}

void KisConfig::setLineSmoothingTailAggressiveness(qreal value)
{
    m_cfg.writeEntry("LineSmoothingTailAggressiveness", value);
}

bool KisConfig::lineSmoothingSmoothPressure() const
{
    return m_cfg.readEntry("LineSmoothingSmoothPressure", false);
}

void KisConfig::setLineSmoothingSmoothPressure(bool value)
{
    m_cfg.writeEntry("LineSmoothingSmoothPressure", value);
}

bool KisConfig::lineSmoothingScalableDistance() const
{
    return m_cfg.readEntry("LineSmoothingScalableDistance", true);
}

void KisConfig::setLineSmoothingScalableDistance(bool value)
{
    m_cfg.writeEntry("LineSmoothingScalableDistance", value);
}

qreal KisConfig::lineSmoothingDelayDistance() const
{
    return m_cfg.readEntry("LineSmoothingDelayDistance", 50.0);
}

void KisConfig::setLineSmoothingDelayDistance(qreal value)
{
    m_cfg.writeEntry("LineSmoothingDelayDistance", value);
}

bool KisConfig::lineSmoothingUseDelayDistance() const
{
    return m_cfg.readEntry("LineSmoothingUseDelayDistance", true);
}

void KisConfig::setLineSmoothingUseDelayDistance(bool value)
{
    m_cfg.writeEntry("LineSmoothingUseDelayDistance", value);
}

bool KisConfig::lineSmoothingFinishStabilizedCurve() const
{
    return m_cfg.readEntry("LineSmoothingFinishStabilizedCurve", true);
}

void KisConfig::setLineSmoothingFinishStabilizedCurve(bool value)
{
    m_cfg.writeEntry("LineSmoothingFinishStabilizedCurve", value);
}

bool KisConfig::lineSmoothingStabilizeSensors() const
{
    return m_cfg.readEntry("LineSmoothingStabilizeSensors", true);
}

void KisConfig::setLineSmoothingStabilizeSensors(bool value)
{
    m_cfg.writeEntry("LineSmoothingStabilizeSensors", value);
}

int KisConfig::paletteDockerPaletteViewSectionSize() const
{
    return m_cfg.readEntry("paletteDockerPaletteViewSectionSize", 12);
}

void KisConfig::setPaletteDockerPaletteViewSectionSize(int value) const
{
    m_cfg.writeEntry("paletteDockerPaletteViewSectionSize", value);
}

int KisConfig::tabletEventsDelay() const
{
    return m_cfg.readEntry("tabletEventsDelay", 10);
}

void KisConfig::setTabletEventsDelay(int value)
{
    m_cfg.writeEntry("tabletEventsDelay", value);
}

bool KisConfig::testingAcceptCompressedTabletEvents() const
{
    return m_cfg.readEntry("testingAcceptCompressedTabletEvents", false);
}

void KisConfig::setTestingAcceptCompressedTabletEvents(bool value)
{
    m_cfg.writeEntry("testingAcceptCompressedTabletEvents", value);
}

bool KisConfig::testingCompressBrushEvents() const
{
    return m_cfg.readEntry("testingCompressBrushEvents", false);
}

void KisConfig::setTestingCompressBrushEvents(bool value)
{
    m_cfg.writeEntry("testingCompressBrushEvents", value);
}

bool KisConfig::useVerboseOpenGLDebugOutput() const
{
    return m_cfg.readEntry("useVerboseOpenGLDebugOutput", false);
}

int KisConfig::workaroundX11SmoothPressureSteps() const
{
    return m_cfg.readEntry("workaroundX11SmoothPressureSteps", 0);
}

bool KisConfig::showCanvasMessages() const
{
    return m_cfg.readEntry("showOnCanvasMessages", true);
}
void KisConfig::setShowCanvasMessages(bool show)
{
    m_cfg.writeEntry("showOnCanvasMessages", show);
}

const KoColorSpace* KisConfig::customColorSelectorColorSpace() const
{
    const KoColorSpace *cs = 0;

    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");
    if(cfg.readEntry("useCustomColorSpace", true)) {
        KoColorSpaceRegistry* csr = KoColorSpaceRegistry::instance();
        cs = csr->colorSpace(cfg.readEntry("customColorSpaceModel", "RGBA"),
                             cfg.readEntry("customColorSpaceDepthID", "U8"),
                             cfg.readEntry("customColorSpaceProfile", "sRGB built-in - (lcms internal)"));
    }

    return cs;
}

void KisConfig::setCustomColorSelectorColorSpace(const KoColorSpace *cs)
{
    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");
    cfg.writeEntry("useCustomColorSpace", bool(cs));
    if(cs) {
        cfg.writeEntry("customColorSpaceModel", cs->colorModelId().id());
        cfg.writeEntry("customColorSpaceDepthID", cs->colorDepthId().id());
        cfg.writeEntry("customColorSpaceProfile", cs->profile()->name());
    }

    KisConfigNotifier::instance()->notifyConfigChanged();
}
