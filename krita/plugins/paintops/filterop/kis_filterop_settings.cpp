/*
 *  Copyright (c) 2008 Boudewijn Rempt <boud@valdyas.org>
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
#include "kis_filterop_settings.h"

#include <QDomElement>
#include <QRect>
#include <QGridLayout>
#include <QLabel>
#include <kis_image.h>
#include <kis_debug.h>

#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoInputDevice.h>

#include <kis_brush.h>
#include <kis_global.h>
#include <kis_paint_device.h>
#include <kis_painter.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <filter/kis_filter_config_widget.h>
#include <kis_processing_information.h>
#include <filter/kis_filter_registry.h>
#include <kis_node.h>
#include <kis_types.h>
#include <kis_iterators_pixel.h>
#include <kis_paintop.h>
#include <kis_selection.h>
#include <kis_paintop_settings.h>
#include "ui_FilterOpOptionsWidget.h"



KisFilterOpSettings::KisFilterOpSettings(QWidget* parent, KisImageSP image)
    : QObject(parent)
    , KisPaintOpSettings()
    , m_optionsWidget(new QWidget(parent))
    , m_uiOptions(new Ui_FilterOpOptions())
    , m_currentFilterConfigWidget(0)
    , m_image(image)
{
    m_uiOptions->setupUi(m_optionsWidget);
    m_layout = new QGridLayout( m_uiOptions->grpFilterOptions );

    // Check which filters support painting
    QList<KoID> l = KisFilterRegistry::instance()->listKeys();
    QList<KoID> l2;
    QList<KoID>::iterator it;
    for (it = l.begin(); it !=  l.end(); ++it) {
        KisFilterSP f = KisFilterRegistry::instance()->value((*it).id());
        if (f->supportsPainting()) {
            l2.push_back(*it);
        }
    }
    m_uiOptions->filtersList->setIDList( l2 );
    connect(m_uiOptions->filtersList, SIGNAL(activated(const KoID &)), SLOT(setCurrentFilter(const KoID &)));
    if(!l2.empty())
    {
        setCurrentFilter( l2.first() );
    }

}

void KisFilterOpSettings::setNode( KisNodeSP node )
{
    if (node) {
        m_paintDevice = node->paintDevice();

        // The "not m_currentFilterConfigWidget" is a corner case
        // which happens because the first configuration settings is
        // created before any layer is selected in the view
        if( !m_currentFilterConfigWidget ||
            ( m_currentFilterConfigWidget && m_currentFilterConfigWidget->configuration()->isCompatible(m_paintDevice)) )
        {
            if(m_currentFilter)
            {
                setCurrentFilter(KoID(m_currentFilter->id()));
            }
        }
    }
    else
        m_paintDevice = 0;
}

KisFilterOpSettings::~KisFilterOpSettings()
{
    delete m_uiOptions;
}

void KisFilterOpSettings::setCurrentFilter(const KoID & id)
{
    m_currentFilter = KisFilterRegistry::instance()->get(id.id());
    updateFilterConfigWidget();
}

void KisFilterOpSettings::updateFilterConfigWidget()
{
    if ( m_currentFilterConfigWidget ) {
        m_currentFilterConfigWidget->hide();
        m_layout->removeWidget( m_currentFilterConfigWidget );
        m_layout->invalidate();
        delete m_currentFilterConfigWidget;
    }
    m_currentFilterConfigWidget = 0;

    if ( m_currentFilter ) {
        m_currentFilterConfigWidget =
            m_currentFilter->createConfigurationWidget(m_uiOptions->grpFilterOptions, m_paintDevice, m_image);
        if (m_currentFilterConfigWidget) {
            m_layout->addWidget( m_currentFilterConfigWidget );
            m_uiOptions->grpFilterOptions->updateGeometry();
            m_currentFilterConfigWidget->show();
        }
    }
    m_layout->update();
}

const KisFilterSP KisFilterOpSettings::filter() const
{
    return m_currentFilter;
}

KisFilterConfiguration* KisFilterOpSettings::filterConfig() const
{
    if(!m_currentFilterConfigWidget) return 0;
    return m_currentFilterConfigWidget->configuration();
}

bool KisFilterOpSettings::ignoreAlpha() const
{
    return m_uiOptions->checkBoxIgnoreAlpha->isChecked();
}

KisPaintOpSettingsSP KisFilterOpSettings::clone() const
{
    KisFilterOpSettings* s = new KisFilterOpSettings(0, m_image);
    s->m_paintDevice = m_paintDevice;
    s->setCurrentFilter( KoID(m_currentFilter->id()) );
    s->m_uiOptions->checkBoxIgnoreAlpha->setChecked( ignoreAlpha() );
    if(s->m_currentFilterConfigWidget && m_currentFilterConfigWidget)
    {
        s->m_currentFilterConfigWidget->setConfiguration( m_currentFilterConfigWidget->configuration() );
    }
    return s;
}

void KisFilterOpSettings::fromXML(const QDomElement& elt)
{
    QDomElement e = elt.firstChildElement( "Filter" );
    if( !e.isNull() )
    {
        QString filterName = e.attribute("name");
        m_currentFilter = KisFilterRegistry::instance()->get(filterName);
        if(m_currentFilter)
        {
            delete m_currentFilterConfigWidget;
            m_currentFilterConfigWidget = m_currentFilter->createConfigurationWidget(m_optionsWidget, m_paintDevice, m_image);
            KisFilterConfiguration * kfc = m_currentFilter->defaultConfiguration(m_paintDevice);
            if(kfc && m_currentFilterConfigWidget)
            {
                kfc->fromXML( e );
                m_currentFilterConfigWidget->setConfiguration( kfc );
            }
        }
        m_uiOptions->checkBoxIgnoreAlpha->setChecked( elt.attribute("IgnoreAlpha").toInt(0));
    }
}

void KisFilterOpSettings::toXML(QDomDocument& doc, QDomElement& rootElt) const
{
    QDomElement filterElt = doc.createElement( "Filter" );
    rootElt.appendChild( filterElt );
    if( m_currentFilterConfigWidget )
    {
        KisFilterConfiguration* config = m_currentFilterConfigWidget->configuration();
        config->toXML( doc, filterElt );
        delete config;
    }
    rootElt.setAttribute( "IgnoreAlpha", QString::number( ignoreAlpha() ) );
}

#include "kis_filterop_settings.moc"
