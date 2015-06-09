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

KisConfig::KisConfig()
    : m_cfg(KGlobal::config()->group(""))
{
}

KisConfig::~KisConfig()
{
    if (qApp->thread() != QThread::currentThread()) {
        qDebug() << "WARNING: KisConfig: requested config synchronization from nonGUI thread! Skipping...";
        return;
    }

    m_cfg.sync();
}


bool KisConfig::disableTouchOnCanvas(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("disableTouchOnCanvas", false));
}

void KisConfig::setDisableTouchOnCanvas(bool value) const
{
    m_cfg.writeEntry("disableTouchOnCanvas", value);
}

bool KisConfig::useProjections(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("useProjections", true));
}

void KisConfig::setUseProjections(bool useProj) const
{
    m_cfg.writeEntry("useProjections", useProj);
}

bool KisConfig::undoEnabled(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("undoEnabled", true));
}

void KisConfig::setUndoEnabled(bool undo) const
{
    m_cfg.writeEntry("undoEnabled", undo);
}

int KisConfig::undoStackLimit(bool defaultValue) const
{
    return (defaultValue ? 30 : m_cfg.readEntry("undoStackLimit", 30));
}

void KisConfig::setUndoStackLimit(int limit) const
{
    m_cfg.writeEntry("undoStackLimit", limit);
}

bool KisConfig::useCumulativeUndoRedo(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("useCumulativeUndoRedo",false));
}

void KisConfig::setCumulativeUndoRedo(bool value)
{
    m_cfg.writeEntry("useCumulativeUndoRedo", value);
}

double KisConfig::stackT1(bool defaultValue) const
{
     return (defaultValue ? 5 : m_cfg.readEntry("stackT1",5));
}

void KisConfig::setStackT1(int T1)
{
    m_cfg.writeEntry("stackT1", T1);
}

double KisConfig::stackT2(bool defaultValue) const
{
     return (defaultValue ? 1 : m_cfg.readEntry("stackT2",1));
}

void KisConfig::setStackT2(int T2)
{
    m_cfg.writeEntry("stackT2", T2);
}

int KisConfig::stackN(bool defaultValue) const
{
    return (defaultValue ? 5 : m_cfg.readEntry("stackN",5));
}

void KisConfig::setStackN(int N)
{
     m_cfg.writeEntry("stackN", N);
}

qint32 KisConfig::defImageWidth(bool defaultValue) const
{
    return (defaultValue ? 1600 : m_cfg.readEntry("imageWidthDef", 1600));
}

qint32 KisConfig::defImageHeight(bool defaultValue) const
{
    return (defaultValue ? 1200 : m_cfg.readEntry("imageHeightDef", 1200));
}

double KisConfig::defImageResolution(bool defaultValue) const
{
    return (defaultValue ? 100.0 : m_cfg.readEntry("imageResolutionDef", 100.0)) / 72.0;
}

QString KisConfig::defColorModel(bool defaultValue) const
{
    return (defaultValue ? KoColorSpaceRegistry::instance()->rgb8()->colorModelId().id()
                        : m_cfg.readEntry("colorModelDef", KoColorSpaceRegistry::instance()->rgb8()->colorModelId().id()));
}

void KisConfig::defColorModel(const QString & model) const
{
    m_cfg.writeEntry("colorModelDef", model);
}

QString KisConfig::defaultColorDepth(bool defaultValue) const
{
    return (defaultValue ? KoColorSpaceRegistry::instance()->rgb8()->colorDepthId().id()
                        : m_cfg.readEntry("colorDepthDef", KoColorSpaceRegistry::instance()->rgb8()->colorDepthId().id()));
}

void KisConfig::setDefaultColorDepth(const QString & depth) const
{
    m_cfg.writeEntry("colorDepthDef", depth);
}

QString KisConfig::defColorProfile(bool defaultValue) const
{
    return (defaultValue ? KoColorSpaceRegistry::instance()->rgb8()->profile()->name() :
                           m_cfg.readEntry("colorProfileDef",
                                           KoColorSpaceRegistry::instance()->rgb8()->profile()->name()));
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

void cleanOldCursorStyleKeys(KConfigGroup &cfg)
{
    if (cfg.hasKey("newCursorStyle") &&
        cfg.hasKey("newOutlineStyle")) {

        cfg.deleteEntry("cursorStyleDef");
    }
}

CursorStyle KisConfig::newCursorStyle(bool defaultValue) const
{
    if (defaultValue) {
        return CURSOR_STYLE_NO_CURSOR;
    }


    int style = m_cfg.readEntry("newCursorStyle", int(-1));

    if (style < 0) {
        // old style format
        style = m_cfg.readEntry("cursorStyleDef", int(OLD_CURSOR_STYLE_OUTLINE));

        switch (style) {
        case OLD_CURSOR_STYLE_TOOLICON:
            style = CURSOR_STYLE_TOOLICON;
            break;
        case OLD_CURSOR_STYLE_CROSSHAIR:
        case OLD_CURSOR_STYLE_OUTLINE_CENTER_CROSS:
            style = CURSOR_STYLE_CROSSHAIR;
            break;
        case OLD_CURSOR_STYLE_POINTER:
            style = CURSOR_STYLE_POINTER;
            break;
        case OLD_CURSOR_STYLE_OUTLINE:
        case OLD_CURSOR_STYLE_NO_CURSOR:
            style = CURSOR_STYLE_NO_CURSOR;
            break;
        case OLD_CURSOR_STYLE_SMALL_ROUND:
        case OLD_CURSOR_STYLE_OUTLINE_CENTER_DOT:
            style = CURSOR_STYLE_SMALL_ROUND;
            break;
        case OLD_CURSOR_STYLE_TRIANGLE_RIGHTHANDED:
        case OLD_CURSOR_STYLE_OUTLINE_TRIANGLE_RIGHTHANDED:
            style = CURSOR_STYLE_TRIANGLE_RIGHTHANDED;
            break;
        case OLD_CURSOR_STYLE_TRIANGLE_LEFTHANDED:
        case OLD_CURSOR_STYLE_OUTLINE_TRIANGLE_LEFTHANDED:
            style = CURSOR_STYLE_TRIANGLE_LEFTHANDED;
            break;
        default:
            style = -1;
        }
    }

    cleanOldCursorStyleKeys(m_cfg);

    // compatibility with future versions
    if (style < 0 || style >= N_CURSOR_STYLE_SIZE) {
        style = CURSOR_STYLE_NO_CURSOR;
    }

    return (CursorStyle) style;
}

void KisConfig::setNewCursorStyle(CursorStyle style)
{
    m_cfg.writeEntry("newCursorStyle", (int)style);
}

OutlineStyle KisConfig::newOutlineStyle(bool defaultValue) const
{
    if (defaultValue) {
        return OUTLINE_FULL;
    }

    int style = m_cfg.readEntry("newOutlineStyle", int(-1));

    if (style < 0) {
        // old style format
        style = m_cfg.readEntry("cursorStyleDef", int(OLD_CURSOR_STYLE_OUTLINE));

        switch (style) {
        case OLD_CURSOR_STYLE_TOOLICON:
        case OLD_CURSOR_STYLE_CROSSHAIR:
        case OLD_CURSOR_STYLE_POINTER:
        case OLD_CURSOR_STYLE_NO_CURSOR:
        case OLD_CURSOR_STYLE_SMALL_ROUND:
        case OLD_CURSOR_STYLE_TRIANGLE_RIGHTHANDED:
        case OLD_CURSOR_STYLE_TRIANGLE_LEFTHANDED:
            style = OUTLINE_NONE;
            break;
        case OLD_CURSOR_STYLE_OUTLINE:
        case OLD_CURSOR_STYLE_OUTLINE_CENTER_DOT:
        case OLD_CURSOR_STYLE_OUTLINE_CENTER_CROSS:
        case OLD_CURSOR_STYLE_OUTLINE_TRIANGLE_RIGHTHANDED:
        case OLD_CURSOR_STYLE_OUTLINE_TRIANGLE_LEFTHANDED:
            style = OUTLINE_FULL;
            break;
        default:
            style = -1;
        }
    }

    cleanOldCursorStyleKeys(m_cfg);

    // compatibility with future versions
    if (style < 0 || style >= N_OUTLINE_STYLE_SIZE) {
        style = OUTLINE_FULL;
    }

    return (OutlineStyle) style;
}

void KisConfig::setNewOutlineStyle(OutlineStyle style)
{
    m_cfg.writeEntry("newOutlineStyle", (int)style);
}

QRect KisConfig::colorPreviewRect() const
{
    return m_cfg.readEntry("colorPreviewRect", QVariant(QRect(32, 32, 48, 48))).toRect();
}

void KisConfig::setColorPreviewRect(const QRect &rect)
{
    m_cfg.writeEntry("colorPreviewRect", QVariant(rect));
}

bool KisConfig::useDirtyPresets(bool defaultValue) const
{
   return (defaultValue ? false : m_cfg.readEntry("useDirtyPresets",false));
}
void KisConfig::setUseDirtyPresets(bool value)
{
    m_cfg.writeEntry("useDirtyPresets",value);
    KisConfigNotifier::instance()->notifyConfigChanged();
}

bool KisConfig::useEraserBrushSize(bool defaultValue) const
{
   return (defaultValue ? false : m_cfg.readEntry("useEraserBrushSize",false));
}

void KisConfig::setUseEraserBrushSize(bool value)
{
    m_cfg.writeEntry("useEraserBrushSize",value);
    KisConfigNotifier::instance()->notifyConfigChanged();
}

QColor KisConfig::getMDIBackgroundColor(bool defaultValue) const
{
    QColor col(77, 77, 77);
    return (defaultValue ? col : m_cfg.readEntry("mdiBackgroundColor", col));
}

void KisConfig::setMDIBackgroundColor(const QColor &v) const
{
    m_cfg.writeEntry("mdiBackgroundColor", v);
}

QString KisConfig::getMDIBackgroundImage(bool defaultValue) const
{
    return (defaultValue ? "" : m_cfg.readEntry("mdiBackgroundImage", ""));
}

void KisConfig::setMDIBackgroundImage(const QString &filename) const
{
    m_cfg.writeEntry("mdiBackgroundImage", filename);
}

QString KisConfig::monitorProfile(int screen) const
{
    // Note: keep this in sync with the default profile for the RGB colorspaces!
    QString profile = m_cfg.readEntry("monitorProfile" + QString(screen == 0 ? "": QString("_%1").arg(screen)), "sRGB-elle-V2-srgbtrc.icc");
    //qDebug() << "KisConfig::monitorProfile()" << profile;
    return profile;
}

QString KisConfig::monitorForScreen(int screen, const QString &defaultMonitor, bool defaultValue) const
{
    return (defaultValue ? defaultMonitor
                         : m_cfg.readEntry(QString("monitor_for_screen_%1").arg(screen), defaultMonitor));
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

QString KisConfig::workingColorSpace(bool defaultValue) const
{
    return (defaultValue ? "RGBA" : m_cfg.readEntry("workingColorSpace", "RGBA"));
}

void KisConfig::setWorkingColorSpace(const QString & workingColorSpace) const
{
    m_cfg.writeEntry("workingColorSpace", workingColorSpace);
}

QString KisConfig::printerColorSpace(bool /*defaultValue*/) const
{
    //TODO currently only rgb8 is supported
    //return (defaultValue ? "RGBA" : m_cfg.readEntry("printerColorSpace", "RGBA"));
    return QString("RGBA");
}

void KisConfig::setPrinterColorSpace(const QString & printerColorSpace) const
{
    m_cfg.writeEntry("printerColorSpace", printerColorSpace);
}


QString KisConfig::printerProfile(bool defaultValue) const
{
    return (defaultValue ? "" : m_cfg.readEntry("printerProfile", ""));
}

void KisConfig::setPrinterProfile(const QString & printerProfile) const
{
    m_cfg.writeEntry("printerProfile", printerProfile);
}


bool KisConfig::useBlackPointCompensation(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("useBlackPointCompensation", true));
}

void KisConfig::setUseBlackPointCompensation(bool useBlackPointCompensation) const
{
    m_cfg.writeEntry("useBlackPointCompensation", useBlackPointCompensation);
}

bool KisConfig::allowLCMSOptimization(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("allowLCMSOptimization", true));
}

void KisConfig::setAllowLCMSOptimization(bool allowLCMSOptimization)
{
    m_cfg.writeEntry("allowLCMSOptimization", allowLCMSOptimization);
}


bool KisConfig::showRulers(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("showrulers", false));
}

void KisConfig::setShowRulers(bool rulers) const
{
    m_cfg.writeEntry("showrulers", rulers);
}


qint32 KisConfig::pasteBehaviour(bool defaultValue) const
{
    return (defaultValue ? 2 : m_cfg.readEntry("pasteBehaviour", 2));
}

void KisConfig::setPasteBehaviour(qint32 renderIntent) const
{
    m_cfg.writeEntry("pasteBehaviour", renderIntent);
}


qint32 KisConfig::monitorRenderIntent(bool defaultValue) const
{
    qint32 intent = m_cfg.readEntry("renderIntent", INTENT_PERCEPTUAL);
    if (intent > 3) intent = 3;
    if (intent < 0) intent = 0;
    return (defaultValue ? INTENT_PERCEPTUAL : intent);
}

void KisConfig::setRenderIntent(qint32 renderIntent) const
{
    if (renderIntent > 3) renderIntent = 3;
    if (renderIntent < 0) renderIntent = 0;
    m_cfg.writeEntry("renderIntent", renderIntent);
}

bool KisConfig::useOpenGL(bool defaultValue) const
{

    if (qApp->applicationName() == "krita") {
        if (defaultValue) {
#ifdef Q_WS_MAC
            return false;
#else
            return true;
#endif
        }

        //qDebug() << "use opengl" << m_cfg.readEntry("useOpenGL", true) << "success" << m_cfg.readEntry("canvasState", "OPENGL_SUCCESS");
        QString canvasState = m_cfg.readEntry("canvasState", "OPENGL_SUCCESS");
#ifdef Q_WS_MAC
        return (m_cfg.readEntry("useOpenGL", false) && (canvasState == "OPENGL_SUCCESS" || canvasState == "TRY_OPENGL"));
#else
        return (m_cfg.readEntry("useOpenGL", true) && (canvasState == "OPENGL_SUCCESS" || canvasState == "TRY_OPENGL"));
#endif
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

int KisConfig::openGLFilteringMode(bool defaultValue) const
{
    return (defaultValue ? 3 : m_cfg.readEntry("OpenGLFilterMode", 3));
}

void KisConfig::setOpenGLFilteringMode(int filteringMode)
{
    m_cfg.writeEntry("OpenGLFilterMode", filteringMode);
}

bool KisConfig::useOpenGLTextureBuffer(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("useOpenGLTextureBuffer", true));
}

void KisConfig::setUseOpenGLTextureBuffer(bool useBuffer)
{
    m_cfg.writeEntry("useOpenGLTextureBuffer", useBuffer);
}

int KisConfig::openGLTextureSize(bool defaultValue) const
{
    return (defaultValue ? 256 : m_cfg.readEntry("textureSize", 256));
}


bool KisConfig::disableDoubleBuffering(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("disableDoubleBuffering", true));
}

void KisConfig::setDisableDoubleBuffering(bool disableDoubleBuffering)
{
    m_cfg.writeEntry("disableDoubleBuffering", disableDoubleBuffering);
}

bool KisConfig::disableVSync(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("disableVSync", true));
}

void KisConfig::setDisableVSync(bool disableVSync)
{
    m_cfg.writeEntry("disableVSync", disableVSync);
}

bool KisConfig::showAdvancedOpenGLSettings(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("showAdvancedOpenGLSettings", false));
}

bool KisConfig::forceOpenGLFenceWorkaround(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("forceOpenGLFenceWorkaround", false));
}

int KisConfig::numMipmapLevels(bool defaultValue) const
{
    return (defaultValue ? 4 : m_cfg.readEntry("numMipmapLevels", 4));
}

int KisConfig::textureOverlapBorder() const
{
    return 1 << qMax(0, numMipmapLevels());
}

qint32 KisConfig::maxNumberOfThreads(bool defaultValue) const
{
    return (defaultValue ? QThread::idealThreadCount() : m_cfg.readEntry("maxthreads", QThread::idealThreadCount()));
}

void KisConfig::setMaxNumberOfThreads(qint32 maxThreads)
{
    m_cfg.writeEntry("maxthreads", maxThreads);
}

quint32 KisConfig::getGridMainStyle(bool defaultValue) const
{
    quint32 v = m_cfg.readEntry("gridmainstyle", 0);
    if (v > 2)
        v = 2;
    return (defaultValue ? 0 : v);
}

void KisConfig::setGridMainStyle(quint32 v) const
{
    m_cfg.writeEntry("gridmainstyle", v);
}

quint32 KisConfig::getGridSubdivisionStyle(bool defaultValue) const
{
    quint32 v = m_cfg.readEntry("gridsubdivisionstyle", 1);
    if (v > 2) v = 2;
    return (defaultValue ? 1 : v);
}

void KisConfig::setGridSubdivisionStyle(quint32 v) const
{
    m_cfg.writeEntry("gridsubdivisionstyle", v);
}

QColor KisConfig::getGridMainColor(bool defaultValue) const
{
    QColor col(99, 99, 99);
    return (defaultValue ? col : m_cfg.readEntry("gridmaincolor", col));
}

void KisConfig::setGridMainColor(const QColor & v) const
{
    m_cfg.writeEntry("gridmaincolor", v);
}

QColor KisConfig::getGridSubdivisionColor(bool defaultValue) const
{
    QColor col(150, 150, 150);
    return (defaultValue ? col : m_cfg.readEntry("gridsubdivisioncolor", col));
}

void KisConfig::setGridSubdivisionColor(const QColor & v) const
{
    m_cfg.writeEntry("gridsubdivisioncolor", v);
}

quint32 KisConfig::getGridHSpacing(bool defaultValue) const
{
    qint32 v = m_cfg.readEntry("gridhspacing", 10);
    return (defaultValue ? 10 : (quint32)qMax(1, v));
}

void KisConfig::setGridHSpacing(quint32 v) const
{
    m_cfg.writeEntry("gridhspacing", v);
}

quint32 KisConfig::getGridVSpacing(bool defaultValue) const
{
    qint32 v = m_cfg.readEntry("gridvspacing", 10);
    return (defaultValue ? 10 : (quint32)qMax(1, v));
}

void KisConfig::setGridVSpacing(quint32 v) const
{
    m_cfg.writeEntry("gridvspacing", v);
}

bool KisConfig::getGridSpacingAspect(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("gridspacingaspect", false));
}

void KisConfig::setGridSpacingAspect(bool v) const
{
    m_cfg.writeEntry("gridspacingaspect", v);
}

quint32 KisConfig::getGridSubdivisions(bool defaultValue) const
{
    qint32 v = m_cfg.readEntry("gridsubsivisons", 2);
    return (defaultValue ? 2 : (quint32)qMax(1, v));
}

void KisConfig::setGridSubdivisions(quint32 v) const
{
    m_cfg.writeEntry("gridsubsivisons", v);
}

quint32 KisConfig::getGridOffsetX(bool defaultValue) const
{
    qint32 v = m_cfg.readEntry("gridoffsetx", 0);
    return (defaultValue ? 0 : (quint32)qMax(0, v));
}

void KisConfig::setGridOffsetX(quint32 v) const
{
    m_cfg.writeEntry("gridoffsetx", v);
}

quint32 KisConfig::getGridOffsetY(bool defaultValue) const
{
    qint32 v = m_cfg.readEntry("gridoffsety", 0);
    return (defaultValue ? 0 : (quint32)qMax(0, v));
}

void KisConfig::setGridOffsetY(quint32 v) const
{
    m_cfg.writeEntry("gridoffsety", v);
}

bool KisConfig::getGridOffsetAspect(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("gridoffsetaspect", false));
}

void KisConfig::setGridOffsetAspect(bool v) const
{
    m_cfg.writeEntry("gridoffsetaspect", v);
}

qint32 KisConfig::checkSize(bool defaultValue) const
{
    return (defaultValue ? 32 : m_cfg.readEntry("checksize", 32));
}

void KisConfig::setCheckSize(qint32 checksize) const
{
    m_cfg.writeEntry("checksize", checksize);
}

bool KisConfig::scrollCheckers(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("scrollingcheckers", false));
}

void KisConfig::setScrollingCheckers(bool sc) const
{
    m_cfg.writeEntry("scrollingcheckers", sc);
}

QColor KisConfig::canvasBorderColor(bool defaultValue) const
{
    QColor color(QColor(128,128,128));
    return (defaultValue ? color : m_cfg.readEntry("canvasBorderColor", color));
}

void KisConfig::setCanvasBorderColor(const QColor& color) const
{
    m_cfg.writeEntry("canvasBorderColor", color);
}

bool KisConfig::hideScrollbars(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("hideScrollbars", false));
}

void KisConfig::setHideScrollbars(bool value) const
{
    m_cfg.writeEntry("hideScrollbars", value);
}


QColor KisConfig::checkersColor1(bool defaultValue) const
{
    QColor col(220, 220, 220);
    return (defaultValue ? col : m_cfg.readEntry("checkerscolor", col));
}

void KisConfig::setCheckersColor1(const QColor & v) const
{
    m_cfg.writeEntry("checkerscolor", v);
}

QColor KisConfig::checkersColor2(bool defaultValue) const
{
    return (defaultValue ? QColor(Qt::white) : m_cfg.readEntry("checkerscolor2", QColor(Qt::white)));
}

void KisConfig::setCheckersColor2(const QColor & v) const
{
    m_cfg.writeEntry("checkerscolor2", v);
}

bool KisConfig::antialiasCurves(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("antialiascurves", true));
}

void KisConfig::setAntialiasCurves(bool v) const
{
    m_cfg.writeEntry("antialiascurves", v);
}

QColor KisConfig::selectionOverlayMaskColor(bool defaultValue) const
{
    QColor def(255, 0, 0, 220);
    return (defaultValue ? def : m_cfg.readEntry("selectionOverlayMaskColor", def));
}

void KisConfig::setSelectionOverlayMaskColor(const QColor &color)
{
    m_cfg.writeEntry("selectionOverlayMaskColor", color);
}

bool KisConfig::antialiasSelectionOutline(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("AntialiasSelectionOutline", false));
}

void KisConfig::setAntialiasSelectionOutline(bool v) const
{
    m_cfg.writeEntry("AntialiasSelectionOutline", v);
}

bool KisConfig::showRootLayer(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("ShowRootLayer", false));
}

void KisConfig::setShowRootLayer(bool showRootLayer) const
{
    m_cfg.writeEntry("ShowRootLayer", showRootLayer);
}

bool KisConfig::showGlobalSelection(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("ShowGlobalSelection", false));
}

void KisConfig::setShowGlobalSelection(bool showGlobalSelection) const
{
    m_cfg.writeEntry("ShowGlobalSelection", showGlobalSelection);
}

bool KisConfig::showOutlineWhilePainting(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("ShowOutlineWhilePainting", true));
}

void KisConfig::setShowOutlineWhilePainting(bool showOutlineWhilePainting) const
{
    m_cfg.writeEntry("ShowOutlineWhilePainting", showOutlineWhilePainting);
}

bool KisConfig::hideSplashScreen(bool defaultValue) const
{
    KConfigGroup cfg(KGlobal::config(), "SplashScreen");
    return (defaultValue ? true : cfg.readEntry("HideSplashAfterStartup", true));
}

void KisConfig::setHideSplashScreen(bool hideSplashScreen) const
{
    KConfigGroup cfg(KGlobal::config(), "SplashScreen");
    cfg.writeEntry("HideSplashAfterStartup", hideSplashScreen);
}

qreal KisConfig::outlineSizeMinimum(bool defaultValue) const
{
    return (defaultValue ? 1.0 : m_cfg.readEntry("OutlineSizeMinimum", 1.0));
}

void KisConfig::setOutlineSizeMinimum(qreal outlineSizeMinimum) const
{
    m_cfg.writeEntry("OutlineSizeMinimum", outlineSizeMinimum);
}

int KisConfig::autoSaveInterval(bool defaultValue)  const
{
    return (defaultValue ? KisDocument::defaultAutoSave() : m_cfg.readEntry("AutoSaveInterval", KisDocument::defaultAutoSave()));
}

void KisConfig::setAutoSaveInterval(int seconds)  const
{
    return m_cfg.writeEntry("AutoSaveInterval", seconds);
}

bool KisConfig::backupFile(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("CreateBackupFile", true));
}

void KisConfig::setBackupFile(bool backupFile) const
{
    m_cfg.writeEntry("CreateBackupFile", backupFile);
}

bool KisConfig::showFilterGallery(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("showFilterGallery", false));
}

void KisConfig::setShowFilterGallery(bool showFilterGallery) const
{
    m_cfg.writeEntry("showFilterGallery", showFilterGallery);
}

bool KisConfig::showFilterGalleryLayerMaskDialog(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("showFilterGalleryLayerMaskDialog", true));
}

void KisConfig::setShowFilterGalleryLayerMaskDialog(bool showFilterGallery) const
{
    m_cfg.writeEntry("setShowFilterGalleryLayerMaskDialog", showFilterGallery);
}

QString KisConfig::canvasState(bool defaultValue) const
{
    return (defaultValue ? "OPENGL_NOT_TRIED" : m_cfg.readEntry("canvasState", "OPENGL_NOT_TRIED"));
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

bool KisConfig::paintopPopupDetached(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("PaintopPopupDetached", false));
}

void KisConfig::setPaintopPopupDetached(bool detached) const
{
    m_cfg.writeEntry("PaintopPopupDetached", detached);
}

QString KisConfig::pressureTabletCurve(bool defaultValue) const
{
    return (defaultValue ? "0,0;1,1" : m_cfg.readEntry("tabletPressureCurve","0,0;1,1;"));
}

void KisConfig::setPressureTabletCurve(const QString& curveString) const
{
    m_cfg.writeEntry("tabletPressureCurve", curveString);
}

qreal KisConfig::vastScrolling(bool defaultValue) const
{
    return (defaultValue ? 0.9 : m_cfg.readEntry("vastScrolling", 0.9));
}

void KisConfig::setVastScrolling(const qreal factor) const
{
    m_cfg.writeEntry("vastScrolling", factor);
}

int KisConfig::presetChooserViewMode(bool defaultValue) const
{
    return (defaultValue ? 0 : m_cfg.readEntry("presetChooserViewMode", 0));
}

void KisConfig::setPresetChooserViewMode(const int mode) const
{
    m_cfg.writeEntry("presetChooserViewMode", mode);
}

bool KisConfig::firstRun(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("firstRun", true));
}

void KisConfig::setFirstRun(const bool first) const
{
    m_cfg.writeEntry("firstRun", first);
}

int KisConfig::horizontalSplitLines(bool defaultValue) const
{
    return (defaultValue ? 1 : m_cfg.readEntry("horizontalSplitLines", 1));
}
void KisConfig::setHorizontalSplitLines(const int numberLines) const
{
    m_cfg.writeEntry("horizontalSplitLines", numberLines);
}

int KisConfig::verticalSplitLines(bool defaultValue) const
{
    return (defaultValue ? 1 : m_cfg.readEntry("verticalSplitLines", 1));
}

void KisConfig::setVerticalSplitLines(const int numberLines) const
{
    m_cfg.writeEntry("verticalSplitLines", numberLines);
}

bool KisConfig::clicklessSpacePan(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("clicklessSpacePan", true));
}

void KisConfig::setClicklessSpacePan(const bool toggle) const
{
    m_cfg.writeEntry("clicklessSpacePan", toggle);
}


bool KisConfig::hideDockersFullscreen(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("hideDockersFullScreen", true));
}

void KisConfig::setHideDockersFullscreen(const bool value) const
{
    m_cfg.writeEntry("hideDockersFullScreen", value);
}

bool KisConfig::showDockerTitleBars(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("showDockerTitleBars", true));
}

void KisConfig::setShowDockerTitleBars(const bool value) const
{
    m_cfg.writeEntry("showDockerTitleBars", value);
}

bool KisConfig::hideMenuFullscreen(bool defaultValue) const
{
    return (defaultValue ? true: m_cfg.readEntry("hideMenuFullScreen", true));
}

void KisConfig::setHideMenuFullscreen(const bool value) const
{
    m_cfg.writeEntry("hideMenuFullScreen", value);
}

bool KisConfig::hideScrollbarsFullscreen(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("hideScrollbarsFullScreen", true));
}

void KisConfig::setHideScrollbarsFullscreen(const bool value) const
{
    m_cfg.writeEntry("hideScrollbarsFullScreen", value);
}

bool KisConfig::hideStatusbarFullscreen(bool defaultValue) const
{
    return (defaultValue ? true: m_cfg.readEntry("hideStatusbarFullScreen", true));
}

void KisConfig::setHideStatusbarFullscreen(const bool value) const
{
    m_cfg.writeEntry("hideStatusbarFullScreen", value);
}

bool KisConfig::hideTitlebarFullscreen(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("hideTitleBarFullscreen", true));
}

void KisConfig::setHideTitlebarFullscreen(const bool value) const
{
    m_cfg.writeEntry("hideTitleBarFullscreen", value);
}

bool KisConfig::hideToolbarFullscreen(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("hideToolbarFullscreen", true));
}

void KisConfig::setHideToolbarFullscreen(const bool value) const
{
    m_cfg.writeEntry("hideToolbarFullscreen", value);
}

QStringList KisConfig::favoriteCompositeOps(bool defaultValue) const
{
    return (defaultValue ? QStringList() : m_cfg.readEntry("favoriteCompositeOps", QStringList()));
}

void KisConfig::setFavoriteCompositeOps(const QStringList& compositeOps) const
{
    m_cfg.writeEntry("favoriteCompositeOps", compositeOps);
}

QString KisConfig::exportConfiguration(const QString &filterId, bool defaultValue) const
{
    return (defaultValue ? QString() : m_cfg.readEntry("ExportConfiguration-" + filterId, QString()));
}

void KisConfig::setExportConfiguration(const QString &filterId, const KisPropertiesConfiguration &properties) const
{
    QString exportConfig = properties.toXML();
    m_cfg.writeEntry("ExportConfiguration-" + filterId, exportConfig);

}

bool KisConfig::useOcio(bool defaultValue) const
{
#ifdef HAVE_OCIO
    return (defaultValue ? false : m_cfg.readEntry("Krita/Ocio/UseOcio", false));
#else
    return false;
#endif
}

void KisConfig::setUseOcio(bool useOCIO) const
{
    m_cfg.writeEntry("Krita/Ocio/UseOcio", useOCIO);
}

int KisConfig::favoritePresets(bool defaultValue) const
{
    return (defaultValue ? 10 : m_cfg.readEntry("numFavoritePresets", 10));
}

void KisConfig::setFavoritePresets(const int value)
{
    m_cfg.writeEntry("numFavoritePresets", value);
}


KisConfig::OcioColorManagementMode
KisConfig::ocioColorManagementMode(bool defaultValue) const
{
    return (OcioColorManagementMode)(defaultValue ? INTERNAL
                                                  : m_cfg.readEntry("Krita/Ocio/OcioColorManagementMode", (int) INTERNAL));
}

void KisConfig::setOcioColorManagementMode(OcioColorManagementMode mode) const
{
    m_cfg.writeEntry("Krita/Ocio/OcioColorManagementMode", (int) mode);
}

QString KisConfig::ocioConfigurationPath(bool defaultValue) const
{
    return (defaultValue ? QString() : m_cfg.readEntry("Krita/Ocio/OcioConfigPath", QString()));
}

void KisConfig::setOcioConfigurationPath(const QString &path) const
{
    m_cfg.writeEntry("Krita/Ocio/OcioConfigPath", path);
}

QString KisConfig::ocioLutPath(bool defaultValue) const
{
    return (defaultValue ? QString() : m_cfg.readEntry("Krita/Ocio/OcioLutPath", QString()));
}

void KisConfig::setOcioLutPath(const QString &path) const
{
    m_cfg.writeEntry("Krita/Ocio/OcioLutPath", path);
}

int KisConfig::ocioLutEdgeSize(bool defaultValue) const
{
    return (defaultValue ? 64 : m_cfg.readEntry("Krita/Ocio/LutEdgeSize", 64));
}

void KisConfig::setOcioLutEdgeSize(int value)
{
    m_cfg.writeEntry("Krita/Ocio/LutEdgeSize", value);
}

bool KisConfig::ocioLockColorVisualRepresentation(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("Krita/Ocio/OcioLockColorVisualRepresentation", false));
}

void KisConfig::setOcioLockColorVisualRepresentation(bool value)
{
    m_cfg.writeEntry("Krita/Ocio/OcioLockColorVisualRepresentation", value);
}

QString KisConfig::defaultPalette(bool defaultValue) const
{
    return (defaultValue ? QString() : m_cfg.readEntry("defaultPalette", QString()));
}

void KisConfig::setDefaultPalette(const QString& name) const
{
    m_cfg.writeEntry("defaultPalette", name);
}

QString KisConfig::toolbarSlider(int sliderNumber, bool defaultValue) const
{
    QString def = "flow";
    if (sliderNumber == 1) {
        def = "opacity";
    }
    if (sliderNumber == 2) {
        def = "size";
    }
    return (defaultValue ? def : m_cfg.readEntry(QString("toolbarslider_%1").arg(sliderNumber), def));
}

void KisConfig::setToolbarSlider(int sliderNumber, const QString &slider)
{
    m_cfg.writeEntry(QString("toolbarslider_%1").arg(sliderNumber), slider);
}

bool KisConfig::sliderLabels(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("sliderLabels", true));
}

void KisConfig::setSliderLabels(bool enabled)
{
    m_cfg.writeEntry("sliderLabels", enabled);
}

QString KisConfig::currentInputProfile(bool defaultValue) const
{
    return (defaultValue ? QString() : m_cfg.readEntry("currentInputProfile", QString()));
}

void KisConfig::setCurrentInputProfile(const QString& name)
{
    m_cfg.writeEntry("currentInputProfile", name);
}

bool KisConfig::useSystemMonitorProfile(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("ColorManagement/UseSystemMonitorProfile", false));
}

void KisConfig::setUseSystemMonitorProfile(bool _useSystemMonitorProfile) const
{
    m_cfg.writeEntry("ColorManagement/UseSystemMonitorProfile", _useSystemMonitorProfile);
}

bool KisConfig::presetStripVisible(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("presetStripVisible", true));
}

void KisConfig::setPresetStripVisible(bool visible)
{
    m_cfg.writeEntry("presetStripVisible", visible);
}

bool KisConfig::scratchpadVisible(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("scratchpadVisible", true));
}

void KisConfig::setScratchpadVisible(bool visible)
{
    m_cfg.writeEntry("scratchpadVisible", visible);
}

bool KisConfig::showSingleChannelAsColor(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("showSingleChannelAsColor", false));
}

void KisConfig::setShowSingleChannelAsColor(bool asColor)
{
    m_cfg.writeEntry("showSingleChannelAsColor", asColor);
}

bool KisConfig::hidePopups(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("hidePopups", false));
}

void KisConfig::setHidePopups(bool hidepopups)
{
    m_cfg.writeEntry("hidePopups", hidepopups);
}

int KisConfig::numDefaultLayers(bool defaultValue) const
{
    return (defaultValue ? 2 : m_cfg.readEntry("NumberOfLayersForNewImage", 2));
}

void KisConfig::setNumDefaultLayers(int num)
{
    m_cfg.writeEntry("NumberOfLayersForNewImage", num);
}

quint8 KisConfig::defaultBackgroundOpacity(bool defaultValue) const
{
  return (defaultValue ? (int)OPACITY_OPAQUE_U8 : m_cfg.readEntry("BackgroundOpacityForNewImage", (int)OPACITY_OPAQUE_U8));
}

void KisConfig::setDefaultBackgroundOpacity(quint8 value)
{
  m_cfg.writeEntry("BackgroundOpacityForNewImage", (int)value);
}

QColor KisConfig::defaultBackgroundColor(bool defaultValue) const
{
  return (defaultValue ? QColor(Qt::white) : m_cfg.readEntry("BackgroundColorForNewImage", QColor(Qt::white)));
}

void KisConfig::setDefaultBackgroundColor(QColor value) 
{
  m_cfg.writeEntry("BackgroundColorForNewImage", value);
}

KisConfig::BackgroundStyle KisConfig::defaultBackgroundStyle(bool defaultValue) const
{
  return (KisConfig::BackgroundStyle)(defaultValue ? LAYER : m_cfg.readEntry("BackgroundStyleForNewImage", (int)LAYER));
}

void KisConfig::setDefaultBackgroundStyle(KisConfig::BackgroundStyle value)
{
  m_cfg.writeEntry("BackgroundStyleForNewImage", (int)value);
}

int KisConfig::lineSmoothingType(bool defaultValue) const
{
    return (defaultValue ? 1 : m_cfg.readEntry("LineSmoothingType", 1));
}

void KisConfig::setLineSmoothingType(int value)
{
    m_cfg.writeEntry("LineSmoothingType", value);
}

qreal KisConfig::lineSmoothingDistance(bool defaultValue) const
{
    return (defaultValue ? 50.0 : m_cfg.readEntry("LineSmoothingDistance", 50.0));
}

void KisConfig::setLineSmoothingDistance(qreal value)
{
    m_cfg.writeEntry("LineSmoothingDistance", value);
}

qreal KisConfig::lineSmoothingTailAggressiveness(bool defaultValue) const
{
    return (defaultValue ? 0.15 : m_cfg.readEntry("LineSmoothingTailAggressiveness", 0.15));
}

void KisConfig::setLineSmoothingTailAggressiveness(qreal value)
{
    m_cfg.writeEntry("LineSmoothingTailAggressiveness", value);
}

bool KisConfig::lineSmoothingSmoothPressure(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("LineSmoothingSmoothPressure", false));
}

void KisConfig::setLineSmoothingSmoothPressure(bool value)
{
    m_cfg.writeEntry("LineSmoothingSmoothPressure", value);
}

bool KisConfig::lineSmoothingScalableDistance(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("LineSmoothingScalableDistance", true));
}

void KisConfig::setLineSmoothingScalableDistance(bool value)
{
    m_cfg.writeEntry("LineSmoothingScalableDistance", value);
}

qreal KisConfig::lineSmoothingDelayDistance(bool defaultValue) const
{
    return (defaultValue ? 50.0 : m_cfg.readEntry("LineSmoothingDelayDistance", 50.0));
}

void KisConfig::setLineSmoothingDelayDistance(qreal value)
{
    m_cfg.writeEntry("LineSmoothingDelayDistance", value);
}

bool KisConfig::lineSmoothingUseDelayDistance(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("LineSmoothingUseDelayDistance", true));
}

void KisConfig::setLineSmoothingUseDelayDistance(bool value)
{
    m_cfg.writeEntry("LineSmoothingUseDelayDistance", value);
}

bool KisConfig::lineSmoothingFinishStabilizedCurve(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("LineSmoothingFinishStabilizedCurve", true));
}

void KisConfig::setLineSmoothingFinishStabilizedCurve(bool value)
{
    m_cfg.writeEntry("LineSmoothingFinishStabilizedCurve", value);
}

bool KisConfig::lineSmoothingStabilizeSensors(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("LineSmoothingStabilizeSensors", true));
}

void KisConfig::setLineSmoothingStabilizeSensors(bool value)
{
    m_cfg.writeEntry("LineSmoothingStabilizeSensors", value);
}

int KisConfig::paletteDockerPaletteViewSectionSize(bool defaultValue) const
{
    return (defaultValue ? 12 : m_cfg.readEntry("paletteDockerPaletteViewSectionSize", 12));
}

void KisConfig::setPaletteDockerPaletteViewSectionSize(int value) const
{
    m_cfg.writeEntry("paletteDockerPaletteViewSectionSize", value);
}

int KisConfig::tabletEventsDelay(bool defaultValue) const
{
    return (defaultValue ? 10 : m_cfg.readEntry("tabletEventsDelay", 10));
}

void KisConfig::setTabletEventsDelay(int value)
{
    m_cfg.writeEntry("tabletEventsDelay", value);
}

bool KisConfig::testingAcceptCompressedTabletEvents(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("testingAcceptCompressedTabletEvents", false));
}

void KisConfig::setTestingAcceptCompressedTabletEvents(bool value)
{
    m_cfg.writeEntry("testingAcceptCompressedTabletEvents", value);
}

bool KisConfig::testingCompressBrushEvents(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("testingCompressBrushEvents", false));
}

void KisConfig::setTestingCompressBrushEvents(bool value)
{
    m_cfg.writeEntry("testingCompressBrushEvents", value);
}

bool KisConfig::useVerboseOpenGLDebugOutput(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("useVerboseOpenGLDebugOutput", false));
}

int KisConfig::workaroundX11SmoothPressureSteps(bool defaultValue) const
{
    return (defaultValue ? 0 : m_cfg.readEntry("workaroundX11SmoothPressureSteps", 0));
}

bool KisConfig::showCanvasMessages(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("showOnCanvasMessages", true));
}

void KisConfig::setShowCanvasMessages(bool show)
{
    m_cfg.writeEntry("showOnCanvasMessages", show);
}

bool KisConfig::compressKra(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("compressLayersInKra", false));
}

void KisConfig::setCompressKra(bool compress)
{
    m_cfg.writeEntry("compressLayersInKra", compress);
}

const KoColorSpace* KisConfig::customColorSelectorColorSpace(bool defaultValue) const
{
    const KoColorSpace *cs = 0;

    KConfigGroup cfg = KGlobal::config()->group("advancedColorSelector");
    if (defaultValue || cfg.readEntry("useCustomColorSpace", true)) {
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
