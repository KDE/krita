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
    QWidget* widget = new QWidget(this);
    QGridLayout* layout = new QGridLayout( widget );
    
    // Current Color
    KoColorPatch* currentColor = new KoColorPatch(this);
    currentColor->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    currentColor->setMinimumWidth(12);
    layout->addWidget(currentColor, 0, 0,3,1);
    
    // Create the sliders
    
    for(int i = 0; i < 6; ++i)
    {
        KoColorPatch* targetColor = new DigitalMixerPatch(this);
        layout->addWidget(targetColor, 0, i + 1);
        KoColorSlider* targetSlider = new KoColorSlider(Qt::Vertical, this);
        layout->addWidget(targetSlider, 1, i + 1);
        QToolButton* colorSelector = new QToolButton( this );
        KoColorPopupAction* m_actionColor = new KoColorPopupAction(this);
        colorSelector->setDefaultAction(m_actionColor);
        layout->addWidget(colorSelector, 2, i + 1);
    }
    
    setWidget( widget );
}

#include "digitalmixer_dock.moc"
