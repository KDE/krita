/* This file is part of the KDE project
 * Copyright (C) Boudewijn Rempt <boud@valdyas.org>, (C) 2008
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include "kis_filter_option.h"

#include <QRect>
#include <QGridLayout>
#include <QLabel>

#include <klocale.h>

#include <KoColorSpace.h>
#include <KoCompositeOp.h>
#include <KoInputDevice.h>
#include <KoID.h>

#include <filter/kis_filter_registry.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_config_widget.h>

#include "ui_wdgfilteroption.h"

class KisFilterOptionWidget : public QWidget, public Ui::FilterOpOptions
{
public:
    KisFilterOptionWidget(QWidget* parent = 0)
            : QWidget(parent) {
        setupUi(this);
    }
};

KisFilterOption::KisFilterOption()
        : KisPaintOpOption(i18n("Filter"))
{
    m_image = 0;
    m_checkable = false;
    m_currentFilterConfigWidget = 0;

    m_options = new KisFilterOptionWidget;
    m_options->hide();
    setConfigurationPage(m_options);

    m_layout = new QGridLayout(m_options->grpFilterOptions);

    // Check which filters support painting
    QList<QString> l = KisFilterRegistry::instance()->keys();
    QList<KoID> l2;
    QList<QString>::iterator it;
    for (it = l.begin(); it !=  l.end(); ++it) {
        KisFilterSP f = KisFilterRegistry::instance()->value((*it));
        if (f->supportsPainting()) {
            l2.push_back(KoID(*it, f->name()));
        }
    }
    m_options->filtersList->setIDList(l2);
    connect(m_options->filtersList, SIGNAL(activated(const KoID &)), SLOT(setCurrentFilter(const KoID &)));
    if (!l2.empty()) {
        setCurrentFilter(l2.first());
    }


}

const KisFilterSP KisFilterOption::filter() const
{
    return m_currentFilter;
}

KisFilterConfiguration* KisFilterOption::filterConfig() const
{
    if (!m_currentFilterConfigWidget) return 0;
    return static_cast<KisFilterConfiguration*>(m_currentFilterConfigWidget->configuration());
}


bool KisFilterOption::ignoreAlpha()
{
    return m_options->checkBoxIgnoreAlpha->isChecked();
}


void KisFilterOption::setNode(KisNodeSP node)
{
    if (node) {
        m_paintDevice = node->paintDevice();

        // The "not m_currentFilterConfigWidget" is a corner case
        // which happens because the first configuration settings is
        // created before any layer is selected in the view
        if (!m_currentFilterConfigWidget ||
                (m_currentFilterConfigWidget && static_cast<KisFilterConfiguration*>(m_currentFilterConfigWidget->configuration())->isCompatible(m_paintDevice))) {

            if (m_currentFilter) {
                KisPropertiesConfiguration* configuration = 0;
                if (m_currentFilterConfigWidget)
                    configuration = m_currentFilterConfigWidget->configuration();

                setCurrentFilter(KoID(m_currentFilter->id()));
                if (configuration)
                    m_currentFilterConfigWidget->setConfiguration(configuration);
                delete configuration;
            }
        }
    } else
        m_paintDevice = 0;
}

void KisFilterOption::setImage( KisImageWSP image )
{
    if (image->objectName() == "stroke sample image") {
        m_image = new KisImage(*image.data());
    }
    else {
        m_image = image;
    }
}

void KisFilterOption::setCurrentFilter( const KoID& id)
{
    m_currentFilter = KisFilterRegistry::instance()->get(id.id());
    updateFilterConfigWidget();
}


void KisFilterOption::updateFilterConfigWidget()
{
    if (m_currentFilterConfigWidget) {
        m_currentFilterConfigWidget->hide();
        m_layout->removeWidget(m_currentFilterConfigWidget);
        m_layout->invalidate();
        delete m_currentFilterConfigWidget;
    }
    m_currentFilterConfigWidget = 0;

    if (m_currentFilter && m_image) {
        m_currentFilterConfigWidget =
            m_currentFilter->createConfigurationWidget(m_options->grpFilterOptions,
                    m_paintDevice,
                    m_image);
        if (m_currentFilterConfigWidget) {
            m_layout->addWidget(m_currentFilterConfigWidget);
            m_options->grpFilterOptions->updateGeometry();
            m_currentFilterConfigWidget->show();
            connect(m_currentFilterConfigWidget, SIGNAL(sigConfigurationUpdated()), this, SIGNAL(sigSettingChanged()));
        }

    }
    m_layout->update();
}



void KisFilterOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    Q_UNUSED(setting);
#ifdef __GNUC__
#warning "KisFilterOption::writeOptionSetting: write the filter option setting"
#endif
}

void KisFilterOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    Q_UNUSED(setting);
#ifdef __GNUC__
#warning "KisFilterOption::readOptionSetting: restore the filter option setting"
#endif
}

#include "kis_filter_option.moc"
