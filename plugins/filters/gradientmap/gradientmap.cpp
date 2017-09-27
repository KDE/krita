/*
 * This file is part of the KDE project
 *
 * Copyright (c) 2016 Spencer Brown <sbrown655@gmail.com>
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

#include "QObject"
#include "gradientmap.h"
#include <kpluginfactory.h>
#include <filter/kis_filter_registry.h>
#include "krita_filter_gradient_map.h"
#include "KoResourceServerProvider.h"
#include "kis_config_widget.h"
#include <KoAbstractGradient.h>
#include <KoStopGradient.h>


K_PLUGIN_FACTORY_WITH_JSON(KritaGradientMapFactory, "kritagradientmap.json", registerPlugin<KritaGradientMap>();)

KritaGradientMapConfigWidget::KritaGradientMapConfigWidget(QWidget *parent, KisPaintDeviceSP dev, Qt::WFlags f)
    : KisConfigWidget(parent, f)
{
    Q_UNUSED(dev)
    m_page = new WdgGradientMap(this);
    QHBoxLayout *l = new QHBoxLayout(this);
    Q_CHECK_PTR(l);
    l->addWidget(m_page);
    l->setContentsMargins(0, 0, 0, 0);

    connect(m_page->gradientchooser, SIGNAL(resourceSelected(KoResource*)), SIGNAL(sigConfigurationItemChanged()));
}

KritaGradientMapConfigWidget::~KritaGradientMapConfigWidget()
{
    delete m_page;
}

void KritaGradientMapConfigWidget::gradientResourceChanged(KoResource* gradient)
{
    Q_UNUSED(gradient)
}

KisPropertiesConfigurationSP KritaGradientMapConfigWidget::configuration() const
{
    KisFilterConfigurationSP cfg = new KisFilterConfiguration("gradientmap", 2);
    if (m_page && m_page->gradientchooser && m_page->gradientchooser->currentResource()) {
        KoAbstractGradient *gradient = dynamic_cast<KoAbstractGradient*>(m_page->gradientchooser->currentResource());
        KoStopGradient *stopGradient = KoStopGradient::fromQGradient(gradient->toQGradient());
        QDomDocument doc;
        QDomElement elt = doc.createElement("gradient");
        stopGradient->toXML(doc, elt);
        doc.appendChild(elt);
        cfg->setProperty("gradientXML", doc.toString());
    }

    return cfg;
}

//-----------------------------

void KritaGradientMapConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    //m_page->gradientchooser->setCurrentResource(KoResourceServerProvider::instance()->gradientServer(false)->resourceByName(config->getString("gradientName")));
    Q_ASSERT(config);
    Q_UNUSED(config);
}

void KritaGradientMapConfigWidget::setView(KisViewManager *view)
{
    Q_UNUSED(view)
}
//------------------------------
KritaGradientMap::KritaGradientMap(QObject *parent, const QVariantList &)
        : QObject(parent)
{
    KisFilterRegistry::instance()->add(KisFilterSP(new KritaFilterGradientMap()));
}

KritaGradientMap::~KritaGradientMap()
{
}

//-----------------------------



#include "gradientmap.moc"
