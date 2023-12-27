/*
 *  SPDX-FileCopyrightText: 2022 Dmitry Kazakov <dimula73@gmail.com>
 *
 *  SPDX-License-Identifier: GPL-2.0-or-later
 */
#include "KisColorSourceOptionWidget.h"

#include <QGroupBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QVBoxLayout>

#include <KoID.h>

#include <KisColorSourceOptionModel.h>
#include <KisWidgetConnectionUtils.h>

struct KisColorSourceOptionWidget::Private
{
    Private(lager::cursor<KisColorSourceOptionData> optionData)
        : model(optionData)
    {
    }

    KisColorSourceOptionModel model;
};

KisColorSourceOptionWidget::KisColorSourceOptionWidget(lager::cursor<KisColorSourceOptionData> optionData)
    : KisPaintOpOption(i18nc("Color source", "Source"), KisPaintOpOption::COLOR, true)
    , m_d(new Private(optionData))
{
    using namespace KisWidgetConnectionUtils;

    setObjectName("KisColorSourceOptionWidget");
    m_checkable = false;

    QWidget* widget = new QWidget;

    QGroupBox* groupBox = new QGroupBox(widget);
    QVBoxLayout* verticalLayout = new QVBoxLayout(groupBox);

    QButtonGroup *buttonGroup = new QButtonGroup(widget);
    buttonGroup->setExclusive(true);

    Q_FOREACH (const KoID &id, KisColorSourceOptionData::colorSourceTypeIds()) {
        QRadioButton* radioButton = new QRadioButton(groupBox);
        radioButton->setText(id.name());
        verticalLayout->addWidget(radioButton);
        buttonGroup->addButton(radioButton, KisColorSourceOptionData::id2Type(id));
    }
    QVBoxLayout* verticalLayout_2 = new QVBoxLayout(widget);
    verticalLayout_2->setMargin(0);
    verticalLayout_2->addWidget(groupBox);
    verticalLayout_2->addStretch();

    setConfigurationPage(widget);

    connectControl(buttonGroup, &m_d->model, "type");

    m_d->model.optionData.bind(std::bind(&KisColorSourceOptionWidget::emitSettingChanged, this));
}

KisColorSourceOptionWidget::~KisColorSourceOptionWidget()
{
}

void KisColorSourceOptionWidget::writeOptionSetting(KisPropertiesConfigurationSP setting) const
{
    m_d->model.optionData->write(setting.data());
}

void KisColorSourceOptionWidget::readOptionSetting(const KisPropertiesConfigurationSP setting)
{
    KisColorSourceOptionData data = *m_d->model.optionData;
    data.read(setting.data());
    m_d->model.optionData.set(data);
}
