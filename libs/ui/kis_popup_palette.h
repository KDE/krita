/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2009 Vera Lukman <shicmap@gmail.com>
   SPDX-FileCopyrightText: 2016 Scott Petrovic <scottpetrovic@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only
*/

#ifndef KIS_POPUP_PALETTE_H
#define KIS_POPUP_PALETTE_H

#include <QPushButton>
#include <QSlider>
#include <QGraphicsOpacityEffect>
#include "KisViewManager.h"
#include "kactioncollection.h"
#include "kis_tool_button.h"
#include "KisHighlightedToolButton.h"
#include "KisColorSelectorInterface.h"
#include "KisPopupWidgetInterface.h"

class KisFavoriteResourceManager;
class QWidget;
class KoColor;
class KoTriangleColorSelector;
class KisSignalCompressor;
class KisBrushHud;
class KisRoundHudButton;
class KisCanvasResourceProvider;
class KisVisualColorSelector;
class KisAcyclicSignalConnector;
class KisMouseClickEater;


struct CachedPresetLayout {
    int ringCount{1};
    qreal firstRowRadius{0};
    qreal secondRowRadius{0};
    qreal thirdRowRadius{0};
    qreal firstRowPos{0};
    qreal secondRowPos{0};
    qreal thirdRowPos{0};
};

class KisPopupPalette : public QWidget, public KisPopupWidgetInterface
{
    Q_OBJECT

    Q_PROPERTY(int hoveredPreset READ hoveredPreset WRITE setHoveredPreset)
    Q_PROPERTY(int hoveredColor READ hoveredColor WRITE setHoveredColor)
    Q_PROPERTY(int selectedColor READ selectedColor WRITE setSelectedColor)

public:
    KisPopupPalette(KisViewManager*, KisCoordinatesConverter* ,KisFavoriteResourceManager*, const KoColorDisplayRendererInterface *displayRenderer,
                    KisCanvasResourceProvider *provider, QWidget *parent = 0);
    ~KisPopupPalette() override;
    QSize sizeHint() const override;

    //functions to set up selectedColor
    void setSelectedColor(int x);
    int selectedColor() const;
    void setParent(QWidget *parent);

    void tabletEvent(QTabletEvent *event) override;

    void popup(const QPoint& position) override;
    
    void ensureWithinParent(const QPoint& position, bool useUpperLeft);

protected:
    void showEvent(QShowEvent *event) override;
    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;

    /**
     * @brief Calculate index of recent color in array
     * @param numColors the total number of recent colors
     * @return -1 if numColors < 1
     */
    int calculateColorIndex(QPointF position, int numColors) const;
    /**
     * @brief/ find the index of the brush preset slot containing @position.
     * @return -1 if none is found
     */
    int findPresetSlot(QPointF position) const;

    //functions to set up hoveredBrush
    void setHoveredPreset(int x);
    int hoveredPreset() const;
    //functions to set up hoveredColor
    void setHoveredColor(int x);
    int hoveredColor() const;

private:
    void reconfigure();

    QPainterPath drawDonutPathFull(int, int, int, int);
    QPainterPath drawDonutPathAngle(int, int, int);
    QPainterPath drawFgBgColorIndicator(int type) const;
    QRectF rotationIndicatorRect(qreal rotationAngle) const;
    bool isPointInPixmap(QPointF&, int pos);

    QPointF drawPointOnAngle(qreal angle, qreal radius) const;
    /**
     * @brief Determine the number of rings to distribute the presets
     * and calculate the radius of the brush preset slots
     * and saves the "layout" to m_cachedPresetLayout.
    */
    void calculatePresetLayout();
    QPainterPath createPathFromPresetIndex(int index) const;

    void calculateRotationSnapAreas();

    QPoint m_mirrorPos {};
    int m_maxPresetSlotCount {10};
    int m_presetSlotCount {10};
    int m_hoveredPreset {0};
    bool m_useDynamicSlotCount {true};
    int m_hoveredColor {0};
    int m_selectedColor {0};
    bool m_isOverFgBgColors {false};
    bool m_snapRotation {false};
    qreal m_rotationSnapAngle {0};
    qreal m_snapRadius {15};
    std::array<QRect, 24> m_snapRects{};
    std::array<QLineF, 24> m_snapLines {};

    KisCoordinatesConverter *m_coordinatesConverter;

    KisViewManager *m_viewManager;
    KisActionManager *m_actionManager;
    KisFavoriteResourceManager *m_resourceManager;
    KisColorSelectorInterface *m_colorSelector {0};
    const KoColorDisplayRendererInterface *m_displayRenderer;
    QScopedPointer<KisSignalCompressor> m_colorChangeCompressor;
    KActionCollection *m_actionCollection;

    QSpacerItem *m_mainArea {0};
    KisBrushHud *m_brushHud {0};
    QWidget* m_bottomBarWidget {0};
    qreal m_popupPaletteSize {385.0};
    qreal m_colorHistoryInnerRadius {72.0};
    qreal m_colorHistoryOuterRadius {92.0};
    bool m_showColorHistory {true};
    qreal m_rotationTrackSize {18.0};
    bool m_showRotationTrack {true};
    qreal m_presetRingMargin {3.0};

    KisRoundHudButton *m_clearColorHistoryButton {0};
    KisRoundHudButton *m_tagsButton {0};
    KisRoundHudButton *m_bottomBarButton {0};
    KisRoundHudButton *m_brushHudButton {0};
    QRectF m_canvasRotationIndicatorRect;
    QRectF m_resetCanvasRotationIndicatorRect;
    bool m_isOverCanvasRotationIndicator {false};
    bool m_isOverResetCanvasRotationIndicator{false};
    bool m_isRotatingCanvasIndicator{false};
    bool m_isZoomingCanvas {false};

    KisHighlightedToolButton *mirrorMode {0};
    KisHighlightedToolButton *canvasOnlyButton {0};
    QPushButton *zoomToOneHundredPercentButton {0};
    QSlider *zoomCanvasSlider {0};
    int zoomSliderMinValue {10};
    int zoomSliderMaxValue {200};
    KisAcyclicSignalConnector *m_acyclicConnector = 0;

    CachedPresetLayout m_cachedPresetLayout;

    // updates the transparency and effects of the whole widget
    QGraphicsOpacityEffect *opacityChange {0};
    KisMouseClickEater *m_clicksEater;

Q_SIGNALS:
    void sigChangeActivePaintop(int);
    void sigUpdateRecentColor(int);
    void sigChangefGColor(const KoColor&);
    void sigUpdateCanvas();
    void zoomLevelChanged(int);

public Q_SLOTS:
    void slotUpdateIcons();

private Q_SLOTS:
    void slotSetMirrorPos();
    void slotRemoveMirrorPos();
    void slotDisplayConfigurationChanged();
    void slotConfigurationChanged();
    void slotExternalFgColorChanged(const KoColor &color);
    void slotEmitColorChanged();
    void slotSetSelectedColor(int x) { setSelectedColor(x); update(); }
    void slotUpdate();
    void slotHide() { setVisible(false); }
    void slotShowTagsPopup();
    void showHudWidget(bool visible);
    void showBottomBarWidget(bool visible);
    void slotZoomToOneHundredPercentClicked();
    void slotZoomSliderChanged(int zoom);

    void slotZoomSliderPressed();
    void slotZoomSliderReleased();
};

#endif // KIS_POPUP_PALETTE_H
