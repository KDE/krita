/*
 *  SPDX-FileCopyrightText: 2004 Cyrille Berger <cberger@cberger.net>
 *                2016 Sven Langkamp <sven.langkamp@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */

#include "kis_stopgradient_editor.h"
#include <QPainter>
#include <QSpinBox>
#include <QDoubleSpinBox>
#include <QPoint>
#include <QMenu>

#include <KoColorSpace.h>
#include <resources/KoStopGradient.h>

#include "kis_debug.h"

#include <kis_icon_utils.h>

#include <KoCanvasResourcesIds.h>
#include <KoCanvasResourcesInterface.h>

/****************************** KisStopGradientEditor ******************************/

KisStopGradientEditor::KisStopGradientEditor(QWidget *parent)
    : QWidget(parent),
      m_gradient(0)
{
    setupUi(this);

    connect(gradientSlider, SIGNAL(sigSelectedStop(int)), this, SLOT(stopChanged(int)));
    connect(nameedit, SIGNAL(editingFinished()), this, SLOT(nameChanged()));
    connect(colorButton, SIGNAL(changed(KoColor)), SLOT(colorChanged(KoColor)));


    connect(colorRadioButton, SIGNAL(toggled(bool)), this, SLOT(stopTypeChanged()));
    connect(foregroundRadioButton, SIGNAL(toggled(bool)), this, SLOT(stopTypeChanged()));
    connect(backgroundRadioButton, SIGNAL(toggled(bool)), this, SLOT(stopTypeChanged()));

    opacitySlider->setPrefix(i18n("Opacity: "));
    opacitySlider->setRange(0.0, 1.0, 2);
    connect(opacitySlider, SIGNAL(valueChanged(qreal)), this, SLOT(opacityChanged(qreal)));


    buttonReverse->setIcon(KisIconUtils::loadIcon("transform_icons_mirror_x"));
    buttonReverse->setToolTip(i18n("Flip Gradient"));
    KisIconUtils::updateIcon(buttonReverse);
    connect(buttonReverse, SIGNAL(pressed()), SLOT(reverse()));

    buttonReverseSecond->setIcon(KisIconUtils::loadIcon("transform_icons_mirror_x"));
    buttonReverseSecond->setToolTip(i18n("Flip Gradient"));
    KisIconUtils::updateIcon(buttonReverseSecond);
    connect(buttonReverseSecond, SIGNAL(clicked()), SLOT(reverse()));

    this->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, SIGNAL(customContextMenuRequested(const QPoint &)),
            this, SLOT(showContextMenu(const QPoint &)));

    setCompactMode(false);

    setGradient(0);
    stopChanged(-1);
}

KisStopGradientEditor::KisStopGradientEditor(KoStopGradientSP gradient, QWidget *parent, const char* name, const QString& caption,
      KoCanvasResourcesInterfaceSP canvasResourcesInterface)
    : KisStopGradientEditor(parent)
{
    m_canvasResourcesInterface = canvasResourcesInterface;
    setObjectName(name);
    setWindowTitle(caption);
    setGradient(gradient);
}

void KisStopGradientEditor::setCompactMode(bool value)
{
    lblName->setVisible(!value);
    buttonReverse->setVisible(!value);
    nameedit->setVisible(!value);
    foregroundRadioButton->setVisible(!value);
    backgroundRadioButton->setVisible(!value);
    colorRadioButton->setVisible(!value);

    buttonReverseSecond->setVisible(value);
}

void KisStopGradientEditor::setGradient(KoStopGradientSP gradient)
{
    m_gradient = gradient;
    setEnabled(m_gradient);

    if (m_gradient) {
        gradientSlider->setGradientResource(m_gradient);
        nameedit->setText(gradient->name());
        stopChanged(gradientSlider->selectedStop());
    }

    emit sigGradientChanged();
}

void KisStopGradientEditor::setCanvasResourcesInterface(KoCanvasResourcesInterfaceSP canvasResourcesInterface)
{
    m_canvasResourcesInterface = canvasResourcesInterface;
}

KoCanvasResourcesInterfaceSP KisStopGradientEditor::canvasResourcesInterface() const
{
    return m_canvasResourcesInterface;
}

void KisStopGradientEditor::notifyGlobalColorChanged(const KoColor &color)
{
    if (colorButton->isEnabled() &&
        color != colorButton->color()) {

        colorButton->setColor(color);
    }
}

boost::optional<KoColor> KisStopGradientEditor::currentActiveStopColor() const
{
    if (!colorButton->isEnabled()) return boost::none;
    return colorButton->color();
}

void KisStopGradientEditor::stopChanged(int stop)
{
    if (!m_gradient) return;

    const bool hasStopSelected = stop >= 0;

    opacitySlider->setEnabled(hasStopSelected);
    colorButton->setEnabled(hasStopSelected);
    stopLabel->setEnabled(hasStopSelected);
    foregroundRadioButton->setEnabled(hasStopSelected);
    backgroundRadioButton->setEnabled(hasStopSelected);
    colorRadioButton->setEnabled(hasStopSelected);

    if (hasStopSelected) {
        KoColor color;
        KoGradientStopType type = m_gradient->stops()[stop].type;
        if (type == FOREGROUNDSTOP) {
            foregroundRadioButton->setChecked(true);
            opacitySlider->setEnabled(false);
            if (m_canvasResourcesInterface) {
                color = m_canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
            } else {
                color = m_gradient->stops()[stop].color;
            }
        }
        else if (type == BACKGROUNDSTOP) {
            backgroundRadioButton->setChecked(true);
            opacitySlider->setEnabled(false);
            if (m_canvasResourcesInterface) {
                color = m_canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>();
            } else {
                color = m_gradient->stops()[stop].color;
            }
        }
        else {
            colorRadioButton->setChecked(true);
            opacitySlider->setEnabled(true);
            color = m_gradient->stops()[stop].color;
        }

        opacitySlider->setValue(color.opacityF());

        color.setOpacity(1.0);
        colorButton->setColor(color);

    }

    emit sigGradientChanged();
}

void KisStopGradientEditor::stopTypeChanged() {
    QList<KoGradientStop> stops = m_gradient->stops();
    int currentStop = gradientSlider->selectedStop();
    double t = stops[currentStop].position;
    KoColor color = stops[currentStop].color;

    KoGradientStopType type;    
    if (foregroundRadioButton->isChecked()) {
        type = FOREGROUNDSTOP;
        if (m_canvasResourcesInterface) {
            color = m_canvasResourcesInterface->resource(KoCanvasResource::ForegroundColor).value<KoColor>();
        }
        opacitySlider->setEnabled(false);
    } else if (backgroundRadioButton->isChecked()) {
        type = BACKGROUNDSTOP;
        if (m_canvasResourcesInterface) {
            color = m_canvasResourcesInterface->resource(KoCanvasResource::BackgroundColor).value<KoColor>();
        }
        opacitySlider->setEnabled(false);
    }
    else {
        type = COLORSTOP;
        opacitySlider->setEnabled(true);
    }

    stops.removeAt(currentStop);
    stops.insert(currentStop, KoGradientStop(t, color, type));
    m_gradient->setStops(stops);
    gradientSlider->update(); //setSelectedStopType(type);
    emit sigGradientChanged();
}

void KisStopGradientEditor::colorChanged(const KoColor& color)
{
    if (!m_gradient) return;

    QList<KoGradientStop> stops = m_gradient->stops();

    int currentStop = gradientSlider->selectedStop();
    double t = stops[currentStop].position;
    
    KoColor c(color);
    c.setOpacity(stops[currentStop].color.opacityU8());

    KoGradientStopType type = stops[currentStop].type;
    
    stops.removeAt(currentStop);
    stops.insert(currentStop, KoGradientStop(t, c, type));
    m_gradient->setStops(stops);
    gradientSlider->update();

    emit sigGradientChanged();
}

void KisStopGradientEditor::opacityChanged(qreal value)
{
    if (!m_gradient) return;

    QList<KoGradientStop> stops = m_gradient->stops();

    int currentStop = gradientSlider->selectedStop();
    double t = stops[currentStop].position;
    
    KoColor c = stops[currentStop].color;
    c.setOpacity(value);
    
    KoGradientStopType type = stops[currentStop].type;

    stops.removeAt(currentStop);
    stops.insert(currentStop, KoGradientStop(t, c, type));
    m_gradient->setStops(stops);
    gradientSlider->update();

    emit sigGradientChanged();
}


void KisStopGradientEditor::nameChanged()
{
    if (!m_gradient) return;

    m_gradient->setName(nameedit->text());
    emit sigGradientChanged();
}

void KisStopGradientEditor::reverse()
{
    if (!m_gradient) return;

    QList<KoGradientStop> stops = m_gradient->stops();
    QList<KoGradientStop> reversedStops;
    for(const KoGradientStop& stop : stops) {
        reversedStops.push_front(KoGradientStop(1 - stop.position, stop.color, stop.type));
    }
    m_gradient->setStops(reversedStops);
    gradientSlider->setSelectedStop(stops.size() - 1 - gradientSlider->selectedStop());

    emit sigGradientChanged();
}

void KisStopGradientEditor::sortByValue( SortFlags flags = SORT_ASCENDING )
{
    if (!m_gradient) return;

    bool ascending = (flags & SORT_ASCENDING) > 0;
    bool evenDistribution = (flags & EVEN_DISTRIBUTION) > 0;

    QList<KoGradientStop> stops = m_gradient->stops();
    const int stopCount = stops.size();

    QList<KoGradientStop> sortedStops;
    std::sort(stops.begin(), stops.end(), KoGradientStopValueSort());

    int stopIndex = 0;
    for (const KoGradientStop& stop : stops) {
        const float value = evenDistribution ? (float)stopIndex / (float)(stopCount - 1) : stop.color.toQColor().valueF();
        const float position = ascending ? value : 1.f - value;

        if (ascending) {
            sortedStops.push_back(KoGradientStop(position, stop.color, stop.type));
        } else {
            sortedStops.push_front(KoGradientStop(position, stop.color, stop.type));
        }

        stopIndex++;
    }

    m_gradient->setStops(sortedStops);
    gradientSlider->setSelectedStop(stopCount - 1);
    gradientSlider->update();

    emit sigGradientChanged();
}

void KisStopGradientEditor::showContextMenu(const QPoint &origin)
{
    QMenu contextMenu(i18n("Options"), this);

    QAction reverseValues(i18n("Reverse Values"), this);
    connect(&reverseValues, &QAction::triggered, this, &KisStopGradientEditor::reverse);

    QAction sortAscendingValues(i18n("Sort by Value"), this);
    connect(&sortAscendingValues, &QAction::triggered, this, [this]{ this->sortByValue(SORT_ASCENDING); } );
    QAction sortAscendingDistributed(i18n("Sort by Value (Even Distribution)"), this);
    connect(&sortAscendingDistributed, &QAction::triggered, this, [this]{ this->sortByValue(SORT_ASCENDING | EVEN_DISTRIBUTION);} );

    contextMenu.addAction(&reverseValues);
    contextMenu.addSeparator();
    contextMenu.addAction(&sortAscendingValues);
    contextMenu.addAction(&sortAscendingDistributed);

    contextMenu.exec(mapToGlobal(origin));
}

