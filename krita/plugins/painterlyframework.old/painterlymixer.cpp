/*
 * painterlymixer.cc -- Part of Krita
 *
 * Copyright (c) 2007 Boudewijn Rempt (boud@valdyas.org)
 * Copyright (c) 2007 Emanuele Tamponi (emanuele@valinor.it)
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

#include <klocale.h>
#include <kiconloader.h>
#include <kdebug.h>
#include <kgenericfactory.h>

#include "kis_view2.h"
#include "painterlymixer.h"
#include "kis_painterlymixerdocker.h"

typedef KGenericFactory<PainterlyMixer> PainterlyMixerFactory;
K_EXPORT_COMPONENT_FACTORY( kritapainterlymixer, PainterlyMixerFactory( "krita" ) )

PainterlyMixer::PainterlyMixer(QObject *parent, const QStringList &)
    : KParts::Plugin(parent)
{
    if (parent->inherits("KisView2"))
    {
        setComponentData(PainterlyMixerFactory::componentData());
        m_view = static_cast<KisView2*>(parent);
        m_factory = new KisPainterlyMixerDockerFactory(m_view);
        m_view->createDockWidget(m_factory);
    }
}

PainterlyMixer::~PainterlyMixer()
{
    delete m_factory;
    m_view = 0;
}

#include "painterlymixer.moc"
