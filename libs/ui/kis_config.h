/*
 *  SPDX-FileCopyrightText: 2002 Patrick Julien <freak@codepimps.org>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#ifndef KIS_CONFIG_H_
#define KIS_CONFIG_H_

#include <QString>
#include <QStringList>
#include <QList>
#include <QColor>
#include <QObject>

#include <ksharedconfig.h>
#include <kconfiggroup.h>

#include <kis_global.h>
#include <kis_properties_configuration.h>
#include "kritaui_export.h"

class KoColorProfile;
class KoColorSpace;
class KisSnapConfig;
class QSettings;
class KisOcioConfiguration;
struct KisCumulativeUndoData;

class KRITAUI_EXPORT KisConfig
{
public:
    /**
     * @brief KisConfig create a kisconfig object
     * @param readOnly if true, there will be no call to sync when the object is deleted.
     *  Any KisConfig object created in a thread must be read-only.
     */
    KisConfig(bool readOnly);

    ~KisConfig();

public Q_SLOTS:
    /// Log the most interesting settings to the usage log
    void logImportantSettings() const;
public:

    enum TouchPainting {
        TOUCH_PAINTING_AUTO,
        TOUCH_PAINTING_ENABLED,
        TOUCH_PAINTING_DISABLED,
    };

    TouchPainting touchPainting(bool defaultValue = false) const;
    void setTouchPainting(TouchPainting value) const;
    bool disableTouchOnCanvas() const;

    // XXX Unused?
    bool useProjections(bool defaultValue = false) const;
    void setUseProjections(bool useProj) const;

    bool undoEnabled(bool defaultValue = false) const;
    void setUndoEnabled(bool undo) const;

    int undoStackLimit(bool defaultValue = false) const;
    void setUndoStackLimit(int limit) const;

    bool useCumulativeUndoRedo(bool defaultValue = false) const;
    void setCumulativeUndoRedo(bool value);

    KisCumulativeUndoData cumulativeUndoData(bool defaultValue = false) const;
    void setCumulativeUndoData(KisCumulativeUndoData value);

    qint32 defImageWidth(bool defaultValue = false) const;
    void defImageWidth(qint32 width) const;

    qint32 defImageHeight(bool defaultValue = false) const;
    void defImageHeight(qint32 height) const;

    qreal defImageResolution(bool defaultValue = false) const;
    void defImageResolution(qreal res) const;

    int preferredVectorImportResolutionPPI(bool defaultValue = false) const;
    void setPreferredVectorImportResolutionPPI(int value) const;

    bool useDefaultColorSpace(bool defaultvalue = false) const;
    void setUseDefaultColorSpace(bool value) const;
    /**
     * @return the id of the default color model used for creating new images.
     */
    QString defColorModel(bool defaultValue = false) const;
    /**
     * set the id of the default color model used for creating new images.
     */
    void defColorModel(const QString & model) const;

    /**
     * @return the id of the default color depth used for creating new images.
     */
    QString defaultColorDepth(bool defaultValue = false) const;
    /**
     * set the id of the default color depth used for creating new images.
     */
    void setDefaultColorDepth(const QString & depth) const;

    /**
     * @return the id of the default color profile used for creating new images.
     */
    QString defColorProfile(bool defaultValue = false) const;
    /**
     * set the id of the default color profile used for creating new images.
     */
    void defColorProfile(const QString & depth) const;

    CursorStyle newCursorStyle(bool defaultValue = false) const;
    void setNewCursorStyle(CursorStyle style);

    QColor getCursorMainColor(bool defaultValue = false) const;
    void setCursorMainColor(const QColor& v) const;

    OutlineStyle newOutlineStyle(bool defaultValue = false) const;
    void setNewOutlineStyle(OutlineStyle style);

    OutlineStyle lastUsedOutlineStyle(bool defaultValue = false) const;
    void setLastUsedOutlineStyle(OutlineStyle style);

    bool separateEraserCursor(bool defaultValue = false) const;
    void setSeparateEraserCursor(bool value) const;

    CursorStyle eraserCursorStyle(bool defaultValue = false) const;
    void setEraserCursorStyle(CursorStyle style);

    QColor getEraserCursorMainColor(bool defaultValue = false) const;
    void setEraserCursorMainColor(const QColor& v) const;

    OutlineStyle eraserOutlineStyle(bool defaultValue = false) const;
    void setEraserOutlineStyle(OutlineStyle style);

    QRect colorPreviewRect() const;
    void setColorPreviewRect(const QRect &rect);

    /// get the profile the user has selected for the given screen
    QString monitorProfile(int screen) const;
    void setMonitorProfile(int screen, const QString & monitorProfile, bool override) const;

    QString monitorForScreen(int screen, const QString &defaultMonitor, bool defaultValue = true) const;
    void setMonitorForScreen(int screen, const QString& monitor);

    /// Get the actual profile to be used for the given screen, which is
    /// either the screen profile set by the color management system or
    /// the custom monitor profile set by the user, depending on the configuration
    const KoColorProfile *displayProfile(int screen) const;

    const QString getScreenStringIdentfier(int screenNo) const;

    QString workingColorSpace(bool defaultValue = false) const;
    void setWorkingColorSpace(const QString & workingColorSpace) const;

    QString importProfile(bool defaultValue = false) const;
    void setImportProfile(const QString & importProfile) const;

    QString printerColorSpace(bool defaultValue = false) const;
    void setPrinterColorSpace(const QString & printerColorSpace) const;

    QString printerProfile(bool defaultValue = false) const;
    void setPrinterProfile(const QString & printerProfile) const;

    bool useBlackPointCompensation(bool defaultValue = false) const;
    void setUseBlackPointCompensation(bool useBlackPointCompensation) const;

    bool allowLCMSOptimization(bool defaultValue = false) const;
    void setAllowLCMSOptimization(bool allowLCMSOptimization);

    bool forcePaletteColors(bool defaultValue = false) const;
    void setForcePaletteColors(bool forcePaletteColors);

    void writeKoColor(const QString& name, const KoColor& color) const;
    KoColor readKoColor(const QString& name, const KoColor& color = KoColor()) const;

    bool showRulers(bool defaultValue = false) const;
    void setShowRulers(bool rulers) const;

    bool forceShowSaveMessages(bool defaultValue = true) const;
    void setForceShowSaveMessages(bool value) const;

    bool forceShowAutosaveMessages(bool defaultValue = true) const;
    void setForceShowAutosaveMessages(bool ShowAutosaveMessages) const;

    bool rulersTrackMouse(bool defaultValue = false) const;
    void setRulersTrackMouse(bool value) const;

    qint32 pasteBehaviour(bool defaultValue = false) const;
    void setPasteBehaviour(qint32 behaviour) const;

    qint32 pasteFormat(bool defaultValue) const;
    void setPasteFormat(qint32 format);

    qint32 monitorRenderIntent(bool defaultValue = false) const;
    void setRenderIntent(qint32 monitorRenderIntent) const;

    bool useOpenGL(bool defaultValue = false) const;
    void disableOpenGL() const;

    int openGLFilteringMode(bool defaultValue = false) const;
    void setOpenGLFilteringMode(int filteringMode);

    void setWidgetStyle(QString name);
    QString widgetStyle(bool defaultValue = false);

    bool useOpenGLTextureBuffer(bool defaultValue = false) const;
    void setUseOpenGLTextureBuffer(bool useBuffer);

    bool forceOpenGLFenceWorkaround(bool defaultValue = false) const;

    int numMipmapLevels(bool defaultValue = false) const;
    int openGLTextureSize(bool defaultValue = false) const;
    int textureOverlapBorder() const;

    quint32 getGridMainStyle(bool defaultValue = false) const;
    void setGridMainStyle(quint32 v) const;

    quint32 getGridSubdivisionStyle(bool defaultValue = false) const;
    void setGridSubdivisionStyle(quint32 v) const;

    QColor getGridMainColor(bool defaultValue = false) const;
    void setGridMainColor(const QColor & v) const;

    QColor getGridSubdivisionColor(bool defaultValue = false) const;
    void setGridSubdivisionColor(const QColor & v) const;

    QColor getPixelGridColor(bool defaultValue = false) const;
    void setPixelGridColor(const QColor & v) const;

    qreal getPixelGridDrawingThreshold(bool defaultValue = false) const;
    void setPixelGridDrawingThreshold(qreal v) const;

    bool pixelGridEnabled(bool defaultValue = false) const;
    void enablePixelGrid(bool v) const;

    quint32 guidesLineStyle(bool defaultValue = false) const;
    void setGuidesLineStyle(quint32 v) const;
    QColor guidesColor(bool defaultValue = false) const;
    void setGuidesColor(const QColor & v) const;

    void loadSnapConfig(KisSnapConfig *config, bool defaultValue = false) const;
    void saveSnapConfig(const KisSnapConfig &config);

    qint32 checkSize(bool defaultValue = false) const;
    void setCheckSize(qint32 checkSize) const;

    bool scrollCheckers(bool defaultValue = false) const;
    void setScrollingCheckers(bool scrollCheckers) const;

    QColor checkersColor1(bool defaultValue = false) const;
    void setCheckersColor1(const QColor & v) const;

    QColor checkersColor2(bool defaultValue = false) const;
    void setCheckersColor2(const QColor & v) const;

    QColor canvasBorderColor(bool defaultValue = false) const;
    void setCanvasBorderColor(const QColor &color) const;

    bool hideScrollbars(bool defaultValue = false) const;
    void setHideScrollbars(bool value) const;

    bool scrollbarZoomEnabled(bool defaultValue = false) const;
    void setScrollbarZoomEnabled(bool enabled) const;

    bool antialiasCurves(bool defaultValue = false) const;
    void setAntialiasCurves(bool v) const;

    bool antialiasSelectionOutline(bool defaultValue = false) const;
    void setAntialiasSelectionOutline(bool v) const;

    bool showRootLayer(bool defaultValue = false) const;
    void setShowRootLayer(bool showRootLayer) const;

    bool showGlobalSelection(bool defaultValue = false) const;
    void setShowGlobalSelection(bool showGlobalSelection) const;

    bool showOutlineWhilePainting(bool defaultValue = false) const;
    void setShowOutlineWhilePainting(bool showOutlineWhilePainting) const;

    bool forceAlwaysFullSizedOutline(bool defaultValue = false) const;
    void setForceAlwaysFullSizedOutline(bool value) const;

    bool showEraserOutlineWhilePainting(bool defaultValue = false) const;
    void setShowEraserOutlineWhilePainting(bool showEraserOutlineWhilePainting) const;

    bool forceAlwaysFullSizedEraserOutline(bool defaultValue = false) const;
    void setForceAlwaysFullSizedEraserOutline(bool value) const;

    enum SessionOnStartup {
        SOS_BlankSession,
        SOS_PreviousSession,
        SOS_ShowSessionManager
    };
    SessionOnStartup sessionOnStartup(bool defaultValue = false) const;
    void setSessionOnStartup(SessionOnStartup value);

    bool saveSessionOnQuit(bool defaultValue) const;
    void setSaveSessionOnQuit(bool value);

    bool hideDevFundBanner(bool defaultValue = false) const;
    void setHideDevFundBanner(bool value = true);

    qreal outlineSizeMinimum(bool defaultValue = false) const;
    void setOutlineSizeMinimum(qreal outlineSizeMinimum) const;

    qreal selectionViewSizeMinimum(bool defaultValue = false) const;
    void setSelectionViewSizeMinimum(qreal outlineSizeMinimum) const;

    int autoSaveInterval(bool defaultValue = false) const;
    void setAutoSaveInterval(int seconds) const;

    bool backupFile(bool defaultValue = false) const;
    void setBackupFile(bool backupFile) const;

    bool showFilterGallery(bool defaultValue = false) const;
    void setShowFilterGallery(bool showFilterGallery) const;

    bool showFilterGalleryLayerMaskDialog(bool defaultValue = false) const;
    void setShowFilterGalleryLayerMaskDialog(bool showFilterGallery) const;

    // OPENGL_SUCCESS, TRY_OPENGL, OPENGL_NOT_TRIED, OPENGL_FAILED
    QString canvasState(bool defaultValue = false) const;
    void setCanvasState(const QString& state) const;

    bool toolOptionsPopupDetached(bool defaultValue = false) const;
    void setToolOptionsPopupDetached(bool detached) const;

    bool paintopPopupDetached(bool defaultValue = false) const;
    void setPaintopPopupDetached(bool detached) const;

    QString pressureTabletCurve(bool defaultValue = false) const;
    void setPressureTabletCurve(const QString& curveString) const;

    bool useWin8PointerInput(bool defaultValue = false) const;
    void setUseWin8PointerInput(bool value);

    static bool useWin8PointerInputNoApp(QSettings *settings, bool defaultValue = false);
    static void setUseWin8PointerInputNoApp(QSettings *settings, bool value);

    bool useRightMiddleTabletButtonWorkaround(bool defaultValue = false) const;
    void setUseRightMiddleTabletButtonWorkaround(bool value);

    qreal vastScrolling(bool defaultValue = false) const;
    void setVastScrolling(const qreal factor) const;

    int presetChooserViewMode(bool defaultValue = false) const;
    void setPresetChooserViewMode(const int mode) const;

    int presetIconSize(bool defaultValue = false) const;
    void setPresetIconSize(const int value) const;


    bool firstRun(bool defaultValue = false) const;
    void setFirstRun(const bool firstRun) const;

    bool clicklessSpacePan(bool defaultValue = false) const;
    void setClicklessSpacePan(const bool toggle) const;

    int horizontalSplitLines(bool defaultValue = false) const;
    void setHorizontalSplitLines(const int numberLines) const;

    int verticalSplitLines(bool defaultValue = false) const;
    void setVerticalSplitLines(const int numberLines) const;

    bool hideDockersFullscreen(bool defaultValue = false) const;
    void setHideDockersFullscreen(const bool value) const;

    bool showDockerTitleBars(bool defaultValue = false) const;
    void setShowDockerTitleBars(const bool value) const;

    bool showDockers(bool defaultValue = false) const;
    void setShowDockers(const bool value) const;

    bool showStatusBar(bool defaultValue = false) const;
    void setShowStatusBar(const bool value) const;

    bool hideMenuFullscreen(bool defaultValue = false) const;
    void setHideMenuFullscreen(const bool value) const;

    bool hideScrollbarsFullscreen(bool defaultValue = false) const;
    void setHideScrollbarsFullscreen(const bool value) const;

    bool hideStatusbarFullscreen(bool defaultValue = false) const;
    void setHideStatusbarFullscreen(const bool value) const;

    bool hideTitlebarFullscreen(bool defaultValue = false) const;
    void setHideTitlebarFullscreen(const bool value) const;

    bool hideToolbarFullscreen(bool defaultValue = false) const;
    void setHideToolbarFullscreen(const bool value) const;

    bool fullscreenMode(bool defaultValue = false) const;
    void setFullscreenMode(const bool value) const;

    QStringList favoriteCompositeOps(bool defaultValue = false) const;
    void setFavoriteCompositeOps(const QStringList& compositeOps) const;

    QString exportConfigurationXML(const QString &filterId, bool defaultValue = false) const;
    KisPropertiesConfigurationSP exportConfiguration(const QString &filterId, bool defaultValue = false) const;
    void setExportConfiguration(const QString &filterId, KisPropertiesConfigurationSP properties) const;

    QString importConfiguration(const QString &filterId, bool defaultValue = false) const;
    void setImportConfiguration(const QString &filterId, KisPropertiesConfigurationSP properties) const;

    bool useOcio(bool defaultValue = false) const;
    void setUseOcio(bool useOCIO) const;

    int favoritePresets(bool defaultValue = false) const;
    void setFavoritePresets(const int value);

    bool levelOfDetailEnabled(bool defaultValue = false) const;
    void setLevelOfDetailEnabled(bool value);

    KisOcioConfiguration ocioConfiguration(bool defaultValue = false) const;
    void setOcioConfiguration(const KisOcioConfiguration &cfg);

    enum OcioColorManagementMode {
        INTERNAL = 0,
        OCIO_CONFIG,
        OCIO_ENVIRONMENT
    };

    OcioColorManagementMode ocioColorManagementMode(bool defaultValue = false) const;
    void setOcioColorManagementMode(OcioColorManagementMode mode) const;

    int ocioLutEdgeSize(bool defaultValue = false) const;
    void setOcioLutEdgeSize(int value);

    bool ocioLockColorVisualRepresentation(bool defaultValue = false) const;
    void setOcioLockColorVisualRepresentation(bool value);

    bool useSystemMonitorProfile(bool defaultValue = false) const;
    void setUseSystemMonitorProfile(bool _useSystemMonitorProfile) const;

    QString defaultPalette(bool defaultValue = false) const;
    void setDefaultPalette(const QString& name) const;

    QString toolbarSlider(int sliderNumber, bool defaultValue = false) const;
    void setToolbarSlider(int sliderNumber, const QString &slider);


    int layerThumbnailSize(bool defaultValue = false) const;
    void setLayerThumbnailSize(int size);

    int layerTreeIndentation(bool defaultValue = false) const;
    void setLayerTreeIndentation(int percentage);


    bool sliderLabels(bool defaultValue = false) const;
    void setSliderLabels(bool enabled);

    QString currentInputProfile(bool defaultValue = false) const;
    void setCurrentInputProfile(const QString& name);

    bool presetStripVisible(bool defaultValue = false) const;
    void setPresetStripVisible(bool visible);

    bool scratchpadVisible(bool defaultValue = false) const;
    void setScratchpadVisible(bool visible);

    bool showSingleChannelAsColor(bool defaultValue = false) const;
    void setShowSingleChannelAsColor(bool asColor);

    bool hidePopups(bool defaultValue = false) const;
    void setHidePopups(bool hidePopups);

    int numDefaultLayers(bool defaultValue = false) const;
    void setNumDefaultLayers(int num);

    quint8 defaultBackgroundOpacity(bool defaultValue = false) const;
    void setDefaultBackgroundOpacity(quint8 value);

    QColor defaultBackgroundColor(bool defaultValue = false) const;
    void setDefaultBackgroundColor(const QColor &value);

    enum BackgroundStyle {
        RASTER_LAYER = 0,
        CANVAS_COLOR = 1,
        FILL_LAYER = 2
    };

    BackgroundStyle defaultBackgroundStyle(bool defaultValue = false) const;
    void setDefaultBackgroundStyle(BackgroundStyle value);

    int lineSmoothingType(bool defaultValue = false) const;
    void setLineSmoothingType(int value);

    qreal lineSmoothingDistance(bool defaultValue = false) const;
    void setLineSmoothingDistance(qreal value);

    qreal lineSmoothingTailAggressiveness(bool defaultValue = false) const;
    void setLineSmoothingTailAggressiveness(qreal value);

    bool lineSmoothingSmoothPressure(bool defaultValue = false) const;
    void setLineSmoothingSmoothPressure(bool value);

    bool lineSmoothingScalableDistance(bool defaultValue = false) const;
    void setLineSmoothingScalableDistance(bool value);

    qreal lineSmoothingDelayDistance(bool defaultValue = false) const;
    void setLineSmoothingDelayDistance(qreal value);

    bool lineSmoothingUseDelayDistance(bool defaultValue = false) const;
    void setLineSmoothingUseDelayDistance(bool value);

    bool lineSmoothingFinishStabilizedCurve(bool defaultValue = false) const;
    void setLineSmoothingFinishStabilizedCurve(bool value);

    bool lineSmoothingStabilizeSensors(bool defaultValue = false) const;
    void setLineSmoothingStabilizeSensors(bool value);

    int tabletEventsDelay(bool defaultValue = false) const;
    void setTabletEventsDelay(int value);

    bool trackTabletEventLatency(bool defaultValue = false) const;
    void setTrackTabletEventLatency(bool value);

    bool ignoreHighFunctionKeys(bool defaultValue = false) const;
    void setIgnoreHighFunctionKeys(bool value);

    bool testingAcceptCompressedTabletEvents(bool defaultValue = false) const;
    void setTestingAcceptCompressedTabletEvents(bool value);

    bool shouldEatDriverShortcuts(bool defaultValue = false) const;

    bool testingCompressBrushEvents(bool defaultValue = false) const;
    void setTestingCompressBrushEvents(bool value);

    const KoColorSpace* customColorSelectorColorSpace(bool defaultValue = false) const;
    void setCustomColorSelectorColorSpace(const KoColorSpace *cs);

    bool useDirtyPresets(bool defaultValue = false) const;
    void setUseDirtyPresets(bool value);

    bool useEraserBrushSize(bool defaultValue = false) const;
    void setUseEraserBrushSize(bool value);

    bool useEraserBrushOpacity(bool defaultValue = false) const;
    void setUseEraserBrushOpacity(bool value);

    QString getMDIBackgroundColor(bool defaultValue = false) const;
    void setMDIBackgroundColor(const QString & v) const;

    QString getMDIBackgroundImage(bool defaultValue = false) const;
    void setMDIBackgroundImage(const QString & fileName) const;

    int workaroundX11SmoothPressureSteps(bool defaultValue = false) const;

    bool showCanvasMessages(bool defaultValue = false) const;
    void setShowCanvasMessages(bool show);

    bool compressKra(bool defaultValue = false) const;
    void setCompressKra(bool compress);

    bool trimKra(bool defaultValue = false) const;
    void setTrimKra(bool trim);

    bool trimFramesImport(bool defaultValue = false) const;
    void setTrimFramesImport(bool trim);

    bool toolOptionsInDocker(bool defaultValue = false) const;
    void setToolOptionsInDocker(bool inDocker);

    bool kineticScrollingEnabled(bool defaultValue = false) const;
    void setKineticScrollingEnabled(bool enabled);

    int kineticScrollingGesture(bool defaultValue = false) const;
    void setKineticScrollingGesture(int kineticScroll);

    int kineticScrollingSensitivity(bool defaultValue = false) const;
    void setKineticScrollingSensitivity(int sensitivity);

    bool kineticScrollingHiddenScrollbars(bool defaultValue = false) const;
    void setKineticScrollingHideScrollbars(bool scrollbar);

    int zoomSteps(bool defaultValue = false) const;
    void setZoomSteps(int steps);

    int zoomMarginSize(bool defaultValue = false) const;
    void setZoomMarginSize(int zoomMarginSize);

    void setEnableOpenGLFramerateLogging(bool value) const;
    bool enableOpenGLFramerateLogging(bool defaultValue = false) const;

    void setEnableBrushSpeedLogging(bool value) const;
    bool enableBrushSpeedLogging(bool defaultValue = false) const;

    void setDisableVectorOptimizations(bool value);
    bool disableVectorOptimizations(bool defaultValue = false) const;

    void setDisableAVXOptimizations(bool value);
    bool disableAVXOptimizations(bool defaultValue = false) const;

    void setAnimationPlaybackBackend(int value);
    int animationPlaybackBackend(bool defaultValue = false) const;

    bool animationDropFrames(bool defaultValue = false) const;
    void setAnimationDropFrames(bool value);

    bool autoPinLayersToTimeline(bool defaultValue = false) const;
    void setAutoPinLayersToTimeline(bool value);

    bool adaptivePlaybackRange(bool defaultValue = false) const;
    void setAdaptivePlaybackRange(bool value);
    
    QString ffmpegLocation(bool defaultValue = false) const;
    void setFFMpegLocation(const QString& value);

    qreal timelineZoom(bool defaultValue = false) const;
    void setTimelineZoom(qreal value);

    int scrubbingUpdatesDelay(bool defaultValue = false) const;
    void setScrubbingUpdatesDelay(int value);

    int scrubbingAudioUpdatesDelay(bool defaultValue = false) const;
    void setScrubbingAudioUpdatesDelay(int value);

    int audioOffsetTolerance(bool defaultValue = false) const;
    void setAudioOffsetTolerance(int value);

    bool switchSelectionCtrlAlt(bool defaultValue = false) const;
    void setSwitchSelectionCtrlAlt(bool value);

    bool convertToImageColorspaceOnImport(bool defaultValue = false) const;
    void setConvertToImageColorspaceOnImport(bool value);

    int stabilizerSampleSize(bool defaultValue = false) const;
    void setStabilizerSampleSize(int value);

    bool stabilizerDelayedPaint(bool defaultValue = false) const;
    void setStabilizerDelayedPaint(bool value);

    bool showBrushHud(bool defaultValue = false) const;
    void setShowBrushHud(bool value);
    
    bool showPaletteBottomBar(bool defaultValue = false) const;
    void setShowPaletteBottomBar(bool value);

    QString brushHudSetting(bool defaultValue = false) const;
    void setBrushHudSetting(const QString &value) const;

    bool calculateAnimationCacheInBackground(bool defaultValue = false) const;
    void setCalculateAnimationCacheInBackground(bool value);

    QColor defaultAssistantsColor(bool defaultValue = false) const;
    void setDefaultAssistantsColor(const QColor &color) const;

    bool autoSmoothBezierCurves(bool defaultValue = false) const;
    void setAutoSmoothBezierCurves(bool value);
    
    bool activateTransformToolAfterPaste(bool defaultValue = false) const;
    void setActivateTransformToolAfterPaste(bool value);
    
    enum RootSurfaceFormat {
        BT709_G22 = 0,
        BT709_G10,
        BT2020_PQ
    };
    RootSurfaceFormat rootSurfaceFormat(bool defaultValue = false) const;
    void setRootSurfaceFormat(RootSurfaceFormat value);

    static RootSurfaceFormat rootSurfaceFormat(QSettings *displayrc, bool defaultValue = false);
    static void setRootSurfaceFormat(QSettings *displayrc, RootSurfaceFormat value);

    bool useZip64(bool defaultValue = false) const;
    void setUseZip64(bool value);

    bool convertLayerColorSpaceInProperties(bool defaultValue = false) const;
    void setConvertLayerColorSpaceInProperties(bool value);

    bool renamePastedLayers(bool defaultValue = false) const;
    void setRenamePastedLayers(bool value);

    enum LayerInfoTextStyle {
        INFOTEXT_NONE = 0,
        INFOTEXT_SIMPLE,
        INFOTEXT_BALANCED,
        INFOTEXT_DETAILED
    };
    LayerInfoTextStyle layerInfoTextStyle(bool defaultValue = false) const;
    void setLayerInfoTextStyle(LayerInfoTextStyle value);

    int layerInfoTextOpacity(bool defaultValue = false) const;
    void setLayerInfoTextOpacity(int value);

    bool useInlineLayerInfoText(bool defaultValue = false) const;
    void setUseInlineLayerInfoText(bool value);

    bool useLayerSelectionCheckbox(bool defaultValue = false) const;
    void setUseLayerSelectionCheckbox(bool value);

    enum AssistantsDrawMode {
        ASSISTANTS_DRAW_MODE_DIRECT = 0,             // no caching, draw directly on canvas
        ASSISTANTS_DRAW_MODE_PIXMAP_CACHE = 1,
        ASSISTANTS_DRAW_MODE_LARGE_PIXMAP_CACHE = 2,
    };
    AssistantsDrawMode assistantsDrawMode(bool defaultValue = false) const;
    void setAssistantsDrawMode(AssistantsDrawMode value);

    bool longPressEnabled(bool defaultValue = false) const;
    void setLongPressEnabled(bool value);

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


    /// get the profile the color management system has stored for the given screen
    static const KoColorProfile* getScreenProfile(int screen);

private:
    KisConfig(const KisConfig&);
    KisConfig& operator=(const KisConfig&) const;


private:
    mutable KConfigGroup m_cfg;
    bool m_readOnly;
};

#endif // KIS_CONFIG_H_
