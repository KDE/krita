/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisFilterOptionWidget.h"

#include <QWidget>
#include <QCheckBox>
#include <QGridLayout>

#include <kis_signals_blocker.h>

#include <kis_config_widget.h>
#include <filter/kis_filter.h>
#include <filter/kis_filter_registry.h>
#include <filter/kis_filter_configuration.h>

#include <KisFilterOptionModel.h>
#include <KisWidgetConnectionUtils.h>

#include "ui_wdgfilteroption.h"


namespace {
class FilterWidget : public QWidget, public Ui::FilterOpOptions
{
public:
    FilterWidget(QWidget* parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }
};
}


struct KisFilterOptionWidget::Private
{
    Private(lager::cursor<KisFilterOptionData> optionData)
        : model(optionData)
    {
    }

    QGridLayout *layout {0};

    KisFilterOptionModel model;

    KisImageWSP image;
    KisPaintDeviceSP paintDevice;

    KisFilterSP currentFilter;
    FilterWidget *page {0};
    KisConfigWidget *currentFilterConfigWidget {0};
};


KisFilterOptionWidget::KisFilterOptionWidget(lager::cursor<KisFilterOptionData> optionData)
    : KisPaintOpOption(i18n("Filter"), KisPaintOpOption::COLOR, true)
    , m_d(new Private(optionData))
{
    setObjectName("KisFilterOption");
    m_checkable = false;
    using namespace KisWidgetConnectionUtils;

    m_d->page = new FilterWidget();
    setConfigurationPage(m_d->page);

    m_d->layout = new QGridLayout(m_d->page->grpFilterOptions);

    // Check which filters support painting
    QList<QString> allFilters = KisFilterRegistry::instance()->keys();
    QList<KoID> supportedFilters;

    for (auto it = allFilters.begin(); it != allFilters.end(); ++it) {
        KisFilterSP f = KisFilterRegistry::instance()->value((*it));
        if (f->supportsPainting()) {
            supportedFilters.push_back(KoID(*it, f->name()));
        }
    }

    m_d->page->filtersList->setIDList(supportedFilters);

    m_d->model.LAGER_QT(effectiveFilterState).bind(
                kismpl::unzip_wrapper(
                    std::bind(&KisFilterOptionWidget::updateFilterState,
                              this,
                              std::placeholders::_1,
                              std::placeholders::_2,
                              false)));

    connect(m_d->page->filtersList, &KisCmbIDList::activated, this, &KisFilterOptionWidget::slotFilterIdChangedInGui);

    connectControl(m_d->page->checkBoxSmudgeMode, &m_d->model, "smudgeMode");

    m_d->model.optionData.bind(std::bind(&KisFilterOptionWidget::emitSettingChanged, this));
}

KisFilterOptionWidget::~KisFilterOptionWidget()
{
}

void KisFilterOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    m_d->model.bakedOptionData().write(setting.data());
}

void KisFilterOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisFilterOptionData data = m_d->model.bakedOptionData();
    data.read(setting.data());
    m_d->model.optionData.set(data);

}

void KisFilterOptionWidget::setImage(KisImageWSP image)
{
    m_d->image = image;
    m_d->model.LAGER_QT(effectiveFilterState).nudge();
}

void KisFilterOptionWidget::setNode(KisNodeWSP node)
{
    m_d->paintDevice = node && node->paintDevice() ? node->paintDevice() : nullptr;

    QString filterId;
    QString filterConfig;
    std::tie(filterId, filterConfig) = m_d->model.effectiveFilterState();
    updateFilterState(filterId, filterConfig, true);
}

void KisFilterOptionWidget::updateFilterState(const QString &filterId, const QString &filterConfig, bool forceResetWidget)
{
    if (!m_d->currentFilter ||
        m_d->currentFilter->id() != filterId) {

        KisSignalsBlocker b(m_d->page->filtersList);
        m_d->page->filtersList->setCurrent(filterId);

        m_d->currentFilter = KisFilterRegistry::instance()->value(filterId);
        KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->currentFilter);

        forceResetWidget = true;
    }

    if (!m_d->paintDevice) {
        forceResetWidget = true;
    }

    if (forceResetWidget && m_d->currentFilterConfigWidget) {
        m_d->currentFilterConfigWidget->hide();
        m_d->layout->removeWidget(m_d->currentFilterConfigWidget);
        m_d->layout->invalidate();
        delete m_d->currentFilterConfigWidget;
        m_d->currentFilterConfigWidget = nullptr;
    }

    if (m_d->paintDevice && !m_d->currentFilterConfigWidget) {
        m_d->currentFilterConfigWidget =
            m_d->currentFilter->createConfigurationWidget(m_d->page->grpFilterOptions, m_d->paintDevice, true);

        if (m_d->currentFilterConfigWidget) {
            KisSignalsBlocker b(m_d->currentFilterConfigWidget);

            // TODO: init resources in ctor
            m_d->currentFilterConfigWidget->setCanvasResourcesInterface(canvasResourcesInterface());
            m_d->currentFilterConfigWidget->setConfiguration(m_d->currentFilter->defaultConfiguration(resourcesInterface()));
            m_d->layout->addWidget(m_d->currentFilterConfigWidget);
            m_d->page->grpFilterOptions->updateGeometry();
            m_d->currentFilterConfigWidget->show();
            connect(m_d->currentFilterConfigWidget, SIGNAL(sigConfigurationUpdated()),
                    this, SLOT(slotFilterConfigChangedInGui()));
        }
    }

    if (m_d->currentFilterConfigWidget) {
        KisFilterConfigurationSP config =
            m_d->currentFilter->factoryConfiguration(resourcesInterface());

        config->fromXML(filterConfig);

        KisSignalsBlocker b(m_d->currentFilterConfigWidget);
        m_d->currentFilterConfigWidget->setConfiguration(config);
    }
}

void KisFilterOptionWidget::slotFilterIdChangedInGui(const KoID &filterId)
{
    if (m_d->currentFilter && m_d->currentFilter->id() == filterId.id()) return;

    KisFilterSP newFilter = KisFilterRegistry::instance()->value(filterId.id());
    KIS_SAFE_ASSERT_RECOVER_RETURN(newFilter);

    KisFilterConfigurationSP filterConfig = newFilter->defaultConfiguration(resourcesInterface());
    m_d->model.seteffectiveFilterState(std::make_tuple(filterId.id(), filterConfig->toXML()));
}

void KisFilterOptionWidget::slotFilterConfigChangedInGui()
{
    KIS_SAFE_ASSERT_RECOVER_RETURN(m_d->currentFilter);

    QString config;

    if (m_d->currentFilterConfigWidget) {
        config = m_d->currentFilterConfigWidget->configuration()->toXML();
    } else {
        KisFilterConfigurationSP filterConfig = m_d->currentFilter->defaultConfiguration(resourcesInterface());
        config = filterConfig->toXML();
    }

    m_d->model.seteffectiveFilterState(std::make_tuple(m_d->currentFilter->id(), config));
}
