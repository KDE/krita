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

    connect( widget()->intHWavelength, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( widget()->intHShift, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( widget()->intHAmplitude, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( widget()->cbHShape, SIGNAL( activated(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( widget()->intVWavelength, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( widget()->intVShift, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( widget()->intVAmplitude, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( widget()->cbVShape, SIGNAL( activated(int)), SIGNAL(sigPleaseUpdatePreview()));
}

KisWdgWave::~KisWdgWave()
{
}

void KisWdgWave::setConfiguration(KisFilterConfiguration* config)
{
    QVariant value;
    if (config->getProperty("horizontalwavelength", value))
    {
        widget()->intHWavelength->setValue( value.toUInt() );
    }
    if (config->getProperty("horizontalshift", value))
    {
        widget()->intHShift->setValue( value.toUInt() );
    }
    if (config->getProperty("horizontalamplitude", value))
    {
        widget()->intHAmplitude->setValue( value.toUInt() );
    }
    if (config->getProperty("horizontalshape", value))
    {
        widget()->cbHShape->setCurrentItem( value.toUInt() );
    }
    if (config->getProperty("verticalwavelength", value))
    {
        widget()->intVWavelength->setValue( value.toUInt() );
    }
    if (config->getProperty("verticalshift", value))
    {
        widget()->intVShift->setValue( value.toUInt() );
    }
    if (config->getProperty("verticalamplitude", value))
    {
        widget()->intVAmplitude->setValue( value.toUInt() );
    }
    if (config->getProperty("verticalshape", value))
    {
        widget()->cbVShape->setCurrentItem( value.toUInt() );
    }
}


#include "kis_wdg_wave.moc"

