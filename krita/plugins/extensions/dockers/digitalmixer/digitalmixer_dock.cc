/*
 *  Copyright (c) 2009 Cyrille Berger <cberger@cberger.net>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU Lesser General Public License as published by
 *  the Free Software Foundation; version 2 of the License.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301, USA.
 */

#include "digitalmixer_dock.h"

#include <kis_view2.h>

#include <QGridLayout>
#include <QToolButton>
#include <QSignalMapper>

#include <klocale.h>

#include <KoColorPatch.h>
#include <KoColorSlider.h>
#include <KoColorPopupAction.h>
#include <KoColorSpaceRegistry.h>
#include <KoResourceManager.h>
#include <KoCanvasBase.h>

class DigitalMixerPatch : public KoColorPatch {
    public:
        DigitalMixerPatch(QWidget* parent) : KoColorPatch(parent) {}
        QSize sizeHint() const
        {
            return QSize(24,24);
        }
};

DigitalMixerDock::DigitalMixerDock( KisView2 *view ) : QDockWidget(i18n("Digital Colors Mixer")), m_canvas(0), m_view(view), m_tellCanvas(true)
{
    QColor initColors[6] = { Qt::black, Qt::white, Qt::red, Qt::green, Qt::blue, Qt::yellow };
    
    QWidget* widget = new QWidget(this);
    QGridLayout* layout = new QGridLayout( widget );
    
    // Current Color
    m_currentColorPatch = new KoColorPatch(this);
    m_currentColorPatch->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    m_currentColorPatch->setMinimumWidth(12);
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
        layout->addWidget(mixer.targetColor, 0, i + 1);
        mixer.targetSlider = new KoColorSlider(Qt::Vertical, this);
        layout->addWidget(mixer.targetSlider, 1, i + 1);
        QToolButton* colorSelector = new QToolButton( this );
        mixer.actionColor = new KoColorPopupAction(this);
        mixer.actionColor->setCurrentColor(initColors[i]);
        colorSelector->setDefaultAction(mixer.actionColor);
        layout->addWidget(colorSelector, 2, i + 1);
                
        m_mixers.push_back(mixer);
        
        connect(mixer.actionColor, SIGNAL(colorChanged(KoColor)), signalMapperSelectColor, SLOT(map()));
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
    m_canvas = canvas;
    connect(m_canvas->resourceManager(), SIGNAL(resourceChanged(int, const QVariant&)),
            this, SLOT(resourceChanged(int, const QVariant&)));
    setCurrentColor(m_canvas->resourceManager()->foregroundColor());
}

void DigitalMixerDock::popupColorChanged(int i)
{
    KoColor color = m_mixers[i].actionColor->currentKoColor();
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

void DigitalMixerDock::resourceChanged(int key, const QVariant& v)
{
    m_tellCanvas = false;
    if (key == KoCanvasResource::ForegroundColor)
        setCurrentColor(v.value<KoColor>());
    m_tellCanvas = true;
}

#include "digitalmixer_dock.moc"
