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

#include <klocalizedstring.h>

#include <KoCompositeOp.h>
#include <KoID.h>

#include <filter/kis_filter_registry.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_configuration.h>
#include <kis_image.h>
#include <kis_paint_device.h>
#include <kis_config_widget.h>

#include "ui_wdgfilteroption.h"

#include "kis_paintop_lod_limitations.h"

class KisFilterOptionWidget : public QWidget, public Ui::FilterOpOptions
{
public:
    KisFilterOptionWidget(QWidget* parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }
};

KisFilterOption::KisFilterOption()
    : KisPaintOpOption(KisPaintOpOption::FILTER, true)
{
    setObjectName("KisFilterOption");

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

    connect(m_options->checkBoxSmudgeMode, SIGNAL(stateChanged(int)), this, SLOT(emitSettingChanged()));
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

bool KisFilterOption::smudgeMode() const
{
    return m_options->checkBoxSmudgeMode->isChecked();
}

void KisFilterOption::setNode(KisNodeWSP node)
{
    if (node && node->paintDevice()) {
        m_paintDevice = node->paintDevice();

        // The "not m_currentFilterConfigWidget" is a corner case
        // which happens because the first configuration settings is
        // created before any layer is selected in the view
        if (!m_currentFilterConfigWidget
                || (m_currentFilterConfigWidget
                    && static_cast<KisFilterConfiguration*>(m_currentFilterConfigWidget->configuration())->isCompatible(m_paintDevice)
                   )
           ) {
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
    }
    else {
        m_paintDevice = 0;
    }
}

void KisFilterOption::setImage(KisImageWSP image)
{
    m_image = image;
    if (!m_currentFilterConfigWidget) {
        updateFilterConfigWidget();
    }
}

void KisFilterOption::setCurrentFilter(const KoID& id)
{
    m_currentFilter = KisFilterRegistry::instance()->get(id.id());
    m_options->filtersList->setCurrent(id);
    updateFilterConfigWidget();
    emitSettingChanged();
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

    if (m_currentFilter && m_image && m_paintDevice) {
        m_currentFilterConfigWidget =
            m_currentFilter->createConfigurationWidget(m_options->grpFilterOptions,
                    m_paintDevice);
        if (m_currentFilterConfigWidget) {
            m_layout->addWidget(m_currentFilterConfigWidget);
            m_options->grpFilterOptions->updateGeometry();
            m_currentFilterConfigWidget->show();
            connect(m_currentFilterConfigWidget, SIGNAL(sigConfigurationUpdated()), this, SLOT(emitSettingChanged()));
        }
    }
    m_layout->update();
}

void KisFilterOption::writeOptionSetting(KisPropertiesConfiguration* setting) const
{
    if (!m_currentFilter) return;

    setting->setProperty(FILTER_ID, m_currentFilter->id());
    setting->setProperty(FILTER_SMUDGE_MODE, smudgeMode());
    if (filterConfig()) {
        setting->setProperty(FILTER_CONFIGURATION, filterConfig()->toXML());
    }
}

void KisFilterOption::readOptionSetting(const KisPropertiesConfiguration* setting)
{
    KoID id(setting->getString(FILTER_ID), "");
    setCurrentFilter(id);
    m_options->checkBoxSmudgeMode->setChecked(setting->getBool(FILTER_SMUDGE_MODE));
    KisFilterConfiguration* configuration = filterConfig();
    if (configuration) {
        configuration->fromXML(setting->getString(FILTER_CONFIGURATION));
        m_currentFilterConfigWidget->setConfiguration(configuration);
    }
}

void KisFilterOption::lodLimitations(KisPaintopLodLimitations *l) const
{
    KisFilterConfiguration *config = filterConfig();

    if (m_currentFilter && config) {
        QRect testRect(0,0,100,100);
        if (m_currentFilter->neededRect(testRect, config, 0) != testRect ||
            m_currentFilter->changedRect(testRect, config, 0) != testRect) {

            l->blockers << KoID("filter-nonlinear", i18nc("PaintOp instant preview limitation", "\"%1\" does not support scaled preview (non-linear filter)", config->name()));
        }
    }
}

#include "moc_kis_filter_option.cpp"
