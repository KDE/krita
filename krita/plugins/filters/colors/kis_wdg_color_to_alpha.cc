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

#include "kis_wdg_color_to_alpha.h"

#include <qcheckbox.h>
#include <qlayout.h>
#include <qspinbox.h>

#include <kcolorbutton.h>

#include <kis_filter.h>

#include "ui_wdgcolortoalphabase.h"

KisWdgColorToAlpha::KisWdgColorToAlpha( KisFilter* nfilter, QWidget * parent, const char * name) : KisFilterConfigWidget ( parent, name )
{
    m_widget = new Ui_WdgColorToAlphaBase();
    m_widget->setupUi(this);
    connect( m_widget->colorTarget, SIGNAL( changed(const QColor&)), SIGNAL(sigPleaseUpdatePreview()));
    connect( m_widget->intThreshold, SIGNAL( valueChanged ( int value) ), SIGNAL(sigPleaseUpdatePreview()));
}

void KisWdgColorToAlpha::setConfiguration(KisFilterConfiguration* config)
{
    QVariant value;
    if(config->getProperty("targetcolor", value))
    {
        m_widget->colorTarget->setColor( value.value<QColor>() );
    }
    if(config->getProperty("threshold", value))
    {
        m_widget->intThreshold->setValue( value.toInt() );
    }
}

#include "kis_wdg_color_to_alpha.moc"
