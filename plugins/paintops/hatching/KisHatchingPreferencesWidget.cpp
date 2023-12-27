/*
 *  SPDX-FileCopyrightText: 2008 Lukas Tvrdy <lukast.dev@gmail.com>
 *  SPDX-FileCopyrightText: 2010 José Luis Vergara <pentalis@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisHatchingPreferencesWidget.h"

#include <lager/constant.hpp>
#include "ui_wdghatchingpreferences.h"

#include "KisHatchingPreferencesModel.h"
#include <kis_paintop_lod_limitations.h>

namespace {


class KisHatchingPreferences: public QWidget, public Ui::WdgHatchingPreferences
{
public:
    KisHatchingPreferences(QWidget *parent = 0)
        : QWidget(parent) {
        setupUi(this);
    }
};


}


struct KisHatchingPreferencesWidget::Private
{
    Private(lager::cursor<KisHatchingPreferencesData> optionData)
        : model(optionData)
    {
    }

    KisHatchingPreferencesModel model;
};


KisHatchingPreferencesWidget::KisHatchingPreferencesWidget(lager::cursor<KisHatchingPreferencesData> optionData)
    : KisPaintOpOption(i18n("Hatching preferences"), KisPaintOpOption::GENERAL, true)
    , m_d(new Private(optionData))
{

	KisHatchingPreferences *widget = new KisHatchingPreferences();
	setObjectName("KisHatchingPreferences");

	m_checkable = false;

    using namespace KisWidgetConnectionUtils;

    connectControl(widget->antialiasCheckBox, &m_d->model, "useAntialias");
    connectControl(widget->opaqueBackgroundCheckBox, &m_d->model, "useOpaqueBackground");
    connectControl(widget->subpixelPrecisionCheckBox, &m_d->model, "useSubpixelPrecision");

    m_d->model.optionData.bind(std::bind(&KisHatchingPreferencesWidget::emitSettingChanged, this));

    setConfigurationPage(widget);
}

KisHatchingPreferencesWidget::~KisHatchingPreferencesWidget()
{
}

void KisHatchingPreferencesWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    KisHatchingPreferencesData data = *m_d->model.optionData;
    data.write(setting.data());
}

void KisHatchingPreferencesWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisHatchingPreferencesData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
