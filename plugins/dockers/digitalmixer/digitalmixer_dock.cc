/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This library is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "digitalmixer_dock.h"

#include <QGridLayout>
#include <QToolButton>
#include <QSignalMapper>

#include <klocalizedstring.h>

#include <KoColorPatch.h>
#include <KoColorSlider.h>
#include <KoColorPopupAction.h>
#include <KoColorSpaceRegistry.h>
#include <KoCanvasResourceProvider.h>
#include <KoCanvasBase.h>

#include <kis_color_button.h>

class DigitalMixerPatch : public KoColorPatch {
    public:
        DigitalMixerPatch(QWidget* parent) : KoColorPatch(parent) {}
        QSize sizeHint() const override
        {
            return QSize(24,24);
        }
};

DigitalMixerDock::DigitalMixerDock( )
    : QDockWidget(i18n("Digital Colors Mixer")), m_canvas(0)
    , m_tellCanvas(true)
{
    const KoColorSpace *sRGB = KoColorSpaceRegistry::instance()->rgb8();
    KoColor initColors[6] = { KoColor(Qt::black, sRGB),
                              KoColor(Qt::white, sRGB),
                              KoColor(Qt::red, sRGB),
                              KoColor(Qt::green, sRGB),
                              KoColor(Qt::blue, sRGB),
                              KoColor(Qt::yellow, sRGB) };

    QWidget* widget = new QWidget(this);
    QGridLayout* layout = new QGridLayout( widget );

    // Current Color
    m_currentColorPatch = new KoColorPatch(this);
    m_currentColorPatch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_currentColorPatch->setMinimumWidth(48);
    layout->addWidget(m_currentColorPatch, 0, 0,3,1);

    // Create the sliders

    QSignalMapper* signalMapperSelectColor = new QSignalMapper(this);
    connect(signalMapperSelectColor, SIGNAL(mapped(int)), SLOT(popupColorChanged(int)));

    QSignalMapper* signalMapperColorSlider = new QSignalMapper(this);
    connect(signalMapperColorSlider, SIGNAL(mapped(int)), SLOT(colorSliderChanged(int)));

    QSignalMapper* signalMapperTargetColor = new QSignalMapper(this);
    connect(signalMapperTargetColor, SIGNAL(mapped(int)), SLOT(targetColorChanged(int)));

    for(int i = 0; i < 6; ++i)
    {
        Mixer mixer;
        mixer.targetColor = new DigitalMixerPatch(this);
        mixer.targetColor->setFixedSize(32, 22);
        layout->addWidget(mixer.targetColor, 0, i + 1);
        mixer.targetSlider = new KoColorSlider(Qt::Vertical, this);
        mixer.targetSlider->setFixedWidth(22);
        mixer.targetSlider->setMinimumHeight(66);
        layout->addWidget(mixer.targetSlider, 1, i + 1);
        mixer.actionColor = new KisColorButton( this );
        mixer.actionColor->setColor(initColors[i]);
        mixer.actionColor->setFixedWidth(22);
        layout->addWidget(mixer.actionColor, 2, i + 1);

        m_mixers.push_back(mixer);

        connect(mixer.actionColor, SIGNAL(changed(KoColor)), signalMapperSelectColor, SLOT(map()));
        signalMapperSelectColor->setMapping(mixer.actionColor, i);

        connect(mixer.targetSlider, SIGNAL(valueChanged(int)), signalMapperColorSlider, SLOT(map()));
        signalMapperColorSlider->setMapping(mixer.targetSlider, i);
        mixer.targetSlider->setValue(125);

        connect(mixer.targetColor, SIGNAL(triggered(KoColorPatch*)), signalMapperTargetColor, SLOT(map()));
        signalMapperTargetColor->setMapping(mixer.targetColor, i);
    }
    setCurrentColor(KoColor(Qt::black, KoColorSpaceRegistry::instance()->rgb8()));
    setWidget( widget );
}

void DigitalMixerDock::setCanvas(KoCanvasBase * canvas)
{
    setEnabled(canvas != 0);

    if (m_canvas) {
        m_canvas->disconnectCanvasObserver(this);
    }

    m_canvas = canvas;

    if (m_canvas) {
        connect(m_canvas->resourceManager(), SIGNAL(canvasResourceChanged(int,QVariant)),
                this, SLOT(canvasResourceChanged(int,QVariant)));

        m_tellCanvas=false;
        setCurrentColor(m_canvas->resourceManager()->foregroundColor());
        m_tellCanvas=true;
    }
}

void DigitalMixerDock::popupColorChanged(int i)
{
    KoColor color = m_mixers[i].actionColor->color();
    color.convertTo(m_currentColor.colorSpace());
    m_mixers[i].targetSlider->setColors( color, m_currentColor);
    colorSliderChanged(i);
}

void DigitalMixerDock::colorSliderChanged(int i)
{
    m_mixers[i].targetColor->setColor(m_mixers[i].targetSlider->currentColor());
}

void DigitalMixerDock::targetColorChanged(int i)
{
    setCurrentColor(m_mixers[i].targetColor->color());
}

void DigitalMixerDock::setCurrentColor(const KoColor& color)
{
    m_currentColor = color;
    m_currentColorPatch->setColor(color);
    for(int i = 0; i < m_mixers.size(); ++i)
    {
        popupColorChanged(i);
        colorSliderChanged(i);
    }
    if (m_canvas && m_tellCanvas)
    {
        m_canvas->resourceManager()->setForegroundColor(m_currentColor);
    }
}

void DigitalMixerDock::canvasResourceChanged(int key, const QVariant& v)
{
    m_tellCanvas = false;
    if (key == KoCanvasResourceProvider::ForegroundColor)
        setCurrentColor(v.value<KoColor>());
    m_tellCanvas = true;
}

