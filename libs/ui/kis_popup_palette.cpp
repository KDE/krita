/* This file is part of the KDE project
   Copyright 2009 Vera Lukman <shicmap@gmail.com>
   Copyright 2011 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_canvas2.h"
#include "kis_config.h"
#include "kis_popup_palette.h"
#include "kis_favorite_resource_manager.h"
#include "kis_icon_utils.h"
#include "KisResourceServerProvider.h"
#include <kis_canvas_resource_provider.h>
#include <KoTriangleColorSelector.h>
#include <KisVisualColorSelector.h>
#include <kis_config_notifier.h>
#include <QtGui>
#include <QMenu>
#include <QWhatsThis>
#include <QVBoxLayout>
#include <QElapsedTimer>
#include "kis_signal_compressor.h"
#include "brushhud/kis_brush_hud.h"
#include "brushhud/kis_round_hud_button.h"
#include "kis_signals_blocker.h"
#include "kis_canvas_controller.h"
#include "kis_acyclic_signal_connector.h"


class PopupColorTriangle : public KoTriangleColorSelector
{
public:
    PopupColorTriangle(const KoColorDisplayRendererInterface *displayRenderer, QWidget* parent)
        : KoTriangleColorSelector(displayRenderer, parent)
        , m_dragging(false) {
    }

    ~PopupColorTriangle() override {}

    void tabletEvent(QTabletEvent* event) override {
        event->accept();
        QMouseEvent* mouseEvent = 0;
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
{
    // some UI controls are defined and created based off these variables

    const int borderWidth = 3;

    if (KisConfig(true).readEntry<bool>("popuppalette/usevisualcolorselector", false)) {
        m_triangleColorSelector = new KisVisualColorSelector(this);
    }
    else {
        m_triangleColorSelector  = new PopupColorTriangle(displayRenderer, this);
    }
    m_triangleColorSelector->setDisplayRenderer(displayRenderer);
    m_triangleColorSelector->setConfig(true,false);
    m_triangleColorSelector->move(m_popupPaletteSize/2-m_colorHistoryInnerRadius+borderWidth, m_popupPaletteSize/2-m_colorHistoryInnerRadius+borderWidth);
    m_triangleColorSelector->resize(m_colorHistoryInnerRadius*2-borderWidth*2, m_colorHistoryInnerRadius*2-borderWidth*2);
    m_triangleColorSelector->setVisible(true);
    KoColor fgcolor(Qt::black, KoColorSpaceRegistry::instance()->rgb8());
    if (m_resourceManager) {
        fgcolor = provider->fgColor();
    }
    m_triangleColorSelector->slotSetColor(fgcolor);

    QRegion maskedRegion(0, 0, m_triangleColorSelector->width(), m_triangleColorSelector->height(), QRegion::Ellipse );
    m_triangleColorSelector->setMask(maskedRegion);

    //setAttribute(Qt::WA_TranslucentBackground, true);

    connect(m_triangleColorSelector, SIGNAL(sigNewColor(KoColor)),
            m_colorChangeCompressor.data(), SLOT(start()));
    connect(m_colorChangeCompressor.data(), SIGNAL(timeout()),
            SLOT(slotEmitColorChanged()));

    connect(KisConfigNotifier::instance(), SIGNAL(configChanged()), m_triangleColorSelector, SLOT(configurationChanged()));

    m_acyclicConnector->connectForwardKoColor(m_resourceManager, SIGNAL(sigChangeFGColorSelector(KoColor)),
                                              this, SLOT(slotExternalFgColorChanged(KoColor)));

    m_acyclicConnector->connectBackwardKoColor(this, SIGNAL(sigChangefGColor(KoColor)),
                                               m_resourceManager, SIGNAL(sigSetFGColor(KoColor)));

    connect(this, SIGNAL(sigChangeActivePaintop(int)), m_resourceManager, SLOT(slotChangeActivePaintop(int)));
    connect(this, SIGNAL(sigUpdateRecentColor(int)), m_resourceManager, SLOT(slotUpdateRecentColor(int)));

    connect(m_resourceManager, SIGNAL(setSelectedColor(int)), SLOT(slotSetSelectedColor(int)));
    connect(m_resourceManager, SIGNAL(updatePalettes()), SLOT(slotUpdate()));
    connect(m_resourceManager, SIGNAL(hidePalettes()), SLOT(slotHide()));

    // This is used to handle a bug:
    // If pop up palette is visible and a new colour is selected, the new colour
    // will be added when the user clicks on the canvas to hide the palette
    // In general, we want to be able to store recent color if the pop up palette
    // is not visible
    m_timer.setSingleShot(true);
    connect(this, SIGNAL(sigTriggerTimer()), this, SLOT(slotTriggerTimer()));
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotEnableChangeFGColor()));
    connect(this, SIGNAL(sigEnableChangeFGColor(bool)), m_resourceManager, SIGNAL(sigEnableChangeColor(bool)));

    setCursor(Qt::ArrowCursor);
    setMouseTracking(true);
    setHoveredPreset(-1);
    setHoveredColor(-1);
    setSelectedColor(-1);

    m_brushHud = new KisBrushHud(provider, parent);
    m_brushHud->setFixedHeight(int(m_popupPaletteSize));
    m_brushHud->setVisible(false);

    const int auxButtonSize = 35;

    m_settingsButton = new KisRoundHudButton(this);

    m_settingsButton->setGeometry(m_popupPaletteSize - 2.2 * auxButtonSize, m_popupPaletteSize - auxButtonSize,
                                  auxButtonSize, auxButtonSize);

    connect(m_settingsButton, SIGNAL(clicked()), SLOT(slotShowTagsPopup()));

    KisConfig cfg(true);
    m_brushHudButton = new KisRoundHudButton(this);
    m_brushHudButton->setCheckable(true);

    m_brushHudButton->setGeometry(m_popupPaletteSize - 1.0 * auxButtonSize, m_popupPaletteSize - auxButtonSize,
                                  auxButtonSize, auxButtonSize);
    connect(m_brushHudButton, SIGNAL(toggled(bool)), SLOT(showHudWidget(bool)));
    m_brushHudButton->setChecked(cfg.showBrushHud());


    // add some stuff below the pop-up palette that will make it easier to use for tablet people
    QVBoxLayout* vLayout = new QVBoxLayout(this); // main layout

    QSpacerItem* verticalSpacer = new QSpacerItem(0, 0, QSizePolicy::Fixed, QSizePolicy::Expanding);
    vLayout->addSpacerItem(verticalSpacer); // this should push the box to the bottom


    QHBoxLayout* hLayout = new QHBoxLayout();

    vLayout->addLayout(hLayout);

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

    slotUpdateIcons();

    hLayout->addWidget(mirrorMode);
    hLayout->addWidget(canvasOnlyButton);
    hLayout->addWidget(zoomToOneHundredPercentButton);
    hLayout->addWidget(zoomCanvasSlider);

    setVisible(true);
    setVisible(false);

    opacityChange = new QGraphicsOpacityEffect(this);
    setGraphicsEffect(opacityChange);

    // Prevent tablet events from being captured by the canvas
    setAttribute(Qt::WA_NoMousePropagation, true);
}

void KisPopupPalette::slotExternalFgColorChanged(const KoColor &color)
{
    //hack to get around cmyk for now.
    if (color.colorSpace()->colorChannelCount()>3) {
        KoColor c(KoColorSpaceRegistry::instance()->rgb8());
        c.fromKoColor(color);
        m_triangleColorSelector->slotSetColor(c);
    } else {
        m_triangleColorSelector->slotSetColor(color);
    }
}

void KisPopupPalette::slotEmitColorChanged()
{
    if (isVisible()) {
        update();
        emit sigChangefGColor(m_triangleColorSelector->getCurrentColor());
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

void KisPopupPalette::slotTriggerTimer()
{
    m_timer.start(750);
}

void KisPopupPalette::slotEnableChangeFGColor()
{
    emit sigEnableChangeFGColor(true);
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

void KisPopupPalette::adjustLayout(const QPoint &p)
{
    KIS_ASSERT_RECOVER_RETURN(m_brushHud);
    if (isVisible() && parentWidget())  {

        float hudMargin = 30.0;
        const QRect fitRect = kisGrowRect(parentWidget()->rect(), -20.0); // -20 is widget margin
        const QPoint paletteCenterOffset(m_popupPaletteSize / 2, m_popupPaletteSize / 2);
        QRect paletteRect = rect();
        paletteRect.moveTo(p - paletteCenterOffset);
        if (m_brushHudButton->isChecked()) {
            m_brushHud->updateGeometry();
            paletteRect.adjust(0, 0, m_brushHud->width() + hudMargin, 0);
        }

        paletteRect = kisEnsureInRect(paletteRect, fitRect);
        move(paletteRect.topLeft());
        m_brushHud->move(paletteRect.topLeft() + QPoint(m_popupPaletteSize + hudMargin, 0));
        m_lastCenterPoint = p;
    }
}

void KisPopupPalette::slotUpdateIcons()
{
    zoomToOneHundredPercentButton->setIcon(m_actionCollection->action("zoom_to_100pct")->icon());
    m_brushHud->updateIcons();
    m_settingsButton->setIcon(KisIconUtils::loadIcon("tag"));
    m_brushHudButton->setOnOffIcons(KisIconUtils::loadIcon("arrow-left"), KisIconUtils::loadIcon("arrow-right"));
}

void KisPopupPalette::showHudWidget(bool visible)
{
    KIS_ASSERT_RECOVER_RETURN(m_brushHud);

    const bool reallyVisible = visible && m_brushHudButton->isChecked();

    if (reallyVisible) {
        m_brushHud->updateProperties();
    }

    m_brushHud->setVisible(reallyVisible);
    adjustLayout(m_lastCenterPoint);

    KisConfig cfg(false);
    cfg.setShowBrushHud(visible);
}

void KisPopupPalette::showPopupPalette(const QPoint &p)
{
    showPopupPalette(!isVisible());
    adjustLayout(p);
}

void KisPopupPalette::showPopupPalette(bool show)
{
    if (show) {
        m_hadMousePressSinceOpening = false;
        m_timeSinceOpening.start();


        // don't set the zoom slider if we are outside of the zoom slider bounds. It will change the zoom level to within
        // the bounds and cause the canvas to jump between the slider's min and max
        if (m_coordinatesConverter->zoomInPercent() > zoomSliderMinValue &&
                m_coordinatesConverter->zoomInPercent() < zoomSliderMaxValue  ){

            KisSignalsBlocker b(zoomCanvasSlider);
            zoomCanvasSlider->setValue(m_coordinatesConverter->zoomInPercent()); // sync the zoom slider
        }
        emit sigEnableChangeFGColor(!show);
    } else {
        emit sigTriggerTimer();
    }
    setVisible(show);
    m_brushHud->setVisible(show && m_brushHudButton->isChecked());
}

//redefinition of setVariable function to change the scope to private
void KisPopupPalette::setVisible(bool b)
{
    QWidget::setVisible(b);
}

void KisPopupPalette::setParent(QWidget *parent) {
    m_brushHud->setParent(parent);
    QWidget::setParent(parent);
}

QSize KisPopupPalette::sizeHint() const
{
    return QSize(m_popupPaletteSize, m_popupPaletteSize + 50); // last number is the space for the toolbar below
}

void KisPopupPalette::resizeEvent(QResizeEvent*)
{
}

void KisPopupPalette::paintEvent(QPaintEvent* e)
{
    Q_UNUSED(e);

    QPainter painter(this);

    QPen pen(palette().color(QPalette::Text));
    pen.setWidth(3);
    painter.setPen(pen);

    painter.setRenderHint(QPainter::Antialiasing);
    painter.setRenderHint(QPainter::SmoothPixmapTransform);

    // painting background color indicator
    QPainterPath bgColor;
    bgColor.addEllipse(QPoint( 50, 80), 30, 30);
    painter.fillPath(bgColor, m_displayRenderer->toQColor(m_resourceManager->bgColor()));
    painter.drawPath(bgColor);

    // painting foreground color indicator
    QPainterPath fgColor;
    fgColor.addEllipse(QPoint( 60, 50), 30, 30);
    painter.fillPath(fgColor, m_displayRenderer->toQColor(m_triangleColorSelector->getCurrentColor()));
    painter.drawPath(fgColor);

    // create a circle background that everything else will go into
    QPainterPath backgroundContainer;
    float shrinkCircleAmount = 3;// helps the circle when the stroke is put around it

    QRectF circleRect(shrinkCircleAmount, shrinkCircleAmount,
                      m_popupPaletteSize - shrinkCircleAmount*2,m_popupPaletteSize - shrinkCircleAmount*2);


    backgroundContainer.addEllipse( circleRect );
    painter.fillPath(backgroundContainer,palette().brush(QPalette::Background));

    painter.drawPath(backgroundContainer);

    // create a path slightly inside the container circle. this will create a 'track' to indicate that we can rotate the canvas
    // with the indicator
    QPainterPath rotationTrackPath;
    shrinkCircleAmount = 18;
    QRectF circleRect2(shrinkCircleAmount, shrinkCircleAmount,
                       m_popupPaletteSize - shrinkCircleAmount*2,m_popupPaletteSize - shrinkCircleAmount*2);

    rotationTrackPath.addEllipse( circleRect2 );
    pen.setWidth(1);
    painter.setPen(pen);
    painter.drawPath(rotationTrackPath);

    // this thing will help indicate where the starting brush preset is at.
    // also what direction they go to give sor order to the presets populated
    /*
    pen.setWidth(6);
    pen.setCapStyle(Qt::RoundCap);
    painter.setPen(pen);
    painter.drawArc(circleRect, (16*90), (16*-30)); // span angle (last parameter) is in 16th of degrees

    QPainterPath brushDir;
    brushDir.arcMoveTo(circleRect, 60);
    brushDir.lineTo(brushDir.currentPosition().x()-5, brushDir.currentPosition().y() - 14);
    painter.drawPath(brushDir);

    brushDir.lineTo(brushDir.currentPosition().x()-2, brushDir.currentPosition().y() + 6);
    painter.drawPath(brushDir);
    */

    // the following things needs to be based off the center, so let's translate the painter
    painter.translate(m_popupPaletteSize / 2, m_popupPaletteSize / 2);

    // create the canvas rotation handle
    QPainterPath rotationIndicator = drawRotationIndicator(m_coordinatesConverter->rotationAngle(), true);

    painter.fillPath(rotationIndicator,palette().brush(QPalette::Text));

    // hover indicator for the canvas rotation
    if (m_isOverCanvasRotationIndicator == true) {
        painter.save();

        QPen pen(palette().color(QPalette::Highlight));
        pen.setWidth(2);
        painter.setPen(pen);
        painter.drawPath(rotationIndicator);

        painter.restore();
    }

    // create a reset canvas rotation indicator to bring the canvas back to 0 degrees
    QPainterPath resetRotationIndicator = drawRotationIndicator(0, false);

    QPen resetPen(palette().color(QPalette::Text));
    resetPen.setWidth(1);
    painter.save();
    painter.setPen(resetPen);
    painter.drawPath(resetRotationIndicator);

    painter.restore();

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
            painter.drawImage(bounds.topLeft() , images.at(pos).scaled(bounds.size() , Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation));
        }
        else {
            painter.fillPath(presetPath, palette().brush(QPalette::Window));  // brush slot that has no brush in it
        }
        QPen pen = painter.pen();
        pen.setWidth(1);
        painter.setPen(pen);
        painter.drawPath(presetPath);

        painter.restore();
    }
    if (hoveredPreset() > -1) {
        presetPath = createPathFromPresetIndex(hoveredPreset());
        QPen pen(palette().color(QPalette::Highlight));
        pen.setWidth(3);
        painter.setPen(pen);
        painter.drawPath(presetPath);
    }

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


    // if we are actively rotating the canvas or zooming, make the panel slightly transparent to see the canvas better
    if(m_isRotatingCanvasIndicator || m_isZoomingCanvas) {
        opacityChange->setOpacity(0.4);
    } else {
        opacityChange->setOpacity(1.0);
    }

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

QPainterPath KisPopupPalette::drawRotationIndicator(qreal rotationAngle, bool canDrag)
{
    // used for canvas rotation. This function gets called twice. Once by the canvas rotation indicator,
    // and another time by the reset canvas position

    float canvasRotationRadians = qDegreesToRadians(rotationAngle - 90);  // -90 will make 0 degrees be at the top
    float rotationDialXPosition = qCos(canvasRotationRadians) * (m_popupPaletteSize/2 - 10); // m_popupPaletteSize/2  = radius
    float rotationDialYPosition = qSin(canvasRotationRadians) * (m_popupPaletteSize/2 - 10);

    QPainterPath canvasRotationIndicator;
    int canvasIndicatorSize = 15;
    float canvasIndicatorMiddle = canvasIndicatorSize/2;
    QRect indicatorRectangle = QRect( rotationDialXPosition - canvasIndicatorMiddle, rotationDialYPosition - canvasIndicatorMiddle,
                                      canvasIndicatorSize, canvasIndicatorSize );

    if (canDrag) {
        m_canvasRotationIndicatorRect = indicatorRectangle;
    } else {
        m_resetCanvasRotationIndicatorRect = indicatorRectangle;
    }

    canvasRotationIndicator.addEllipse(indicatorRectangle.x(), indicatorRectangle.y(),
                                       indicatorRectangle.width(), indicatorRectangle.height() );

    return canvasRotationIndicator;
}


void KisPopupPalette::mouseMoveEvent(QMouseEvent *event)
{
    QPointF point = event->localPos();
    event->accept();

    setToolTip(QString());
    setHoveredPreset(-1);
    setHoveredColor(-1);

    // calculate if we are over the canvas rotation knob
    // before we started painting, we moved the painter to the center of the widget, so the X/Y positions are offset. we need to
    // correct them first before looking for a click event intersection

    float rotationCorrectedXPos = m_canvasRotationIndicatorRect.x() + (m_popupPaletteSize / 2);
    float rotationCorrectedYPos = m_canvasRotationIndicatorRect.y() + (m_popupPaletteSize / 2);
    QRect correctedCanvasRotationIndicator = QRect(rotationCorrectedXPos, rotationCorrectedYPos,
                                                   m_canvasRotationIndicatorRect.width(), m_canvasRotationIndicatorRect.height());

    if (correctedCanvasRotationIndicator.contains(point.x(), point.y())) {
        m_isOverCanvasRotationIndicator = true;
    } else {
        m_isOverCanvasRotationIndicator = false;
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


        emit sigUpdateCanvas();
    }

    // don't highlight the presets if we are in the middle of rotating the canvas
    if (m_isRotatingCanvasIndicator == false) {
        QPainterPath pathColor(drawDonutPathFull(m_popupPaletteSize / 2, m_popupPaletteSize / 2, m_colorHistoryInnerRadius, m_colorHistoryOuterRadius));
        {
            int pos = calculatePresetIndex(point, m_resourceManager->numFavoritePresets());

            if (pos >= 0 && pos < m_resourceManager->numFavoritePresets()) {
                setToolTip(m_resourceManager->favoritePresetList().at(pos).data()->name());
                setHoveredPreset(pos);
            }
        }
        if (pathColor.contains(point)) {
            int pos = calculateIndex(point, m_resourceManager->recentColorsTotal());

            if (pos >= 0 && pos < m_resourceManager->recentColorsTotal()) {
                setHoveredColor(pos);
            }
        }
    }

    update();
}

void KisPopupPalette::mousePressEvent(QMouseEvent *event)
{
    QPointF point = event->localPos();
    event->accept();

    /**
     * Tablet support code generates a spurious right-click right after opening
     * the window, so we should ignore it. Next right-click will be used for
     * closing the popup palette
     */
    if (!m_hadMousePressSinceOpening && m_timeSinceOpening.elapsed() > 100) {
        m_hadMousePressSinceOpening = true;
    }

    if (event->button() == Qt::LeftButton) {

        //in favorite brushes area
        int pos = calculateIndex(point, m_resourceManager->numFavoritePresets());
        if (pos >= 0 && pos < m_resourceManager->numFavoritePresets()
                && isPointInPixmap(point, pos)) {
            //setSelectedBrush(pos);
            update();
        }

        if (m_isOverCanvasRotationIndicator) {
            m_isRotatingCanvasIndicator = true;
        }

        // reset the canvas if we are over the reset canvas rotation indicator
        float rotationCorrectedXPos = m_resetCanvasRotationIndicatorRect.x() + (m_popupPaletteSize / 2);
        float rotationCorrectedYPos = m_resetCanvasRotationIndicatorRect.y() + (m_popupPaletteSize / 2);
        QRect correctedResetCanvasRotationIndicator = QRect(rotationCorrectedXPos, rotationCorrectedYPos,
                                                            m_resetCanvasRotationIndicatorRect.width(), m_resetCanvasRotationIndicatorRect.height());

        if (correctedResetCanvasRotationIndicator.contains(point.x(), point.y())) {
            float angleDifference = -m_coordinatesConverter->rotationAngle(); // the rotation function accepts diffs
            KisCanvasController *canvasController =
                    dynamic_cast<KisCanvasController*>(m_viewManager->canvasBase()->canvasController());
            canvasController->rotateCanvas(angleDifference);

            emit sigUpdateCanvas();
        }
    }
}

void KisPopupPalette::slotShowTagsPopup()
{
    KisPaintOpPresetResourceServer *rServer = KisResourceServerProvider::instance()->paintOpPresetServer();
    QStringList tags = rServer->tagNamesList();
    std::sort(tags.begin(), tags.end());

    if (!tags.isEmpty()) {
        QMenu menu;
        Q_FOREACH (const QString& tag, tags) {
            menu.addAction(tag);
        }

        QAction *action = menu.exec(QCursor::pos());
        if (action) {
            m_resourceManager->setCurrentTag(action->text());
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

void KisPopupPalette::mouseReleaseEvent(QMouseEvent *event)
{
    QPointF point = event->localPos();
    event->accept();

    // see a comment in KisPopupPalette::mousePressEvent
    if (m_hadMousePressSinceOpening &&
        event->buttons() == Qt::NoButton &&
        event->button() == Qt::RightButton) {

        showPopupPalette(false);
        return;
    }

    m_isOverCanvasRotationIndicator = false;
    m_isRotatingCanvasIndicator = false;

    if (event->button() == Qt::LeftButton) {
        QPainterPath pathColor(drawDonutPathFull(m_popupPaletteSize / 2, m_popupPaletteSize / 2, m_colorHistoryInnerRadius, m_colorHistoryOuterRadius));

        //in favorite brushes area
        if (hoveredPreset() > -1) {
            //setSelectedBrush(hoveredBrush());
            emit sigChangeActivePaintop(hoveredPreset());
        }
        if (pathColor.contains(point)) {
            int pos = calculateIndex(point, m_resourceManager->recentColorsTotal());

            if (pos >= 0 && pos < m_resourceManager->recentColorsTotal()) {
                emit sigUpdateRecentColor(pos);
            }
        }
    }
}

int KisPopupPalette::calculateIndex(QPointF point, int n)
{
    calculatePresetIndex(point, n);
    //translate to (0,0)
    point.setX(point.x() - m_popupPaletteSize / 2);
    point.setY(point.y() - m_popupPaletteSize / 2);

    //rotate
    float smallerAngle = M_PI / 2 + M_PI / n - atan2(point.y(), point.x());
    float radius = sqrt((float)point.x() * point.x() + point.y() * point.y());
    point.setX(radius * cos(smallerAngle));
    point.setY(radius * sin(smallerAngle));

    //calculate brush index
    int pos = floor(acos(point.x() / radius) * n / (2 * M_PI));
    if (point.y() < 0) pos =  n - pos - 1;

    return pos;
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

QPainterPath KisPopupPalette::createPathFromPresetIndex(int index)
{
    qreal angleSlice = 360.0 / numSlots() ; // how many degrees each slice will get

    // the starting angle of the slice we need to draw. the negative sign makes us go clockwise.
    // adding 90 degrees makes us start at the top. otherwise we would start at the right
    qreal startingAngle = -(index * angleSlice) + 90;

    // the radius will get smaller as the amount of presets shown increases. 10 slots == 41
    qreal radians = qDegreesToRadians((360.0/10)/2);
    qreal maxRadius = (m_colorHistoryOuterRadius * qSin(radians) / (1-qSin(radians)))-2;

    radians = qDegreesToRadians(angleSlice/2);
    qreal presetRadius = m_colorHistoryOuterRadius * qSin(radians) / (1-qSin(radians));
    //If we assume that circles will mesh like a hexagonal grid, then 3.5r is the size of two hexagons interlocking.

    qreal length = m_colorHistoryOuterRadius + presetRadius;
    // can we can fit in a second row? We don't want the preset icons to get too tiny.
    if (maxRadius > presetRadius) {
        //redo all calculations assuming a second row.
        if (numSlots() % 2) {
            angleSlice = 360.0/(numSlots()+1);
            startingAngle = -(index * angleSlice) + 90;
        }
        if (numSlots() != m_cachedNumSlots){
            qreal tempRadius = presetRadius;
            qreal distance = 0;
            do{
                tempRadius+=0.1;

                // Calculate the XY of two adjectant circles using this tempRadius.
                qreal length1 = m_colorHistoryOuterRadius + tempRadius;
                qreal length2 = m_colorHistoryOuterRadius + ((maxRadius*2)-tempRadius);
                qreal pathX1 = length1 * qCos(qDegreesToRadians(startingAngle)) - tempRadius;
                qreal pathY1 = -(length1) * qSin(qDegreesToRadians(startingAngle)) - tempRadius;
                qreal startingAngle2 = -(index+1 * angleSlice) + 90;
                qreal pathX2 = length2 * qCos(qDegreesToRadians(startingAngle2)) - tempRadius;
                qreal pathY2 = -(length2) * qSin(qDegreesToRadians(startingAngle2)) - tempRadius;

                // Use Pythagorean Theorem to calculate the distance between these two values.
                qreal m1 = pathX2-pathX1;
                qreal m2 = pathY2-pathY1;

                distance = sqrt((m1*m1)+(m2*m2));
            }
            //As long at there's more distance than the radius of the two presets, continue increasing the radius.
            while((tempRadius+1)*2 < distance);
            m_cachedRadius = tempRadius;
        }
        m_cachedNumSlots = numSlots();
        presetRadius = m_cachedRadius;
        length = m_colorHistoryOuterRadius + presetRadius;
        if (index % 2) {
            length = m_colorHistoryOuterRadius + ((maxRadius*2)-presetRadius);
        }
    }
    QPainterPath path;
    qreal pathX = length * qCos(qDegreesToRadians(startingAngle)) - presetRadius;
    qreal pathY = -(length) * qSin(qDegreesToRadians(startingAngle)) - presetRadius;
    qreal pathDiameter = 2 * presetRadius; // distance is used to calculate the X/Y in addition to the preset circle size
    path.addEllipse(pathX, pathY, pathDiameter, pathDiameter);
    return path;
}

int KisPopupPalette::calculatePresetIndex(QPointF point, int /*n*/)
{
    for(int i = 0; i < numSlots(); i++)
    {
        QPointF adujustedPoint = point - QPointF(m_popupPaletteSize/2, m_popupPaletteSize/2);
        if(createPathFromPresetIndex(i).contains(adujustedPoint))
        {
            return i;
        }
    }
    return -1;
}

int KisPopupPalette::numSlots()
{
    KisConfig config(true);
    return qMax(config.favoritePresets(), 10);
}
