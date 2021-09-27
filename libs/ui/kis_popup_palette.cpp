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

static const int WIDGET_MARGIN = 16;
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
                    event->accept();
                    break;
                case QEvent::TabletMove:
                    mouseEvent = new QMouseEvent(QEvent::MouseMove, event->pos(),
                                                 (m_dragging) ? Qt::LeftButton : Qt::NoButton,
                                                 (m_dragging) ? Qt::LeftButton : Qt::NoButton, event->modifiers());
                    mouseMoveEvent(mouseEvent);
                    event->accept();
                    break;
                case QEvent::TabletRelease:
                    mouseEvent = new QMouseEvent(QEvent::MouseButtonRelease, event->pos(),
                                                 Qt::LeftButton,
                                                 Qt::LeftButton,
                                                 event->modifiers());
                    m_dragging = false;
                    mouseReleaseEvent(mouseEvent);
                    event->accept();
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

    connect(m_resourceManager, SIGNAL(setSelectedColor(int)), this, SLOT(slotSetSelectedColor(int)));
    connect(m_resourceManager, SIGNAL(updatePalettes()), this, SLOT(slotUpdate()));
    connect(m_resourceManager, SIGNAL(hidePalettes()), this, SIGNAL(finished()));

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

    m_clearColorHistoryButton = new KisRoundHudButton(this);
    m_clearColorHistoryButton->setToolTip(i18n("Clear color history"));

    connect(m_clearColorHistoryButton, SIGNAL(clicked(bool)), m_resourceManager, SLOT(slotClearHistory()));
    //Otherwise the colors won't disappear until the cursor moves away from the button:
    connect(m_clearColorHistoryButton, SIGNAL(released()), this, SLOT(slotUpdate()));

    // add some stuff below the pop-up palette that will make it easier to use for tablet people
    QGridLayout* gLayout = new QGridLayout(this);
    gLayout->setSizeConstraint(QLayout::SetFixedSize);
    gLayout->setSpacing(0);
    gLayout->setContentsMargins(QMargins());
    m_mainArea = new QSpacerItem(m_popupPaletteSize, m_popupPaletteSize);
    gLayout->addItem(m_mainArea, 0, 0); // this should push the box to the bottom
    gLayout->setColumnMinimumWidth(1, WIDGET_MARGIN);
    gLayout->addWidget(m_brushHud, 0, 2);
    gLayout->setRowMinimumHeight(1, WIDGET_MARGIN);
    gLayout->addWidget(m_bottomBarWidget, 2, 0);

    QHBoxLayout* hLayout = new QHBoxLayout(m_bottomBarWidget);
    
    mirrorMode = new KisHighlightedToolButton(this);
    mirrorMode->setFixedSize(35, 35);

    mirrorMode->setToolTip(i18n("Mirror Canvas"));
    mirrorMode->setDefaultAction(m_actionCollection->action("mirror_canvas"));
    connect(mirrorMode, SIGNAL(clicked(bool)), this, SLOT(slotUpdate()));

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
    
    slotUpdateIcons();

    hLayout->setSpacing(2);
    hLayout->setContentsMargins(0, 6, 0, 0);
    hLayout->addWidget(mirrorMode);
    hLayout->addWidget(canvasOnlyButton);
    hLayout->addWidget(zoomCanvasSlider);
    hLayout->addWidget(zoomToOneHundredPercentButton);
    
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

KisPopupPalette::~KisPopupPalette()
{
}

void KisPopupPalette::slotConfigurationChanged()
{
    reconfigure();
    layout()->invalidate();
}

void KisPopupPalette::reconfigure()
{
    KisConfig config(true);
    m_useDynamicSlotCount = config.readEntry("popuppalette/useDynamicSlotCount", true);
    m_maxPresetSlotCount = config.favoritePresets();
    if (m_useDynamicSlotCount) {
        int presetCount = m_resourceManager->numFavoritePresets();
        // if there are no presets because the tag is empty
        // show the maximum number allowed (they will be painted as empty slots)
        m_presetSlotCount = presetCount == 0
            ? m_maxPresetSlotCount
            : qMin(m_maxPresetSlotCount, m_resourceManager->numFavoritePresets());
    } else {
        m_presetSlotCount = m_maxPresetSlotCount;
    }
    m_popupPaletteSize = config.readEntry("popuppalette/size", 385);
    qreal selectorRadius = config.readEntry("popuppalette/selectorSize", 140) / 2;
    
    m_showColorHistory = config.readEntry("popuppalette/showColorHistory", true);
    m_showRotationTrack = config.readEntry("popuppalette/showRotationTrack", true);
    
    m_colorHistoryInnerRadius = selectorRadius + m_presetRingMargin;
    m_colorHistoryOuterRadius = m_colorHistoryInnerRadius;
    if (m_showColorHistory) {
         m_colorHistoryOuterRadius += 20;
         m_clearColorHistoryButton->setVisible(true);
    } else {
         m_clearColorHistoryButton->setVisible(false);
    }

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
            connect(m_colorSelector, SIGNAL(requestCloseContainer()), this, SIGNAL(finished()));
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
    // rectangle - to make sure the area doesn't extend outside of the widget
    QRegion maskedEllipse(-2, -2, m_colorSelector->width() + 4, m_colorSelector->height() + 4, QRegion::Ellipse );
    QRegion maskedRectangle(0, 0, m_colorSelector->width(), m_colorSelector->height(), QRegion::Rectangle);
    QRegion maskedRegion = maskedEllipse.intersected(maskedRectangle);

    m_colorSelector->setMask(maskedRegion);

    m_brushHud->setFixedHeight(int(m_popupPaletteSize));
    
    // arranges the buttons around the popup palette
    // buttons are spread out from the center of the set arc length
    
    // the margin in degrees between buttons
    qreal margin = 10.0; 
    // visual center
    qreal center = m_popupPaletteSize/2 - auxButtonSize/2;
    qreal length = m_popupPaletteSize/2 + auxButtonSize/2 + 5;
    {
        int buttonCount = 2;
        int arcLength = 90;
        // note the int buttonCount/2 is on purpose
        qreal start = arcLength/2 - (buttonCount/2) * margin;
        if (buttonCount % 2 == 0) start += margin / 2;
        int place = 0;
        m_brushHudButton->setGeometry(
            center + qCos(qDegreesToRadians(start + place*margin))*length,
            center + qSin(qDegreesToRadians(start + place*margin))*length,
            auxButtonSize, auxButtonSize
        );
        place++;
        m_bottomBarButton->setGeometry (
            center + qCos(qDegreesToRadians(start + place*margin))*length,
            center + qSin(qDegreesToRadians(start + place*margin))*length,
            auxButtonSize, auxButtonSize
        );
    }
    {
        int buttonCount = m_showColorHistory ? 2 : 1 ;
        int arcLength = 90;
        int shiftArc = 90;
        // note the int buttonCount/2 is on purpose
        qreal start = shiftArc + arcLength / 2 - (buttonCount/2) * margin;
        if (buttonCount % 2 == 0) start += margin / 2;
        int place = 0;
        if (m_showColorHistory) {
            m_clearColorHistoryButton->setGeometry(
                center + qCos(qDegreesToRadians(start + place * margin)) * length,
                center + qSin(qDegreesToRadians(start + place * margin)) * length,
                auxButtonSize, auxButtonSize);
            place++;
        }
        m_tagsButton->setGeometry(
            center + qCos(qDegreesToRadians(start + place*margin))*length,
            center + qSin(qDegreesToRadians(start + place*margin))*length,
            auxButtonSize, auxButtonSize
        );
    }
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

void KisPopupPalette::slotUpdate() {
    int presetCount = m_resourceManager->numFavoritePresets();
    if (m_useDynamicSlotCount && presetCount != m_presetSlotCount) {
         m_presetSlotCount = presetCount == 0
            ? m_maxPresetSlotCount
            : qMin(m_maxPresetSlotCount, presetCount);
        calculatePresetLayout();
    }
    m_canvasRotationIndicatorRect = rotationIndicatorRect(m_coordinatesConverter->rotationAngle());
    update();
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
    m_clearColorHistoryButton->setIcon(KisIconUtils::loadIcon("reload-preset-16"));
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

    if (m_isOverFgBgColors) {
        painter.save();
        painter.setPen(QPen(palette().color(QPalette::Highlight), BORDER_WIDTH));
    }
    
    // painting background color indicator
    QPainterPath bgColor(drawFgBgColorIndicator(0));
    painter.fillPath(bgColor, m_displayRenderer->toQColor(m_resourceManager->bgColor()));
    painter.drawPath(bgColor);

    // painting foreground color indicator
    QPainterPath fgColor(drawFgBgColorIndicator(1));
    painter.fillPath(fgColor, m_displayRenderer->toQColor(m_colorSelector->getCurrentColor()));
    painter.drawPath(fgColor);
    
    if (m_isOverFgBgColors) painter.restore();
    

    // create a circle background that everything else will go into
    QPainterPath backgroundContainer;

    // draws the circle halfway into the border so that the border never goes past the bounds of the popup
    QRectF circleRect(BORDER_WIDTH/2, BORDER_WIDTH/2, m_popupPaletteSize - BORDER_WIDTH, m_popupPaletteSize - BORDER_WIDTH);
    backgroundContainer.addEllipse(circleRect);
    painter.fillPath(backgroundContainer, palette().brush(QPalette::Background));
    painter.drawPath(backgroundContainer);

    if (m_showRotationTrack) {
        painter.save();
        QPen pen(palette().color(QPalette::Background).lighter(150), 2);
        painter.setPen(pen);

        // draw rotation snap lines
        if (m_isRotatingCanvasIndicator) {
            for (QLineF &line: m_snapLines) {
                painter.drawLine(line);
            }
        }
        // create a path slightly inside the container circle. this will create a 'track' to indicate that we can rotate the canvas
        // with the indicator
        QPainterPath rotationTrackPath;
        QRectF circleRect2(m_rotationTrackSize, m_rotationTrackSize,
                        m_popupPaletteSize - m_rotationTrackSize * 2, m_popupPaletteSize - m_rotationTrackSize * 2);

        rotationTrackPath.addEllipse(circleRect2);
        painter.drawPath(rotationTrackPath);
        
        // create a reset canvas rotation indicator to bring the canvas back to 0 degrees
        QRectF resetRotationIndicator = m_resetCanvasRotationIndicatorRect;

        pen.setColor(m_isOverResetCanvasRotationIndicator
            ? palette().color(QPalette::Highlight)
            : palette().color(QPalette::Text));
        // cover the first snap line
        painter.setBrush(palette().brush(QPalette::Background));
        painter.setPen(pen);
        painter.drawEllipse(resetRotationIndicator);

        // create the canvas rotation handle
        // highlight if either just hovering or currently rotating
        pen.setColor(m_isOverCanvasRotationIndicator || m_isRotatingCanvasIndicator
            ? palette().color(QPalette::Highlight)
            : palette().color(QPalette::Text));
        painter.setPen(pen);
        
        // fill with highlight if snapping
        painter.setBrush(m_isRotatingCanvasIndicator && m_snapRotation
            ? palette().brush(QPalette::Highlight)
            : palette().brush(QPalette::Text));
        
        painter.drawEllipse(m_canvasRotationIndicatorRect);

        painter.restore();
    }
    
    // the following things needs to be based off the center, so let's translate the painter
    painter.translate(m_popupPaletteSize / 2, m_popupPaletteSize / 2);

    // painting favorite brushes
    QList<QImage> images(m_resourceManager->favoritePresetImages());
    
    // painting favorite brushes pixmap/icon
    QPainterPath presetPath;
    int presetCount = images.size();
    bool isTagEmpty = presetCount == 0;
    for (int pos = 0; pos < m_presetSlotCount; pos++) {
        painter.save();
        presetPath = createPathFromPresetIndex(pos);

        if (pos < presetCount) {
            painter.setClipPath(presetPath);

            QRect bounds = presetPath.boundingRect().toAlignedRect();
            if (!images.at(pos).isNull()) {
                QImage previewHighDPI = images.at(pos).scaled(bounds.size()*devicePixelRatioF() , Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);
                previewHighDPI.setDevicePixelRatio(devicePixelRatioF());
                painter.drawImage(bounds.topLeft(), previewHighDPI);
            }
        } else {
            painter.fillPath(presetPath, palette().brush(QPalette::Window));  // brush slot that has no brush in it
        }
        // needs to be called here so that the clipping is removed
        painter.restore();
        // if the slot is empty, stroke it slightly darker
        QColor color = isTagEmpty || pos >= presetCount
            ? palette().color(QPalette::Background).lighter(150)
            : palette().color(QPalette::Text);
        painter.setPen(QPen(color, 1));
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
        qreal rotationAngle = -360.0 / m_resourceManager->recentColorsTotal();

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
    calculateRotationSnapAreas();
    m_resetCanvasRotationIndicatorRect = rotationIndicatorRect(0);
    m_canvasRotationIndicatorRect = rotationIndicatorRect(m_coordinatesConverter->rotationAngle());
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

QPainterPath KisPopupPalette::drawFgBgColorIndicator(int type) const
{
    QPointF edgePoint = QPointF(0.14645, 0.14645) * (m_popupPaletteSize);
    
    // the points are really (-5, 15) and (5, 15) shifted right 1px
    // this is so that where the circles meet the circle of the palette, the space occupied is exactly half to either side of the -45deg line
    QPainterPath indicator;
    switch (type) {
        case 0: { // background
            indicator.addEllipse(edgePoint + QPointF(-4, 15), 30, 30);
            break;
        }
        case 1: { //foreground
            indicator.addEllipse(edgePoint + QPointF(6, -15), 30, 30);
            break;
        }
    }
    return indicator;
}

QRectF KisPopupPalette::rotationIndicatorRect(qreal rotationAngle) const
{
    qreal paletteRadius = 0.5 * m_popupPaletteSize;
    QPointF rotationDialPosition(drawPointOnAngle(rotationAngle, paletteRadius - 10));
    rotationDialPosition += QPointF(paletteRadius, paletteRadius);
    
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
        bool wasOverResetRotationIndicator = m_isOverResetCanvasRotationIndicator;
        m_isOverResetCanvasRotationIndicator = m_resetCanvasRotationIndicatorRect.contains(point);

        if (
            wasOverRotationIndicator != m_isOverCanvasRotationIndicator ||
            wasOverResetRotationIndicator != m_isOverResetCanvasRotationIndicator
        ) {
            update();
        }

        if (m_isRotatingCanvasIndicator) {
            m_snapRotation = false;
            int i = 0;
            for (QRect &rect: m_snapRects) {
                QPainterPath circle;
                circle.addEllipse(rect);
                if (circle.contains(point)) {
                    m_snapRotation = true;
                    m_rotationSnapAngle = i * 15;
                    break;
                }
                i++;
            }
            qreal finalAngle = 0.0;
            if (m_snapRotation) {
                finalAngle = m_rotationSnapAngle;
                // to match the numbers displayed when rotating without snapping
                if (finalAngle >= 270) {
                    finalAngle = finalAngle - 360;
                }
            } else {
                // we are rotating the canvas, so calculate the rotation angle based off the center
                // calculate the angle we are at first
                QPoint widgetCenterPoint = QPoint(m_popupPaletteSize/2, m_popupPaletteSize/2);

                qreal dX = point.x() - widgetCenterPoint.x();
                qreal dY = point.y() - widgetCenterPoint.y();


                finalAngle = qAtan2(dY,dX) * 180 / M_PI; // what we need if we have two points, but don't know the angle
                finalAngle = finalAngle + 90; // add 90 degrees so 0 degree position points up
            }
            qreal angleDifference = finalAngle - m_coordinatesConverter->rotationAngle(); // the rotation function accepts diffs, so find it out

            KisCanvasController *canvasController =
                dynamic_cast<KisCanvasController*>(m_viewManager->canvasBase()->canvasController());
            canvasController->rotateCanvas(angleDifference);
            m_canvasRotationIndicatorRect = rotationIndicatorRect(finalAngle);

            update();
            emit sigUpdateCanvas();
        }
    }

    if (m_isRotatingCanvasIndicator == false) {
        QPainterPath bgColor(drawFgBgColorIndicator(0));
        QPainterPath fgColor(drawFgBgColorIndicator(1));
        QPainterPath backgroundContainer;
        QRectF circleRect(BORDER_WIDTH / 2, BORDER_WIDTH / 2, m_popupPaletteSize - BORDER_WIDTH, m_popupPaletteSize - BORDER_WIDTH);
        backgroundContainer.addEllipse(circleRect);
        
        QPainterPath fgBgColors = (fgColor + bgColor) - backgroundContainer;
        
        if (fgBgColors.contains(point)) {
            if (!m_isOverFgBgColors) {
                m_isOverFgBgColors = true;
                setToolTip(i18n("Click to swap foreground and background colors.\nRight click to set to black and white."));
                update();
            }
        } else {
            if (m_isOverFgBgColors) {
                m_isOverFgBgColors = false;
                setToolTip(QString());
                update();
            }
        }

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
    event->accept();

    if (event->button() == Qt::LeftButton) {
        if (m_showRotationTrack) {
            if (m_isOverCanvasRotationIndicator) {
                m_isRotatingCanvasIndicator = true;
                update();
            }

            if (m_isOverResetCanvasRotationIndicator) {
                qreal angleDifference = -m_coordinatesConverter->rotationAngle(); // the rotation function accepts diffs
                KisCanvasController *canvasController =
                        dynamic_cast<KisCanvasController*>(m_viewManager->canvasBase()->canvasController());
                canvasController->rotateCanvas(angleDifference);
                m_canvasRotationIndicatorRect = rotationIndicatorRect(0);

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
                    reconfigure();
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

void KisPopupPalette::popup(const QPoint &position) {
    setVisible(true);
    ensureWithinParent(position, false);
}

void KisPopupPalette::dismiss()
{
    setVisible(false);
}

bool KisPopupPalette::onScreen()
{
    return isVisible();
}

void KisPopupPalette::ensureWithinParent(const QPoint& position, bool useUpperLeft) {
    if (isVisible() && parentWidget())  {
        const qreal widgetMargin = -20.0;
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

void KisPopupPalette::tabletEvent(QTabletEvent *event)
{
    if (event->button() == Qt::RightButton && event->type() == QEvent::TabletPress) {
        m_tabletRightClickPressed = true;
    }

    event->ignore();
}

void KisPopupPalette::mouseReleaseEvent(QMouseEvent *event)
{
    QPointF point = event->localPos();
    event->accept();

    if (m_isRotatingCanvasIndicator) {
        update();
    }

    m_isRotatingCanvasIndicator = false;

    if (event->button() == Qt::LeftButton) {
        if (m_isOverFgBgColors) {
            m_viewManager->slotToggleFgBg();
        }
        
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
    } else if (event->button() == Qt::RightButton) {
        emit finished();
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

QPointF KisPopupPalette::drawPointOnAngle(qreal angle, qreal radius) const
{
    QPointF p(
        // -90 so it starts at the top since this is mainly used by calculatePresetLayout
        radius * qCos(qDegreesToRadians(angle - 90)),
        radius * qSin(qDegreesToRadians(angle - 90))
    );
    return p;
}

void KisPopupPalette::calculatePresetLayout()
{
    if (m_presetSlotCount == 0) {
        m_cachedPresetLayout = {};
        return;
    }
    // how many degrees each slice will get
    // if the slot count is 1, we must divide by 2 to get 180 for the algorithm to work
    qreal angleSlice = 360.0 / qMax(m_presetSlotCount, 2);
    qreal outerRadius = m_popupPaletteSize/2 - m_presetRingMargin - (m_showRotationTrack ? m_rotationTrackSize + 1 /* half of stroke */ : BORDER_WIDTH);
    qreal innerRadius = m_colorHistoryOuterRadius +
    (m_showColorHistory
        ? 1 /* half of stroke */ + m_presetRingMargin
        : 0 /* preset margin is already included in either color history radius when it's not showing */
    );
    
    qreal ringWidth = outerRadius - innerRadius;
    qreal halfRingWidth = 0.5 * ringWidth;
    qreal ringMidRadius = innerRadius + halfRingWidth;

    // reset the cached layout
    m_cachedPresetLayout = {};
    CachedPresetLayout& c = m_cachedPresetLayout;

    // note: adding the margin the way it's done
    // (calculating the radiuses without taking it into account then subtracting it after)
    // is not particularly accurate, but it looks fine since it's so small
    int margin = 2;

    // assume one row and get the max radius until the circles would touch
    qreal oneRowAngleSlice = angleSlice / 2;
    qreal oneRowMaxRadius = ringMidRadius * qSin(qDegreesToRadians(oneRowAngleSlice));

    // if the circles are bigger than the ring we're limited by the
    // ring's width instead and only one row would fit
    
    // oneRowMaxRadius * 0.2 is to make sure that we still do one row if
    // there isn't that much of a difference and two would just look weirder
    if (oneRowMaxRadius - margin > halfRingWidth - oneRowMaxRadius * 0.2) {
        c.ringCount = 1;
        c.firstRowRadius = qMin(halfRingWidth, oneRowMaxRadius - margin);
        c.firstRowPos = ringMidRadius;
        return;
    }
    
    // otherwise 2 or 3 rows always fit
    qreal tempRadius = halfRingWidth;
    {
        // for two rows, the first row is tangent to the inner radius
        // and the second row to the outer one
        qreal twoRowInnerCount = ceil(qMax(m_presetSlotCount, 2) / 2.0);
        qreal twoRowAngleSlice = 360.0 / twoRowInnerCount;

        // we can start at half the ring width and shrink the radius until nothing is overlapping
        while (tempRadius >= 0) {
            tempRadius -= 0.2;
            QPointF r1p1(drawPointOnAngle(twoRowAngleSlice / 2, innerRadius + tempRadius));
            QPointF r1p2(drawPointOnAngle(twoRowAngleSlice / 2 * 3, innerRadius + tempRadius));
            QPointF r2p(drawPointOnAngle(twoRowAngleSlice, outerRadius - tempRadius));
            qreal row1SiblingDistance = kisDistance(r1p1, r1p2);
            qreal row1To2Distance = kisDistance(r1p1, r2p);
            if (row1To2Distance >= (tempRadius + margin) * 2) {
                // the previous radius is the one that's guaranteed not to be overlapping
                if (row1SiblingDistance < (tempRadius + margin) * 2) {
                    // the inner row still overlaps, attempt 3 rows instead
                    break;
                }
                c.ringCount = 2;
                c.secondRowRadius = tempRadius;
                c.secondRowPos = outerRadius - tempRadius;
                c.firstRowRadius = tempRadius;
                c.firstRowPos = innerRadius + tempRadius;
                return;
            }
        }
    }

    // for three rows, we initially arrange them like so:
    // the first row tangent to the inner radius
    // the second row in the middle
    // the third row tangent to the outer radius

    qreal threeRowInnerCount = ceil(qMax(m_presetSlotCount, 2) / 3.0);
    qreal threeRowAngleSlice = 360.0 / threeRowInnerCount;

    // then we decrease the radius until no row is overlapping eachother or itself
    while (tempRadius >= 0) {
        QPointF r1p1(drawPointOnAngle(threeRowAngleSlice / 2, innerRadius + tempRadius));
        QPointF r1p2(drawPointOnAngle(threeRowAngleSlice / 2 * 3, innerRadius + tempRadius));
        QPointF r2p1(drawPointOnAngle(threeRowAngleSlice, ringMidRadius));
        QPointF r2p2(drawPointOnAngle(threeRowAngleSlice * 2, ringMidRadius));
        QPointF r3p(drawPointOnAngle(threeRowAngleSlice / 2, outerRadius - tempRadius));

        qreal row1SiblingDistance = kisDistance(r1p1, r1p2);
        qreal row1to2Distance = kisDistance(r1p1, r2p1);
        qreal row2to3Distance = kisDistance(r2p1, r3p);
        qreal row1to3Distance = kisDistance(r1p1, r3p);

        if (
            row1to2Distance >= tempRadius * 2 &&
            row2to3Distance >= tempRadius * 2 &&
            row1to3Distance >= tempRadius * 2 &&
            row1SiblingDistance >= tempRadius * 2
        ) {

            qreal row2SiblingDistance = kisDistance(r2p1, r2p2);

            qreal firstRowRadius = tempRadius;
            qreal thirdRowRadius = tempRadius;
            qreal secondRowRadius = tempRadius;

            bool firstRowTouching = row1SiblingDistance - firstRowRadius * 2 < 1;
            if (firstRowTouching) {
                // attempt to expand the second row
                // and expand + move the third row inwards
                QPointF tempR3p = r3p;
                qreal tempSecondThirdRowRadius = secondRowRadius;
                qreal tempRow2to3Distance = row2to3Distance;
                qreal tempRow1to3Distance = row1to3Distance;
                while (
                    tempSecondThirdRowRadius * 2 < tempRow2to3Distance &&
                    tempSecondThirdRowRadius * 2 < row2SiblingDistance &&
                    tempSecondThirdRowRadius * 2 < tempRow1to3Distance &&
                    tempSecondThirdRowRadius + firstRowRadius < row1to2Distance
                ) {
                    // the previous temp variables are within limits
                    r3p = tempR3p;
                    row2to3Distance = tempRow2to3Distance;
                    row1to3Distance = tempRow1to3Distance;
                    secondRowRadius = tempSecondThirdRowRadius;

                    tempSecondThirdRowRadius += 1;

                    tempR3p = drawPointOnAngle(threeRowAngleSlice / 2, outerRadius - tempSecondThirdRowRadius);

                    tempRow2to3Distance = kisDistance(r2p1, tempR3p);
                    tempRow1to3Distance = kisDistance(r1p1, tempR3p);
                }
                thirdRowRadius = secondRowRadius;
            }

            {
                // the third row can sometimes be expanded + moved a bit more
                qreal tempThirdRowRadius = thirdRowRadius;
                QPointF tempR3p = r3p;
                qreal tempRow2to3Distance = row2to3Distance;
                qreal tempRow1to3Distance = row1to3Distance;
                while (
                    tempThirdRowRadius < halfRingWidth &&
                    secondRowRadius + tempThirdRowRadius < tempRow2to3Distance &&
                    firstRowRadius + tempThirdRowRadius < tempRow1to3Distance
                ) {
                    r3p = tempR3p;
                    row2to3Distance = tempRow2to3Distance;
                    row1to3Distance = tempRow1to3Distance;
                    thirdRowRadius = tempThirdRowRadius;

                    tempThirdRowRadius += 1;

                    tempR3p = drawPointOnAngle(threeRowAngleSlice / 2, outerRadius - tempThirdRowRadius);
                    tempRow2to3Distance = kisDistance(r2p1, tempR3p);
                    tempRow1to3Distance = kisDistance(r1p1, tempR3p);
                }
            }
            // the third row is no longer moved
            qreal thirdRowPos = outerRadius - thirdRowRadius;

            // many times, e.g. when the second row is touching
            // the first row can be moved outwards and expanded
            // sometimes it will even detach from the inner radius if the ringwidth is large enough
            // and there's a lot of presets
            qreal firstRowPos = innerRadius + tempRadius;
            {
                qreal tempFirstRowPos = firstRowPos;
                qreal tempFirstRowRadius = firstRowRadius;
                qreal tempRow1SiblingDistance = row1SiblingDistance;
                qreal tempRow1to3Distance = row1to3Distance;
                qreal tempRow1to2Distance = row1to2Distance;
                QPointF tempR1p1 = r1p1;
                QPointF tempR1p2 = r1p2;

                while (
                    tempFirstRowPos < ringMidRadius &&
                    tempFirstRowRadius + secondRowRadius < tempRow1to2Distance &&
                    tempFirstRowRadius + thirdRowRadius < tempRow1to3Distance
                ) {
                    firstRowPos = tempFirstRowPos;
                    firstRowRadius = tempFirstRowRadius;
                    row1to2Distance = tempRow1to2Distance;
                    r1p1 = tempR1p1;
                    // these are unused after so it's not neccesary to update them
                    // row1to3Distance = tempRow1to3Distance;
                    // row1SiblingDistance = tempRow1SiblingDistance;
                    // r1p2 = tempR1p2;

                    tempFirstRowPos += 1;

                    tempR1p1 = drawPointOnAngle(threeRowAngleSlice / 2, tempFirstRowPos);
                    tempR1p2 = drawPointOnAngle(threeRowAngleSlice / 2 * 3, tempFirstRowPos);
                    tempRow1SiblingDistance = kisDistance(tempR1p1, tempR1p2);
                    // expand it to the max size
                    tempFirstRowRadius = tempRow1SiblingDistance / 2;
                    tempRow1to2Distance = kisDistance(tempR1p2, r2p1);
                    tempRow1to3Distance = kisDistance(tempR1p1, r3p);
                }
            }

            // finally it's rare, but sometimes possible to also move + expand the second row
            qreal secondRowPos = ringMidRadius;
            bool row2touching1 = row1to2Distance - (firstRowRadius + secondRowRadius) < 1;
            bool row2touching3 = row2to3Distance - (thirdRowRadius + secondRowRadius) < 1;
            if (!row2touching1 && !row2touching3) {
                // move the second row in until it's touching the first row
                qreal knownAngleRatio = qSin(qDegreesToRadians(threeRowAngleSlice / 2)) /
                                        (firstRowRadius + secondRowRadius);
                qreal angleRow1Row2Center = qAsin(knownAngleRatio * firstRowPos);
                qreal angleCenterRow2Row1 = 180 - threeRowAngleSlice / 2 - qRadiansToDegrees(angleRow1Row2Center);
                secondRowPos = qSin(qDegreesToRadians(angleCenterRow2Row1)) / knownAngleRatio;
            }
            if (!row2touching3) {
                QPointF tempR2p1 = r2p1;
                qreal tempRadius = secondRowRadius;
                qreal tempRow1to2Distance = row1to2Distance;
                qreal tempRow2to3Distance = row2to3Distance;
                qreal tempSecondRowPos = secondRowPos;
                while (
                    tempSecondRowPos < thirdRowPos &&
                    tempRadius + thirdRowRadius < tempRow2to3Distance &&
                    // this is an artificial limit, it could get bigger but looks weird
                    tempRadius < thirdRowRadius
                ) {
                    secondRowRadius = tempRadius;
                    secondRowPos = tempSecondRowPos;
                    // these are unused after so it's not neccesary to update them
                    // r2p1 = tempR2p1;
                    // row1to2Distance = tempRow1to2Distance;
                    // row2to3Distance = tempRow2to3Distance;

                    tempSecondRowPos += 1;

                    tempR2p1 = drawPointOnAngle(threeRowAngleSlice, secondRowPos + 1);
                    tempRow1to2Distance = kisDistance(tempR2p1, r1p1);
                    tempRow2to3Distance = kisDistance(tempR2p1, r3p);
                    tempRadius = tempRow1to2Distance - firstRowRadius;
                }
            }
            c = {
                3, //ringCount
                firstRowRadius - margin,
                secondRowRadius - margin,
                thirdRowRadius - margin,
                firstRowPos,
                secondRowPos,
                thirdRowPos
            };
            return;
        }
        tempRadius -= 0.2;
    }
}

QPainterPath KisPopupPalette::createPathFromPresetIndex(int index) const
{
    // how many degrees each slice will get
    // if the slot count is 1, we must divide by 2 to get 180 for the algorithm to work
    qreal angleSlice = 360.0 / qMax(m_presetSlotCount, 2);
    // the starting angle of the slice we need to draw. the negative sign makes us go clockwise.
    // adding 90 degrees makes us start at the top. otherwise we would start at the right
    qreal startingAngle = -(index * angleSlice) + 90;
    qreal length = m_cachedPresetLayout.firstRowPos;
    qreal radius = m_cachedPresetLayout.firstRowRadius;
    switch (m_cachedPresetLayout.ringCount) {
    case 1: break;
    case 2: {
        angleSlice = 180.0/((m_presetSlotCount+1) / 2);
        startingAngle = -(index * angleSlice) + 90;

        if (index % 2) {
            length = m_cachedPresetLayout.secondRowPos;
            radius = m_cachedPresetLayout.secondRowRadius;
        }
        break;
    }
    case 3: {
        int triplet = index / 3;
        angleSlice = 180.0 / ((m_presetSlotCount + 2) / 3);
        switch (index % 3) {
        case 0:
            startingAngle = -(triplet * 2 * angleSlice) + 90;
            length = m_cachedPresetLayout.firstRowPos;
            radius = m_cachedPresetLayout.firstRowRadius;
            break;
        case 1:
            startingAngle = -(triplet * 2 * angleSlice) + 90;
            length = m_cachedPresetLayout.thirdRowPos;
            radius = m_cachedPresetLayout.thirdRowRadius;
            break;
        case 2:
            startingAngle = -((triplet * 2 + 1) * angleSlice) + 90;
            length = m_cachedPresetLayout.secondRowPos;
            radius = m_cachedPresetLayout.secondRowRadius;
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
    qreal pathX = length * qCos(qDegreesToRadians(startingAngle)) - radius;
    qreal pathY = -(length) * qSin(qDegreesToRadians(startingAngle)) - radius;
    qreal pathDiameter = 2 * radius; // distance is used to calculate the X/Y in addition to the preset circle size
    path.addEllipse(pathX, pathY, pathDiameter, pathDiameter);
    return path;
}

void KisPopupPalette::calculateRotationSnapAreas() {
    int i = 0;
    for (QRect &rect: m_snapRects) {
        QPointF point(drawPointOnAngle(i * 15, m_popupPaletteSize / 2 - BORDER_WIDTH - m_snapRadius/2));
        point += QPointF(m_popupPaletteSize / 2 - m_snapRadius, m_popupPaletteSize / 2 - m_snapRadius);
        rect = QRect(point.x(), point.y(), m_snapRadius*2, m_snapRadius*2);
        i++;
    }
    i = 0;
    for (QLineF &line: m_snapLines) {
        qreal penWidth = BORDER_WIDTH / 2;
        QPointF point1 = drawPointOnAngle(i * 15, m_popupPaletteSize / 2 - m_rotationTrackSize + penWidth);
        point1 += QPointF(m_popupPaletteSize / 2, m_popupPaletteSize / 2);
        QPointF point2 = drawPointOnAngle(i * 15, m_popupPaletteSize / 2 - BORDER_WIDTH - penWidth);
        point2 += QPointF(m_popupPaletteSize / 2, m_popupPaletteSize / 2);
        line = QLineF(point1, point2);
        i++;
    }
}

int KisPopupPalette::findPresetSlot(QPointF position) const
{
    QPointF adujustedPoint = position - QPointF(m_popupPaletteSize/2, m_popupPaletteSize/2);
    for (int i = 0; i < m_presetSlotCount; i++) {
        if (createPathFromPresetIndex(i).contains(adujustedPoint)) {
            return i;
        }
    }
    return -1;
}
