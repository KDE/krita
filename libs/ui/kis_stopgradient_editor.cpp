/*
 *  Copyright (c) 2004 Cyrille Berger <cberger@cberger.net>
 *                2016 Sven Langkamp <sven.langkamp@gmail.com>
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

#include "kis_stopgradient_editor.h"
#include <QPainter>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include <KoColorSpace.h>
#include <resources/KoStopGradient.h>

#include "kis_debug.h"

#include "widgets/kis_gradient_slider_widget.h"
#include <kis_icon_utils.h>

/****************************** KisStopGradientEditor ******************************/

KisStopGradientEditor::KisStopGradientEditor(KoStopGradient* gradient, QWidget *parent, const char* name, const QString& caption)
    : QWidget(parent), m_gradient(gradient)
{
    setObjectName(name);
    setupUi(this);
    setWindowTitle(caption);
    
    connect(gradientSlider, SIGNAL(sigSelectedStop(int)), this, SLOT(stopChanged(int)));
    gradientSlider->setGradientResource(m_gradient);

    nameedit->setText(gradient->name());
    connect(nameedit, SIGNAL(editingFinished()), this, SLOT(nameChanged()));

    connect(colorButton, SIGNAL(changed(const QColor&)), SLOT(colorChanged(const QColor&)));
    
    opacitySlider->setRange(0.0, 1.0, 2);
    connect(opacitySlider, SIGNAL(valueChanged(qreal)), this, SLOT(opacityChanged(qreal)));
    
    buttonReverse->setIcon(KisIconUtils::loadIcon("mirrorAxis-HorizontalMove"));
    KisIconUtils::updateIcon(buttonReverse);
    connect(buttonReverse, SIGNAL(pressed()), this, SLOT(reverse()));
    
    stopChanged(gradientSlider->selectedStop());
}

void KisStopGradientEditor::activate()
{
    paramChanged();
}

void KisStopGradientEditor::stopChanged(int stop)
{
    QColor color = m_gradient->stops()[stop].second.toQColor();
    opacitySlider->setValue(color.alphaF());
    
    color.setAlphaF(1.0);
    colorButton->setColor(color);
    
    paramChanged();
}

void KisStopGradientEditor::colorChanged(const QColor& color)
{
    QList<KoGradientStop> stops = m_gradient->stops();

    int currentStop = gradientSlider->selectedStop();
    double t = stops[currentStop].first;
    
    KoColor c(color, stops[currentStop].second.colorSpace());
    c.setOpacity(stops[currentStop].second.opacityU8());
    
    stops.removeAt(currentStop);
    stops.insert(currentStop, KoGradientStop(t, c));
    
    m_gradient->setStops(stops);

    paramChanged();
}

void KisStopGradientEditor::opacityChanged(qreal value)
{
    QList<KoGradientStop> stops = m_gradient->stops();

    int currentStop = gradientSlider->selectedStop();
    double t = stops[currentStop].first;
    
    KoColor c = stops[currentStop].second;
    c.setOpacity(value);
    
    stops.removeAt(currentStop);
    stops.insert(currentStop, KoGradientStop(t, c));
    
    m_gradient->setStops(stops);

    paramChanged();
}


void KisStopGradientEditor::nameChanged()
{
     m_gradient->setName(nameedit->text());
}

void KisStopGradientEditor::paramChanged()
{
    m_gradient->updatePreview();
    gradientSlider->update();     
}

void KisStopGradientEditor::reverse()
{
    QList<KoGradientStop> stops = m_gradient->stops();
    QList<KoGradientStop> reversedStops;
    for(const KoGradientStop& stop : stops)
    {
        reversedStops.push_front(KoGradientStop(1 - stop.first, stop.second));
    }
    m_gradient->setStops(reversedStops);
    gradientSlider->setSeletectStop(stops.size()-1 -gradientSlider->selectedStop());
    paramChanged();
}

