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
#include <KoGradientBackground.h>
#include <KoResourceServerAdapter.h>


K_PLUGIN_FACTORY_WITH_JSON(KritaGradientMapFactory, "kritagradientmap.json", registerPlugin<KritaGradientMap>();)

KritaGradientMapConfigWidget::KritaGradientMapConfigWidget(QWidget *parent, KisPaintDeviceSP dev, Qt::WindowFlags f)
    : KisConfigWidget(parent, f)
{
    Q_UNUSED(dev)
    m_page = new WdgGradientMap(this);
    QHBoxLayout *l = new QHBoxLayout(this);
    Q_CHECK_PTR(l);
    l->addWidget(m_page);
    l->setContentsMargins(0, 0, 0, 0);
    KoResourceServerProvider *serverProvider = KoResourceServerProvider::instance();
    QSharedPointer<KoAbstractResourceServerAdapter> gradientResourceAdapter(
                new KoResourceServerAdapter<KoAbstractGradient>(serverProvider->gradientServer()));
    m_gradientChangedCompressor = new KisSignalCompressor(100, KisSignalCompressor::FIRST_ACTIVE);

    m_gradientPopUp = new KoResourcePopupAction(gradientResourceAdapter,
                                                  m_page->btnGradientChooser);
    m_activeGradient = KoStopGradient::fromQGradient(dynamic_cast<KoAbstractGradient*>(gradientResourceAdapter->resources().first())->toQGradient());
    m_page->gradientEditor->setGradient(m_activeGradient);
    m_page->gradientEditor->setCompactMode(true);
    m_page->gradientEditor->setEnabled(true);
    m_page->btnGradientChooser->setDefaultAction(m_gradientPopUp);
    m_page->btnGradientChooser->setPopupMode(QToolButton::InstantPopup);
    connect(m_gradientPopUp, SIGNAL(resourceSelected(QSharedPointer<KoShapeBackground>)), this, SLOT(setAbstractGradientToEditor()));
    connect(m_page->gradientEditor, SIGNAL(sigGradientChanged()), m_gradientChangedCompressor, SLOT(start()));
    connect(m_gradientChangedCompressor, SIGNAL(timeout()), this, SIGNAL(sigConfigurationItemChanged()));

    QObject::connect(m_page->ditherGroupBox, &QGroupBox::toggled, this, &KisConfigWidget::sigConfigurationItemChanged);
    QObject::connect(m_page->ditherWidget, &KisDitherWidget::sigConfigurationItemChanged, this, &KisConfigWidget::sigConfigurationItemChanged);
}

KritaGradientMapConfigWidget::~KritaGradientMapConfigWidget()
{
    delete m_page;
}

void KritaGradientMapConfigWidget::setAbstractGradientToEditor()
{
    QSharedPointer<KoGradientBackground> bg =
        qSharedPointerDynamicCast<KoGradientBackground>(
            m_gradientPopUp->currentBackground());
    m_activeGradient = KoStopGradient::fromQGradient(bg->gradient());
    m_page->gradientEditor->setGradient(m_activeGradient);

}

KisPropertiesConfigurationSP KritaGradientMapConfigWidget::configuration() const
{
    KisFilterConfigurationSP cfg = new KisFilterConfiguration("gradientmap", 2);
    if (m_activeGradient) {
        QDomDocument doc;
        QDomElement elt = doc.createElement("gradient");
        m_activeGradient->toXML(doc, elt);
        doc.appendChild(elt);
        cfg->setProperty("gradientXML", doc.toString());
    }

    cfg->setProperty("ditherEnabled", m_page->ditherGroupBox->isChecked());
    m_page->ditherWidget->configuration(*cfg, "dither/");

    return cfg;
}

//-----------------------------

void KritaGradientMapConfigWidget::setConfiguration(const KisPropertiesConfigurationSP config)
{
    Q_ASSERT(config);
    QDomDocument doc;
    if (config->hasProperty("gradientXML")) {
        doc.setContent(config->getString("gradientXML", ""));
        KoStopGradient gradient = KoStopGradient::fromXML(doc.firstChildElement());
        if (gradient.stops().size()>0) {
            m_activeGradient->setStops(gradient.stops());
        }
    }

    m_page->ditherGroupBox->setChecked(config->getBool("ditherEnabled"));
    m_page->ditherWidget->setConfiguration(*config, "dither/");
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
