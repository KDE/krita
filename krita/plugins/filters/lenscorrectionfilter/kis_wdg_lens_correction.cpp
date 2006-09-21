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

#include "kis_wdg_lens_correction.h"

#include <qlayout.h>

#include <knuminput.h>

#include "ui_wdglenscorrectionoptions.h"

KisWdgLensCorrection::KisWdgLensCorrection(KisFilter* /*nfilter*/, QWidget* parent, const char* name)
    : KisFilterConfigWidget(parent,name)
{
    m_widget = new Ui_WdgLensCorrectionOptions();
    m_widget->setupUi(this);

    connect( widget()->intXCenter, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( widget()->intYCenter, SIGNAL( valueChanged(int)), SIGNAL(sigPleaseUpdatePreview()));
    connect( widget()->dblCorrectionNearCenter, SIGNAL( valueChanged(double)), SIGNAL(sigPleaseUpdatePreview()));
    connect( widget()->dblCorrectionNearEdges, SIGNAL( valueChanged(double)), SIGNAL(sigPleaseUpdatePreview()));
    connect( widget()->dblBrightness, SIGNAL( valueChanged(double)), SIGNAL(sigPleaseUpdatePreview()));
}

KisWdgLensCorrection::~KisWdgLensCorrection()
{
}

void KisWdgLensCorrection::setConfiguration(KisFilterConfiguration* config)
{
    QVariant value;
    if (config->getProperty("xcenter", value))
    {
        widget()->intXCenter->setValue( value.toUInt() );
    }
    if (config->getProperty("ycenter", value))
    {
        widget()->intYCenter->setValue( value.toUInt() );
    }
    if (config->getProperty("correctionnearcenter", value))
    {
        widget()->dblCorrectionNearCenter->setValue( value.toDouble() );
    }
    if (config->getProperty("correctionnearedges", value))
    {
        widget()->dblCorrectionNearEdges->setValue( value.toDouble() );
    }
    if (config->getProperty("brightness", value))
    {
        widget()->dblBrightness->setValue( value.toDouble() );
    }
}


#include "kis_wdg_lens_correction.moc"

