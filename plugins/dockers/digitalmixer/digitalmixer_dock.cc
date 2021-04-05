/*
 *  SPDX-FileCopyrightText: 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "digitalmixer_dock.h"

#include <QGridLayout>
#include <QToolButton>
#include <KisSignalMapper.h>

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

    KisSignalMapper* signalMapperSelectColor = new KisSignalMapper(this);
    connect(signalMapperSelectColor, SIGNAL(mapped(int)), SLOT(popupColorChanged(int)));

    KisSignalMapper* signalMapperColorSlider = new KisSignalMapper(this);
    connect(signalMapperColorSlider, SIGNAL(mapped(int)), SLOT(colorSliderChanged(int)));

    KisSignalMapper* signalMapperTargetColor = new KisSignalMapper(this);
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

    //Gradient Mixer
    KisSignalMapper* signalMapperGradientStartColor = new KisSignalMapper(this);
    connect(signalMapperGradientStartColor, SIGNAL(mapped(int)), SLOT(gradientStartColorChanged(int)));

    KisSignalMapper* signalMapperGradientColorSlider = new KisSignalMapper(this);
    connect(signalMapperGradientColorSlider, SIGNAL(mapped(int)), SLOT(gradientColorSliderChanged(int)));

    KisSignalMapper* signalMapperGradientEndColor = new KisSignalMapper(this);
    connect(signalMapperGradientEndColor, SIGNAL(mapped(int)), SLOT(gradientEndColorChanged(int)));

    KisSignalMapper* signalMapperGradientTargetColor = new KisSignalMapper(this);
    connect(signalMapperGradientTargetColor, SIGNAL(mapped(int)), SLOT(gradientTargetColorChanged(int)));


    m_gradientMixer.targetColor = new DigitalMixerPatch(this);
    m_gradientMixer.targetColor->setFixedSize(32,32);
    layout->addWidget(m_gradientMixer.targetColor, 3, 0);

    m_gradientMixer.startColor = new KisColorButton(this);
    m_gradientMixer.startColor->setColor(KoColor(Qt::black, sRGB));
    m_gradientMixer.startColor->setFixedWidth(22);
    layout->addWidget(m_gradientMixer.startColor, 3, 1);

    m_gradientMixer.targetSlider = new KoColorSlider(Qt::Horizontal, this);
    m_gradientMixer.targetSlider->setFixedHeight(22);
    // m_gradientMixer.targetSlider->setMinimumWidth(20);
    // m_gradientMixer.targetSlider->setMaximumWidth(120);
    // m_gradientMixer.targetSlider->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    layout->addWidget(m_gradientMixer.targetSlider, 3, 2, 1, 4);

    m_gradientMixer.endColor = new KisColorButton(this);
    m_gradientMixer.endColor->setColor(KoColor(Qt::white, sRGB));
    m_gradientMixer.endColor->setFixedWidth(22);
    layout->addWidget(m_gradientMixer.endColor, 3, 6);

    connect(m_gradientMixer.startColor, SIGNAL(changed(KoColor)), signalMapperGradientStartColor, SLOT(map()));
    signalMapperGradientStartColor->setMapping(m_gradientMixer.startColor, 6);

    connect(m_gradientMixer.targetSlider, SIGNAL(valueChanged(int)), signalMapperGradientColorSlider, SLOT(map()));
    signalMapperGradientColorSlider->setMapping(m_gradientMixer.targetSlider, 6);
    m_gradientMixer.targetSlider->setValue(125);

    connect(m_gradientMixer.endColor, SIGNAL(changed(KoColor)), signalMapperGradientEndColor, SLOT(map()));
    signalMapperGradientEndColor->setMapping(m_gradientMixer.endColor, 6);

    connect(m_gradientMixer.targetColor, SIGNAL(triggered(KoColorPatch*)), signalMapperGradientTargetColor, SLOT(map()));
    signalMapperGradientTargetColor->setMapping(m_gradientMixer.targetColor, 6);

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


void DigitalMixerDock::gradientStartColorChanged(int id)
{
    KoColor color = m_gradientMixer.startColor->color();
    KoColor end_color = m_gradientMixer.endColor->color();
    color.convertTo(end_color.colorSpace());
    m_gradientMixer.targetSlider->setColors(color, end_color);

    gradientColorSliderChanged(id);
}

void DigitalMixerDock::gradientColorSliderChanged(int /*id*/)
{
    m_gradientMixer.targetColor->setColor(m_gradientMixer.targetSlider->currentColor());
}

void DigitalMixerDock::gradientEndColorChanged(int id)
{
    KoColor color = m_gradientMixer.endColor->color();
    KoColor start_color = m_gradientMixer.startColor->color();
    color.convertTo(start_color.colorSpace());
    m_gradientMixer.targetSlider->setColors(start_color, color);

    gradientColorSliderChanged(id);
}

void DigitalMixerDock::gradientTargetColorChanged(int /*id*/)
{
    setCurrentColor(m_gradientMixer.targetColor->color());
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
    if (key == KoCanvasResource::ForegroundColor)
        setCurrentColor(v.value<KoColor>());
    m_tellCanvas = true;
}

