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

#include <QtGlobal>
#include <QApplication>
#include <QDesktopWidget>
#include <QMutex>
#include <QFont>
#include <QThread>
#include <QStringList>
#include <QSettings>
#include <QStandardPaths>

#include <kconfig.h>

#include <KisDocument.h>

#include <KoColor.h>
#include <KoColorSpaceRegistry.h>
#include <KoColorModelStandardIds.h>
#include <KoColorProfile.h>

#include <kis_debug.h>
#include <kis_types.h>

#include "kis_canvas_resource_provider.h"
#include "kis_config_notifier.h"
#include "kis_snap_config.h"

#include <config-ocio.h>

#include <kis_color_manager.h>
#include <KisOcioConfiguration.h>

KisConfig::KisConfig(bool readOnly)
    : m_cfg( KSharedConfig::openConfig()->group(""))
    , m_readOnly(readOnly)
{
    if (!readOnly) {
        KIS_SAFE_ASSERT_RECOVER_RETURN(qApp && qApp->thread() == QThread::currentThread());
    }
}

KisConfig::~KisConfig()
{
    if (m_readOnly) return;

    if (qApp && qApp->thread() != QThread::currentThread()) {
        dbgKrita << "WARNING: KisConfig: requested config synchronization from nonGUI thread! Called from:" << kisBacktrace();
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

qreal KisConfig::stackT1(bool defaultValue) const
{
     return (defaultValue ? 5 : m_cfg.readEntry("stackT1",5));
}

void KisConfig::setStackT1(int T1)
{
    m_cfg.writeEntry("stackT1", T1);
}

qreal KisConfig::stackT2(bool defaultValue) const
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

qreal KisConfig::defImageResolution(bool defaultValue) const
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

void KisConfig::defImageResolution(qreal res) const
{
    m_cfg.writeEntry("imageResolutionDef", res*72.0);
}

int KisConfig::preferredVectorImportResolutionPPI(bool defaultValue) const
{
    return defaultValue ? 100.0 : m_cfg.readEntry("preferredVectorImportResolution", 100.0);
}

void KisConfig::setPreferredVectorImportResolutionPPI(int value) const
{
    m_cfg.writeEntry("preferredVectorImportResolution", value);
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

QColor KisConfig::getCursorMainColor(bool defaultValue) const
{
    QColor col;
    col.setRgbF(0.501961, 1.0, 0.501961);
    return (defaultValue ? col : m_cfg.readEntry("cursorMaincColor", col));
}

void KisConfig::setCursorMainColor(const QColor &v) const
{
    m_cfg.writeEntry("cursorMaincColor", v);
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

bool KisConfig::useEraserBrushOpacity(bool defaultValue) const
{
   return (defaultValue ? false : m_cfg.readEntry("useEraserBrushOpacity",false));
}

void KisConfig::setUseEraserBrushOpacity(bool value)
{
    m_cfg.writeEntry("useEraserBrushOpacity",value);
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
    //dbgKrita << "KisConfig::monitorProfile()" << profile;
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
    if (screen < 0) return 0;

    KisConfig cfg(true);
    QString monitorId;
    if (KisColorManager::instance()->devices().size() > screen) {
        monitorId = cfg.monitorForScreen(screen, KisColorManager::instance()->devices()[screen]);
    }
    //dbgKrita << "getScreenProfile(). Screen" << screen << "monitor id" << monitorId;

    if (monitorId.isEmpty()) {
        return 0;
    }

    QByteArray bytes = KisColorManager::instance()->displayProfile(monitorId);

    //dbgKrita << "\tgetScreenProfile()" << bytes.size();

    if (bytes.length() > 0) {
        const KoColorProfile *profile = KoColorSpaceRegistry::instance()->createColorProfile(RGBAColorModelID.id(), Integer8BitsColorDepthID.id(), bytes);
        //dbgKrita << "\tKisConfig::getScreenProfile for screen" << screen << profile->name();
        return profile;
    }
    else {
        //dbgKrita << "\tCould not get a system monitor profile";
        return 0;
    }
}

const KoColorProfile *KisConfig::displayProfile(int screen) const
{
    if (screen < 0) return 0;

    // if the user plays with the settings, they can override the display profile, in which case
    // we don't want the system setting.
    bool override = useSystemMonitorProfile();
    //dbgKrita << "KisConfig::displayProfile(). Override X11:" << override;
    const KoColorProfile *profile = 0;
    if (override) {
        //dbgKrita << "\tGoing to get the screen profile";
        profile = KisConfig::getScreenProfile(screen);
    }

    // if it fails. check the configuration
    if (!profile || !profile->isSuitableForDisplay()) {
        //dbgKrita << "\tGoing to get the monitor profile";
        QString monitorProfileName = monitorProfile(screen);
        //dbgKrita << "\t\tmonitorProfileName:" << monitorProfileName;
        if (!monitorProfileName.isEmpty()) {
            profile = KoColorSpaceRegistry::instance()->profileByName(monitorProfileName);
        }
        if (profile) {
            //dbgKrita << "\t\tsuitable for display" << profile->isSuitableForDisplay();
        }
        else {
            //dbgKrita << "\t\tstill no profile";
        }
    }
    // if we still don't have a profile, or the profile isn't suitable for display,
    // we need to get a last-resort profile. the built-in sRGB is a good choice then.
    if (!profile || !profile->isSuitableForDisplay()) {
        //dbgKrita << "\tnothing worked, going to get sRGB built-in";
        profile = KoColorSpaceRegistry::instance()->profileByName("sRGB Built-in");
    }

    if (profile) {
        //dbgKrita << "\tKisConfig::displayProfile for screen" << screen << "is" << profile->name();
    }
    else {
        //dbgKrita << "\tCouldn't get a display profile at all";
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

bool KisConfig::forcePaletteColors(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("colorsettings/forcepalettecolors", false));
}

void KisConfig::setForcePaletteColors(bool forcePaletteColors)
{
    m_cfg.writeEntry("colorsettings/forcepalettecolors", forcePaletteColors);
}

bool KisConfig::showRulers(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("showrulers", false));
}

void KisConfig::setShowRulers(bool rulers) const
{
    m_cfg.writeEntry("showrulers", rulers);
}

bool KisConfig::forceShowSaveMessages(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("forceShowSaveMessages", false));
}

void KisConfig::setForceShowSaveMessages(bool value) const
{
    m_cfg.writeEntry("forceShowSaveMessages", value);
}

bool KisConfig::forceShowAutosaveMessages(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("forceShowAutosaveMessages", false));
}

void KisConfig::setForceShowAutosaveMessages(bool value) const
{
    m_cfg.writeEntry("forceShowAutosaveMessages", value);
}

bool KisConfig::rulersTrackMouse(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("rulersTrackMouse", true));
}

void KisConfig::setRulersTrackMouse(bool value) const
{
    m_cfg.writeEntry("rulersTrackMouse", value);
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
    if (defaultValue) {
        return true;
    }

    //dbgKrita << "use opengl" << m_cfg.readEntry("useOpenGL", true) << "success" << m_cfg.readEntry("canvasState", "OPENGL_SUCCESS");
    QString cs = canvasState();
#ifdef Q_OS_WIN
    return (m_cfg.readEntry("useOpenGLWindows", true) && (cs == "OPENGL_SUCCESS" || cs == "TRY_OPENGL"));
#else
    return (m_cfg.readEntry("useOpenGL", true) && (cs == "OPENGL_SUCCESS" || cs == "TRY_OPENGL"));
#endif
}

void KisConfig::setUseOpenGL(bool useOpenGL) const
{
#ifdef Q_OS_WIN
    m_cfg.writeEntry("useOpenGLWindows", useOpenGL);
#else
    m_cfg.writeEntry("useOpenGL", useOpenGL);
#endif
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

quint32 KisConfig::getGridMainStyle(bool defaultValue) const
{
    int v = m_cfg.readEntry("gridmainstyle", 0);
    v = qBound(0, v, 2);
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

QColor KisConfig::getPixelGridColor(bool defaultValue) const
{
    QColor col(255, 255, 255);
    return (defaultValue ? col : m_cfg.readEntry("pixelGridColor", col));
}

void KisConfig::setPixelGridColor(const QColor & v) const
{
    m_cfg.writeEntry("pixelGridColor", v);
}

qreal KisConfig::getPixelGridDrawingThreshold(bool defaultValue) const
{
    qreal border = 24.0f;
    return (defaultValue ? border : m_cfg.readEntry("pixelGridDrawingThreshold", border));
}

void KisConfig::setPixelGridDrawingThreshold(qreal v) const
{
    m_cfg.writeEntry("pixelGridDrawingThreshold", v);
}

bool KisConfig::pixelGridEnabled(bool defaultValue) const
{
    bool enabled = true;
    return (defaultValue ? enabled : m_cfg.readEntry("pixelGridEnabled", enabled));
}

void KisConfig::enablePixelGrid(bool v) const
{
    m_cfg.writeEntry("pixelGridEnabled", v);
}

quint32 KisConfig::guidesLineStyle(bool defaultValue) const
{
    int v = m_cfg.readEntry("guidesLineStyle", 0);
    v = qBound(0, v, 2);
    return (defaultValue ? 0 : v);
}

void KisConfig::setGuidesLineStyle(quint32 v) const
{
    m_cfg.writeEntry("guidesLineStyle", v);
}

QColor KisConfig::guidesColor(bool defaultValue) const
{
    QColor col(99, 99, 99);
    return (defaultValue ? col : m_cfg.readEntry("guidesColor", col));
}

void KisConfig::setGuidesColor(const QColor & v) const
{
    m_cfg.writeEntry("guidesColor", v);
}

void KisConfig::loadSnapConfig(KisSnapConfig *config, bool defaultValue) const
{
    KisSnapConfig defaultConfig(false);

    if (defaultValue) {
        *config = defaultConfig;
        return;
    }

    config->setOrthogonal(m_cfg.readEntry("globalSnapOrthogonal", defaultConfig.orthogonal()));
    config->setNode(m_cfg.readEntry("globalSnapNode", defaultConfig.node()));
    config->setExtension(m_cfg.readEntry("globalSnapExtension", defaultConfig.extension()));
    config->setIntersection(m_cfg.readEntry("globalSnapIntersection", defaultConfig.intersection()));
    config->setBoundingBox(m_cfg.readEntry("globalSnapBoundingBox", defaultConfig.boundingBox()));
    config->setImageBounds(m_cfg.readEntry("globalSnapImageBounds", defaultConfig.imageBounds()));
    config->setImageCenter(m_cfg.readEntry("globalSnapImageCenter", defaultConfig.imageCenter()));
}

void KisConfig::saveSnapConfig(const KisSnapConfig &config)
{
    m_cfg.writeEntry("globalSnapOrthogonal", config.orthogonal());
    m_cfg.writeEntry("globalSnapNode", config.node());
    m_cfg.writeEntry("globalSnapExtension", config.extension());
    m_cfg.writeEntry("globalSnapIntersection", config.intersection());
    m_cfg.writeEntry("globalSnapBoundingBox", config.boundingBox());
    m_cfg.writeEntry("globalSnapImageBounds", config.imageBounds());
    m_cfg.writeEntry("globalSnapImageCenter", config.imageCenter());
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

bool KisConfig::forceAlwaysFullSizedOutline(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("forceAlwaysFullSizedOutline", false));
}

void KisConfig::setForceAlwaysFullSizedOutline(bool value) const
{
    m_cfg.writeEntry("forceAlwaysFullSizedOutline", value);
}

KisConfig::SessionOnStartup KisConfig::sessionOnStartup(bool defaultValue) const
{
    int value = defaultValue ? SOS_BlankSession : m_cfg.readEntry("sessionOnStartup", (int)SOS_BlankSession);
    return (KisConfig::SessionOnStartup)value;
}
void KisConfig::setSessionOnStartup(SessionOnStartup value)
{
    m_cfg.writeEntry("sessionOnStartup", (int)value);
}

bool KisConfig::saveSessionOnQuit(bool defaultValue) const
{
    return defaultValue ? false : m_cfg.readEntry("saveSessionOnQuit", false);
}
void KisConfig::setSaveSessionOnQuit(bool value)
{
    m_cfg.writeEntry("saveSessionOnQuit", value);
}

qreal KisConfig::outlineSizeMinimum(bool defaultValue) const
{
    return (defaultValue ? 1.0 : m_cfg.readEntry("OutlineSizeMinimum", 1.0));
}

void KisConfig::setOutlineSizeMinimum(qreal outlineSizeMinimum) const
{
    m_cfg.writeEntry("OutlineSizeMinimum", outlineSizeMinimum);
}

qreal KisConfig::selectionViewSizeMinimum(bool defaultValue) const
{
    return (defaultValue ? 5.0 : m_cfg.readEntry("SelectionViewSizeMinimum", 5.0));
}

void KisConfig::setSelectionViewSizeMinimum(qreal outlineSizeMinimum) const
{
    m_cfg.writeEntry("SelectionViewSizeMinimum", outlineSizeMinimum);
}

int KisConfig::autoSaveInterval(bool defaultValue)  const
{
    return (defaultValue ? 15 * 60 : m_cfg.readEntry("AutoSaveInterval", 15 * 60));
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
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QSettings kritarc(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);
    return (defaultValue ? "OPENGL_NOT_TRIED" : kritarc.value("canvasState", "OPENGL_NOT_TRIED").toString());
}

void KisConfig::setCanvasState(const QString& state) const
{
    static QStringList acceptableStates;
    if (acceptableStates.isEmpty()) {
        acceptableStates << "OPENGL_SUCCESS" << "TRY_OPENGL" << "OPENGL_NOT_TRIED" << "OPENGL_FAILED";
    }
    if (acceptableStates.contains(state)) {
        const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
        QSettings kritarc(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);
        kritarc.setValue("canvasState", state);
    }
}

bool KisConfig::toolOptionsPopupDetached(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("ToolOptionsPopupDetached", false));
}

void KisConfig::setToolOptionsPopupDetached(bool detached) const
{
    m_cfg.writeEntry("ToolOptionsPopupDetached", detached);
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

bool KisConfig::useWin8PointerInput(bool defaultValue) const
{
#ifdef Q_OS_WIN
    return (defaultValue ? false : m_cfg.readEntry("useWin8PointerInput", false));
#else
    Q_UNUSED(defaultValue);
    return false;
#endif
}

void KisConfig::setUseWin8PointerInput(bool value) const
{
#ifdef Q_OS_WIN
    // Special handling: Only set value if changed
    // I don't want it to be set if the user hasn't touched it
    if (useWin8PointerInput() != value) {
        m_cfg.writeEntry("useWin8PointerInput", value);
    }
#else
    Q_UNUSED(value)
#endif
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

int KisConfig::presetIconSize(bool defaultValue) const
{
    return (defaultValue ? 60 : m_cfg.readEntry("presetIconSize", 60));
}

void KisConfig::setPresetIconSize(const int value) const
{
    m_cfg.writeEntry("presetIconSize", value);
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

bool KisConfig::showDockers(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("showDockers", true));
}

void KisConfig::setShowDockers(const bool value) const
{
    m_cfg.writeEntry("showDockers", value);
}

bool KisConfig::showStatusBar(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("showStatusBar", true));
}

void KisConfig::setShowStatusBar(const bool value) const
{
    m_cfg.writeEntry("showStatusBar", value);
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

bool KisConfig::fullscreenMode(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("fullscreenMode", true));
}

void KisConfig::setFullscreenMode(const bool value) const
{
    m_cfg.writeEntry("fullscreenMode", value);
}

QStringList KisConfig::favoriteCompositeOps(bool defaultValue) const
{
    return (defaultValue ? QStringList() : m_cfg.readEntry("favoriteCompositeOps", QStringList()));
}

void KisConfig::setFavoriteCompositeOps(const QStringList& compositeOps) const
{
    m_cfg.writeEntry("favoriteCompositeOps", compositeOps);
}

QString KisConfig::exportConfigurationXML(const QString &filterId, bool defaultValue) const
{
    return (defaultValue ? QString() : m_cfg.readEntry("ExportConfiguration-" + filterId, QString()));
}

KisPropertiesConfigurationSP KisConfig::exportConfiguration(const QString &filterId, bool defaultValue) const
{
    KisPropertiesConfigurationSP cfg = new KisPropertiesConfiguration();
    const QString xmlData = exportConfigurationXML(filterId, defaultValue);
    cfg->fromXML(xmlData);
    return cfg;
}

void KisConfig::setExportConfiguration(const QString &filterId, KisPropertiesConfigurationSP properties) const
{
    QString exportConfig = properties->toXML();
    m_cfg.writeEntry("ExportConfiguration-" + filterId, exportConfig);
}

QString KisConfig::importConfiguration(const QString &filterId, bool defaultValue) const
{
    return (defaultValue ? QString() : m_cfg.readEntry("ImportConfiguration-" + filterId, QString()));
}

void KisConfig::setImportConfiguration(const QString &filterId, KisPropertiesConfigurationSP properties) const
{
    QString importConfig = properties->toXML();
    m_cfg.writeEntry("ImportConfiguration-" + filterId, importConfig);
}

bool KisConfig::useOcio(bool defaultValue) const
{
#ifdef HAVE_OCIO
    return (defaultValue ? false : m_cfg.readEntry("Krita/Ocio/UseOcio", false));
#else
    Q_UNUSED(defaultValue);
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

bool KisConfig::levelOfDetailEnabled(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("levelOfDetailEnabled", false));
}

void KisConfig::setLevelOfDetailEnabled(bool value)
{
    m_cfg.writeEntry("levelOfDetailEnabled", value);
}

KisOcioConfiguration KisConfig::ocioConfiguration(bool defaultValue) const
{
    KisOcioConfiguration cfg;

    if (!defaultValue) {
        cfg.mode = (KisOcioConfiguration::Mode)m_cfg.readEntry("Krita/Ocio/OcioColorManagementMode", 0);
        cfg.configurationPath = m_cfg.readEntry("Krita/Ocio/OcioConfigPath", QString());
        cfg.lutPath = m_cfg.readEntry("Krita/Ocio/OcioLutPath", QString());
        cfg.inputColorSpace = m_cfg.readEntry("Krita/Ocio/InputColorSpace", QString());
        cfg.displayDevice = m_cfg.readEntry("Krita/Ocio/DisplayDevice", QString());
        cfg.displayView = m_cfg.readEntry("Krita/Ocio/DisplayView", QString());
        cfg.look = m_cfg.readEntry("Krita/Ocio/DisplayLook", QString());
    }

    return cfg;
}

void KisConfig::setOcioConfiguration(const KisOcioConfiguration &cfg)
{
    m_cfg.writeEntry("Krita/Ocio/OcioColorManagementMode", (int) cfg.mode);
    m_cfg.writeEntry("Krita/Ocio/OcioConfigPath", cfg.configurationPath);
    m_cfg.writeEntry("Krita/Ocio/OcioLutPath", cfg.lutPath);
    m_cfg.writeEntry("Krita/Ocio/InputColorSpace", cfg.inputColorSpace);
    m_cfg.writeEntry("Krita/Ocio/DisplayDevice", cfg.displayDevice);
    m_cfg.writeEntry("Krita/Ocio/DisplayView", cfg.displayView);
    m_cfg.writeEntry("Krita/Ocio/DisplayLook", cfg.look);
}

KisConfig::OcioColorManagementMode
KisConfig::ocioColorManagementMode(bool defaultValue) const
{
    // FIXME: this option duplicates ocioConfiguration(), please deprecate it
    return (OcioColorManagementMode)(defaultValue ? INTERNAL
                                                  : m_cfg.readEntry("Krita/Ocio/OcioColorManagementMode", (int) INTERNAL));
}

void KisConfig::setOcioColorManagementMode(OcioColorManagementMode mode) const
{
    // FIXME: this option duplicates ocioConfiguration(), please deprecate it
    m_cfg.writeEntry("Krita/Ocio/OcioColorManagementMode", (int) mode);
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
    return (defaultValue ? QString() : m_cfg.readEntry("defaultPalette", "Default"));
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

int KisConfig::layerThumbnailSize(bool defaultValue) const
{
    return (defaultValue ? 20 : m_cfg.readEntry("layerThumbnailSize", 20));
}

void KisConfig::setLayerThumbnailSize(int size)
{
    m_cfg.writeEntry("layerThumbnailSize", size);
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
  return (KisConfig::BackgroundStyle)(defaultValue ? RASTER_LAYER : m_cfg.readEntry("BackgroundStyleForNewImage", (int)RASTER_LAYER));
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

int KisConfig::tabletEventsDelay(bool defaultValue) const
{
    return (defaultValue ? 10 : m_cfg.readEntry("tabletEventsDelay", 10));
}

void KisConfig::setTabletEventsDelay(int value)
{
    m_cfg.writeEntry("tabletEventsDelay", value);
}

bool KisConfig::trackTabletEventLatency(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("trackTabletEventLatency", false));
}

void KisConfig::setTrackTabletEventLatency(bool value)
{
    m_cfg.writeEntry("trackTabletEventLatency", value);
}

bool KisConfig::testingAcceptCompressedTabletEvents(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("testingAcceptCompressedTabletEvents", false));
}

void KisConfig::setTestingAcceptCompressedTabletEvents(bool value)
{
    m_cfg.writeEntry("testingAcceptCompressedTabletEvents", value);
}

bool KisConfig::shouldEatDriverShortcuts(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("shouldEatDriverShortcuts", false));
}

bool KisConfig::testingCompressBrushEvents(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("testingCompressBrushEvents", false));
}

void KisConfig::setTestingCompressBrushEvents(bool value)
{
    m_cfg.writeEntry("testingCompressBrushEvents", value);
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

bool KisConfig::toolOptionsInDocker(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("ToolOptionsInDocker", true));
}

void KisConfig::setToolOptionsInDocker(bool inDocker)
{
    m_cfg.writeEntry("ToolOptionsInDocker", inDocker);
}

bool KisConfig::kineticScrollingEnabled(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("KineticScrollingEnabled", true));
}

void KisConfig::setKineticScrollingEnabled(bool value)
{
    m_cfg.writeEntry("KineticScrollingEnabled", value);
}

int KisConfig::kineticScrollingGesture(bool defaultValue) const
{
    return (defaultValue ? 2 : m_cfg.readEntry("KineticScrollingGesture", 2));
}

void KisConfig::setKineticScrollingGesture(int gesture)
{
    m_cfg.writeEntry("KineticScrollingGesture", gesture);
}

int KisConfig::kineticScrollingSensitivity(bool defaultValue) const
{
    return (defaultValue ? 75 : m_cfg.readEntry("KineticScrollingSensitivity", 75));
}

void KisConfig::setKineticScrollingSensitivity(int sensitivity)
{
    m_cfg.writeEntry("KineticScrollingSensitivity", sensitivity);
}

bool KisConfig::kineticScrollingHiddenScrollbars(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("KineticScrollingHideScrollbar", false));
}

void KisConfig::setKineticScrollingHideScrollbars(bool scrollbar)
{
    m_cfg.writeEntry("KineticScrollingHideScrollbar", scrollbar);
}

const KoColorSpace* KisConfig::customColorSelectorColorSpace(bool defaultValue) const
{
    const KoColorSpace *cs = 0;

    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
    if (defaultValue || cfg.readEntry("useCustomColorSpace", true)) {
        KoColorSpaceRegistry* csr = KoColorSpaceRegistry::instance();
        QString modelID = cfg.readEntry("customColorSpaceModel", "RGBA");
        QString depthID = cfg.readEntry("customColorSpaceDepthID", "U8");
        QString profile = cfg.readEntry("customColorSpaceProfile", "sRGB built-in - (lcms internal)");
        if (profile == "default") {
          // qDebug() << "Falling back to default color profile.";
          profile = "sRGB built-in - (lcms internal)";
        }
        cs = csr->colorSpace(modelID, depthID, profile);
    }

    return cs;
}

void KisConfig::setCustomColorSelectorColorSpace(const KoColorSpace *cs)
{
    KConfigGroup cfg =  KSharedConfig::openConfig()->group("advancedColorSelector");
    cfg.writeEntry("useCustomColorSpace", bool(cs));
    if(cs) {
        cfg.writeEntry("customColorSpaceModel", cs->colorModelId().id());
        cfg.writeEntry("customColorSpaceDepthID", cs->colorDepthId().id());
        cfg.writeEntry("customColorSpaceProfile", cs->profile()->name());
    }

    KisConfigNotifier::instance()->notifyConfigChanged();
}

bool KisConfig::enableOpenGLFramerateLogging(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("enableOpenGLFramerateLogging", false));
}

void KisConfig::setEnableOpenGLFramerateLogging(bool value) const
{
    m_cfg.writeEntry("enableOpenGLFramerateLogging", value);
}

bool KisConfig::enableBrushSpeedLogging(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("enableBrushSpeedLogging", false));
}

void KisConfig::setEnableBrushSpeedLogging(bool value) const
{
    m_cfg.writeEntry("enableBrushSpeedLogging", value);
}

void KisConfig::setEnableAmdVectorizationWorkaround(bool value)
{
    m_cfg.writeEntry("amdDisableVectorWorkaround", value);
}

bool KisConfig::enableAmdVectorizationWorkaround(bool defaultValue) const
{
    return (defaultValue ? false : m_cfg.readEntry("amdDisableVectorWorkaround", false));
}

void KisConfig::setAnimationDropFrames(bool value)
{
    bool oldValue = animationDropFrames();

    if (value == oldValue) return;

    m_cfg.writeEntry("animationDropFrames", value);
    KisConfigNotifier::instance()->notifyDropFramesModeChanged();
}

bool KisConfig::animationDropFrames(bool defaultValue) const
{
    return (defaultValue ? true : m_cfg.readEntry("animationDropFrames", true));
}

int KisConfig::scrubbingUpdatesDelay(bool defaultValue) const
{
    return (defaultValue ? 30 : m_cfg.readEntry("scrubbingUpdatesDelay", 30));
}

void KisConfig::setScrubbingUpdatesDelay(int value)
{
    m_cfg.writeEntry("scrubbingUpdatesDelay", value);
}

int KisConfig::scrubbingAudioUpdatesDelay(bool defaultValue) const
{
    return (defaultValue ? -1 : m_cfg.readEntry("scrubbingAudioUpdatesDelay", -1));
}

void KisConfig::setScrubbingAudioUpdatesDelay(int value)
{
    m_cfg.writeEntry("scrubbingAudioUpdatesDelay", value);
}

int KisConfig::audioOffsetTolerance(bool defaultValue) const
{
    return (defaultValue ? -1 : m_cfg.readEntry("audioOffsetTolerance", -1));
}

void KisConfig::setAudioOffsetTolerance(int value)
{
    m_cfg.writeEntry("audioOffsetTolerance", value);
}

bool KisConfig::switchSelectionCtrlAlt(bool defaultValue) const
{
    return defaultValue ? false : m_cfg.readEntry("switchSelectionCtrlAlt", false);
}

void KisConfig::setSwitchSelectionCtrlAlt(bool value)
{
    m_cfg.writeEntry("switchSelectionCtrlAlt", value);
    KisConfigNotifier::instance()->notifyConfigChanged();
}

bool KisConfig::convertToImageColorspaceOnImport(bool defaultValue) const
{
    return defaultValue ? false : m_cfg.readEntry("ConvertToImageColorSpaceOnImport", false);
}

void KisConfig::setConvertToImageColorspaceOnImport(bool value)
{
    m_cfg.writeEntry("ConvertToImageColorSpaceOnImport", value);
}

int KisConfig::stabilizerSampleSize(bool defaultValue) const
{
#ifdef Q_OS_WIN
    const int defaultSampleSize = 50;
#else
    const int defaultSampleSize = 15;
#endif

    return defaultValue ?
        defaultSampleSize : m_cfg.readEntry("stabilizerSampleSize", defaultSampleSize);
}

void KisConfig::setStabilizerSampleSize(int value)
{
    m_cfg.writeEntry("stabilizerSampleSize", value);
}

bool KisConfig::stabilizerDelayedPaint(bool defaultValue) const
{
    const bool defaultEnabled = true;

    return defaultValue ?
        defaultEnabled : m_cfg.readEntry("stabilizerDelayedPaint", defaultEnabled);
}

void KisConfig::setStabilizerDelayedPaint(bool value)
{
    m_cfg.writeEntry("stabilizerDelayedPaint", value);
}

bool KisConfig::showBrushHud(bool defaultValue) const
{
    return defaultValue ? false : m_cfg.readEntry("showBrushHud", false);
}

void KisConfig::setShowBrushHud(bool value)
{
    m_cfg.writeEntry("showBrushHud", value);
}

QString KisConfig::brushHudSetting(bool defaultValue) const
{
    QString defaultDoc = "<!DOCTYPE hud_properties>\n<hud_properties>\n <version value=\"1\" type=\"value\"/>\n <paintbrush>\n  <properties_list type=\"array\">\n   <item_0 value=\"size\" type=\"value\"/>\n   <item_1 value=\"opacity\" type=\"value\"/>\n   <item_2 value=\"angle\" type=\"value\"/>\n  </properties_list>\n </paintbrush>\n <colorsmudge>\n  <properties_list type=\"array\">\n   <item_0 value=\"size\" type=\"value\"/>\n   <item_1 value=\"opacity\" type=\"value\"/>\n   <item_2 value=\"smudge_mode\" type=\"value\"/>\n   <item_3 value=\"smudge_length\" type=\"value\"/>\n   <item_4 value=\"smudge_color_rate\" type=\"value\"/>\n  </properties_list>\n </colorsmudge>\n <sketchbrush>\n  <properties_list type=\"array\">\n   <item_0 value=\"opacity\" type=\"value\"/>\n   <item_1 value=\"size\" type=\"value\"/>\n  </properties_list>\n </sketchbrush>\n <hairybrush>\n  <properties_list type=\"array\">\n   <item_0 value=\"size\" type=\"value\"/>\n   <item_1 value=\"opacity\" type=\"value\"/>\n  </properties_list>\n </hairybrush>\n <experimentbrush>\n  <properties_list type=\"array\">\n   <item_0 value=\"opacity\" type=\"value\"/>\n   <item_1 value=\"shape_windingfill\" type=\"value\"/>\n  </properties_list>\n </experimentbrush>\n <spraybrush>\n  <properties_list type=\"array\">\n   <item_0 value=\"size\" type=\"value\"/>\n   <item_1 value=\"opacity\" type=\"value\"/>\n   <item_2 value=\"spray_particlecount\" type=\"value\"/>\n   <item_3 value=\"spray_density\" type=\"value\"/>\n  </properties_list>\n </spraybrush>\n <hatchingbrush>\n  <properties_list type=\"array\">\n   <item_0 value=\"size\" type=\"value\"/>\n   <item_1 value=\"opacity\" type=\"value\"/>\n   <item_2 value=\"hatching_angle\" type=\"value\"/>\n   <item_3 value=\"hatching_thickness\" type=\"value\"/>\n   <item_4 value=\"hatching_separation\" type=\"value\"/>\n  </properties_list>\n </hatchingbrush>\n <gridbrush>\n  <properties_list type=\"array\">\n   <item_0 value=\"size\" type=\"value\"/>\n   <item_1 value=\"opacity\" type=\"value\"/>\n   <item_2 value=\"grid_divisionlevel\" type=\"value\"/>\n  </properties_list>\n </gridbrush>\n <curvebrush>\n  <properties_list type=\"array\">\n   <item_0 value=\"opacity\" type=\"value\"/>\n   <item_1 value=\"curve_historysize\" type=\"value\"/>\n   <item_2 value=\"curve_linewidth\" type=\"value\"/>\n   <item_3 value=\"curve_lineopacity\" type=\"value\"/>\n   <item_4 value=\"curve_connectionline\" type=\"value\"/>\n  </properties_list>\n </curvebrush>\n <dynabrush>\n  <properties_list type=\"array\">\n   <item_0 value=\"dyna_diameter\" type=\"value\"/>\n   <item_1 value=\"opacity\" type=\"value\"/>\n   <item_2 value=\"dyna_mass\" type=\"value\"/>\n   <item_3 value=\"dyna_drag\" type=\"value\"/>\n  </properties_list>\n </dynabrush>\n <particlebrush>\n  <properties_list type=\"array\">\n   <item_0 value=\"opacity\" type=\"value\"/>\n   <item_1 value=\"particle_particles\" type=\"value\"/>\n   <item_2 value=\"particle_opecityweight\" type=\"value\"/>\n   <item_3 value=\"particle_iterations\" type=\"value\"/>\n  </properties_list>\n </particlebrush>\n <duplicate>\n  <properties_list type=\"array\">\n   <item_0 value=\"size\" type=\"value\"/>\n   <item_1 value=\"opacity\" type=\"value\"/>\n   <item_2 value=\"clone_healing\" type=\"value\"/>\n   <item_3 value=\"clone_movesource\" type=\"value\"/>\n  </properties_list>\n </duplicate>\n <deformbrush>\n  <properties_list type=\"array\">\n   <item_0 value=\"size\" type=\"value\"/>\n   <item_1 value=\"opacity\" type=\"value\"/>\n   <item_2 value=\"deform_amount\" type=\"value\"/>\n   <item_3 value=\"deform_mode\" type=\"value\"/>\n  </properties_list>\n </deformbrush>\n <tangentnormal>\n  <properties_list type=\"array\">\n   <item_0 value=\"size\" type=\"value\"/>\n   <item_1 value=\"opacity\" type=\"value\"/>\n  </properties_list>\n </tangentnormal>\n <filter>\n  <properties_list type=\"array\">\n   <item_0 value=\"size\" type=\"value\"/>\n   <item_1 value=\"opacity\" type=\"value\"/>\n  </properties_list>\n </filter>\n <roundmarker>\n  <properties_list type=\"array\">\n   <item_0 value=\"opacity\" type=\"value\"/>\n   <item_1 value=\"size\" type=\"value\"/>\n  </properties_list>\n </roundmarker>\n</hud_properties>\n";
    return defaultValue ? defaultDoc : m_cfg.readEntry("brushHudSettings", defaultDoc);
}

void KisConfig::setBrushHudSetting(const QString &value) const
{
    m_cfg.writeEntry("brushHudSettings", value);
}

bool KisConfig::calculateAnimationCacheInBackground(bool defaultValue) const
{
    return defaultValue ? true : m_cfg.readEntry("calculateAnimationCacheInBackground", true);
}

void KisConfig::setCalculateAnimationCacheInBackground(bool value)
{
    m_cfg.writeEntry("calculateAnimationCacheInBackground", value);
}

QColor KisConfig::defaultAssistantsColor(bool defaultValue) const
{
    static const QColor defaultColor = QColor(176, 176, 176, 255);
    return defaultValue ? defaultColor : m_cfg.readEntry("defaultAssistantsColor", defaultColor);
}

void KisConfig::setDefaultAssistantsColor(const QColor &color) const
{
    m_cfg.writeEntry("defaultAssistantsColor", color);
}

bool KisConfig::autoSmoothBezierCurves(bool defaultValue) const
{
    return defaultValue ? false : m_cfg.readEntry("autoSmoothBezierCurves", false);
}

void KisConfig::setAutoSmoothBezierCurves(bool value)
{
    m_cfg.writeEntry("autoSmoothBezierCurves", value);
}

bool KisConfig::activateTransformToolAfterPaste(bool defaultValue) const
{
    return defaultValue ? false : m_cfg.readEntry("activateTransformToolAfterPaste", false);
}

void KisConfig::setActivateTransformToolAfterPaste(bool value)
{
    m_cfg.writeEntry("activateTransformToolAfterPaste", value);
}

KisConfig::RootSurfaceFormat KisConfig::rootSurfaceFormat(bool defaultValue) const
{
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QSettings kritarc(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);

    return rootSurfaceFormat(&kritarc, defaultValue);
}

void KisConfig::setRootSurfaceFormat(KisConfig::RootSurfaceFormat value)
{
    const QString configPath = QStandardPaths::writableLocation(QStandardPaths::GenericConfigLocation);
    QSettings kritarc(configPath + QStringLiteral("/kritadisplayrc"), QSettings::IniFormat);

    setRootSurfaceFormat(&kritarc, value);
}

KisConfig::RootSurfaceFormat KisConfig::rootSurfaceFormat(QSettings *displayrc, bool defaultValue)
{
    QString textValue = "bt709-g22";

    if (!defaultValue) {
        textValue = displayrc->value("rootSurfaceFormat", textValue).toString();
    }

    return textValue == "bt709-g10" ? BT709_G10 :
           textValue == "bt2020-pq" ? BT2020_PQ :
           BT709_G22;
}

void KisConfig::setRootSurfaceFormat(QSettings *displayrc, KisConfig::RootSurfaceFormat value)
{
    const QString textValue =
        value == BT709_G10 ? "bt709-g10" :
        value == BT2020_PQ ? "bt2020-pq" :
        "bt709-g22";

    displayrc->setValue("rootSurfaceFormat", textValue);
}

bool KisConfig::useZip64(bool defaultValue) const
{
    return defaultValue ? false : m_cfg.readEntry("UseZip64", false);
}

void KisConfig::setUseZip64(bool value)
{
    m_cfg.writeEntry("UseZip64", value);
}

#include <QDomDocument>
#include <QDomElement>

void KisConfig::writeKoColor(const QString& name, const KoColor& color) const
{
    QDomDocument doc = QDomDocument(name);
    QDomElement el = doc.createElement(name);
    doc.appendChild(el);
    color.toXML(doc, el);
    m_cfg.writeEntry(name, doc.toString());
}

//ported from kispropertiesconfig.
KoColor KisConfig::readKoColor(const QString& name, const KoColor& color) const
{
    QDomDocument doc;
    if (!m_cfg.readEntry(name).isNull()) {
        doc.setContent(m_cfg.readEntry(name));
        QDomElement e = doc.documentElement().firstChild().toElement();
        return KoColor::fromXML(e, Integer16BitsColorDepthID.id());
    } else {
        QString blackColor = "<!DOCTYPE Color>\n<Color>\n <RGB r=\"0\" space=\"sRGB-elle-V2-srgbtrc.icc\" b=\"0\" g=\"0\"/>\n</Color>\n";
        doc.setContent(blackColor);
        QDomElement e = doc.documentElement().firstChild().toElement();
        return KoColor::fromXML(e, Integer16BitsColorDepthID.id());
    }
    return color;

}
