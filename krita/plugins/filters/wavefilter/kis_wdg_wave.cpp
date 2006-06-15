/*
 * This file is part of Krita
 *
 * Copyright (c) 2006 Cyrille Berger <cberger@cberger.net>
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

#include "kis_wdg_wave.h"

#include <qlayout.h>

#include <kcombobox.h>
#include <knuminput.h>

#include "wdgwaveoptions.h"

KisWdgWave::KisWdgWave(KisFilter* /*nfilter*/, QWidget* parent, const char* name)
    : KisFilterConfigWidget(parent,name)
{
    QGridLayout *widgetLayout = new QGridLayout(this, 1, 1);
    m_widget = new WdgWaveOptions(this);
    widgetLayout -> addWidget(m_widget, 0, 0);

    connect( widget()->intWavelength, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( widget()->intShift, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( widget()->intAmplitude, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( widget()->cbShape, SIGNAL( activated(int)), SIGNAL(sigPleaseUpdatePreview()));
}

KisWdgWave::~KisWdgWave()
{
}

void KisWdgWave::setConfiguration(KisFilterConfiguration* config)
{
    QVariant value;
    if (config->getProperty("wavelength", value))
    {
        widget()->intWavelength->setValue( value.toUInt() );
    }
    if (config->getProperty("shift", value))
    {
        widget()->intShift->setValue( value.toUInt() );
    }
    if (config->getProperty("amplitude", value))
    {
        widget()->intAmplitude->setValue( value.toUInt() );
    }
    if (config->getProperty("shape", value))
    {
        widget()->cbShape->setCurrentItem( value.toUInt() );
    }
}


#include "kis_wdg_wave.moc"

