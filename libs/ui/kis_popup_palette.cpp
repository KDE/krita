/* This file is part of the KDE project
   SPDX-FileCopyrightText: 2009 Vera Lukman <shicmap@gmail.com>
   SPDX-FileCopyrightText: 2011 Sven Langkamp <sven.langkamp@gmail.com>
   SPDX-FileCopyrightText: 2016 Scott Petrovic <scottpetrovic@gmail.com>

   SPDX-License-Identifier: LGPL-2.0-only

*/
#include <QtGui>
#include <QMenu>
#include <QWhatsThis>
#include <QVBoxLayout>

#include <KisTagModel.h>

#include "kis_canvas2.h"
#include "kis_config.h"
#include "kis_popup_palette.h"
#include "kis_favorite_resource_manager.h"
#include "kis_icon_utils.h"
#include <kis_canvas_resource_provider.h>
#include <KoTriangleColorSelector.h>
#include "KoColorDisplayRendererInterface.h"
#include <KisVisualColorSelector.h>
#include <kis_config_notifier.h>
#include "kis_signal_compressor.h"
#include "brushhud/kis_brush_hud.h"
#include "brushhud/kis_round_hud_button.h"
#include "kis_signals_blocker.h"
#include "kis_canvas_controller.h"
#include "kis_acyclic_signal_connector.h"
#include <kis_paintop_preset.h>
#include "KisMouseClickEater.h"

static const int BRUSH_HUD_MARGIN = 16;
static const qreal BORDER_WIDTH = 3.0;

class PopupColorTriangle : public KoTriangleColorSelector
{
public:
    PopupColorTriangle(const KoColorDisplayRendererInterface *displayRenderer, QWidget* parent)
        : KoTriangleColorSelector(displayRenderer, parent)
        , m_dragging(false)
    {
    }

    ~PopupColorTriangle() override {}

    void tabletEvent(QTabletEvent* event) override {
        event->accept();
        QMouseEvent* mouseEvent = 0;

        // ignore any tablet events that are done with the right click
        // Tablet move events don't return a "button", so catch that too
        if(event->button() == Qt::LeftButton || event->type() == QEvent::TabletMove)
        {
            switch (event->type()) {
                case QEvent::TabletPress:
                    mouseEvent = new QMouseEvent(QEvent::MouseButtonPress, event->pos(),
                                                 Qt::LeftButton, Qt::LeftButton, event->modifiers());
                    m_dragging = true;
                    mousePressEvent(mouseEvent);
                    break;
                case QEvent::TabletMove:
                    mouseEvent = new QMouseEvent(QEvent::MouseMove, event->pos(),
                                                 (m_dragging) ? Qt::LeftButton : Qt::NoButton,
                                                 (m_dragging) ? Qt::LeftButton : Qt::NoButton, event->modifiers());
                    mouseMoveEvent(mouseEvent);
                    break;
                case QEvent::TabletRelease:
                    mouseEvent = new QMouseEvent(QEvent::MouseButtonRelease, event->pos(),
                                                 Qt::LeftButton,
                                                 Qt::LeftButton,
                                                 event->modifiers());
                    m_dragging = false;
                    mouseReleaseEvent(mouseEvent);
                    break;
                default: break;
            }
        }

        delete mouseEvent;
    }

private:
    bool m_dragging;
};

KisPopupPalette::KisPopupPalette(KisViewManager* viewManager, KisCoordinatesConverter* coordinatesConverter ,KisFavoriteResourceManager* manager,
                                 const KoColorDisplayRendererInterface *displayRenderer, KisCanvasResourceProvider *provider, QWidget *parent)
    : QWidget(parent, Qt::FramelessWindowHint)
    , m_coordinatesConverter(coordinatesConverter)
    , m_viewManager(viewManager)
    , m_actionManager(viewManager->actionManager())
    , m_resourceManager(manager)
    , m_displayRenderer(displayRenderer)
    , m_colorChangeCompressor(new KisSignalCompressor(50, KisSignalCompressor::POSTPONE))
    , m_actionCollection(viewManager->actionCollection())
    , m_acyclicConnector(new KisAcyclicSignalConnector(this))
    , m_clicksEater(new KisMouseClickEater(Qt::RightButton, 1, this))
{
    connect(m_colorChangeCompressor.data(), SIGNAL(timeout()),
            SLOT(slotEmitColorChanged()));

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), this, SLOT(slotConfigurationChanged()));
    connect(m_displayRenderer,  SIGNAL(displayConfigurationChanged()), this, SLOT(slotDisplayConfigurationChanged()));

    m_acyclicConnector->connectForwardKoColor(m_resourceManager, SIGNAL(sigChangeFGColorSelector(KoColor)),
                                              this, SLOT(slotExternalFgColorChanged(KoColor)));

    m_acyclicConnector->connectBackwardKoColor(this, SIGNAL(sigChangefGColor(KoColor)),
                                               m_resourceManager, SIGNAL(sigSetFGColor(KoColor)));

    connect(this, SIGNAL(sigChangeActivePaintop(int)), m_resourceManager, SLOT(slotChangeActivePaintop(int)));
    connect(this, SIGNAL(sigUpdateRecentColor(int)), m_resourceManager, SLOT(slotUpdateRecentColor(int)));

    connect(m_resourceManager, SIGNAL(setSelectedColor(int)), SLOT(slotSetSelectedColor(int)));
    connect(m_resourceManager, SIGNAL(updatePalettes()), SLOT(slotUpdate()));
    connect(m_resourceManager, SIGNAL(hidePalettes()), SLOT(slotHide()));

    setCursor(Qt::ArrowCursor);
    setMouseTracking(true);
    setHoveredPreset(-1);
    setHoveredColor(-1);
    setSelectedColor(-1);

    m_brushHud = new KisBrushHud(provider, this);

    m_tagsButton = new KisRoundHudButton(this);

    connect(m_tagsButton, SIGNAL(clicked()), SLOT(slotShowTagsPopup()));

    m_brushHudButton = new KisRoundHudButton(this);
    m_brushHudButton->setCheckable(true);

    connect(m_brushHudButton, SIGNAL(toggled(bool)), SLOT(showHudWidget(bool)));
    
    m_bottomBarWidget = new QWidget(this);
    
    m_bottomBarButton = new KisRoundHudButton(this);
    m_bottomBarButton->setCheckable(true);

    connect( m_bottomBarButton, SIGNAL(toggled(bool)), SLOT(showBottomBarWidget(bool)));


    // add some stuff below the pop-up palette that will make it easier to use for tablet people
    QGridLayout* gLayout = new QGridLayout(this);
    gLayout->setSizeConstraint(QLayout::SetFixedSize);
    gLayout->setSpacing(0);
    gLayout->setContentsMargins(QMargins());
    m_mainArea = new QSpacerItem(m_popupPaletteSize, m_popupPaletteSize);
    gLayout->addItem(m_mainArea, 0, 0); // this should push the box to the bottom
    gLayout->setColumnMinimumWidth(1, BRUSH_HUD_MARGIN);
    gLayout->addWidget(m_brushHud, 0, 2);
    gLayout->addWidget(m_bottomBarWidget, 2, 0);

    QHBoxLayout* hLayout = new QHBoxLayout(m_bottomBarWidget);
    
    mirrorMode = new KisHighlightedToolButton(this);
    mirrorMode->setFixedSize(35, 35);

    mirrorMode->setToolTip(i18n("Mirror Canvas"));
    mirrorMode->setDefaultAction(m_actionCollection->action("mirror_canvas"));

    canvasOnlyButton = new KisHighlightedToolButton(this);
    canvasOnlyButton->setFixedSize(35, 35);

    canvasOnlyButton->setToolTip(i18n("Canvas Only"));
    canvasOnlyButton->setDefaultAction(m_actionCollection->action("view_show_canvas_only"));

    zoomToOneHundredPercentButton = new QPushButton(this);
    zoomToOneHundredPercentButton->setText(i18n("100%"));
    zoomToOneHundredPercentButton->setFixedHeight(35);

    zoomToOneHundredPercentButton->setToolTip(i18n("Zoom to 100%"));
    connect(zoomToOneHundredPercentButton, SIGNAL(clicked(bool)), this, SLOT(slotZoomToOneHundredPercentClicked()));

    zoomCanvasSlider = new QSlider(Qt::Horizontal, this);
    zoomSliderMinValue = 10; // set in %
    zoomSliderMaxValue = 200; // set in %

    zoomCanvasSlider->setRange(zoomSliderMinValue, zoomSliderMaxValue);
    zoomCanvasSlider->setFixedHeight(35);
    zoomCanvasSlider->setValue(m_coordinatesConverter->zoomInPercent());

    zoomCanvasSlider->setSingleStep(1);
    zoomCanvasSlider->setPageStep(1);

    connect(zoomCanvasSlider, SIGNAL(valueChanged(int)), this, SLOT(slotZoomSliderChanged(int)));
    connect(zoomCanvasSlider, SIGNAL(sliderPressed()), this, SLOT(slotZoomSliderPressed()));
    connect(zoomCanvasSlider, SIGNAL(sliderReleased()), this, SLOT(slotZoomSliderReleased()));

    clearHistoryButton = new QPushButton(this);
    clearHistoryButton->setFixedHeight(35);

    clearHistoryButton->setText(i18nc("verb, to clear", "Clear colors"));
    clearHistoryButton->setToolTip(i18n("Clear the colors of the popup palette"));

    connect(clearHistoryButton, SIGNAL(clicked(bool)), m_resourceManager, SLOT(slotClearHistory()));
    //Otherwise the colors won't disappear until the cursor moves away from the button:
    connect(clearHistoryButton, SIGNAL(released()), this, SLOT(slotUpdate()));
    
    slotUpdateIcons();

    hLayout->setSpacing(2);
    hLayout->setContentsMargins(0, 6, 0, 0);
    hLayout->addWidget(mirrorMode);
    hLayout->addWidget(canvasOnlyButton);
    hLayout->addWidget(zoomToOneHundredPercentButton);
    hLayout->addWidget(zoomCanvasSlider);
    hLayout->addWidget(clearHistoryButton);
    
    setVisible(true);
    setVisible(false);
    reconfigure();

    opacityChange = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityChange);

    /**
     * Tablet support code generates a spurious right-click right after opening
     * the window, so we should ignore it. Next right-click will be used for
     * closing the popup palette
     */
    this->installEventFilter(m_clicksEater);
    m_colorSelector->installEventFilter(m_clicksEater);

    // Prevent tablet events from being captured by the canvas
    setAttribute(Qt::WA_NoMousePropagation, true);
    
    // Load configuration..
    KisConfig cfg(true);
    m_brushHudButton->setChecked(cfg.showBrushHud());
    m_bottomBarButton->setChecked(cfg.showPaletteBottomBar());
}

void KisPopupPalette::slotConfigurationChanged()
{
    reconfigure();
    layout()->invalidate();
}

void KisPopupPalette::reconfigure()
{
    KisConfig config(true);
    m_presetSlotCount = qMax(config.favoritePresets(), 10);
    m_popupPaletteSize = config.readEntry("popuppalette/size", 385);
    qreal selectorRadius = config.readEntry("popuppalette/selectorSize", 140) / 2;
    
    m_showColorHistory = config.readEntry("popuppalette/showColorHistory", true);
    m_showRotationTrack = config.readEntry("popuppalette/showRotationTrack", true);
    
    m_colorHistoryInnerRadius = selectorRadius + m_presetRingMargin;
    m_colorHistoryOuterRadius = m_colorHistoryInnerRadius;
    if (m_showColorHistory) m_colorHistoryOuterRadius += 20;

    m_mainArea->changeSize(m_popupPaletteSize, m_popupPaletteSize);

    bool useVisualSelector = config.readEntry<bool>("popuppalette/usevisualcolorselector", false);
    if (m_colorSelector) {
        // if the selector type changed, delete it
        bool haveVisualSelector = qobject_cast<KisVisualColorSelector*>(m_colorSelector) != 0;
        if (useVisualSelector != haveVisualSelector) {
            delete m_colorSelector;
            m_colorSelector = 0;
        }
    }
    if (!m_colorSelector) {
        if (useVisualSelector) {
            KisVisualColorSelector *selector = new KisVisualColorSelector(this);
            selector->setAcceptTabletEvents(true);
            m_colorSelector = selector;
        }
        else {
            m_colorSelector  = new PopupColorTriangle(m_displayRenderer, this);
            connect(m_colorSelector, SIGNAL(requestCloseContainer()), this, SLOT(slotHide()));
        }
        m_colorSelector->setDisplayRenderer(m_displayRenderer);
        m_colorSelector->setConfig(true,false);
        m_colorSelector->setVisible(true);
        slotDisplayConfigurationChanged();
        connect(m_colorSelector, SIGNAL(sigNewColor(KoColor)),
                m_colorChangeCompressor.data(), SLOT(start()));
        connect(KisConfigNotifier::instance(), SIGNAL(configChanged()),
                m_colorSelector, SLOT(configurationChanged()));
    }


    const int auxButtonSize = 35;
    m_colorSelector->move(m_popupPaletteSize/2 - selectorRadius, m_popupPaletteSize/2 - selectorRadius);
    m_colorSelector->resize(m_popupPaletteSize - 2*m_colorSelector->x(), m_popupPaletteSize - 2*m_colorSelector->y());

    // ellipse - to make sure the widget doesn't eat events meant for recent colors or brushes
    //         - needs to be +2 pixels on every side for anti-aliasing to look nice on high dpi displays
    // rectange - to make sure the area doesn't extend outside of the widget
    QRegion maskedEllipse(-2, -2, m_colorSelector->width() + 4, m_colorSelector->height() + 4, QRegion::Ellipse );
    QRegion maskedRectange(0, 0, m_colorSelector->width(), m_colorSelector->height(), QRegion::Rectangle);
    QRegion maskedRegion = maskedEllipse.intersected(maskedRectange);

    m_colorSelector->setMask(maskedRegion);

    m_brushHud->setFixedHeight(int(m_popupPaletteSize));

    m_bottomBarButton->setGeometry(m_popupPaletteSize - 3.4 * auxButtonSize, m_popupPaletteSize - auxButtonSize,
                                  auxButtonSize, auxButtonSize);
    m_tagsButton->setGeometry(m_popupPaletteSize - 2.2 * auxButtonSize, m_popupPaletteSize - auxButtonSize,
                                  auxButtonSize, auxButtonSize);
    m_brushHudButton->setGeometry(m_popupPaletteSize - 1.0 * auxButtonSize, m_popupPaletteSize - auxButtonSize,
                                  auxButtonSize, auxButtonSize);
    calculatePresetLayout();
}

void KisPopupPalette::slotDisplayConfigurationChanged()
{
    // Visual Color Selector picks up color space from input
    KoColor col = m_viewManager->canvasResourceProvider()->fgColor();
    const KoColorSpace *paintingCS = m_displayRenderer->getPaintingColorSpace();
    //hack to get around cmyk for now.
    if (paintingCS->colorChannelCount()>3) {
        paintingCS = KoColorSpaceRegistry::instance()->rgb8();
    }
    m_colorSelector->slotSetColorSpace(paintingCS);
    m_colorSelector->slotSetColor(col);
}

void KisPopupPalette::slotExternalFgColorChanged(const KoColor &color)
{
    m_colorSelector->slotSetColor(color);
}

void KisPopupPalette::slotEmitColorChanged()
{
    if (isVisible()) {
        update();
        emit sigChangefGColor(m_colorSelector->getCurrentColor());
    }
}

//setting KisPopupPalette properties
int KisPopupPalette::hoveredPreset() const
{
    return m_hoveredPreset;
}

void KisPopupPalette::setHoveredPreset(int x)
{
    m_hoveredPreset = x;
}

int KisPopupPalette::hoveredColor() const
{
    return m_hoveredColor;
}

void KisPopupPalette::setHoveredColor(int x)
{
    m_hoveredColor = x;
}

int KisPopupPalette::selectedColor() const
{
    return m_selectedColor;
}

void KisPopupPalette::setSelectedColor(int x)
{
    m_selectedColor = x;
}

void KisPopupPalette::slotZoomSliderChanged(int zoom) {
    emit zoomLevelChanged(zoom);
}

void KisPopupPalette::slotZoomSliderPressed()
{
   m_isZoomingCanvas = true;
}

void KisPopupPalette::slotZoomSliderReleased()
{
    m_isZoomingCanvas = false;
}

void KisPopupPalette::slotUpdateIcons()
{
    this->setPalette(qApp->palette());

    for(int i=0; i<this->children().size(); i++) {
        QWidget *w = qobject_cast<QWidget*>(this->children().at(i));
        if (w) {
            w->setPalette(qApp->palette());
        }
    }
    zoomToOneHundredPercentButton->setIcon(m_actionCollection->action("zoom_to_100pct")->icon());
    m_brushHud->updateIcons();
    m_tagsButton->setIcon(KisIconUtils::loadIcon("tag"));
    m_bottomBarButton->setOnOffIcons(KisIconUtils::loadIcon("arrow-up"), KisIconUtils::loadIcon("arrow-down"));
    m_brushHudButton->setOnOffIcons(KisIconUtils::loadIcon("arrow-left"), KisIconUtils::loadIcon("arrow-right"));
}

void KisPopupPalette::showHudWidget(bool visible)
{
    const bool reallyVisible = visible && m_brushHudButton->isChecked();

    if (reallyVisible) {
        m_brushHud->updateProperties();
    }

    m_brushHud->setVisible(reallyVisible);

    KisConfig cfg(false);
    cfg.setShowBrushHud(visible);
}

void KisPopupPalette::showBottomBarWidget(bool visible)
{
    const bool reallyVisible = visible && m_bottomBarButton->isChecked();

    m_bottomBarWidget->setVisible(reallyVisible);

    KisConfig cfg(false);
    cfg.setShowPaletteBottomBar(visible);
}

void KisPopupPalette::setParent(QWidget *parent) {
    QWidget::setParent(parent);
}


QSize KisPopupPalette::sizeHint() const
{
    // Note: the canvas popup widget system "abuses" the sizeHint to determine
    // the position to show the widget; this does not reflect the true size.
    return QSize(m_popupPaletteSize, m_popupPaletteSize);
}

void KisPopupPalette::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);

    QPainter painter(this);

    QPen pen(palette().color(QPalette::Text), BORDER_WIDTH);
    painter.setPen(pen);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    QPointF edgePoint = QPointF(0.14645, 0.14645) * m_popupPaletteSize;
    // painting background color indicator
    QPainterPath bgColor;
    bgColor.addEllipse(edgePoint + QPointF(-6, 24), 30, 30);
    painter.fillPath(bgColor, m_displayRenderer->toQColor(m_resourceManager->bgColor()));
    painter.drawPath(bgColor);

    // painting foreground color indicator
    QPainterPath fgColor;
    fgColor.addEllipse(edgePoint + QPointF(4, -6), 30, 30);
    painter.fillPath(fgColor, m_displayRenderer->toQColor(m_colorSelector->getCurrentColor()));
    painter.drawPath(fgColor);

    // create a circle background that everything else will go into
    QPainterPath backgroundContainer;

    // draws the circle halfway into the border so that the border never goes past the bounds of the popup
    QRectF circleRect(BORDER_WIDTH/2, BORDER_WIDTH/2, m_popupPaletteSize - BORDER_WIDTH, m_popupPaletteSize - BORDER_WIDTH);
    backgroundContainer.addEllipse(circleRect);
    painter.fillPath(backgroundContainer, palette().brush(QPalette::Background));
    painter.drawPath(backgroundContainer);

    if (m_showRotationTrack) {
        painter.save();
        // create a path slightly inside the container circle. this will create a 'track' to indicate that we can rotate the canvas
        // with the indicator
        QPainterPath rotationTrackPath;
        QRectF circleRect2(m_rotationTrackSize, m_rotationTrackSize,
                        m_popupPaletteSize - m_rotationTrackSize * 2, m_popupPaletteSize - m_rotationTrackSize * 2);

        rotationTrackPath.addEllipse(circleRect2);
        QPen pen = painter.pen();
        pen.setColor(palette().color(QPalette::Background).lighter(150));
        pen.setWidth(2);
        painter.setPen(pen);
        painter.drawPath(rotationTrackPath);
        painter.restore();
    }
    
    // the following things needs to be based off the center, so let's translate the painter
    painter.translate(m_popupPaletteSize / 2, m_popupPaletteSize / 2);

    if (m_showRotationTrack) {
        painter.save();
        // create a reset canvas rotation indicator to bring the canvas back to 0 degrees
        QRectF resetRotationIndicator = rotationIndicatorRect(0, false);

        painter.setPen(QPen(palette().color(QPalette::Text), 1.0));
        painter.setBrush(Qt::NoBrush);
        painter.drawEllipse(resetRotationIndicator);
        QRectF rotationIndicator = rotationIndicatorRect(m_coordinatesConverter->rotationAngle(), false);

        // create the canvas rotation handle
        painter.setBrush(palette().brush(QPalette::Text));

        // hover indicator for the canvas rotation
        // draw highlight if either just hovering or currently rotating
        if (m_isOverCanvasRotationIndicator || m_isRotatingCanvasIndicator) {
            QPen pen(palette().color(QPalette::Highlight), 2.0);
            painter.setPen(pen);
        }
        painter.drawEllipse(rotationIndicator);

        painter.restore();
    }

    // painting favorite brushes
    QList<QImage> images(m_resourceManager->favoritePresetImages());

    // painting favorite brushes pixmap/icon
    QPainterPath presetPath;
    for (int pos = 0; pos < numSlots(); pos++) {
        painter.save();

        presetPath = createPathFromPresetIndex(pos);

        if (pos < images.size()) {
            painter.setClipPath(presetPath);

            QRect bounds = presetPath.boundingRect().toAlignedRect();
            QImage previewHighDPI = images.at(pos).scaled(bounds.size()*devicePixelRatioF() , Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
            previewHighDPI.setDevicePixelRatio(devicePixelRatioF());
            painter.drawImage(bounds.topLeft(), previewHighDPI);
        }
        else {
            painter.fillPath(presetPath, palette().brush(QPalette::Window));  // brush slot that has no brush in it
        }
        // needs to be called here so that the clipping is removed
        painter.restore();
        QPen pen = painter.pen();
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawPath(presetPath);
    }
    if (hoveredPreset() > -1) {
        presetPath = createPathFromPresetIndex(hoveredPreset());
        painter.setPen(QPen(palette().color(QPalette::Highlight), BORDER_WIDTH));
        painter.drawPath(presetPath);
    }

    if (m_showColorHistory) {
        // paint recent colors area.
        painter.setPen(Qt::NoPen);
        float rotationAngle = -360.0 / m_resourceManager->recentColorsTotal();

        // there might be no recent colors at the start, so paint a placeholder
        if (m_resourceManager->recentColorsTotal() == 0) {
            painter.setBrush(Qt::transparent);

            QPainterPath emptyRecentColorsPath(drawDonutPathFull(0, 0, m_colorHistoryInnerRadius, m_colorHistoryOuterRadius));
            painter.setPen(QPen(palette().color(QPalette::Background).lighter(150), 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));
            painter.drawPath(emptyRecentColorsPath);
        } else {

            for (int pos = 0; pos < m_resourceManager->recentColorsTotal(); pos++) {
                QPainterPath recentColorsPath(drawDonutPathAngle(m_colorHistoryInnerRadius, m_colorHistoryOuterRadius, m_resourceManager->recentColorsTotal()));

                //accessing recent color of index pos
                painter.fillPath(recentColorsPath, m_displayRenderer->toQColor( m_resourceManager->recentColorAt(pos) ));
                painter.drawPath(recentColorsPath);
                painter.rotate(rotationAngle);
            }
        }

        // painting hovered color
        if (hoveredColor() > -1) {
            painter.setPen(QPen(palette().color(QPalette::Highlight), 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));

            if (m_resourceManager->recentColorsTotal() == 1) {
                QPainterPath path_ColorDonut(drawDonutPathFull(0, 0, m_colorHistoryInnerRadius, m_colorHistoryOuterRadius));
                painter.drawPath(path_ColorDonut);
            } else {
                painter.rotate((m_resourceManager->recentColorsTotal() + hoveredColor()) *rotationAngle);
                QPainterPath path(drawDonutPathAngle(m_colorHistoryInnerRadius, m_colorHistoryOuterRadius, m_resourceManager->recentColorsTotal()));
                painter.drawPath(path);
                painter.rotate(hoveredColor() * -1 * rotationAngle);
            }
        }

        // painting selected color
        if (selectedColor() > -1) {
            painter.setPen(QPen(palette().color(QPalette::Highlight).darker(130), 2, Qt::SolidLine, Qt::FlatCap, Qt::MiterJoin));

            if (m_resourceManager->recentColorsTotal() == 1) {
                QPainterPath path_ColorDonut(drawDonutPathFull(0, 0, m_colorHistoryInnerRadius, m_colorHistoryOuterRadius));
                painter.drawPath(path_ColorDonut);
            } else {
                painter.rotate((m_resourceManager->recentColorsTotal() + selectedColor()) *rotationAngle);
                QPainterPath path(drawDonutPathAngle(m_colorHistoryInnerRadius, m_colorHistoryOuterRadius, m_resourceManager->recentColorsTotal()));
                painter.drawPath(path);
                painter.rotate(selectedColor() * -1 * rotationAngle);
            }
        }
    }


    // if we are actively rotating the canvas or zooming, make the panel slightly transparent to see the canvas better
    if(m_isRotatingCanvasIndicator || m_isZoomingCanvas) {
        opacityChange->setOpacity(0.4);
    } else {
        opacityChange->setOpacity(1.0);
    }

}

void KisPopupPalette::resizeEvent(QResizeEvent* resizeEvent) {
    Q_UNUSED(resizeEvent);
    m_resetCanvasRotationIndicatorRect = rotationIndicatorRect(0, true);
    m_canvasRotationIndicatorRect = rotationIndicatorRect(m_coordinatesConverter->rotationAngle(), true);
    // Ensure that the resized geometry fits within the desired rect...
    QRect tempGeo = rect(); 
    tempGeo.translate(pos());
    ensureWithinParent(tempGeo.topLeft(), true);
}

QPainterPath KisPopupPalette::drawDonutPathFull(int x, int y, int inner_radius, int outer_radius)
{
    QPainterPath path;
    path.addEllipse(QPointF(x, y), outer_radius, outer_radius);
    path.addEllipse(QPointF(x, y), inner_radius, inner_radius);
    path.setFillRule(Qt::OddEvenFill);

    return path;
}

QPainterPath KisPopupPalette::drawDonutPathAngle(int inner_radius, int outer_radius, int limit)
{
    QPainterPath path;
    path.moveTo(-0.999 * outer_radius * sin(M_PI / limit), 0.999 * outer_radius * cos(M_PI / limit));
    path.arcTo(-1 * outer_radius, -1 * outer_radius, 2 * outer_radius, 2 * outer_radius, -90.0 - 180.0 / limit,
               360.0 / limit);
    path.arcTo(-1 * inner_radius, -1 * inner_radius, 2 * inner_radius, 2 * inner_radius, -90.0 + 180.0 / limit,
               - 360.0 / limit);
    path.closeSubpath();

    return path;
}

QRectF KisPopupPalette::rotationIndicatorRect(qreal rotationAngle, bool absolute) const
{
    qreal indicatorRadians = qDegreesToRadians(rotationAngle - 90);  // -90 will make 0 degrees be at the top
    qreal paletteRadius = 0.5 * m_popupPaletteSize;
    QPointF rotationDialPosition(qCos(indicatorRadians) * (paletteRadius - 10),
                                 qSin(indicatorRadians) * (paletteRadius - 10));
    if (absolute) {
        rotationDialPosition += QPointF(paletteRadius, paletteRadius);
    }
    QPointF indicatorDiagonal(7.5, 7.5);
    return QRectF(rotationDialPosition - indicatorDiagonal, rotationDialPosition + indicatorDiagonal);
}

void KisPopupPalette::mouseMoveEvent(QMouseEvent *event)
{
    QPointF point = event->localPos();
    event->accept();

    if (m_showRotationTrack) {
        // check if mouse is over the canvas rotation knob
        bool wasOverRotationIndicator = m_isOverCanvasRotationIndicator;
        m_isOverCanvasRotationIndicator = m_canvasRotationIndicatorRect.contains(point);

        if (wasOverRotationIndicator != m_isOverCanvasRotationIndicator) {
            update();
        }

        if (m_isRotatingCanvasIndicator) {
            // we are rotating the canvas, so calculate the rotation angle based off the center
            // calculate the angle we are at first
            QPoint widgetCenterPoint = QPoint(m_popupPaletteSize/2, m_popupPaletteSize/2);

            float dX = point.x() - widgetCenterPoint.x();
            float dY = point.y() - widgetCenterPoint.y();


            float finalAngle = qAtan2(dY,dX) * 180 / M_PI; // what we need if we have two points, but don't know the angle
            finalAngle = finalAngle + 90; // add 90 degrees so 0 degree position points up
            float angleDifference = finalAngle - m_coordinatesConverter->rotationAngle(); // the rotation function accepts diffs, so find it out

            KisCanvasController *canvasController =
                dynamic_cast<KisCanvasController*>(m_viewManager->canvasBase()->canvasController());
            canvasController->rotateCanvas(angleDifference);
            m_canvasRotationIndicatorRect = rotationIndicatorRect(finalAngle, true);

            update();
            emit sigUpdateCanvas();
        }
    }

    if (m_isRotatingCanvasIndicator == false) {
        QPainterPath colorHistoryPath(drawDonutPathFull(m_popupPaletteSize / 2, m_popupPaletteSize / 2, m_colorHistoryInnerRadius, m_colorHistoryOuterRadius));
        if (colorHistoryPath.contains(point)) {
            if (hoveredPreset() >= 0) {
                setToolTip(QString());
                setHoveredPreset(-1);
            }

            int pos = calculateColorIndex(point, m_resourceManager->recentColorsTotal());

            if (pos != hoveredColor()) {
                setHoveredColor(pos);
                update();
            }
        }
        else {
            if (hoveredColor() >= 0) {
                setHoveredColor(-1);
                update();
            }

            int pos = findPresetSlot(point);

            if (pos != hoveredPreset()) {

                if (pos >= 0 && pos < m_resourceManager->numFavoritePresets()) {
                    setToolTip(m_resourceManager->favoritePresetNamesList().at(pos));
                    setHoveredPreset(pos);
                }
                else {
                    setToolTip(QString());
                    setHoveredPreset(-1);
                }

                update();
            }
        }
    }
}

void KisPopupPalette::mousePressEvent(QMouseEvent *event)
{
    QPointF point = event->localPos();
    event->accept();

    if (event->button() == Qt::LeftButton) {
        if (m_showRotationTrack) {
            if (m_isOverCanvasRotationIndicator) {
                m_isRotatingCanvasIndicator = true;
                update();
            }

            if (m_resetCanvasRotationIndicatorRect.contains(point)) {
                float angleDifference = -m_coordinatesConverter->rotationAngle(); // the rotation function accepts diffs
                KisCanvasController *canvasController =
                        dynamic_cast<KisCanvasController*>(m_viewManager->canvasBase()->canvasController());
                canvasController->rotateCanvas(angleDifference);
                m_canvasRotationIndicatorRect = rotationIndicatorRect(0, true);

                emit sigUpdateCanvas();
            }
        }
    }
}

void KisPopupPalette::slotShowTagsPopup()
{
    KisTagModel model (ResourceType::PaintOpPresets);
    QVector<QString> tags;
    for (int i = 0; i < model.rowCount(); ++i) {
        QModelIndex idx = model.index(i, 0);
        tags << model.data(idx, Qt::DisplayRole).toString();
    }

    //std::sort(tags.begin(), tags.end());

    if (!tags.isEmpty()) {
        QMenu menu;
        Q_FOREACH (const QString& tag, tags) {
            menu.addAction(tag);
        }

        QAction *action = menu.exec(QCursor::pos());
        if (action) {

            for (int i = 0; i < model.rowCount(); ++i) {
                QModelIndex idx = model.index(i, 0);
                if (model.data(idx, Qt::DisplayRole).toString() == KLocalizedString::removeAcceleratorMarker(action->text())) {
                    m_resourceManager->setCurrentTag(model.tagForIndex(idx));
                    break;
                }
            }
        }
    } else {
        QWhatsThis::showText(QCursor::pos(),
                             i18n("There are no tags available to show in this popup. To add presets, you need to tag them and then select the tag here."));
    }

}

void KisPopupPalette::slotZoomToOneHundredPercentClicked() {
    QAction *action = m_actionCollection->action("zoom_to_100pct");

    if (action) {
        action->trigger();
    }

    // also move the zoom slider to 100% position so they are in sync
    zoomCanvasSlider->setValue(100);
}

void KisPopupPalette::tabletEvent(QTabletEvent *event) {
    event->ignore();
}

void KisPopupPalette::popup(const QPoint &position) {
    setVisible(!isVisible());
    ensureWithinParent(position, false);
}

void KisPopupPalette::ensureWithinParent(const QPoint& position, bool useUpperLeft) {
    if (isVisible() && parentWidget())  {
        const float widgetMargin = -20.0f;
        const QRect fitRect = kisGrowRect(parentWidget()->rect(), widgetMargin);
        const QPoint paletteCenterOffset(sizeHint().width() / 2, sizeHint().height() / 2);

        QRect paletteRect = rect();

        if (!useUpperLeft) {
            paletteRect.moveTo(position - paletteCenterOffset);
        } else {
            paletteRect.moveTopLeft(position);
        }

        paletteRect = kisEnsureInRect(paletteRect, fitRect);
        move(paletteRect.topLeft());
    }
}

void KisPopupPalette::showEvent(QShowEvent *event)
{
    m_clicksEater->reset();

    // don't set the zoom slider if we are outside of the zoom slider bounds. It will change the zoom level to within
    // the bounds and cause the canvas to jump between the slider's min and max
    if (m_coordinatesConverter->zoomInPercent() > zoomSliderMinValue &&
            m_coordinatesConverter->zoomInPercent() < zoomSliderMaxValue){
        KisSignalsBlocker b(zoomCanvasSlider);
        zoomCanvasSlider->setValue(m_coordinatesConverter->zoomInPercent()); // sync the zoom slider
    }

    m_brushHud->setVisible(m_brushHudButton->isChecked());
    m_bottomBarWidget->setVisible(m_bottomBarButton->isChecked());

    QWidget::showEvent(event);
}

void KisPopupPalette::mouseReleaseEvent(QMouseEvent *event)
{
    QPointF point = event->localPos();
    event->accept();

   if (event->buttons() == Qt::NoButton &&
       event->button() == Qt::RightButton) {

       setVisible(false);
       return;
   }

    if (m_isRotatingCanvasIndicator) {
        update();
    }

    m_isRotatingCanvasIndicator = false;

    if (event->button() == Qt::LeftButton) {
        //in favorite brushes area
        if (hoveredPreset() > -1) {
            //setSelectedBrush(hoveredBrush());
            emit sigChangeActivePaintop(hoveredPreset());
        }

        if (m_showColorHistory) {
            QPainterPath pathColor(drawDonutPathFull(m_popupPaletteSize / 2, m_popupPaletteSize / 2, m_colorHistoryInnerRadius, m_colorHistoryOuterRadius));
            if (pathColor.contains(point)) {
                int pos = calculateColorIndex(point, m_resourceManager->recentColorsTotal());

                if (pos >= 0 && pos < m_resourceManager->recentColorsTotal()) {
                    emit sigUpdateRecentColor(pos);
                }
            }
        }
    }
}

int KisPopupPalette::calculateColorIndex(QPointF position, int numColors) const
{
    if (numColors < 1) {
        return -1;
    }
    // relative to palette center
    QPointF relPosition = position - QPointF(0.5 * m_popupPaletteSize, 0.5 * m_popupPaletteSize);

    qreal angle = qAtan2(relPosition.x(), relPosition.y()) + M_PI/numColors;
    if (angle < 0) {
        angle += 2 * M_PI;
    }

    int index = floor(angle * numColors / (2 * M_PI));

    return qBound(0, index, numColors - 1);
}

bool KisPopupPalette::isPointInPixmap(QPointF &point, int pos)
{
    if (createPathFromPresetIndex(pos).contains(point + QPointF(-m_popupPaletteSize / 2, -m_popupPaletteSize / 2))) {
        return true;
    }
    return false;
}

KisPopupPalette::~KisPopupPalette()
{
}

void KisPopupPalette::calculatePresetLayout()
{
    qreal angleSlice = 360.0 / numSlots() ; // how many degrees each slice will get
    qreal outerRadius = m_popupPaletteSize/2 - m_presetRingMargin - (m_showRotationTrack ? m_rotationTrackSize + 1 /* half of stroke */ : BORDER_WIDTH);
    qreal innerRadius = m_colorHistoryOuterRadius +
    (m_showColorHistory
        ? 1 /* half of stroke */ + m_presetRingMargin
        : 0 /* preset margin is already included in either color history radius when it's not showing */
    );
    
    qreal ringWidth = outerRadius - innerRadius;
    qreal ringMidRadius = innerRadius + 0.5 * ringWidth;

    // the radius will get smaller as the amount of presets shown increases.
    // max radius until they touch = (innerRadius * sin(180/nSlots) / (1 - sin(180/nSlots)
    // 10 slots == 41.14365, at default inner radius of 92

    qreal presetRadius = qMin(0.5 * ringWidth, ringMidRadius * qSin(qDegreesToRadians(angleSlice/2)) - 1);
    m_presetRingCount = 1;

    // can we can fit in a second row? We don't want the preset icons to get too tiny.
    // staggered arrangement only makes sense if there is more than a tiny offset.
    if (ringWidth > presetRadius * 2.2) {
        //redo all calculations assuming a second row.
        angleSlice = 180.0/((numSlots()+1) / 2);
        qreal tempRadius = ringMidRadius * qSin(0.5 * qDegreesToRadians(angleSlice)) - 1;

        qreal distance = 0;
        do {
            tempRadius += 0.1;

            // Calculate the X,Y of two adjectant circles using this tempRadius. (Y1 == 0)
            qreal radius2 = innerRadius + ringWidth - tempRadius;
            qreal pathX1 = innerRadius + tempRadius;
            qreal pathX2 = radius2 * qCos(qDegreesToRadians(angleSlice));
            qreal pathY2 = radius2 * qSin(qDegreesToRadians(angleSlice));

            // Use Pythagorean Theorem to calculate the distance between these two points.
            qreal deltaX = pathX2-pathX1;
            distance = sqrt((deltaX*deltaX)+(pathY2*pathY2));
        }
        //As long at there's more distance than the radius of the two presets, continue increasing the radius.
        while ((tempRadius+1)*2 < distance);

        // even when presets between rings don't touch, the ones on the inner ring may overlap,
        // so determine the radius where inner ones touch
        qreal sinAngleSlice = qSin(qDegreesToRadians(angleSlice));
        qreal maxRadius = innerRadius * sinAngleSlice / (1 - sinAngleSlice);
        tempRadius = qMin(maxRadius, tempRadius);
        // since we may round up the preset count, radius may end up smaller than originally
        // but we want at least a small gain (3% currently)
        if (tempRadius > 1.03 * presetRadius) {
            m_presetRingCount = 2;
            presetRadius = tempRadius;
        }
    }
    // can we use even bigger icons by forming triplets with two on the same angle
    // and a third one touching both?
    if (ringWidth > presetRadius * 4) {
        angleSlice = 180.0 / ((numSlots() + 2) / 3);
        qreal sinAngleSlice = qSin(qDegreesToRadians(angleSlice));
        // radius where innermost circles would touch (2 angle slices apart)
        qreal maxRadius = innerRadius * sinAngleSlice / (1 - sinAngleSlice);
        // radius where innermost and outermost circles touch
        maxRadius = qMin(maxRadius, ringWidth * 0.25);

        qreal tempRadius = maxRadius;
        qreal pathX2 = ringMidRadius;
        qreal pathY2 = ringMidRadius * sinAngleSlice;
        bool found = false;
        // do we need to reduce radius even more?
        while (tempRadius > presetRadius) {
            qreal pathX1 = innerRadius + tempRadius;

            qreal deltaX = pathX2-pathX1;
            qreal distance = sqrt((deltaX*deltaX)+(pathY2*pathY2));

            if (distance > 2*(tempRadius+1)) {
                found = true;
                break;
            }
            tempRadius -= 0.1;
        }
        if (found) {
            //qDebug() << "triplet possible, radius:" << tempRadius << "/ staggered radius:" << presetRadius;
            m_presetRingCount = 3;
            m_middleRadius = sqrt(pathX2 * pathX2 + pathY2 * pathY2);
            presetRadius = tempRadius;
        }
    }
    m_cachedRadius = presetRadius;
}

QPainterPath KisPopupPalette::createPathFromPresetIndex(int index) const
{
    // how many degrees each slice will get
    // if the slot count is 1, we must divide by 2 to get 180 for the algorithm to work
    qreal angleSlice = 360.0 / qMax(m_presetSlotCount, 2);
    qreal outerRadius = m_popupPaletteSize/2 - m_presetRingMargin - (m_showRotationTrack ? m_rotationTrackSize + 1 /* half of stroke */  : BORDER_WIDTH);
    qreal innerRadius = m_colorHistoryOuterRadius +
    (m_showColorHistory
        ? 1 /* half of stroke */ + m_presetRingMargin
        : 0 /* preset margin is already included in either color history radius when it's not showing */
    );
    qreal ringWidth = outerRadius - innerRadius;
    // the starting angle of the slice we need to draw. the negative sign makes us go clockwise.
    // adding 90 degrees makes us start at the top. otherwise we would start at the right
    qreal startingAngle = -(index * angleSlice) + 90;
    qreal length = innerRadius + 0.5 * ringWidth;
    switch (m_presetRingCount) {
    case 1: break;
    case 2: {
        angleSlice = 180.0/((numSlots()+1) / 2);
        startingAngle = -(index * angleSlice) + 90;

        length = innerRadius + m_cachedRadius;
        if (index % 2) {
            length = innerRadius + ringWidth - m_cachedRadius;
        }
        break;
    }
    case 3: {
        int triplet = index / 3;
        angleSlice = 180.0 / ((numSlots() + 2) / 3);
        switch (index % 3) {
        case 0:
            startingAngle = -(triplet * 2 * angleSlice) + 90;
            length = innerRadius + m_cachedRadius;
            break;
        case 1:
            startingAngle = -(triplet * 2 * angleSlice) + 90;
            length = innerRadius + ringWidth - m_cachedRadius;
            break;
        case 2:
            startingAngle = -((triplet * 2 + 1) * angleSlice) + 90;
            length = m_middleRadius;
            break;
        default:
            KIS_ASSERT(false);
        }
        break;
    }
    default:
        KIS_ASSERT_RECOVER_NOOP(false);
    }
    QPainterPath path;
    qreal pathX = length * qCos(qDegreesToRadians(startingAngle)) - m_cachedRadius;
    qreal pathY = -(length) * qSin(qDegreesToRadians(startingAngle)) - m_cachedRadius;
    qreal pathDiameter = 2 * m_cachedRadius; // distance is used to calculate the X/Y in addition to the preset circle size
    path.addEllipse(pathX, pathY, pathDiameter, pathDiameter);
    return path;
}

int KisPopupPalette::findPresetSlot(QPointF position) const
{
    QPointF adujustedPoint = position - QPointF(m_popupPaletteSize/2, m_popupPaletteSize/2);
    for (int i = 0; i < numSlots(); i++) {
        if (createPathFromPresetIndex(i).contains(adujustedPoint)) {
            return i;
        }
    }
    return -1;
}

int KisPopupPalette::numSlots() const
{
    return m_presetSlotCount;
}
