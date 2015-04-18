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
#ifndef KIS_CONFIG_H_
#define KIS_CONFIG_H_

#include <QString>
#include <QStringList>
#include <QList>
#include <QColor>

#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include "kis_global.h"
#include "kis_properties_configuration.h"
#include "krita_export.h"

class KoColorProfile;
class KoColorSpace;

class KRITAUI_EXPORT KisConfig
{
public:
    KisConfig();
    ~KisConfig();

    bool disableTouchOnCanvas() const;
    void setDisableTouchOnCanvas(bool value) const;

    bool useProjections() const;
    void setUseProjections(bool useProj) const;

    bool undoEnabled() const;
    void setUndoEnabled(bool undo) const;

    int undoStackLimit() const;
    void setUndoStackLimit(int limit) const;

    bool useCumulativeUndoRedo();
    void setCumulativeUndoRedo(bool value);

    double stackT1();
    void setStackT1(int T1);

    double stackT2();
    void setStackT2(int T2);

    int stackN();
    void setStackN(int N);


    qint32 defImageWidth() const;
    void defImageWidth(qint32 width) const;

    qint32 defImageHeight() const;
    void defImageHeight(qint32 height) const;

    double defImageResolution() const;
    void defImageResolution(double res) const;

    bool defAutoFrameBreakEnabled() const;
    void defAutoFrameBreakEnabled(bool state) const;

    bool defOnionSkinningEnabled() const;
    void defOnionSkinningEnabled(bool state) const;

    int defFps() const;
    void defFps(int value) const;

    int defLocalPlaybackRange() const;
    void defLocalPlaybackRange(int value) const;

    bool defLoopingEnabled() const;
    void defLoopingEnabled(bool state) const;

    /**
     * @return the id of the default color model used for creating new images.
     */
    QString defColorModel() const;
    /**
     * set the id of the default color model used for creating new images.
     */
    void defColorModel(const QString & model) const;

    /**
     * @return the id of the default color depth used for creating new images.
     */
    QString defaultColorDepth() const;
    /**
     * set the id of the default color depth used for creating new images.
     */
    void setDefaultColorDepth(const QString & depth) const;

    /**
     * @return the id of the default color profile used for creating new images.
     */
    QString defColorProfile() const;
    /**
     * set the id of the default color profile used for creating new images.
     */
    void defColorProfile(const QString & depth) const;

    enumCursorStyle cursorStyle() const;
    enumCursorStyle getDefaultCursorStyle() const;
    void setCursorStyle(enumCursorStyle style) const;

    /// get the profile the user has selected for the given screen
    QString monitorProfile(int screen) const;
    void setMonitorProfile(int screen, const QString & monitorProfile, bool override) const;

    QString monitorForScreen(int screen, const QString &defaultMonitor) const;
    void setMonitorForScreen(int screen, const QString& monitor);

    /// Get the actual profile to be used for the given screen, which is
    /// either the screen profile set by the color management system or
    /// the custom monitor profile set by the user, depending on the configuration
    const KoColorProfile *displayProfile(int screen) const;

    QString workingColorSpace() const;
    void setWorkingColorSpace(const QString & workingColorSpace) const;

    QString importProfile() const;
    void setImportProfile(const QString & importProfile) const;

    QString printerColorSpace() const;
    void setPrinterColorSpace(const QString & printerColorSpace) const;

    QString printerProfile() const;
    void setPrinterProfile(const QString & printerProfile) const;

    bool useBlackPointCompensation() const;
    void setUseBlackPointCompensation(bool useBlackPointCompensation) const;

    bool allowLCMSOptimization() const;
    void setAllowLCMSOptimization(bool allowLCMSOptimization);

    bool showRulers() const;
    void setShowRulers(bool rulers) const;

    qint32 pasteBehaviour() const;
    void setPasteBehaviour(qint32 behaviour) const;

    qint32 renderIntent() const;
    void setRenderIntent(qint32 renderIntent) const;

    bool useOpenGL() const;
    void setUseOpenGL(bool useOpenGL) const;

    int openGLFilteringMode() const;
    void setOpenGLFilteringMode(int filteringMode);

    bool useOpenGLTextureBuffer() const;
    void setUseOpenGLTextureBuffer(bool useBuffer);

    bool disableDoubleBuffering() const;
    void setDisableDoubleBuffering(bool disableDoubleBuffering);

    bool disableVSync() const;
    void setDisableVSync(bool disableVSync);

    bool showAdvancedOpenGLSettings() const;

    bool forceOpenGLFenceWorkaround() const;

    int numMipmapLevels() const;
    int openGLTextureSize() const;
    int textureOverlapBorder() const;

    qint32 maxNumberOfThreads();
    void setMaxNumberOfThreads(qint32 numberOfThreads);

    /// Maximum tiles in memory (this is a guideline, not absolute)
    qint32 maxTilesInMem() const;
    void setMaxTilesInMem(qint32 tiles) const;

    quint32 getGridMainStyle() const;
    void setGridMainStyle(quint32 v) const;

    quint32 getGridSubdivisionStyle() const;
    void setGridSubdivisionStyle(quint32 v) const;

    QColor getGridMainColor() const;
    void setGridMainColor(const QColor & v) const;

    QColor getGridSubdivisionColor() const;
    void setGridSubdivisionColor(const QColor & v) const;

    quint32 getGridHSpacing() const;
    void setGridHSpacing(quint32 v) const;

    quint32 getGridVSpacing() const;
    void setGridVSpacing(quint32 v) const;

    bool getGridSpacingAspect() const;
    void setGridSpacingAspect(bool v) const;

    quint32 getGridSubdivisions() const;
    void setGridSubdivisions(quint32 v) const;

    quint32 getGridOffsetX() const;
    void setGridOffsetX(quint32 v) const;

    quint32 getGridOffsetY() const;
    void setGridOffsetY(quint32 v) const;

    bool getGridOffsetAspect() const;
    void setGridOffsetAspect(bool v) const;

    qint32 checkSize() const;
    void setCheckSize(qint32 checkSize) const;

    bool scrollCheckers() const;
    void setScrollingCheckers(bool scollCheckers) const;

    QColor checkersColor1() const;
    void setCheckersColor1(const QColor & v) const;

    QColor checkersColor2() const;
    void setCheckersColor2(const QColor & v) const;

    QColor canvasBorderColor() const;
    void setCanvasBorderColor(const QColor &color) const;

    bool hideScrollbars() const;
    void setHideScrollbars(bool value) const;

    bool antialiasCurves() const;
    void setAntialiasCurves(bool v) const;

    QColor selectionOverlayMaskColor() const;
    void setSelectionOverlayMaskColor(const QColor &color);

    bool antialiasSelectionOutline() const;
    void setAntialiasSelectionOutline(bool v) const;

    bool showRootLayer() const;
    void setShowRootLayer(bool showRootLayer) const;

    bool showGlobalSelection() const;
    void setShowGlobalSelection(bool showGlobalSelection) const;

    bool showOutlineWhilePainting() const;
    void setShowOutlineWhilePainting(bool showOutlineWhilePainting) const;

    bool hideSplashScreen() const;
    void setHideSplashScreen(bool hideSplashScreen) const;

    qreal outlineSizeMinimum() const;
    void setOutlineSizeMinimum(qreal outlineSizeMinimum) const;

    int autoSaveInterval() const;
    void setAutoSaveInterval(int seconds) const;

    bool backupFile() const;
    void setBackupFile(bool backupFile) const;

    bool showFilterGallery() const;
    void setShowFilterGallery(bool showFilterGallery) const;

    bool showFilterGalleryLayerMaskDialog() const;
    void setShowFilterGalleryLayerMaskDialog(bool showFilterGallery) const;

    QString defaultPainterlyColorModelId() const;
    void setDefaultPainterlyColorModelId(const QString& def) const;

    QString defaultPainterlyColorDepthId() const;
    void setDefaultPainterlyColorDepthId(const QString& def) const;

    // OPENGL_SUCCESS, TRY_OPENGL, OPENGL_NOT_TRIED, OPENGL_FAILED
    QString canvasState() const;
    void setCanvasState(const QString& state) const;

    bool paintopPopupDetached() const;
    void setPaintopPopupDetached(bool detached) const;

    QString pressureTabletCurve() const;
    void setPressureTabletCurve(const QString& curveString) const;

    qreal vastScrolling() const;
    void setVastScrolling(const qreal factor) const;

    int presetChooserViewMode() const;
    void setPresetChooserViewMode(const int mode) const;

    bool firstRun() const;
    void setFirstRun(const bool firstRun) const;

    bool clicklessSpacePan() const;
    void setClicklessSpacePan(const bool toggle) const;

    int horizontalSplitLines() const;
    void setHorizontalSplitLines(const int numberLines) const;

    int verticalSplitLines() const;
    void setVerticalSplitLines(const int numberLines) const;

    int hideDockersFullscreen() const;
    void setHideDockersFullscreen(const int value) const;

    bool showDockerTitleBars() const;
    void setShowDockerTitleBars(const bool value) const;

    int hideMenuFullscreen() const;
    void setHideMenuFullscreen(const int value) const;

    int hideScrollbarsFullscreen() const;
    void setHideScrollbarsFullscreen(const int value) const;

    int hideStatusbarFullscreen() const;
    void setHideStatusbarFullscreen(const int value) const;

    int hideTitlebarFullscreen() const;
    void setHideTitlebarFullscreen(const int value) const;

    int hideToolbarFullscreen() const;
    void setHideToolbarFullscreen(const int value) const;

    QStringList favoriteCompositeOps() const;
    void setFavoriteCompositeOps(const QStringList& compositeOps) const;

    QString exportConfiguration(const QString &filterId) const;
    void setExportConfiguration(const QString &filterId, const KisPropertiesConfiguration &properties) const;

    bool useOcio() const;
    void setUseOcio(bool useOCIO) const;

    int favoritePresets() const;
    void setFavoritePresets(const int value);


    enum OcioColorManagementMode {
        INTERNAL = 0,
        OCIO_CONFIG,
        OCIO_ENVIRONMENT
    };

    OcioColorManagementMode ocioColorManagementMode() const;
    void setOcioColorManagementMode(OcioColorManagementMode mode) const;

    QString ocioConfigurationPath() const;
    void setOcioConfigurationPath(const QString &path) const;

    QString ocioLutPath() const;
    void setOcioLutPath(const QString &path) const;

    int ocioLutEdgeSize() const;
    void setOcioLutEdgeSize(int value);

    bool ocioLockColorVisualRepresentation() const;
    void setOcioLockColorVisualRepresentation(bool value);

    bool useSystemMonitorProfile() const;
    void setUseSystemMonitorProfile(bool _useSystemMonitorProfile) const;

    QString defaultPalette() const;
    void setDefaultPalette(const QString& name) const;

    QString toolbarSlider(int sliderNumber);
    void setToolbarSlider(int sliderNumber, const QString &slider);

    bool sliderLabels() const;
    void setSliderLabels(bool enabled);

    QString currentInputProfile() const;
    void setCurrentInputProfile(const QString& name);

    bool presetStripVisible() const;
    void setPresetStripVisible(bool visible);

    bool scratchpadVisible() const;
    void setScratchpadVisible(bool visible);

    bool showSingleChannelAsColor() const;
    void setShowSingleChannelAsColor(bool asColor);

    bool hidePopups() const;
    void setHidePopups(bool hidepopups);

    int numDefaultLayers() const;
    void setNumDefaultLayers(int num);

    quint8 defaultBackgroundOpacity() const;
    void setDefaultBackgroundOpacity(quint8 value);

    QColor defaultBackgroundColor() const;
    void setDefaultBackgroundColor(QColor value);

    enum BackgroundStyle {
        LAYER = 0,
        PROJECTION = 1
    };

    BackgroundStyle defaultBackgroundStyle() const;
    void setDefaultBackgroundStyle(BackgroundStyle value);
    
    int lineSmoothingType() const;
    void setLineSmoothingType(int value);

    qreal lineSmoothingDistance() const;
    void setLineSmoothingDistance(qreal value);

    qreal lineSmoothingTailAggressiveness() const;
    void setLineSmoothingTailAggressiveness(qreal value);

    bool lineSmoothingSmoothPressure() const;
    void setLineSmoothingSmoothPressure(bool value);

    bool lineSmoothingScalableDistance() const;
    void setLineSmoothingScalableDistance(bool value);

    qreal lineSmoothingDelayDistance() const;
    void setLineSmoothingDelayDistance(qreal value);

    bool lineSmoothingUseDelayDistance() const;
    void setLineSmoothingUseDelayDistance(bool value);

    bool lineSmoothingFinishStabilizedCurve() const;
    void setLineSmoothingFinishStabilizedCurve(bool value);

    bool lineSmoothingStabilizeSensors() const;
    void setLineSmoothingStabilizeSensors(bool value);

    int paletteDockerPaletteViewSectionSize() const;
    void setPaletteDockerPaletteViewSectionSize(int value) const;

    int tabletEventsDelay() const;
    void setTabletEventsDelay(int value);

    bool testingAcceptCompressedTabletEvents() const;
    void setTestingAcceptCompressedTabletEvents(bool value);

    bool testingCompressBrushEvents() const;
    void setTestingCompressBrushEvents(bool value);

    const KoColorSpace* customColorSelectorColorSpace() const;
    void setCustomColorSelectorColorSpace(const KoColorSpace *cs);

    bool useDirtyPresets() const;
    void setUseDirtyPresets(bool value);

    bool useEraserBrushSize() const;
    void setUseEraserBrushSize(bool value);    

    QColor getMDIBackgroundColor() const;
    void setMDIBackgroundColor(const QColor & v) const;

    QString getMDIBackgroundImage() const;
    void setMDIBackgroundImage(const QString & fileName) const;

    bool useVerboseOpenGLDebugOutput() const;

    int workaroundX11SmoothPressureSteps() const;

    bool showCanvasMessages() const;
    void setShowCanvasMessages(bool show);

    bool compressKra() const;
    void setCompressKra(bool compress);

    template<class T>
    void writeEntry(const QString& name, const T& value) {
        m_cfg.writeEntry(name, value);
    }

    template<class T>
    void writeList(const QString& name, const QList<T>& value) {
        m_cfg.writeEntry(name, value);
    }

    template<class T>
    T readEntry(const QString& name, const T& defaultValue=T()) {
        return m_cfg.readEntry(name, defaultValue);
    }

    template<class T>
    QList<T> readList(const QString& name, const QList<T>& defaultValue=QList<T>()) {
        return m_cfg.readEntry(name, defaultValue);
    }

private:
    KisConfig(const KisConfig&);
    KisConfig& operator=(const KisConfig&) const;

    /// get the profile the color managment system has stored for the given screen
    static const KoColorProfile* getScreenProfile(int screen);

private:
    mutable KConfigGroup m_cfg;
};

#endif // KIS_CONFIG_H_
