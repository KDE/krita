/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <shicmap@gmail.com>
   Copyright 2016 Scott Petrovic <scottpetrovic@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef KIS_POPUP_PALETTE_H
#define KIS_POPUP_PALETTE_H

#include <QElapsedTimer>
#include <QPushButton>
#include <QSlider>
#include <QGraphicsOpacityEffect>
#include "KisViewManager.h"
#include "kactioncollection.h"
#include "kis_tool_button.h"
#include "kis_highlighted_button.h"
#include <KisColorSelectorInterface.h>

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

class KisPopupPalette : public QWidget
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

    void showPopupPalette(const QPoint&);
    void showPopupPalette(bool b);

    //functions to set up selectedBrush
    void setSelectedBrush(int x);
    int selectedBrush() const;
    //functions to set up selectedColor
    void setSelectedColor(int x);
    int selectedColor() const;
    void setParent(QWidget *parent);

    void tabletEvent(QTabletEvent *event) override;

protected:

    void paintEvent(QPaintEvent*) override;
    void resizeEvent(QResizeEvent*) override;
    void mouseReleaseEvent(QMouseEvent*) override;
    void mouseMoveEvent(QMouseEvent*) override;
    void mousePressEvent(QMouseEvent*) override;

    //functions to calculate index of favorite brush or recent color in array
    //n is the total number of favorite brushes or recent colors
    int calculateIndex(QPointF, int n);

    int calculatePresetIndex(QPointF, int n);

    //functions to set up hoveredBrush
    void setHoveredPreset(int x);
    int hoveredPreset() const;
    //functions to set up hoveredColor
    void setHoveredColor(int x);
    int hoveredColor() const;

private:
    void setVisible(bool b) override;

    QPainterPath drawDonutPathFull(int, int, int, int);
    QPainterPath drawDonutPathAngle(int, int, int);
    QPainterPath drawRotationIndicator(qreal rotationAngle, bool canDrag);
    bool isPointInPixmap(QPointF&, int pos);

    QPainterPath createPathFromPresetIndex(int index);

    int numSlots();
    void adjustLayout(const QPoint &p);

private:
    int m_hoveredPreset {0};
    int m_hoveredColor {0};
    int m_selectedColor {0};

    QElapsedTimer m_timeSinceOpening;
    bool m_hadMousePressSinceOpening {false};


    KisCoordinatesConverter *m_coordinatesConverter;

    KisViewManager *m_viewManager;
    KisActionManager *m_actionManager;
    KisFavoriteResourceManager *m_resourceManager;
    KisColorSelectorInterface *m_triangleColorSelector {0};
    const KoColorDisplayRendererInterface *m_displayRenderer;
    QScopedPointer<KisSignalCompressor> m_colorChangeCompressor;
    KActionCollection *m_actionCollection;

    QTimer m_timer;

    KisBrushHud *m_brushHud {0};
    float m_popupPaletteSize {385.0};
    float m_colorHistoryInnerRadius {72.0};
    qreal m_colorHistoryOuterRadius {92.0};

    KisRoundHudButton *m_settingsButton {0};
    KisRoundHudButton *m_brushHudButton {0};
    QPoint m_lastCenterPoint;
    QRect m_canvasRotationIndicatorRect;
    QRect m_resetCanvasRotationIndicatorRect;
    bool m_isOverCanvasRotationIndicator {false};
    bool m_isRotatingCanvasIndicator {false};
    bool m_isZoomingCanvas {false};


    KisHighlightedToolButton *mirrorMode {0};
    KisHighlightedToolButton *canvasOnlyButton {0};
    QPushButton *zoomToOneHundredPercentButton {0};
    QSlider *zoomCanvasSlider {0};
    int zoomSliderMinValue {10};
    int zoomSliderMaxValue {200};
    KisAcyclicSignalConnector *m_acyclicConnector = 0;

    int m_cachedNumSlots {0};
    qreal m_cachedRadius {0.0};

    // updates the transparency and effects of the whole widget
    QGraphicsOpacityEffect *opacityChange {0};

Q_SIGNALS:
    void sigChangeActivePaintop(int);
    void sigUpdateRecentColor(int);
    void sigChangefGColor(const KoColor&);
    void sigUpdateCanvas();
    void zoomLevelChanged(int);

    // These are used to handle a bug:
    // If pop up palette is visible and a new colour is selected, the new colour
    // will be added when the user clicks on the canvas to hide the palette
    // In general, we want to be able to store recent color if the pop up palette
    // is not visible
    void sigEnableChangeFGColor(bool);
    void sigTriggerTimer();

public Q_SLOTS:
    void slotUpdateIcons();

private Q_SLOTS:
    void slotExternalFgColorChanged(const KoColor &color);
    void slotEmitColorChanged();
    void slotSetSelectedColor(int x) { setSelectedColor(x); update(); }
    void slotTriggerTimer();
    void slotEnableChangeFGColor();
    void slotUpdate() { update(); }
    void slotHide() { showPopupPalette(false); }
    void slotShowTagsPopup();
    void showHudWidget(bool visible);
    void slotZoomToOneHundredPercentClicked();
    void slotZoomSliderChanged(int zoom);

    void slotZoomSliderPressed();
    void slotZoomSliderReleased();

};

#endif // KIS_POPUP_PALETTE_H
