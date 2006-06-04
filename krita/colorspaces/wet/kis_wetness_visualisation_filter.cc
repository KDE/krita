/*
 * kis_wetness_visualisation_filter.cc -- Part of Krita
 *
 * Copyright (c) 2005 Bart Coppens <kde@bartcoppens.be>
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

#include <kdebug.h>

#include <klocale.h>
#include "kis_meta_registry.h"
#include <kis_view.h>
#include <kis_image.h>
#include <KoColorSpaceFactoryRegistry.h>
#include <kis_factory.h>
#include "kis_wet_colorspace.h"
#include <kis_debug_areas.h>
#include "kis_wetness_visualisation_filter.h"

WetnessVisualisationFilter::WetnessVisualisationFilter(KisView* view)
    : m_view(view), m_action(0) {
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(slotTimeout()));
}

// XXX this needs to work on a per-layer basis!

void WetnessVisualisationFilter::setAction(KToggleAction* action) {
    m_action = action;
    if (!m_action)
        return;
    KisWetColorSpace* cs = dynamic_cast<KisWetColorSpace*>(
            KisMetaRegistry::instance()->csRegistry()->getColorSpace(KoID("WET", ""),"") );
    Q_ASSERT(cs);
    m_action->setChecked(cs->paintWetness());
}

void WetnessVisualisationFilter::slotActivated() {
    kDebug(DBG_AREA_CMS) << "activated" << endl;
    if (!m_action) {
        kDebug(DBG_AREA_CMS) << "no action" << endl;
        return;
    }
    KisWetColorSpace* cs = dynamic_cast<KisWetColorSpace*>(
            KisMetaRegistry::instance()->csRegistry()->getColorSpace(KoID("WET", ""),"") );
    Q_ASSERT(cs);
    if (!m_action->isChecked()) {
        m_timer.stop();
        cs->setPaintWetness(false);
    } else {
        m_timer.start(500);
        cs->setPaintWetness(true);
    }
}

void WetnessVisualisationFilter::slotTimeout() {
    KisWetColorSpace* cs = dynamic_cast<KisWetColorSpace*>(
            KisMetaRegistry::instance()->csRegistry()->getColorSpace(KoID("WET", ""),"") );
    Q_ASSERT(cs);
    if (!cs) return;
    cs->resetPhase();

}

#include "kis_wetness_visualisation_filter.moc"
