/*
 * SPDX-FileCopyrightText: 2022 Sharaf Zaman <shzam@sdf.org>
 *
 * SPDX-License-Identifier: LGPL-2.0-or-later
 */

#include "KisDuplicateOptionWidget.h"

#include <QWidget>

#include <KisDuplicateOptionModel.h>
#include <KisWidgetConnectionUtils.h>

#include "ui_wdgduplicateop.h"

struct KisDuplicateOptionWidget::Private {
public:
    Private(lager::cursor<KisDuplicateOptionData> optionData)
        : model(optionData)
    {
    }

    KisDuplicateOptionModel model;
};

class KisDuplicateOpOptionsWidget : public QWidget, public Ui::DuplicateOpOptionsWidget
{
public:
    KisDuplicateOpOptionsWidget(QWidget *parent = 0)
        : QWidget(parent)
    {
        setupUi(this);
    }
};

KisDuplicateOptionWidget::KisDuplicateOptionWidget(lager::cursor<KisDuplicateOptionData> optionData)
    : KisPaintOpOption(i18n("Painting Mode"), KisPaintOpOption::COLOR, true)
    , m_d(new Private(optionData))
{
    KisDuplicateOpOptionsWidget *page = new KisDuplicateOpOptionsWidget;
    m_checkable = false;

    setObjectName("KisDuplicateOptionWidget");

    {
        using namespace KisWidgetConnectionUtils;
        connectControl(page->cbHealing, &m_d->model, "healing");
        connectControl(page->cbPerspective, &m_d->model, "correctPerspective");
        connectControl(page->cbSourcePoint, &m_d->model, "moveSourcePoint");
        connectControl(page->cbResetSourcePoint, &m_d->model, "resetSourcePoint");
        connectControl(page->chkCloneProjection, &m_d->model, "cloneFromProjection");
    }

    // This pushes our model into the setting.
    m_d->model.optionData.bind(std::bind(&KisDuplicateOptionWidget::emitSettingChanged, this));

    setConfigurationPage(page);

}

KisDuplicateOptionWidget::~KisDuplicateOptionWidget()
{
}

void KisDuplicateOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    m_d->model.optionData->write(setting.data());
}

void KisDuplicateOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisDuplicateOptionData optionData = *m_d->model.optionData;
    optionData.read(setting.data());
    m_d->model.optionData.set(optionData);
}
