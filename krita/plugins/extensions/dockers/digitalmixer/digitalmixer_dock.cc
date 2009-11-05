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

#include <klocale.h>

#include <KoColorPatch.h>
#include <KoColorSlider.h>
#include <KoColorPopupAction.h>
#include <KoColorSpaceRegistry.h>
#include <qsignalmapper.h>

class DigitalMixerPatch : public KoColorPatch {
    public:
        DigitalMixerPatch(QWidget* parent) : KoColorPatch(parent) {}
        QSize sizeHint() const
        {
            return QSize(24,24);
        }
};

DigitalMixerDock::DigitalMixerDock( KisView2 *view ) : QDockWidget(i18n("Digital Colors Mixer")), m_view(view)
{
    m_currentColor = KoColor(Qt::black, KoColorSpaceRegistry::instance()->rgb8());
    
    QColor initColors[6] = { Qt::black, Qt::white, Qt::red, Qt::green, Qt::blue, Qt::yellow };
    
    QWidget* widget = new QWidget(this);
    QGridLayout* layout = new QGridLayout( widget );
    
    // Current Color
    KoColorPatch* currentColor = new KoColorPatch(this);
    currentColor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    currentColor->setMinimumWidth(12);
    layout->addWidget(currentColor, 0, 0,3,1);
    
    // Create the sliders
    
    QSignalMapper* signalMapperSelectColor = new QSignalMapper(this);
    connect(signalMapperSelectColor, SIGNAL(mapped(int)), SLOT(popupColorChanged(int)));
    
    QSignalMapper* signalMapperColorSlider = new QSignalMapper(this);
    connect(signalMapperColorSlider, SIGNAL(mapped(int)), SLOT(colorSliderChanged(int)));
    
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
        
        mixer.targetSlider->setColors(mixer.actionColor->currentKoColor(), m_currentColor);
        
        m_mixers.push_back(mixer);
        
        connect(mixer.actionColor, SIGNAL(colorChanged(KoColor)), signalMapperSelectColor, SLOT(map()));
        signalMapperSelectColor->setMapping(mixer.actionColor, i);
        
        connect(mixer.targetSlider, SIGNAL(valueChanged(int)), signalMapperColorSlider, SLOT(map()));
        signalMapperColorSlider->setMapping(mixer.targetSlider, i);
        mixer.targetSlider->setValue(125);
    }
    
    
    setWidget( widget );
}

void DigitalMixerDock::popupColorChanged(int i)
{
    m_mixers[i].targetSlider->setColors(m_mixers[i].actionColor->currentKoColor(), m_currentColor);    
}

void DigitalMixerDock::colorSliderChanged(int i)
{
    m_mixers[i].targetColor->setColor(m_mixers[i].targetSlider->currentColor());
}

#include "digitalmixer_dock.moc"
