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

#include "KisWdgColorify.h"

#include <qcheckbox.h>
#include <qlayout.h>
#include <qspinbox.h>

#include <kcolorbutton.h>

#include <kis_filter.h>

#include "WdgColorifyBase.h"

KisWdgColorify::KisWdgColorify( KisFilter* nfilter, QWidget * parent, const char * name) : KisFilterConfigWidget ( parent, name )
{
    QGridLayout *widgetLayout = new QGridLayout(this, 1, 1);
    m_widget = new WdgColorifyBase(this);
    widgetLayout -> addWidget(m_widget,0,0);
    connect( m_widget->colorTarget, SIGNAL( changed(const QColor&)), SIGNAL(sigPleaseUpdatePreview()));
}

void KisWdgColorify::setConfiguration(KisFilterConfiguration* config)
{
    QVariant value;
    if(config->getProperty("color", value))
    {
        m_widget->colorTarget->setColor( value.toColor() );
    }
}

#include "KisWdgColorify.moc"
